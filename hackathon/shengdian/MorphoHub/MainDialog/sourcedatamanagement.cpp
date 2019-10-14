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
    saveButton->setEnabled(false);
    connect(saveButton,SIGNAL(clicked()),this,SLOT(saveButton_slot()));
    newSDButton=new QPushButton(tr("NewRow"));
    connect(newSDButton,SIGNAL(clicked()),this,SLOT(newSDButton_slot()));
    deleteSDButton=new QPushButton(tr("DaleteRow"));
    connect(deleteSDButton,SIGNAL(clicked()),this,SLOT(deleteSDButton_slot()));
    newSDitemButton=new QPushButton(tr("NewColumn"));
    newSDitemButton->setEnabled(false);
    deleteSDitemButton=new QPushButton(tr("DeleteColumn"));
    deleteSDitemButton->setEnabled(false);
    loadSomalist2Conf=new QPushButton(tr("UploadSomalistToConf"));
    loadSomalist2Conf->setEnabled(false);
    //enable this button when somatab widget is on.
    connect(loadSomalist2Conf,SIGNAL(clicked()),this,SLOT(loadSomalist2Conf_slot()));
    functionLayout->addWidget(saveButton);
    functionLayout->addWidget(newSDButton);
    functionLayout->addWidget(deleteSDButton);
    functionLayout->addWidget(newSDitemButton);
    functionLayout->addWidget(deleteSDitemButton);
    functionLayout->addWidget(loadSomalist2Conf);
    functionGroupBox->setLayout(functionLayout);
}

