#ifndef MORPHOHUB_GUI_H
#define MORPHOHUB_GUI_H
#include <QWidget>
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
#include "morphohub_dbms_basic.h"
#include "GUI/newimage_gui.h"
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
    mDatabase mDB;
    V3DPluginCallback2 *morphoHubcallback;

    /*Initialization*/
    void morphoHub_Init();
    void createActions();
    void setMainLayout();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void updateStatusBar(const QString& showtext);

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
    /*Image part*/
    QAction *newImageAction;
    QAction *deleteImageAction;
    QAction *updateImageAction;
    QAction *download_Img_metadataAction;
    /*Soma part*/
    QAction *newSomataAction;/*from apo file; from metadata*/
    QAction *deleteSomataAction;
    QAction *updateSomataAction;
    QAction *download_Soma_metadataAction;
    /*Morphometry part*/
    QAction *newMorphoAction;
    QAction *deleteMorphoAction;
    QAction *updateMorphoAction;
    QAction *download_Morpho_metadataAction;
    QAction *download_MorphoAction;
    /*User part*/

    /*action list:services*/
    QAction *monitorAction;

    /*login or out Dialog*/
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
    //database slots
    void newDBAction_slot();
    void loadDBAction_slot();
    void setImgPathAction_slot();
    void settingsAction_slot();
    //management action slot
    /*Image part*/
    void newImageAction_slot();
    void deleteImageAction_slot();
    void updateImageAction_slot();
    void download_Img_metadataAction_slot();
    /*Soma part*/
    void newSomataAction_slot();
    void deleteSomataAction_slot();
    void updateSomataAction_slot();
    void download_Soma_metadataAction_slot();
    /*Morphometry part*/
    void newMorphoAction_slot();
    void deleteMorphoAction_slot();
    void updateMorphoAction_slot();
    void download_Morpho_metadataAction_slot();
    void download_MorphoAction_slot();
    //services action list
    void monitorAction_slot();
    //login and logout
    //help action slot
    void helpAction_slot();
signals:

};


#endif // MORPHOHUB_GUI_H
