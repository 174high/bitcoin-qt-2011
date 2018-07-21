// Copyright (c) 2009-2011 Satoshi Nakamoto & Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_WALLET_H
#define BITCOIN_WALLET_H

#include "bignum.h"
#include "key.h"
#include "script.h"
#include <QtDebug>
#include "keystore.h"

class CWallet : public CCryptoKeyStore
{
//private:


public:

    CWallet()
    {
    }
    CWallet(std::string strWalletFileIn)
    {
    }


    void SetBestChain()
    {
//        CWalletDB walletdb(strWalletFile);
//        walletdb.WriteBestBlock(loc);
    }

};


#endif 

