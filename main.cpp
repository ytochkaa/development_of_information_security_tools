#include <QCoreApplication>
#include <iostream>
#include "crypto_constants.h"
#include "crypto_manager.h"
#include "password_key_derivation.h"
#include "test_menu.h"

using namespace std;

int main(int argc, char *argv[]){
    setlocale(LC_ALL, "Russian");
    QCoreApplication a(argc, argv);

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--test") {
            TestMenu testMenu;
            testMenu.run();
            return 0;
        }
    }

    while (true) {
        cout << "Выберите действие:" << endl;
        cout << "1. Зашифровать файл" << endl;
        cout << "2. Расшифровать файл" << endl;
        cout << "3. Зашифровать директорию" << endl;
        cout << "4. Расшифровать директорию" << endl;
        cout << "0. Выход" << endl;
        cout << "Ваш выбор: ";

        string choice;
        std::getline(cin, choice);
        
        if (choice == "0") {
            cout << "Программа завершена." << endl;
            return 0;
        } 
        
        string filePathStr;
        string passwordStr;
        
        if (choice == "1") {
            cout << "ШИФРОВАНИЕ" << endl;
                
            cout << "Введите путь к файлу для шифрования: ";
            std::getline(cin, filePathStr);
                
            cout << "Введите пароль (от " << MIN_PASSWORD_LENGTH << " до " 
                 << MAX_PASSWORD_LENGTH << " символов): ";
            std::getline(cin, passwordStr);
            
            if (!PasswordKeyDerivation::validatePassword(passwordStr)) {
                continue;
            }
                
            if (CryptoManager::instance().encrypt(
                QString::fromStdString(filePathStr), 
                QString::fromStdString(passwordStr))) {
                cout << "Файл успешно зашифрован!" << endl;
            } else {
                cout << "Ошибка при шифровании файла!" << endl;
            }
            
            // Очистка памяти пароля
            std::fill(passwordStr.begin(), passwordStr.end(), '\0');
        }    
        else if (choice == "2") {
            cout << "ДЕШИФРОВАНИЕ" << endl;
                
            cout << "Введите путь к файлу для дешифрования: ";
            std::getline(cin, filePathStr);
                
            cout << "Введите пароль (от " << MIN_PASSWORD_LENGTH << " до " 
                 << MAX_PASSWORD_LENGTH << " символов): ";
            std::getline(cin, passwordStr);
            
            if (!PasswordKeyDerivation::validatePassword(passwordStr)) {
                continue;
            }
                
            // Выполняем дешифрование
            if (CryptoManager::instance().decrypt(
                QString::fromStdString(filePathStr), 
                QString::fromStdString(passwordStr))) {
                cout << "Файл успешно дешифрован!" << endl;
            } else {
                cout << "Ошибка при дешифровании файла!" << endl;
            }
            
            // Очистка памяти пароля
            std::fill(passwordStr.begin(), passwordStr.end(), '\0');
        }
        else if (choice == "3") {
            cout << "ШИФРОВАНИЕ ДИРЕКТОРИИ" << endl;
                
            cout << "Введите путь к директории для шифрования: ";
            std::getline(cin, filePathStr);
                
            cout << "Введите пароль (от " << MIN_PASSWORD_LENGTH << " до " 
                 << MAX_PASSWORD_LENGTH << " символов): ";
            std::getline(cin, passwordStr);
            
            if (!PasswordKeyDerivation::validatePassword(passwordStr)) {
                continue;
            }
                
            if (CryptoManager::instance().encryptDirectory(
                QString::fromStdString(filePathStr), 
                QString::fromStdString(passwordStr))) {
                cout << "Директория успешно зашифрована!" << endl;
            } else {
                cout << "Ошибка при шифровании директории!" << endl;
            }
            
            // Очистка памяти пароля
            std::fill(passwordStr.begin(), passwordStr.end(), '\0');
        }
        else if (choice == "4") {
            cout << "ДЕШИФРОВАНИЕ ДИРЕКТОРИИ" << endl;
                
            cout << "Введите путь к директории для дешифрования: ";
            std::getline(cin, filePathStr);
                
            cout << "Введите пароль (от " << MIN_PASSWORD_LENGTH << " до " 
                 << MAX_PASSWORD_LENGTH << " символов): ";
            std::getline(cin, passwordStr);
            
            if (!PasswordKeyDerivation::validatePassword(passwordStr)) {
                continue;
            }
                
            if (CryptoManager::instance().decryptDirectory(
                QString::fromStdString(filePathStr), 
                QString::fromStdString(passwordStr))) {
                cout << "Директория успешно дешифрована!" << endl;
            } else {
                cout << "Ошибка при дешифровании директории!" << endl;
            }
            
            // Очистка памяти пароля
            std::fill(passwordStr.begin(), passwordStr.end(), '\0');
        }
        else { 
        cout << "Неверный выбор!" << endl;
        }
    }
    return 0;
}
