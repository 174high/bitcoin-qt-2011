#ifndef BITCOINGUI_H
#define BITCOINGUI_H
#include <QMainWindow>
#include <QSystemTrayIcon>

class TransactionTableModel;
class ClientModel;
class WalletModel;
class TransactionView;
class OverviewPage;
class AddressBookPage;
class SendCoinsDialog;


QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QTableView;
class QAbstractItemModel;
class QModelIndex;
class QProgressBar;
class QStackedWidget;
QT_END_NAMESPACE


class BitcoinGUI : public QMainWindow
{
    Q_OBJECT
public:
    explicit BitcoinGUI(QWidget *parent = 0);
    ~BitcoinGUI();

//protected:
private:

    QStackedWidget *centralWidget;

    OverviewPage *overviewPage;
    QLabel *progressBarLabel;

    QAction *overviewAction;

    void createActions();

//public slots:
   private slots:
   void gotoOverviewPage();

};



#endif
 
