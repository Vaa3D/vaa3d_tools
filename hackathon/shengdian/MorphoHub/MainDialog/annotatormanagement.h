#ifndef ANNOTATORMANAGEMENT_H
#define ANNOTATORMANAGEMENT_H
#include <QDialog>
#include <QtGui>
#include <QWidget>
#include "../BasicInfo.h"
using namespace std;
class AnnotatorManagement: public QDialog
{
    Q_OBJECT
private:
    QString dbpath;
    QString userconfpath;
    QGridLayout *mainLayout;
    QVBoxLayout *functionLayout;
    QGroupBox *functionGroupBox;

    QPushButton *saveButton;
    QPushButton *newButton;
    QPushButton *deleteButton;

    QTabWidget *tabwidget;
    QTableWidget* userConfTab;
    QTextEdit *logTextEdit;
    QStringList userconfTitle;
    QStringList prioritylist;

public:
    explicit AnnotatorManagement(const QString &path,QWidget *parent = 0);
    ~AnnotatorManagement();
    AnnotatorManagement();
    void setupDBpath(const QString &path);
    void UserManagementInit();
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

#endif // ANNOTATORMANAGEMENT_H
