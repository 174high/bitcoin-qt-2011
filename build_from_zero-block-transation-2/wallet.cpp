#include "headers.h"
#include "db.h"
#include "cryptopp/sha.h"
#include "crypter.h"

using namespace std;

bool CWallet::AddKey(const CKey& key)
{
    if (!CCryptoKeyStore::AddKey(key))
        return false;
    if (!fFileBacked)
        return true;
    if (!IsCrypted())
        return CWalletDB(strWalletFile).WriteKey(key.GetPubKey(), key.GetPrivKey());
    return true;
}

bool CWallet::AddCryptedKey(const vector<unsigned char> &vchPubKey, const vector<unsigned char> &vchCryptedSecret)
{
    if (!CCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret))
        return false;
    if (!fFileBacked)
        return true;
    CRITICAL_BLOCK(cs_pwalletdbEncryption)
    {
        if (pwalletdbEncryption)
            return pwalletdbEncryption->WriteCryptedKey(vchPubKey, vchCryptedSecret);
        else
            return CWalletDB(strWalletFile).WriteCryptedKey(vchPubKey, vchCryptedSecret);
    }

}




