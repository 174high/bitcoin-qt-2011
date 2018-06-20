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


int main (int argc, char *argv[])
{
    int ret; 

    //CAddress addrConnect("216.155.130.130:6667"); // chat.freenode.net
    CAddress addrConnect("92.243.23.21", 6667); // irc.lfnet.org

    addrConnect.print();

    //struct hostent* phostent = gethostbyname("chat.freenode.net");
    CAddress addrIRC("irc.lfnet.org", 6667, true);

    addrIRC.print();

    if (addrIRC.IsValid())
    addrConnect = addrIRC;

    addrConnect.print();

    return ret; 
}









