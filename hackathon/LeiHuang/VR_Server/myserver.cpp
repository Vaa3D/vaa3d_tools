#include "myserver.h"

MyServer::MyServer(ThreadList *list,Global_Parameters *parameters, QObject *PARENT)
    :threadlist(list),global_parameters(parameters),QTcpServer (PARENT)
{
    qDebug()<<"make a server";
}

void MyServer::incomingConnection(int socketDesc)
{
    qDebug()<<"a new connection comming";

    MyThread *thread= new MyThread(socketDesc,threadlist,global_parameters);
    connect(thread,SIGNAL(MyThreadSignalToMyServer_sendtoall(const QString &)),this,SLOT(MyServerSlotAnswerMyThread_sendtoall(const QString &)));


    thread->start();
}

void MyServer::MyServerSlotAnswerMyThread_sendtoall(const QString &msg)
{
    threadlist->sendtoall(msg);
}

