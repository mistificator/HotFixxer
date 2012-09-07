#include <QCoreApplication>
#include <QLocale>
#include <QTextCodec>
#include "qhotfixxer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 866"));

    QHotFixxer fixxer;
    fixxer.fix();

    return 0;
}
