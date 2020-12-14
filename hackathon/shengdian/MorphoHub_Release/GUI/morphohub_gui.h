#ifndef MORPHOHUB_GUI_H
#define MORPHOHUB_GUI_H

#include <QMainWindow>
#include <QtGui>
#include "v3d_interface.h"
#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <iostream>
#include <algorithm>
#include <QtCore>
#include <QDir>
#include <QSettings>
#include "../basic_conf.h"
#include "../Dialogs/imagedatamanagement.h"
#include "../Dialogs/usermanagement.h"
#include "../Service/screenwall.h"
#include "../Service/screenwall_func.h"
using namespace std;
class morphoHub_GUI : public QMainWindow
{
    Q_OBJECT
public:
    explicit morphoHub_GUI(V3DPluginCallback2 &callback,QWidget *parent = 0);
public:
    morphoHub_GUI();
    ~morphoHub_GUI();
private:
    QWidget *mainparent;
    mFileSystem mfs;
    V3DPluginCallback2 *morphoHubcallback;
    Annotator curOperator;

    /*Initialization*/
    void morphoHub_Init();
    void createActions();
    void setMainLayout();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void updateStatusBar(const QString& showtext);
    void checkUserState();

    /*Qtreewidget for content*/
    void createContentTreeWidget(bool init);
    void createTabWindow(bool init);
    QTreeWidget *contentTreewidget;
    QTreeWidgetItem *content_morphometry;
    QTreeWidgetItem *content_images;
    /*data tab*/
    QTabWidget *dataTabwidget;

    /*menu list*/
    QMenu *dbMenu;
    QMenu *managementMenu;
    QMenu *servicesMenu;
    QMenu *menuWindow;
    QMenu *helpMenu;
    /*tool bars*/
    QToolBar *dbToolBar;
    /*..............Database layout ............*/
    QWidget *mainWidget;
    QHBoxLayout *mainlayout;

    /*action list: db*/
    QAction *newDBAction;
    QAction *loadDBAction;
    QAction *settingsAction;
    QAction *setImgPathAction;

    /*action list: management*/
    QAction *imageManagementAction;
    ImageDataManagement *idataDialog;

    UserManagement *userManagementDialog;
    QAction *userManagementAction;
    /*action list:services*/
    QAction *monitorAction;

    /*login or out Dialog*/
    QToolBar *loginToolbar;
    QDialog *loginDialog;
    QGridLayout *loginMainlayout;
    QAction *loginAction;
    QLineEdit *loginUserIDQLineEdit;
    QLineEdit *loginPasswordQlineedit;
    QPushButton *loginCancelButton;
    QPushButton *loginOkayButton;
    QAction *logoutAction;

    /*action list: help*/
    QAction *helpAction;

    //bottom status lable
    QLabel *statusLabel;
    QLabel *userStatusLabel;

    //for operation log
    QDockWidget *MainLogwidget;
    void toLogWindow(const QString& logtext);
    QTextEdit *logtextedit;

private slots:
    //content tab
    void dataTabChange(int tabindex);
    void contentValueChange(QTreeWidgetItem *item,int column);
    //db slots
    void newDBAction_slot();
    void loadDBAction_slot();
    void setImgPathAction_slot();
    void settingsAction_slot();
    //management action slot
    void imageManagementAction_slot();
    //user management
    void userManagementAction_slot();
    //services action list
    void monitorAction_slot();

    //login and logout
    void loginAction_slot();
    void loginOkayButton_slot();
    void loginCancelButton_slot();
    void logoutAction_slot();
    //help action slot
    void helpAction_slot();
signals:

};


#endif // MORPHOHUB_GUI_H
