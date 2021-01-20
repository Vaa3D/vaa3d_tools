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
MorphoHub_MainWindow::MorphoHub_MainWindow(V3DPluginCallback2 &callback,QWidget *parent) :
    QMainWindow(parent)
{
    this->originparent=parent;
    MorphoHubcallback=&callback;
    setWindowTitle(tr("MorphoHub-DBMS"));
    MorphoHub_Init();
    createActions();
    createStatusBar();
    createToolBar();
    createMenus();
    createContentTreeWidget(true);
    createTabWindow(true);
    setMainLayout();
    if(this->dbpath.isEmpty())
    {
        //remind user to setup the dbpath
        //QMessageBox::warning(this,"Initializtion","Please set the database path!");
        toLogWindow("Warning: Please set the database path!");
    }
    else
    {
        QDir::setCurrent(dbpath);
        QDir dir(QDir::currentPath());
        dbpath=dir.absolutePath();
        qDebug()<<"path is : "<<dbpath;
        toLogWindow(tr("Load Database: %1").arg(dbpath));
        createTabWindow(false);
        updateStatusBar(tr("Database: %1").arg(dbpath));
    }
    if(curOperator.UserID.isEmpty())
    {
        loginAction->setEnabled(true);
        logoutAction->setEnabled(false);
        setProtocolFunctionEnabled(false);
        toLogWindow("Warning: You have to sign in to get the advanced functions.");
        userStatusLabel->setText("UserID: Nobody");
    }
    else
    {
        loginAction->setEnabled(false);
        logoutAction->setEnabled(true);
        toLogWindow(tr("Current UserID: %1").arg(curOperator.UserID));
        setProtocolFunctionEnabled(true);
        userStatusLabel->setText(tr("UserID: %1").arg(curOperator.UserID));
    }

}

void MorphoHub_MainWindow::MorphoHub_Init()
{
    initworkingspaceTablist=seuallenAP.architechure.initworkingspaceTablist;
    QSettings settings("MorphoHub","Vaa3d");
    //read and update settings
    QStringList setinitworkingspaceTablist=settings.value("InitTab").toStringList();
    if(setinitworkingspaceTablist.size()==0)
    {
        qDebug()<<"No init setting";
//        initworkingspaceTablist <<"L1A"<<"L1B"<<"L1C"<<"L2A"<<"L2B"<<"L2C";
        settings.setValue("InitTab",initworkingspaceTablist);
    }
    else if(setinitworkingspaceTablist.size()==1
            &&setinitworkingspaceTablist.at(0).isEmpty())
    {
        qDebug()<<"settings empty";
//        initworkingspaceTablist <<"L1A"<<"L1B"<<"L1C"<<"L2A"<<"L2B"<<"L2C";
        settings.setValue("InitTab",initworkingspaceTablist);
    }
    else
        initworkingspaceTablist=setinitworkingspaceTablist;
    dbpath=settings.value("dbpath").toString();
    curOperator.UserID=settings.value("UserID").toString();

    datatitle=seuallenAP.ReconstructionConfItems;
    DBBasicConf=seuallenAP.architechure.originForder;
    workingspacelevellist=seuallenAP.architechure.workingSpace;
    for(int i=0;i<workingspacelevellist.size();i++)
    {
        QString thisname="/"+workingspacelevellist.at(i).ParentDir+"/"+workingspacelevellist.at(i).Name;
        workingspaceConf.append(thisname);
        if(workingspacelevellist.at(i).childlist.size()>0)
        {
            for(int j=0;j<workingspacelevellist.at(i).childlist.size();j++)
            {
                workingspaceConf.append(thisname+"/"+workingspacelevellist.at(i).childlist.at(j));
            }
        }

    }
    finishedlevellist=seuallenAP.architechure.finished;
    for(int i=0;i<finishedlevellist.size();i++)
    {
        QString thisname="/"+finishedlevellist.at(i).ParentDir+"/"+finishedlevellist.at(i).Name;
        finishedConf.append(thisname);
    }
    //init of annotation protocol
    //seuallenAP.ApConfPath=this->dbpath+"/Configuration/WorkingSpace_Conf/AnnotationProtocol.conf";
    //InitofAnnotationProtocol();
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

    SettingAction = new QAction(tr("&Settings"), this);
    SettingAction->setStatusTip(tr("Default settings"));
    connect(SettingAction, SIGNAL(triggered()), this, SLOT(SettingAction_slot()));

    //actions for management
    sdconfAction= new QAction(tr("&SoueceData"), this);
    sdconfAction->setStatusTip(tr("Source Data Management"));
    connect(sdconfAction,SIGNAL(triggered()),this,SLOT(sourceDataMAction()));
    userManagementAction= new QAction(tr("&User"), this);
    userManagementAction->setToolTip(tr("User Management"));
    connect(userManagementAction,SIGNAL(triggered()),this,SLOT(userManagementAction_slot()));

    //actions for services
    visualizationAction=new QAction(tr("Screenwall"),this);
    visualizationAction->setToolTip(tr("Visualiztion of the workflow"));
    connect(visualizationAction,SIGNAL(triggered()),this,SLOT(visualizationAction_slot()));
//    QAction *qualityControlAction;
//    QAction *registrationAction;
//    QAction *releaseAction;
    errorCheckAction=new QAction(tr("ErrorCheck"),this);
    errorCheckAction->setToolTip(tr("Check the error of the Workingspace."));
    connect(errorCheckAction,SIGNAL(triggered()),this,SLOT(errorCheckAction_slot()));
    //actions for levelcontrol
    commitAction= new QAction(tr("&Commit"), this);
    commitAction->setToolTip(tr("Commit one neuron to next level"));
    connect(commitAction,SIGNAL(triggered()),this,SLOT(commitAction_slot()));

    checkAction= new QAction(tr("&Check"), this);
    checkAction->setToolTip(tr("Check one neuron"));
    connect(checkAction,SIGNAL(triggered()),this,SLOT(checkAction_slot()));

    skipAction= new QAction(tr("&Skip"), this);
    skipAction->setToolTip(tr("Skip one neuron"));
    connect(skipAction,SIGNAL(triggered()),this,SLOT(skipAction_slot()));

    rollbackAction= new QAction(tr("&Rollback"), this);
    rollbackAction->setToolTip(tr("rollback one neuron"));
    connect(rollbackAction,SIGNAL(triggered()),this,SLOT(rollbackAction_slot()));

    reassignAction= new QAction(tr("&Reassign"), this);
    reassignAction->setToolTip(tr("reassign one neuron"));
    connect(reassignAction,SIGNAL(triggered()),this,SLOT(reassignAction_slot()));    

    //Actions for window
    loginAction=new QAction(tr("&Login"),this);
    loginAction->setToolTip(tr("You have to sign in to get the advanced functions."));
    loginAction->setEnabled(true);
    connect(loginAction,SIGNAL(triggered()),this,SLOT(loginAction_slot()));

    logoutAction=new QAction(tr("&Logout"),this);
    logoutAction->setEnabled(false);
    connect(logoutAction,SIGNAL(triggered()),this,SLOT(logoutAction_slot()));

    //Action for Help
    helpAction=new QAction(tr("&Help"),this);
    helpAction->setToolTip(tr("Get the help from this."));
    connect(helpAction,SIGNAL(triggered()),this,SLOT(helpAction_slot()));

    //Action for data tab
    //set content menu
    popAction_3Dview=new QAction("See In 3D View",this);
    connect(popAction_3Dview,SIGNAL(triggered()),this,SLOT(popAction_3Dview_slot()));
}

