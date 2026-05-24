#include <iostream>
#include "encryptor.h"
#include "crypto_constants.h"
#include "password_key_derivation.h"
#include <cstring>
#include <QFile>
#include <QDir>
#include <QSaveFile>
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

    unsigned char salt[SALT_SIZE];
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
    
    QSaveFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        cout << "Ошибка создания временного файла для безопасной записи!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    if (outFile.write(MAGIC, MAGIC_SIZE) != MAGIC_SIZE ||
        outFile.write((char*)&FORMAT_VERSION, FORMAT_VERSION_SIZE) != FORMAT_VERSION_SIZE ||
        outFile.write((char*)salt, SALT_SIZE) != SALT_SIZE ||
        outFile.write((char*)nonce, NONCE_SIZE) != NONCE_SIZE) {
        cout << "Ошибка записи заголовка зашифрованного файла!" << endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        cout << "Ошибка открытия файла для чтения!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        outFile.cancelWriting();
        return false;
    }
    
    cout << "Шифрование файла: " << filePath.toStdString() << endl;
    
    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE];
    
    int bytesRead = 0;
    int outLen = 0;
    qint64 totalBytes = 0;
    
    while ((bytesRead = inFile.read((char*)inBuffer, BUFFER_SIZE)) > 0) {
        if (EVP_EncryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead) != 1) {
            cout << "Ошибка при шифровании блока!" << endl;
            inFile.close();
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        if (outFile.write((char*)outBuffer, outLen) != outLen) {
            cout << "Ошибка записи зашифрованного блока!" << endl;
            inFile.close();
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        totalBytes += bytesRead;
    }
    
    unsigned char tag[TAG_SIZE];
    if (EVP_EncryptFinal_ex(ctx, outBuffer, &outLen) != 1) {
        cout << "Ошибка при финализации шифрования!" << endl;
        inFile.close();
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        cout << "Ошибка получения тега аутентификации!" << endl;
        inFile.close();
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (outFile.write((char*)tag, TAG_SIZE) != TAG_SIZE) {
        cout << "Ошибка записи тега аутентификации!" << endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    
    inFile.close();
    EVP_CIPHER_CTX_free(ctx);
    
    cout << "Исходный размер: " << totalBytes << " байт" << endl;

    if (!outFile.commit()) {
        cout << "Ошибка при безопасной замене файла!" << endl;
        return false;
    }
    
    qint64 encryptedSize = QFileInfo(filePath).size();
    cout << "Зашифрованный размер: " << encryptedSize << " байт" << endl;
    
    cout << "=== AES-GCM ШИФРОВАНИЕ УСПЕШНО ЗАВЕРШЕНО ===\n" << endl;
    return true;
}
