#include <QString>
#include <QByteArray>
#include "crypto_constants.h"

class Encryptor {
public:
    Encryptor();
    static bool isFileEncrypted(const QString &filePath);
    
    bool encryptFile(const QString &filePath, const QString &password);
};