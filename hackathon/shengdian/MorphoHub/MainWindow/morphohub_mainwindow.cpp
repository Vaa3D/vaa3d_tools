#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <iostream>
#include <algorithm>
#include <QtCore>
#include <QDir>
#include "morphohub_mainwindow.h"
#include <QSettings>
using namespace std;
MorphoHub_MainWindow::MorphoHub_MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    this->originparent=parent;
    setWindowTitle(tr("MorphoHub-DBMS"));
    MorphoHub_Init();
    createActions();
    createStatusBar();
    createToolBar();
    createMenus();
    createContentTreeWidget();
    createTabWindow();
    setMainLayout();
}
void MorphoHub_MainWindow::MorphoHub_Init()
{
    dbpath="/home/penglab/Data/MorphoHub_DB";
    datatitle <<"BrainID"  << "NeuronID" << "Author" << "Checkerlist" << "LevelID"<<"Time";
    DBBasicConf <<"BasicData"
                <<"WorkingSpace"
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
    workingspaceContentConf          <<"Assigned1"
                                     <<"L1A"
                                     <<"L1ACheck"
                                     <<"L1B"
                                     <<"L1C"
                                     <<"L1CCheck"
                                     <<"L1D"
                                     <<"Assigned2"
                                     <<"L2A"
                                     <<"L2ACheck"
                                     <<"L2B"
                                     <<"L2C"
                                     <<"L2CCheck"
                                     <<"L2D";
    initworkingspaceTablist <<"L1A"
                            <<"L1B"
                            <<"L1C"
                            <<"L2A"
                            <<"L2B"
                            <<"L2C";
    //morphoHub_dialog=new MainDialog(this->originparent);
    mainWidget=new QWidget(this);
    mainlayout=new QHBoxLayout();
    logtextedit=new QTextEdit(this);
    logtextedit->setText(tr("Welcome to MorphoHub."));
    //init of annotation protocol
    seuallenAP.ApConfPath=this->dbpath+"/Configuration/WorkingSpace_Conf/AnnotationProtocol.conf";
    InitofAnnotationProtocol();
    //init of annotator
    //need a sign in window for this.
    curOperator.UserID="JSD";
    curOperator.workingplace="SEU";
    curOperator.priority=APvisitor;
}
void MorphoHub_MainWindow::InitofAnnotationProtocol()
{
    QString confpath=seuallenAP.ApConfPath;
    if (confpath.isEmpty())
        return;
    QFile scanconffile(confpath);
    if(!scanconffile.exists())
    {
        int reply;
        reply=QMessageBox::warning(this,"File Not Found",QObject::tr("Can't find configuration file of Annotation protocol!\n")+
                                   QObject::tr("MorphoHub will create an initialized annotation protocol: %1?\n").arg(confpath),
                                   QMessageBox::Ok,QMessageBox::Cancel);
        if(reply==QMessageBox::Cancel)
            return;
        else
        {
            if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                //write head
                QString data="StartLevel_EndLevel";
                for(int i=0;i<seuallenAP.protocolLevel.size();i++)
                {
                    data=data+","+seuallenAP.protocolLevel.at(i);
                }
                data+="\n";
                scanconffile.write(data.toAscii());
                //write inside
                for(int i=0;i<seuallenAP.protocolLevel.size();i++)
                {
                    QString startlevel=seuallenAP.protocolLevel.at(i);
                    QString levelrules=startlevel;
                    for(int j=0;j<seuallenAP.protocolLevel.size();j++)
                    {
                        QString endlevel=seuallenAP.protocolLevel.at(j);
                        QString startlevel_endlevel=startlevel+"_"+endlevel;;
                        AnnotationProtocolFunction apfunc=seuallenAP.protocolrules.value(startlevel_endlevel);
                        int apfuncint=apfunc;
                        levelrules+=(","+(QString::number(apfuncint)));
                    }
                    qDebug()<<levelrules;
                    levelrules+="\n";
                    scanconffile.write(levelrules.toAscii());
                }
                scanconffile.close();
            }
        }
    }
    if(scanconffile.exists())
    {
        return;
    }
}

