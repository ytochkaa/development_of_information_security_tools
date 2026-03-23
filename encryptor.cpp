#include <iostream>
#include "encryptor.h"
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QCryptographicHash>
#include <QDebug>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace std;

Encryptor::Encryptor() {
    OpenSSL_add_all_algorithms();
}

QByteArray Encryptor::deriveKeyFromPassword(const QString &password) {
    unsigned char key[KEY_SIZE];
    QByteArray passwordBytes = password.toUtf8();
    
    QByteArray hash = QCryptographicHash::hash(
        passwordBytes, 
        QCryptographicHash::Sha256
    );
    memcpy(key, hash.constData(), KEY_SIZE);
    
    return QByteArray((char*)key, KEY_SIZE);
}

QByteArray Encryptor::calculateFileHash(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    const int BUFFER_SIZE = 8192;
    QByteArray buffer;
    
    while (!file.atEnd()) {
        buffer = file.read(BUFFER_SIZE);
        hash.addData(buffer);
    }
    
    file.close();
    return hash.result();
}

bool Encryptor::isFileEncrypted(const QString &filePath) {
    if (!QFile::exists(filePath)) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    qint64 fileSize = file.size();
    
    if (fileSize < HASH_SIZE + NONCE_SIZE + TAG_SIZE + 1) {
        file.close();
        return false;
    }
    
    QByteArray savedHash = file.read(HASH_SIZE);
    
    if (savedHash.size() != HASH_SIZE) {
        file.close();
        return false;
    }
    
    // Теперь читаем nonce, зашифрованные данные и тег
    QByteArray nonce = file.read(NONCE_SIZE);
    if (nonce.size() != NONCE_SIZE) {
        file.close();
        return false;
    }
    
    qint64 encryptedDataSize = fileSize - HASH_SIZE - NONCE_SIZE - TAG_SIZE;
    QByteArray encryptedData = file.read(encryptedDataSize);
    
    QByteArray tag = file.read(TAG_SIZE);
    if (tag.size() != TAG_SIZE) {
        file.close();
        return false;
    }
    
    file.close();
    return true;
}

bool Encryptor::encryptFile(const QString &filePath, const QString &password) {
    cout << "\nНАЧАЛО ШИФРОВАНИЯ ===" << endl;
    
    if (!QFile::exists(filePath)) {
        cout << "ОШИБКА: Файл не найден: " << filePath.toStdString() << endl;
        return false;
    }

    if (isFileEncrypted(filePath)) {
        cout << "ОШИБКА: Файл уже зашифрован! Повторное шифрование запрещено." << endl;
        return false;
    }
    cout << "Вычисление хэша оригинального файла..." << endl;
    QByteArray originalHash = calculateFileHash(filePath);
    
    if (originalHash.isEmpty()) {
        cout << "ОШИБКА: Не удалось вычислить хэш файла!" << endl;
        return false;
    }

    QByteArray key = deriveKeyFromPassword(password);
    
    unsigned char nonce[NONCE_SIZE];
    if (RAND_bytes(nonce, NONCE_SIZE) != 1) {
        cout << "Ошибка генерации nonce!" << endl;
        return false;
    }
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        cout << "Ошибка создания контекста EVP!" << endl;
        return false;
    }
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        cout << "Ошибка инициализации шифрования!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, (unsigned char*)key.constData(), nonce) != 1) {
        cout << "Ошибка установки ключа и nonce!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        cout << "Ошибка создания временного файла!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    tempFile.write(originalHash);
    tempFile.write((char*)nonce, NONCE_SIZE);    
    
    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        cout << "Ошибка открытия файла для чтения!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        tempFile.close();
        return false;
    }
    
    cout << "Шифрование файла: " << filePath.toStdString() << endl;
    
    const int BUFFER_SIZE = 4096;
    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE];
    
    int bytesRead = 0;
    int outLen = 0;
    qint64 totalBytes = 0;
    
    while ((bytesRead = inFile.read((char*)inBuffer, BUFFER_SIZE)) > 0) {
        if (EVP_EncryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead) != 1) {
            cout << "Ошибка при шифровании блока!" << endl;
            inFile.close();
            tempFile.close();
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        tempFile.write((char*)outBuffer, outLen);
        totalBytes += bytesRead;
    }
    
    unsigned char tag[TAG_SIZE];
    if (EVP_EncryptFinal_ex(ctx, outBuffer, &outLen) != 1) {
        cout << "Ошибка при финализации шифрования!" << endl;
        inFile.close();
        tempFile.close();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        cout << "Ошибка получения тега аутентификации!" << endl;
        inFile.close();
        tempFile.close();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    tempFile.write((char*)tag, TAG_SIZE);
    
    inFile.close();
    tempFile.close();
    EVP_CIPHER_CTX_free(ctx);
    
    cout << "Исходный размер: " << totalBytes << " байт" << endl;
    
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }
    
    if (!QFile::copy(tempFile.fileName(), filePath)) {
        cout << "Ошибка при замене файла!" << endl;
        return false;
    }
    
    qint64 encryptedSize = QFileInfo(filePath).size();
    cout << "Зашифрованный размер: " << encryptedSize << " байт" << endl;
    cout << "Формат файла: [Хэш SHA-256 (32 байт)][Nonce (12 байт)][Зашифрованные данные][Тег (16 байт)]" << endl;
    
    cout << "=== AES-GCM ШИФРОВАНИЕ УСПЕШНО ЗАВЕРШЕНО ===\n" << endl;
    return true;
}

