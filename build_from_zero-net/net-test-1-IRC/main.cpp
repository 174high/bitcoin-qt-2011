#include <iostream>
#include <string.h>
#include <db_cxx.h>
//#include <db.h>
#include <QtDebug>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "uint256.h"
#include "util.h"
#include "serialize.h"
#include "net.h"
#include "db.h"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include "base58.h"
#include "sha.h"
#include <stdlib.h>
#include "strlcpy.h"
#include "main.h"

using namespace std;
using namespace boost;

map<uint256, CBlockIndex*> mapBlockIndex;
CBlockIndex* pindexGenesisBlock = NULL;
uint256 hashGenesisBlock("0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");

int nGotIRCAddresses = 0;
bool fGotExternalIP = false;

#pragma pack(push, 1)
struct ircaddr
{
    int ip;
    short port;
};
#pragma pack(pop)



string EncodeAddress(const CAddress& addr)
{
    struct ircaddr tmp;
    tmp.ip    = addr.ip;
    tmp.port  = addr.port;

    vector<unsigned char> vch(UBEGIN(tmp), UEND(tmp));
    return string("u") + EncodeBase58Check(vch);
}

bool DecodeAddress(string str, CAddress& addr)
{
    vector<unsigned char> vch;
    if (!DecodeBase58Check(str.substr(1), vch))
        return false;

    struct ircaddr tmp;
    if (vch.size() != sizeof(tmp))
        return false;
    memcpy(&tmp, &vch[0], sizeof(tmp));

    addr = CAddress(tmp.ip, ntohs(tmp.port), NODE_NETWORK);
    return true;
}



static bool Send(SOCKET hSocket, const char* pszSend)
{
    if (strstr(pszSend, "PONG") != pszSend)
        printf("IRC SENDING: %s\n", pszSend);
  
    printf("###########\n");

    const char* psz = pszSend;
    const char* pszEnd = psz + strlen(psz);
    while (psz < pszEnd)
    {
        int ret = send(hSocket, psz, pszEnd - psz, MSG_NOSIGNAL);
        if (ret < 0)
            return false;
        psz += ret;
    }
    return true;
}


bool RecvLine(SOCKET hSocket, string& strLine)
{
    strLine = "";
    loop
    {
        char c;
        int nBytes = recv(hSocket, &c, 1, 0);
        if (nBytes > 0)
        {
            if (c == '\n')
                continue;
            if (c == '\r')
                return true;
            strLine += c;
            if (strLine.size() >= 9000)
                return true;
        }
        else if (nBytes <= 0)
        {
            if (fShutdown)
                return false;
            if (nBytes < 0)
            {
                int nErr = WSAGetLastError();
                if (nErr == WSAEMSGSIZE)
                    continue;
                if (nErr == WSAEWOULDBLOCK || nErr == WSAEINTR || nErr == WSAEINPROGRESS)
                {
                    Sleep(10);
                    continue;
                }
            }
            if (!strLine.empty())
                return true;
            if (nBytes == 0)
            {
                // socket closed
                printf("IRC socket closed\n");
                return false;
            }
            else
            {
                // socket error
                int nErr = WSAGetLastError();
                printf("IRC recv failed: %d\n", nErr);
                return false;
            }
        }
    }
}


bool RecvLineIRC(SOCKET hSocket, string& strLine)
{
    loop
    {
        bool fRet = RecvLine(hSocket, strLine);
        if (fRet)
        {
            if (fShutdown)
                return false;
            vector<string> vWords;
            ParseString(strLine, ' ', vWords);
            if (vWords.size() >= 1 && vWords[0] == "PING")
            {
                strLine[1] = 'O';
                strLine += '\r';
                Send(hSocket, strLine.c_str());
                continue;
            }
        }
        return fRet;
    }
}



int RecvUntil(SOCKET hSocket, const char* psz1, const char* psz2=NULL, const char* psz3=NULL, const char* psz4=NULL)
{
    loop
    {
        string strLine;
        strLine.reserve(10000);
        if (!RecvLineIRC(hSocket, strLine))
            return 0;
        std::cout<<"IRC="<<strLine<<std::endl ;
        if (psz1 && strLine.find(psz1) != -1)
            return 1;
        if (psz2 && strLine.find(psz2) != -1)
            return 2;
        if (psz3 && strLine.find(psz3) != -1)
            return 3;
        if (psz4 && strLine.find(psz4) != -1)
            return 4;
    }
}


