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


   // keystore implementation
    bool AddKey(const CKey& key);
    bool LoadKey(const CKey& key) { return CCryptoKeyStore::AddKey(key); }
    bool AddCryptedKey(const std::vector<unsigned char> &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);
    bool LoadCryptedKey(const std::vector<unsigned char> &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret) { return CCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret); }

    bool Unlock(const std::string& strWalletPassphrase);
    bool ChangeWalletPassphrase(const std::string& strOldWalletPassphrase, const std::string& strNewWalletPassphrase);
    bool EncryptWallet(const std::string& strWalletPassphrase);

    bool AddToWallet(const CWalletTx& wtxIn);
    bool AddToWalletIfInvolvingMe(const CTransaction& tx, const CBlock* pblock, bool fUpdate = false);
    bool EraseFromWallet(uint256 hash);
    void WalletUpdateSpent(const CTransaction& prevout);
    int ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate = false);
    void ReacceptWalletTransactions();
    void ResendWalletTransactions();
    int64 GetBalance() const;
    int64 GetUnconfirmedBalance() const;
    bool CreateTransaction(const std::vector<std::pair<CScript, int64> >& vecSend, CWalletTx& wtxNew, CReserveKey& reservekey, int64& nFeeRet);
    bool CreateTransaction(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew, CReserveKey& reservekey, int64& nFeeRet);
    bool CommitTransaction(CWalletTx& wtxNew, CReserveKey& reservekey);
    bool BroadcastTransaction(CWalletTx& wtxNew);
    std::string SendMoney(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew, bool fAskFee=false);
    std::string SendMoneyToBitcoinAddress(std::string strAddress, int64 nValue, CWalletTx& wtxNew, bool fAskFee=false);

    bool TopUpKeyPool();
 //   void ReserveKeyFromKeyPool(int64& nIndex, CKeyPool& keypool);
    void KeepKey(int64 nIndex);
    void ReturnKey(int64 nIndex);
    std::vector<unsigned char> GetOrReuseKeyFromPool();
    int64 GetOldestKeyPoolTime();
   bool IsMine(const CTxIn& txin) const;
    int64 GetDebit(const CTxIn& txin) const;
    bool IsMine(const CTxOut& txout) const
    {
//        return ::IsMine(*this, txout.scriptPubKey);  snq cautious
   }

   int64 GetCredit(const CTxOut& txout) const
    {
        if (!MoneyRange(txout.nValue))
            throw std::runtime_error("CWallet::GetCredit() : value out of range");
        return (IsMine(txout) ? txout.nValue : 0);
    }

    bool IsFromMe(const CTransaction& tx) const
    {
//        return (GetDebit(tx) > 0);    snq cautions
    }

  void SetBestChain(const CBlockLocator& loc)
    {
        CWalletDB walletdb(strWalletFile);
        walletdb.WriteBestBlock(loc);
    }


    void UpdatedTransaction(const uint256 &hashTx)
    {
        CRITICAL_BLOCK(cs_mapWallet)
            vWalletUpdated.push_back(hashTx);
    }

};

//
// A transaction with a bunch of additional info that only the owner cares
// about.  It includes any unrecorded transactions needed to link it back
// to the block chain.
//
class CWalletTx : public CMerkleTx
{
public:
    const CWallet* pwallet;

    std::vector<CMerkleTx> vtxPrev;
    std::map<std::string, std::string> mapValue;
    std::vector<std::pair<std::string, std::string> > vOrderForm;
    unsigned int fTimeReceivedIsTxTime;
    unsigned int nTimeReceived;  // time received by this node
    char fFromMe;
    std::string strFromAccount;
    std::vector<char> vfSpent;

    // memory only
    mutable char fDebitCached;
    mutable char fCreditCached;
    mutable char fAvailableCreditCached;
    mutable char fChangeCached;
    mutable int64 nDebitCached;
    mutable int64 nCreditCached;
    mutable int64 nAvailableCreditCached;
    mutable int64 nChangeCached;

    // memory only UI hints
    mutable unsigned int nTimeDisplayed;
    mutable int nLinesDisplayed;
    mutable char fConfirmedDisplayed;

    CWalletTx()
    {
        Init(NULL);
    }

    CWalletTx(const CWallet* pwalletIn)
    {
        Init(pwalletIn);
    }

    CWalletTx(const CWallet* pwalletIn, const CMerkleTx& txIn) : CMerkleTx(txIn)
    {
        Init(pwalletIn);
    }

    CWalletTx(const CWallet* pwalletIn, const CTransaction& txIn) : CMerkleTx(txIn)
    {
        Init(pwalletIn);
    }

    void Init(const CWallet* pwalletIn)
    {
        pwallet = pwalletIn;
        vtxPrev.clear();
        mapValue.clear();
        vOrderForm.clear();
        fTimeReceivedIsTxTime = false;
        nTimeReceived = 0;
        fFromMe = false;
        strFromAccount.clear();
        vfSpent.clear();
        fDebitCached = false;
        fCreditCached = false;
        fAvailableCreditCached = false;
        fChangeCached = false;
        nDebitCached = 0;
        nCreditCached = 0;
        nAvailableCreditCached = 0;
        nChangeCached = 0;
        nTimeDisplayed = 0;
        nLinesDisplayed = 0;
        fConfirmedDisplayed = false;
    }
    IMPLEMENT_SERIALIZE
    (
        CWalletTx* pthis = const_cast<CWalletTx*>(this);
        if (fRead)
            pthis->Init(NULL);
        char fSpent = false;

        if (!fRead)
        {
            pthis->mapValue["fromaccount"] = pthis->strFromAccount;

            std::string str;
            BOOST_FOREACH(char f, vfSpent)
            {
                str += (f ? '1' : '0');
                if (f)
                    fSpent = true;
            }
            pthis->mapValue["spent"] = str;
        }

        nSerSize += SerReadWrite(s, *(CMerkleTx*)this, nType, nVersion,ser_action);
        READWRITE(vtxPrev);
        READWRITE(mapValue);
        READWRITE(vOrderForm);
        READWRITE(fTimeReceivedIsTxTime);
        READWRITE(nTimeReceived);
        READWRITE(fFromMe);
        READWRITE(fSpent);

        if (fRead)
        {
            pthis->strFromAccount = pthis->mapValue["fromaccount"];

            if (mapValue.count("spent"))
                BOOST_FOREACH(char c, pthis->mapValue["spent"])
                    pthis->vfSpent.push_back(c != '0');
            else
                pthis->vfSpent.assign(vout.size(), fSpent);
        }

        pthis->mapValue.erase("fromaccount");
        pthis->mapValue.erase("version");
        pthis->mapValue.erase("spent");
    )


};

#endif 

