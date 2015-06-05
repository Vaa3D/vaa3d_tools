#include "Dialog.h"
#include "Pages.h"
#include "ItkPluginManager.h"
#include <QListWidget>
#include <QStackedWidget>

#include <iostream>

Dialog::Dialog( QWidget *parent ) : QDialog(parent)
{
    //intial the itkPluginManager
    this->itkPluginManager = new ItkPluginManager();
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(96, 84));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(128);
    contentsWidget->setSpacing(12);

    pagesWidget = new QStackedWidget;

    autoPipepage=new AutoPipePage(this);
    userPipepage=new UserPipePage(this);
    userFilterpage=new UserFilterPage(this);

    autoPipepage->setItkPluginManager(this->itkPluginManager);
    userPipepage->setItkPluginManager(this->itkPluginManager);
    userFilterpage->setItkPluginManager(this->itkPluginManager);


    pagesWidget->addWidget(autoPipepage);
    pagesWidget->addWidget(userPipepage);
    pagesWidget->addWidget(userFilterpage);

    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    m_dirChangeButton = new QPushButton(tr("change search dir"), this);

    createIcons();
    contentsWidget->setCurrentRow(0);



    //Add the pluginpath to search so that the following ones can get the plugins name
    //dirToSearch = tr("./plugins/MyITK3/Superplugin_new/Plugin2Call");
    //this->getAllThePluginsToCall();
    //this->userPipepage->setPluginName( this->pluginName );
    //this->userPipepage->setPluginPath( this->pluginPath );
    //this->userFilterpage->setPluginName ( this -> pluginName );
    //this->userFilterpage->setPluginPath ( this->pluginPath );

    connect(closeButton, SIGNAL(clicked()), this, SLOT(myclose()));
    connect(m_dirChangeButton, SIGNAL(clicked()), this, SLOT(onDirChangeButtonClicked()));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(m_dirChangeButton);
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
    move(1500,0);//move the dialog so that one can see the progress dialog
    setWindowTitle(tr("SuperPlugin Pipeline"));
    setWindowFlags(this->windowFlags() & Qt::WindowMaximizeButtonHint & Qt::WindowMinimizeButtonHint);
}
void Dialog::createIcons()
{
    QListWidgetItem *configButton = new QListWidgetItem(contentsWidget);
    configButton->setIcon(QIcon(tr(":/images/pic1.png")));
    configButton->setText(tr("Auto Pipeline"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *updateButton = new QListWidgetItem(contentsWidget);
    updateButton->setIcon(QIcon(tr(":/images/pic2.png")));
    updateButton->setText(tr("User Pipeline"));
    updateButton->setTextAlignment(Qt::AlignHCenter);
    updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *queryButton = new QListWidgetItem(contentsWidget);
    queryButton->setIcon(QIcon(tr(":/images/pic4.png")));
    queryButton->setText(tr("User Filter"));
    queryButton->setTextAlignment(Qt::AlignHCenter);
    queryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void Dialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}
void Dialog::setCallback(V3DPluginCallback2 &callback)
{
    this->itkPluginManager->setCallback(&callback);

}
void Dialog::setInitialDir(const QString& intialDir) {
  this->m_initialDir = intialDir;
}

void Dialog::intialPluginManager () {
    bool done = false;
    this->itkPluginManager->setIntialDir(m_initialDir);
    done = this->itkPluginManager->searchAllItkPlugins();
    if ( !done ) {std::cerr << "erro search the plugins! "<< std::endl;
                  return;
    }
    this->autoPipepage->initialTest();
    this->userPipepage->getItkPluginList();
    this->userFilterpage->getItkPluginList();
}
Dialog :: ~Dialog() {
    delete this->itkPluginManager;
}
//add new function to get the pluginname by search
/*
void Dialog::getAllThePluginsToCall()
{
    QDir pluginToCallDir(dirToSearch);
    this->cdFurtherDir(pluginToCallDir);
    this->showAllTheFile();
}
*/
//the main function to get all the path and the filename
/*
void Dialog::cdFurtherDir (QDir &dir)
{
    dir.setFilter (QDir::NoDotAndDotDot | QDir::AllDirs);
    QStringList dirList = dir.entryList ();
    if (0 != dirList.size ())
    {
        for (int i = 0; i < dirList.size (); i++)
        {
            QDir tempDir(dir);
            tempDir.cd (dirList.at(i));
            this->cdFurtherDir (tempDir);
        }
    }
    dir.setFilter (QDir::Files);
    QStringList fileList = dir.entryList ();
    for (int i = 0; i < fileList.size(); i++)
    {
        QString file = fileList.at(i);
        QStringList fileFinal = file.split (tr("."));
        this->pluginName << fileFinal[0];
        this->pluginPath << dir.absolutePath ()+ "/" + fileList.at(i);
    }
}
//function to show all the filename to check
void Dialog::showAllTheFile()
{
    for ( int i = 0; i < pluginName.count(); i++)
    {
        std::cout << pluginName.at(i).toStdString() << std::endl;
        std::cout << pluginPath.at(i).toStdString() << std::endl;
    }
}
*/
void Dialog::setVaa3DWorkingPluginsDir(const QString& workingDir)
{
  this->m_vaa3DworkingPluginsDir = workingDir;
}

void Dialog::myclose()
{
  qDebug()<<"Xiaoxiao : close";
  qDebug()<<"Xiaoxiao : close";
  qDebug()<<"Xiaoxiao : close";
  qDebug()<<"Xiaoxiao : close";
  qDebug()<<"Xiaoxiao : close";
  
}

void Dialog::onDirChangeButtonClicked()
{
  QString folderName = QFileDialog::getExistingDirectory(this,
                        tr("Get the folder of itk-plugins that can be called"), 
                        m_vaa3DworkingPluginsDir, 
                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                      );
  if (folderName == "") return;

  if (! folderName.startsWith(m_vaa3DworkingPluginsDir) )
  {
    v3d_msg(tr("Error path! The plugins path should under folder plugins"));
    return ;
  }
  qDebug() << "new folder: " << folderName;
  this->m_initialDir = folderName;
  this->intialPluginManager();
}
