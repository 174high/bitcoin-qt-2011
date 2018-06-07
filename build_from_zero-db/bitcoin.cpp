/*
 * W.J. van der Laan 2011
 */
#include "bitcoingui.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "qtwin.h"

#include "headers.h"
#include "init.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <QApplication>
#include <QMessageBox>
#include <QThread>
#include <QLocale>
#include <QTranslator>
#include <QtDebug>
#include <utility>
#include <stdexcept>

using std::cout;
using std::endl;
using std::cerr;
using std::vector; 
using std::pair;

using std::map;
using std::make_pair;
using std::runtime_error;

using namespace boost;

map<uint256, CBlockIndex*> mapBlockIndex1; 

DbEnv dbenv1(0);
static bool fDbEnvInit = false;
Db* pdb;

// Need a global reference for the notifications to find the GUI
BitcoinGUI *guiref;

int MyMessageBox(const std::string& message, const std::string& caption, int style, wxWindow* parent, int x, int y)
{
    // Message from main thread
    if(guiref)
    {
        guiref->error(QString::fromStdString(caption),
                      QString::fromStdString(message));
    }
    else
    {
        QMessageBox::critical(0, QString::fromStdString(caption),
            QString::fromStdString(message),
            QMessageBox::Ok, QMessageBox::Ok);
    }
    return 4;
}

int ThreadSafeMessageBox(const std::string& message, const std::string& caption, int style, wxWindow* parent, int x, int y)
{
    // Message from network thread
    if(guiref)
    {
        QMetaObject::invokeMethod(guiref, "error", Qt::QueuedConnection,
                                   Q_ARG(QString, QString::fromStdString(caption)),
                                   Q_ARG(QString, QString::fromStdString(message)));
    }
    else
    {
        printf("%s: %s\n", caption.c_str(), message.c_str());
        fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
    }
    return 4;
}

bool ThreadSafeAskFee(int64 nFeeRequired, const std::string& strCaption, wxWindow* parent)
{
    if(!guiref)
        return false;
    if(nFeeRequired < MIN_TX_FEE || nFeeRequired <= nTransactionFee || fDaemon)
        return true;
    bool payFee = false;

    // Call slot on GUI thread.
    // If called from another thread, use a blocking QueuedConnection.
    Qt::ConnectionType connectionType = Qt::DirectConnection;
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
        connectionType = Qt::BlockingQueuedConnection;
    }

    QMetaObject::invokeMethod(guiref, "askFee", connectionType,
                               Q_ARG(qint64, nFeeRequired),
                               Q_ARG(bool*, &payFee));

    return payFee;
}

void CalledSetStatusBar(const std::string& strText, int nField)
{
    // Only used for built-in mining, which is disabled, simple ignore
}

void UIThreadCall(boost::function0<void> fn)
{
    // Only used for built-in mining, which is disabled, simple ignore
}

void MainFrameRepaint()
{
}

/*
   Translate string to current locale using Qt.
 */
std::string _(const char* psz)
{
    return QCoreApplication::translate("bitcoin-core", psz).toStdString();
}

CBlockIndex static * InsertBlockIndex(uint256 hash)
{
    if (hash == 0)
        return NULL;

    // Return existing
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex1.find(hash);
    if (mi != mapBlockIndex1.end())
        return (*mi).second;

    // Create new
    CBlockIndex* pindexNew = new CBlockIndex();
    if (!pindexNew)
        throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");
    mi = mapBlockIndex1.insert(make_pair(hash, pindexNew)).first;
    pindexNew->phashBlock = &((*mi).first);

    return pindexNew;
}

int main(int argc, char *argv[])
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


	printf("dbenv1.open strLogDir=%s strErrorFile=%s\n", strLogDir.c_str(), strErrorFile.c_str());

	//    dbenv.set_lg_dir(strLogDir.c_str());
	dbenv1.set_lg_max(10000000);
	dbenv1.set_lk_max_locks(10000);
	dbenv1.set_lk_max_objects(10000);
	dbenv1.set_errfile(fopen(strErrorFile.c_str(), "a")); /// debug
	dbenv1.set_flags(DB_AUTO_COMMIT, 1);

	qDebug()<<__FUNCTION__<<"open:1"<<strDataDir.c_str() ;

	ret = dbenv1.open(strDataDir.c_str(),
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


	std::string pszFile="blkindex.dat"; 

	pdb = new Db(&dbenv1, 0);

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

	Dbc* cursor;

        pdb->cursor(NULL,&cursor,0);

        cout<<"open cursor"<<endl;
 
        unsigned int fFlags = DB_SET_RANGE;

        unsigned int test_num=13,seq=0 ;
 
        while(test_num>0)
        {
        test_num--; 

	std::string strType ;
	// Read next record
	CDataStream ssKey;

        if(fFlags == DB_SET_RANGE)                              // at /root/.db_test/blockindex.dat
        ssKey << make_pair(std::string("blockindex"), uint256(0));

        CDataStream ssValue;

	Dbt datKey;
	datKey.set_data(&ssKey[0]);
	datKey.set_size(ssKey.size());

	Dbt datValue;

	datValue.set_data(&ssValue[0]);
	datValue.set_size(ssValue.size());

	datKey.set_flags(DB_DBT_MALLOC);
	datValue.set_flags(DB_DBT_MALLOC);

	if((ret = cursor->get(&datKey,&datValue,fFlags)) != DB_NOTFOUND)
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

       fFlags = DB_NEXT;

        if(strType == "blockindex")
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

/*
            std::cout<<"seq="<<++seq<<std::endl; 
            // Construct block index object
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
*/



        }

        std::cout<<"ret"<<ret<<std::endl  ;

        }

       std::cout<<"#############################################################"<<std::endl; 

       map<uint256, CBlockIndex*>::iterator mi=mapBlockIndex1.begin();
        

       std::cout<<"test1 size="<<mapBlockIndex1.size()<<std::endl; 
   
       seq=0; 
