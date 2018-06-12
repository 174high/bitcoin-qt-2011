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

DbEnv dbenv1(0);
static bool fDbEnvInit = false;
Db* pdb;

Dbc* cursor;

std::map<vector<unsigned char>, CAddress> mapAddresses1;
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


	printf("dbenv.open strLogDir=%s strErrorFile=%s\n", strLogDir.c_str(), strErrorFile.c_str());

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


	std::string pszFile="addr.dat"; 

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

        if(strType=="addr")
        {
              CAddress addr;
              ssValue >> addr;
              mapAddresses1.insert(make_pair(addr.GetKey(), addr));
        }

        std::cout<<"ret"<<ret<<std::endl  ;

        if(ret!=0)
        break; 

        }

	if (cursor != NULL)
	{
	    cursor->close();
	}


    return ret; 


/*###############################################################################################*/

    //return 0;
}
