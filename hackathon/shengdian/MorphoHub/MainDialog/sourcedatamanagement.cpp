#include "sourcedatamanagement.h"
using namespace std;
SourceDataManagement::SourceDataManagement(const QString &path, QWidget *parent)
 :QDialog(parent)
{
    setupDBpath(path);
    setWindowTitle(tr("MorphoHub-SourceData-Management"));
    SDMInit();
    createTabView();
    createActions();
    mainLayout=new QGridLayout;
    mainLayout->addWidget(tabwidget,0,0,8,6);
    mainLayout->addWidget(functionGroupBox,0,6,4,2);
    mainLayout->addWidget(logTextEdit,4,6,4,2);
    setLayout(mainLayout);
}
void SourceDataManagement::createActions()
{
    functionGroupBox=new QGroupBox(tr("Operation Buttons"));
    functionLayout=new QVBoxLayout;
    saveButton=new QPushButton(tr("Save"));
    connect(saveButton,SIGNAL(clicked()),this,SLOT(saveButton_slot()));
    newSDButton=new QPushButton(tr("NewRow"));
    connect(newSDButton,SIGNAL(clicked()),this,SLOT(newSDButton_slot()));
    deleteSDButton=new QPushButton(tr("DaleteRow"));
    connect(deleteSDButton,SIGNAL(clicked()),this,SLOT(deleteSDButton_slot()));
    newSDitemButton=new QPushButton(tr("NewColumn"));
    deleteSDitemButton=new QPushButton(tr("DeleteColumn"));
    loadSomalist2Conf=new QPushButton(tr("SomaToConf"));
    connect(loadSomalist2Conf,SIGNAL(clicked()),this,SLOT(loadSomalist2Conf_slot()));
    functionLayout->addWidget(saveButton);
    functionLayout->addWidget(newSDButton);
    functionLayout->addWidget(deleteSDButton);
    functionLayout->addWidget(newSDitemButton);
    functionLayout->addWidget(deleteSDitemButton);
    functionGroupBox->setLayout(functionLayout);
}

