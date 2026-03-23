#include <QString>
#include <QByteArray>

class Encryptor {
private:
    static const int KEY_SIZE = 32;  
    static const int NONCE_SIZE = 12;
    static const int TAG_SIZE = 16;
    
public:
    Encryptor();
    
    bool encryptFile(const QString &filePath, const QString &password);
    bool decryptFile(const QString &filePath, const QString &password);
    
private:
    QByteArray deriveKeyFromPassword(const QString &password);
};