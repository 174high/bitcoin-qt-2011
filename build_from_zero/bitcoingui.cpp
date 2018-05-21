/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011
 */
#include "bitcoingui.h"


BitcoinGUI::BitcoinGUI(QWidget *parent)
{
    resize(850, 550);
    setWindowTitle(tr("Bitcoin Wallet"));
    setWindowIcon(QIcon(":icons/bitcoin"));

}



BitcoinGUI::~BitcoinGUI(void)
{

}


