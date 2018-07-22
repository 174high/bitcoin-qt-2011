#include <iostream>
#include <string.h>
#include <db_cxx.h>
//#include <db.h>
#include <QtDebug>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "headers.h"
#include "bignum.h"
#include "uint256.h"
#include "util.h"
#include "serialize.h"
#include "net.h"
#include "db.h"
#include "wallet.h"

using namespace std; 
using namespace boost;
//
// CDB
//

static CCriticalSection cs_db;
static bool fDbEnvInit = false;
DbEnv dbenv(0);
static map<string, int> mapFileUseCount;
static map<string, Db*> mapDb;


class CDBInit
{
public:
    CDBInit()
    {
    }
    ~CDBInit()
    {
        if (fDbEnvInit)
        {
            dbenv.close(0);
            fDbEnvInit = false;
        }
    }
}
instance_of_cdbinit;


CDB::CDB(const char* pszFile, const char* pszMode) : pdb(NULL)
{
    #ifdef DEBUG_WALLET 
    qDebug()<<__FUNCTION__<<"created base class !!!"<<pszFile;
    #endif

    int ret;
    if (pszFile == NULL)
        return;

    fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
    bool fCreate = strchr(pszMode, 'c');
    unsigned int nFlags = DB_THREAD;
    if (fCreate)
        nFlags |= DB_CREATE;

    CRITICAL_BLOCK(cs_db)
    {
        if (!fDbEnvInit)
        {
            if (fShutdown)
                return;
            string strDataDir = GetDataDir();
            string strLogDir = strDataDir + "/database";
            filesystem::create_directory(strLogDir.c_str());
            string strErrorFile = strDataDir + "/db.log";
            printf("dbenv.open strLogDir=%s strErrorFile=%s\n", strLogDir.c_str(), strErrorFile.c_str());

            dbenv.set_lg_dir(strLogDir.c_str());
            dbenv.set_lg_max(10000000);
            dbenv.set_lk_max_locks(10000);
            dbenv.set_lk_max_objects(10000);
            dbenv.set_errfile(fopen(strErrorFile.c_str(), "a")); /// debug
            dbenv.set_flags(DB_AUTO_COMMIT, 1);

            #ifdef DEBUG_WALLET 
            qDebug()<<__FUNCTION__<<"open:1"<<strDataDir.c_str() ;
            #endif
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
                throw runtime_error(strprintf("CDB() : error %d opening database environment", ret));
            fDbEnvInit = true;
        }

        strFile = pszFile;
        ++mapFileUseCount[strFile];
        pdb = mapDb[strFile];
        if (pdb == NULL)
        {
            pdb = new Db(&dbenv, 0);

           #ifdef DEBUG_WALLET 
            qDebug()<<__FUNCTION__<<"open:2 " <<pszFile ;
            #endif

            ret = pdb->open(NULL,      // Txn pointer
                            pszFile,   // Filename
                            "main",    // Logical db name
                            DB_BTREE,  // Database type
                            nFlags,    // Flags
                            0);

            if (ret > 0)
            {
                delete pdb;
                pdb = NULL;
                CRITICAL_BLOCK(cs_db)
                    --mapFileUseCount[strFile];
                strFile = "";
                throw runtime_error(strprintf("CDB() : can't open database file %s, error %d", pszFile, ret));
            }

            if (fCreate && !Exists(string("version")))
            {
                bool fTmp = fReadOnly;
                fReadOnly = false;
  //              WriteVersion(VERSION);
                fReadOnly = fTmp;
            }

            mapDb[strFile] = pdb;
        }
    }
}

void CDB::Close()
{
    if (!pdb)
        return;
//    if (!vTxn.empty())
//        vTxn.front()->abort();
//    vTxn.clear();
    pdb = NULL;

    // Flush database activity from memory pool to disk log
    unsigned int nMinutes = 0;
    if (fReadOnly)
        nMinutes = 1;
    if (strFile == "addr.dat")
        nMinutes = 2;
//    if (strFile == "blkindex.dat" && IsInitialBlockDownload() && nBestHeight % 500 != 0)
//        nMinutes = 1;
    dbenv.txn_checkpoint(0, nMinutes, 0);

    CRITICAL_BLOCK(cs_db)
        --mapFileUseCount[strFile];
}


CBlockIndex static * InsertBlockIndex(uint256 hash)
{
    if (hash == 0)
        return NULL;

    // Return existing
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
    if (mi != mapBlockIndex.end())
        return (*mi).second;

    // Create new
    CBlockIndex* pindexNew = new CBlockIndex();
    if (!pindexNew)
        throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");
    mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
    pindexNew->phashBlock = &((*mi).first);

    return pindexNew;
}




