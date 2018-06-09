#include <iostream>
#include <string.h>
#include <db_cxx.h>
#include <db.h>
#include <QtDebug>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "serialize.h"
#include "uint256.h"
#include "key.h"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include "sha.h"
#include "base58.h"

using std::cout;
using std::endl;
using std::cerr;

using namespace boost;

std::map<uint160, std::vector<unsigned char> > mapPubKeys;

bool fTestNet = false;

DbEnv dbenv(0);
static bool fDbEnvInit = false;
Db* pdb;
Dbc* cursor;


std::vector<unsigned char> vchDefaultKey;
std::set<int64> setKeyPool;


typedef std::map<std::vector<unsigned char>, CPrivKey> KeyMap;
KeyMap mapKeys;
/*
#define UEND(a)             ((unsigned char*)&((&(a))[1]))
#define UBEGIN(a)           ((unsigned char*)&(a))
bool fTestNet = false;

#define ADDRESSVERSION   ((unsigned char)(fTestNet ? 111 : 0))


static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";


inline std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
    CAutoBN_CTX pctx;
    CBigNum bn58 = 58;
    CBigNum bn0 = 0;

    // Convert big endian data to little endian
    // Extra zero at the end make sure bignum will interpret as a positive number
    std::vector<unsigned char> vchTmp(pend-pbegin+1, 0);
    reverse_copy(pbegin, pend, vchTmp.begin());

    // Convert little endian data to bignum
    CBigNum bn;
    bn.setvch(vchTmp);

    // Convert bignum to std::string
    std::string str;
    // Expected size increase from base58 conversion is approximately 137%
    // use 138% to be safe
    str.reserve((pend - pbegin) * 138 / 100 + 1);
    CBigNum dv;
    CBigNum rem;
    while (bn > bn0)
    {
        if (!BN_div(&dv, &rem, &bn, &bn58, pctx))
            throw bignum_error("EncodeBase58 : BN_div failed");
        bn = dv;
        unsigned int c = rem.getulong();
        str += pszBase58[c];
    }

    // Leading zeroes encoded as base58 zeros
    for (const unsigned char* p = pbegin; p < pend && *p == 0; p++)
        str += pszBase58[0];

   // Convert little endian std::string to big endian
    reverse(str.begin(), str.end());
    return str;
}




template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1];
    uint256 hash1;
    SHA256((pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]), (unsigned char*)&hash1);
    uint256 hash2;
    SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
} 



inline std::string EncodeBase58(const std::vector<unsigned char>& vch)
{
    return EncodeBase58(&vch[0], &vch[0] + vch.size());
}

inline std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn)
{
    // add 4-byte hash check to the end
    std::vector<unsigned char> vch(vchIn);
    uint256 hash = Hash(vch.begin(), vch.end());
    vch.insert(vch.end(), (unsigned char*)&hash, (unsigned char*)&hash + 4);
    return EncodeBase58(vch);
}


inline uint160 Hash160(const std::vector<unsigned char>& vch)
{   
    uint256 hash1;
    SHA256(&vch[0], vch.size(), (unsigned char*)&hash1);
    uint160 hash2;
    RIPEMD160((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}  

inline std::string Hash160ToAddress(uint160 hash160)
{
    // add 1-byte version number to the front
    std::vector<unsigned char> vch(1, ADDRESSVERSION);
    vch.insert(vch.end(), UBEGIN(hash160), UEND(hash160));
    return EncodeBase58Check(vch);
}  

inline std::string PubKeyToAddress(const std::vector<unsigned char>& vchPubKey)
{
    return Hash160ToAddress(Hash160(vchPubKey));
}
*/

