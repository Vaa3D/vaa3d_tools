
#include <QtGui>
#include <stdio.h>

#include "pages.h"
#include "Superplugin.h"


QString picName=":/images/app.png";

AutoPipePage::AutoPipePage(QWidget *parent):QWidget(parent)
{
    QGroupBox *pipelineGroup = new QGroupBox(tr("Example of Segmentation Pipeline "));

    pipelineName=new QLabel(tr("Segmentation Pipeline:"));
    pipelineExp=new QComboBox;
    pipelineExp->addItem(tr("Example Pipeline1"));
    pipelineExp->addItem(tr("Example Pipeline2"));
    pipelineExp->addItem(tr("Example Pipeline3"));
    pipelineExp->addItem(tr("Example Pipeline4"));
    pipelineExp->addItem(tr("Example Pipeline5"));
    pipelineExp->addItem(tr("Example Pipeline6"));

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
    this->CreateIcon();
    connect(StartPipe,SIGNAL(clicked()),this,SLOT(CallPipeline()));
    connect(pipelineExp,SIGNAL(currentIndexChanged(int)),this,SLOT(CreateIcon()));
}
void AutoPipePage::CallPipeline()
{

    for(int i=0;i<PaintFilter->count();i++)
    {
      QString pluginName=PaintFilter->item(i)->text();
      QString pluginFileName = m_pluginsHash.value(pluginName);
	    PluginSpecialized<unsigned char> runner(callback);
	    runner.SetPluginName(pluginFileName, pluginName);
	    runner.Execute(menu_name,0);
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
    if(i==0){QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(tr("CurvatureFlow_called"));
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
        QListWidgetItem * item1=new QListWidgetItem;
        item1->setIcon(QIcon(picName));
        item1->setText(tr("ITKConfidenceConnected_called"));
        item1->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item1);

        }
    else if(i==1){
        QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(tr("CurvatureFlow_called"));
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
        QListWidgetItem * item1=new QListWidgetItem;
        item1->setIcon(QIcon(picName));
        item1->setText(tr("ITKConnectedThreshold_called"));
        item1->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item1);
    }
    else if(i==2){
        QListWidgetItem * item=new QListWidgetItem;
        item->setIcon(QIcon(picName));
        item->setText(tr("CurvatureFlow_called"));
        item->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item);
        QListWidgetItem * item1=new QListWidgetItem;
        item1->setIcon(QIcon(picName));
        item1->setText(tr("ITKIsolatedConnected_called"));
        item1->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item1);

    }
    else if(i==3){
        QListWidgetItem * item1=new QListWidgetItem;
        item1->setIcon(QIcon(picName));
        item1->setText(tr("CurvatureAnisotropicDiffusion_called"));
        item1->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item1);
        QListWidgetItem * item2=new QListWidgetItem;
        item2->setIcon(QIcon(picName));
        item2->setText(tr("GradientMagnitudeRecursiveGaussian_called"));
        item2->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item2);
        QListWidgetItem * item3=new QListWidgetItem;
        item3->setIcon(QIcon(picName));
        item3->setText(tr("Sigmoid_called"));
        item3->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item3);
        QListWidgetItem * item4=new QListWidgetItem;
        item4->setIcon(QIcon(picName));
        item4->setText(tr("ITKFastMarching_called"));
        item4->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item4);
        QListWidgetItem * item5=new QListWidgetItem;
        item5->setIcon(QIcon(picName));
        item5->setText(tr("BinaryThreshold_called"));
        item5->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item5);
    }
    else if(i==4){
        QListWidgetItem * item1=new QListWidgetItem;
        item1->setIcon(QIcon(picName));
        item1->setText(tr("CurvatureAnisotropicDiffusion_called"));
        item1->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item1);
        QListWidgetItem * item2=new QListWidgetItem;
        item2->setIcon(QIcon(picName));
        item2->setText(tr("GradientMagnitudeRecursiveGaussian_called"));
        item2->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item2);
        QListWidgetItem * item3=new QListWidgetItem;
        item3->setIcon(QIcon(picName));
        item3->setText(tr("Sigmoid_called"));
        item3->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item3);
        QListWidgetItem * item4=new QListWidgetItem;
        item4->setIcon(QIcon(picName));
        item4->setText(tr("ITKShapeDectection_called"));
        item4->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item4);
        QListWidgetItem * item5=new QListWidgetItem;
        item5->setIcon(QIcon(picName));
        item5->setText(tr("BinaryThreshold_called"));
        item5->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item5);
    }
    else if(i==5){
        QListWidgetItem * item1=new QListWidgetItem;
        item1->setIcon(QIcon(picName));
        item1->setText(tr("CurvatureAnisotropicDiffusion_called"));
        item1->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item1);
        QListWidgetItem * item2=new QListWidgetItem;
        item2->setIcon(QIcon(picName));
        item2->setText(tr("GradientMagnitudeRecursiveGaussian_called"));
        item2->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item2);
        QListWidgetItem * item3=new QListWidgetItem;
        item3->setIcon(QIcon(picName));
        item3->setText(tr("Sigmoid_called"));
        item3->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item3);
        QListWidgetItem * item4=new QListWidgetItem;
        item4->setIcon(QIcon(picName));
        item4->setText(tr("ITKGeodesicActiveContour_called"));
        item4->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item4);
        QListWidgetItem * item5=new QListWidgetItem;
        item5->setIcon(QIcon(picName));
        item5->setText(tr("BinaryThreshold_called"));
        item5->setTextAlignment(Qt::AlignLeft);
        PaintFilter->addItem(item5);
    }
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
      QString pluginFileName = m_pluginsHash.value(pluginName);
	    PluginSpecialized<unsigned char> runner(callback);
	    runner.SetPluginName(pluginFileName, pluginName);
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
    QString name = m_pluginsHash.value(plugin_name);
	  runner.SetPluginName(name, plugin_name);
	  runner.Execute(menu_name,0);
	}
	else
	{ 
	  PluginSpecialized<unsigned char> runner(callback);	
    QString name = m_pluginsHash.value(plugin_name);
	  runner.SetPluginName(name, plugin_name);
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
    for ( int i = 0; i < this->m_pluginNames.count(); i++) {
        this->FilterItem->addItem(m_pluginNames.at(i));
    }
}
