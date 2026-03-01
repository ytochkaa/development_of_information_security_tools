#include <QDir>
//#include <QDebug>
#include <QTextStream>
#include <QCoreApplication>

void listDirsRecursively(const QString &path, QTextStream &out)
{
    QDir dir(path);

    if (!dir.exists()) {
        out << "The path does not exist:" << path;
        return;
    }

    out << "Directory:" << dir.absolutePath();

    //Подпапки
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList subDirs = dir.entryList();

    //Рекурсия
    for (const QString &subDir : subDirs) {
        QString fullPath = dir.absoluteFilePath(subDir);
        listDirsRecursively(fullPath, out);
    }
}

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);

    QString startPath = " ";//тут должен быть путь 
    QTextStream out(stdout);

    listDirsRecursively(startPath, out);

    return 0;
}