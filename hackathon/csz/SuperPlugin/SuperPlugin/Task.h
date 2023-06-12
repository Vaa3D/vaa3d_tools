#ifndef TASK_H
#define TASK_H


#include <QThread>
#include "CallPlugin.h"
#include "TaskManage.h"
#include "FileManage.h"
#include "PluginPipeline.h"

class TaskManage;

class Task:public QThread{
    Q_OBJECT
public:
    Task(){
        showThreadId=true;
        isrunning=false;
        pp=nullptr;
    }
    ~Task(){}

    void newTask(CallPlugin *cp,PluginPipeLine *pluginp){
        mcp=*cp;
        pp=pluginp;
        isrunning=true;
//        QString cpini = QString::number(reinterpret_cast<quintptr>(cp), 16);
//        QString cpadd = QString::number(reinterpret_cast<quintptr>(&mcp), 16);
//        QString ppadd = QString::number(reinterpret_cast<quintptr>(pp), 16);
//        QString taskid = QString::number(reinterpret_cast<quintptr>(this), 16);
//        SpLog::debug("newtask:"+taskid+" "+cpini+" "+cpadd+" "+ppadd);
    }
    bool getStatus(){
        return isrunning;
    }


signals:
    void preprocess(QString name,PluginPipeLine*p);
    void trace(QString name,PluginPipeLine *p);
    void postprocess(QString name,PluginPipeLine *p);
    void predict(QString name,PluginPipeLine*p);
    void concat(QString name,PluginPipeLine*p);
protected:
    void run(){
            if(showThreadId)
                qDebug() << "Task Thread started, thread id: " << QThread::currentThreadId();
            if(mcp.getReady()){
                mcp.RunPlugin();
                vector<char*>* out=mcp.getOutput();
                if(out&&pp){
                    QString name=QString(out->at(0));

//                    QString cpadd = QString::number(reinterpret_cast<quintptr>(&mcp), 16);
//                    QString ppadd = QString::number(reinterpret_cast<quintptr>(pp), 16);
//                    QString taskid = QString::number(reinterpret_cast<quintptr>(this), 16);
//                    SpLog::debug("task:"+taskid+" "+cpadd+" "+ppadd+" "+name);

                    connect(this,SIGNAL(preprocess(QString,PluginPipeLine*)),pp,SLOT(ConvertFileFormat(QString,PluginPipeLine*)));
                    connect(this,SIGNAL(trace(QString,PluginPipeLine*)),pp,SLOT(tracingapp2(QString,PluginPipeLine*)));
                    connect(this,SIGNAL(postprocess(QString,PluginPipeLine*)),pp,SLOT(getLandMarkList(QString,PluginPipeLine*)));
                    connect(this,SIGNAL(predict(QString,PluginPipeLine*)),pp,SLOT(predict(QString,PluginPipeLine*)));
                    connect(this,SIGNAL(concat(QString,PluginPipeLine*)),pp,SLOT(submitSWC(QString,PluginPipeLine*)));
                    switch(mcp.type){
                    case 0:{

                        emit preprocess(name,pp);
                        break;
                    }
                    case 1:{
                        emit trace(name,pp);
                        break;
                    }
                    case 2:{
                        emit postprocess(name,pp);
                        break;
                    }
                    case 3:{
                        emit predict(name,pp);
                        break;
                    }
                    case 4:{
                        emit concat(name,pp);
                        break;
                    }
                    }
                }
                isrunning=false;
            }
            TaskManage::get_instance()->updateStatus(this);
    }

private:
    CallPlugin mcp;
    PluginPipeLine *pp;
    bool showThreadId;
    bool isrunning;
    static QMutex debugf;
};

#endif // TASK_H
