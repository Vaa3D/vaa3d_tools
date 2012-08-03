#ifndef PAGES_H
#define PAGES_H

#include <QWidget>
#include <v3d_interface.h>
#include <QHash>

class QLabel;
class QPushButton;
class QListWidget;
class QComboBox;


class AutoPipePage:public QWidget
{
    Q_OBJECT
public:
    AutoPipePage(QWidget *parent=0);
    void SetCallback( V3DPluginCallback &callback);
    void setPluginsHash(const QHash<QString, QString>& pluginsHash);
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
    QHash<QString, QString> m_pluginsHash;

};

class UserPipePage : public QWidget
{
    Q_OBJECT
public:
    UserPipePage(QWidget *parent = 0);
    void SetCallback( V3DPluginCallback &callback);
    void setPluginNames(const QStringList& pluginNames);
    void setPluginsHash(const QHash<QString, QString>& pluginsHash); 
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
    QStringList m_pluginNames;
    QHash<QString, QString> m_pluginsHash;
    V3DPluginCallback *callback;
    QString menu_name;

};

class UserFilterPage : public QWidget
{
    Q_OBJECT
public:
    UserFilterPage(QWidget *parent = 0);
    void SetCallback( V3DPluginCallback &callback);
    void setPluginNames(const QStringList& pluginNames);
    void setPluginsHash(const QHash<QString, QString>& pluginsHash); 
private:
    void setItemList();
public slots:
    void CallFilter();
private:
    QLabel *FilterName;
    QComboBox *FilterItem;
    QPushButton *StartButton;
    V3DPluginCallback *callback;
    QString menu_name;
    QStringList m_pluginNames;
    QHash<QString, QString> m_pluginsHash;

};

#endif