void SourceDataManagement::createTabView()
{
    tabwidget=new QTabWidget();
    tabwidget->setTabsClosable(false);
    sourcedataTab=createTableSourceData();
    tabwidget->addTab(sourcedataTab,tr("SourceData"));
    //set close function
    connect(tabwidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int)));
    connect(tabwidget,SIGNAL(currentChanged(int)),this,SLOT(switchTab(int)));
}
void SourceDataManagement::removeSubTab(int subindex)
{
    tabwidget->removeTab(subindex);
}
void SourceDataManagement::switchTab(int toindex)
{
    if(toindex==0)
    {
        loadSomalist2Conf->setEnabled(false);
        newSDButton->setEnabled(true);
        deleteSDButton->setEnabled(true);
    }
    else
    {
        saveButton->setEnabled(false);
        newSDButton->setEnabled(false);
        deleteSDButton->setEnabled(false);
        loadSomalist2Conf->setEnabled(true);
    }
    tabwidget->setCurrentIndex(toindex);
}
QTableWidget* SourceDataManagement::createTableSourceData()
{
    //scan sourcedata.conf
    //get every cloum
    QStringList title=this->sourcedataconfTitle;
    QTableWidget* sdtablewidget;
    int col = title.size();
    int  row=0;
    if(title.size()>0)
    {
        QList<SourceDataInfo> tempsdlist=getSourceDatalist(this->sourcedataconfpath);
        row=tempsdlist.size();
        sdtablewidget= new QTableWidget(row,col, this);
        sdtablewidget->setHorizontalHeaderLabels(title);
        if(tempsdlist.size())
        {
            for(int i=0;i<tempsdlist.size();i++)
            {
                SourceDataInfo tempsdi=tempsdlist.at(i);
//                //foreach item
                sdtablewidget->setItem(i,0,new QTableWidgetItem(tempsdi.SdataID));
                sdtablewidget->setItem(i,1,new QTableWidgetItem(tempsdi.SDSize));
                sdtablewidget->setItem(i,2,new QTableWidgetItem(tempsdi.SDResolution));
                sdtablewidget->setItem(i,3,new QTableWidgetItem(tempsdi.SDState));
                sdtablewidget->setItem(i,4,new QTableWidgetItem(tempsdi.SDBit));
                sdtablewidget->setItem(i,5,new QTableWidgetItem(tempsdi.SDComments));
                sdtablewidget->setItem(i,6,new QTableWidgetItem(tempsdi.SDPriority));
                sdtablewidget->setItem(i,7,new QTableWidgetItem(tempsdi.SDLabeledSomaNumber));
            }
        }
        sdtablewidget->setEditTriggers(QAbstractItemView::DoubleClicked);
        sdtablewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        sdtablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
        sdtablewidget->resizeColumnsToContents();
        sdtablewidget->resizeRowsToContents();
        connect(sdtablewidget,SIGNAL(cellEntered(int,int)),this,SLOT(createSomaTab_slot(int,int)));
        connect(sdtablewidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(reviseSDTab_slot(int,int)));
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
    int  row=0;
    if(title.size()>0)
    {
        QList<SomaConfInfo> tempsdlist=getSomaConf(somaconfpath);
        row=tempsdlist.size();
        sdtablewidget= new QTableWidget(row,col, this);
        sdtablewidget->setHorizontalHeaderLabels(title);
        if(tempsdlist.size())
        {
            for(int i=0;i<tempsdlist.size();i++)
            {
                SomaConfInfo tempsdi=tempsdlist.at(i);
//                //foreach item
                sdtablewidget->setItem(i,0,new QTableWidgetItem(tempsdi.SomaID));
                sdtablewidget->setItem(i,1,new QTableWidgetItem(tempsdi.SdataID));
                sdtablewidget->setItem(i,2,new QTableWidgetItem(tempsdi.CellName));
                sdtablewidget->setItem(i,3,new QTableWidgetItem(tempsdi.Zprojection));
                sdtablewidget->setItem(i,4,new QTableWidgetItem(tempsdi.Xcoordinate));
                sdtablewidget->setItem(i,5,new QTableWidgetItem(tempsdi.Ycoordinate));
                sdtablewidget->setItem(i,6,new QTableWidgetItem(tempsdi.SomaComments));
                sdtablewidget->setItem(i,7,new QTableWidgetItem(tempsdi.SomaState));
                sdtablewidget->setItem(i,8,new QTableWidgetItem(tempsdi.SomaLocation));
                sdtablewidget->setItem(i,9,new QTableWidgetItem(tempsdi.CellType));
            }
        }
        sdtablewidget->setEditTriggers(QAbstractItemView::DoubleClicked);
        sdtablewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        sdtablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
        sdtablewidget->resizeColumnsToContents();
        sdtablewidget->resizeRowsToContents();
    }
    return sdtablewidget;
}

void SourceDataManagement::SDMInit()
{
    logTextEdit=new QTextEdit(this);
    logTextEdit->setText("Welcome to Source data management");
    sourcedataconfTitle <<"SdataID"
                        <<"Size"
                        <<"Resolution"
                        <<"State"
                        <<"Bit"
                        <<"Comments"
                        <<"Priority"
                        <<"LabeledSomaNumber";

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
            toLog("Make a new source data configuration file at Configuration/BrainConf/sourceData.conf");
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
            SourceDataInfo tempsdi;
            tempsdi.SdataID=newSdataID;
            int currow=sourcedataTab->rowCount();
            sourcedataTab->insertRow(currow);
            //init
            sourcedataTab->setItem(currow,0,new QTableWidgetItem(tempsdi.SdataID));
            sourcedataTab->setItem(currow,1,new QTableWidgetItem(tempsdi.SDSize));
            sourcedataTab->setItem(currow,2,new QTableWidgetItem(tempsdi.SDResolution));
            sourcedataTab->setItem(currow,3,new QTableWidgetItem(tempsdi.SDState));
            sourcedataTab->setItem(currow,4,new QTableWidgetItem(tempsdi.SDBit));
            sourcedataTab->setItem(currow,5,new QTableWidgetItem(tempsdi.SDComments));
            sourcedataTab->setItem(currow,6,new QTableWidgetItem(tempsdi.SDPriority));
            sourcedataTab->setItem(currow,7,new QTableWidgetItem(tempsdi.SDLabeledSomaNumber));
            saveButton->setEnabled(true);
        }
    }
}
void SourceDataManagement::saveButton_slot()
{
    //save changes from source data tab to conf file.
    //1.get all the information at tab widget
        //foreach item at sourcedataTab
        //get row_num and column_num
        //store at a SourceDataInfo qlist
    int sdtrownum=sourcedataTab->rowCount();
    QList<SourceDataInfo> taballitems;
    for(int i=0;i<sdtrownum;i++)
    {
        SourceDataInfo tempsdi;
        tempsdi.SdataID=sourcedataTab->item(i,0)->text();
        tempsdi.SDSize=sourcedataTab->item(i,1)->text();
        tempsdi.SDResolution=sourcedataTab->item(i,2)->text();
        tempsdi.SDState=sourcedataTab->item(i,3)->text();
        tempsdi.SDBit=sourcedataTab->item(i,4)->text();
        tempsdi.SDComments=sourcedataTab->item(i,5)->text();
        tempsdi.SDPriority=sourcedataTab->item(i,6)->text();
        tempsdi.SDLabeledSomaNumber=sourcedataTab->item(i,7)->text();
        taballitems.append(tempsdi);
    }
    //2.write to conf. need a function.
    bool saveflag=WriteSourceDataToFile(this->sourcedataconfpath,taballitems);
    if(saveflag)
    {
        saveButton->setEnabled(false);
        toLog(QString("Saved all the changes to %1").arg(this->sourcedataconfpath));
    }
    else
        toLog(QString("Save Failed to %1").arg(this->sourcedataconfpath));
}

