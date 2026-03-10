#ifndef DIRECTORYWALKER_H
#define DIRECTORYWALKER_H

#include <QString>
#include <QTextStream>

class DirectoryWalker{
public: 
    void listDirsRecursively(const QString &path, QTextStream &out);
};

#endif