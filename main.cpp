#include <QCoreApplication>
#include <clocale>

#include "console_interface.h"

int main(int argc, char *argv[]){
    setlocale(LC_ALL, "Russian");
    QCoreApplication a(argc, argv);

    ConsoleInterface consoleInterface;
    consoleInterface.run();

    return 0;
}
