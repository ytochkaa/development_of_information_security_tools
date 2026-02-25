#include <QDir>
#include <QDebug>
#include <QCoreApplication>

void listDirsRecursively(const QString &path)
{
    QDir dir(path);

    if (!dir.exists()) {
        qDebug() << "The path does not exist:" << path;
        return;
    }

    qDebug() << "Directory:" << dir.absolutePath();

    //Подпапки
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList subDirs = dir.entryList();

    //Рекурсия
    for (const QString &subDir : subDirs) {
        QString fullPath = dir.absoluteFilePath(subDir);
        listDirsRecursively(fullPath);
    }
}

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);

    QString startPath = " ";//тут должен быть путь 

    listDirsRecursively(startPath);

    return 0;
}