void MorphoHub_MainWindow::setProtocolFunctionEnabled(bool en)
{
    sdconfAction->setEnabled(en);
    userManagementAction->setEnabled(en);
    commitAction->setEnabled(en);
    checkAction->setEnabled(en);
    skipAction->setEnabled(en);
    rollbackAction->setEnabled(en);
    reassignAction->setEnabled(en);
    //releaseAction->setEnabled(en);
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

    loginToolbar=this->addToolBar(tr("Login"));
    loginToolbar->addAction(loginAction);
    loginToolbar->addAction(logoutAction);

    servicesToolbar=this->addToolBar(tr("Services"));
    servicesToolbar->addAction(errorCheckAction);
}

void MorphoHub_MainWindow::createMenus()
{
    //file menu
    file = menuBar()->addMenu(tr("&Database"));
    file->addAction(NewDBAction);
    file->addAction(SetDBAction);
    file->addAction(SettingAction);

    //Management menu
    managementMenu=menuBar()->addMenu(tr("Management"));
    managementMenu->addAction(sdconfAction);
    managementMenu->addAction(userManagementAction);
    //services menu
    servicesMenu = menuBar()->addMenu(tr("&Services"));
    servicesMenu->addAction(visualizationAction);
    servicesMenu->addAction(errorCheckAction);

    //level control menu
    levelControlMenu=menuBar()->addMenu(tr("LevelControl"));
    levelControlMenu->addAction(commitAction);
    levelControlMenu->addAction(checkAction);
    levelControlMenu->addAction(skipAction);
    levelControlMenu->addAction(rollbackAction);
    levelControlMenu->addAction(reassignAction);

    //window menu
    menuWindow = menuBar()->addMenu(tr("&Window"));
    menuWindow->addAction(loginAction);
    menuWindow->addAction(logoutAction);
    //help menu
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpAction);

    //pop menu
    popMenuFordataTab=new QMenu();

}
/**********************************************************/
/****************Bottom status bar*************************/
/**********************************************************/

void MorphoHub_MainWindow::createStatusBar()
{
    statusLabel=new QLabel("MorphoHub");
    statusLabel->setAlignment(Qt::AlignHCenter);
    statusLabel->setMinimumSize(statusLabel->sizeHint());

    userStatusLabel=new QLabel("MorphoHub");
    userStatusLabel->setAlignment(Qt::AlignRight);
    userStatusLabel->setMinimumSize(userStatusLabel->sizeHint());
    statusBar()->addWidget(statusLabel);
    statusBar()->addWidget(userStatusLabel,1);
}


void MorphoHub_MainWindow::setMainLayout()
{
    mainWidget=new QWidget(this);
    mainlayout=new QHBoxLayout();
    logtextedit=new QTextEdit(this);
    logtextedit->setText(tr("Welcome to MorphoHub."));
    MainLogwidget=new QDockWidget("Main Log");
    MainLogwidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    MainLogwidget->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
    MainLogwidget->setWidget(logtextedit);
    this->addDockWidget(Qt::BottomDockWidgetArea,MainLogwidget);

    mainlayout->addWidget(contentTreewidget,1);
    mainlayout->addWidget(dataTabwidget,6);
    //mainlayout->addWidget(MainLogwidget,2);
    mainWidget->setLayout(mainlayout);
    setCentralWidget(mainWidget);    
}