bool CTxDB::LoadBlockIndex()
{
#ifdef DEBUG_WALLET
    qDebug()<<__FUNCTION__ ; 
#endif 

    // Get database cursor
    Dbc* pcursor = GetCursor();
    if (!pcursor)
        return false;

    // Load mapBlockIndex
    unsigned int fFlags = DB_SET_RANGE;
    loop
    {
        // Read next record
        CDataStream ssKey;
        if (fFlags == DB_SET_RANGE)
            ssKey << make_pair(string("blockindex"), uint256(0));
        CDataStream ssValue;
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0)
            return false;

        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType == "blockindex")
        {
            CDiskBlockIndex diskindex;
            ssValue >> diskindex;

            // Construct block index object
            CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
            pindexNew->pprev          = InsertBlockIndex(diskindex.hashPrev);
            pindexNew->pnext          = InsertBlockIndex(diskindex.hashNext);
            pindexNew->nFile          = diskindex.nFile;
            pindexNew->nBlockPos      = diskindex.nBlockPos;
            pindexNew->nHeight        = diskindex.nHeight;
            pindexNew->nVersion       = diskindex.nVersion;
            pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
            pindexNew->nTime          = diskindex.nTime;
            pindexNew->nBits          = diskindex.nBits;
            pindexNew->nNonce         = diskindex.nNonce;


            #ifdef DEBUG_DB
            std::cout<<"BlockHash:"<<diskindex.GetBlockHash().ToString()<<std::endl ;
            std::cout<<"hashPrev :"<<diskindex.hashPrev.ToString()<<std::endl ;
            std::cout<<"hashNext :"<<diskindex.hashNext.ToString()<<std::endl;
            std::cout<<"nFile    :"<<diskindex.nFile<<std::endl;
            std::cout<<"nBlockPos:"<<diskindex.nBlockPos<<std::endl;
            std::cout<<"nHeight  :"<<diskindex.nHeight<<std::endl;
            std::cout<<"nVersion :"<<diskindex.nVersion<<std::endl;
            std::cout<<"hashMerkleRoot:"<<diskindex.hashMerkleRoot.ToString()<<std::endl;
            std::cout<<"nTime    :"<<diskindex.nTime<<std::endl;
            std::cout<<"nBits    :"<<diskindex.nBits<<std::endl;
            std::cout<<"nNonce   :"<<diskindex.nNonce<<std::endl;
	    #endif 
	
            // Watch for genesis block
            if (pindexGenesisBlock == NULL && diskindex.GetBlockHash() == hashGenesisBlock)
            {
                qDebug()<<"!!! the pindexGenesisBlock here and pindexGenesisBlock=" ;
                pindexGenesisBlock = pindexNew;
            }
 

          //  if (!pindexNew->CheckIndex())
          //      return error("LoadBlockIndex() : CheckIndex failed at %d", pindexNew->nHeight);
        }
        else
        {
            break;
        }
    }
    pcursor->close();


}

bool CTxDB::WriteBlockIndex(const CDiskBlockIndex& blockindex)
{
    return Write(make_pair(string("blockindex"), blockindex.GetBlockHash()), blockindex);
}


//  
// CAddrDB
//  

bool CAddrDB::WriteAddress(const CAddress& addr)
{   
    return Write(make_pair(string("addr"), addr.GetKey()), addr);
}


bool CAddrDB::LoadAddresses()
{

    #ifdef DEBUG_DB
    qDebug()<<__FUNCTION__<<" 1:";
    #endif

//    CRITICAL_BLOCK(cs_mapAddresses)
    {
        // Load user provided addresses
        CAutoFile filein = fopen((GetDataDir() + "/addr.txt").c_str(), "rt");

        #ifdef DEBUG_DB
        qDebug()<<__FUNCTION__<<(GetDataDir() + "/addr.txt").c_str() ;
        #endif

        if (filein)
        {

        #ifdef DEBUG_DB
        qDebug()<<__FUNCTION__<<" 2:";
        #endif

            try
            {
                char psz[1000];
                while (fgets(psz, sizeof(psz), filein))
                {
                    CAddress addr(psz, NODE_NETWORK);
                    addr.nTime = 0; // so it won't relay unless successfully connected
                    if (addr.IsValid())
                      ;//  AddAddress(addr);
                }
            }
            catch (...) { }
        }

        // Get cursor
        Dbc* pcursor = GetCursor();
        if (!pcursor)
            return false;

        loop
        {
            // Read next record
            CDataStream ssKey;
            CDataStream ssValue;
            int ret = ReadAtCursor(pcursor, ssKey, ssValue);
            if (ret == DB_NOTFOUND)
                break;
            else if (ret != 0)
                return false;

            // Unserialize
            string strType;
            ssKey >> strType;
            #ifdef DEBUG_DB
            qDebug()<<__FUNCTION__<<" Read next record "<<strType.c_str();
            #endif
            if (strType == "addr")
            {
                CAddress addr;
                ssValue >> addr;
                mapAddresses.insert(make_pair(addr.GetKey(), addr));
           	#ifdef DEBUG_DB
                addr.print();
 	  	#endif 
            }
        }
        pcursor->close();

        printf("Loaded %d addresses\n", mapAddresses.size());
    }

    return true;
}

bool LoadAddresses()
{
    #ifdef DEBUG_WALLET
    qDebug()<<__FUNCTION__<<" a function here";
    #endif
    return CAddrDB("cr+").LoadAddresses();
}