void MorphoHub_MainWindow::createActions()
{
    //action for database
    NewDBAction = new QAction(tr("&NewDB"), this);
    NewDBAction->setShortcuts(QKeySequence::New);
    NewDBAction->setStatusTip(tr("Setup a new DB"));
    connect(NewDBAction, SIGNAL(triggered()), this, SLOT(NewDB_slot()));

    SetDBAction = new QAction(tr("&LoadDB"), this);
    SetDBAction->setStatusTip(tr("load an exist DB"));
    connect(SetDBAction, SIGNAL(triggered()), this, SLOT(SetDB_slot()));

    //actions for management
    sdconfAction= new QAction(tr("&SoueceData"), this);
    sdconfAction->setStatusTip(tr("Source Data Management"));
    connect(sdconfAction,SIGNAL(triggered()),this,SLOT(sourceDataMAction()));
    annotatorconfAction= new QAction(tr("&Annotator"), this);
    annotatorconfAction->setToolTip(tr("Annotator Conf Management"));

    //actions for Functions
    //actions for levelcontrol
    commitAction= new QAction(tr("&Commit"), this);
    commitAction->setToolTip(tr("Commit one neuron to next level"));
    connect(commitAction,SIGNAL(triggered()),this,SLOT(commitAction_slot()));

    checkAction= new QAction(tr("&Check"), this);
    checkAction->setToolTip(tr("Check one neuron"));
    skipAction= new QAction(tr("&Skip"), this);
    skipAction->setToolTip(tr("Skip one neuron"));
    rollbackAction= new QAction(tr("&Rollback"), this);
    rollbackAction->setToolTip(tr("rollback one neuron"));
    reassignAction= new QAction(tr("&Reassign"), this);
    reassignAction->setToolTip(tr("reassign one neuron"));
}

void MorphoHub_MainWindow::createToolBar()
{
    dbToolbar=this->addToolBar(tr("DB"));
    dbToolbar->addAction(NewDBAction);
    dbToolbar->addAction(SetDBAction);

    levelControlToolbar=this->addToolBar(tr("LevelControl"));
    //commit,skip,rollback,assign,...functions
    levelControlToolbar->addAction(commitAction);
    levelControlToolbar->addAction(checkAction);
    levelControlToolbar->addAction(skipAction);
    levelControlToolbar->addAction(rollbackAction);
    levelControlToolbar->addAction(reassignAction);
}
void MorphoHub_MainWindow::createMenus()
{
    //file menu
    file = menuBar()->addMenu(tr("&Database"));
    file->addAction(NewDBAction);
    file->addAction(SetDBAction);

    //Management menu
    managementMenu=menuBar()->addMenu(tr("Management"));
    managementMenu->addAction(sdconfAction);
    managementMenu->addAction(annotatorconfAction);
    //functions menu
    funcs = menuBar()->addMenu(tr("&Functions"));

    //level control menu
    levelControlMenu=menuBar()->addMenu(tr("LevelControl"));
    levelControlMenu->addAction(commitAction);
    levelControlMenu->addAction(checkAction);
    levelControlMenu->addAction(skipAction);
    levelControlMenu->addAction(rollbackAction);
    levelControlMenu->addAction(reassignAction);

    //window menu
    menuWindow = menuBar()->addMenu(tr("&Window"));

    //help menu
    helpMenu = menuBar()->addMenu(tr("&Help"));
}
/**********************************************************/
/****************Bottom status bar*************************/
/**********************************************************/
void MorphoHub_MainWindow::createStatusBar()
{
    statusLabel=new QLabel("MorphoHub");
    statusLabel->setAlignment(Qt::AlignHCenter);
    statusLabel->setMinimumSize(statusLabel->sizeHint());
    statusBar()->addWidget(statusLabel);
}

