#ifndef PAGES_H
#define PAGES_H

#include <QWidget>
#include <v3d_interface.h>

class QLabel;
class QPushButton;
class QListWidget;
class QComboBox;
class ItkPluginManager;

class AutoPipePage:public QWidget
{
    Q_OBJECT
public:
    explicit AutoPipePage(QWidget *parent=0);
    void setItkPluginManager( ItkPluginManager* itkPluginManager);
    void initialTest();
public slots:
    void CallPipeline();
    void CreateIcon();
private:
    bool validatePluginExits(const QStringList& pluginList);
private:
    QLabel *pipelineName;
    QComboBox *pipelineExp;
    QPushButton *StartPipe;
    QListWidget *PaintFilter;
    ItkPluginManager* itkPluginManager;
    QList<QStringList> m_pipeLineList;
};

class UserPipePage : public QWidget
{
    Q_OBJECT
public:
    explicit UserPipePage(QWidget *parent = 0);
    void setItkPluginManager( ItkPluginManager* itkPluginManager);
    void getItkPluginList();
 //   void setPluginName( const QStringList &pluginName);
 //   void setPluginPath( const QStringList &pluginPath);
 private:
    void setSourceFilterList();
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
    QStringList itkPluginNames;
//    QStringList pluginPath;
    ItkPluginManager* itkPluginManager;
};

class UserFilterPage : public QWidget
{
    Q_OBJECT
public:
    explicit UserFilterPage(QWidget *parent = 0);
    void setItkPluginManager( ItkPluginManager* itkPluginManager);
    void getItkPluginList();
//    void SetCallback( V3DPluginCallback &callback);
//    void setPluginName( const QStringList &pluginName);
//    void setPluginPath( const QStringList &pluginPath);
private:
    void setItemList();
public slots:
    void CallFilter();
private:
    QStringList itkPluginNames;
 //   QStringList pluginPath;
    QLabel *FilterName;
    QComboBox *FilterItem;
    QPushButton *StartButton;
    ItkPluginManager* itkPluginManager;
};
#endif

