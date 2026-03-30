#include <QString>
#include <QByteArray>
#include "crypto_constants.h"
class Decryptor {
public:
    Decryptor();
    bool decryptFile(const QString &filePath, const QString &password);
    
private:
    QByteArray deriveKeyFromPassword(const QString &password, const unsigned char* salt);
};