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
    setMainLayout();
}
/*Initialization of MorphoHub, load settings*/
void morphoHub_GUI::morphoHub_Init()
{
    qDebug()<<"load settings"<<endl;
    QSettings settings("MorphoHub","Vaa3d");
    if(settings.contains("dbpath"))
        mDB.loadDB(settings.value("dbpath").toString());
    else
        toLogWindow("Please set or load the database path!");
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
    /*Image part*/
    managementMenu->addAction(newImageAction);
    managementMenu->addAction(deleteImageAction);
    managementMenu->addAction(updateImageAction);
    managementMenu->addAction(download_Img_metadataAction);
    /*Soma part*/
    managementMenu->addAction(newSomataAction);
    managementMenu->addAction(deleteSomataAction);
    managementMenu->addAction(updateSomataAction);
    managementMenu->addAction(download_Soma_metadataAction);
    /*Morphometry part*/
    managementMenu->addAction(newMorphoAction);
    managementMenu->addAction(deleteMorphoAction);
    managementMenu->addAction(updateMorphoAction);
    managementMenu->addAction(download_Morpho_metadataAction);
    managementMenu->addAction(download_MorphoAction);
    //services menu
    servicesMenu = menuBar()->addMenu(tr("&Services"));
    servicesMenu->addAction(monitorAction);
//    servicesMenu->addAction(errorCheckAction);
    //window menu
    menuWindow = menuBar()->addMenu(tr("&Window"));
    //help menu
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpAction);
}
void morphoHub_GUI::createActions()
{
    //action for database
    newDBAction = new QAction(tr("&Create Database"), this);
    newDBAction->setShortcuts(QKeySequence::New);
    newDBAction->setStatusTip(tr("Create a new database"));
    connect(newDBAction, SIGNAL(triggered()), this, SLOT(newDBAction_slot()));

    loadDBAction = new QAction(tr("&Load Database"), this);
    loadDBAction->setStatusTip(tr("load an exist database"));
    connect(loadDBAction, SIGNAL(triggered()), this, SLOT(loadDBAction_slot()));

    setImgPathAction=new QAction(tr("&Set Image Path"), this);
    setImgPathAction->setStatusTip(tr("load image path"));
    connect(setImgPathAction, SIGNAL(triggered()), this, SLOT(setImgPathAction_slot()));

    settingsAction = new QAction(tr("&Settings"), this);
    settingsAction->setStatusTip(tr("Default settings"));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(settingsAction_slot()));

    //action for Management
    /*Image part*/
    newImageAction = new QAction(tr("&New Image"), this);
    newImageAction->setStatusTip(tr("Create a new image dataset"));
    connect(newImageAction, SIGNAL(triggered()), this, SLOT(newImageAction_slot()));
    deleteImageAction = new QAction(tr("&Delete Image"), this);
    deleteImageAction->setStatusTip(tr("Delete an existed image dataset"));
    connect(deleteImageAction, SIGNAL(triggered()), this, SLOT(deleteImageAction_slot()));

    updateImageAction = new QAction(tr("&Update Image Metadata"), this);
    connect(updateImageAction, SIGNAL(triggered()), this, SLOT(updateImageAction_slot()));
    download_Img_metadataAction = new QAction(tr("&Download Image Metadata"), this);
    connect(download_Img_metadataAction, SIGNAL(triggered()), this, SLOT(download_Img_metadataAction_slot()));
    /*Soma part*/
    newSomataAction = new QAction(tr("&New Soma"), this);
    newSomataAction->setStatusTip(tr("Create a new soma metadata for specified Image"));
    connect(newSomataAction, SIGNAL(triggered()), this, SLOT(newSomataAction_slot()));
    deleteSomataAction = new QAction(tr("&Delete Soma"), this);
    deleteSomataAction->setStatusTip(tr("Delete an existed soma metadata of an image dataset"));
    connect(deleteSomataAction, SIGNAL(triggered()), this, SLOT(deleteSomataAction_slot()));

    updateSomataAction = new QAction(tr("&Update Soma"), this);
    updateSomataAction->setStatusTip(tr("Update soma metadata of an image dataset (from apo file)"));
    connect(updateSomataAction, SIGNAL(triggered()), this, SLOT(updateSomataAction_slot()));
    download_Soma_metadataAction = new QAction(tr("&Download Soma Metadata"), this);
    connect(download_Soma_metadataAction, SIGNAL(triggered()), this, SLOT(download_Soma_metadataAction_slot()));
    /*Morpho part*/
    newMorphoAction = new QAction(tr("&New Morphometry"), this);
    newMorphoAction->setStatusTip(tr("Create a new morphometry"));
    connect(newMorphoAction, SIGNAL(triggered()), this, SLOT(newMorphoAction_slot()));
    deleteMorphoAction = new QAction(tr("&Delete Morphometry"), this);
    deleteMorphoAction->setStatusTip(tr("Delete an existed morphometry"));
    connect(deleteMorphoAction, SIGNAL(triggered()), this, SLOT(deleteMorphoAction_slot()));

    updateMorphoAction = new QAction(tr("&Update Morphometry"), this);
    connect(updateMorphoAction, SIGNAL(triggered()), this, SLOT(updateMorphoAction_slot()));
    download_Morpho_metadataAction = new QAction(tr("&Download Morphometry Metadata"), this);
    connect(download_Morpho_metadataAction, SIGNAL(triggered()), this, SLOT(download_Morpho_metadataAction_slot()));
    download_MorphoAction = new QAction(tr("&Download Morphometry data"), this);
    connect(download_MorphoAction, SIGNAL(triggered()), this, SLOT(download_MorphoAction_slot()));

    //action for services
    monitorAction=new QAction(tr("Monitoring"),this);
    monitorAction->setToolTip(tr("Monitoring of the database"));
    connect(monitorAction,SIGNAL(triggered()),this,SLOT(monitorAction_slot()));

    //Actions for window
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
    mainWidget->setLayout(mainlayout);
    setCentralWidget(mainWidget);

    if(mDB.getDBpath().isEmpty())
        toLogWindow("Warning: Please set the database path!");
    else
    {
        toLogWindow(tr("Load Database: %1").arg(mDB.getDBpath()));
        updateStatusBar(tr("Database: %1").arg(mDB.getDBpath()));
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
        bool setflag=mDB.createDB(dir.absolutePath());
        if(!setflag)
            return;
        updateStatusBar(tr("Database path : %1").arg(mDB.getDBpath()));
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("dbpath",mDB.getDBpath());
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
        bool setflag=mDB.createDB(dir.absolutePath());
        if(!setflag)
            return;
        updateStatusBar(tr("Database path : %1").arg(mDB.getDBpath()));
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("dbpath",mDB.getDBpath());
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
        bool setflag=mDB.setImgDBpath(dir.absolutePath());
        if(!setflag)
            return;
        toLogWindow(tr("Database image datasets path : %1").arg(mDB.getImgDBpath()));
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("imgpath",mDB.getImgDBpath());
    }
    else
    {
        QMessageBox::warning(this,"Path Error","Can't find database path, Please reset it again!");
        return;
    }
}
void morphoHub_GUI::settingsAction_slot()
{
    /*Parameters that need to be initialized.
     * database:
        *dbpath,imgdbpath
        *db_init_1stlayer,db_init_2stlayer_metadata,db_init_3stlayer_metadata
    */
    toLogWindow("hello, developing");

}
/*Image part*/
void morphoHub_GUI::newImageAction_slot()
{
    /*All i need is to, get the input-para-list of Image, and call the database function
     * 1. generate imageID (addImage)
     * 2. set metadata (updateImage), use a dialog for inputing para-list
    */
    mImage newimage;
    NewImage_gui *newImage_gui=new NewImage_gui(newimage,this->mainparent);
    newImage_gui->setMinimumSize(200,300);
    newImage_gui->setMaximumSize(400,400);
    newImage_gui->setGeometry(50,50,800,500);
    newImage_gui->exec();
    toLogWindow(newimage.getName());
}
void morphoHub_GUI::deleteImageAction_slot()
{
    toLogWindow("hello, developing");
}
void morphoHub_GUI::updateImageAction_slot()
{
    toLogWindow("hello, developing");
}
void morphoHub_GUI::download_Img_metadataAction_slot()
{
    toLogWindow("hello, developing");
}
/*Soma part*/
void morphoHub_GUI::newSomataAction_slot()
{
    toLogWindow("hello, developing");
}
void morphoHub_GUI::deleteSomataAction_slot()
{
    toLogWindow("hello, developing");
}
void morphoHub_GUI::updateSomataAction_slot()
{
    toLogWindow("hello, developing");
}
void morphoHub_GUI::download_Soma_metadataAction_slot()
{
    toLogWindow("hello, developing");
}
/*Morphometry part*/
void morphoHub_GUI::newMorphoAction_slot()
{

}
void morphoHub_GUI::deleteMorphoAction_slot()
{

}
void morphoHub_GUI::updateMorphoAction_slot()
{

}
void morphoHub_GUI::download_Morpho_metadataAction_slot()
{

}
void morphoHub_GUI::download_MorphoAction_slot()
{

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
