#include "encryptor.h"
#include "decryptor.h"

class CryptoManager {
public:
    static CryptoManager& instance();

    bool encrypt(const QString &filePath, const QString &password);
    bool decrypt(const QString &filePath, const QString &password);

private:
    CryptoManager();

    Encryptor encryptor;
    Decryptor decryptor;
};