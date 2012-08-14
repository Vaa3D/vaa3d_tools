
#include <QtGui>
#include <stdio.h>

#include "pages.h"
#include "Superplugin.h"


static const QString picName=":/images/app.png";

AutoPipePage::AutoPipePage(QWidget *parent):QWidget(parent)
{
    QGroupBox *pipelineGroup = new QGroupBox(tr("Example of Segmentation Pipeline "));

    pipelineName=new QLabel(tr("Segmentation Pipeline:"));
    pipelineExp=new QComboBox;
    //pipelineExp->addItem(tr("Example Pipeline1"));
    //pipelineExp->addItem(tr("Example Pipeline2"));
    //pipelineExp->addItem(tr("Example Pipeline3"));
    //pipelineExp->addItem(tr("Example Pipeline4"));
    //pipelineExp->addItem(tr("Example Pipeline5"));
    //pipelineExp->addItem(tr("Example Pipeline6"));

    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(pipelineName);
    serverLayout->addWidget(pipelineExp);

    StartPipe=new QPushButton(tr("Start the Pipeline"));

    PaintFilter=new QListWidget;
    PaintFilter->setViewMode(QListView::IconMode);
    PaintFilter->setIconSize(QSize(100, 100));
    PaintFilter->setMovement(QListView::Static);
    PaintFilter->setMaximumHeight(250);

    QVBoxLayout *pipelineLayout=new QVBoxLayout;
    pipelineLayout->addWidget(PaintFilter);
    pipelineLayout->addSpacing(12);
    pipelineLayout->addLayout(serverLayout);
    pipelineLayout->addSpacing(12);
    pipelineLayout->addWidget(StartPipe);
    pipelineLayout->addSpacing(12);
    pipelineGroup->setLayout(pipelineLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pipelineGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
    //this->CreateIcon();
    connect(StartPipe,SIGNAL(clicked()),this,SLOT(CallPipeline()));
    //connect(pipelineExp,SIGNAL(currentIndexChanged(int)),this,SLOT(CreateIcon()));
}
void AutoPipePage::CallPipeline()
{
  int i = this->pipelineExp->currentIndex();
  if ( i == 0 ) {
    v3d_msg(tr("example to count the number of the cells"));
    qDebug() << "use pipeline function to do this";
    PluginSpecialized< unsigned char > runner(callback);
    for (int i = 0; i < PaintFilter->count(); i ++)
    {
     QString pluginName = PaintFilter->item(i)->text();
     QString pluginPath = m_pluginsHash.value(pluginName);
     runner.AddPluginName(pluginPath, pluginName);
    }
    runner.SetUsePipeline(true);
    runner.SetPluginsHash(m_pluginsHash);
    runner.Execute(menu_name, 0);
  }
  else {

    for(int i=0;i<PaintFilter->count();i++)
    {
      QString pluginName=PaintFilter->item(i)->text();
      QString pluginPath = m_pluginsHash.value(pluginName);
	    PluginSpecialized<unsigned char> runner(callback);
	    runner.SetPluginName(pluginPath, pluginName);
      runner.SetPluginsHash(m_pluginsHash);
	    runner.Execute(menu_name,0);
    }	
  }
}
void AutoPipePage::setPluginsHash(const QHash<QString, QString>& pluginsHash)
{
  m_pluginsHash = pluginsHash;
} 
void AutoPipePage::CreateIcon()
{
    PaintFilter->clear();
    int i=pipelineExp->currentIndex();
    if (i < 0) return;
    QStringList pluginList = m_pipeLineList.at(i);
    for (int i = 0; i < pluginList.size(); i++ )
    {
        QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(pluginList.at(i));
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
    }
}
void AutoPipePage::initialTest()
{
  m_pipeLineList.clear();
  pipelineExp->clear();
  QStringList example0List;
  QStringList example1List;
  QStringList example2List; 
  QStringList example3List;
  QStringList example4List;
  QStringList example5List;
  QStringList example6List;

  example0List << QString ("CurvatureFlow_called")
               << QString ("GradientMagnitudeRecursiveGaussian_called")
               << QString ("ITKWatershed_called");
  if (this->validatePluginExits(example0List))
    m_pipeLineList << example0List;
  example1List << QString("CurvatureFlow_called")
               << QString("ITKConfidenceConnected_called");
  if (this->validatePluginExits(example1List))
    m_pipeLineList << example1List;

  example2List << QString("CurvatureFlow_called")
               << QString("ITKConnectedThreshold_called");
  if (this->validatePluginExits(example2List))
    m_pipeLineList << example2List;

  example3List << QString("CurvatureFlow_called")
               << QString("ITKIsolatedConnected_called");
  if (this->validatePluginExits(example3List))
    m_pipeLineList << example3List;
  
  example4List << QString("CurvatureAnisotropicDiffusion_called")
               << QString("GradientMagnitudeRecursiveGaussian_called")
               << QString("Sigmoid_called")
               << QString("ITKFastMarching_called")
               << QString("BinaryThreshold_called");
  if (this->validatePluginExits(example4List))
    m_pipeLineList << example4List;
  
  example5List << QString("CurvatureAnisotropicDiffusion_called")
               << QString("GradientMagnitudeRecursiveGaussian_called")
               << QString("Sigmoid_called")
               << QString("ITKShapeDetection_called")
               << QString("BinaryThreshold_called");
  if (this->validatePluginExits(example5List))
    m_pipeLineList << example5List;
  
  example6List << QString("CurvatureAnisotropicDiffusion_called")
               << QString("GradientMagnitudeRecursiveGaussian_called")
               << QString("Sigmoid_called")
               << QString("ITKGeodesicActiveContour_called")
               << QString("BinaryThreshold_called");
  if (this->validatePluginExits(example6List))
    m_pipeLineList << example6List;

  for (int i = 0; i < m_pipeLineList.size(); i++)
  {
    pipelineExp->addItem(QString("Example Pipeline%1").arg(i+1));
  }

  this->CreateIcon();
  connect(pipelineExp,SIGNAL(currentIndexChanged(int)),this,SLOT(CreateIcon()));
}
bool AutoPipePage::validatePluginExits(const QStringList& pluginList)
{
  QStringList::const_iterator it = pluginList.constBegin();
  bool result = true;
  while (it != pluginList.constEnd())
  {
    if (result && m_pluginsHash.value(*it) != "") it++;
    else { result = false; break;}
  }
  return result;
}
void AutoPipePage::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}

