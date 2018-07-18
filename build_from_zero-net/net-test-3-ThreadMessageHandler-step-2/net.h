// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_NET_H
#define BITCOIN_NET_H

#include <deque>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <openssl/rand.h>


#include <arpa/inet.h>　

#include <vector>

class CInv;
class CAddress;
class CAddrDB;
class CRequestTracker;
class CNode;
class CBlockIndex;
extern int nBestHeight;
extern int nConnectTimeout;
extern bool fAllowDNS;

inline unsigned int ReceiveBufferSize() { return 1000*GetArg("-maxreceivebuffer", 10*1000); }
inline unsigned int SendBufferSize() { return 1000*GetArg("-maxsendbuffer", 10*1000); }
inline unsigned short GetDefaultPort() { return fTestNet ? 18333 : 8333; } 
enum
{
    NODE_NETWORK = (1 << 0),
};


bool AddAddress(CAddress addr, int64 nTimePenalty=0, CAddrDB *pAddrDB=NULL);
bool ConnectSocket(const CAddress& addrConnect, SOCKET& hSocketRet, int nTimeout=nConnectTimeout);
bool Lookup(const char *pszName, std::vector<CAddress>& vaddr, int nServices, int nMaxSolutions, bool fAllowLookup = false, int portDefault = 0, bool fAllowPort = false);
bool Lookup(const char *pszName, CAddress& addr, int nServices, bool fAllowLookup = false, int portDefault = 0, bool fAllowPort = false);
void StartNode(void* parg);
CNode* ConnectNode(CAddress addrConnect, int64 nTimeout=0);
static const unsigned char pchIPv4[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };


//
// Message header
//  (4) message start
//  (12) command
//  (4) size
//  (4) checksum

extern char pchMessageStart[4];

class CMessageHeader
{
public:
    enum { COMMAND_SIZE=12 };
    char pchMessageStart[sizeof(::pchMessageStart)];
    char pchCommand[COMMAND_SIZE];
    unsigned int nMessageSize;
    unsigned int nChecksum;

    CMessageHeader()
    {
        memcpy(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart));
        memset(pchCommand, 0, sizeof(pchCommand));
        pchCommand[1] = 1;
        nMessageSize = -1;
        nChecksum = 0;
    }

    CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn)
    {
        memcpy(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart));
        strncpy(pchCommand, pszCommand, COMMAND_SIZE);
        nMessageSize = nMessageSizeIn;
        nChecksum = 0;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(FLATDATA(pchMessageStart));
        READWRITE(FLATDATA(pchCommand));
        READWRITE(nMessageSize);
        if (nVersion >= 209)
            READWRITE(nChecksum);
    )

    std::string GetCommand()
    {
        if (pchCommand[COMMAND_SIZE-1] == 0)
            return std::string(pchCommand, pchCommand + strlen(pchCommand));
        else
            return std::string(pchCommand, pchCommand + COMMAND_SIZE);
    }

    bool IsValid()
    {
        // Check start string
        if (memcmp(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart)) != 0)
            return false;

        // Check the command string for errors
        for (char* p1 = pchCommand; p1 < pchCommand + COMMAND_SIZE; p1++)
        {
            if (*p1 == 0)
            {
                // Must be all zeros after the first zero
                for (; p1 < pchCommand + COMMAND_SIZE; p1++)
                    if (*p1 != 0)
                        return false;
            }
            else if (*p1 < ' ' || *p1 > 0x7E)
                return false;
        }

        // Message size
        if (nMessageSize > MAX_SIZE)
        {
            printf("CMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand().c_str(), nMessageSize);
            return false;
        }

        return true;
    }
};




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

    friend inline bool operator==(const CAddress& a, const CAddress& b)
    {
        return (memcmp(a.pchReserved, b.pchReserved, sizeof(a.pchReserved)) == 0 &&
                a.ip   == b.ip &&
                a.port == b.port);
    }

    friend inline bool operator!=(const CAddress& a, const CAddress& b)
    {
        return (!(a == b));
    }

    friend inline bool operator<(const CAddress& a, const CAddress& b)
    {
        int ret = memcmp(a.pchReserved, b.pchReserved, sizeof(a.pchReserved));
        if (ret < 0)
            return true;
        else if (ret == 0)
        {
            if (ntohl(a.ip) < ntohl(b.ip))
                return true;
            else if (a.ip == b.ip)
                return ntohs(a.port) < ntohs(b.port);
        }
        return false;
    }


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



enum
{
    MSG_TX = 1,
    MSG_BLOCK,
};

static const char* ppszTypeName[] =
{
    "ERROR",
    "tx",
    "block",
};

class CInv
{
public:
    int type;
    uint256 hash;

    CInv()
    {
        type = 0;
        hash = 0;
    }