void MorphoHub_MainWindow::createContentTreeWidget(bool init)
{
    if(init)
    {
        contentTreewidget=new QTreeWidget(this);
        contentTreewidget->setColumnCount(1);
        contentTreewidget->setHeaderLabel(tr("Content"));
        //contentTreewidget->setGeometry(0,0,180,50);
        QList<QTreeWidgetItem*> contentitems;
        //create nodes
        content_workingspace=new QTreeWidgetItem(contentTreewidget,QStringList(QString("WorkingSpace")));
        //content_basicData=new QTreeWidgetItem(contentTreewidget,QStringList(QString("Brain")));
        contentitems.append(content_workingspace);
        //contentitems.append(content_basicData);

        QList<Annotationlevel> workingspacelevellisttmp=seuallenAP.architechure.workingSpace;
        for(int i=0;i<workingspacelevellisttmp.size();i++)
        {
            QString thisname=workingspacelevellist.at(i).Name;
            QString tmpitem;
            tmpitem=thisname.simplified();
            QTreeWidgetItem *content_workingspace_childnode=new QTreeWidgetItem(content_workingspace,QStringList(tmpitem));
            content_workingspace->addChild(content_workingspace_childnode);
        }
        //create parent node
        contentTreewidget->insertTopLevelItems(0,contentitems);
        contentTreewidget->setItemsExpandable(true);
        contentTreewidget->expandAll();
        connect(contentTreewidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(contentValueChange(QTreeWidgetItem*,int)));
    }
}
void MorphoHub_MainWindow::createTabWindow(bool init)
{
    if(init)
    {
        //new tab window
        dataTabwidget=new QTabWidget();
        dataTabwidget->setTabsClosable(false);
        dataTabwidget->setMovable(false);
    }
    else
    {
        //clear old state
//        dataTabwidget->clear();
        if(dataTabwidget->count()>0)
        {
            //set tab widget
            dataTabwidget->disconnect();
            //disconnect(dataTabwidget,SIGNAL(currentChanged(int)),this,SLOT(dataTabChange(int)));
            qDebug()<<"remove tab";
            dataTabwidget->clear();
        }
        datatablelist.clear();
        datatabletitlelist.clear();
        //update
        qDebug()<<"size= "<<initworkingspaceTablist.size();
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
                    qDebug()<<"create Tab: "<<initlevel;
                    datatabletitlelist.append(initlevel);
                    datatablelist.append(levelTable);
                    dataTabwidget->addTab(levelTable,initlevel);
                }
            }
        }
        //set tab widget
        connect(dataTabwidget,SIGNAL(currentChanged(int)),this,SLOT(dataTabChange(int)));
    }
}
void MorphoHub_MainWindow::contentValueChange(QTreeWidgetItem *item,int column)
{
    if(this->dbpath.isEmpty())
    {
        QMessageBox::warning(this,"Initializtion","Please set the database path!");
        toLogWindow("Warning: Please set the database path!");
        return;
    }
    QTreeWidgetItem *itemparent=item->parent();
    if(itemparent==NULL)
        return;
    QString itemtext=item->text(column);
    //shield Assigned1 , Assigned2 and Questionzone
    if(itemtext.compare("Assigned1")==0||
            itemtext.compare("Assigned2")==0||
            itemtext.compare("QuestionZone")==0)
    {
        QMessageBox::warning(this,"Function Not Ready","Please Wait!");
        return;
    }
    //make sure this table hasn't been created.
    //if already created, update it
    if(datatabletitlelist.contains(itemtext))
    {
        //1.move to this tab
        int tabindex=datatabletitlelist.indexOf(itemtext);
        if(dataTabwidget->currentIndex()!=tabindex)
        {
            dataTabwidget->setCurrentIndex(tabindex);
        }
        else
        {
            //2.update the content of this tab
            QTableWidget *levelTable=datatablelist.at(tabindex);
            levelTable->clear();
            //get reconstructions info from this level
            QList<ReconstructionInfo> thislevelres;
            thislevelres=getReconstuctionsFromLevel(itemtext);
            updateTableDataLevel(levelTable,thislevelres);
            toLogWindow(tr("Tab: %1 update!").arg(itemtext));
        }
    }
    else
    {
        //get reconstructions info from this level
        QList<ReconstructionInfo> thislevelres;
        thislevelres=getReconstuctionsFromLevel(itemtext);
        if(thislevelres.size()>0)
        {
            QTableWidget* levelTable=new QTableWidget();
            levelTable=createTableDataLevel(thislevelres);
            if(levelTable)
            {
                datatabletitlelist.append(itemtext);
                datatablelist.append(levelTable);
                dataTabwidget->addTab(levelTable,itemtext);
                toLogWindow(tr("New Tab: %1 !").arg(itemtext));
            }
        }
    }
    //qDebug()<<"cliked content "<<contentTreewidget->currentItem()->text(contentTreewidget->currentColumn());
}

void MorphoHub_MainWindow::cellSortColumn(int c)
{
    if(c>=0)
    {
        int curtabindex=dataTabwidget->currentIndex();
        QTableWidget *levelTable=datatablelist.at(curtabindex);
        if(levelTable->rowCount()>0)
        {
            levelTable->sortItems(c,Qt::AscendingOrder);
            toLogWindow(tr("Sort Column by the %1").arg(datatitle.at(c)));
        }
    }
}

void MorphoHub_MainWindow::popAction_3Dview_slot()
{
    qDebug()<<"call 3d view";
    int curtabindex=dataTabwidget->currentIndex();
    QTableWidget *levelTable=datatablelist.at(curtabindex);
    if(levelTable!=NULL)
    {
        QTreeWidgetItem* tempqtreeitem=contentTreewidget->currentItem();
        QString parentSpaceName="WorkingSpace";
        if(tempqtreeitem != NULL)
        {
            if(tempqtreeitem->parent()!= NULL)
            {
                parentSpaceName=tempqtreeitem->parent()->text(0);
            }
            else
            {
                parentSpaceName=contentTreewidget->currentItem()->text(contentTreewidget->currentColumn());
            }
        }
    //    qDebug()<<"content "<<parentSpaceName;
        QString curPathSWC = this->dbpath+"/"+parentSpaceName+"/"+curRecon.levelID+"/"+curRecon.fatherDirName+"/"+curRecon.fileName+".ano";
        //QString curPathSWC = this->dbpath+"/"+this->contentTreewidget->currentItem()->text(this->contentTreewidget->currentColumn())+"/"+curRecon.levelID+"/"+curRecon.fatherDirName+"/"+curRecon.fileName+".ano";
        //iscurReconExist();
        //qDebug()<<"swc path:"<<curPathSWC;
        QFileInfo curSWCBase(curPathSWC);
        if(curSWCBase.exists())
        {
            //qDebug()<<"path "<<curPathSWC;
            V3dR_MainWindow * surface_win = MorphoHubcallback->open3DViewerForLinkerFile(curPathSWC);
//            MorphoHubcallback->open3DViewerForLinkerFile(curPathSWC);
//                new3DWindow = MorphoHubcallback->open3DViewerForSingleSurfaceFile(curPathSWC);
            //reset window title to basename instead of path name
                MorphoHubcallback->setWindowDataTitle(surface_win,curSWCBase.baseName());
        }
    }
}

