#include "dbmsdialog.h"

DBMSDialog::DBMSDialog(QWidget *parent) :
    QDialog(parent)
{


    /*********************main Item********************************/
    mainItemGroupBox=new QGroupBox(tr("Menu"));
    mainItemLable=new QLabel(tr("Main item"));
    mainItemComboBox=new QComboBox;
    mainItemComboBox->addItem(tr("Brain"));
    mainItemComboBox->addItem(tr("Neuron"));
    mainItemComboBox->addItem(tr("Annotator"));
    connect(mainItemComboBox,SIGNAL(activated(int)),this,SLOT(mainItemComboBox_slot()));

    mainItemLayout=new QVBoxLayout;
    mainItemLayout->addWidget(mainItemLable,0,0);
    mainItemLayout->addWidget(mainItemComboBox,1,0);
    mainItemGroupBox->setLayout(mainItemLayout);

    /*********************Tab view 1********************************/

    createTabView();
    firstViewGroupBox =new QGroupBox("Information View");
    firstViewLayout=new QHBoxLayout;

    firstViewLayout->addWidget(tabwidget);
    firstViewGroupBox->setLayout(firstViewLayout);

    /*********************Manipulate Item********************************/
    manipulateGroupBox=new QGroupBox(tr("Functions"));
    NewPushButton=new QPushButton(tr("Add"));
    DetelePushButton=new QPushButton(tr("Delete"));
    manipulateLayout=new QVBoxLayout;
    manipulateLayout->addWidget(NewPushButton,0,0);
    manipulateLayout->addWidget(DetelePushButton,1,0);
    manipulateGroupBox->setLayout(manipulateLayout);

    mainLayout=new QGridLayout;
    mainLayout->addWidget(mainItemGroupBox,0,0,2,1);
    mainLayout->addWidget(firstViewGroupBox,0,1,8,4);
    mainLayout->addWidget(manipulateGroupBox,0,5,2,1);
    setLayout(mainLayout);
    setWindowTitle(tr("DB Management System"));
}
void DBMSDialog::createTabView()
{
    tabwidget=new QTabWidget();
    brainBasictable= createTableBrainBasic();
    brainAnnotaiontable= createTableBrainAnnotation();
    tabwidget->addTab(brainBasictable,tr("Brain Basic"));
    tabwidget->addTab(brainAnnotaiontable,tr("Brain Annotation"));
}

QTableWidget* DBMSDialog::createTableBrainBasic()
{
    QStringList qsl;
    qsl << "ID"  << "state" << "priority" << "resolution" << "comments" << "size" << "bit";
    int row = 4;
    int col = qsl.size();

    QTableWidget* t = new QTableWidget(row,col, this);
    t->setHorizontalHeaderLabels(qsl);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
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

void DBMSDialog::mainItemComboBox_slot()
{
    qDebug()<<"main item"<<endl;
}
