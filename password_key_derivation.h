#ifndef password_key_derivation_h
#define password_key_derivation_h

#include <QString>
#include <QByteArray>

class PasswordKeyDerivation {
public:
    static QByteArray deriveKeyFromPassword(const QString &password, const unsigned char* salt);
};

#endif 