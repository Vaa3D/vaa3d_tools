

#ifndef PAGES_H
#define PAGES_H

#include <QWidget>
#include <QtGui>
#include <v3d_interface.h>

class AutoPipePage:public QWidget
{
    Q_OBJECT
public:
    AutoPipePage(QWidget *parent=0);
    void SetCallback( V3DPluginCallback &callback);
public slots:
    void CallPipeline();
    void CreateIcon();
private:
    QLabel *pipelineName;
    QComboBox *pipelineExp;
    QPushButton *StartPipe;
    QListWidget *PaintFilter;
    V3DPluginCallback *callback;
    QString menu_name;

};

class UserPipePage : public QWidget
{
    Q_OBJECT
public:
    UserPipePage(QWidget *parent = 0);
    void SetCallback( V3DPluginCallback &callback);
signals:
    void countChanged();
public slots:
    void CallPipeline();
    void ADDItem();
    void CreatIcon();
private:
    QListWidget *SourceFilter;
    QListWidget *UsedFilter;
    QPushButton *StartPipe;
    QPushButton *AddItem;
    QPushButton *ClearItem;
    QLabel *SourceLabel;
    QLabel *UsedLabel;
    QListWidget *PaintFilter;
    V3DPluginCallback *callback;
    QString menu_name;

};

class UserFilterPage : public QWidget
{
    Q_OBJECT
public:
    UserFilterPage(QWidget *parent = 0);
    void SetCallback( V3DPluginCallback &callback);
public slots:
    void CallFilter();
private:
    QLabel *FilterName;
    QComboBox *FilterItem;
    QPushButton *StartButton;
    V3DPluginCallback *callback;
    QString menu_name;

};

#endif
