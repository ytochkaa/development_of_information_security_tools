#ifndef password_key_derivation_h
#define password_key_derivation_h

#include <QString>
#include <QByteArray>
#include <string>

// Константы валидации пароля
const int MIN_PASSWORD_LENGTH = 8;
const int MAX_PASSWORD_LENGTH = 256;

class PasswordKeyDerivation {
public:
    static QByteArray deriveKeyFromPassword(const QString &password, const unsigned char* salt);
    static bool validatePassword(const std::string &password);
};

#endif 