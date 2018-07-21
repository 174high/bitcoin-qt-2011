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
private:
    bool SelectCoinsMinConf(int64 nTargetValue, int nConfMine, int nConfTheirs, std::set<std::pair<const CWalletTx*,unsigned int> >& setCoinsRet, int64& nValueRet) const;
    bool SelectCoins(int64 nTargetValue, std::set<std::pair<const CWalletTx*,unsigned int> >& setCoinsRet, int64& nValueRet) const;

    CWalletDB *pwalletdbEncryption;
    CCriticalSection cs_pwalletdbEncryption;

public:

    bool fFileBacked;
    std::string strWalletFile;

    std::set<int64> setKeyPool;
    CCriticalSection cs_setKeyPool;

    typedef std::map<unsigned int, CMasterKey> MasterKeyMap;
    MasterKeyMap mapMasterKeys;
    unsigned int nMasterKeyMaxID;

    CWallet()
    {
        qDebug()<<"CWallet()" ;
        fFileBacked = false;
        nMasterKeyMaxID = 0;
        pwalletdbEncryption = NULL;
    }
    CWallet(std::string strWalletFileIn)
    {
        qDebug()<<"CWallet(std::string strWalletFileIn)" ;
        strWalletFile = strWalletFileIn;
        fFileBacked = true;
        nMasterKeyMaxID = 0;
        pwalletdbEncryption = NULL;
    }

    mutable CCriticalSection cs_mapWallet;
    std::map<uint256, CWalletTx> mapWallet;
    std::vector<uint256> vWalletUpdated;

    std::map<uint256, int> mapRequestCount;
    mutable CCriticalSection cs_mapRequestCount;

    std::map<std::string, std::string> mapAddressBook;
    mutable CCriticalSection cs_mapAddressBook;

    std::vector<unsigned char> vchDefaultKey;

};

//
// A transaction with a bunch of additional info that only the owner cares
// about.  It includes any unrecorded transactions needed to link it back
// to the block chain.
//
//class CWalletTx : public CMerkleTx
class CWalletTx 
{
//public:


};

#endif 

