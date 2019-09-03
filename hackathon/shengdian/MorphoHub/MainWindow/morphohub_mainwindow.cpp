#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <iostream>
#include <QtCore>
#include <QDir>
#include "morphohub_mainwindow.h"
#include <QSettings>
using namespace std;
MorphoHub_MainWindow::MorphoHub_MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(tr("MorphoHub-DBMS"));
    createActions();
    createMenus();
    MorphoHub_Init();
    setMainLayout();
}

void MorphoHub_MainWindow::createMenus()
{
    //file menu
    QMenu *file = menuBar()->addMenu(tr("&Database"));
    file->addAction(NewDBAction);
    file->addAction(SetDBAction);

    //service menu
    QMenu *funcs = menuBar()->addMenu(tr("&Functions"));
//    funcs->addAction(AnnotationSAction);
//    funcs->addAction(DBMSAction);

    //window menu
    QMenu *menuWindow = menuBar()->addMenu(tr("&Window"));

    //help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
}
void MorphoHub_MainWindow::createActions()
{
    //action for database
    NewDBAction = new QAction(tr("&New DB"), this);
    NewDBAction->setShortcuts(QKeySequence::New);
    NewDBAction->setStatusTip(tr("Setup a new DB"));
    connect(NewDBAction, SIGNAL(triggered()), this, SLOT(NewDB_slot()));

    SetDBAction = new QAction(tr("&Load DB"), this);
    SetDBAction->setStatusTip(tr("load an exist DB"));
    connect(SetDBAction, SIGNAL(triggered()), this, SLOT(SetDB_slot()));
}
void MorphoHub_MainWindow::setMainLayout()
{
    QGridLayout *gridlayouttest=new QGridLayout(this);
}
void MorphoHub_MainWindow::MorphoHub_Init()
{
    DBBasicConf <<"BasicData"
                <<"WorkingSpace"
                    <<"WorkingSpace/Assigned1"
                        <<"WorkingSpace/Assigned1/Annotator"
                        <<"WorkingSpace/Assigned1/Priority"
                    <<"WorkingSpace/L1A"
                    <<"WorkingSpace/L1ACheck"
                    <<"WorkingSpace/L1B"
                    <<"WorkingSpace/L1C"
                    <<"WorkingSpace/L1CCheck"
                    <<"WorkingSpace/L1D"
                    <<"WorkingSpace/Assigned2"
                        <<"WorkingSpace/Assigned2/Annotator"
                        <<"WorkingSpace/Assigned2/Priority"
                    <<"WorkingSpace/L2A"
                    <<"WorkingSpace/L2ACheck"
                    <<"WorkingSpace/L2B"
                    <<"WorkingSpace/L2C"
                    <<"WorkingSpace/L2CCheck"
                    <<"WorkingSpace/L2D"
                    <<"WorkingSpace/QuestionZone"
                        <<"WorkingSpace/QuestionZone/tmp"
                <<"Finished"
                    <<"Finished/L1"
                    <<"Finished/L2"
                <<"Configuration"
                    <<"Configuration/BasicData_Conf"
                    <<"Configuration/WorkingSpace_Conf";
    workingspaceConf               <<"Assigned1"
                                       <<"Assigned1/Annotator"
                                       <<"Assigned1/Priority"
                                   <<"L1A"
                                   <<"L1ACheck"
                                   <<"L1B"
                                   <<"L1C"
                                   <<"L1CCheck"
                                   <<"L1D"
                                   <<"Assigned2"
                                       <<"Assigned2/Annotator"
                                       <<"Assigned2/Priority"
                                   <<"L2A"
                                   <<"L2ACheck"
                                   <<"L2B"
                                   <<"L2C"
                                   <<"L2CCheck"
                                   <<"L2D"
                                   <<"QuestionZone"
                                       <<"QuestionZone/tmp";
}

void MorphoHub_MainWindow::NewDB_slot()
{
    //QMessageBox::information(this, tr("Information"), tr("Open"));
    QString title="please select a path for DB";
    this->dbpath = QFileDialog::getExistingDirectory(this, title,
                                                     "~/",
                                                     QFileDialog::ShowDirsOnly
                                                     |QFileDialog::DontResolveSymlinks);
    if(!dbpath.isEmpty())
    {
        qDebug()<<dbpath<<endl;
        //make new dir
        for(int i=0;i<DBBasicConf.size();i++)
        {
            QString thisfolderpath=dbpath+"/"+DBBasicConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
            }
        }
        //create a configuration file for recording the basic folder
        QString basic_db_conf=dbpath+"/Configuration/DB_Basic.conf";
        QFile configurationfile(basic_db_conf);
        if(!configurationfile.exists())
        {
            if(configurationfile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                for(int i=0;i<DBBasicConf.size();i++)
                {
                    QString data=DBBasicConf[i]+"\n";
                    configurationfile.write(data.toAscii());
                }
                configurationfile.close();
            }
        }
        //create a configuration file for WorkingSpace folder
        QString basic_workingspace_conf=dbpath+"/Configuration/WorkingSpace_Conf/workingspace.conf";
        QFile wconfigurationfile(basic_workingspace_conf);
        if(!wconfigurationfile.exists())
        {
            if(wconfigurationfile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                for(int i=0;i<workingspaceConf.size();i++)
                {
                    QString data=workingspaceConf[i]+"\n";
                    wconfigurationfile.write(data.toAscii());
                }
                wconfigurationfile.close();
            }
        }
    }
}
void MorphoHub_MainWindow::SetDB_slot()
{
    QString title="please select a path for DB";
    this->dbpath = QFileDialog::getExistingDirectory(this, title,
                                                     "~/",
                                                     QFileDialog::ShowDirsOnly
                                                     |QFileDialog::DontResolveSymlinks);
    if(!dbpath.isEmpty())
    {
        qDebug()<<dbpath<<endl;

        QString basic_db_conf=dbpath+"/Configuration/DB_Basic.conf";
//        QString brainconf=dbpath+"/DBMS/brain.conf";
//        readBrainConf(brainconf);
        //bool okay=isFileExist(basic_db_conf);
        QFile configurationfile(basic_db_conf);
        if(configurationfile.exists())
        {
            if(configurationfile.open(QIODevice::ReadWrite))
            {
                QString tmp=configurationfile.readAll();
                configurationfile.close();
            }
        }
        else
        {
            //if not exists, give a warning about this
            QMessageBox::warning(this,"Conf Warning","Can't find Configuration file of basic database!");
        }
    }
}
MorphoHub_MainWindow::~MorphoHub_MainWindow()
{

}
MorphoHub_MainWindow::MorphoHub_MainWindow()
{
}
