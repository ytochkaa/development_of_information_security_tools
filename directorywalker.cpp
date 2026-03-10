#include <QDir>
#include "directorywalker.h"
#include <QTextStream>
#include <QCoreApplication>

void DirectoryWalker::listDirsRecursively(const QString &path, QTextStream &out)
{
    QDir dir(path);

    if (!dir.exists()) {
        out << "The path does not exist:" << path << Qt::endl;
        return;
    }

    out << "Directory:" << dir.absolutePath() << Qt::endl;

    //Подпапки
    //QDir::AllDirs	попробовать
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList subDirs = dir.entryList();

    //Рекурсия
    for (const QString &subDir : subDirs) {
        QString fullPath = dir.absoluteFilePath(subDir);
        listDirsRecursively(fullPath, out);
    }
}