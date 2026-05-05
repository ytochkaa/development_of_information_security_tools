#include "crypto_manager.h"
#include <QDir>
#include <QDirIterator>
#include <iostream>

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

bool CryptoManager::encryptDirectory(const QString &dirPath, const QString &password) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        std::cout << "Директория не найдена: " << dirPath.toStdString() << std::endl;
        return false;
    }

    QDirIterator it(dirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool success = true;
    while (it.hasNext()) {
        QString filePath = it.next();
        if (!encryptor.isFileEncrypted(filePath)) {
            if (!encryptor.encryptFile(filePath, password)) {
                success = false;
            }
        }
    }
    return success;
}

bool CryptoManager::decryptDirectory(const QString &dirPath, const QString &password) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        std::cout << "Директория не найдена: " << dirPath.toStdString() << std::endl;
        return false;
    }

    QDirIterator it(dirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool success = true;
    while (it.hasNext()) {
        QString filePath = it.next();
        if (encryptor.isFileEncrypted(filePath)) {
            if (!decryptor.decryptFile(filePath, password)) {
                success = false;
            }
        }
    }
    return success;
}