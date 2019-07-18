#include "dbmsdialog.h"
#include "dbms_basic.h"

DBMSDialog::DBMSDialog(QWidget *parent) :
    QDialog(parent)
{
    Initnumber();
    /*********************main Item********************************/
    createMainMenu();
    createSubMainMenu();
    /*********************Tab view 1********************************/

    createTabView();
    firstViewGroupBox =new QGroupBox("Information View");
    firstViewLayout=new QHBoxLayout;

    firstViewLayout->addWidget(tabwidget);
    firstViewGroupBox->setLayout(firstViewLayout);    

    createTableFunction();
    createLoginMenu();
    createBasicView();

    mainLayout=new QGridLayout;
    mainLayout->addWidget(mainItemGroupBox,0,0,2,1);
    mainLayout->addWidget(submainItemGroupBox,2,0,2,1);
    mainLayout->addWidget(firstViewGroupBox,0,1,8,6);
    mainLayout->addWidget(manipulateGroupBox,8,1,1,6);
    mainLayout->addWidget(loginGroupBox,0,7,3,2);
    mainLayout->addWidget(basicViewGroupBox,3,7,6,2);
    setLayout(mainLayout);
    setWindowTitle(tr("DB Management System"));
}
void DBMSDialog::Initnumber()
{
//    submainItemLableName=tr("sub main menu");
}
void DBMSDialog::setbasicViewText(const QString &showText)
{
//    QString showText="This is cool and Hello world.\n";

    basicViewTextEdit->setText(showText);
}

void DBMSDialog::createMainMenu()
{
    mainItemGroupBox=new QGroupBox(tr("Menu"));
    mainItemLable=new QLabel(tr("Main item"));
    mainItemComboBox=new QComboBox;
    //convert every item to configuration file
    //mainitem.conf
    mainItemComboBox->addItem(tr("Brain"));
    mainItemComboBox->addItem(tr("Annotator"));
    connect(mainItemComboBox,SIGNAL(activated(int)),this,SLOT(mainItemComboBox_slot()));

    mainItemLayout=new QVBoxLayout;
    mainItemLayout->addWidget(mainItemLable,0,0);
    mainItemLayout->addWidget(mainItemComboBox,1,0);
    mainItemGroupBox->setLayout(mainItemLayout);

}
void DBMSDialog::createSubMainMenu()
{
    submainItemGroupBox=new QGroupBox(tr("Sub Main"));
    submainItemLable=new QLabel(tr("sub main Item"));
    //submainItemLable->text()=submainItemLableName;
    submainItemComboBox=new QComboBox;
    submainItemComboBox->addItem(tr("ALL"));
    submainItemComboBox->addItem(tr("17300"));
    connect(submainItemComboBox,SIGNAL(activated(int)),this,SLOT(submainItemComboBox_slot()));

    submainItemLayout=new QVBoxLayout;
    submainItemLayout->addWidget(submainItemLable,0,0);
    submainItemLayout->addWidget(submainItemComboBox,1,0);

    submainItemGroupBox->setLayout(submainItemLayout);
}
void DBMSDialog::createTableFunction()
{
    /*********************Manipulate Item********************************/
    manipulateGroupBox=new QGroupBox(tr("Functions"));
    NewPushButton=new QPushButton(tr("Add"));
    DeletePushButton=new QPushButton(tr("Delete"));
    SearchPushButton=new QPushButton(tr("Search"));
    SelectPushButton=new QPushButton(tr("Select All"));
    //button function
    connect(NewPushButton,SIGNAL(clicked()),this,SLOT(newButton_slot()));
    connect(DeletePushButton,SIGNAL(clicked()),this,SLOT(deleteButton_slot()));

    manipulateLayout=new QHBoxLayout;
    manipulateLayout->addWidget(SelectPushButton);
    manipulateLayout->addWidget(SearchPushButton);
    manipulateLayout->addWidget(DeletePushButton);
    manipulateLayout->addWidget(NewPushButton);
    manipulateGroupBox->setLayout(manipulateLayout);
}
void DBMSDialog::createLoginMenu()
{
    loginLayout=new QGridLayout;
    loginGroupBox=new QGroupBox(tr("Login"),this);
    userLabel=new QLabel(tr("User :"),this);
    userNameLabel=new QLabel(tr("Visitor"),this);
    priorityLabel=new QLabel(tr("Priority :"),this);
    priorityNameLabel=new QLabel(tr("Low"),this);
    loginOroutButton=new QPushButton(tr("Login"));
    //connect this button to login dialog
    connect(loginOroutButton,SIGNAL(clicked()),this,SLOT(loginButton_slot()));

    loginLayout->addWidget(userLabel,0,0,1,1);
    loginLayout->addWidget(userNameLabel,0,1,1,2);
    loginLayout->addWidget(priorityLabel,1,0,1,1);
    loginLayout->addWidget(priorityNameLabel,1,1,1,2);
    loginLayout->addWidget(loginOroutButton,2,1,1,1);
    loginGroupBox->setLayout(loginLayout);
}
void DBMSDialog::createBasicView()
{
    basicViewTextEdit=new QTextEdit(this);
    QString showText="This is cool and Hello world.\n";
    basicViewTextEdit->setText(showText);
    basicViewLayout=new QVBoxLayout;
    basicViewGroupBox=new QGroupBox(tr("Basic View"));
    basicViewLayout->addWidget(basicViewTextEdit);
    basicViewGroupBox->setLayout(basicViewLayout);
}