bool Encryptor::decryptFile(const QString &filePath, const QString &password) {
    cout << "\n=== НАЧАЛО AES-GCM ДЕШИФРОВАНИЯ ===" << endl;
    
    if (!QFile::exists(filePath)) {
        cout << "Ошибка: Файл не найден - " << filePath.toStdString() << endl;
        return false;
    }
    
    if (!isFileEncrypted(filePath)) {
        cout << "ОШИБКА: Файл не является зашифрованным!" << endl;
        return false;
    }
        
    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        cout << "Ошибка открытия файла для чтения!" << endl;
        return false;
    }
    
    QByteArray savedOriginalHash = inFile.read(HASH_SIZE);
    if (savedOriginalHash.size() != HASH_SIZE) {
        cout << "Ошибка чтения хэша из файла!" << endl;
        inFile.close();
        return false;
    }

    unsigned char nonce[NONCE_SIZE];
    if (inFile.read((char*)nonce, NONCE_SIZE) != NONCE_SIZE) {
        cout << "Ошибка чтения nonce из файла!" << endl;
        inFile.close();
        return false;
    }
    
    qint64 fileSize = inFile.size();
    qint64 dataSize = fileSize - HASH_SIZE - NONCE_SIZE - TAG_SIZE;
    
    if (dataSize <= 0) {
        cout << "Ошибка: Файл слишком мал или поврежден!" << endl;
        inFile.close();
        return false;
    }
    
    QByteArray encryptedData = inFile.read(dataSize);
    
    unsigned char tag[TAG_SIZE];
    if (inFile.read((char*)tag, TAG_SIZE) != TAG_SIZE) {
        cout << "Ошибка чтения тега аутентификации!" << endl;
        inFile.close();
        return false;
    }
    
    inFile.close();
    
        QByteArray key = deriveKeyFromPassword(password);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        cout << "Ошибка создания контекста дешифрования!" << endl;
        return false;
    }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        cout << "Ошибка инициализации дешифрования!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, 
                           (unsigned char*)key.constData(), nonce) != 1) {
        cout << "Ошибка установки ключа и nonce!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    QByteArray decryptedData;
    decryptedData.resize(dataSize);
    unsigned char* outBuffer = (unsigned char*)decryptedData.data();
    int outLen = 0;
    
    if (EVP_DecryptUpdate(ctx, outBuffer, &outLen, 
                          (unsigned char*)encryptedData.constData(), 
                          encryptedData.size()) != 1) {
        cout << "Ошибка при дешифровании данных!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    decryptedData.resize(outLen);
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        cout << "Ошибка установки тега аутентификации!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    int finalLen = 0;
    int result = EVP_DecryptFinal_ex(ctx, outBuffer + outLen, &finalLen);
    
    EVP_CIPHER_CTX_free(ctx);
    
    if (result != 1) {
        cout << "ОШИБКА: Неверный пароль или файл поврежден!" << endl;
        cout << "Тег аутентификации не совпадает." << endl;
        return false;
    }
    
    decryptedData.resize(outLen + finalLen);
    
    cout << "Дешифрование успешно!" << endl;
    cout << "Размер после дешифрования: " << decryptedData.size() << " байт" << endl;
    
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        cout << "Ошибка создания временного файла!" << endl;
        return false;
    }
    
    tempFile.write(decryptedData);
    tempFile.close();
    
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }
    
    if (!QFile::copy(tempFile.fileName(), filePath)) {
        cout << "Ошибка при замене файла!" << endl;
        return false;
    }
    
    cout << "ДЕШИФРОВАНИЕ ЗАВЕРШЕНО" << endl;
    return true;
}