bool MorphoHub_MainWindow::iscurReconExist()
{
    bool outflag=false;
    QTreeWidgetItem* tempqtreeitem=contentTreewidget->currentItem();
    QString parentSpaceName="WorkingSpace";
    if(tempqtreeitem != NULL)
    {
        if(tempqtreeitem->parent()!= NULL)
        {
            parentSpaceName=tempqtreeitem->parent()->text(0);
        }
        else
        {
            parentSpaceName=contentTreewidget->currentItem()->text(contentTreewidget->currentColumn());
        }
    }
//    qDebug()<<"content "<<parentSpaceName;
    QString curPathSWC = this->dbpath+"/"+parentSpaceName+"/"+curRecon.levelID+"/"+curRecon.fatherDirName+"/"+curRecon.fileName+".ano";
    QFileInfo curSWCBase(curPathSWC);
//    qDebug()<<"swc path:"<<curPathSWC;
    if(curSWCBase.exists())
    {
//        qDebug()<<"path "<<curPathSWC;
//        qDebug()<<"move in";
        outflag=true;
    }
    return outflag;
}

void MorphoHub_MainWindow::ondataTab_customContextmenuRequested(const QPoint &pos)
{
    qDebug()<<"Call Right-Click Menu";
    int curtabindex=dataTabwidget->currentIndex();
    QTableWidget *levelTable=datatablelist.at(curtabindex);
    QTableWidgetItem *item=levelTable->itemAt(pos);
    if(item!=NULL)
    {
        int row=item->row();
        curRecon.SdataID=levelTable->item(row,0)->text();
        curRecon.SomaID=levelTable->item(row,1)->text();
        curRecon.author.UserID=levelTable->item(row,2)->text();
        curRecon.checkers=levelTable->item(row,3)->text();
        curRecon.levelID=levelTable->item(row,4)->text();
        curRecon.updateTime=levelTable->item(row,5)->text();
        curRecon.fatherDirName=levelTable->item(row,6)->text();
        curRecon.fileName=levelTable->item(row,7)->text();

        popMenuFordataTab->clear();
        popMenuFordataTab->addAction(popAction_3Dview);
        popMenuFordataTab->addSeparator();
        popMenuFordataTab->exec(QCursor::pos());
    }
}

void MorphoHub_MainWindow::seeIn3Dview_slot(int row, int column)
{
//    qDebug()<<"row= "<<row<<"cloumn = "<<column;
    if(column==1)
    {
        //get the path of the clicked neuron
        int curtabindex=dataTabwidget->currentIndex();
        QTableWidget *levelTable=datatablelist.at(curtabindex);
        if(levelTable!=NULL)
        {
            QString levelid=levelTable->item(row,4)->text();
            QString parentdir=levelTable->item(row,6)->text();
            QString clickedName=levelTable->item(row,7)->text();
            QTreeWidgetItem* tempqtreeitem=contentTreewidget->currentItem();
            QString parentSpaceName="WorkingSpace";
            if(tempqtreeitem != NULL)
            {
                if(tempqtreeitem->parent()!= NULL)
                {
                    parentSpaceName=tempqtreeitem->parent()->text(0);
                }
                else
                {
                    parentSpaceName=contentTreewidget->currentItem()->text(contentTreewidget->currentColumn());
                }
            }
        QString curPathSWC = this->dbpath+"/"+parentSpaceName+"/"+levelid+"/"+parentdir+"/"+clickedName+".ano";

            QFileInfo curSWCBase(curPathSWC);
            if(curSWCBase.exists())
            {
                qDebug()<<"path "<<curPathSWC;
//                V3dR_MainWindow * surface_win = MorphoHubcallback->createEmpty3DViewer();
                MorphoHubcallback->open3DViewerForLinkerFile(curPathSWC);
//                new3DWindow = MorphoHubcallback->open3DViewerForSingleSurfaceFile(curPathSWC);
                //reset window title to basename instead of path name
//                MorphoHubcallback->setWindowDataTitle(new3DWindow,curSWCBase.baseName());
            }
        }
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
            curRecon.fatherDirName=levelTable->item(row,6)->text();
            curRecon.fileName=levelTable->item(row,7)->text();
        }
    }
}
void MorphoHub_MainWindow::updateTableDataLevel(QTableWidget *t,QList<ReconstructionInfo> levelres)
{
    int col = datatitle.size();
    int row= levelres.size();
    t->setRowCount(row);
    t->setColumnCount(col);
    t->setHorizontalHeaderLabels(datatitle);
    if(levelres.size()>0)
    {
//        t= new QTableWidget(row,col, this);
        for(int i=0;i<levelres.size();i++)
        {
            ReconstructionInfo tmprecons=levelres.at(i);
            t->setItem(i,0,new QTableWidgetItem(tmprecons.SdataID));
            t->setItem(i,1,new QTableWidgetItem(tmprecons.SomaID));
            t->setItem(i,2,new QTableWidgetItem(tmprecons.author.UserID));
            t->setItem(i,3,new QTableWidgetItem(tmprecons.checkers));
            t->setItem(i,4,new QTableWidgetItem(tmprecons.levelID));
            t->setItem(i,5,new QTableWidgetItem(tmprecons.updateTime));
            t->setItem(i,6,new QTableWidgetItem(tmprecons.fatherDirName));
            t->setItem(i,7,new QTableWidgetItem(tmprecons.fileName));
            //add item to tab.
        }
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        t->setSelectionMode(QAbstractItemView::SingleSelection);
        t->resizeColumnsToContents();
        t->resizeRowsToContents();

        t->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(t,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ondataTab_customContextmenuRequested(QPoint)));
        //get table cell info when clicked.
        connect(t,SIGNAL(cellClicked(int,int)),this,SLOT(celltableInfoUpdate(int,int)));
        //double click to get a 3D view of the neuron
//        connect(t,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(seeIn3Dview_slot(int,int)));
        //sort Item
        connect(t->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(cellSortColumn(int)));
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
            t->setItem(i,6,new QTableWidgetItem(tmprecons.fatherDirName));
            t->setItem(i,7,new QTableWidgetItem(tmprecons.fileName));
            //add item to tab.
        }
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        t->setSelectionMode(QAbstractItemView::SingleSelection);
        t->resizeColumnsToContents();
        t->resizeRowsToContents();
        //set content menu
        t->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(t,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ondataTab_customContextmenuRequested(QPoint)));
        //get table cell info when clicked.
        connect(t,SIGNAL(cellClicked(int,int)),this,SLOT(celltableInfoUpdate(int,int)));
        //double click to get a 3D view of the neuron
