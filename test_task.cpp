#include <QDir>
//#include <QDebug>
#include <QTextStream>
#include <QCoreApplication>

void listDirsRecursively(const QString &path, QTextStream &out)
{
    QDir dir(path);

    if (!dir.exists()) {
        out << "The path does not exist:" << path << endl;
        return;
    }

    out << "Directory:" << dir.absolutePath() << endl;

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

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);

    QString startPath = "C:/Users/darya/Desktop/Combez/7_semester/Number_Theory_Methods_in_Cryptography";    
    QTextStream out(stdout);

    listDirsRecursively(startPath, out);

    return 0;
}