void DBMSDialog::createTabView()
{
    tabwidget=new QTabWidget();
    brainBasictable= createTableBrainBasic();
    //brainAnnotaiontable= createTableBrainAnnotation();
    tabwidget->addTab(brainBasictable,tr("Brain Basic"));
    //tabwidget->addTab(brainAnnotaiontable,tr("Brain Annotation"));
}

QTableWidget* DBMSDialog::createTableBrainBasic()
{
    QStringList qsl;
    qsl << "ID"  << "state" << "priority" << "resolution" << "comments" << "size" << "bit";
    int row = 4;
    int col = qsl.size();

    QTableWidget* t = new QTableWidget(row,col, this);
    QTableWidgetItem *checkbox_test=new QTableWidgetItem();
    checkbox_test->setCheckState(Qt::Checked);
    t->setHorizontalHeaderLabels(qsl);
    t->setItem(0,0,checkbox_test);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    return t;
}

QTableWidget* DBMSDialog::createTableBrainAnnotation()
{
    QStringList qsl;
    qsl << "ID"  << "SomaNumber" << "UndoNeurons" << "UndoPriority" << "Annotation_L0" << "Annotation_L1" << "Annotation_L2";
    int row = 4;
    int col = qsl.size();

    QTableWidget* t = new QTableWidget(row,col, this);
    t->setHorizontalHeaderLabels(qsl);
    return t;
}

