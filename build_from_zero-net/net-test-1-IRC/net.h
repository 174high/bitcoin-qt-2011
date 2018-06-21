// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_NET_H
#define BITCOIN_NET_H

#include <arpa/inet.h>　

#include <vector>

class CAddress;
extern int nConnectTimeout;


inline unsigned short GetDefaultPort() { return fTestNet ? 18333 : 8333; } 
enum
{
    NODE_NETWORK = (1 << 0),
};


bool ConnectSocket(const CAddress& addrConnect, SOCKET& hSocketRet, int nTimeout=nConnectTimeout);
bool Lookup(const char *pszName, std::vector<CAddress>& vaddr, int nServices, int nMaxSolutions, bool fAllowLookup = false, int portDefault = 0, bool fAllowPort = false);
bool Lookup(const char *pszName, CAddress& addr, int nServices, bool fAllowLookup = false, int portDefault = 0, bool fAllowPort = false);


static const unsigned char pchIPv4[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };

class CAddress
{
public:
    uint64 nServices;
    unsigned char pchReserved[12];
    unsigned int ip;
    unsigned short port;

    // disk and network only
    unsigned int nTime;

    // memory only
    unsigned int nLastTry;

    CAddress()
    {
        std::cout<<"CAddress :0 "<<std::endl; 
        Init();
    }

    CAddress(unsigned int ipIn, unsigned short portIn=0, uint64 nServicesIn=NODE_NETWORK)
    {
        std::cout<<"CAddress :1 "<<"ipIn="<<ipIn<<"  portIn="<<portIn<<std::endl ;

        Init();
        ip = ipIn;
        port = htons(portIn == 0 ? GetDefaultPort() : portIn);
        nServices = nServicesIn;
    }

    explicit CAddress(const struct sockaddr_in& sockaddr, uint64 nServicesIn=NODE_NETWORK)
    {
        std::cout<<"CAddress :2"<<std::endl ;
        Init();
        ip = sockaddr.sin_addr.s_addr;
        port = sockaddr.sin_port;
        nServices = nServicesIn;
    }

    explicit CAddress(const char* pszIn, int portIn, bool fNameLookup = false, uint64 nServicesIn=NODE_NETWORK)
    {
        std::cout<<"CAddress :3 "<<" ip="<< pszIn<<" port= "<<portIn<<std::endl ;
        Init();
        Lookup(pszIn, *this, nServicesIn, fNameLookup, portIn);
    }

    explicit CAddress(const char* pszIn, bool fNameLookup = false, uint64 nServicesIn=NODE_NETWORK)
    {
        std::cout<<"CAddress :4"<<std::endl ;
        Init();
        Lookup(pszIn, *this, nServicesIn, fNameLookup, 0, true);
    }

    explicit CAddress(std::string strIn, int portIn, bool fNameLookup = false, uint64 nServicesIn=NODE_NETWORK)
    {
        std::cout<<"CAddress :5"<<std::endl ;
        Init();
        Lookup(strIn.c_str(), *this, nServicesIn, fNameLookup, portIn);
    }

    explicit CAddress(std::string strIn, bool fNameLookup = false, uint64 nServicesIn=NODE_NETWORK)
    {
        std::cout<<"CAddress :6"<<std::endl ;
        Init();
        Lookup(strIn.c_str(), *this, nServicesIn, fNameLookup, 0, true);
    }

    void Init()
    {
        nServices = NODE_NETWORK;
        memcpy(pchReserved, pchIPv4, sizeof(pchReserved));
        ip = INADDR_NONE;
        port = htons(GetDefaultPort());
        nTime = 100000000;
        nLastTry = 0;
    }

    IMPLEMENT_SERIALIZE
    (
        if (fRead)
            const_cast<CAddress*>(this)->Init();
        if (nType & SER_DISK)
            READWRITE(nVersion);
        if ((nType & SER_DISK) || (nVersion >= 31402 && !(nType & SER_GETHASH)))
            READWRITE(nTime);
        READWRITE(nServices);
        READWRITE(FLATDATA(pchReserved)); // for IPv6
        READWRITE(ip);
        READWRITE(port);
    )

    std::vector<unsigned char> GetKey() const
    {
        CDataStream ss;
        ss.reserve(18);
        ss << FLATDATA(pchReserved) << ip << port;

        #if defined(_MSC_VER) && _MSC_VER < 1300
        return std::vector<unsigned char>((unsigned char*)&ss.begin()[0], (unsigned char*)&ss.end()[0]);
        #else
        return std::vector<unsigned char>(ss.begin(), ss.end());
        #endif
    }

    struct sockaddr_in GetSockAddr() const
    {
        struct sockaddr_in sockaddr;
        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = ip;
        sockaddr.sin_port = port;
        return sockaddr;
    }


    bool IsIPv4() const
    {
        return (memcmp(pchReserved, pchIPv4, sizeof(pchIPv4)) == 0);
    }

    bool IsRFC1918() const
    {
      return IsIPv4() && (GetByte(3) == 10 ||
        (GetByte(3) == 192 && GetByte(2) == 168) ||
        (GetByte(3) == 172 &&
          (GetByte(2) >= 16 && GetByte(2) <= 31)));
    }

    bool IsRFC3927() const
    {
      return IsIPv4() && (GetByte(3) == 169 && GetByte(2) == 254);
    }

    bool IsLocal() const
    {
      return IsIPv4() && (GetByte(3) == 127 ||
          GetByte(3) == 0);
    }

    bool IsRoutable() const
    {
        return IsValid() &&
            !(IsRFC1918() || IsRFC3927() || IsLocal());
    }


   bool IsValid() const
    {
        // Clean up 3-byte shifted addresses caused by garbage in size field
        // of addr messages from versions before 0.2.9 checksum.
        // Two consecutive addr messages look like this:
        // header20 vectorlen3 addr26 addr26 addr26 header20 vectorlen3 addr26 addr26 addr26...
        // so if the first length field is garbled, it reads the second batch
        // of addr misaligned by 3 bytes.
        if (memcmp(pchReserved, pchIPv4+3, sizeof(pchIPv4)-3) == 0)
            return false;

        return (ip != 0 && ip != INADDR_NONE && port != htons(USHRT_MAX));
    }



    unsigned char GetByte(int n) const
    {
        return ((unsigned char*)&ip)[3-n];
    }


    std::string ToStringIPPort() const
    {
        return strprintf("%u.%u.%u.%u:%u", GetByte(3), GetByte(2), GetByte(1), GetByte(0), ntohs(port));
    }

    std::string ToStringIP() const
    {
        return strprintf("%u.%u.%u.%u", GetByte(3), GetByte(2), GetByte(1), GetByte(0));
    }

    std::string ToStringPort() const
    {
        return strprintf("%u", ntohs(port));
    }

    std::string ToString() const
    {
        return strprintf("%u.%u.%u.%u:%u", GetByte(3), GetByte(2), GetByte(1), GetByte(0), ntohs(port));
    }

    void print() const
    {
         std::cout<<"CAddress="<<ToString()<<std::endl ;
    }

};


extern CAddress addrLocalHost;


// Settings
extern int fUseProxy;

#endif 
