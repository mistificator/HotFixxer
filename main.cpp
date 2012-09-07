#include <QCoreApplication>
#include "qhotfixxer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QHotFixxer fixxer;
    fixxer.fix();

    return 0;
}
