#include "password_key_derivation.h"
#include "crypto_constants.h"

#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <iostream>

using namespace std;

QByteArray PasswordKeyDerivation::deriveKeyFromPassword(const QString &password, const unsigned char* salt) {
    
    unsigned char key[KEY_SIZE];
    QByteArray passwordBytes = password.toUtf8();
    
    int result = PKCS5_PBKDF2_HMAC(
        passwordBytes.constData(),
        passwordBytes.size(),
        salt,
        SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        KEY_SIZE,
        key
    );

    if (result != 1) {
        return QByteArray();
    }
    
    QByteArray resultKey((char*)key, KEY_SIZE);

    OPENSSL_cleanse(key, KEY_SIZE);

    return resultKey;
}

bool PasswordKeyDerivation::validatePassword(const string &password) {
    if (password.empty()) {
        cout << "ОШИБКА: Пароль не может быть пустым!" << endl;
        return false;
    }
    
    if (password.length() < MIN_PASSWORD_LENGTH) {
        cout << "ОШИБКА: Пароль должен быть не менее " << MIN_PASSWORD_LENGTH 
             << " символов!" << endl;
        return false;
    }
    
    if (password.length() > MAX_PASSWORD_LENGTH) {
        cout << "ОШИБКА: Пароль не может быть более " << MAX_PASSWORD_LENGTH 
             << " символов!" << endl;
        return false;
    }
    
    return true;
}