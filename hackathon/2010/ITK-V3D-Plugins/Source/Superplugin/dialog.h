#ifndef DIALOG_H
#define DIALOG_H
#include <QtGui>
#include <v3d_interface.h>
#include "pages.h"

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class Dialog: public QDialog
{
    Q_OBJECT
public:
    Dialog();
    void SetCallback(V3DPluginCallback &callback);
public slots:
    void changePage(QListWidgetItem *currunt,QListWidgetItem *previous);
private:
    void createIcons();

    AutoPipePage * autoPipepage;
    UserPipePage * userPipepage;
    UserFilterPage *userFilterpage;
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
};

#endif // DIALOG_H
