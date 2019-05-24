#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <iostream>
#include <QtCore>
#include <QDir>
#include "morphohub_mainwindow.h"
#include "DBMS/dbms_basic.h"
using namespace std;

MorphoHub_MainWindow::MorphoHub_MainWindow(/*V3DPluginCallback2 *callback, */QWidget *parent)
    :QMainWindow(parent)
{
    this->originparent=parent;
    setWindowTitle(tr("MorphoHub"));
    createActions();
    createServices();
    createMenus();
    setMainLayout();
    //QToolBar *toolBar = addToolBar(tr("&File"));
    //toolBar->addAction(NewDBAction);

    //statusBar() ;
}
void MorphoHub_MainWindow::createActions()
{
    //action for new a database
    NewDBAction = new QAction(tr("&New DB"), this);
    NewDBAction->setShortcuts(QKeySequence::New);
    NewDBAction->setStatusTip(tr("Setup a new DB"));
    connect(NewDBAction, SIGNAL(triggered()), this, SLOT(NewDB_slot()));

    SetDBAction = new QAction(tr("&Load DB"), this);
    SetDBAction->setStatusTip(tr("load an exist DB"));
    connect(SetDBAction, SIGNAL(triggered()), this, SLOT(SetDB_slot()));

}
void MorphoHub_MainWindow::createServices()
{
    //services list
    createAnnotationService();
    createDBMSService();
}
void MorphoHub_MainWindow::createAnnotationService()
{
    AnnotationSAction = new QAction(tr("&AnnotationService"), this);
    AnnotationSAction->setShortcut(tr("Ctrl+A"));
    AnnotationSAction->setStatusTip(tr("Annotation"));
    connect(AnnotationSAction, SIGNAL(triggered()), this, SLOT(Annotation_slot()));
}
void MorphoHub_MainWindow::createDBMSService()
{
    DBMSAction = new QAction(tr("&DBMSService"), this);
    DBMSAction->setShortcut(tr("Ctrl+D"));
    DBMSAction->setStatusTip(tr("DBManagementSystem"));
    connect(DBMSAction, SIGNAL(triggered()), this, SLOT(DBMS_slot()));
}

void MorphoHub_MainWindow::createMenus()
{
    //file menu
    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(NewDBAction);
    file->addAction(SetDBAction);

    //service menu
    QMenu *services = menuBar()->addMenu(tr("&Services"));
    services->addAction(AnnotationSAction);
    services->addAction(DBMSAction);

    //window menu
    QMenu *menuWindow = menuBar()->addMenu(tr("&Window"));

    //help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MorphoHub_MainWindow::setMainLayout()
{
    logwideget=new QWidget;

    //setcentralWidget(logwideget);
    QGridLayout *gridlayouttest=new QGridLayout(this);

    //this->setLayout(gridlayout);
    logtext=new QTextEdit();

    gridlayouttest->addWidget(logtext,0,1);

    logwideget->setGeometry(10,10,200,200);
    logwideget->setLayout(gridlayouttest);
    setCentralWidget(logwideget);
    //logwideget->show();

}

MorphoHub_MainWindow::~MorphoHub_MainWindow()
{

}
MorphoHub_MainWindow::MorphoHub_MainWindow()
{
}
void MorphoHub_MainWindow::Annotation_slot()
{
    //annotation qdialog
}
void MorphoHub_MainWindow::DBMS_slot()
{
    //DB management system
    //manage brain,neuron,user data
    dbmsdialog=new DBMSDialog(this->originparent);
    dbmsdialog->show();
    dbmsdialog->setGeometry(50,50,1200,800);
    this->raise();
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
        //TeraBrainDepository,MorphoDepository,ServiceDepository,DBMS
        QStringList basic_name;
        //basic_name+="/TeraBrainDepository";
        basic_name+="TeraBrainDepository";
        basic_name+="MorphoDepository";
        basic_name+="ServiceDepository";
        basic_name+="DBMS";
        for(int i=0;i<basic_name.size();i++)
        {
            QString thisfolderpath=dbpath+"/"+basic_name[i];
            QString getlogtext=logtext->toPlainText();
            QString showText=getlogtext+"\n"+thisfolderpath;
            logtext->setText(showText);
            qDebug()<<thisfolderpath;
            makeDir(thisfolderpath);
        }
        //create a configuration file for recording the basic folder
//        QString brain_conf=dbpath+"/DBMS/brain.conf";
//        writeBrainConf(brain_conf);
        QString basic_db_conf=dbpath+"/DBMS/basic_db_folderlist.conf";
        bool okay=isFileExist(basic_db_conf);
        if(!okay)
        {
            QFile configurationfile(basic_db_conf);
            if(configurationfile.open(QIODevice::ReadWrite))
            {
                for(int i=0;i<basic_name.size();i++)
                {
                    QString data=basic_name[i]+"\n";
                    configurationfile.write(data.toAscii());
                }
                configurationfile.close();
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

        QString basic_db_conf=dbpath+"/DBMS/basic_db_folderlist.conf";
//        QString brainconf=dbpath+"/DBMS/brain.conf";
//        readBrainConf(brainconf);
        bool okay=isFileExist(basic_db_conf);
        if(okay)
        {
            QFile configurationfile(basic_db_conf);
            if(configurationfile.open(QIODevice::ReadWrite))
            {
                QString tmp=configurationfile.readAll();
                configurationfile.close();
                QString getlogtext=logtext->toPlainText();
                QString showText=getlogtext+"\n"+tmp;
                logtext->setText(showText);
            }
        }
        else
        {
            //if not exists, give a warning about this
        }
    }
}

