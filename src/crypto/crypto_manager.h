#ifndef CRYPTO_MANAGER_H
#define CRYPTO_MANAGER_H

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
    enum class Operation {
        Encrypt,
        Decrypt
    };

    CryptoManager();
    CryptoManager(const CryptoManager&) = delete;
    CryptoManager& operator=(const CryptoManager&) = delete;

    static bool isFileEncrypted(const QString &filePath);
    bool processFile(const QString &filePath, const QString &password, Operation operation);
    bool encryptFile(const QString &filePath, const QString &password);
    bool decryptFile(const QString &filePath, const QString &password);
};

#endif // CRYPTO_MANAGER_H