//=============================================================
UserPipePage::UserPipePage(QWidget *parent):QWidget(parent)
{
    QGroupBox *UserPipe=new QGroupBox(tr("User's Own Segmentation Pipeline"));
    PaintFilter=new QListWidget;
    PaintFilter->setViewMode(QListView::IconMode);
    PaintFilter->setIconSize(QSize(90, 90));
    PaintFilter->setMovement(QListView::Static);
    PaintFilter->setMaximumHeight(250);

    StartPipe=new QPushButton("Start PipeLine");
    SourceLabel=new QLabel(tr("Source Filter"));
    UsedLabel=new QLabel(tr("Used Filter"));

    AddItem=new QPushButton(tr("Add Filter"));
    ClearItem=new QPushButton(tr("Clear All"));

    SourceFilter=new QListWidget;

    UsedFilter=new QListWidget;
    //UsedFilter->setMaximumWidth(170);

    QHBoxLayout *listLayout=new QHBoxLayout;
    listLayout->addWidget(SourceFilter);
    listLayout->addWidget(UsedFilter);
    QHBoxLayout *labelLayout=new QHBoxLayout;
    labelLayout->addWidget(SourceLabel);
    labelLayout->addWidget(UsedLabel);
    QHBoxLayout *buttonLayout=new QHBoxLayout;
    buttonLayout->addWidget(AddItem);
    buttonLayout->addWidget(ClearItem);

    QVBoxLayout *pipeLayout=new QVBoxLayout;
    pipeLayout->addWidget(PaintFilter);
    pipeLayout->addLayout(labelLayout);
    pipeLayout->addLayout(listLayout);
    pipeLayout->addLayout(buttonLayout);
    pipeLayout->addWidget(StartPipe);
    UserPipe->setLayout(pipeLayout);

    QVBoxLayout *mainLayout=new QVBoxLayout;
    mainLayout->addWidget(UserPipe);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    connect(AddItem,SIGNAL(clicked()),this,SLOT(ADDItem()));
    connect(ClearItem,SIGNAL(clicked()),UsedFilter,SLOT(clear()));
    connect(this,SIGNAL(countChanged()),this,SLOT(CreatIcon()));
    connect(ClearItem,SIGNAL(clicked()),PaintFilter,SLOT(clear()));
    connect(StartPipe,SIGNAL(clicked()),this,SLOT(CallPipeline()));
}
void UserPipePage::CallPipeline()
{
    for(int i=0;i<PaintFilter->count();i++)
    {
      QString pluginName=UsedFilter->item(i)->text();
      QString pluginPath = m_pluginsHash.value(pluginName);
	    PluginSpecialized<unsigned char> runner(callback);
	    runner.SetPluginName(pluginPath, pluginName);
      runner.SetPluginsHash(m_pluginsHash);
	    runner.Execute(menu_name,0);
    }	
}
	
