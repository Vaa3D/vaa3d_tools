
#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <v3d_interface.h>

class AutoPipePage;
class UserPipePage;
class UserFilterPage;
class ItkPluginManager;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;


class Dialog: public QDialog
{
    Q_OBJECT
public:
    explicit Dialog( QWidget *parent = 0);
    ~Dialog();
    void setCallback(V3DPluginCallback2 &callback);
    void setInitialDir (const QString& intialDir);
    void intialPluginManager ();
public slots:
    void changePage(QListWidgetItem *currunt,QListWidgetItem *previous);
private:
    void createIcons();
 //   void getAllThePluginsToCall();
  //  void cdFurtherDir(QDir &dir);
  //  void showAllTheFile();
 //   QString dirToSearch;
//    QStringList pluginName;
//    QStringList pluginPath;
    AutoPipePage * autoPipepage;
    UserPipePage * userPipepage;
    UserFilterPage *userFilterpage;
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
    ItkPluginManager* itkPluginManager;
};
#endif
