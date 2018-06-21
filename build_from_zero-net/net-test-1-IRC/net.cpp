#include "util.h"
#include "uint256.h"
#include "net.h"
#include <vector>
#include <arpa/inet.h>
using std::vector ; 
#include <string.h>   
#include "strlcpy.h"
#include <sys/socket.h>  
#include <netdb.h>
#include <fcntl.h> 


// Settings
int fUseProxy = false;
int nConnectTimeout = 5000;
CAddress addrProxy("127.0.0.1",9050);

bool ConnectSocket(const CAddress& addrConnect, SOCKET& hSocketRet, int nTimeout)
{
    hSocketRet = INVALID_SOCKET;

    SOCKET hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hSocket == INVALID_SOCKET)
        return false;
#ifdef BSD
    int set = 1;
    setsockopt(hSocket, SOL_SOCKET, SO_NOSIGPIPE, (void*)&set, sizeof(int));
#endif

    bool fProxy = (fUseProxy && addrConnect.IsRoutable());
    struct sockaddr_in sockaddr = (fProxy ? addrProxy.GetSockAddr() : addrConnect.GetSockAddr());

#ifdef __WXMSW__
    u_long fNonblock = 1;
    if (ioctlsocket(hSocket, FIONBIO, &fNonblock) == SOCKET_ERROR)
#else
    int fFlags = fcntl(hSocket, F_GETFL, 0);
    if (fcntl(hSocket, F_SETFL, fFlags | O_NONBLOCK) == -1)
#endif
    {
        closesocket(hSocket);
        return false;
    }


    if (connect(hSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR)
    {
        // WSAEINVAL is here because some legacy version of winsock uses it
        if (WSAGetLastError() == WSAEINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINVAL)
        {
            struct timeval timeout;
            timeout.tv_sec  = nTimeout / 1000;
            timeout.tv_usec = (nTimeout % 1000) * 1000;

            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(hSocket, &fdset);
           int nRet = select(hSocket + 1, NULL, &fdset, NULL, &timeout);
            if (nRet == 0)
            {
                printf("connection timeout\n");
                closesocket(hSocket);
                return false;
            }
            if (nRet == SOCKET_ERROR)
            {
                printf("select() for connection failed: %i\n",WSAGetLastError());
                closesocket(hSocket);
                return false;
            }
            socklen_t nRetSize = sizeof(nRet);
#ifdef __WXMSW__
            if (getsockopt(hSocket, SOL_SOCKET, SO_ERROR, (char*)(&nRet), &nRetSize) == SOCKET_ERROR)
#else
            if (getsockopt(hSocket, SOL_SOCKET, SO_ERROR, &nRet, &nRetSize) == SOCKET_ERROR)
#endif
            {
                printf("getsockopt() for connection failed: %i\n",WSAGetLastError());
                closesocket(hSocket);
                return false;
            }
            if (nRet != 0)
            {
                printf("connect() failed after select(): %s\n",strerror(nRet));
                closesocket(hSocket);
                return false;
            }
        }
#ifdef __WXMSW__
        else if (WSAGetLastError() != WSAEISCONN)
#else
        else
#endif
        {
            printf("connect() failed: %i\n",WSAGetLastError());
            closesocket(hSocket);
            return false;
        }
    }

    /*
    this isn't even strictly necessary
    CNode::ConnectNode immediately turns the socket back to non-blocking
    but we'll turn it back to blocking just in case
    */
#ifdef __WXMSW__
    fNonblock = 0;
    if (ioctlsocket(hSocket, FIONBIO, &fNonblock) == SOCKET_ERROR)
#else
    fFlags = fcntl(hSocket, F_GETFL, 0);
    if (fcntl(hSocket, F_SETFL, fFlags & !O_NONBLOCK) == SOCKET_ERROR)
#endif
    {
        closesocket(hSocket);
        return false;
    }

    if (fProxy)
    {
        printf("proxy connecting %s\n", addrConnect.ToString().c_str());
        char pszSocks4IP[] = "\4\1\0\0\0\0\0\0user";
        memcpy(pszSocks4IP + 2, &addrConnect.port, 2);
        memcpy(pszSocks4IP + 4, &addrConnect.ip, 4);
        char* pszSocks4 = pszSocks4IP;
        int nSize = sizeof(pszSocks4IP);

        int ret = send(hSocket, pszSocks4, nSize, MSG_NOSIGNAL);
        if (ret != nSize)
        {
            closesocket(hSocket);
            return std::cout<<"Error sending to proxy"<<std::endl;
        }

        char pchRet[8];
        if (recv(hSocket, pchRet, 8, 0) != 8)
        {
            closesocket(hSocket);
            return std::cout<<"Error reading proxy response"<<std::endl ;
        }
        if (pchRet[1] != 0x5a)
        {
            closesocket(hSocket);
            if (pchRet[1] != 0x5b)
                printf("ERROR: Proxy returned error %d\n", pchRet[1]);
            return false;
        }
        printf("proxy connected %s\n", addrConnect.ToString().c_str());
    }

    hSocketRet = hSocket;
    return true;
}




bool Lookup(const char *pszName, vector<CAddress>& vaddr, int nServices, int nMaxSolutions, bool fAllowLookup, int portDefault, bool fAllowPort)
{
    vaddr.clear();
    if (pszName[0] == 0)
        return false;
    int port = portDefault;
    char psz[256];
    char *pszHost = psz;
    strlcpy(psz, pszName, sizeof(psz));
    if (fAllowPort)
    {
        char* pszColon = strrchr(psz+1,':');
        char *pszPortEnd = NULL;
        int portParsed = pszColon ? strtoul(pszColon+1, &pszPortEnd, 10) : 0;
        if (pszColon && pszPortEnd && pszPortEnd[0] == 0)
        {
            if (psz[0] == '[' && pszColon[-1] == ']')
            {
                // Future: enable IPv6 colon-notation inside []
                pszHost = psz+1;
                pszColon[-1] = 0;
            }
            else
                pszColon[0] = 0;
            port = portParsed;
            if (port < 0 || port > USHRT_MAX)
                port = USHRT_MAX;
        }
    }

    unsigned int addrIP = inet_addr(pszHost);
    if (addrIP != INADDR_NONE)
    {
        // valid IP address passed
        vaddr.push_back(CAddress(addrIP, port, nServices));
        return true;
    }

    if (!fAllowLookup)
        return false;

    struct hostent* phostent = gethostbyname(pszHost);
    if (!phostent)
        return false;

    if (phostent->h_addrtype != AF_INET)
        return false;

    char** ppAddr = phostent->h_addr_list;
    while (*ppAddr != NULL && vaddr.size() != nMaxSolutions)
    {
        CAddress addr(((struct in_addr*)ppAddr[0])->s_addr, port, nServices);
        if (addr.IsValid())
            vaddr.push_back(addr);
        ppAddr++;
    }

    return (vaddr.size() > 0);
}



// portDefault is in host order
bool Lookup(const char *pszName, CAddress& addr, int nServices, bool fAllowLookup, int portDefault, bool fAllowPort)
{
    vector<CAddress> vaddr;
    bool fRet = Lookup(pszName, vaddr, nServices, 1, fAllowLookup, portDefault, fAllowPort);
    if (fRet)
        addr = vaddr[0];
    return fRet;
}

