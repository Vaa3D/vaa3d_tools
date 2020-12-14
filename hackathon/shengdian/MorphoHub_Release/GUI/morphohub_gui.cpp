#include "morphohub_gui.h"
morphoHub_GUI::morphoHub_GUI(V3DPluginCallback2 &callback,QWidget *parent) :
    QMainWindow(parent)
{
    this->mainparent=parent;
    morphoHubcallback=&callback;
    setWindowTitle(tr("MorphoHub-GUI"));
    morphoHub_Init();
    createActions();
    createStatusBar();
    createToolBar();
    createMenus();
    createContentTreeWidget(true);
    createTabWindow(true);
    setMainLayout();
    checkUserState();
}
/*Initialization of MorphoHub, load settings*/
void morphoHub_GUI::checkUserState()
{
    if(curOperator.UserID.isEmpty())
    {
        loginAction->setEnabled(true);
        logoutAction->setEnabled(false);
        toLogWindow("Warning: You have to sign in to get the advanced functions.");
        userStatusLabel->setText("UserID: Nobody");
    }
    else
    {
        loginAction->setEnabled(false);
        logoutAction->setEnabled(true);
        toLogWindow(tr("Current UserID: %1").arg(curOperator.UserID));
        userStatusLabel->setText(tr("UserID: %1").arg(curOperator.UserID));
    }
}
void morphoHub_GUI::morphoHub_Init()
{
    qDebug()<<"load settings"<<endl;
    QSettings settings("MorphoHub","Vaa3d");
    if(settings.contains("dbpath"))
        mfs.setDBPath(settings.value("dbpath").toString());
    else
        toLogWindow("Please set or load the database path!");
    if(settings.contains("imgpath"))
        mfs.setImgPath(settings.value("imgpath").toString());
    curOperator.UserID=settings.value("UserID").toString();
}
void morphoHub_GUI::createMenus()
{
    //database menu
    dbMenu = menuBar()->addMenu(tr("&Database"));
    dbMenu->addAction(newDBAction);
    dbMenu->addAction(loadDBAction);
    dbMenu->addAction(setImgPathAction);
    dbMenu->addAction(settingsAction);
    //Management menu
    managementMenu=menuBar()->addMenu(tr("Management"));
    managementMenu->addAction(imageManagementAction);
    managementMenu->addAction(userManagementAction);
    //services menu
    servicesMenu = menuBar()->addMenu(tr("&Services"));
    servicesMenu->addAction(monitorAction);
//    servicesMenu->addAction(errorCheckAction);
    //window menu
    menuWindow = menuBar()->addMenu(tr("&Window"));
    menuWindow->addAction(loginAction);
    menuWindow->addAction(logoutAction);
    //help menu
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpAction);
}
void morphoHub_GUI::createActions()
{
    //action for database
    newDBAction = new QAction(tr("&NewDB"), this);
    newDBAction->setShortcuts(QKeySequence::New);
    newDBAction->setStatusTip(tr("Set up a new DB"));
    connect(newDBAction, SIGNAL(triggered()), this, SLOT(newDBAction_slot()));

    loadDBAction = new QAction(tr("&LoadDB"), this);
    loadDBAction->setStatusTip(tr("load an exist DB"));
    connect(loadDBAction, SIGNAL(triggered()), this, SLOT(loadDBAction_slot()));

    setImgPathAction=new QAction(tr("&setImagePath"), this);
    setImgPathAction->setStatusTip(tr("load image path"));
    connect(setImgPathAction, SIGNAL(triggered()), this, SLOT(setImgPathAction_slot()));

    settingsAction = new QAction(tr("&Settings"), this);
    settingsAction->setStatusTip(tr("Default settings"));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(settingsAction_slot()));

    //action for Management
    imageManagementAction= new QAction(tr("&Image"), this);
    imageManagementAction->setStatusTip(tr("Image Dataset Management"));
    connect(imageManagementAction,SIGNAL(triggered()),this,SLOT(imageManagementAction_slot()));
    userManagementAction= new QAction(tr("&User"), this);
    userManagementAction->setToolTip(tr("User Management"));
    connect(userManagementAction,SIGNAL(triggered()),this,SLOT(userManagementAction_slot()));

    //action for services
    monitorAction=new QAction(tr("Monitoring"),this);
    monitorAction->setToolTip(tr("Monitoring of the database"));
    connect(monitorAction,SIGNAL(triggered()),this,SLOT(monitorAction_slot()));

    //Actions for window
    loginAction=new QAction(tr("&Login"),this);
    loginAction->setToolTip(tr("You have to sign in to get the advanced functions."));
    loginAction->setEnabled(true);
    connect(loginAction,SIGNAL(triggered()),this,SLOT(loginAction_slot()));

    logoutAction=new QAction(tr("&Logout"),this);
    logoutAction->setEnabled(false);
    connect(logoutAction,SIGNAL(triggered()),this,SLOT(logoutAction_slot()));

    //action for Help
    helpAction=new QAction(tr("&Help"),this);
    helpAction->setToolTip(tr("MorphoHub documents"));
    connect(helpAction,SIGNAL(triggered()),this,SLOT(helpAction_slot()));
}
void morphoHub_GUI::createStatusBar()
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
void morphoHub_GUI::updateStatusBar(const QString& showtext)
{
    QString log="Input log error";
    if(!showtext.isEmpty())
        log=showtext;
    statusLabel->setText(log);
    toLogWindow(log);
}
void morphoHub_GUI::toLogWindow(const QString &logtext)
{
    QString getlogtext=logtextedit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logtextedit->setText(showText);
    logtextedit->moveCursor(QTextCursor::End);
}
void morphoHub_GUI::createToolBar()
{
    dbToolBar=this->addToolBar(tr("DB"));
    dbToolBar->addAction(newDBAction);
    dbToolBar->addAction(loadDBAction);
    loginToolbar=this->addToolBar(tr("User"));
    loginToolbar->addAction(loginAction);
    loginToolbar->addAction(logoutAction);
}
void morphoHub_GUI::setMainLayout()
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
    mainWidget->setLayout(mainlayout);
    setCentralWidget(mainWidget);
}
void morphoHub_GUI::createContentTreeWidget(bool init)
{
    if(init)
    {
        contentTreewidget=new QTreeWidget(this);
        contentTreewidget->setColumnCount(1);
        contentTreewidget->setHeaderLabel(tr("Content"));
        QList<QTreeWidgetItem*> contentitems;
        //create nodes
        content_morphometry=new QTreeWidgetItem(contentTreewidget,QStringList(QString("Morphometry")));
        contentitems.append(content_morphometry);
        //create parent node
        contentTreewidget->insertTopLevelItems(0,contentitems);
        contentTreewidget->setItemsExpandable(true);
        contentTreewidget->expandAll();
        connect(contentTreewidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(contentValueChange(QTreeWidgetItem*,int)));
    }
}
void morphoHub_GUI::createTabWindow(bool init)
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
        //come here later
        toLogWindow(tr("under developing"));
    }
}
void morphoHub_GUI::contentValueChange(QTreeWidgetItem *item,int column)
{
    if(mfs.dbPath.isEmpty())
    {
        QMessageBox::warning(this,"Initializtion","Please set the database path!");
        toLogWindow("Warning: Please set the database path!");
        return;
    }
    QTreeWidgetItem *itemparent=item->parent();
    if(itemparent==NULL)
        return;
    QString itemtext=item->text(column);
    toLogWindow(itemtext);
}
/*-------------------slot action-------------------*/
/*User management*/
void morphoHub_GUI::userManagementAction_slot()
{
    QDir dbdir(mfs.dbPath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Dir Not Found","Please setup database path!");
        return;
    }
    else
    {
        userManagementDialog=new UserManagement(mfs,this->mainparent);
        userManagementDialog->show();
        userManagementDialog->setMinimumSize(600,400);
        userManagementDialog->setMaximumSize(800,800);
        userManagementDialog->setGeometry(50,50,800,500);
    }
}
/*Image data management:*/
void morphoHub_GUI::imageManagementAction_slot()
{
    QDir dbdir(mfs.dbPath);
    if(!dbdir.exists())
    {
        QMessageBox::warning(this,"Image Path Not Found","Please reset image path!");
        return;
    }
    else
    {
        idataDialog=new ImageDataManagement(mfs,this->mainparent);
        idataDialog->show();
        idataDialog->setMinimumSize(600,400);
        idataDialog->setMaximumSize(1200,1000);
        idataDialog->setGeometry(50,50,1000,800);
    }
}
/*Services slot*/
void morphoHub_GUI::monitorAction_slot()
{
    ScreenWall bigscreencontroldialog(mainparent);
    if(bigscreencontroldialog.exec()!=QDialog::Accepted)
        return;
    bigscreencontroldialog.update();
    DisplayPARA controlPara=bigscreencontroldialog.controlPara;
    MethodForBigScreenDisplay(*morphoHubcallback,mainparent,controlPara);
}
/*login and logout*/
void morphoHub_GUI::loginAction_slot()
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
    loginDialog->setGeometry(100,100,400,400);
    loginDialog->setModal(true);
    loginDialog->show();
}
void morphoHub_GUI::loginOkayButton_slot()
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
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("UserID",curOperator.UserID);
    }
    else
    {
        QMessageBox::warning(this,"Input Errot","Please Input User ID!");
        return;
    }
}
void morphoHub_GUI::loginCancelButton_slot()
{
    loginDialog->close();
}
void morphoHub_GUI::logoutAction_slot()
{
    QString olduserID=curOperator.UserID;
    curOperator.UserID="";
    curOperator.workingplace="";
    curOperator.priority=APvisitor;
    loginAction->setEnabled(true);
    logoutAction->setEnabled(false);
    toLogWindow(tr("%1 logout").arg(olduserID));
    userStatusLabel->setText(tr("UserID: Nobody"));
}
void morphoHub_GUI::newDBAction_slot()
{
    //QMessageBox::information(this, tr("Information"), tr("Open"));
    QString title="choose path";
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
        bool setflag=mfs.setDBPath(dir.absolutePath());
        if(!setflag)
            return;
        updateStatusBar(tr("Database path : %1").arg(mfs.dbPath));
        //make new dir for basic db
        for(int i=0;i<mfs.basicDirs.size();i++)
        {
            QString thisfolderpath=mfs.dbPath+"/"+mfs.basicDirs[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New Dir: %1").arg(thisfolderpath));
            }
        }
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("dbpath",mfs.dbPath);
    }
}
void morphoHub_GUI::loadDBAction_slot()
{
    QString title="please select an existed database";
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
        bool setflag=mfs.setDBPath(dir.absolutePath());
        if(!setflag)
            return;
        //1.check the basic path of the database.
        //if not exist, make new dir and give a note.
        updateStatusBar(tr("Database path : %1").arg(mfs.dbPath));
        toLogWindow(tr("Database path : %1").arg(mfs.dbPath));
        //make new dir for basic db
        for(int i=0;i<mfs.basicDirs.size();i++)
        {
            QString thisfolderpath=mfs.dbPath+"/"+mfs.basicDirs[i];
            QDir dir(thisfolderpath);
            if(!dir.exists())
            {
                dir.mkdir(thisfolderpath);
                toLogWindow(tr("New Dir: %1").arg(thisfolderpath));
            }
        }
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("dbpath",mfs.dbPath);
    }
    else
    {
        QMessageBox::warning(this,"Path Error","Can't find database path, Please reset it again!");
        return;
    }
}
void morphoHub_GUI::setImgPathAction_slot()
{
    QString title="please select image datasets path";
    QString inputpath = QFileDialog::getExistingDirectory(this, title,
                                                     "~/",
                                                     QFileDialog::ShowDirsOnly
                                                     |QFileDialog::DontResolveSymlinks);
    if(!inputpath.isEmpty())
    {
        //converter dir path
        int reply;
        reply=QMessageBox::warning(this,"Database Initialization",QObject::tr("Set up image datasets path!\n"),
                                   QMessageBox::Ok,QMessageBox::Cancel);
        if(reply==QMessageBox::Cancel)
            return;
        QDir::setCurrent(inputpath);
        QDir dir(QDir::currentPath());
        bool setflag=mfs.setImgPath(dir.absolutePath());
        if(!setflag)
            return;
        toLogWindow(tr("Database image datasets path : %1").arg(mfs.imgDBPath));
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("imgpath",mfs.imgDBPath);
    }
    else
    {
        QMessageBox::warning(this,"Path Error","Can't find database path, Please reset it again!");
        return;
    }
}
void morphoHub_GUI::settingsAction_slot()
{
    toLogWindow("hello, developing");
}
void morphoHub_GUI::helpAction_slot()
{
    QString helptext=
            "MorphoHub--PB scale multi-morphometry of single neurons for whole brains<br>"
            "This plugin is developed by Shengdian Jiang. 2019-10 <br>"
            "Email : shengdianjiang@seu.edu.cn or shengdianjiang@gmail.com <br>"
            "Version:1.2<br>"
            "<H2>Introduction: GUI </H2>"
            "MorphoHub handles four types of data, including image volumes, neuron morphology, software components and meta data of user interactions<br>"
            "1.Image volumes.<br>"
            "----2D and 3D image datasets,scan the list of image datasets from configuration.<br>"
            "----configuration name: <image_datasets_of_morphoHub.csv>.<br>"
            "-----------------.<br>"
            "2.function toolbar:<br>"
            "   2.1 NewDB and LoadDB button are placed at toolbar so that you can quickly setup the software environment.<br>"
            "   2.2 Serval fuctions of level control are also placed at toolbar.<br>"
            "   2.3 In order to manage the data, you have to log in to MorphoHub with a registered UserID.<br>"
            "3.Main view of MorphoHub is combined with three parts:<br>"
            "   3.1 Content Window.<br>"
            "       Click one of the item at content, a new data tab about this level will be created at Data window.<br>"
            "       Note: If this level is already created at Data Window, the data tab will be updated.<br>"
            "   3.2 Data Window.<br>"
            "       This part will show the basic information of neurons of the chosen level.<br>"
            "   3.3 Main Log Window.<br>"
            "       This window will show the operations you make.<br>"

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
            "<H2>MorphoHub file system</H2>"
            "At morphoHub, there will be a configuration file for each brain datasets. and a collection table for all the datasets.<br>"
            ""
            ;
    QTextEdit *textEdit=new QTextEdit(helptext);
    textEdit->setWindowTitle("MorphoHub Document and Help");
    textEdit->resize(850, 700); //use the QWidget function
    textEdit->setReadOnly(true);
    textEdit->setFontPointSize(16);
    textEdit->show();
}

morphoHub_GUI::morphoHub_GUI()
{
}
morphoHub_GUI::~morphoHub_GUI()
{
}
