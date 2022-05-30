#include "neuronquerymainwindow.h"
#include "ui_neuronquerymainwindow.h"
#include "NeuronQuerySystem_plugin.h"

#include <QDialog>
#include <QMessageBox>
NeuronQueryMainWindow::NeuronQueryMainWindow(V3DPluginCallback2 &callback,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NeuronQueryMainWindow)
{
    ui->setupUi(this);
    QueryNeuroncallback=&callback;
    this->setCentralWidget(ui->dataTabWidget);
    this->createMenuBar();

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

void NeuronQueryMainWindow::about()
{
    QMessageBox::about(this, tr("About Books"),
            tr("<p>The <b>Books</b> example shows how to use Qt SQL classes "
               "with a model/view framework."));

}



void NeuronQueryMainWindow::on_ClickToQueryApo_customContextMenuRequested(const QPoint &pos)
{

}


void NeuronQueryMainWindow::on_loadApoButton_customContextMenuRequested(const QPoint &pos)
{

}


void NeuronQueryMainWindow::on_querySwcButton_clicked()
{

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
    loginDialog->setWindowTitle("Neuron-Query-System-Login-Window");

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


