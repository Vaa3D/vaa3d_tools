#include "neuronquerymainwindow.h"
#include "ui_neuronquerymainwindow.h"
#include "NeuronQuerySystem_plugin.h"

#include <QDialog>
#include <QMessageBox>
#include <QProgressDialog>

enum { absoluteFileNameRole = Qt::UserRole + 1 };
static inline QString fileNameOfItem(const QTableWidgetItem *item)
{
    return item->data(absoluteFileNameRole).toString();
}

static inline void openFile(const QString &fileName)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

NeuronQueryMainWindow::NeuronQueryMainWindow(V3DPluginCallback2 &callback,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NeuronQueryMainWindow)
{
    ui->setupUi(this);
    QueryNeuroncallback=&callback;
    this->setCentralWidget(ui->dataTabWidget);

    this->createMenuBar();
    this->init();

//    connect(ui->dataTabWidget,SIGNAL(currentChanged(int)),this,SLOT(dataTabChange(int)));

    connect(ui->browse_button, &QAbstractButton::clicked, this, &NeuronQueryMainWindow::browse);
    connect(ui->querySwcButton,&QAbstractButton::clicked,this,&NeuronQueryMainWindow::on_querySwcButton_clicked);


}

NeuronQueryMainWindow::~NeuronQueryMainWindow()
{
    delete ui;
}

void NeuronQueryMainWindow::createMenuBar(){

    quitAction = new QAction(tr("&Quit"), this);
    aboutAction = new QAction(tr("&About"), this);
    aboutQtAction = new QAction(tr("&About Qt"), this);
    loginAction = new QAction(tr("&Login"),this);
    logoutAction=new QAction(tr("&Logout"),this);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);

    QMenu *LoginMenu = menuBar()->addMenu(tr("&Login"));
    LoginMenu->addAction(loginAction);
    LoginMenu->addAction(logoutAction);
    loginAction->setToolTip(tr("You have to sign in to get the advanced functions."));
    loginAction->setEnabled(true);
    connect(loginAction,SIGNAL(triggered()),this,SLOT(loginAction_slot()));

    logoutAction->setEnabled(false);
    connect(logoutAction,SIGNAL(triggered()),this,SLOT(logoutAction_slot()));
    connect(quitAction, &QAction::triggered, this, &NeuronQueryMainWindow::close);
    connect(aboutAction, &QAction::triggered, this, &NeuronQueryMainWindow::about);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}


void NeuronQueryMainWindow::init()

{
    ui->directoryComboBox->setEditable(true);
    ui->directoryComboBox->addItem(QDir::toNativeSeparators(QDir::currentPath()));
    ui->directoryComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    ui->BrainID_Swc_ComboBox->setEditable(true);
    ui->BrainID_Swc_ComboBox->addItem(QDir::toNativeSeparators(QDir::currentPath()));
    ui->BrainID_Swc_ComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    connect(ui->BrainID_Swc_ComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &NeuronQueryMainWindow::animateFindClick);

    connect(ui->directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &NeuronQueryMainWindow::animateFindClick);

    ui->querySwcTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    QStringList labels;
    labels << tr("Filename") << tr("Size");
    ui->querySwcTable->setHorizontalHeaderLabels(labels);
//    ui->querySwcTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->querySwcTable->verticalHeader()->hide();
    ui->querySwcTable->setShowGrid(false);
    ui->querySwcTable->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->querySwcTable, &QTableWidget::customContextMenuRequested,
            this, &NeuronQueryMainWindow::contextMenu);
    connect(ui->querySwcTable, &QTableWidget::cellActivated,
            this, &NeuronQueryMainWindow::openFileOfItem);
}


void NeuronQueryMainWindow::openFileOfItem(int row, int /* column */)
{
    const QTableWidgetItem *item = ui->querySwcTable->item(row, 0);
    openFile(fileNameOfItem(item));
}

void NeuronQueryMainWindow::contextMenu(const QPoint &pos)
{
    const QTableWidgetItem *item = ui->querySwcTable->itemAt(pos);
    if (!item)
        return;
    QMenu menu;
#ifndef QT_NO_CLIPBOARD
    QAction *copyAction = menu.addAction("Copy Name");
#endif
    QAction *openAction = menu.addAction("Open");
    QAction *action = menu.exec(ui->querySwcTable->mapToGlobal(pos));
    if (!action)
        return;
    const QString fileName = fileNameOfItem(item);
    if (action == openAction)
        openFile(fileName);
#ifndef QT_NO_CLIPBOARD
    else if (action == copyAction)
        QGuiApplication::clipboard()->setText(QDir::toNativeSeparators(fileName));
#endif
}


void NeuronQueryMainWindow::animateFindClick()
{
    ui->querySwcButton->animateClick();
}




//void NeuronQueryMainWindow::on_loadApoButton_customContextMenuRequested(const QPoint &pos)
//{

//}


