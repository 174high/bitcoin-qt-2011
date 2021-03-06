#include "util.h"
#include "strlcpy.h"
#include "string"
#include <openssl/rand.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>


using std::string ;
using std::vector;

bool fTestNet = false;
bool fShutdown = false;

char pszSetDataDir[MAX_PATH] = "";

string GetDefaultDataDir()
{
    // Windows: C:\Documents and Settings\username\Application Data\Bitcoin
    // Mac: ~/Library/Application Support/Bitcoin
    // Unix: ~/.bitcoin
#ifdef __WXMSW__
    // Windows
    return MyGetSpecialFolderPath(CSIDL_APPDATA, true) + "\\Bitcoin";
#else
    char* pszHome = getenv("HOME");
    if (pszHome == NULL || strlen(pszHome) == 0)
        pszHome = (char*)"/";
    string strHome = pszHome;
    if (strHome[strHome.size()-1] != '/')
        strHome += '/';
#ifdef __WXMAC_OSX__
    // Mac
    strHome += "Library/Application Support/";
    filesystem::create_directory(strHome.c_str());
    return strHome + "Bitcoin";
#else
    // Unix
    return strHome + ".bitcoin";
#endif
#endif
}


void GetDataDir(char* pszDir)
{
    // pszDir must be at least MAX_PATH length.
    int nVariation;
    if (pszSetDataDir[0] != 0)
    {
        strlcpy(pszDir, pszSetDataDir, MAX_PATH);
        nVariation = 0;
    }
    else
    {
        // This can be called during exceptions by printf, so we cache the
        // value so we don't have to do memory allocations after that.
        static char pszCachedDir[MAX_PATH];
        if (pszCachedDir[0] == 0)
            strlcpy(pszCachedDir, GetDefaultDataDir().c_str(), sizeof(pszCachedDir));
        strlcpy(pszDir, pszCachedDir, MAX_PATH);
        nVariation = 1;
    }
    if (fTestNet)
    {
        char* p = pszDir + strlen(pszDir);
        if (p > pszDir && p[-1] != '/' && p[-1] != '\\')
            *p++ = '/';
        strcpy(p, "testnet");
        nVariation += 2;
    }
    static bool pfMkdir[4];
    if (!pfMkdir[nVariation])
    {
        pfMkdir[nVariation] = true;
        boost::filesystem::create_directory(pszDir);
    }
}



void ParseString(const string& str, char c, vector<string>& v)
{
    if (str.empty())
        return;
    string::size_type i1 = 0;
    string::size_type i2;
    loop
    {
        i2 = str.find(c, i1);
        if (i2 == str.npos)
        {
            v.push_back(str.substr(i1));
            return;
        }
        v.push_back(str.substr(i1, i2-i1));
        i1 = i2+1;
    }
}


string strprintf(const std::string &format, ...)
{
    char buffer[50000];
    char* p = buffer;
    int limit = sizeof(buffer);
    int ret;
    loop
    {
        va_list arg_ptr;
        va_start(arg_ptr, format);
        ret = _vsnprintf(p, limit, format.c_str(), arg_ptr);
        va_end(arg_ptr); 
        if (ret >= 0 && ret < limit)
            break;
        if (p != buffer)
            delete[] p;
        limit *= 2;
        p = new char[limit];
        if (p == NULL)
            throw std::bad_alloc();
    }
    string str(p, p+ret);
    if (p != buffer)
        delete[] p;
    return str;
}


uint64 GetRand(uint64 nMax)
{
    if (nMax == 0)
        return 0;

    // The range of the random source must be a multiple of the modulus
    // to give every possible output value an equal possibility
    uint64 nRange = (UINT64_MAX / nMax) * nMax;
    uint64 nRand = 0;
    do
        RAND_bytes((unsigned char*)&nRand, sizeof(nRand));
    while (nRand >= nRange);
    return (nRand % nMax);
}

int GetRandInt(int nMax)
{
    return GetRand(nMax);
}


//
// "Never go to sea with two chronometers; take one or three."
// Our three time sources are:
//  - System clock
//  - Median of other nodes's clocks
//  - The user (asking the user to fix the system clock if the first two disagree)
//
int64 GetTime()
{
    return time(NULL);
}

static int64 nTimeOffset = 0;

int64 GetAdjustedTime()
{
    return GetTime() + nTimeOffset;
}

string GetDataDir()
{
    char pszDir[MAX_PATH];
    GetDataDir(pszDir);
    return pszDir;
}

