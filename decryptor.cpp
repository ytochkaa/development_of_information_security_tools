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

    unsigned char salt[16];
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
    qint64 dataSize = fileSize - 16 - NONCE_SIZE - TAG_SIZE - MAGIC_SIZE;    
    
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
    
    QByteArray key = PasswordKeyDerivation::deriveKeyFromPassword(password, salt); 

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
    
    QSaveFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        cout << "Ошибка создания временного файла для безопасной записи!" << endl;
        return false;
    }
    
    if (outFile.write(decryptedData) != decryptedData.size()) {
        cout << "Ошибка записи расшифрованного файла!" << endl;
        outFile.cancelWriting();
        return false;
    }
    if (!outFile.commit()) {
        cout << "Ошибка при безопасной замене файла!" << endl;
        return false;
    }
    
    cout << "ДЕШИФРОВАНИЕ ЗАВЕРШЕНО" << endl;
    return true;
}