void NeuronQueryMainWindow::on_querySwcButton_clicked()
{
    qDebug()<<"begin to click query swc";
    ui->querySwcTable->setRowCount(0);

//    QString BrainArea = ui->BrainArea_Swc_ComboBox->currentText();
//    QString MorphoID = ui->MorphoID_Swc_ComboBox->currentText();
//    int Batch = ui->Batch_Swc_SpinBox->value();
//    QString NeuronID = ui->NeuronID_Swc_lineEdit->displayText();
    QString BrainID =ui->BrainID_Swc_ComboBox->currentText();
    qDebug()<<"brain id: "<<BrainID;

    QString path = QDir::cleanPath(ui->directoryComboBox->currentText());
    qDebug()<<path;
    currentDir = QDir(path);

    if (ui->BrainID_Swc_ComboBox->findText(ui->BrainID_Swc_ComboBox->currentText()) == -1)
        ui->BrainID_Swc_ComboBox->addItem(ui->BrainID_Swc_ComboBox->currentText());
    if (ui->directoryComboBox->findText(ui->directoryComboBox->currentText()) == -1)
        ui->directoryComboBox->addItem(ui->directoryComboBox->currentText());


    QStringList filter;
    if(!BrainID.isEmpty()){
        filter <<BrainID;
        QDirIterator it(path, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        QStringList files;
        while (it.hasNext()) {
            files<<it.next();
            qDebug()<<files;
        }

        if(!!BrainID.isEmpty())
            files = querySwcFiles(files,BrainID);
            qDebug()<<"query swc files"<<files;
        files.sort();
        showSwcTables(files);
    }
}
QStringList NeuronQueryMainWindow::querySwcFiles(const QStringList &files, const QString &text)
{
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size());
    progressDialog.setWindowTitle(tr("Find Files"));

//! [5] //! [6]
    QMimeDatabase mimeDatabase;
    QStringList foundFiles;

    for (int i = 0; i < files.size(); ++i) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Searching file number %1 of %n...", nullptr, files.size()).arg(i));
        QCoreApplication::processEvents();
//! [6]

        if (progressDialog.wasCanceled())
            break;

//! [7]
        const QString fileName = files.at(i);
        const QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileName);
        if (mimeType.isValid() && !mimeType.inherits(QStringLiteral("text/plain"))) {
            qWarning() << "Not searching binary file " << QDir::toNativeSeparators(fileName);
            continue;
        }
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);
            while (!in.atEnd()) {
                if (progressDialog.wasCanceled())
                    break;
                line = in.readLine();
                if (line.contains(text, Qt::CaseInsensitive)) {
                    foundFiles << files[i];
                    break;
                }
            }
        }
    }
    return foundFiles;

}


void NeuronQueryMainWindow::browse()
{
    QString directory =
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath()));

    if (!directory.isEmpty()) {
        if (ui->directoryComboBox->findText(directory) == -1)
            ui->directoryComboBox->addItem(directory);
        ui->directoryComboBox->setCurrentIndex(ui->directoryComboBox->findText(directory));
    }
}

void NeuronQueryMainWindow::on_queryAnoButton_clicked()
{

}


void NeuronQueryMainWindow::on_queryApoButton_clicked()
{

}


void NeuronQueryMainWindow::on_LoadSwcButton_clicked()
{

}

void NeuronQueryMainWindow::showSwcTables(const QStringList &paths)
{
    qDebug()<<"begin to showSwcTables";
    for (const QString &filePath : paths) {
        const QString toolTip = QDir::toNativeSeparators(filePath);
        const QString relativePath = QDir::toNativeSeparators(currentDir.relativeFilePath(filePath));
        const qint64 size = QFileInfo(filePath).size();
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(relativePath);
        fileNameItem->setData(absoluteFileNameRole, QVariant(filePath));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        qDebug()<<"fileNameItem:"<<fileNameItem;
        QTableWidgetItem *sizeItem = new QTableWidgetItem(QLocale().formattedDataSize(size));
        sizeItem->setData(absoluteFileNameRole, QVariant(filePath));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);
        qDebug()<<"sizeItem:"<<sizeItem;

        int row = ui->querySwcTable->rowCount();
        ui->querySwcTable->insertRow(row);
        ui->querySwcTable->setItem(row, 0, fileNameItem);
        ui->querySwcTable->setItem(row, 1, sizeItem);
    }
    ui->swcFileFoundLable->setText(tr("%n file(s) found (Double click on a file to open it)", nullptr, paths.size()));
    ui->swcFileFoundLable->setWordWrap(true);
}

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

void NeuronQueryMainWindow::toLogWindow(const QString &logtext)
{
    QString getlogtext=logtextedit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logtextedit->setText(showText);
    logtextedit->moveCursor(QTextCursor::End);
}

