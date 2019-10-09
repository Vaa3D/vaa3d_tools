#ifndef MYSERVER_H
#define MYSERVER_H


#include "mythread.h"



class MyServer:public QTcpServer
{
    Q_OBJECT

public:
    explicit MyServer(ThreadList *list=0,Global_Parameters *parameters=0, QObject *PARENT=0);

public slots:
    void MyServerSlotAnswerMyThread_sendtoall(const QString &msg);

private:

    void incomingConnection(int socketDesc);

private:
    ThreadList *threadlist;
    Global_Parameters *global_parameters;

};

#endif // MYSERVER_H
