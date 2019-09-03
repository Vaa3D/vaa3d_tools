#ifndef MORPHOHUB_MAINWINDOW_H
#define MORPHOHUB_MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

class MorphoHub_MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MorphoHub_MainWindow(QWidget *parent = 0);
public:
    //store the basic init folder list for db.
    QStringList DBBasicConf;
    QStringList workingspaceConf;

    MorphoHub_MainWindow();
    ~MorphoHub_MainWindow();
private:

    QString dbpath;

    void createActions();
    void createMenus();
    void setMainLayout();
    /*..............Database basic action............*/
    QAction *NewDBAction;
    QAction *SetDBAction;

    /*Initialization*/
    void MorphoHub_Init();

    QGridLayout *gridlayout;

private slots:
    void NewDB_slot();
    void SetDB_slot();
signals:
    
public slots:
    
};

#endif // MORPHOHUB_MAINWINDOW_H
