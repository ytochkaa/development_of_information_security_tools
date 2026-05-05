#include "encryptor.h"
#include "decryptor.h"

class CryptoManager {
public:
    static CryptoManager& instance();

    bool encrypt(const QString &filePath, const QString &password);
    bool decrypt(const QString &filePath, const QString &password);
    bool encryptDirectory(const QString &dirPath, const QString &password);
    bool decryptDirectory(const QString &dirPath, const QString &password);

private:
    CryptoManager();

    Encryptor encryptor;
    Decryptor decryptor;
};