void UserPipePage::ADDItem()
{
    UsedFilter->addItem(new QListWidgetItem(*SourceFilter->currentItem()));
    emit this->countChanged();
}
void UserPipePage::setPluginsHash(const QHash<QString, QString>& pluginsHash)
{
  m_pluginsHash = pluginsHash;
} 
void UserPipePage::setPluginNames(const QStringList& pluginNames)
{
  m_pluginNames = pluginNames;
  this->setSourceFilterList();
}
void UserPipePage::CreatIcon()
{
    PaintFilter->clear();
    for(int i=0;i<UsedFilter->count();i++)
    {
        QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(UsedFilter->item(i)->text());
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
    }
}
void UserPipePage::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}
void UserPipePage::setSourceFilterList() {
    this->SourceFilter->clear();
    for (int i =0; i < this->m_pluginNames.size(); i++) {
        QListWidgetItem* Filter = new QListWidgetItem(this->SourceFilter);
        Filter->setText(m_pluginNames.at(i));
    }
}

//===============================================================
UserFilterPage::UserFilterPage(QWidget *parent):QWidget(parent)
{
    QGroupBox *UserFilter=new QGroupBox(tr("User's Own Filter"));
    FilterName=new QLabel(tr("Filter Name:"));
    FilterItem=new QComboBox;
				
    StartButton=new QPushButton(tr("Start Call Other Filter"));
    QHBoxLayout *FilterLayout=new QHBoxLayout;
    FilterLayout->addWidget(FilterName);
    FilterLayout->addWidget(FilterItem);

    QVBoxLayout *UserLayout=new QVBoxLayout;
    UserLayout->addLayout(FilterLayout);
    UserLayout->addSpacing(20);
    UserLayout->addWidget(StartButton);
    UserFilter->setLayout(UserLayout);

    QVBoxLayout *mainLayout=new QVBoxLayout;
    mainLayout->addWidget(UserFilter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    connect(StartButton,SIGNAL(clicked()),this,SLOT(CallFilter()));

}
void UserFilterPage::CallFilter()
{
	QString plugin_name=FilterItem->currentText();
	if(plugin_name=="ITKCannySegmentation"||plugin_name=="And")
	{
    PluginSpecializedForDual<unsigned char> runner(callback);
    QString pluginPath = m_pluginsHash.value(plugin_name);
	  runner.SetPluginName(pluginPath, plugin_name);
    runner.SetPluginsHash(m_pluginsHash);
	  runner.Execute(menu_name,0);
	}
	else
	{ 
	  PluginSpecialized<unsigned char> runner(callback);	
    QString pluginPath = m_pluginsHash.value(plugin_name);
	  runner.SetPluginName(pluginPath, plugin_name);
    runner.SetPluginsHash(m_pluginsHash);
	  runner.Execute(menu_name,0);
	}
}

void UserFilterPage::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}
void UserFilterPage::setPluginsHash(const QHash<QString, QString>& pluginsHash)
{
  m_pluginsHash = pluginsHash;
} 
void UserFilterPage::setPluginNames(const QStringList& pluginNames)
{
  m_pluginNames = pluginNames;
  this->setItemList();
}
void UserFilterPage::setItemList() {
    this->FilterItem->clear();
    for ( int i = 0; i < this->m_pluginNames.count(); i++) {
        this->FilterItem->addItem(m_pluginNames.at(i));
    }
}
