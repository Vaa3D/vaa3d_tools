#include "annotatormanagement.h"

AnnotatorManagement::AnnotatorManagement(const QString &path, QWidget *parent)
    :QDialog(parent)
{
    setupDBpath(path);
    this->setWindowTitle("MorphoHub-User-Management");
    UserManagementInit();
    createTabView();
    createActions();
    mainLayout=new QGridLayout;
    mainLayout->addWidget(tabwidget,0,0,8,6);
    mainLayout->addWidget(functionGroupBox,0,6,4,2);
    mainLayout->addWidget(logTextEdit,4,6,4,2);
    setLayout(mainLayout);
}

void AnnotatorManagement::UserManagementInit()
{
    logTextEdit=new QTextEdit(this);
    logTextEdit->setText("Welcome to User management");
    userconfTitle <<"UserID"<<"Name"<<"workingplace"<<"priority";
    prioritylist<<"visitor"<<"Annotator"<<"Juniorchecker"<<"SeniorChecker"<<"Administrater";
//    switch(tempsdi.priority)
//    {
//    case APvisitor:thisappriority="visitor";
//        break;
//    case APannotate:thisappriority="Annotator";
//        break;
//    case APjuniorcheck:thisappriority="Juniorchecker";
//        break;
//    case APseniorcheck:thisappriority="SeniorChecker";
//        break;
//    case APAdministrater:thisappriority="Administrater";
//        break;
//    }
    QFile conffile(this->userconfpath);
    if(!conffile.exists())
    {
        if(conffile.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QString data=userconfTitle[0];
            for(int i=1;i<userconfTitle.size();i++)
            {
                data=data+","+userconfTitle[i];
            }
            conffile.write(data.toAscii());
            conffile.close();
            toLog("Make a new user configuration file at Configuration/User.conf");
        }
    }
}

void AnnotatorManagement::createTabView()
{
    tabwidget=new QTabWidget();
    tabwidget->setTabsClosable(false);
    userConfTab=createTableUserData();
    if(userConfTab)
        tabwidget->addTab(userConfTab,tr("User Infomation"));
}

void AnnotatorManagement::createActions()
{
    functionGroupBox=new QGroupBox(tr("Operation Buttons"));
    functionLayout=new QVBoxLayout;
    saveButton=new QPushButton(tr("Save"));
    saveButton->setEnabled(true);
    connect(saveButton,SIGNAL(clicked()),this,SLOT(saveButton_slot()));
    newButton=new QPushButton(tr("New User"));
    connect(newButton,SIGNAL(clicked()),this,SLOT(newButton_slot()));
    deleteButton=new QPushButton(tr("Remove User"));
    connect(deleteButton,SIGNAL(clicked()),this,SLOT(deleteButton_slot()));

    functionLayout->addWidget(saveButton);
    functionLayout->addWidget(newButton);
    functionLayout->addWidget(deleteButton);
    functionGroupBox->setLayout(functionLayout);
}

QTableWidget* AnnotatorManagement::createTableUserData()
{
    //scan sourcedata.conf
    //get every cloum
    QStringList title=this->userconfTitle;
    QTableWidget* usertablewidget;
    int col = title.size();
    int  row=0;
    if(title.size()>0)
    {
        QList<Annotator> templist=getAnnotatorlist(this->userconfpath);
        row=templist.size();
        usertablewidget= new QTableWidget(row,col, this);
        usertablewidget->setHorizontalHeaderLabels(title);
        if(templist.size())
        {
            for(int i=0;i<templist.size();i++)
            {
                Annotator tempsdi=templist.at(i);
//                //foreach item
                usertablewidget->setItem(i,0,new QTableWidgetItem(tempsdi.UserID));
                usertablewidget->setItem(i,1,new QTableWidgetItem(tempsdi.Name));
                usertablewidget->setItem(i,2,new QTableWidgetItem(tempsdi.workingplace));
                QComboBox *priorityCombobox=new QComboBox();
                priorityCombobox->addItems(prioritylist);
                priorityCombobox->setCurrentIndex((int)tempsdi.priority);
                usertablewidget->setCellWidget(i,3,priorityCombobox);
            }
        }
        usertablewidget->setEditTriggers(QAbstractItemView::DoubleClicked);
        usertablewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        usertablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
        usertablewidget->resizeColumnsToContents();
        usertablewidget->resizeRowsToContents();
        connect(usertablewidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(reviseTab_slot(int,int)));
    }
    return usertablewidget;
}