/*
       while(mi!=mapBlockIndex1.end())
       {
           std::cout<<"seq="<<++seq<<std::endl;
            // Construct block index object
            std::cout<<"BlockHash:"<<(*mi).second->GetBlockHash().ToString()<<std::endl ;
            if((*mi).second->pprev!=NULL)
            std::cout<<"hashPrev :"<<(*mi).second->pprev->GetBlockHash().ToString()<<std::endl ;
            if((*mi).second->pnext!=NULL )
            std::cout<<"hashNext :"<<(*mi).second->pnext->GetBlockHash().ToString()<<std::endl;
            std::cout<<"nFile    :"<<(*mi).second->nFile<<std::endl;
            std::cout<<"nBlockPos:"<<(*mi).second->nBlockPos<<std::endl;
            std::cout<<"nHeight  :"<<(*mi).second->nHeight<<std::endl;
            std::cout<<"nVersion :"<<(*mi).second->nVersion<<std::endl;
            std::cout<<"hashMerkleRoot:"<<(*mi).second->hashMerkleRoot.ToString()<<std::endl;
            std::cout<<"nTime    :"<<(*mi).second->nTime<<std::endl;
            std::cout<<"nBits    :"<<(*mi).second->nBits<<std::endl;
            std::cout<<"nNonce   :"<<(*mi).second->nNonce<<std::endl;

           mi++; 
       }
*/
       std::cout<<"test2"<<std::endl; 

	if (cursor != NULL)
	{

	    cursor->close();

	}

   // Calculate bnChainWork
    vector<pair<int, CBlockIndex*> > vSortedByHeight;
    vSortedByHeight.reserve(mapBlockIndex1.size());
    BOOST_FOREACH(const PAIRTYPE(uint256, CBlockIndex*)& item, mapBlockIndex1)
    {
        CBlockIndex* pindex = item.second;
        vSortedByHeight.push_back(make_pair(pindex->nHeight, pindex));
    }

    std::cout<<"vSortedByHeight  size="<<vSortedByHeight.size()<<std::endl; 

    sort(vSortedByHeight.begin(), vSortedByHeight.end());

    BOOST_FOREACH(const PAIRTYPE(int, CBlockIndex*)& item, vSortedByHeight)
    {
       	    CBlockIndex* pindex = item.second;
	    // Construct block index object
	    std::cout<<"BlockHash:"<<pindex->GetBlockHash().ToString()<<std::endl ;
	    if(pindex->pprev!=NULL)
	    std::cout<<"hashPrev :"<<pindex->pprev->GetBlockHash().ToString()<<std::endl ;
	    if(pindex->pnext!=NULL )
	    std::cout<<"hashNext :"<<pindex->pnext->GetBlockHash().ToString()<<std::endl;
	    std::cout<<"nFile    :"<<pindex->nFile<<std::endl;
	    std::cout<<"nBlockPos:"<<pindex->nBlockPos<<std::endl;
	    std::cout<<"nHeight  :"<<pindex->nHeight<<std::endl;
	    std::cout<<"nVersion :"<<pindex->nVersion<<std::endl;
	    std::cout<<"hashMerkleRoot:"<<pindex->hashMerkleRoot.ToString()<<std::endl;
	    std::cout<<"nTime    :"<<pindex->nTime<<std::endl;
	    std::cout<<"nBits    :"<<pindex->nBits<<std::endl;
	    std::cout<<"nNonce   :"<<pindex->nNonce<<std::endl;

    }

    BOOST_FOREACH(const PAIRTYPE(int, CBlockIndex*)& item, vSortedByHeight)
    {
        CBlockIndex* pindex = item.second;
        if(pindex->pprev!=NULL)
        {    
            std::cout<<"pprev->bnChainWork="<<pindex->pprev->bnChainWork.ToString()<<std::endl;
            std::cout<<"pindex->GetBlockWork="<<pindex->GetBlockWork().ToString()<<std::endl; 
        }
//        std::cout<<"pprev->bnChainWork="<<pindex->pprev->bnChainWork.getuint256().ToString()<<std::endl; 
        pindex->bnChainWork = (pindex->pprev ? pindex->pprev->bnChainWork : 0) + pindex->GetBlockWork();
        
        if(pindex->pprev!=NULL)
        {

            std::cout<<"pindex->bnChainWork="<<pindex->bnChainWork.ToString()<<std::endl;
        }

    }







    return ret; 


/*###############################################################################################*/

    //return 0;
}
