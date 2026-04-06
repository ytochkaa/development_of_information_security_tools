#include "password_key_derivation.h"
#include "crypto_constants.h"

#include <openssl/evp.h>
#include <openssl/crypto.h>

QByteArray PasswordKeyDerivation::deriveKeyFromPassword(const QString &password, const unsigned char* salt) {
    
    unsigned char key[KEY_SIZE];
    QByteArray passwordBytes = password.toUtf8();
    
    int result = PKCS5_PBKDF2_HMAC(
        passwordBytes.constData(),
        passwordBytes.size(),
        salt,
        16,
        100000,
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