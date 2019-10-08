#ifndef SOURCEDATAMANAGEMENT_H
#define SOURCEDATAMANAGEMENT_H
#include <QDialog>
#include <QWidget>
#include <QtGui>
#include "../BasicInfo.h"
#include "basic_surf_objs.h"
using namespace std;
class SourceDataManagement:public QDialog
{
    Q_OBJECT
public:
    explicit SourceDataManagement(const QString &path,QWidget *parent = 0);
    ~SourceDataManagement();
    SourceDataManagement();
    void setupDBpath(const QString &path);
    void toLog(const QString &logtext);
    QTableWidget* createTableSourceData();
    QTableWidget* createTableSomaConf(const QString& somaconfpath);
//    QList<SourceDataInfo> getSourceDataConf();
private:
    void SDMInit();
    void createTabView();
    QGridLayout *mainLayout;
    //QHBoxLayout *tabLayout;

    QVBoxLayout *functionLayout;
    QGroupBox *functionGroupBox;
    QPushButton *saveButton;
    QPushButton *newSDButton;
    QPushButton *deleteSDButton;
    QPushButton *newSDitemButton;
    QPushButton *deleteSDitemButton;

    QPushButton *loadSomalist2Conf;

    QTextEdit *logTextEdit;

    QString dbpath;
    QString sourcedataconfpath;
    QStringList sourcedataconfTitle;
    QStringList somaconfTitle;


    QTabWidget *tabwidget;
    QTableWidget* sourcedataTab;

    void createActions();
public slots:
    void removeSubTab(int subindex);
    void switchTab(int toindex);
    void createSomaTab_slot(int row, int column);
    void reviseSDTab_slot(int row,int column);

    void saveButton_slot();
    void newSDButton_slot();
    void deleteSDButton_slot();
    void loadSomalist2Conf_slot();
};

#endif // SOURCEDATAMANAGEMENT_H
