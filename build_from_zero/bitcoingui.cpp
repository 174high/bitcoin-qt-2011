/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011
 */
#include "bitcoingui.h"
#include <QMenuBar>
#include <QLabel>
#include <QStackedWidget>
#include <QToolBar>

#include "overviewpage.h"



BitcoinGUI::BitcoinGUI(QWidget *parent)
{
    resize(850, 550);
    setWindowTitle(tr("Bitcoin Wallet"));
    setWindowIcon(QIcon(":icons/bitcoin"));

    createActions();
 
   // Menus
    QMenu *file = menuBar()->addMenu("&File");

    QMenu *settings = menuBar()->addMenu("&Settings");

    QMenu *help = menuBar()->addMenu("&Help");

    QToolBar *toolbar = addToolBar("Main toolbar");
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(overviewAction);

    QToolBar *toolbar2 = addToolBar("Transactions toolbar");
    toolbar2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
 //   toolbar2->addAction(exportAction);

      // Overview page
    overviewPage = new OverviewPage();

//    progressBarLabel = new QLabel(tr("Synchronizing with network..."));    

    centralWidget = new QStackedWidget(this);

    centralWidget->addWidget(overviewPage);
    setCentralWidget(centralWidget);


    gotoOverviewPage();    
}



BitcoinGUI::~BitcoinGUI(void)
{

}

void BitcoinGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    overviewAction = new QAction(QIcon(":/icons/overview"), tr("&Overview"), this);
    overviewAction->setCheckable(true);
    tabGroup->addAction(overviewAction);

//    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
//    exportAction->setToolTip(tr("Export data in current view to a file")); 


   connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));



}



void BitcoinGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);

    //exportAction->setEnabled(false);
    //disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}


