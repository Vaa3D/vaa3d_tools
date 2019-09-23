#ifndef MYTHREAD_H
#define MYTHREAD_H

#include "mysocket.h"

class ThreadList;

class MyThread:public QThread
{
    Q_OBJECT
public:
    explicit MyThread(int socketDesc,ThreadList *list=0, Global_Parameters *parameters=0, QObject *parent=0);
    void sendtouser(const QString &msg);
public slots:
    void MyThreadSlotAnswerMySocket_disconnect();
    void MyThreadSlotAnswerMySocket_sendtoall(const QString &msg);

signals:
    void MyThreadSignalToMySocket_sendtouser(const QString &msg);
    void MyThreadSignalToMyServer_sendtoall(const QString &msg);

private:
    void run();

private:
    MySocket *socket;
    Global_Parameters *global_parameters;
    int socketId;
    ThreadList *threadlist;

};

class ThreadList
{
public:
    void add(MyThread* thread, MySocket* socket)
    {
        lock_threadlist.lockForWrite();
        threadlist[thread]=socket;
        lock_threadlist.unlock();
    }
    void remove(MyThread* thread)
    {
        lock_threadlist.lockForWrite();
        threadlist.remove(thread);
        lock_threadlist.unlock();
    }

    void sendtoall(const QString &msg)//used by server
    {
        lock_threadlist.lockForRead();
        foreach(MyThread *thread,threadlist.keys())
        {
            thread->sendtouser(msg);
        }
        lock_threadlist.unlock();
    }


private:
    QMap <MyThread *,MySocket*> threadlist;
    QReadWriteLock lock_threadlist;
};

#endif // MYTHREAD_H
