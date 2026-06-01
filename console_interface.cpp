#include "console_interface.h"

#include "crypto_constants.h"
#include "crypto_manager.h"
#include "password_key_derivation.h"

#include <QFileInfo>
#include <algorithm>
#include <iostream>

void ConsoleInterface::run() {
    while (true) {
        std::cout << "Выберите действие:" << std::endl;
        std::cout << "1. Зашифровать" << std::endl;
        std::cout << "2. Расшифровать" << std::endl;
        std::cout << "0. Выход" << std::endl;
        std::cout << "Ваш выбор: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "0") {
            std::cout << "Программа завершена." << std::endl;
            return;
        }

        if (choice == "1") {
            handleOperation(Operation::Encrypt);
        } else if (choice == "2") {
            handleOperation(Operation::Decrypt);
        } else {
            std::cout << "Неверный выбор!" << std::endl;
        }
    }
}

void ConsoleInterface::handleOperation(Operation operation) {
    std::cout << operationTitle(operation) << std::endl;

    std::string path;
    std::string password;

    std::cout << "Введите путь к файлу или директории: ";
    std::getline(std::cin, path);

    std::cout << "Введите пароль (от " << MIN_PASSWORD_LENGTH << " до "
              << MAX_PASSWORD_LENGTH << " символов): ";
    std::getline(std::cin, password);

    if (!PasswordKeyDerivation::validatePassword(password)) {
        clearPassword(password);
        return;
    }

    const bool success = processPath(
        QString::fromStdString(path),
        QString::fromStdString(password),
        operation
    );

    if (success) {
        std::cout << "Операция успешно завершена!" << std::endl;
    } else {
        std::cout << "Ошибка при выполнении операции!" << std::endl;
    }

    clearPassword(password);
}

bool ConsoleInterface::processPath(const QString &path, const QString &password, Operation operation) {
    QFileInfo pathInfo(path);

    if (!pathInfo.exists()) {
        std::cout << "Ошибка: указанный путь не существует: " << path.toStdString() << std::endl;
        return false;
    }

    CryptoManager &cryptoManager = CryptoManager::instance();

    if (pathInfo.isFile()) {
        if (operation == Operation::Encrypt) {
            return cryptoManager.encrypt(path, password);
        }

        return cryptoManager.decrypt(path, password);
    }

    if (pathInfo.isDir()) {
        if (operation == Operation::Encrypt) {
            return cryptoManager.encryptDirectory(path, password);
        }

        return cryptoManager.decryptDirectory(path, password);
    }

    std::cout << "Ошибка: путь не является файлом или директорией" << std::endl;
    return false;
}

const char* ConsoleInterface::operationTitle(Operation operation) {
    if (operation == Operation::Encrypt) {
        return "ШИФРОВАНИЕ";
    }

    return "ДЕШИФРОВАНИЕ";
}

void ConsoleInterface::clearPassword(std::string &password) {
    std::fill(password.begin(), password.end(), '\0');
}
