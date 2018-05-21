/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011
 */
#include "bitcoingui.h"
#include <QMenuBar>


BitcoinGUI::BitcoinGUI(QWidget *parent)
{
    resize(850, 550);
    setWindowTitle(tr("Bitcoin Wallet"));
    setWindowIcon(QIcon(":icons/bitcoin"));

    // Menus
    QMenu *file = menuBar()->addMenu("&File");

}



BitcoinGUI::~BitcoinGUI(void)
{

}


