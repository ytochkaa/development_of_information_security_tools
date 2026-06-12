#include "test_utils.h"
#include "crypto_constants.h"
#include "password_key_derivation.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <iostream>
#include <algorithm>

QString TestUtils::testsBasePath() {
    return QDir::currentPath() + "/tests/";
}

bool TestUtils::copyDirectory(const QString &src, const QString &dst) {
    QDir srcDir(src);
    if (!srcDir.exists()) {
        std::cout << "Ошибка: шаблон не найден: " << src.toStdString() << std::endl;
        return false;
    }

    QDir().mkpath(dst);

    QDirIterator it(src, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QString relativePath = srcDir.relativeFilePath(it.filePath());
        QString dstPath = dst + "/" + relativePath;

        if (it.fileInfo().isDir()) {
            QDir().mkpath(dstPath);
        } else {
            QFile::remove(dstPath);
            QFile::copy(it.filePath(), dstPath);
        }
    }
    return true;
}

std::string TestUtils::getPassword(const std::string &prompt) {
    std::string password;
    std::cout << prompt << "(от " << MIN_PASSWORD_LENGTH
              << " до " << MAX_PASSWORD_LENGTH << " символов): ";
    std::getline(std::cin, password);
    return password;
}

void TestUtils::clearPassword(std::string &password) {
    std::fill(password.begin(), password.end(), '\0');
}