    CInv(int typeIn, const uint256& hashIn)
    {
        type = typeIn;
        hash = hashIn;
    }

    CInv(const std::string& strType, const uint256& hashIn)
    {
        int i;
        for (i = 1; i < ARRAYLEN(ppszTypeName); i++)
        {
            if (strType == ppszTypeName[i])
            {
                type = i;
                break;
            }
        }
        if (i == ARRAYLEN(ppszTypeName))
            throw std::out_of_range(strprintf("CInv::CInv(string, uint256) : unknown type '%s'", strType.c_str()));
        hash = hashIn;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(type);
        READWRITE(hash);
    )

    friend inline bool operator<(const CInv& a, const CInv& b)
    {
        return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
    }

    bool IsKnownType() const
    {
        return (type >= 1 && type < ARRAYLEN(ppszTypeName));
    }

    const char* GetCommand() const
    {
        if (!IsKnownType())
            throw std::out_of_range(strprintf("CInv::GetCommand() : type=%d unknown type", type));
        return ppszTypeName[type];
    }

    std::string ToString() const
    {
        return strprintf("%s %s", GetCommand(), hash.ToString().substr(0,20).c_str());
    }

    void print() const
    {
        printf("CInv(%s)\n", ToString().c_str());
    }
};

class CRequestTracker
{
public:
    void (*fn)(void*, CDataStream&);
    void* param1;

    explicit CRequestTracker(void (*fnIn)(void*, CDataStream&)=NULL, void* param1In=NULL)
    {
        fn = fnIn;
        param1 = param1In;
    }

    bool IsNull()
    {
        return fn == NULL;
    }
};


extern bool fClient;
extern bool fAllowDNS;
extern uint64 nLocalServices;
extern CAddress addrLocalHost;
extern CNode* pnodeLocalHost;
extern uint64 nLocalHostNonce;
extern boost::array<int, 10> vnThreadsRunning;
extern SOCKET hListenSocket;

extern std::vector<CNode*> vNodes;
extern CCriticalSection cs_vNodes;
extern std::map<std::vector<unsigned char>, CAddress> mapAddresses;
extern CCriticalSection cs_mapAddresses;
extern std::map<CInv, CDataStream> mapRelay;
extern std::deque<std::pair<int64, CInv> > vRelayExpiration;
extern CCriticalSection cs_mapRelay;
extern std::map<CInv, int64> mapAlreadyAskedFor;

// Settings
extern int fUseProxy;
extern CAddress addrProxy;

class CNode
{
public:
    // socket
    uint64 nServices;
    SOCKET hSocket;
    CDataStream vSend;
    CDataStream vRecv;
    CCriticalSection cs_vSend;
    CCriticalSection cs_vRecv;
    int64 nLastSend;
    int64 nLastRecv;
    int64 nLastSendEmpty;
    int64 nTimeConnected;
    unsigned int nHeaderStart;
    unsigned int nMessageStart;
    CAddress addr;
    int nVersion;
    std::string strSubVer;
    bool fClient;
    bool fInbound;
    bool fNetworkNode;
    bool fSuccessfullyConnected;
    bool fDisconnect;
protected:
    int nRefCount;
public:
    int64 nReleaseTime;
    std::map<uint256, CRequestTracker> mapRequests;
    CCriticalSection cs_mapRequests;
    uint256 hashContinue;
    CBlockIndex* pindexLastGetBlocksBegin;
    uint256 hashLastGetBlocksEnd;
    int nStartingHeight;

    // flood relay
    std::vector<CAddress> vAddrToSend;
    std::set<CAddress> setAddrKnown;
    bool fGetAddr;
    std::set<uint256> setKnown;

    // inventory based relay
    std::set<CInv> setInventoryKnown;
    std::vector<CInv> vInventoryToSend;
    CCriticalSection cs_inventory;
    std::multimap<int64, CInv> mapAskFor;

    // publish and subscription
    std::vector<char> vfSubscribe;


    CNode(SOCKET hSocketIn, CAddress addrIn, bool fInboundIn=false)
    {
        nServices = 0;
        hSocket = hSocketIn;
        vSend.SetType(SER_NETWORK);
        vSend.SetVersion(0);
        vRecv.SetType(SER_NETWORK);
        vRecv.SetVersion(0);
        // Version 0.2 obsoletes 20 Feb 2012
        if (GetTime() > 1329696000)
        {
            vSend.SetVersion(209);
            vRecv.SetVersion(209);
        }
        nLastSend = 0;
        nLastRecv = 0;
        nLastSendEmpty = GetTime();
        nTimeConnected = GetTime();
        nHeaderStart = -1;
        nMessageStart = -1;
        addr = addrIn;
        nVersion = 0;
        strSubVer = "";
        fClient = false; // set by version message
        fInbound = fInboundIn;
        fNetworkNode = false;
        fSuccessfullyConnected = false;
        fDisconnect = false;
        nRefCount = 0;
        nReleaseTime = 0;
        hashContinue = 0;
        pindexLastGetBlocksBegin = 0;
        hashLastGetBlocksEnd = 0;
        nStartingHeight = -1;
        fGetAddr = false;
        vfSubscribe.assign(256, false);

        // Be shy and don't send version until we hear
        if (!fInbound)
            PushVersion();
    }

