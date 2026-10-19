#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <QString>
#include <string>

namespace TestUtils {
    QString testsBasePath();
    bool copyDirectory(const QString &src, const QString &dst);
    std::string getPassword(const std::string &prompt = "Введите пароль: ");
    void clearPassword(std::string &password);
}

#endif // TEST_UTILS_H
