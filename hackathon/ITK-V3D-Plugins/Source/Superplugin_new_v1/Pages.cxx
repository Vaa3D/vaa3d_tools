#include "Pages.h"
#include "ItkPluginManager.h"

#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QComboBox>

static QString picName = ":/images/app.png";

//============================================
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

    StartPipe=new QPushButton(tr("Start"));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout -> addStretch(1);
    buttonLayout ->addWidget(StartPipe);


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
    pipelineLayout->addLayout(buttonLayout);
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
    for (int i = 0; i < PaintFilter->count(); i++) {
        QString itkPluginName = PaintFilter->item(i)->text();
        this->itkPluginManager->runItkPluginMenuFunc(itkPluginName);
    }
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
  pipelineExp->clear();
  m_pipeLineList.clear();
  QStringList example1List;
  QStringList example2List; 
  QStringList example3List;
  QStringList example4List;
  QStringList example5List;
  QStringList example6List;

  example1List << QString("CurvatureFlow")
               << QString("ITKConfidenceConnected");
  if (this->validatePluginExits(example1List))
    m_pipeLineList << example1List;

  example2List << QString("CurvatureFlow")
               << QString("ITKConnectedThreshold");
  if (this->validatePluginExits(example2List))
    m_pipeLineList << example2List;

  example3List << QString("CurvatureFlow")
               << QString("ITKIsolatedConnected");
  if (this->validatePluginExits(example3List))
    m_pipeLineList << example3List;
  
  example4List << QString("CurvatureAnisotropicDiffusion")
               << QString("GradientMagnitudeRecursiveGaussian")
               << QString("Sigmoid")
               << QString("ITKFastMarching")
               << QString("BinaryThreshold");
  if (this->validatePluginExits(example4List))
    m_pipeLineList << example4List;
  
  example5List << QString("CurvatureAnisotropicDiffusion")
               << QString("GradientMagnitudeRecursiveGaussian")
               << QString("Sigmoid")
               << QString("ITKShapeDetection")
               << QString("BinaryThreshold");
  if (this->validatePluginExits(example5List))
    m_pipeLineList << example5List;
  
  example6List << QString("CurvatureAnisotropicDiffusion")
               << QString("GradientMagnitudeRecursiveGaussian")
               << QString("Sigmoid")
               << QString("ITKGeodesicActiveContour")
               << QString("BinaryThreshold");
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
  QHash<QString, int> m_pluginsHash = this->itkPluginManager->getItkPluginsHash();
  while (it != pluginList.constEnd())
  {
    if (result && m_pluginsHash.value(*it) != 0) it++;
    else { result = false; break;}
  }
  return result;
}

