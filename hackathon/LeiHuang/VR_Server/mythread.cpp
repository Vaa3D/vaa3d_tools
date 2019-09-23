#include "mythread.h"
MyThread::MyThread(int socketDesc, ThreadList *list,Global_Parameters *parameters, QObject *parent)
    :socketId(socketDesc),global_parameters(parameters),QThread (parent),threadlist(list)
{
    qDebug()<<"make a MyThread."<<QThread::currentThreadId();
}

void MyThread::sendtouser(const QString &msg)
{
    emit MyThreadSignalToMySocket_sendtouser(msg);
}

void MyThread::MyThreadSlotAnswerMySocket_disconnect()
{
    this->socket->deleteLater();
    threadlist->remove(this);
    this->exit();
    this->deleteLater();
}

void MyThread::MyThreadSlotAnswerMySocket_sendtoall(const QString &msg)
{
    emit MyThreadSignalToMyServer_sendtoall(msg);
}

void MyThread::run()
{
    socket=new MySocket(socketId,global_parameters);
    threadlist->add(this,socket);

    if(!socket->setSocketDescriptor(socketId))
    {
        qDebug()<<"fatal error .";
        return;
    }

    connect(socket,SIGNAL(MySocketSignalToMyThread_disconnected()),this,SLOT(MyThreadSlotAnswerMySocket_disconnect()));
    connect(socket,SIGNAL(MySocketSignalToMyThread_sendtoall(const QString &)),this,SLOT(MyThreadSlotAnswerMySocket_sendtoall(const QString &)));
    connect(this,SIGNAL(MyThreadSignalToMySocket_sendtouser(const QString &)),socket,SLOT(MySocketSlotAnswerMyThread_sendtouser(const QString &)));

    this->exec();
}
