#include "sourcedatamanagement.h"

SourceDataManagement::SourceDataManagement(const QString &path, QWidget *parent)
 :QDialog(parent)
{
    setupDBpath(path);
    setWindowTitle(tr("MorphoHub-SourceData-Management"));
    SDMInit();
    createTabView();
    mainLayout=new QGridLayout;
    mainLayout->addWidget(tabwidget,0,0,7,6);
    setLayout(mainLayout);
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
    int col = title.size();
    int  row=5;
    QTableWidget* t;
    if(title.size()>0)
    {
        t= new QTableWidget(row,col, this);
        t->setHorizontalHeaderLabels(title);
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        t->setSelectionMode(QAbstractItemView::SingleSelection);
        t->resizeColumnsToContents();
        t->resizeRowsToContents();
    }
    return t;
}
void SourceDataManagement::SDMInit()
{
    sourcedataconfTitle <<"SDID"
                       <<"Size"
                      <<"Resolution"
                     <<"State"
                    <<"Bit"
                   <<"Comments"
                  <<"Priority"
                 <<"LabeledSomaNumber";
    somaconfTitle<<"SomaID"
                <<"SDID"
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
                data=","+sourcedataconfTitle[i];

            }
            sourcedataconffile.write(data.toAscii());
            sourcedataconffile.close();
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
SourceDataManagement::SourceDataManagement()
{
}
SourceDataManagement::~SourceDataManagement()
{

}
