#include <QCoreApplication>
#include <QtCore>
#include "server.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;
    return a.exec();
}