int main (int argc, char *argv[])
{

	int ret; 
	unsigned int nFlags = DB_THREAD;
	nFlags |= DB_CREATE;

	if(!fDbEnvInit)
	{

	std::string strDataDir="/root/.db_test" ;

	filesystem::create_directory(strDataDir.c_str());

	std::string strLogDir = strDataDir + "/database" ;
	//    filesystem::create_directory(strLogDir.c_str());
	std::string strErrorFile = strDataDir + "/db.log";


	printf("dbenv.open strLogDir=%s strErrorFile=%s\n", strLogDir.c_str(), strErrorFile.c_str());

	//    dbenv.set_lg_dir(strLogDir.c_str());
	dbenv.set_lg_max(10000000);
	dbenv.set_lk_max_locks(10000);
	dbenv.set_lk_max_objects(10000);
	dbenv.set_errfile(fopen(strErrorFile.c_str(), "a")); /// debug
	dbenv.set_flags(DB_AUTO_COMMIT, 1);

	qDebug()<<__FUNCTION__<<"open:1"<<strDataDir.c_str() ;

	ret = dbenv.open(strDataDir.c_str(),
		     DB_CREATE     |
		     DB_INIT_LOCK  |
		     DB_INIT_LOG   |
		     DB_INIT_MPOOL |
		     DB_INIT_TXN   |
		     DB_THREAD     |
		     DB_RECOVER,
		     S_IRUSR | S_IWUSR);

	if (ret > 0)
	printf("error while creat enviromrnt !!!");
	//    throw runtime_error(strprintf("CDB() : error %d opening database environment", ret));
	fDbEnvInit = true;
	}


	std::string pszFile="wallet.dat"; 

	pdb = new Db(&dbenv, 0);

	qDebug()<<__FUNCTION__<<"open:2 " <<pszFile.c_str() ;

	ret = pdb->open(NULL,      // Txn pointer
		    pszFile.c_str(),   // Filename
		    "main",    // Logical db name
		    DB_BTREE,  // Database type
		    nFlags,    // Flags
		    0);

	if (ret > 0)
	{

	}

        pdb->cursor(NULL,&cursor,0);

        while(1)
        {

	std::string strType ;
	// Read next record
	CDataStream ssKey;
	CDataStream ssValue;

	Dbt datKey;

	Dbt datValue;

	datKey.set_flags(DB_DBT_MALLOC);
	datValue.set_flags(DB_DBT_MALLOC);

	if((ret = cursor->get(&datKey,&datValue,DB_NEXT)) != DB_NOTFOUND)
	{
	     // Convert to streams
	     ssKey.SetType(SER_DISK);
	     ssKey.clear();
	     ssKey.write((char*)datKey.get_data(), datKey.get_size());
	     ssValue.SetType(SER_DISK);
	     ssValue.clear();
	     ssValue.write((char*)datValue.get_data(), datValue.get_size()) ;

	     ssKey >> strType;

	     std::cout<<" get type ="<<strType<<endl  ;

	}

        if(strType=="key")
        {
            std::vector<unsigned char> vchPubKey;
            ssKey >> vchPubKey;
	    std::vector<unsigned char>::iterator it;
            for(it=vchPubKey.begin();it!=vchPubKey.end();it++)
                std::cout<<static_cast<unsigned>(*it);
            
            std::cout<<""<<std::endl; 

            CKey key;

            if (strType == "key")
            {
                CPrivKey pkey;
                ssValue >> pkey;
                key.SetPrivKey(pkey);
            }

            mapKeys[key.GetPubKey()] = key.GetPrivKey();
            mapPubKeys[Hash160(key.GetPubKey())] = key.GetPubKey(); 
            

        }
        else if (strType == "name")
        {
            std::string strAddress;
            ssKey >> strAddress;
            std::cout<<"name="<<strAddress<<std::endl; 
        }
        else if(strType == "defaultkey")
        {
            ssValue >> vchDefaultKey;
            std::vector<unsigned char>::iterator it;
            for(it=vchDefaultKey.begin();it!=vchDefaultKey.end();it++)
                std::cout<<static_cast<unsigned>(*it);
            std::cout<<""<<std::endl; 
        }
        else if (strType == "pool")
        {
             int64 nIndex;
             ssKey >> nIndex;
             setKeyPool.insert(nIndex);
        }



        std::cout<<"ret"<<ret<<std::endl  ;

        if(ret!=0)
        break; 

        }

	if (cursor != NULL)
	{
	    cursor->close();
	}

        if(mapKeys.count(vchDefaultKey) > 0)
        {
            std::cout<<"addr="<<PubKeyToAddress(vchDefaultKey)<<std::endl; 
        }



    return ret; 
}
























