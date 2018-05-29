#include <iostream>
#include <string.h>
#include <db_cxx.h>
#include <db.h>
#include <QtDebug>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

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





    return ret; 
}
























