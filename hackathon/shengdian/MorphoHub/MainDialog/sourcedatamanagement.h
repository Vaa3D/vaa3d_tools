#ifndef SOURCEDATAMANAGEMENT_H
#define SOURCEDATAMANAGEMENT_H
#include <QDialog>
#include <QWidget>
#include <QtGui>
#include "../BasicInfo.h"
using namespace std;
class SourceDataManagement:public QDialog
{
    Q_OBJECT
public:
    explicit SourceDataManagement(const QString &path,QWidget *parent = 0);
    ~SourceDataManagement();
    SourceDataManagement();
    void setupDBpath(const QString &path);
    QTableWidget* createTableSourceData();
private:
    void SDMInit();
    void createTabView();
    QGridLayout *mainLayout;
    QHBoxLayout *tabLayout;
    QVBoxLayout *functionLayout;
    QString dbpath;
    QString sourcedataconfpath;
    QStringList sourcedataconfTitle;
    QStringList somaconfTitle;

    QTabWidget *tabwidget;
    QTableWidget* sourcedataTab;
public slots:
    void removeSubTab(int subindex);
};

#endif // SOURCEDATAMANAGEMENT_H
