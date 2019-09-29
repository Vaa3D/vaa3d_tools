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
    setMainLayout();
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
    //menuWindow->addAction(ContentAction);
    menuWindow->addAction(dataTabAction);

    //help menu
    helpMenu = menuBar()->addMenu(tr("&Help"));
}
void MorphoHub_MainWindow::createToolBar()
{
    dbToolbar=this->addToolBar(tr("DB"));
    dbToolbar->addAction(NewDBAction);
    dbToolbar->addAction(SetDBAction);

//    funcsToolbar=this->addToolBar(tr("Funcs"));
//    funcsToolbar->addAction(ContentAction);
//    funcsToolbar->addAction(dataTabAction);

    levelControlToolbar=this->addToolBar(tr("LevelControl"));
    //commit,skip,rollback,assign,...functions
    levelControlToolbar->addAction(commitAction);
    levelControlToolbar->addAction(checkAction);
    levelControlToolbar->addAction(skipAction);
    levelControlToolbar->addAction(rollbackAction);
    levelControlToolbar->addAction(reassignAction);
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

//    ContentAction=new QAction(tr("WorkingSpace"),this);
//    connect(ContentAction,SIGNAL(triggered()),this,SLOT(createContentDockWindow()));
    //actions for management
    sdconfAction= new QAction(tr("&SDconfM"), this);
    sdconfAction->setStatusTip(tr("Source Data Management"));
    connect(sdconfAction,SIGNAL(triggered()),this,SLOT(sourceDataMAction()));
    annotatorconfAction= new QAction(tr("&AConfM"), this);
    annotatorconfAction->setToolTip(tr("Annotator Conf Management"));

    //actions for Functions
    //actions for levelcontrol
    commitAction= new QAction(tr("&Commit"), this);
    commitAction->setToolTip(tr("Commit one neuron to next level"));
    checkAction= new QAction(tr("&Check"), this);
    checkAction->setToolTip(tr("Check one neuron"));
    skipAction= new QAction(tr("&Skip"), this);
    skipAction->setToolTip(tr("Skip one neuron"));
    rollbackAction= new QAction(tr("&Rollback"), this);
    rollbackAction->setToolTip(tr("rollback one neuron"));
    reassignAction= new QAction(tr("&Reassign"), this);
    reassignAction->setToolTip(tr("reassign one neuron"));

    dataTabAction=new QAction(tr("DataTab"),this);
    connect(dataTabAction,SIGNAL(triggered()),this,SLOT(createDataTabDockWindow()));
}
void MorphoHub_MainWindow::setMainLayout()
{

//    LsplitterofM=new QSplitter(Qt::Horizontal,this);
    //QSplitter *RsplitterofM=new QSplitter(Qt::Horizontal,this);
//    mainlayout=new QGridLayout();
//    QWidget *dialogwideget;
//    mainlayout->addWidget(morphoHub_dialog,0,0);
////    dialogwideget->setGeometry(10,10,200,200);
//    dialogwideget->setLayout(mainlayout);

    createContentTreeWidget();
    createDataTabDockWindow();
    mainlayout->addWidget(contentTreewidget,2);
    mainlayout->addWidget(datawidget,5);
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
void MorphoHub_MainWindow::createDataTabDockWindow()
{
    //new Dock window
    datawidget=new QDockWidget("DataTab",this);
    datawidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    datawidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    //dataTabwidget->setWidget(contentlist);
    //new tab window
    dataTabwidget=new QTabWidget();
    dataTabwidget->setTabsClosable(true);
    datatable=createTableDataTotal();
    dataTabwidget->addTab(datatable,tr("ALL"));
    //set close function
//    ((QTabBar*)dataTabwidget->tabBar())->setTabButton(dataTabwidget->indexOf(datatable),QTabBar::RightSide,NULL);
    connect(dataTabwidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int)));
    //set tab widget
    datawidget->setWidget(dataTabwidget);
    //datawidget->setMinimumSize(50,50);
    //datawidget->setMaximumSize(1800,1600);
    //datawidget->setGeometry(180,0,150,300);
    this->addDockWidget(Qt::RightDockWidgetArea,datawidget);

}
void MorphoHub_MainWindow::removeSubTab(int subindex)
{
    dataTabwidget->removeTab(subindex);
}

QTableWidget* MorphoHub_MainWindow::createTableDataTotal()
{
    QStringList qsl;
    qsl << "BrainID"  << "NeuronID" << "Author" << "Checkerlist" << "LevelID" << "UpdateTime";
    int row = 4;
    int col = qsl.size();
    QTableWidget* t = new QTableWidget(row,col, this);
    t->setHorizontalHeaderLabels(qsl);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    return t;
}

void MorphoHub_MainWindow::contentValueChange(QTreeWidgetItem *item,int column)
{
    QTreeWidgetItem *itemparent=item->parent();
    if(itemparent==NULL)
        return;
    //int col=itemparent->indexOfChild(item);
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
QTableWidget* MorphoHub_MainWindow::createTableDataLevel(QList<ReconstructionInfo> levelres)
{
    QStringList title;
    title << "All"<<"BrainID"  << "NeuronID" << "Author" << "Checkerlist" << "LevelID"<<"Time";
    int col = title.size();
    int  row= levelres.size();
    QTableWidget* t;
    if(levelres.size()>0)
    {
        t= new QTableWidget(row,col, this);
        t->setHorizontalHeaderLabels(title);
        for(int i=0;i<levelres.size();i++)
        {
            ReconstructionInfo tmprecons=levelres.at(i);
            QTableWidgetItem *tmpcheckbox=new QTableWidgetItem();
            tmpcheckbox->setCheckState(Qt::Unchecked);
            //foreach item
            t->setItem(i,0,tmpcheckbox);
            t->setItem(i,1,new QTableWidgetItem(tmprecons.SdataID));
            t->setItem(i,2,new QTableWidgetItem(tmprecons.dataID));
            t->setItem(i,3,new QTableWidgetItem(tmprecons.author));
            t->setItem(i,4,new QTableWidgetItem(tmprecons.checkers));
            t->setItem(i,5,new QTableWidgetItem(tmprecons.levelID));
            t->setItem(i,6,new QTableWidgetItem(tmprecons.updateTime));
            //add item to tab.
        }
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        t->setSelectionMode(QAbstractItemView::SingleSelection);
        t->resizeColumnsToContents();
        t->resizeRowsToContents();
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
            tmprecons.dataID=splitAnoFilelist.at(1);
            tmprecons.levelID=levelid;
            int stampindex=splitAnoFilelist.indexOf("stamp");
            if(stampindex>2)
            {
                tmprecons.author=splitAnoFilelist.at(2);
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
void MorphoHub_MainWindow::MorphoHub_Init()
{
    dbpath="/home/penglab/Data/MorphoHub_DB";
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
    //morphoHub_dialog=new MainDialog(this->originparent);
    mainWidget=new QWidget(this);
    mainlayout=new QHBoxLayout();
    logtextedit=new QTextEdit(this);
    logtextedit->setText(tr("Welcome to MorphoHub."));
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
        sdconf_dialog=new SourceDataManagement(this->originparent);
        sdconf_dialog->setupDBpath(this->dbpath);
        sdconf_dialog->show();
        sdconf_dialog->setGeometry(50,50,800,600);
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
//showtext will be showed at bottom of the Window statu bar
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

MorphoHub_MainWindow::~MorphoHub_MainWindow()
{

}
MorphoHub_MainWindow::MorphoHub_MainWindow()
{
}
