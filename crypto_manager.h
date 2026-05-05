#include "encryptor.h"
#include "decryptor.h"
#include <QString>

class CryptoManager {
public:
    static CryptoManager& instance();

    bool encrypt(const QString &filePath, const QString &password);
    bool decrypt(const QString &filePath, const QString &password);
    bool encryptDirectory(const QString &dirPath, const QString &password);
    bool decryptDirectory(const QString &dirPath, const QString &password);

    static bool isValidFileForEncryption(const QString &filePath);
    static bool isValidFileForDecryption(const QString &filePath);
    static bool isValidDirectory(const QString &dirPath);

private:
    CryptoManager();

    Encryptor encryptor;
    Decryptor decryptor;
};