void NeuronQueryMainWindow::loginAction_slot()
{
    loginDialog=new QDialog(this);
    loginDialog->setWindowTitle("Neuron-Query-System");

    QLabel *userIDQLabel=new QLabel("UserID:");
    loginUserIDQLineEdit=new QLineEdit();
    QLabel *passwordQLabel=new QLabel("Password:");
    loginPasswordQlineedit=new QLineEdit();

    loginCancelButton =new QPushButton("Cancel");
    connect(loginCancelButton,SIGNAL(clicked()),this,SLOT(loginCancelButton_slot()));
    loginOkayButton=new QPushButton("Okay");
    connect(loginOkayButton,SIGNAL(clicked()),this,SLOT(loginOkayButton_slot()));

    loginMainlayout=new QGridLayout();
    loginMainlayout->addWidget(userIDQLabel,1,0,1,2);
    loginMainlayout->addWidget(loginUserIDQLineEdit,1,2,1,2);
    loginMainlayout->addWidget(passwordQLabel,2,0,1,1);
    loginMainlayout->addWidget(loginPasswordQlineedit,2,2,1,2);

    loginMainlayout->addWidget(loginCancelButton,4,0,1,1);
    loginMainlayout->addWidget(loginOkayButton,4,3,1,1);

    loginDialog->setLayout(loginMainlayout);
    loginDialog->raise();
    loginDialog->setFixedSize(400,400);

    loginDialog->setGeometry(100,100,400,400);
    loginDialog->setModal(true);
    loginDialog->show();
}


void NeuronQueryMainWindow::loginOkayButton_slot()
{
    if(!loginUserIDQLineEdit->text().isEmpty())
    {
        //check the input at conf
        //if yes
        author.UserID=loginUserIDQLineEdit->text().toUpper();

        loginDialog->close();
        loginAction->setEnabled(false);
        logoutAction->setEnabled(true);
        userStatusLabel->setText(tr("UserID: %1").arg(author.UserID));
        toLogWindow(tr("Welcome %1 login.").arg(author.UserID));
        QSettings settings("MorphoHub","Vaa3d");
        settings.setValue("UserID",author.UserID);
    }
    else
    {
        QMessageBox::warning(this,"Input Errot","Please Input User ID!");
        return;
    }
}


void NeuronQueryMainWindow::loginCancelButton_slot()
{
    loginDialog->close();
}


void NeuronQueryMainWindow::logoutAction_slot()
{
    QString olduserID=author.UserID;
    author.UserID="";
    loginAction->setEnabled(true);
    logoutAction->setEnabled(false);
    toLogWindow(tr("%1 logout").arg(olduserID));
    userStatusLabel->setText(tr("UserID: Nobody"));
}


//void NeuronQueryMainWindow::dataTabChange(int tabIndex){
//    QTableWidget *dataTable = dataTableList.at(tabIndex);
//    QString itemtext = dataTabletilelist.at(tabIndex);
//    dataTable->clear();
//    updateDataTable()
//}



//void NeuronQueryMainWindow::on_querySwcButton_customContextMenuRequested(const QPoint &pos)
//{

//}

void NeuronQueryMainWindow::popAction_3Dview_slot()
{
//    qDebug()<<"call 3d view";
//    int curtabindex=ui->dataTabWidget->currentIndex();
//    QTableWidget *levelTable=datatablelist.at(curtabindex);
//    if(levelTable!=NULL)
//    {
//        QTreeWidgetItem* tempqtreeitem=contentTreewidget->currentItem();
//        QString parentSpaceName="WorkingSpace";
//        if(tempqtreeitem != NULL)
//        {
//            if(tempqtreeitem->parent()!= NULL)
//            {
//                parentSpaceName=tempqtreeitem->parent()->text(0);
//            }
//            else
//            {
//                parentSpaceName=contentTreewidget->currentItem()->text(contentTreewidget->currentColumn());
//            }
//        }
//    //    qDebug()<<"content "<<parentSpaceName;
//        QString curPathSWC = this->dbpath+"/"+parentSpaceName+"/"+curRecon.levelID+"/"+curRecon.fatherDirName+"/"+curRecon.fileName+".ano";
//        //QString curPathSWC = this->dbpath+"/"+this->contentTreewidget->currentItem()->text(this->contentTreewidget->currentColumn())+"/"+curRecon.levelID+"/"+curRecon.fatherDirName+"/"+curRecon.fileName+".ano";
//        //iscurReconExist();
//        //qDebug()<<"swc path:"<<curPathSWC;
//        QFileInfo curSWCBase(curPathSWC);
//        if(curSWCBase.exists())
//        {
//            //qDebug()<<"path "<<curPathSWC;
//            V3dR_MainWindow * surface_win = QueryNeuroncallback->open3DViewerForLinkerFile(curPathSWC);
////            QueryNeuroncallback->open3DViewerForLinkerFile(curPathSWC);
////                new3DWindow = MorphoHubcallback->open3DViewerForSingleSurfaceFile(curPathSWC);
//            //reset window title to basename instead of path name
//                QueryNeuroncallback->setWindowDataTitle(surface_win,curSWCBase.baseName());
//        }
//    }
}


//void NeuronQueryMainWindow::on_dataTabWidget_customContextMenuRequested(const QPoint &pos)
//{
//    int curtabindex = ui->dataTabWidget->currentIndex();
//    QTableWidget *dataTable = dataTableList.at(curtabindex);
//    QTableWidgetItem *item = dataTable->itemAt(pos);

//}

//void NeuronQueryMainWindow::updateDataTable()



void NeuronQueryMainWindow::about()
{
    QMessageBox::about(this, tr("About Books"),
            tr("<p>The <b>Books</b> example shows how to use Qt SQL classes "
               "with a model/view framework."));

}

