#include <QCoreApplication>
#include "customdebug.h"
#include "myserver.h"
#define PORT 1234
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMsgHandler(customMessageHandler);
    ThreadList threadlist;
    Global_Parameters global_parameters;
    global_parameters.clients.clear();
    global_parameters.clientNum=0;
    global_parameters.clientsproperty.clear();
    global_parameters.messagelist1.clear();
    global_parameters.messagelist2.clear();


    global_parameters.currentmessagelist=&(global_parameters.messagelist1);
    global_parameters.lock_currentmessagelist=&(global_parameters.lock_messagelist1);

    global_parameters.backupmessagelist=&(global_parameters.messagelist2);
    global_parameters.lock_backupmessagelist=&(global_parameters.lock_messagelist2);

    MyServer server(&threadlist,&global_parameters);
    if(!server.listen(QHostAddress::Any,PORT))
    {
         qDebug()<<"Server is not started.";
    }else {
            qDebug()<<"Server is started.";
    }
    return a.exec();
}
