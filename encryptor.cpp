#include <iostream>
#include "encryptor.h"
#include "crypto_constants.h"
#include "password_key_derivation.h"
#include <cstring>
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

bool Encryptor::isFileEncrypted(const QString &filePath) {
    if (!QFile::exists(filePath)) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    char magic[MAGIC_SIZE];
    
    if (file.read(magic, MAGIC_SIZE) != MAGIC_SIZE) {
        file.close();
        return false;
    }
    
    file.close();

    //Сравниваем побайтно magic и MAGIC
    return memcmp(magic, MAGIC, MAGIC_SIZE) == 0;
}

bool Encryptor::encryptFile(const QString &filePath, const QString &password) {
    cout << "\nНАЧАЛО ШИФРОВАНИЯ ===" << endl;
    
    if (!QFile::exists(filePath)) {
        cout << "ОШИБКА: Файл не найден: " << filePath.toStdString() << endl;
        return false;
    }

    if (isFileEncrypted(filePath)) {
        cout << "ОШИБКА: Файл уже зашифрован" << endl;
        return false;
    }

    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        cout << "Ошибка генерации salt!" << endl;
        return false;
    }

QByteArray key = PasswordKeyDerivation::deriveKeyFromPassword(password, salt);    

    if (key.isEmpty()) {
    cout << "Ошибка генерации ключа!" << endl;
    return false;
    }
    
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
    
    tempFile.write(MAGIC, MAGIC_SIZE);
    tempFile.write((char*)salt, 16);
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
    
    qint64 encryptedSize = QFileInfo(filePath).size();//самопис???????
    cout << "Зашифрованный размер: " << encryptedSize << " байт" << endl;
    
    cout << "=== AES-GCM ШИФРОВАНИЕ УСПЕШНО ЗАВЕРШЕНО ===\n" << endl;
    return true;
}