void AutoPipePage::setItkPluginManager (ItkPluginManager* itkPluginManager) {
    this->itkPluginManager = itkPluginManager;
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
    //SourceFilter->setMaximumWidth(170);
    /*
    QListWidgetItem *Filter1=new QListWidgetItem(SourceFilter);
    Filter1->setText(tr("BinaryThreshold"));
    QListWidgetItem *Filter2=new QListWidgetItem(SourceFilter);
    Filter2->setText(tr("CurvatureFlow"));
    QListWidgetItem *Filter3=new QListWidgetItem(SourceFilter);
    Filter3->setText(tr("DiscreteGaussianFilter"));
    QListWidgetItem *Filter4=new QListWidgetItem(SourceFilter);
    Filter4->setText(tr("GradientMagnitudeRecursiveGaussian"));
    QListWidgetItem *Filter5=new QListWidgetItem(SourceFilter);
    Filter5->setText(tr("Kmeans"));
    QListWidgetItem *Filter6=new QListWidgetItem(SourceFilter);
    Filter6->setText(tr("MeanFilter"));
    QListWidgetItem *Filter7=new QListWidgetItem(SourceFilter);
    Filter7->setText(tr("MedianFilter"));

    QListWidgetItem *Filter8=new QListWidgetItem(SourceFilter);
    Filter8->setText(tr("RescaleIntensity"));
    QListWidgetItem *Filter9=new QListWidgetItem(SourceFilter);
    Filter9->setText(tr("Sigmoid"));
    QListWidgetItem *Filter10=new QListWidgetItem(SourceFilter);
    Filter10->setText(tr("Cast2UINT8"));
    QListWidgetItem *Filter11=new QListWidgetItem(SourceFilter);
    Filter11->setText(tr("ITKFastMarching"));
    QListWidgetItem *Filter12=new QListWidgetItem(SourceFilter);
    Filter12->setText(tr("CurvatureAnisotropicDiffusion"));
    QListWidgetItem *Filter13=new QListWidgetItem(SourceFilter);
    Filter13->setText(tr("ITKConfidenceConnected"));
    QListWidgetItem *Filter14=new QListWidgetItem(SourceFilter);
    Filter14->setText(tr("ITKIsolatedConnected"));
    QListWidgetItem *Filter15=new QListWidgetItem(SourceFilter);
    Filter15->setText(tr("ITKConnectedThreshold"));
    QListWidgetItem *Filter16=new QListWidgetItem(SourceFilter);
    Filter16->setText(tr("ITKShapeDectection"));
    QListWidgetItem *Filter17=new QListWidgetItem(SourceFilter);
    Filter17->setText(tr("ITKGeodesicActiveContour"));
    QListWidgetItem *Filter18=new QListWidgetItem(SourceFilter);
    Filter18->setText(tr("CurvatureAnisotropicDiffusion"));
*/

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

    QHBoxLayout *buttonLayout1 = new QHBoxLayout;
    buttonLayout1->addStretch(1);
    buttonLayout1->addWidget(StartPipe);

    QVBoxLayout *pipeLayout=new QVBoxLayout;
    pipeLayout->addWidget(PaintFilter);
    pipeLayout->addLayout(labelLayout);
    pipeLayout->addLayout(listLayout);
    pipeLayout->addLayout(buttonLayout);
    pipeLayout->addLayout(buttonLayout1);
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
    /*
    	QString end=".so";
    	QString mid="/";
        for(int i=0;i<UsedFilter->count();i++)
        {
    	QString name="ITK/Superplugin/Plugin2Call/";
            QString plugin_name=UsedFilter->item(i)->text();
    	PluginSpecialized<unsigned char> runner(callback);
            name+=plugin_name;
    	name+=mid;
    	name+=plugin_name;
    	name+=end;
    	runner.SetPluginName(name);
    	runner.Execute(menu_name,0);

        }
    	*/
    /*
    	QString end=".so";
    	QString mid="/";
    	PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char >(callback);
        for(int i=0;i<UsedFilter->count();i++)
        {
    	QString name="ITK/Superplugin/Plugin2Call/";
            QString plugin_name=PaintFilter->item(i)->text();
    	//PluginSpecialized<unsigned char> runner(callback);
            name+=plugin_name;
    	name+=mid;
    	name+=plugin_name;
    	name+=end;
    	runner->AddPluginName(name);
    	//runner.SetPluginName(name);
    	//runner.Execute(menu_name,0);
        }
    	runner->SetUsePipeline(true);
    	runner->Execute(menu_name,0);
    	delete runner;	*/
    	for (int i =0; i < this->UsedFilter->count(); i++) {
            QString itkPluginName = this->UsedFilter->item(i)->text();
            this->itkPluginManager->runItkPluginMenuFunc(itkPluginName);
    	}
}
void UserPipePage::ADDItem()
{
    UsedFilter->addItem(new QListWidgetItem(*SourceFilter->currentItem()));
    emit this->countChanged();
}
void UserPipePage::CreatIcon()
{
    PaintFilter->clear();
    for(int i=0; i<UsedFilter->count(); i++)
    {
        QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(UsedFilter->item(i)->text());
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
    }
}
void UserPipePage::setItkPluginManager (ItkPluginManager* itkPluginManager) {
    this->itkPluginManager = itkPluginManager;
}
void UserPipePage::getItkPluginList() {
    this->itkPluginNames = this->itkPluginManager->getAllItkPluginNames();
    this->setSourceFilterList();
}
void UserPipePage::setSourceFilterList() {
    this->SourceFilter->clear();
    for (int i =0; i < this->itkPluginNames.size(); i++) {
        QListWidgetItem* Filter = new QListWidgetItem(this->SourceFilter);
        Filter->setText(itkPluginNames.at(i));
    }
}
/*
void UserPipePage::SetCallback( V3DPluginCallback &callback)
{
    this->callback=&callback;
}
void UserPipePage::setPluginName( const QStringList &pluginName)
{
    this->pluginName = pluginName;
}
void UserPipePage::setPluginPath( const QStringList &pluginPath)
{
    this->pluginPath = pluginPath;
}
*/
//===============================================================
UserFilterPage::UserFilterPage(QWidget *parent):QWidget(parent)
{
    QGroupBox *UserFilter=new QGroupBox(tr("User's Own Filter"));
    FilterName=new QLabel(tr("Filter Name:"));
    FilterItem=new QComboBox;
    /*
    FilterItem->addItem(tr("BinaryThreshold"));
    FilterItem->addItem(tr("Cast2Float"));
    FilterItem->addItem(tr("DiscreteGaussianFilter"));
    FilterItem->addItem(tr("Erode"));
    FilterItem->addItem(tr("CurvatureAnisotropicDiffusion"));
    FilterItem->addItem(tr("Kmeans"));
    FilterItem->addItem(tr("MeanFilter"));
    FilterItem->addItem(tr("MedianFilter"));
    FilterItem->addItem(tr("RescaleIntensity"));
    FilterItem->addItem(tr("Sigmoid"));
    FilterItem->addItem(tr("ITKCannySegmentation"));
    FilterItem->addItem(tr("And"));
    FilterItem->addItem(tr("ITKThresholdSegmentation"));
    FilterItem->addItem(tr("GradientMagnitudeRecursiveGaussian"));
    FilterItem->addItem(tr("ITKFastMarching"));
    FilterItem->addItem(tr("CurvatureFlow"));
    FilterItem->addItem(tr("ITKConfidenceConnected"));
    FilterItem->addItem(tr("ITKIsolatedConnected"));
    FilterItem->addItem(tr("ITKConnectedThreshold"));
    FilterItem->addItem(tr("ITKShapeDetection"));
    FilterItem->addItem(tr("ITKGeodesicActiveContour"));
*/

    StartButton=new QPushButton(tr("Start Call Other Filter"));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout ->addStretch(1);
    buttonLayout ->addWidget(StartButton);


    QHBoxLayout *FilterLayout=new QHBoxLayout;
    FilterLayout->addWidget(FilterName);
    FilterLayout->addWidget(FilterItem);

    QVBoxLayout *UserLayout=new QVBoxLayout;
    UserLayout->addLayout(FilterLayout);
    UserLayout->addSpacing(20);
    UserLayout->addLayout(buttonLayout);
    UserFilter->setLayout(UserLayout);

    QVBoxLayout *mainLayout=new QVBoxLayout;
    mainLayout->addWidget(UserFilter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    connect(StartButton,SIGNAL(clicked()),this,SLOT(CallFilter()));

}
void UserFilterPage::CallFilter()
{
    /*
    	QString name="ITK/Superplugin/Plugin2Call/";
    	QString end=".so";
    	QString mid="/";
    	QString plugin_name=FilterItem->currentText();
    	if(plugin_name=="ITKCannySegmentation"||plugin_name=="And")
    	{
    	PluginSpecializedForDual<unsigned char> runner(callback);
            name+=plugin_name;
    	name+=mid;
    	name+=plugin_name;
    	name+=end;
    	runner.SetPluginName(name);
    	runner.Execute(menu_name,0);
    	}
    	else
    	{
    	PluginSpecialized<unsigned char> runner(callback);
            name+=plugin_name;
    	name+=mid;
    	name+=plugin_name;
    	name+=end;
    	runner.SetPluginName(name);
    	runner.Execute(menu_name,0);
    	}
    	*/
    	QString itkPluginName = this->FilterItem->currentText();
    	this->itkPluginManager->runItkPluginMenuFunc(itkPluginName);
}
void UserFilterPage::setItkPluginManager( ItkPluginManager* itkPluginManager ){
    this->itkPluginManager = itkPluginManager;
}

void UserFilterPage::getItkPluginList() {
    this->itkPluginNames = this->itkPluginManager->getAllItkPluginNames();
    this->setItemList();
}
void UserFilterPage::setItemList() {
    this->FilterItem->clear();
    for ( int i = 0; i < this->itkPluginNames.count(); i++) {
        this->FilterItem->addItem(itkPluginNames.at(i));
    }
}
/*
void UserFilterPage::SetCallback( V3DPluginCallback &callback)
{
    this->callback=&callback;
}
void UserFilterPage::setPluginName( const QStringList &pluginName)
{
    this->pluginName = pluginName;
}
void UserFilterPage::setPluginPath( const QStringList &pluginPath)
{
    this->pluginPath = pluginPath;
}*/

