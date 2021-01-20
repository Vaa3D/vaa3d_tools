#include <QCoreApplication>
#include "manage.h"
#include "customdebug.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMsgHandler(customMessageHandler);
    ManageServer manageserver;
    return a.exec();
}