void MorphoHub_MainWindow::setMainLayout()
{
    mainlayout->addWidget(contentTreewidget,2);
    mainlayout->addWidget(dataTabwidget,5);
    mainlayout->addWidget(logtextedit,3);
    mainWidget->setLayout(mainlayout);
    setCentralWidget(mainWidget);
}
void MorphoHub_MainWindow::createContentTreeWidget()
{
    contentTreewidget=new QTreeWidget(this);
    contentTreewidget->setColumnCount(1);
    contentTreewidget->setHeaderLabel(tr("Content"));
    //contentTreewidget->setGeometry(0,0,180,50);
    QList<QTreeWidgetItem*> contentitems;
    //create nodes
    content_workingspace=new QTreeWidgetItem(contentTreewidget,QStringList(QString("WorkingSpace")));
    content_basicData=new QTreeWidgetItem(contentTreewidget,QStringList(QString("BasicData")));
    contentitems.append(content_workingspace);
    contentitems.append(content_basicData);

    //create parent node
    contentTreewidget->insertTopLevelItems(0,contentitems);
    contentTreewidget->setItemsExpandable(false);
    contentTreewidget->expandAll();

    //create child node for WorkingSpace
    if(!dbpath.isEmpty())
    {
        QString workingspace_conf=dbpath+"/Configuration/WorkingSpace_Conf/workingspace_content.conf";
        QFile configurationfile(workingspace_conf);
        if(configurationfile.exists())
        {
            if(configurationfile.open(QIODevice::ReadWrite))
            {
                while(!configurationfile.atEnd())
                {
                    QByteArray cline=configurationfile.readLine();
                    QString tmpitem(cline);
                    tmpitem=tmpitem.simplified();
                    QTreeWidgetItem *content_workingspace_childnode=new QTreeWidgetItem(content_workingspace,QStringList(tmpitem));
                    content_workingspace->addChild(content_workingspace_childnode);
                }
                configurationfile.close();
            }
        }
        else
        {
            //if not exists, give a warning about this
            QMessageBox::warning(this,"Workingspace Conf Warning","Can't find Configuration file of basic database!");
        }
    }
    connect(contentTreewidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(contentValueChange(QTreeWidgetItem*,int)));
}
void MorphoHub_MainWindow::createTabWindow()
{
    //new tab window
    dataTabwidget=new QTabWidget();
    dataTabwidget->setTabsClosable(false);
    dataTabwidget->setMovable(false);
    for(int i=0;i<initworkingspaceTablist.size();i++)
    {
        QString initlevel=initworkingspaceTablist.at(i);
        //get reconstructions info from this level
        QList<ReconstructionInfo> thislevelres;
        thislevelres=getReconstuctionsFromLevel(initlevel);
        if(thislevelres.size()>0)
        {
            QTableWidget* levelTable=new QTableWidget();
            levelTable=createTableDataLevel(thislevelres);
            if(levelTable)
            {
                datatablelist.append(levelTable);
                dataTabwidget->addTab(levelTable,initlevel);
            }
        }
    }

    //set close function
    connect(dataTabwidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int)));
    //set tab widget
    connect(dataTabwidget,SIGNAL(currentChanged(int)),this,SLOT(dataTabChange(int)));
}

void MorphoHub_MainWindow::contentValueChange(QTreeWidgetItem *item,int column)
{
    QTreeWidgetItem *itemparent=item->parent();
    if(itemparent==NULL)
        return;
    QString itemtext=item->text(column);
    //get reconstructions info from this level
    QList<ReconstructionInfo> thislevelres;
    thislevelres=getReconstuctionsFromLevel(itemtext);
    if(thislevelres.size()>0)
    {
        QTableWidget* levelTable=new QTableWidget();
        levelTable=createTableDataLevel(thislevelres);
        if(levelTable)
            dataTabwidget->addTab(levelTable,itemtext);
    }
}
void MorphoHub_MainWindow::celltableInfoUpdate(int row, int column)
{
    if(row>=0)
    {
        int curtabindex=dataTabwidget->currentIndex();
        QTableWidget *levelTable=datatablelist.at(curtabindex);
        if(levelTable!=NULL)
        {
            curRecon.SdataID=levelTable->item(row,0)->text();
            curRecon.SomaID=levelTable->item(row,1)->text();
            curRecon.author.UserID=levelTable->item(row,2)->text();
            curRecon.checkers=levelTable->item(row,3)->text();
            curRecon.levelID=levelTable->item(row,4)->text();
            curRecon.updateTime=levelTable->item(row,5)->text();

            if(false)
            {
                qDebug()<<"?";
                commitDialog->setAnnotator(curOperator);//
                commitDialog->setCurNeuron(curRecon);//get current neuron info
                commitDialog->setFunction("Commit");
                commitDialog->updateMainView();
            }
        }
    }
}