    ~CNode()
    {
        if (hSocket != INVALID_SOCKET)
        {
            closesocket(hSocket);
            hSocket = INVALID_SOCKET;
        }
    }

private:
    CNode(const CNode&);
    void operator=(const CNode&);
public:

    int GetRefCount()
    {
        return std::max(nRefCount, 0) + (GetTime() < nReleaseTime ? 1 : 0);
    }

    CNode* AddRef(int64 nTimeout=0)
    {
        if (nTimeout != 0)
            nReleaseTime = std::max(nReleaseTime, GetTime() + nTimeout);
        else
            nRefCount++;
        return this;
    }

    void Release()
    {
        nRefCount--;
    }

  void BeginMessage(const char* pszCommand)
    {
        cs_vSend.Enter();
        if (nHeaderStart != -1)
            AbortMessage();
        nHeaderStart = vSend.size();
        vSend << CMessageHeader(pszCommand, 0);
        nMessageStart = vSend.size();
        if (fDebug)
            printf("%s ", DateTimeStrFormat("%x %H:%M:%S", GetTime()).c_str());
        printf("sending: %s ", pszCommand);
    }

    void AbortMessage()
    {
        if (nHeaderStart == -1)
            return;
        vSend.resize(nHeaderStart);
        nHeaderStart = -1;
        nMessageStart = -1;
        cs_vSend.Leave();
        printf("(aborted)\n");
    }

   void PushAddress(const CAddress& addr)
    {
        // Known checking here is only to save space from duplicates.
        // SendMessages will filter it again for knowns that were added
        // after addresses were pushed.
        if (addr.IsValid() && !setAddrKnown.count(addr))
            vAddrToSend.push_back(addr);
    }


   void PushVersion()
    {
        /// when NTP implemented, change to just nTime = GetAdjustedTime()
        int64 nTime = (fInbound ? GetAdjustedTime() : GetTime());
        CAddress addrYou = (fUseProxy ? CAddress("0.0.0.0") : addr);
        CAddress addrMe = (fUseProxy ? CAddress("0.0.0.0") : addrLocalHost);
        RAND_bytes((unsigned char*)&nLocalHostNonce, sizeof(nLocalHostNonce));
        PushMessage("version", VERSION, nLocalServices, nTime, addrYou, addrMe,
                    nLocalHostNonce, std::string(pszSubVer), nBestHeight);
    }


    void EndMessage()
    {
//        if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
//        {
//            printf("dropmessages DROPPING SEND MESSAGE\n");
//            AbortMessage();
//            return;
//        }

        if (nHeaderStart == -1)
            return;

        // Set the size
        unsigned int nSize = vSend.size() - nMessageStart;
        memcpy((char*)&vSend[nHeaderStart] + offsetof(CMessageHeader, nMessageSize), &nSize, sizeof(nSize));

        // Set the checksum
        if (vSend.GetVersion() >= 209)
        {
            uint256 hash = Hash(vSend.begin() + nMessageStart, vSend.end());
            unsigned int nChecksum = 0;
            memcpy(&nChecksum, &hash, sizeof(nChecksum));
            assert(nMessageStart - nHeaderStart >= offsetof(CMessageHeader, nChecksum) + sizeof(nChecksum));
            memcpy((char*)&vSend[nHeaderStart] + offsetof(CMessageHeader, nChecksum), &nChecksum, sizeof(nChecksum));
        }

        printf("(%d bytes) ", nSize);
        printf("\n");

        nHeaderStart = -1;
        nMessageStart = -1;
        cs_vSend.Leave();
    }


    void PushMessage(const char* pszCommand)
    {
        try
        {
            BeginMessage(pszCommand);
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1>
    void PushMessage(const char* pszCommand, const T1& a1)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2 << a3;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2 << a3 << a4;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2 << a3 << a4 << a5;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2 << a3 << a4 << a5 << a6;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9)
    {
        try
        {
            BeginMessage(pszCommand);
            vSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    void PushGetBlocks(CBlockIndex* pindexBegin, uint256 hashEnd);
    bool IsSubscribed(unsigned int nChannel);
    void Subscribe(unsigned int nChannel, unsigned int nHops=0);
    void CancelSubscribe(unsigned int nChannel);
    void CloseSocketDisconnect();
    void Cleanup();
};
























































































































































extern std::map<std::vector<unsigned char>, CAddress> mapAddresses;

#endif 

