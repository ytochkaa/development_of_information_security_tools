#include <iostream>
#include <cstring>
#include "encryptor.h"
#include "decryptor.h"
#include "crypto_constants.h"
#include "password_key_derivation.h"

#include <QFile>
#include <QDir>
#include <QSaveFile>
#include <QCryptographicHash>
#include <QDebug>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace std;

Decryptor::Decryptor() {
    OpenSSL_add_all_algorithms();
}
bool Decryptor::decryptFile(const QString &filePath, const QString &password) {
    cout << "\n=== НАЧАЛО AES-GCM ДЕШИФРОВАНИЯ ===" << endl;
    
    if (!QFile::exists(filePath)) {
        cout << "Ошибка: Файл не найден - " << filePath.toStdString() << endl;
        return false;
    }
        
    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        cout << "Ошибка открытия файла для чтения!" << endl;
        return false;
    }
    
    char magic[MAGIC_SIZE];

    if (inFile.read(magic, MAGIC_SIZE) != MAGIC_SIZE) {
        cout << "Ошибка чтения сигнатуры!" << endl;
        inFile.close();
        return false;
    }

    if (memcmp(magic, MAGIC, MAGIC_SIZE) != 0) {
        cout << "Ошибка: файл не зашифрован этой программой!" << endl;
        inFile.close();
        return false;
    }

    unsigned char salt[SALT_SIZE];
    if (inFile.read((char*)salt, sizeof(salt)) != sizeof(salt)) {
        cout << "Ошибка чтения salt из файла!" << endl;
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
    qint64 dataSize = fileSize - SALT_SIZE - NONCE_SIZE - TAG_SIZE - MAGIC_SIZE;

    if (dataSize <= 0) {
        cout << "Ошибка: Файл слишком мал или поврежден!" << endl;
        inFile.close();
        return false;
    }

    unsigned char tag[TAG_SIZE];
    // We will read encrypted data in blocks and finally read tag at the end
    // Move file position to the start of encrypted payload (already there)

    QByteArray key = PasswordKeyDerivation::deriveKeyFromPassword(password, salt);
    if (key.isEmpty()) {
        cout << "Ошибка генерации ключа!" << endl;
        inFile.close();
        return false;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        cout << "Ошибка создания контекста дешифрования!" << endl;
        inFile.close();
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        cout << "Ошибка инициализации дешифрования!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, NULL, NULL,
                           (unsigned char*)key.constData(), nonce) != 1) {
        cout << "Ошибка установки ключа и nonce!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    QSaveFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        cout << "Ошибка создания временного файла для безопасной записи!" << endl;
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    const int BUFFER_SIZE = 4096;
    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE + 16];

    qint64 remaining = dataSize;
    int outLen = 0;

    while (remaining > 0) {
        int toRead = remaining > BUFFER_SIZE ? BUFFER_SIZE : (int)remaining;
        int bytesRead = inFile.read((char*)inBuffer, toRead);
        if (bytesRead <= 0) {
            cout << "Ошибка чтения зашифрованного блока!" << endl;
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            inFile.close();
            return false;
        }

        if (EVP_DecryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead) != 1) {
            cout << "Ошибка при дешифровании блока!" << endl;
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            inFile.close();
            return false;
        }

        if (outFile.write((char*)outBuffer, outLen) != outLen) {
            cout << "Ошибка записи расшифрованного блока!" << endl;
            outFile.cancelWriting();
            EVP_CIPHER_CTX_free(ctx);
            inFile.close();
            return false;
        }

        remaining -= bytesRead;
    }

    // После чтения зашифрованных данных читаем тег
    if (inFile.read((char*)tag, TAG_SIZE) != TAG_SIZE) {
        cout << "Ошибка чтения тега аутентификации!" << endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        cout << "Ошибка установки тега аутентификации!" << endl;
        outFile.cancelWriting();
        EVP_CIPHER_CTX_free(ctx);
        inFile.close();
        return false;
    }

    int finalLen = 0;
    int result = EVP_DecryptFinal_ex(ctx, outBuffer, &finalLen);

    EVP_CIPHER_CTX_free(ctx);
    inFile.close();

    if (result != 1) {
        cout << "ОШИБКА: Неверный пароль или файл поврежден!" << endl;
        cout << "Тег аутентификации не совпадает." << endl;
        outFile.cancelWriting();
        return false;
    }

    if (finalLen > 0) {
        if (outFile.write((char*)outBuffer, finalLen) != finalLen) {
            cout << "Ошибка записи финального блока!" << endl;
            outFile.cancelWriting();
            return false;
        }
    }

    if (!outFile.commit()) {
        cout << "Ошибка при безопасной замене файла!" << endl;
        return false;
    }

    cout << "ДЕШИФРОВАНИЕ ЗАВЕРШЕНО" << endl;
    return true;
}