void SourceDataManagement::createTabView()
{
    tabwidget=new QTabWidget();
    tabwidget->setTabsClosable(true);
    sourcedataTab=createTableSourceData();
    tabwidget->addTab(sourcedataTab,tr("SourceData"));
    //set close function
    connect(tabwidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int)));
}
void SourceDataManagement::removeSubTab(int subindex)
{
    tabwidget->removeTab(subindex);
}
QTableWidget* SourceDataManagement::createTableSourceData()
{
    //scan sourcedata.conf
    //get every cloum
    QStringList title=this->sourcedataconfTitle;
    QTableWidget* sdtablewidget;
    int col = title.size();
    int  row=1;
    if(title.size()>0)
    {
        sdtablewidget= new QTableWidget(row,col, this);
        sdtablewidget->setHorizontalHeaderLabels(title);
        QList<SourceDataInfo> tempsdlist=getSourceDatalist(this->sourcedataconfpath);
        if(tempsdlist.size())
        {
            for(int i=0;i<tempsdlist.size();i++)
            {
                SourceDataInfo tempsdi=tempsdlist.at(i);
                QTableWidgetItem *tmpcheckbox=new QTableWidgetItem();
                tmpcheckbox->setCheckState(Qt::Unchecked);
                //foreach item
                sdtablewidget->setItem(i,0,tmpcheckbox);
                sdtablewidget->setItem(i,1,new QTableWidgetItem(tempsdi.SdataID));
                sdtablewidget->setItem(i,2,new QTableWidgetItem(tempsdi.SDSize));
                sdtablewidget->setItem(i,3,new QTableWidgetItem(tempsdi.SDResolution));
                sdtablewidget->setItem(i,4,new QTableWidgetItem(tempsdi.SDState));
                sdtablewidget->setItem(i,5,new QTableWidgetItem(tempsdi.SDBit));
                sdtablewidget->setItem(i,6,new QTableWidgetItem(tempsdi.SDComments));
                sdtablewidget->setItem(i,7,new QTableWidgetItem(tempsdi.SDPriority));
                sdtablewidget->setItem(i,8,new QTableWidgetItem(tempsdi.SDLabeledSomaNumber));
            }
        }
        sdtablewidget->setEditTriggers(QAbstractItemView::DoubleClicked);
//        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        sdtablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
        sdtablewidget->resizeColumnsToContents();
        sdtablewidget->resizeRowsToContents();
        connect(sdtablewidget,SIGNAL(cellClicked(int,int)),this,SLOT(createSomaTab_slot(int,int)));
    }
    return sdtablewidget;
}
QTableWidget* SourceDataManagement::createTableSomaConf(const QString& somaconfpath)
{
    //scan soma.conf
    //get every cloum
    QStringList title=this->somaconfTitle;
    QTableWidget* sdtablewidget;
    int col = title.size();
    int  row=1;
    if(title.size()>0)
    {
        sdtablewidget= new QTableWidget(row,col, this);
        sdtablewidget->setHorizontalHeaderLabels(title);
        QList<SomaConfInfo> tempsdlist=getSomaConf(somaconfpath);
        if(tempsdlist.size())
        {
            for(int i=0;i<tempsdlist.size();i++)
            {
                SourceDataInfo tempsdi=tempsdlist.at(i);
                QTableWidgetItem *tmpcheckbox=new QTableWidgetItem();
                tmpcheckbox->setCheckState(Qt::Unchecked);
                //foreach item
                sdtablewidget->setItem(i,0,tmpcheckbox);
                sdtablewidget->setItem(i,1,new QTableWidgetItem(tempsdi.SdataID));
                sdtablewidget->setItem(i,2,new QTableWidgetItem(tempsdi.SDSize));
                sdtablewidget->setItem(i,3,new QTableWidgetItem(tempsdi.SDResolution));
                sdtablewidget->setItem(i,4,new QTableWidgetItem(tempsdi.SDState));
                sdtablewidget->setItem(i,5,new QTableWidgetItem(tempsdi.SDBit));
                sdtablewidget->setItem(i,6,new QTableWidgetItem(tempsdi.SDComments));
                sdtablewidget->setItem(i,7,new QTableWidgetItem(tempsdi.SDPriority));
                sdtablewidget->setItem(i,8,new QTableWidgetItem(tempsdi.SDLabeledSomaNumber));
            }
        }
        sdtablewidget->setEditTriggers(QAbstractItemView::DoubleClicked);
//        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        sdtablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
        sdtablewidget->setContextMenuPolicy(Qt::CustomContextMenu);
        sdtablewidget->resizeColumnsToContents();
        sdtablewidget->resizeRowsToContents();
        connect(sdtablewidget,SIGNAL(cellClicked(int,int)),this,SLOT(createSomaTab_slot(int,int)));
    }
    return sdtablewidget;
}

void SourceDataManagement::SDMInit()
{
    logTextEdit=new QTextEdit(this);
    logTextEdit->setText("Welcome to Source data management");
    sourcedataconfTitle <<"Select"
                        <<"SdataID"
                        <<"Size"
                        <<"Resolution"
                        <<"State"
                        <<"Bit"
                        <<"Comments"
                        <<"Priority"
                        <<"LabeledSomaNumber"
                       <<"Somalist";

    somaconfTitle<<"SomaID"
                 <<"SdataID"
                 <<"CellName"
                 <<"Zprojection"
                 <<"Xcoordinate"
                 <<"Ycoordinate"
                 <<"Comments"
                 <<"State"
                 <<"SomaLocation"
                 <<"CellType";
    QFile sourcedataconffile(this->sourcedataconfpath);
    if(!sourcedataconffile.exists())
    {
        if(sourcedataconffile.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QString data=sourcedataconfTitle[0];
            for(int i=1;i<sourcedataconfTitle.size();i++)
            {
                data=data+","+sourcedataconfTitle[i];
            }
            sourcedataconffile.write(data.toAscii());
            sourcedataconffile.close();
            toLog("Make a new source data configuration file at Configuration/BasicData_Conf/sourceData.conf");
            toLog(data);
        }
    }
}