void AnnotatorManagement::setupDBpath(const QString &path)
{
    if(path.isEmpty())
    {
        QMessageBox::warning(this,tr("Path Error"),tr("please setup dbpath"));
        return;
    }
    this->dbpath=path;
    this->userconfpath=this->dbpath+"/Configuration/User.conf";
}
void AnnotatorManagement::saveButton_slot()
{
    //save changes from user data tab to conf file.
    //1.get all the information at tab widget
        //foreach item at sourcedataTab
        //get row_num and column_num
        //store at a SourceDataInfo qlist
    int sdtrownum=userConfTab->rowCount();
    QList<Annotator> taballitems;
    for(int i=0;i<sdtrownum;i++)
    {
        Annotator tempsdi;
        tempsdi.UserID=userConfTab->item(i,0)->text();
        tempsdi.Name=userConfTab->item(i,1)->text();
        tempsdi.workingplace=userConfTab->item(i,2)->text();
        QWidget *tmpwidget=userConfTab->cellWidget(i,3);
        QComboBox *tmpcombobox=(QComboBox*)tmpwidget;
        tempsdi.priority=(AuthorPriority)tmpcombobox->currentIndex();
        taballitems.append(tempsdi);
    }
    //2.write to conf. need a function.
    bool saveflag=WriteAnnotatorConfToFile(this->userconfpath,taballitems);
    if(saveflag)
    {
        saveButton->setEnabled(true);
        toLog(QString("Saved all the changes to %1").arg(this->userconfpath));
    }
    else
        toLog(QString("Save Failed to %1").arg(this->userconfpath));
}

void AnnotatorManagement::newButton_slot()
{
    int tabindex=tabwidget->currentIndex();
    if(tabindex==0)
    {
        bool ok;
        QString newID=QInputDialog::getText(this,tr("New User"),tr("please input a new User ID :"),QLineEdit::Normal,"XXX",&ok);
        if(ok)
        {
            Annotator tempsdi;
            tempsdi.UserID=newID;
            int currow=userConfTab->rowCount();
            userConfTab->insertRow(currow);
            //init
            userConfTab->setItem(currow,0,new QTableWidgetItem(tempsdi.UserID));
            userConfTab->setItem(currow,1,new QTableWidgetItem(tempsdi.Name));
            userConfTab->setItem(currow,2,new QTableWidgetItem(tempsdi.workingplace));
            QComboBox *priorityCombobox=new QComboBox();
            priorityCombobox->addItems(prioritylist);
            priorityCombobox->setCurrentIndex(0);
            userConfTab->setCellWidget(currow,3,priorityCombobox);
            saveButton->setEnabled(true);
        }
    }
}

void AnnotatorManagement::deleteButton_slot()
{
    int rowIndex=userConfTab->currentRow();
    int prerowcount=userConfTab->rowCount();
    if(rowIndex!=-1)
    {
        toLog(QString("Deleted %1 info at configuration.").arg(userConfTab->item(rowIndex,0)->text()));
        userConfTab->removeRow(rowIndex);
        saveButton->setEnabled(true);
        if(prerowcount==1)
            deleteButton->setEnabled(false);
    }
}

void AnnotatorManagement::reviseTab_slot(int r, int c)
{
    if(c>0)
    {
        saveButton->setEnabled(true);
    }
}

void AnnotatorManagement::toLog(const QString &logtext)
{
    QString getlogtext=logTextEdit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logTextEdit->setText(showText);
    logTextEdit->moveCursor(QTextCursor::End);
}
AnnotatorManagement::AnnotatorManagement()
{

}
AnnotatorManagement::~AnnotatorManagement()
{

}