//        connect(t,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(seeIn3Dview_slot(int,int)));
        //sort Item
        connect(t->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(cellSortColumn(int)));
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
            scanindex++;
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
                qDebug()<<swcfilelist.size()<<anofilelist.size()<<apofilelist.size();
                QMessageBox::warning(this,"File Error","Abnormal file found at "+thisdirname);
                continue;
            }
            QString anofilebasename=QFileInfo(dirinside,anofilelist.at(0)).completeBaseName();
            QStringList splitAnoFilelist=anofilebasename.split("_");
            if(splitAnoFilelist.size()<9)
            {
                QMessageBox::warning(this,"Name Format Error","Abnormal file name format found at "+thisdirname);
                continue;
            }
            ReconstructionInfo tmprecons;
            tmprecons.fatherDirName=thisdirname;
            tmprecons.fileName=anofilebasename;
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
        }
    }
    //3.write to Qlist
    if(outlist.size()==0)
    {
        toLogWindow(tr("%1 is empty!").arg(levelid));
    }
    return outlist;
}

void MorphoHub_MainWindow::userManagementAction_slot()
{
    QDir dbdir(this->dbpath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        userManagementDialog=new AnnotatorManagement(this->dbpath,this->originparent);
        //sdconf_dialog->setupDBpath();
        userManagementDialog->show();
        userManagementDialog->setMinimumSize(600,400);
        userManagementDialog->setMaximumSize(800,800);
        userManagementDialog->setGeometry(50,50,800,500);
    }
}

/*source data management:*/
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
    }
}

void MorphoHub_MainWindow::NewDB_slot()
{
    //QMessageBox::information(this, tr("Information"), tr("Open"));
    QString title="please select a path for DB";
    QString inputpath = QFileDialog::getExistingDirectory(this, title,
                                                     "~/",
                                                     QFileDialog::ShowDirsOnly
                                                     |QFileDialog::DontResolveSymlinks);
    if(!inputpath.isEmpty())
    {
        int reply;
        reply=QMessageBox::warning(this,"Database Initialization",QObject::tr("You are making a new database now!\n")+
                                   QObject::tr("MorphoHub will create some folders for initialization.\n")+
                                   QObject::tr("Are you sure you want to do this?\n"),
                                   QMessageBox::Ok,QMessageBox::Cancel);
        if(reply==QMessageBox::Cancel)
            return;
        QDir::setCurrent(inputpath);
        QDir dir(QDir::currentPath());
        dbpath=dir.absolutePath();
        qDebug()<<"path is : "<<dbpath;
        updateStatusBar(tr("Database path : %1").arg(dbpath));
        toLogWindow(tr("Database path : %1").arg(dbpath));
        //make new dir for basic db
        for(int i=0;i<DBBasicConf.size();i++)
        {
            QString thisfolderpath=dbpath+"/"+DBBasicConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New Dir: %1").arg(thisfolderpath));
            }
        }
        //make new dir for workingspace
        for(int i=0;i<workingspaceConf.size();i++)
        {
            QString thisfolderpath=dbpath+workingspaceConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New WorkingSpace Dir: %1").arg(thisfolderpath));
            }
        }
        //make new dir for Finished path
        for(int i=0;i<finishedConf.size();i++)
        {
            QString thisfolderpath=dbpath+finishedConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New WorkingSpace Dir: %1").arg(thisfolderpath));
            }
        }
        //content widget init
        createTabWindow(false);
        //test settings
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("dbpath",dbpath);
    }
}


void MorphoHub_MainWindow::SetDB_slot()
{
    QString title="please select a path for DB";
    QString inputpath = QFileDialog::getExistingDirectory(this, title,
                                                     "~/",
                                                     QFileDialog::ShowDirsOnly
                                                     |QFileDialog::DontResolveSymlinks);
    if(!inputpath.isEmpty())
    {
        //converter dir path
        int reply;
        reply=QMessageBox::warning(this,"Database Initialization",QObject::tr("You are switching database now!\n")+
                                   QObject::tr("MorphoHub will create some folders for initialization.\n")+
                                   QObject::tr("Are you sure you want to do this?\n"),
                                   QMessageBox::Ok,QMessageBox::Cancel);
        if(reply==QMessageBox::Cancel)
            return;
        QDir::setCurrent(inputpath);
        QDir dir(QDir::currentPath());
        dbpath=dir.absolutePath();
        qDebug()<<"path is : "<<dbpath;

        //1.check the basic path of the database.
        //if not exist, make new dir and give a note.
        updateStatusBar(tr("Database path : %1").arg(dbpath));
        toLogWindow(tr("Database path : %1").arg(dbpath));
        //make new dir for basic db
        for(int i=0;i<DBBasicConf.size();i++)
        {
            QString thisfolderpath=dbpath+"/"+DBBasicConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New Dir: %1").arg(thisfolderpath));
            }
        }
        //make new dir for workingspace
        for(int i=0;i<workingspaceConf.size();i++)
        {
            QString thisfolderpath=dbpath+workingspaceConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New WorkingSpace Dir: %1").arg(thisfolderpath));
            }
        }
        //make new dir for Finished path
        for(int i=0;i<finishedConf.size();i++)
        {
            QString thisfolderpath=dbpath+finishedConf[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New WorkingSpace Dir: %1").arg(thisfolderpath));
            }
        }
        //content widget init
        createTabWindow(false);
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("dbpath",dbpath);
    }
    else
    {
        QMessageBox::warning(this,"Path Error","Can't find database path, Please reset it again!");
        return;
    }
}

