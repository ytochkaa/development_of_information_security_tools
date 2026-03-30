#include <QCoreApplication>
#include <QTextStream>
#include <QDir>
#include <iostream>
#include "directorywalker.h"
#include "encryptor.h"
#include <openssl/crypto.h>

using namespace std;

int main(int argc, char *argv[]){
    setlocale(LC_ALL, "Russian");
    QCoreApplication a(argc, argv);
    Encryptor encryptor;
    QTextStream out(stdout);

    while (true) {
        cout << "Выберите действие:" << endl;
        cout << "1. Зашифровать файл" << endl;
        cout << "2. Расшифровать файл" << endl;
        cout << "0. Выход" << endl;
        cout << "Ваш выбор: ";
        //C:\Users\darya\Desktop\Combez\8_semester\Development of information security tools\development_of_information_security_tools\test_zone\test1.txt
        //C:\Users\darya\Desktop\Combez\8_semester\Development of information security tools\development_of_information_security_tools\test_zone\тест1.txt
        //1305221
        int choice;
        cin >> choice;
        cin.ignore();
        
        if (choice == 0) {
            cout << "Программа завершена." << endl;
            break;
        } 
        
        string filePathStr;
        string passwordStr;
        
        if (choice==1) {
            cout << "ШИФРОВАНИЕ" << endl;
                
            cout << "Введите путь к файлу для шифрования: ";
            std::getline(cin, filePathStr);
                
            cout << "Введите пароль: ";
            std::getline(cin, passwordStr);
                
            if (encryptor.encryptFile(
                QString::fromStdString(filePathStr), 
                QString::fromStdString(passwordStr))) {
                cout << "Файл успешно зашифрован!" << endl;
            } else {
                cout << "Ошибка при шифровании файла!" << endl;
            }
            break;
        }    
        else if (choice==2) {
            cout << "ДЕШИФРОВАНИЕ" << endl;
                
            cout << "Введите путь к файлу для дешифрования: ";
            std::getline(cin, filePathStr);
                
            cout << "Введите пароль: ";
            std::getline(cin, passwordStr);
                
            // Выполняем дешифрование
            if (encryptor.decryptFile(
                QString::fromStdString(filePathStr), 
                QString::fromStdString(passwordStr))) {
                cout << "Файл успешно дешифрован!" << endl;
            } else {
                cout << "Ошибка при дешифровании файла!" << endl;
            }
            break;
        }
        else { 
        cout << "Неверный выбор!" << endl;
        break;
        }
    }
    return 0;
}
/*    
int main() {
    setlocale(LC_ALL, "Russian");
    std::cout << "OpenSSL version: " << OpenSSL_version(OPENSSL_VERSION) << std::endl;
    return 0;
}
*/