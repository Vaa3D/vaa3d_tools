#include "dialog.h"
#include "pages.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QPluginLoader>
#include <QRegExp>
#include <iostream>

static const QString pic1=":/images/pic1.png";
static const QString pic2=":/images/pic2.png";
static const QString pic3=":/images/pic4.png";

Dialog::Dialog(QWidget* parent) : QDialog(parent)
{
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(96, 84));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(128);
    contentsWidget->setSpacing(12);

    pagesWidget = new QStackedWidget(this);
    autoPipepage=new AutoPipePage(this);
    userPipepage=new UserPipePage(this);
    userFilterpage=new UserFilterPage(this);
    pagesWidget->addWidget(autoPipepage);
    pagesWidget->addWidget(userPipepage);
    pagesWidget->addWidget(userFilterpage);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    m_dirChangeButton = new QPushButton(tr("change the origin search dir"), this);

    createIcons();
    contentsWidget->setCurrentRow(0);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
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
	  this->move(1500,0);//move the dialog so that one can see the progress dialog
    setWindowTitle(tr("SuperPlugin Pipeline"));
    setWindowFlags(this->windowFlags() & Qt::WindowMaximizeButtonHint & Qt::WindowMinimizeButtonHint);
}
void Dialog::createIcons()
{
    QListWidgetItem *configButton = new QListWidgetItem(contentsWidget);
    configButton->setIcon(QIcon(pic1));
    configButton->setText(tr("Auto Pipeline"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *updateButton = new QListWidgetItem(contentsWidget);
    updateButton->setIcon(QIcon(pic2));
    updateButton->setText(tr("User Pipeline"));
    updateButton->setTextAlignment(Qt::AlignHCenter);
    updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *queryButton = new QListWidgetItem(contentsWidget);
    queryButton->setIcon(QIcon(pic3));
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
void Dialog::SetCallback(V3DPluginCallback &callback)
{
	this->userFilterpage->SetCallback(callback);
	this->userPipepage->SetCallback(callback);
	this->autoPipepage->SetCallback(callback);
}
void Dialog::setInitialDir(const QString& initialDir) {
  this->m_initialDir = initialDir;
}
bool Dialog::searchAllItkPlugins()
{
  this->m_pluginNames.clear();
  this->m_pluginsHash.clear();
  QDir initDir(this->m_initialDir);
  try {
    this->getItkPluginFiles(initDir);
  } catch (...) { return false;}
  return true;  
}
void Dialog::getItkPluginFiles( QDir & dir)
{
    dir.setFilter (QDir::NoDotAndDotDot | QDir::AllDirs);
    QStringList dirList = dir.entryList ();
    if (0 != dirList.size ())
    {
        for (int i = 0; i < dirList.size (); i++)
        {
            QDir tempDir(dir);
            tempDir.cd (dirList.at(i));
            this->getItkPluginFiles (tempDir);
        }
    }
    dir.setFilter (QDir::Files);
    QStringList fileList = dir.entryList ();
    //get the plugin name that show on the superplugin
    QRegExp rxFile = QRegExp("(lib)?([^.]+)\\..*");
    for (int i = 0; i < fileList.size(); i++)
    {
        QString file = fileList.at(i);
        QString pluginPath = QDir::fromNativeSeparators(dir.absolutePath ()+ "/" + fileList.at(i));
        QPluginLoader* itkPluginLoader = new QPluginLoader(pluginPath);
        QObject* itkPlugin = itkPluginLoader -> instance();
        if ( !itkPlugin )
        {
            std::cerr << "Can't load the ITK plugin : " << fileList.at(i).toStdString() << " !" << std::endl;
            return;
        }
        else
        {
            if (rxFile.indexIn(fileList.at(i)) >= 0) {
              QString pluginName = rxFile.cap(2);
              this->m_pluginNames << pluginName;
              m_pluginsHash.insert(pluginName, pluginPath);
            //free the memory or useing delete itkPlugin
              itkPluginLoader -> unload ();
            }
            else { 
              itkPluginLoader->unload();
              continue;
            }
        }

        delete itkPluginLoader;
    }
}
void Dialog::initial()
{
  this->searchAllItkPlugins();
  if (m_pluginNames.size() == 0) {
    v3d_msg(tr("Empty plugins under Superplugin, you should move all plugins that can be called by SuperPlugin to plugins/ITK/SuperPlugin/Plugin2Call/"));
  }
  this->userPipepage->setPluginNames(m_pluginNames);
  this->userPipepage->setPluginsHash(m_pluginsHash);
  
  this->userFilterpage->setPluginNames(m_pluginNames);
  this->userFilterpage->setPluginsHash(m_pluginsHash);
  
  this->autoPipepage->setPluginsHash(m_pluginsHash);
  this->autoPipepage->initialTest();
}
QStringList Dialog::getPluginNames()
{
  return m_pluginNames;
}
QHash<QString, QString> Dialog::getPluginsHash() 
{
  return m_pluginsHash;
}
void Dialog::setVaa3DWorkingPluginsDir(const QString& workingDir)
{
  this->m_vaa3DworkingPluginsDir = workingDir;
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
  this->initial();
}