void MorphoHub_MainWindow::setting_Apply_Qpushbutton_slot()
{
    QSettings settings("MorphoHub","Vaa3d");
    settings.setValue("dbpath",dbpath_LineEdit->text());
    settings.setValue("UserID",userID_QLineEdit->text().toUpper());
    QStringList showtextlist=Inittab_LineEdit->text().simplified().split(",");
    settings.setValue("InitTab",showtextlist);
    setting_Apply_Qpushbutton->setEnabled(false);
    QMessageBox::information(this,tr("Success"),tr("All the settings have been saved (Restart to load the new settings)."));
}

void MorphoHub_MainWindow::setting_Cancel_Qpushbutton_slot()
{
    SettingDialog->close();
}

void MorphoHub_MainWindow::setting_Reset_Qpushbutton_slot()
{
    dbpath_LineEdit->setText("");
    userID_QLineEdit->setText("");
    Inittab_LineEdit->setText("L1A,L1B,L1C,L2A,L2B,L2C");
    //Inittab_LineEdit->setPlaceholderText("Input like this: L1A,L1CCheck");
    setting_Apply_Qpushbutton->setEnabled(true);
}

void MorphoHub_MainWindow::settingsValueChanges_slot(const QString &text)
{
    setting_Apply_Qpushbutton->setEnabled(true);
}

void MorphoHub_MainWindow::setdbpath_pushbutton_slot()
{
    QString title="please select a path for DB";
    QString inputpath = QFileDialog::getExistingDirectory(this, title,
                                                     "~/",
                                                     QFileDialog::ShowDirsOnly
                                                     |QFileDialog::DontResolveSymlinks);
    if(inputpath.isEmpty())
    {
        QMessageBox::warning(this,"Path Error","Can't find database path, Please reset it again!");
        return;
    }
    else
    {
        dbpath_LineEdit->setText(inputpath);
        setting_Apply_Qpushbutton->setEnabled(true);
    }
}

void MorphoHub_MainWindow::SettingAction_slot()
{
    QSettings settings("MorphoHub","Vaa3d");
    QString setdbpath=settings.value("dbpath").toString();
    QString setUserID=settings.value("UserID").toString();
    QStringList setinitworkingspaceTablist=settings.value("InitTab").toStringList();
    //    settings.clear();
        //1.dbpath
        //2.userID
        //3.initworkingspaceTablist
    SettingDialog=new QDialog(this);
    SettingDialog->setWindowTitle("MorphoHub-Settings");
    mainlayoutforSettings=new QHBoxLayout();
    SettingsTabwidget=new QTabWidget();
    SettingsTabwidget->setTabsClosable(false);
    SettingsTabwidget->setMovable(false);

    mainWidgetforSettings=new QWidget;
    basictabQGridLayout=new QGridLayout();

    QLabel *dbpath_label=new QLabel("DB path: ");
    dbpath_LineEdit=new QLineEdit();
    dbpath_LineEdit->setReadOnly(true);
    dbpath_LineEdit->setText(setdbpath);
    setdbpath_pushbutton=new QPushButton("Choose");
    connect(setdbpath_pushbutton,SIGNAL(clicked()),this,SLOT(setdbpath_pushbutton_slot()));
    connect(dbpath_LineEdit,SIGNAL(textChanged(QString)),this,SLOT(settingsValueChanges_slot(QString)));

    QLabel *userID_QLabel=new QLabel("UserID:");
    userID_QLineEdit=new QLineEdit();
    userID_QLineEdit->setText(setUserID);
    userID_QLineEdit->setReadOnly(false);
    connect(userID_QLineEdit,SIGNAL(textChanged(QString)),this,SLOT(settingsValueChanges_slot(QString)));

    QLabel *Inittab_label=new QLabel("Init Level tab: ");
    Inittab_LineEdit=new QLineEdit();
    if(setinitworkingspaceTablist.size()==0)
        Inittab_LineEdit->setPlaceholderText("Input like this: L1A,L1CCheck");
    else
    {
        QString showtext=setinitworkingspaceTablist.at(0);
        for(int i=1;i<setinitworkingspaceTablist.size();i++)
        {
            showtext+=(","+setinitworkingspaceTablist.at(i));
        }
        Inittab_LineEdit->setText(showtext);
    }
    connect(Inittab_LineEdit,SIGNAL(textChanged(QString)),this,SLOT(settingsValueChanges_slot(QString)));

    setting_Apply_Qpushbutton=new QPushButton("Apply");
    setting_Apply_Qpushbutton->setEnabled(false);
    connect(setting_Apply_Qpushbutton,SIGNAL(clicked()),this,SLOT(setting_Apply_Qpushbutton_slot()));
    setting_Cancel_Qpushbutton=new QPushButton("Cancel");
    connect(setting_Cancel_Qpushbutton,SIGNAL(clicked()),this,SLOT(setting_Cancel_Qpushbutton_slot()));
    setting_Reset_Qpushbutton=new QPushButton("Reset");
    connect(setting_Reset_Qpushbutton,SIGNAL(clicked()),this,SLOT(setting_Reset_Qpushbutton_slot()));

    //layout of basic setting tab
    basictabQGridLayout->addWidget(dbpath_label,0,0,1,1);
    basictabQGridLayout->addWidget(dbpath_LineEdit,0,1,1,5);
    basictabQGridLayout->addWidget(setdbpath_pushbutton,0,6,1,1);
    basictabQGridLayout->addWidget(userID_QLabel,1,0,1,1);
    basictabQGridLayout->addWidget(userID_QLineEdit,1,1,1,6);
    basictabQGridLayout->addWidget(Inittab_label,2,0,1,1);
    basictabQGridLayout->addWidget(Inittab_LineEdit,2,1,1,6);

    basictabQGridLayout->addWidget(setting_Cancel_Qpushbutton,6,0,1,1);
    basictabQGridLayout->addWidget(setting_Reset_Qpushbutton,6,3,1,1);
    basictabQGridLayout->addWidget(setting_Apply_Qpushbutton,6,6,1,1);

    mainWidgetforSettings->setLayout(basictabQGridLayout);
    SettingsTabwidget->addTab(mainWidgetforSettings,"Basic");
    mainlayoutforSettings->addWidget(SettingsTabwidget,0);
    SettingDialog->setLayout(mainlayoutforSettings);
    SettingDialog->setGeometry(100,100,600,600);
//    SettingDialog->raise();
    SettingDialog->setModal(true);
    SettingDialog->show();
}


