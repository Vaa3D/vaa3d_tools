#include "TaskManage.h"
#include "Task.h"
#include "PluginPipeline.h"

TaskManage *TaskManage::_instance = nullptr;
QMutex TaskManage::mutex_ = QMutex();
QMutex TaskManage::updatemutex = QMutex();
QMutex TaskManage::addtasklog = QMutex();
QMutex TaskManage::updatetasklog = QMutex();
int TaskManage::maxThreadNum = 0;
int TaskManage::currentTask = 0;
int TaskManage::waitingTask = 0;
QList<Task*> TaskManage::taskPool = QList<Task*>();
QMap<CallPlugin*,PluginPipeLine*> TaskManage::waitingPool = QMap<CallPlugin*,PluginPipeLine*>();
QList<bool> TaskManage::boolPool = QList<bool>();

void TaskManage::initThreadPool(int num)
{
    if(maxThreadNum!=0){
        qDebug()<<"The ThreadPool has been initialied.";
    }
    maxThreadNum=num;
    currentTask=0;
    waitingTask=0;
}

void TaskManage::addTask(CallPlugin *cp,PluginPipeLine *pp)
{
    updatemutex.lock();

    if(taskPool.size()<maxThreadNum){
        Task* newtask=new Task;
        newtask->newTask(cp,pp);
        newtask->start();

        taskPool.append(newtask);
        boolPool.append(true);
        currentTask++;
//        QString cpadd = QString::number(reinterpret_cast<quintptr>(cp), 16);
//        QString ppadd = QString::number(reinterpret_cast<quintptr>(pp), 16);
//        SpLog::debug("add1:"+cpadd+" "+ppadd+" "+cp->getOutput()->at(0));
    }else{
        waitingPool.insert(cp,pp);
        waitingTask+=1;
//        QString cpadd = QString::number(reinterpret_cast<quintptr>(cp), 16);
//        QString ppadd = QString::number(reinterpret_cast<quintptr>(pp), 16);
//        SpLog::debug("add2:"+cpadd+" "+ppadd+" "+cp->getOutput()->at(0));
    }
//    if(currentTask<maxThreadNum){
//        if(taskPool.size()<maxThreadNum){
//            Task* newtask=new Task;
//            newtask->newTask(cp,pp);
//            newtask->start();
//            taskPool.append(newtask);
//            boolPool.append(true);
//            currentTask++;
//        }
//        else{
//            for(int i=0;i<taskPool.size();i++){
//                if(!taskPool[i]->isRunning()){
//                    taskPool[i]->newTask(cp,pp);
//                    taskPool[i]->start();
//                    currentTask++;
//                }
//            }
//        }
//    }else{
//        if(waitingPool.find(cp)!=waitingPool.end()){
////            pp->getcenter();
//            waitingPool[cp]->getcenter();

//        }
//        waitingPool.insert(cp,pp);
//        waitingTask+=1;
//    }
    updatemutex.unlock();
}

void TaskManage::updateStatus(Task *currthread)
{
    updatemutex.lock();

    for(int i=0;i<taskPool.size();i++){
        if(taskPool.at(i)==currthread){
            boolPool[i]=false;
            currentTask-=1;
//            QString taskid = QString::number(reinterpret_cast<quintptr>(currthread), 16);
//            SpLog::debug("update:"+taskid);
            break;
        }
    }
    updatemutex.unlock();
}

void TaskManage::printStatus()
{
    int count=0;
    for(int i=0;i<boolPool.size();i++){
        if(boolPool[i]==true){
            count++;
        }
    }
    qDebug()<<maxThreadNum<<currentTask<<waitingTask<<taskPool.size()<<waitingPool.size()<<count;
}

void TaskManage::starttimer()
{
    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(handletimer()));
    m_timer->start(200);
}

void TaskManage::handletimer()
{
    updatemutex.lock();
    QList<int> index;
    if(waitingTask>0&&currentTask<maxThreadNum){
        for(int i=0;i<taskPool.size();i++){
            if(!taskPool[i]->isRunning()){
                if(boolPool[i]==false){
                    index.append(i);
                }
            }
        }
        //if(index.size()>0){
        while(index.size()>0&&waitingTask>0){
            QMap<CallPlugin*,PluginPipeLine*>::iterator it=waitingPool.begin();
            CallPlugin* ready=it.key();
            PluginPipeLine* readyp=it.value();
            taskPool[index[0]]->newTask(ready,readyp);
            boolPool[index[0]]=true;
            currentTask+=1;
            waitingTask-=1;
            waitingPool.erase(it);
            taskPool[index[0]]->start();
            index.pop_front();
        }

//            QString taskid = QString::number(reinterpret_cast<quintptr>(taskPool[index[0]]), 16);
//            QString cpadd = QString::number(reinterpret_cast<quintptr>(ready), 16);
//            QString ppadd = QString::number(reinterpret_cast<quintptr>(readyp), 16);
//            SpLog::debug("timer:"+taskid+" "+cpadd+" "+ppadd+" "+ready->getOutput()->at(0));


        //}

    }
    updatemutex.unlock();

}

void TaskManage::readytopreprocess(QString name)
{
    emit preprocess(name);
}

void TaskManage::readytotrace(QString name)
{
    emit trace(name);
}

void TaskManage::readytopostprocess(QString name)
{
    emit postprocess(name);
}