void SourceDataManagement::deleteSDButton_slot()
{
    int rowIndex=sourcedataTab->currentRow();
    int prerowcount=sourcedataTab->rowCount();
    if(rowIndex!=-1)
    {
        toLog(QString("Deleted %1 info at configuration.").arg(sourcedataTab->item(rowIndex,0)->text()));
        sourcedataTab->removeRow(rowIndex);
        saveButton->setEnabled(true);
        if(prerowcount==1)
            deleteSDButton->setEnabled(false);
    }
}

void SourceDataManagement::loadSomalist2Conf_slot()
{
    //get current SdataID
    int curtabindex=tabwidget->currentIndex();
    QString curtabTitle=tabwidget->tabText(curtabindex);

    QString somaconfpath=this->dbpath+"/Configuration/BrainConf/"+curtabTitle+"_soma.conf";
    QString title="choose soma list file (apo): ";
    QString somaapofilepath=QFileDialog::getOpenFileName(this, title,
                                                    "",
                                                    QObject::tr("soma marker file (*.apo *.APO)"
                                                                ));
    QFile somaapofile(somaapofilepath);
    if(somaapofile.exists())
    {
        QList <CellAPO> cellapolist=readAPO_file(somaapofilepath);
        //from cellapo to struct somaconfinfo
        QList<SomaConfInfo> inputsdlist;
        inputsdlist.clear();
        for(int i=0;i<cellapolist.size();i++)
        {
            CellAPO tmpCellApo=cellapolist.at(i);
            SomaConfInfo tempsdi;
            tempsdi.SomaID=tmpCellApo.name;
            tempsdi.SdataID=curtabTitle;
            tempsdi.Zprojection=QString::number(tmpCellApo.z);
            tempsdi.Xcoordinate=QString::number(tmpCellApo.x);
            tempsdi.Ycoordinate=QString::number(tmpCellApo.y);
            tempsdi.CellName=QString::number((int)tmpCellApo.z)+"-X"+QString::number((int)tmpCellApo.x)+"-Y"+QString::number((int)tmpCellApo.y);
            tempsdi.SomaComments=tmpCellApo.comment;
            tempsdi.SomaState="";
            tempsdi.SomaLocation="";
            tempsdi.CellType="";
            inputsdlist.append(tempsdi);
        }
        //store at conf
        bool isok=WriteSomalistConfToFile(somaconfpath,inputsdlist);
        if(isok)
            toLog(QObject::tr("Stored at File path: %1 \n").arg(somaconfpath));
        //display

    }
}


void SourceDataManagement::createSomaTab_slot(int row, int column)
{
    if(column==0)
    {
        QString tabtile=sourcedataTab->item(row,column)->text();
        QString somaconfpath=this->dbpath+"/Configuration/BrainConf/"+tabtile+"_soma.conf";
        QFile somaconffile(somaconfpath);
        if(!somaconffile.exists())
        {
            int reply;
            reply=QMessageBox::warning(this,"File Not Found",QObject::tr("Can't find configuration file of this data!\n")+
                                 QObject::tr("Do you want to make a new soma configration file for Data %1?\n").arg(tabtile),
                                 QMessageBox::Ok,QMessageBox::Cancel);
            if(reply==QMessageBox::Cancel)
                return;
            else
            {
                toLog(QObject::tr("Make a new soma configration file for Data %1 \n").arg(tabtile));
                QList<SomaConfInfo> inputsdlist;
                inputsdlist.clear();
                bool isok=WriteSomalistConfToFile(somaconfpath,inputsdlist);
                if(isok)
                    toLog(QObject::tr("File path: %1 \n").arg(somaconfpath));
            }
        }
        if(somaconffile.exists())
        {
            QTableWidget* somaconfTab=createTableSomaConf(somaconfpath);
            if(somaconfTab)
            {
                tabwidget->addTab(somaconfTab,tabtile);
                tabwidget->setCurrentIndex(tabwidget->currentIndex()+1);
                loadSomalist2Conf->setEnabled(true);
            }
        }
    }
}
void SourceDataManagement::reviseSDTab_slot(int row, int column)
{
    if(column>0)
    {
        saveButton->setEnabled(true);
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
    this->sourcedataconfpath=this->dbpath+"/Configuration/BrainConf/sourceData.conf";
}
void SourceDataManagement::toLog(const QString &logtext)
{
    QString getlogtext=logTextEdit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logTextEdit->setText(showText);
    logTextEdit->moveCursor(QTextCursor::End);
}
SourceDataManagement::SourceDataManagement()
{
}
SourceDataManagement::~SourceDataManagement()
{

}
