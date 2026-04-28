#include "crypto_manager.h"

CryptoManager::CryptoManager() {}

CryptoManager& CryptoManager::instance() {
    static CryptoManager instance;
    return instance;
}

bool CryptoManager::encrypt(const QString &filePath, const QString &password) {
    return encryptor.encryptFile(filePath, password);
}

bool CryptoManager::decrypt(const QString &filePath, const QString &password) {
    return decryptor.decryptFile(filePath, password);
}