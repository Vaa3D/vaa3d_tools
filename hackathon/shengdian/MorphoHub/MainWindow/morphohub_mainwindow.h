#ifndef MORPHOHUB_MAINWINDOW_H
#define MORPHOHUB_MAINWINDOW_H

#include <QMainWindow>
#include <v3d_interface.h>
#include "DBMS/dbmsdialog.h"
class MorphoHub_MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class DBMSDialog;
public:
    MorphoHub_MainWindow(/*V3DPluginCallback2 *callback, */QWidget *parent=0);
    MorphoHub_MainWindow();
    ~MorphoHub_MainWindow();
protected:
private:
    QWidget *originparent;
    DBMSDialog *dbmsdialog;

    QString dbpath;

    QWidget *logwideget;
    QGridLayout *gridlayout;
    //setup a new Database

    /*..............file action............*/
    QAction *NewDBAction;
    QAction *SetDBAction;

    /*..............service action............*/
    //Annotation service action
    QAction *AnnotationSAction;
    void createAnnotationService();

    //DBMS service
    QAction *DBMSAction;
    void createDBMSService();

    QTextEdit *logtext;

    void createActions();
    void createServices();
    void createMenus();

    void setMainLayout();
private slots:
    void NewDB_slot();
    void SetDB_slot();

    /*..............service slot............*/
    void Annotation_slot();
    void DBMS_slot();
};

#endif // MORPHOHUB_MAINWINDOW_H