void MorphoHub_MainWindow::updateStatusBar(const QString& showtext)
{
    statusLabel->setText(showtext);
    toLogWindow(showtext);
}

void MorphoHub_MainWindow::toLogWindow(const QString &logtext)
{
    QString getlogtext=logtextedit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logtextedit->setText(showText);
    logtextedit->moveCursor(QTextCursor::End);
}

void MorphoHub_MainWindow::removeSubTab(int subindex)
{
    dataTabwidget->removeTab(subindex);
    qDebug()<<"remove tab c1";
}


void MorphoHub_MainWindow::dataTabChange(int tabindex)
{
    //update the content of this tab
    QTableWidget *levelTable=datatablelist.at(tabindex);
    QString itemtext=datatabletitlelist.at(tabindex);
    levelTable->clear();
    toLogWindow(tr("Move to Tab: %1").arg(itemtext));
    //get reconstructions info from this level
    QList<ReconstructionInfo> thislevelres;
    thislevelres=getReconstuctionsFromLevel(itemtext);
    updateTableDataLevel(levelTable,thislevelres);
    toLogWindow(tr("Tab: %1 update!").arg(itemtext));
}


void MorphoHub_MainWindow::loginAction_slot()
{
    loginDialog=new QDialog(this);
    loginDialog->setWindowTitle("MorphoHub-Login-Window");

    QLabel *userIDQLabel=new QLabel("UserID:");
    loginUserIDQLineEdit=new QLineEdit();
    QLabel *passwordQLabel=new QLabel("Password:");
    loginPasswordQlineedit=new QLineEdit();

    loginCancelButton =new QPushButton("Cancel");
    connect(loginCancelButton,SIGNAL(clicked()),this,SLOT(loginCancelButton_slot()));
    loginOkayButton=new QPushButton("Okay");
    connect(loginOkayButton,SIGNAL(clicked()),this,SLOT(loginOkayButton_slot()));

    loginMainlayout=new QGridLayout();
    loginMainlayout->addWidget(userIDQLabel,1,0,1,2);
    loginMainlayout->addWidget(loginUserIDQLineEdit,1,2,1,2);
    loginMainlayout->addWidget(passwordQLabel,2,0,1,1);
    loginMainlayout->addWidget(loginPasswordQlineedit,2,2,1,2);

    loginMainlayout->addWidget(loginCancelButton,4,0,1,1);
    loginMainlayout->addWidget(loginOkayButton,4,3,1,1);

    loginDialog->setLayout(loginMainlayout);
    loginDialog->raise();
//    loginDialog->setFixedSize(400,400);

    loginDialog->setGeometry(100,100,400,400);
    loginDialog->setModal(true);
    loginDialog->show();
}


void MorphoHub_MainWindow::loginOkayButton_slot()
{
    if(!loginUserIDQLineEdit->text().isEmpty())
    {
        //check the input at conf
        //if yes
        curOperator.UserID=loginUserIDQLineEdit->text().toUpper();
        curOperator.workingplace="SEU";
        curOperator.priority=APAdministrater;
        loginDialog->close();
        loginAction->setEnabled(false);
        logoutAction->setEnabled(true);
        userStatusLabel->setText(tr("UserID: %1").arg(curOperator.UserID));
        toLogWindow(tr("Welcome %1 login.").arg(curOperator.UserID));
        setProtocolFunctionEnabled(true);
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("UserID",curOperator.UserID);
    }
    else
    {
        QMessageBox::warning(this,"Input Errot","Please Input User ID!");
        return;
    }
}


void MorphoHub_MainWindow::loginCancelButton_slot()
{
    loginDialog->close();
}


void MorphoHub_MainWindow::logoutAction_slot()
{
    QString olduserID=curOperator.UserID;
    curOperator.UserID="";
    curOperator.workingplace="";
    curOperator.priority=APvisitor;
    loginAction->setEnabled(true);
    logoutAction->setEnabled(false);
    toLogWindow(tr("%1 logout").arg(olduserID));
    userStatusLabel->setText(tr("UserID: Nobody"));
    setProtocolFunctionEnabled(false);
}


