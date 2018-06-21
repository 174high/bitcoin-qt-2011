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

using namespace boost;


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
/*
    if (!RecvUntil(hSocket, "Found your hostname", "using your IP address instead", "Couldn't look up your hostname", "ignoring hostname"))
    {
            closesocket(hSocket);
            hSocket = INVALID_SOCKET;
            nErrorWait = nErrorWait * 11 / 10;
            if (Wait(nErrorWait += 60))
                continue;
            else
                return;
    }


*/

    }

    return 0; 
}