QTableWidget* MorphoHub_MainWindow::createTableDataLevel(QList<ReconstructionInfo> levelres)
{
    int col = datatitle.size();
    int row= levelres.size();
    QTableWidget* t;
    if(levelres.size()>0)
    {
        t= new QTableWidget(row,col, this);
        t->setHorizontalHeaderLabels(datatitle);
        for(int i=0;i<levelres.size();i++)
        {
            ReconstructionInfo tmprecons=levelres.at(i);
            t->setItem(i,0,new QTableWidgetItem(tmprecons.SdataID));
            t->setItem(i,1,new QTableWidgetItem(tmprecons.SomaID));
            t->setItem(i,2,new QTableWidgetItem(tmprecons.author.UserID));
            t->setItem(i,3,new QTableWidgetItem(tmprecons.checkers));
            t->setItem(i,4,new QTableWidgetItem(tmprecons.levelID));
            t->setItem(i,5,new QTableWidgetItem(tmprecons.updateTime));
            //add item to tab.
        }
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        t->setSelectionMode(QAbstractItemView::SingleSelection);
        t->resizeColumnsToContents();
        t->resizeRowsToContents();
        //get table cell info when clicked.
        connect(t,SIGNAL(cellClicked(int,int)),this,SLOT(celltableInfoUpdate(int,int)));
    }
    return t;
}
QList<ReconstructionInfo> MorphoHub_MainWindow::getReconstuctionsFromLevel(const QString& levelid)
{
    QList<ReconstructionInfo> outlist;
    outlist.clear();
    //1.scan Workingspace level path.
    QString scandstpath=this->dbpath+"/WorkingSpace/"+levelid;
    QDir scanReconsdir(scandstpath);
    if(!scanReconsdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Can't find this level at WorkingSpace!");
    }
    //2.get all the fatherdirname of the reconstructions
    scanReconsdir.setFilter(QDir::NoDotAndDotDot|QDir::AllDirs);
    QFileInfoList redirlist=scanReconsdir.entryInfoList();
    int scanindex=0;
    if(redirlist.size()>0)
    {
        while(scanindex<redirlist.size())
        {
            QFileInfo thisdir=redirlist.at(scanindex);
            QString thisdirname=thisdir.fileName();
            QDir dirinside(thisdir.filePath());
            QStringList swcnameFilter;
            swcnameFilter<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
            QStringList swcfilelist=dirinside.entryList(swcnameFilter,QDir::Files|QDir::NoDotAndDotDot,QDir::Name);
            QStringList anonameFilter;
            anonameFilter<<"*.ano"<<"*.ANO";
            QStringList anofilelist=dirinside.entryList(anonameFilter,QDir::Files|QDir::NoDotAndDotDot,QDir::Name);
            QStringList aponameFilter;
            aponameFilter<<"*.apo"<<"*.APO";
            QStringList apofilelist=dirinside.entryList(aponameFilter,QDir::Files|QDir::NoDotAndDotDot,QDir::Name);
            if(swcfilelist.size()!=1||anofilelist.size()!=1||apofilelist.size()!=1)
            {
                QMessageBox::warning(this,"File Error","Abnormal file found at"+thisdirname);
                continue;
            }
            QString anofilebasename=QFileInfo(dirinside,anofilelist.at(0)).completeBaseName();
            QStringList splitAnoFilelist=anofilebasename.split("_");
            if(splitAnoFilelist.size()<9)
            {
                QMessageBox::warning(this,"Name Format Error","Abnormal file name format found at"+thisdirname);
                continue;
            }
            ReconstructionInfo tmprecons;
            tmprecons.SdataID=splitAnoFilelist.at(0);
            tmprecons.SomaID=splitAnoFilelist.at(1);
            tmprecons.levelID=levelid;
            int stampindex=splitAnoFilelist.indexOf("stamp");
            if(stampindex>2)
            {
                tmprecons.author.UserID=splitAnoFilelist.at(2);
                for(int i=3;i<stampindex;i++)
                {
                    if(i==3)
                        tmprecons.checkers=splitAnoFilelist.at(i);
                    else
                        tmprecons.checkers=tmprecons.checkers+"&"+splitAnoFilelist.at(i);
                }
                for(int i=stampindex+1;i<splitAnoFilelist.size();i++)
                {
                    if(i==(stampindex+1))
                        tmprecons.updateTime=splitAnoFilelist.at(i);
                    else
                        tmprecons.updateTime=tmprecons.updateTime+"-"+splitAnoFilelist.at(i);
                }
            }

            outlist.append(tmprecons);
            scanindex++;
        }
    }
    //3.write to Qlist
    return outlist;
}

