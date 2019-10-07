#ifndef MORPHOHUB_MAINWINDOW_H
#define MORPHOHUB_MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include "../MainDialog/maindialog.h"
#include "../MainDialog/sourcedatamanagement.h"
#include "../BasicInfo.h"
using namespace std;

class MorphoHub_MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MorphoHub_MainWindow(QWidget *parent = 0);
public:
    QList<ReconstructionInfo> getReconstuctionsFromLevel(const QString& levelid);

    //store the basic init folder list for db.
    QStringList DBBasicConf;
    QStringList workingspaceConf;
    QStringList workingspaceContentConf;
    QStringList initworkingspaceTablist;
    QStringList datatitle;//for tab data
    //status lable
    QLabel *statusLabel;

    MorphoHub_MainWindow();
    ~MorphoHub_MainWindow();
private:
    QWidget *originparent;
    QString dbpath;
     /*..............For Annotation ............*/
    AnnotationProtocol seuallenAP;
    Annotator curOperator;
    AnnotationProtocolLevel curLevel;

    void createActions();
    QMenu *file;
    QMenu *managementMenu;
    QMenu *funcs;
    QMenu *levelControlMenu;
    QMenu *menuWindow;
    QMenu *helpMenu;
    QToolBar *dbToolbar;
    QToolBar *funcsToolbar;
    QToolBar *levelControlToolbar;

    /*..............Database layout ............*/
    QWidget *mainWidget;
    QHBoxLayout *mainlayout;

    /*..............MainLog dock windows............*/
    QDockWidget *MainLogwidget;
    QTextEdit *logtextedit;//for log
    void toLogWindow(const QString& logtext);

    void createMenus();
    void createStatusBar();
    void createToolBar();
    void updateStatusBar(const QString& showtext);
    void setMainLayout();
    /*..............Database basic action............*/
    QAction *NewDBAction;
    QAction *SetDBAction;
    /*..............Management dialog............*/
    SourceDataManagement *sdconf_dialog;
    /*..............Management actions............*/
    QAction *sdconfAction;
    QAction *annotatorconfAction;

    /*Initialization*/
    void MorphoHub_Init();
    void InitofAnnotationProtocol();

    QString contentindex;
    QTabWidget *dataTabwidget;
    QList<QTableWidget*> datatablelist;//all the created qtable will have a pointer here.
    QStringList datatabletitlelist;//all the tab name of the created qtable
    QTableWidget* createTableDataLevel(QList<ReconstructionInfo> levelres);
    void updateTableDataLevel(QTableWidget* t,QList<ReconstructionInfo> levelres);
    /*..............Database main dialog............*/
    MainDialog *morphoHub_dialog;

    /*Qtreewidget for content*/
    void createContentTreeWidget();
    QTreeWidget *contentTreewidget;
    QTreeWidgetItem *content_workingspace;
    QTreeWidgetItem *content_basicData;

    /****************Protocol functions*************************/
    ReconstructionInfo curRecon;//This is current reconstruction. you can get info from here.
    /*..............Protocol actions............*/
    QAction *commitAction;
    QAction *checkAction;
    QAction *skipAction;
    QAction *rollbackAction;
    QAction *reassignAction;
    QAction *releaseAction;
    MainDialog *commitDialog;

    /*..............Login Dialog............*/
    QToolBar *loginToolbar;
    QDialog *loginDialog;
    QGridLayout *loginMainlayout;
    QAction *loginAction;
    QLineEdit *loginUserIDQLineEdit;
    QLineEdit *loginPasswordQlineedit;
    QPushButton *loginCancelButton;
    QPushButton *loginOkayButton;

    QAction *logoutAction;

private slots:
    void NewDB_slot();
    void SetDB_slot();
    void createTabWindow();
    void removeSubTab(int subindex);
    void dataTabChange(int tabindex);
    void contentValueChange(QTreeWidgetItem *item,int column);
    void celltableInfoUpdate(int row,int column);
    void loginAction_slot();
    void loginOkayButton_slot();
    void loginCancelButton_slot();
    void logoutAction_slot();

signals:
    
public slots:
    //Source data management
    void sourceDataMAction();
    /*..............Protocol slots............*/
    void commitAction_slot();
    void checkAction_slot();
    void skipAction_slot();
    void rollbackAction_slot();
    void reassignAction_slot();

};

#endif // MORPHOHUB_MAINWINDOW_H
