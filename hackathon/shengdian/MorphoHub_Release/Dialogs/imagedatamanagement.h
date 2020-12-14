#ifndef IMAGEDATAMANAGEMENT_H
#define IMAGEDATAMANAGEMENT_H
#include <QDialog>
#include <QWidget>
#include <QtGui>
#include "../basic_conf.h"
#include "basic_surf_objs.h"
using namespace std;
class ImageDataManagement:public QDialog
{
    Q_OBJECT
public:
    explicit ImageDataManagement(mFileSystem imfs,QWidget *parent = 0);
    ImageDataManagement();
    ~ImageDataManagement();
    void toLog(const QString &logtext);

    QTableWidget* createTableImage();
private:
    mImageDB idb;mFileSystem mfs;QString imgconfpath;
    QStringList imgconfTitle;
    QTabWidget *tabwidget;
    QTableWidget* imageTab;

    void IDMInit();
    void createTabView();

    QGridLayout *mainLayout;
    QVBoxLayout *functionLayout;
    QGroupBox *functionGroupBox;
    QPushButton *saveButton;
    QPushButton *newButton;
    QPushButton *deleteButton;

    QPushButton *newitemButton; //need to complete
    QPushButton *deleteitemButton;//need to complete

    QTextEdit *logTextEdit;
    void createActions();
public slots:
    void removeSubTab(int subindex);
    void switchTab(int toindex);
    void reviseTab_slot(int row,int column);

    void saveButton_slot();
    void newButton_slot();
    void deleteButton_slot();
};

#endif // IMAGEDATAMANAGEMENT_H
