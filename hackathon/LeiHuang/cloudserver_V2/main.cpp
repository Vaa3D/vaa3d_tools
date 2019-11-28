#include <QCoreApplication>
#include "manage.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ManageServer manageserver;
    return a.exec();
}
