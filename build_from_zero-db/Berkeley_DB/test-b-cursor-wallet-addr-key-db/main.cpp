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
























