#include "crypto_manager.h"
#include "crypto_constants.h"
#include "password_key_derivation.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QSaveFile>
#include <iostream>
#include <cstring>

CryptoManager::CryptoManager() {}

CryptoManager& CryptoManager::instance() {
    static CryptoManager instance;
    return instance;
}

bool CryptoManager::encrypt(const QString &filePath, const QString &password) {
    if (!isValidFileForEncryption(filePath)) {
        return false;
    }
    return processFile(filePath, password, Operation::Encrypt);
}

bool CryptoManager::decrypt(const QString &filePath, const QString &password) {
    if (!isValidFileForDecryption(filePath)) {
        return false;
    }
    return processFile(filePath, password, Operation::Decrypt);
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
        if (isValidFileForEncryption(filePath) && !isFileEncrypted(filePath)) {
            if (!processFile(filePath, password, Operation::Encrypt)) {
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
        if (isValidFileForDecryption(filePath) && isFileEncrypted(filePath)) {
            if (!processFile(filePath, password, Operation::Decrypt)) {
                success = false;
            }
        }
    }
    return success;
}

bool CryptoManager::isFileEncrypted(const QString &filePath) {
    if (!QFile::exists(filePath)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    char magic[MAGIC_SIZE];
    if (file.read(magic, MAGIC_SIZE) != MAGIC_SIZE) {
        return false;
    }

    return std::memcmp(magic, MAGIC, MAGIC_SIZE) == 0;
}

bool CryptoManager::processFile(const QString &filePath, const QString &password, Operation operation) {
    if (operation == Operation::Encrypt) {
        return encryptFile(filePath, password);
    }

    return decryptFile(filePath, password);
}

bool CryptoManager::encryptFile(const QString &filePath, const QString &password) {
    std::cout << "\nНАЧАЛО ШИФРОВАНИЯ ===" << std::endl;

    if (!QFile::exists(filePath)) {
        std::cout << "ОШИБКА: Файл не найден: " << filePath.toStdString() << std::endl;
        return false;
    }

    if (isFileEncrypted(filePath)) {
        std::cout << "ОШИБКА: Файл уже зашифрован" << std::endl;
        return false;
    }

    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        std::cout << "Ошибка генерации salt!" << std::endl;
        return false;
    }

    QByteArray key = PasswordKeyDerivation::deriveKeyFromPassword(password, salt);
    if (key.isEmpty()) {
        std::cout << "Ошибка генерации ключа!" << std::endl;
        return false;
    }

    unsigned char nonce[NONCE_SIZE];
    if (RAND_bytes(nonce, NONCE_SIZE) != 1) {
        std::cout << "Ошибка генерации nonce!" << std::endl;
        return false;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cout << "Ошибка создания контекста EVP!" << std::endl;
        return false;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        std::cout << "Ошибка инициализации шифрования!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, reinterpret_cast<unsigned char*>(key.data()), nonce) != 1) {
        std::cout << "Ошибка установки ключа и nonce!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    QSaveFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        std::cout << "Ошибка создания временного файла для безопасной записи!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (outFile.write(MAGIC, MAGIC_SIZE) != MAGIC_SIZE ||
        outFile.write(reinterpret_cast<const char*>(&FORMAT_VERSION), FORMAT_VERSION_SIZE) != FORMAT_VERSION_SIZE ||
        outFile.write(reinterpret_cast<char*>(salt), SALT_SIZE) != SALT_SIZE ||
        outFile.write(reinterpret_cast<char*>(nonce), NONCE_SIZE) != NONCE_SIZE) {
        std::cout << "Ошибка записи заголовка зашифрованного файла!" << std::endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        std::cout << "Ошибка открытия файла для чтения!" << std::endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::cout << "Шифрование файла: " << filePath.toStdString() << std::endl;

    constexpr int BUFFER_SIZE = 4096;
    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE];
    int outLen = 0;
    qint64 totalBytes = 0;

    while (true) {
        qint64 bytesRead = inFile.read(reinterpret_cast<char*>(inBuffer), BUFFER_SIZE);
        if (bytesRead < 0) {
            std::cout << "Ошибка чтения исходного файла!" << std::endl;
            inFile.close();
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        if (bytesRead == 0) {
            break;
        }

        if (EVP_EncryptUpdate(ctx, outBuffer, &outLen, inBuffer, static_cast<int>(bytesRead)) != 1) {
            std::cout << "Ошибка при шифровании блока!" << std::endl;
            inFile.close();
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        if (outFile.write(reinterpret_cast<char*>(outBuffer), outLen) != outLen) {
            std::cout << "Ошибка записи зашифрованного блока!" << std::endl;
            inFile.close();
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        totalBytes += bytesRead;
    }

    unsigned char tag[TAG_SIZE];
    if (EVP_EncryptFinal_ex(ctx, outBuffer, &outLen) != 1) {
        std::cout << "Ошибка при финализации шифрования!" << std::endl;
        inFile.close();
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (outLen > 0 && outFile.write(reinterpret_cast<char*>(outBuffer), outLen) != outLen) {
        std::cout << "Ошибка записи финального блока шифрования!" << std::endl;
        inFile.close();
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        std::cout << "Ошибка получения тега аутентификации!" << std::endl;
        inFile.close();
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (outFile.write(reinterpret_cast<char*>(tag), TAG_SIZE) != TAG_SIZE) {
        std::cout << "Ошибка записи тега аутентификации!" << std::endl;
        inFile.close();
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    inFile.close();
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key.data(), key.size());

    std::cout << "Исходный размер: " << totalBytes << " байт" << std::endl;

    if (!outFile.commit()) {
        std::cout << "Ошибка при безопасной замене файла!" << std::endl;
        return false;
    }

    qint64 encryptedSize = QFileInfo(filePath).size();
    std::cout << "Зашифрованный размер: " << encryptedSize << " байт" << std::endl;
    std::cout << "=== AES-GCM ШИФРОВАНИЕ УСПЕШНО ЗАВЕРШЕНО ===\n" << std::endl;
    return true;
}

bool CryptoManager::decryptFile(const QString &filePath, const QString &password) {
    std::cout << "\n=== НАЧАЛО AES-GCM ДЕШИФРОВАНИЯ ===" << std::endl;

    if (!QFile::exists(filePath)) {
        std::cout << "Ошибка: Файл не найден - " << filePath.toStdString() << std::endl;
        return false;
    }

    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        std::cout << "Ошибка открытия файла для чтения!" << std::endl;
        return false;
    }

    char magic[MAGIC_SIZE];
    if (inFile.read(magic, MAGIC_SIZE) != MAGIC_SIZE) {
        std::cout << "Ошибка чтения сигнатуры!" << std::endl;
        inFile.close();
        return false;
    }

    if (std::memcmp(magic, MAGIC, MAGIC_SIZE) != 0) {
        std::cout << "Ошибка: файл не зашифрован этой программой!" << std::endl;
        inFile.close();
        return false;
    }

    unsigned char version;
    if (inFile.read(reinterpret_cast<char*>(&version), FORMAT_VERSION_SIZE) != FORMAT_VERSION_SIZE) {
        std::cout << "Ошибка чтения версии формата файла!" << std::endl;
        inFile.close();
        return false;
    }

    if (version != FORMAT_VERSION) {
        std::cout << "Ошибка: неподдерживаемая версия формата файла!" << std::endl;
        inFile.close();
        return false;
    }

    unsigned char salt[SALT_SIZE];
    if (inFile.read(reinterpret_cast<char*>(salt), sizeof(salt)) != sizeof(salt)) {
        std::cout << "Ошибка чтения salt из файла!" << std::endl;
        inFile.close();
        return false;
    }

    unsigned char nonce[NONCE_SIZE];
    if (inFile.read(reinterpret_cast<char*>(nonce), NONCE_SIZE) != NONCE_SIZE) {
        std::cout << "Ошибка чтения nonce из файла!" << std::endl;
        inFile.close();
        return false;
    }

    qint64 dataSize = inFile.size() - HEADER_SIZE - TAG_SIZE;
    if (dataSize <= 0) {
        std::cout << "Ошибка: Файл слишком мал или поврежден!" << std::endl;
        inFile.close();
        return false;
    }

    QByteArray key = PasswordKeyDerivation::deriveKeyFromPassword(password, salt);
    if (key.isEmpty()) {
        std::cout << "Ошибка генерации ключа!" << std::endl;
        inFile.close();
        return false;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cout << "Ошибка создания контекста дешифрования!" << std::endl;
        inFile.close();
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        std::cout << "Ошибка инициализации дешифрования!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, reinterpret_cast<unsigned char*>(key.data()), nonce) != 1) {
        std::cout << "Ошибка установки ключа и nonce!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    QSaveFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        std::cout << "Ошибка создания временного файла для безопасной записи!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    constexpr int BUFFER_SIZE = 4096;
    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE + TAG_SIZE];
    qint64 remaining = dataSize;
    int outLen = 0;

    while (remaining > 0) {
        int toRead = remaining > BUFFER_SIZE ? BUFFER_SIZE : static_cast<int>(remaining);
        qint64 bytesRead = inFile.read(reinterpret_cast<char*>(inBuffer), toRead);
        if (bytesRead <= 0) {
            std::cout << "Ошибка чтения зашифрованного блока!" << std::endl;
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            inFile.close();
            return false;
        }

        if (EVP_DecryptUpdate(ctx, outBuffer, &outLen, inBuffer, static_cast<int>(bytesRead)) != 1) {
            std::cout << "Ошибка при дешифровании блока!" << std::endl;
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            inFile.close();
            return false;
        }

        if (outFile.write(reinterpret_cast<char*>(outBuffer), outLen) != outLen) {
            std::cout << "Ошибка записи расшифрованного блока!" << std::endl;
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            inFile.close();
            return false;
        }

        remaining -= bytesRead;
    }

    unsigned char tag[TAG_SIZE];
    if (inFile.read(reinterpret_cast<char*>(tag), TAG_SIZE) != TAG_SIZE) {
        std::cout << "Ошибка чтения тега аутентификации!" << std::endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        std::cout << "Ошибка установки тега аутентификации!" << std::endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    int finalLen = 0;
    int result = EVP_DecryptFinal_ex(ctx, outBuffer, &finalLen);
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    OPENSSL_cleanse(key.data(), key.size());

    if (result != 1) {
        std::cout << "ОШИБКА: Неверный пароль или файл поврежден!" << std::endl;
        std::cout << "Тег аутентификации не совпадает." << std::endl;
        outFile.cancelWriting();
        return false;
    }

    if (finalLen > 0 && outFile.write(reinterpret_cast<char*>(outBuffer), finalLen) != finalLen) {
        std::cout << "Ошибка записи финального блока!" << std::endl;
        outFile.cancelWriting();
        return false;
    }

    if (!outFile.commit()) {
        std::cout << "Ошибка при безопасной замене файла!" << std::endl;
        return false;
    }

    std::cout << "ДЕШИФРОВАНИЕ ЗАВЕРШЕНО" << std::endl;
    return true;
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
    return true;
}
