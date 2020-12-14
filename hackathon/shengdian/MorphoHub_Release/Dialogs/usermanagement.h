#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

//class UserManagement
//{
//public:
//    UserManagement();
//};
#include <QDialog>
#include <QtGui>
#include <QWidget>
#include "../basic_conf.h"
using namespace std;
class UserManagement: public QDialog
{
    Q_OBJECT
private:
    mFileSystem mfs; mUsers mus;
    QString userconfpath;
    QStringList userconfTitle;
    QStringList prioritylist;

    QGridLayout *mainLayout;
    QVBoxLayout *functionLayout;
    QGroupBox *functionGroupBox;

    QPushButton *saveButton;
    QPushButton *newButton;
    QPushButton *deleteButton;

    QTabWidget *tabwidget;
    QTableWidget* userConfTab;
    QTextEdit *logTextEdit;

public:
    explicit UserManagement(mFileSystem imfs,QWidget *parent = 0);
    ~UserManagement();
    UserManagement();
    void userManagementInit();
    void createTabView();
    void createActions();
    QTableWidget* createTableUserData();
    void toLog(const QString &logtext);

public slots:
    void saveButton_slot();
    void newButton_slot();
    void deleteButton_slot();
    void reviseTab_slot(int r,int c);
};
#endif // USERMANAGEMENT_H
