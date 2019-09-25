#ifndef MORPHOHUB_MAINWINDOW_H
#define MORPHOHUB_MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include "../MainDialog/maindialog.h"
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
    //status lable
    QLabel *statusLabel;

    MorphoHub_MainWindow();
    ~MorphoHub_MainWindow();
private:
    QWidget *originparent;
    QString dbpath;

    void createActions();
    QMenu *file;
    QMenu *funcs;
    QMenu *menuWindow;
    QMenu *helpMenu;
    QToolBar *dbToolbar;
    QToolBar *funcsToolbar;
    QToolBar *levelControlToolbar;

    /*..............Database layout ............*/
    QWidget *mainWidget;
    QHBoxLayout *mainlayout;
    QSplitter *LsplitterofM;
    QSplitter *RsplitterofM;

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

    /*Initialization*/
    void MorphoHub_Init();

    /*..............Database dock windows............*/
    QDockWidget *contentwidget;
    QAction *ContentAction;
    QString contentindex;
    QDockWidget *datawidget;
    QTabWidget *dataTabwidget;
    QAction *dataTabAction;
    QTableWidget *datatable;
    QTableWidget* createTableDataTotal();
    QTableWidget* createTableDataLevel(QList<ReconstructionInfo> levelres);
    /*..............Database main dialog............*/
    MainDialog *morphoHub_dialog;

    /*Qtreewidget for content*/
    void createContentTreeWidget();
    QTreeWidget *contentTreewidget;
    QTreeWidgetItem *content_workingspace;
    QTreeWidgetItem *content_basicData;

private slots:
    void NewDB_slot();
    void SetDB_slot();
    void createContentDockWindow();
    void createDataTabDockWindow();
    void removeSubTab(int subindex);
    void contentValueChange(QListWidgetItem *item);
    void contentValueChange(QTreeWidgetItem *item,int column);
signals:
    
public slots:
    
};

#endif // MORPHOHUB_MAINWINDOW_H
