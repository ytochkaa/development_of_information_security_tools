#ifndef CONSOLE_INTERFACE_H
#define CONSOLE_INTERFACE_H

#include <QString>
#include <string>

class ConsoleInterface {
public:
    void run();

private:
    enum class Operation {
        Encrypt,
        Decrypt
    };

    void handleOperation(Operation operation);
    bool processPath(const QString &path, const QString &password, Operation operation);
    static const char* operationTitle(Operation operation);
    static void clearPassword(std::string &password);
};

#endif // CONSOLE_INTERFACE_H