bool Wait(int nSeconds)
{
    if (fShutdown)
        return false;
    printf("IRC waiting %d seconds to reconnect\n", nSeconds);
    for (int i = 0; i < nSeconds; i++)
    {
        if (fShutdown)
            return false;
        Sleep(1000);
    }
    return true;
}

bool RecvCodeLine(SOCKET hSocket, const char* psz1, string& strRet)
{
    strRet.clear();
    loop
    {
        string strLine;
        if (!RecvLineIRC(hSocket, strLine))
            return false;

        vector<string> vWords;
        ParseString(strLine, ' ', vWords);
        if (vWords.size() < 2)
            continue;

        if (vWords[1] == psz1)
        {
            printf("IRC %s\n", strLine.c_str());
            strRet = strLine;
            return true;
        }
    }
}


bool GetIPFromIRC(SOCKET hSocket, string strMyName, unsigned int& ipRet)
{
    Send(hSocket, strprintf("USERHOST %s\r", strMyName.c_str()).c_str());

    string strLine;
    if (!RecvCodeLine(hSocket, "302", strLine))
        return false;

    vector<string> vWords;
    ParseString(strLine, ' ', vWords);
    if (vWords.size() < 4)
        return false;

    string str = vWords[3];
    if (str.rfind("@") == string::npos)
        return false;
    string strHost = str.substr(str.rfind("@")+1);

    // Hybrid IRC used by lfnet always returns IP when you userhost yourself,
    // but in case another IRC is ever used this should work.
    printf("GetIPFromIRC() got userhost %s\n", strHost.c_str());
    if (fUseProxy)
        return false;
    CAddress addr(strHost, 0, true);
    if (!addr.IsValid())
        return false;
    ipRet = addr.ip;

    return true;
}


