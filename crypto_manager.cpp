#include "crypto_manager.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <iostream>

CryptoManager::CryptoManager() {}

CryptoManager& CryptoManager::instance() {
    static CryptoManager instance;
    return instance;
}

bool CryptoManager::encrypt(const QString &filePath, const QString &password) {
    if (!isValidFileForEncryption(filePath)) {
        return false;
    }
    return encryptor.encryptFile(filePath, password);
}

bool CryptoManager::decrypt(const QString &filePath, const QString &password) {
    if (!isValidFileForDecryption(filePath)) {
        return false;
    }
    return decryptor.decryptFile(filePath, password);
}

bool CryptoManager::encryptDirectory(const QString &dirPath, const QString &password) {
    if (!isValidDirectory(dirPath)) {
        return false;
    }

    QDir dir(dirPath);
    QDirIterator it(dirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool success = true;
    while (it.hasNext()) {
        QString filePath = it.next();
        if (isValidFileForEncryption(filePath) && !encryptor.isFileEncrypted(filePath)) {
            if (!encryptor.encryptFile(filePath, password)) {
                success = false;
            }
        }
    }
    return success;
}

bool CryptoManager::decryptDirectory(const QString &dirPath, const QString &password) {
    if (!isValidDirectory(dirPath)) {
        return false;
    }

    QDir dir(dirPath);
    QDirIterator it(dirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool success = true;
    while (it.hasNext()) {
        QString filePath = it.next();
        if (isValidFileForDecryption(filePath) && encryptor.isFileEncrypted(filePath)) {
            if (!decryptor.decryptFile(filePath, password)) {
                success = false;
            }
        }
    }
    return success;
}

bool CryptoManager::isValidFileForEncryption(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        std::cout << "Файл не существует: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (!fileInfo.isFile()) {
        std::cout << "Это не файл: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (fileInfo.size() == 0) {
        std::cout << "Файл пустой: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (!fileInfo.isReadable()) {
        std::cout << "Нет прав на чтение файла: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (!fileInfo.isWritable()) {
        std::cout << "Нет прав на запись файла: " << filePath.toStdString() << std::endl;
        return false;
    }
    // Пропуск системных файлов (скрытые или с атрибутами)
    if (fileInfo.isHidden()) {
        std::cout << "Пропуск скрытого файла: " << filePath.toStdString() << std::endl;
        return false;
    }
    // Дополнительно: проверка на системные имена, но упростим
    return true;
}

bool CryptoManager::isValidFileForDecryption(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        std::cout << "Файл не существует: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (!fileInfo.isFile()) {
        std::cout << "Это не файл: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (fileInfo.size() == 0) {
        std::cout << "Файл пустой: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (!fileInfo.isReadable()) {
        std::cout << "Нет прав на чтение файла: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (!fileInfo.isWritable()) {
        std::cout << "Нет прав на запись файла: " << filePath.toStdString() << std::endl;
        return false;
    }
    if (fileInfo.isHidden()) {
        std::cout << "Пропуск скрытого файла: " << filePath.toStdString() << std::endl;
        return false;
    }
    return true;
}

bool CryptoManager::isValidDirectory(const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        std::cout << "Директория не существует: " << dirPath.toStdString() << std::endl;
        return false;
    }
    if (!dir.isReadable()) {
        std::cout << "Нет прав на чтение директории: " << dirPath.toStdString() << std::endl;
        return false;
    }
    // Для записи проверяем, можем ли создать временные файлы, но упростим
    return true;
}