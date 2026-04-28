#include <QString>
#include <QByteArray>
#include "crypto_constants.h"
class Decryptor {
public:
    Decryptor();
    bool decryptFile(const QString &filePath, const QString &password);
};