/*source data management:
    This is a dialog
*/
void MorphoHub_MainWindow::sourceDataMAction()
{
    QDir dbdir(this->dbpath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        sdconf_dialog=new SourceDataManagement(this->dbpath,this->originparent);
        //sdconf_dialog->setupDBpath();
        sdconf_dialog->show();
        sdconf_dialog->setMinimumSize(600,400);
        sdconf_dialog->setMaximumSize(1200,1000);
        sdconf_dialog->setGeometry(50,50,1000,800);
        this->raise();
    }
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
        updateStatusBar(dbpath);
        //make new dir for basic db
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
        //make new dir for workingspace
        for(int i=0;i<workingspaceConf.size();i++)
        {
            QString thisfolderpath=dbpath+"/WorkingSpace/"+workingspaceConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
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
        //create a configuration file for WorkingSpace Content
        QString basic_workingspace_content_conf=dbpath+"/Configuration/WorkingSpace_Conf/workingspace_content.conf";
        QFile wcconfigurationfile(basic_workingspace_content_conf);
        if(!wcconfigurationfile.exists())
        {
            if(wcconfigurationfile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                for(int i=0;i<workingspaceContentConf.size();i++)
                {
                    QString data=workingspaceContentConf[i]+"\n";
                    wcconfigurationfile.write(data.toAscii());
                }
                wcconfigurationfile.close();
            }
        }
    }
}
//need to be revised
void MorphoHub_MainWindow::SetDB_slot()
{
    QString title="please select a path for DB";
    this->dbpath = QFileDialog::getExistingDirectory(this, title,
                                                     "~/",
                                                     QFileDialog::ShowDirsOnly
                                                     |QFileDialog::DontResolveSymlinks);
    if(!dbpath.isEmpty())
    {
        QString basic_db_conf=dbpath+"/Configuration/DB_Basic.conf";
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

void MorphoHub_MainWindow::updateStatusBar(const QString& showtext)
{
    statusLabel->setText(showtext);
}
void MorphoHub_MainWindow::toLogWindow(const QString &logtext)
{
    QString getlogtext=logtextedit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logtextedit->setText(showText);
}
void MorphoHub_MainWindow::removeSubTab(int subindex)
{
    dataTabwidget->removeTab(subindex);
}
void MorphoHub_MainWindow::dataTabChange(int tabindex)
{

}

MorphoHub_MainWindow::~MorphoHub_MainWindow()
{

}
MorphoHub_MainWindow::MorphoHub_MainWindow()
{
}

/**********************************************************/
/****************Protocol functions*************************/
/********Commit,Check,Skip,Rollback,Reassign,Release*****/

void MorphoHub_MainWindow::commitAction_slot()
{
    //
    QDir dbdir(this->dbpath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        commitDialog=new MainDialog(this->dbpath,this->originparent);
        commitDialog->setAnnotator(curOperator);//
        commitDialog->setCurNeuron(curRecon);//get current neuron info
        commitDialog->setFunction("Commit");
        commitDialog->setupAnnotationProtocol(seuallenAP);
        commitDialog->updateMainView();

        commitDialog->show();
        commitDialog->setMinimumSize(400,300);
        commitDialog->setMaximumSize(800,800);
        commitDialog->setGeometry(100,100,600,400);
        this->raise();
    }
}
