#include <iostream>
#include <string.h>
#include <db_cxx.h>
#include <db.h>
#include <QtDebug>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "serialize.h"


using std::cout;
using std::endl;
using std::cerr;

using namespace boost;

DbEnv dbenv(0);
static bool fDbEnvInit = false;


Db* pdb;


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


	std::string pszFile="addr.dat"; 

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


/*
           // 3: create the key and value Dbts 
                char *first_key = "first_record";
                u_int32_t key_len = (u_int32_t) strlen (first_key);

                char * first_value = "Hello World - Berkeley DB style!!";
                u_int32_t value_len = (u_int32_t) strlen (first_value);

                Dbt key (first_key, key_len + 1);
                Dbt value (first_value, value_len + 1);

           // 4: insert the key-value pair into the database 
                ret = pdb->put (0, &key, &value, DB_NOOVERWRITE);

                if (ret == DB_KEYEXIST)
                {
                    cout << "hello_world: " << first_key << " already exists in db" <<endl;
                }

            // 5: read the value stored earlier in a Dbt object 
                Dbt stored_value;
                ret = pdb->get (0, &key, &stored_value, 0);

            //  6: print the value read from the database 
                cout << (char *) stored_value.get_data () << endl;

             // 7: close the database handle 
                pdb->close (0);

*/




        std::string key("version");
        static const int VERSION = 32500;
        bool fOverwrite=true;

	// Key
	CDataStream ssKey(SER_DISK);
	ssKey.reserve(1000);
	ssKey << key;
	Dbt datKey(&ssKey[0], ssKey.size());

	// Value
	CDataStream ssValue(SER_DISK);
	ssValue.reserve(10000);
	ssValue << VERSION;
	Dbt datValue(&ssValue[0], ssValue.size());

	// Write
//	ret = pdb->put(GetTxn(), &datKey, &datValue, (fOverwrite ? 0 : DB_NOOVERWRITE));
        ret = pdb->put(0, &datKey, &datValue, (fOverwrite ? 0 : DB_NOOVERWRITE));

        pdb->close (0);

/*
    CDataStream ssKey(SER_DISK);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());

    // Read
    Dbt datValue;
    datValue.set_flags(DB_DBT_MALLOC);
    int ret = pdb->get(GetTxn(), &datKey, &datValue, 0);
*/

    return ret; 
}
























