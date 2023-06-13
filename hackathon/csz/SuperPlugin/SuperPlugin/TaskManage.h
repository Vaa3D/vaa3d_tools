#ifndef TASKMANAGE_H
#define TASKMANAGE_H

#include <QThread>
#include "CallPlugin.h"
#include <mutex>
//#include "Task.h"
//#include "PluginPipeline.h"
#include <QTimer>
#include <QMap>

class Task;
class PluginPipeLine;
class TaskManage:public QObject{
    Q_OBJECT
public:
    TaskManage(const TaskManage&) = delete;
    TaskManage& operator=(const TaskManage&) = delete;

    static TaskManage* get_instance() {
        if (!_instance) {
            mutex_.lock();
            if (!_instance) {
                _instance = new TaskManage();
            }
            mutex_.unlock();
        }
        return _instance;
    }

    static void initThreadPool(int num);
    static void addTask(CallPlugin *cp,PluginPipeLine *pp);
    static void updateStatus(Task *currthread);
    static void printStatus();
signals:
    void preprocess(QString name);
    void trace(QString name);
    void postprocess(QString name);

public slots:
    void starttimer();
    void handletimer();
    void readytopreprocess(QString name);
    void readytotrace(QString name);
    void readytopostprocess(QString name);
private:
    static TaskManage* _instance;
    static QMutex mutex_;
    static QMutex updatemutex;
    static QMutex addtasklog;
    static QMutex updatetasklog;

    TaskManage() {}

    QTimer *m_timer;

    static int maxThreadNum;
    static int currentTask;
    static int waitingTask;
    static QList<Task*> taskPool;
    static QMap<CallPlugin*,PluginPipeLine*> waitingPool;
    static QList<bool> boolPool;


};

#endif // TASKMANAGE_H
