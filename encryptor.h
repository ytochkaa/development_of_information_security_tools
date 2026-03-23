#include <QString>
#include <QByteArray>

static const int KEY_SIZE = 32;  
static const int NONCE_SIZE = 12;
static const int TAG_SIZE = 16;
const int HASH_SIZE = 32;

class Encryptor {
public:
    Encryptor();
    static bool isFileEncrypted(const QString &filePath);
    
    bool encryptFile(const QString &filePath, const QString &password);
    bool decryptFile(const QString &filePath, const QString &password);
    
private:
    QByteArray deriveKeyFromPassword(const QString &password);
    static QByteArray calculateFileHash(const QString &filePath);
    static bool verifyFileHash(const QString &filePath, const QByteArray &expectedHash);
    
};