void SourceDataManagement::newSDButton_slot()
{
    //This button only open its function to source data management
    int tabindex=tabwidget->currentIndex();
    if(tabindex==0)
    {
        bool ok;
        QString newSdataID=QInputDialog::getText(this,tr("New Source Data"),tr("please input a new source data ID :"),QLineEdit::Normal,"12345",&ok);
        if(ok)
        {
            int currow=sourcedataTab->rowCount();
            sourcedataTab->insertRow(currow);
            //init
            QTableWidgetItem *tmpcheckbox=new QTableWidgetItem();
            tmpcheckbox->setCheckState(Qt::Unchecked);
            //foreach item
            sourcedataTab->setItem(currow,0,tmpcheckbox);
            sourcedataTab->setItem(currow,1,new QTableWidgetItem(newSdataID));
        }
    }
}
void SourceDataManagement::saveButton_slot()
{
    //save changes from source data tab to conf file.

}
void SourceDataManagement::deleteSDButton_slot()
{

}
void SourceDataManagement::loadSomalist2Conf_slot()
{
    QString title="choose soma list file (apo): ";
    QString somaapofilepath=QFileDialog::getOpenFileName(this, title,
                                                    "",
                                                    QObject::tr("soma marker file (*.apo *.APO)"
                                                                ));
    QFile somaapofile(somaapofilepath);
    if(somaapofile.exists())
    {
        QList <CellAPO> cellapolist=readAPO_file(somaapofilepath);
    }
}

void SourceDataManagement::createSomaTab_slot(int row, int column)
{
    if(column==1)
    {
        QString tabtile=sourcedataTab->item(row,1)->text();
        QString somaconfpath=this->dbpath+"/Configuration/BasicData_Conf/"+tabtile+"_soma.conf";
        QFile somaconffile(somaconfpath);
        if(somaconffile.exists())
        {
            QTableWidget* somaconfTab=createTableSomaConf(somaconfpath);
            if(somaconfTab)
                tabwidget->addTab(somaconfTab,tabtile);
        }
        else
        {
            QMessageBox::warning(this,"File Not Found","Can't find configuration file of this data!");
            toLog("hint:");
            toLog("Maybe you need make or upload a soma configuration for this data.");
        }
    }
    else if(column==8)
    {
        //make a new somalist
        int labelednum=sourcedataTab->item(row,column)->text().toInt();
        QString tabtile=sourcedataTab->item(row,1)->text();
        QString somaconfpath=this->dbpath+"/Configuration/BasicData_Conf/"+tabtile+"_soma.conf";
        QFile somaconffile(somaconfpath);
        if(labelednum==0&&(!somaconffile.exists()))
        {
            toLog(tabtile);
            if(somaconffile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QString data=somaconfTitle[0];
                for(int i=1;i<somaconfTitle.size();i++)
                {
                    data=data+","+somaconfTitle[i];
                }
                somaconffile.write(data.toAscii());
                somaconffile.close();
                toLog("Make a new source data configuration file at Configuration/BasicData_Conf/");
                toLog(data);
            }
        }
    }
}

void SourceDataManagement::setupDBpath(const QString &path)
{
    if(path.isEmpty())
    {
        QMessageBox::warning(this,tr("Path Error"),tr("please setup dbpath"));
        return;
    }
    this->dbpath=path;
    this->sourcedataconfpath=this->dbpath+"/Configuration/BasicData_Conf/sourceData.conf";
}
void SourceDataManagement::toLog(const QString &logtext)
{
    QString getlogtext=logTextEdit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logTextEdit->setText(showText);
}
SourceDataManagement::SourceDataManagement()
{
}
SourceDataManagement::~SourceDataManagement()
{

}
