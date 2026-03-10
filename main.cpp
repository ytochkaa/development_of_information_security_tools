#include <QCoreApplication>
#include <QTextStream>
#include "directorywalker.h"
/*
int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);

    QString startPath = "C:/Users/darya/Desktop/Combez/7_semester/Number_Theory_Methods_in_Cryptography";    
    QTextStream out(stdout);

    DirectoryWalker walker;

    walker.listDirsRecursively(startPath, out);

    return 0;
}
    */
#include <iostream>
#include <openssl/crypto.h>

int main() {
    std::cout << "OpenSSL version: " << OpenSSL_version(OPENSSL_VERSION) << std::endl;
    return 0;
}