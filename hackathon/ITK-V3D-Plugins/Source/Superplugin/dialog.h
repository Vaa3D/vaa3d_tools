#ifndef DIALOG_H
#define DIALOG_H
#include <QDialog>
#include <v3d_interface.h>
#include <QHash>

class AutoPipePage;
class UserPipePage;
class UserFilterPage;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QPushButton;

class Dialog: public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = 0);
    void SetCallback(V3DPluginCallback &callback);
    void setInitialDir( const QString& initialDir);
    void setVaa3DWorkingPluginsDir(const QString& workingDir);
    void initial();
    QStringList getPluginNames();
    QHash<QString, QString> getPluginsHash();
private slots:
    void changePage(QListWidgetItem *currunt,QListWidgetItem *previous);
    void onDirChangeButtonClicked();
private:
    void createIcons();
    bool searchAllItkPlugins(); 
    void getItkPluginFiles(QDir & dir);
private:
    AutoPipePage * autoPipepage;
    UserPipePage * userPipepage;
    UserFilterPage *userFilterpage;
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
    QString m_initialDir;
    QString m_vaa3DworkingPluginsDir;
    QStringList m_pluginNames;
    QHash<QString, QString> m_pluginsHash;//use for call other plugins
    QPushButton* m_dirChangeButton;
};

#endif // DIALOG_H