/********************************************SLOT functions********************************************************************/
void DBMSDialog::mainItemComboBox_slot()
{
    qDebug()<<"main item"<<endl;
}
void DBMSDialog::submainItemComboBox_slot()
{
    qDebug()<<"sub main item change"<<endl;
}
void DBMSDialog::loginButton_slot()
{
    qDebug()<<"loginto DBMS"<<endl;
}
void DBMSDialog::newButton_slot()
{
    //new a dialog for INPUT para
    adddialog=new QDialog(this);

    QLabel *brainNameLabel=new QLabel(tr("Name"));
    brainNameLineEdit=new QLineEdit(this);

    QLabel *stateLabel=new QLabel(tr("State"));
    stateCombox=new QComboBox;
    stateCombox->addItem(tr("Unconvert"));
    stateCombox->addItem(tr("Converting"));
    stateCombox->addItem(tr("Converted"));
    stateCombox->addItem(tr("Finished"));

    QLabel *priorityAddLabel=new QLabel(tr("Priority"));
    priorityAddCombox=new QComboBox;
    priorityAddCombox->addItem(tr("Low"));
    priorityAddCombox->addItem(tr("Normal"));
    priorityAddCombox->addItem(tr("High"));

    QLabel *commentsLabel=new QLabel(tr("Comments"));
    commentsLineEdit=new QLineEdit;

    QLabel *xCooridnateLabel=new QLabel(tr("X Resolution"));
    xCooridnateLineEdit=new QLineEdit;
    QLabel *yCooridnateLabel=new QLabel(tr("Y Resolution"));
    yCoordinateLineEdit=new QLineEdit;
    QLabel *zCoordinateLabel=new QLabel(tr("Z Resolution"));
    zCoordinateLineEdit=new QLineEdit;

    QLabel *sizeLabel=new QLabel(tr("Size"));
    sizeLineEdit=new QLineEdit;

    QLabel *bitLabel=new QLabel(tr("Bit"));
    bitLineEdit=new QLineEdit;

    QPushButton *okayButton=new QPushButton(tr("Oaky"));
    QPushButton *cancelButton=new QPushButton(tr("Cancel"));
    connect(okayButton,SIGNAL(clicked()),this,SLOT(addbrain_slot()));
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(cancelbrain_slot()));

    QGridLayout *addLayout=new QGridLayout;
    addLayout->addWidget(brainNameLabel,0,0,1,1);
    addLayout->addWidget(brainNameLineEdit,0,1,1,1);
    addLayout->addWidget(stateLabel,1,0,1,1);
    addLayout->addWidget(stateCombox,1,1,1,1);
    addLayout->addWidget(priorityAddLabel,2,0,1,1);
    addLayout->addWidget(priorityAddCombox,2,1,1,1);
    addLayout->addWidget(commentsLabel,3,0,1,1);
    addLayout->addWidget(commentsLineEdit,3,1,1,1);

    addLayout->addWidget(xCooridnateLabel,4,0,1,1);
    addLayout->addWidget(xCooridnateLineEdit,4,1,1,1);
    addLayout->addWidget(yCooridnateLabel,5,0,1,1);
    addLayout->addWidget(yCoordinateLineEdit,5,1,1,1);
    addLayout->addWidget(zCoordinateLabel,6,0,1,1);
    addLayout->addWidget(zCoordinateLineEdit,6,1,1,1);

    addLayout->addWidget(sizeLabel,7,0,1,1);
    addLayout->addWidget(sizeLineEdit,7,1,1,1);
    addLayout->addWidget(bitLabel,8,0,1,1);
    addLayout->addWidget(bitLineEdit,8,1,1,1);

    addLayout->addWidget(cancelButton,9,0,1,1);
    addLayout->addWidget(okayButton,9,1,1,1);

    adddialog->setLayout(addLayout);
    adddialog->setWindowTitle(tr("Add a new brain"));
    adddialog->setModal(true);
    adddialog->show();
    //update configuration file
    //update GUI
}
void DBMSDialog::addbrain_slot()
{
    //must have brain name
    QStringList qsl;
    if(brainNameLineEdit->text().isEmpty())
    {
//        v3d_msg("Please input the brain number");
        QString msg="Please input the brain number";
        QMessageBox::information(0, "Information", msg);
        return QDialog::reject();
    }
    qsl<<brainNameLineEdit->text();
    qsl<<stateCombox->currentText();
    qsl<<priorityAddCombox->currentText();
    if(!commentsLineEdit->text().isEmpty())
        qsl<<commentsLineEdit->text();
    else
        qsl<<" ";
    qsl<<xCooridnateLineEdit->text();
    qsl<<yCoordinateLineEdit->text()<<zCoordinateLineEdit->text()
         <<sizeLineEdit->text()<<bitLineEdit->text();


    for(int i=0;i<qsl.size();i++)
    {
        qDebug()<<qsl[i]<<",";
    }
    adddialog->close();
}
void DBMSDialog::cancelbrain_slot()
{
    adddialog->close();
}

void DBMSDialog::deleteButton_slot()
{

}
