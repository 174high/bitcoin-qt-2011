#include <iostream>
#include <string.h>
#include <db_cxx.h>
#include <db.h>
#include <QtDebug>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "uint256.h"
#include "util.h"
#include "serialize.h"
#include "net.h"

using std::cout;
using std::endl;
using std::cerr;
using std::string ;
using std::vector; 

using namespace boost;

static bool Send(SOCKET hSocket, const char* pszSend)
{
    if (strstr(pszSend, "PONG") != pszSend)
        printf("IRC SENDING: %s\n", pszSend);
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
        std::cout<<"IRC="<<strLine;
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



int main (int argc, char *argv[])
{
    int nErrorWait = 10;
    int nRetryWait = 10;

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




    }

    return 0; 
}