void MorphoHub_MainWindow::helpAction_slot()
{
    QString helptext=
            "MorphoHub a platform for whole brain neuron morphology reconstruction project.<br>"
            "This plugin is developed by Shengdian Jiang. 2019-10 <br>"
            "Email : shengdianjiang@seu.edu.cn <br>"
            "Version:1.1<br>"
            "Update log:Fixed window switching bug.(2019.10.13)<br>"
            "<br>============================================="
            "<H2>Introduction: Main View </H2>"
            "=============================================<br>"
            "1.MorphoHub is a database management platform.<br>"
            "---------------------------------------------<br>"
            "2.function toolbar:<br>"
            "   2.1 NewDB and LoadDB button are placed at toolbar so that you can quickly setup the software environment.<br>"
            "   2.2 Serval fuctions of level control are also placed at toolbar.<br>"
            "   2.3 In order to manage the data, you have to log in to MorphoHub with a registered UserID.<br>"
            "---------------------------------------------<br>"
            "3.Main view of MorphoHub is combined with three parts:<br>"
            "   3.1 Content Window.<br>"
            "       Click one of the item at content, a new data tab about this level will be created at Data window.<br>"
            "       Note: If this level is already created at Data Window, the data tab will be updated.<br>"
            "   3.2 Data Window.<br>"
            "       This part will show the basic information of neurons of the chosen level.<br>"
            "   3.3 Main Log Window.<br>"
            "       This window will show the operations you make.<br>"
            "---------------------------------------------<br>"
            "<br>============================================="
            "<H2>Usage: Initialization</H2>"
            "1.Database<br>"
            "  1.1 New Database<br>"
            "  1.2 Load Database<br>"
            "  1.3 Settings<br>"
            "2.Level Control<br>"
            "3.Management<br>"
            "  3.1 Data Management<br>"
            "  3.2 User Management<br>"
            "4.Login<br>"
            "=============================================<br>"
            ""
            ;
    QTextEdit *textEdit=new QTextEdit(helptext);
    textEdit->setWindowTitle("MorphoHub Document and Help");
    textEdit->resize(850, 700); //use the QWidget function
    textEdit->setReadOnly(true);
    textEdit->setFontPointSize(16);
    textEdit->show();
}
/**********************************************************/
/****************Services SLots*************************/
/********ErrorCheck,Visualiztion*****/
void MorphoHub_MainWindow::visualizationAction_slot()
{
    BigscreenControlDialog bigscreencontroldialog(originparent);
    if(bigscreencontroldialog.exec()!=QDialog::Accepted)
        return;
    bigscreencontroldialog.update();
    DisplayPARA controlPara=bigscreencontroldialog.controlPara;
    MethodForBigScreenDisplay(*MorphoHubcallback,originparent,controlPara);
}

void MorphoHub_MainWindow::errorCheckAction_slot()
{
    qDebug()<<"ErrorCheck service";
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


void MorphoHub_MainWindow::checkAction_slot()
{
    QDir dbdir(this->dbpath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        if(curRecon.alreadyInit()&&iscurReconExist())
        {
            levelControlDialog=new MainDialog(this->originparent);
            levelControlDialog->setAnnotator(curOperator);//
            levelControlDialog->setCurNeuron(curRecon);//get current neuron info
            levelControlDialog->setFunction("Check");
            levelControlDialog->setupAnnotationProtocol(seuallenAP);
            levelControlDialog->updateMainView();
            levelControlDialog->setModal(true);
            levelControlDialog->show();
            levelControlDialog->setGeometry(100,100,600,600);
        }
        else
        {
            QMessageBox::warning(this,"File Not Found","Please retry or update the table!");
            return;
        }
    }
}

void MorphoHub_MainWindow::commitAction_slot()
{
    //
    toLogWindow("call Commit function");
    QDir dbdir(this->dbpath);
    if(!dbdir.exists())
    {
        toLogWindow("Error: can't find database path!");
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        if(curRecon.alreadyInit()&&iscurReconExist())
        {
            toLogWindow("Go to Commit page.");
            levelControlDialog=new MainDialog(this->dbpath,this->originparent);
            levelControlDialog->setAnnotator(curOperator);//
            levelControlDialog->setCurNeuron(curRecon);//get current neuron info
            levelControlDialog->setFunction("Commit");
            levelControlDialog->setupAnnotationProtocol(seuallenAP);
            levelControlDialog->updateMainView();

            levelControlDialog->setModal(true);
            levelControlDialog->show();
            levelControlDialog->setGeometry(100,100,600,600);
            //this->raise();
        }
        else
        {
            QMessageBox::warning(this,"File Not Found","Please retry or update the table!");
            return;
        }
    }
}

void MorphoHub_MainWindow::skipAction_slot()
{
    QDir dbdir(this->dbpath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        if(curRecon.alreadyInit()&&iscurReconExist())
        {
            levelControlDialog=new MainDialog(this->dbpath,this->originparent);
            levelControlDialog->setAnnotator(curOperator);//
            levelControlDialog->setCurNeuron(curRecon);//get current neuron info
            levelControlDialog->setFunction("Skip");
            levelControlDialog->setupAnnotationProtocol(seuallenAP);
            levelControlDialog->updateMainView();

            levelControlDialog->setModal(true);
            levelControlDialog->show();
            levelControlDialog->setGeometry(100,100,600,600);
        }
        else
        {
            QMessageBox::warning(this,"File Not Found","Please retry or update the table!");
            return;
        }
    }
}

void MorphoHub_MainWindow::rollbackAction_slot()
{
    QDir dbdir(this->dbpath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        if(curRecon.alreadyInit()&&iscurReconExist())
        {
            levelControlDialog=new MainDialog(this->dbpath,this->originparent);
            levelControlDialog->setAnnotator(curOperator);//
            levelControlDialog->setCurNeuron(curRecon);//get current neuron info
            levelControlDialog->setFunction("Rollback");
            levelControlDialog->setupAnnotationProtocol(seuallenAP);
            levelControlDialog->updateMainView();

            levelControlDialog->setModal(true);
            levelControlDialog->show();
            levelControlDialog->setGeometry(100,100,600,600);
        }
        else
        {
            QMessageBox::warning(this,"File Not Found","Please retry or update the table!");
            return;
        }
    }
}

void MorphoHub_MainWindow::reassignAction_slot()
{
    QMessageBox::warning(this,"Not Open","Please Wait!");
    return;
}

