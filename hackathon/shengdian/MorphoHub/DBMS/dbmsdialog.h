#ifndef DBMSDIALOG_H
#define DBMSDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QtGui>
using namespace std;

enum DBMSFirstViewType { BrainBasic=0,
                BrainAnnotation=1
                };
enum DBMSFirstViewStateType{
    BUnconvert=0,
    BConverting=1,
    BConverted=2,
    BCompressed=3,
    BUpload=4
};

enum DBMSFirstViewPriorityType{
    BPNormal=0,
    BPHigh=1,
    BPSuperhigh=2
};

enum DBMSSecondViewType { SBrainID=0,
                SBrainSomaNumber=1,
                SBrainUndoPriority=2,
                SBrainUndoNumber=3,
                SBrainAnnotation_L0=4,
                SBrainAnnotation_L1=5,
                SBrainAnnnotation_L2=6
                };

class DBMSDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DBMSDialog(QWidget *parent = 0);

private:
    QGridLayout *mainLayout;

    QGroupBox *mainItemGroupBox;
    QVBoxLayout *mainItemLayout;
    QLabel *mainItemLable;
    QComboBox *mainItemComboBox;

    QTabWidget *tabwidget;
    QGroupBox *firstViewGroupBox;
    QHBoxLayout *firstViewLayout;
    QTableWidget *brainBasictable;

    QTableWidget* createTableBrainBasic();
    QTableWidget* createTableBrainAnnotation();

    QGroupBox *secondViewGroupBox;
    QHBoxLayout *secondViewLayout;
    QTableWidget *brainAnnotaiontable;

    QGroupBox *detailViewGroupBox;
    QHBoxLayout *detailViewLayout;

    QGroupBox *manipulateGroupBox;
    QVBoxLayout *manipulateLayout;
    QPushButton *NewPushButton;
    QPushButton *DetelePushButton;

    QGroupBox *serachGroupBox;
    QHBoxLayout *serachLayout;


protected:
    void createTabView();

signals:
    
public slots:
    void mainItemComboBox_slot();
    
};

#endif // DBMSDIALOG_H
