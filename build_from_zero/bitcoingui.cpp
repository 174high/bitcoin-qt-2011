/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011
 */
#include "bitcoingui.h"
#include <QMenuBar>
#include <QLabel>
#include <QStackedWidget>


#include "overviewpage.h"



BitcoinGUI::BitcoinGUI(QWidget *parent)
{
    resize(850, 550);
    setWindowTitle(tr("Bitcoin Wallet"));
    setWindowIcon(QIcon(":icons/bitcoin"));

    // Menus
    QMenu *file = menuBar()->addMenu("&File");

    QMenu *settings = menuBar()->addMenu("&Settings");

    QMenu *help = menuBar()->addMenu("&Help");

    QToolBar *toolbar = addToolBar("Main toolbar");

    QToolBar *toolbar2 = addToolBar("Transactions toolbar");
  
      // Overview page
    overviewPage = new OverviewPage();

//    progressBarLabel = new QLabel(tr("Synchronizing with network..."));    

    centralWidget = new QStackedWidget(this);

    centralWidget->addWidget(overviewPage);
    setCentralWidget(centralWidget);
    
}



BitcoinGUI::~BitcoinGUI(void)
{

}