int main (int argc, char *argv[])
{
    int nErrorWait = 10;
    int nRetryWait = 10;
    bool fNameInUse = false;

    while (!fShutdown)
    {
    //CAddress addrConnect("216.155.130.130:6667"); // chat.freenode.net
    CAddress addrConnect("92.243.23.21", 6667); // irc.lfnet.org

    addrConnect.print();

    //struct hostent* phostent = gethostbyname("chat.freenode.net");
    CAddress addrIRC("irc.lfnet.org", 6667, true);

    addrIRC.print();

    if (addrIRC.IsValid())
    addrConnect = addrIRC;

    addrConnect.print();

    SOCKET hSocket;
    if (!ConnectSocket(addrConnect, hSocket))
    {
            std::cout<<"IRC connect failed"<<std::endl ;
            nErrorWait = nErrorWait * 11 / 10;
            if (Wait(nErrorWait += 60))
                continue;
            else
                return 0;
    }

    if (!RecvUntil(hSocket, "Found your hostname", "using your IP address instead", "Couldn't look up your hostname", "ignoring hostname"))
    {
            closesocket(hSocket);
            hSocket = INVALID_SOCKET;
            nErrorWait = nErrorWait * 11 / 10;
            if (Wait(nErrorWait += 60))
                continue;
            else
                return 0;
    }

       string strMyName;
        if (addrLocalHost.IsRoutable() && !fUseProxy && !fNameInUse)
            strMyName = EncodeAddress(addrLocalHost);
        else
            strMyName = strprintf("x%u", GetRand(1000000000));

  
        std::cout<<"addrLocalHost=" ;
        addrLocalHost.print(); 
        std::cout<<"strMyName="<<strMyName<<std::endl  ; 

        Send(hSocket, strprintf("NICK %s\r", strMyName.c_str()).c_str());
        Send(hSocket, strprintf("USER %s 8 * : %s\r", strMyName.c_str(), strMyName.c_str()).c_str());

        int nRet = RecvUntil(hSocket, " 004 ", " 433 ");
        if (nRet != 1)
        {
            closesocket(hSocket);
            hSocket = INVALID_SOCKET;
            if (nRet == 2)
            {
                printf("IRC name already in use\n");
                fNameInUse = true;
                Wait(10);
                continue;
            }
            nErrorWait = nErrorWait * 11 / 10;
            if (Wait(nErrorWait += 60))
                continue;
            else
                return 0;
        }
        Sleep(500);

     // Get our external IP from the IRC server and re-nick before joining the channel
        CAddress addrFromIRC;
        if (GetIPFromIRC(hSocket, strMyName, addrFromIRC.ip))
        {
            printf("GetIPFromIRC() returned %s\n", addrFromIRC.ToStringIP().c_str());
             #ifdef DEBUG_NET && DEBUG_IRC
            std::cout<<__FUNCTION__<<"get ip from irc="<<addrFromIRC.ToStringIP()<<std::endl ;
            #endif
            if (!fUseProxy && addrFromIRC.IsRoutable())
            {
                // IRC lets you to re-nick
                fGotExternalIP = true;
                addrLocalHost.ip = addrFromIRC.ip;
                strMyName = EncodeAddress(addrLocalHost);
                Send(hSocket, strprintf("NICK %s\r", strMyName.c_str()).c_str());
            }
        }

       if (fTestNet) {
            Send(hSocket, "JOIN #bitcoinTEST\r");
            Send(hSocket, "WHO #bitcoinTEST\r");
        } else {
            // randomly join #bitcoin00-#bitcoin99
            int channel_number = GetRandInt(100);
            Send(hSocket, strprintf("JOIN #bitcoin%02d\r", channel_number).c_str());
            Send(hSocket, strprintf("WHO #bitcoin%02d\r", channel_number).c_str());
        }

        int64 nStart = GetTime();
        string strLine;
        strLine.reserve(10000);
        #ifdef DEBUG_NET && DEBUG_IRC
        std::cout<<__FUNCTION__<<" :2"<<std::endl ;
        #endif
        while (!fShutdown && RecvLineIRC(hSocket, strLine))
        {
            #ifdef DEBUG_NET && DEBUG_IRC
            std::cout<<__FUNCTION__<<" :3"<<std::endl ;
            #endif
            if (strLine.empty() || strLine.size() > 900 || strLine[0] != ':')
                continue;

            vector<string> vWords;
            ParseString(strLine, ' ', vWords);
            if (vWords.size() < 2)
                continue;

            char pszName[10000];
            pszName[0] = '\0';

            if (vWords[1] == "352" && vWords.size() >= 8)
            {
                // index 7 is limited to 16 characters
                // could get full length name at index 10, but would be different from join messages
                strlcpy(pszName, vWords[7].c_str(), sizeof(pszName));
                printf("IRC got who\n");
                #ifdef DEBUG_NET && DEBUG_IRC
                std::cout<<__FUNCTION__<<" :4 "<<pszName<<std::endl ;
                #endif
            }

            if (vWords[1] == "JOIN" && vWords[0].size() > 1)
            {
                // :username!username@50000007.F000000B.90000002.IP JOIN :#channelname
                strlcpy(pszName, vWords[0].c_str() + 1, sizeof(pszName));
                if (strchr(pszName, '!'))
                    *strchr(pszName, '!') = '\0';
                printf("IRC got join\n");

                #ifdef DEBUG_NET && DEBUG_IRC
                std::cout<<__FUNCTION__<<" :4 "<<pszName<<std::endl ;
                #endif
            }

            if (pszName[0] == 'u')
            {
                #ifdef DEBUG_NET && DEBUG_IRC
                std::cout<<__FUNCTION__<<" :5 "<<pszName<<std::endl ;
                #endif
                CAddress addr;
                if (DecodeAddress(pszName, addr))
                {
                    #ifdef DEBUG_NET && DEBUG_IRC
                    std::cout<<__FUNCTION__<<" :6 "<<pszName<<std::endl ;
                    #endif
                    addr.nTime = GetAdjustedTime();
                    if (AddAddress(addr, 51 * 60))
                        printf("IRC got new address: %s\n", addr.ToString().c_str());
                    nGotIRCAddresses++;
                }
                else
                {
                    printf("IRC decode failed\n");
                }
            }
        }
        closesocket(hSocket);






    fShutdown=true;

    }

    return 0; 
}









