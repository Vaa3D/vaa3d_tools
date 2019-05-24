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

    /*********************main Item********************************/
    QGridLayout *mainLayout;

    QGroupBox *mainItemGroupBox;
    QVBoxLayout *mainItemLayout;
    QLabel *mainItemLable;
    QComboBox *mainItemComboBox;

    /*********************submian Item********************************/
    QGroupBox *submainItemGroupBox;
    QVBoxLayout *submainItemLayout;
    QLabel *submainItemLable;
    QString submainItemLableName;
    QComboBox *submainItemComboBox;
    /*********************main Item********************************/

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

    /*********************Manipulate functions********************************/
    QGroupBox *manipulateGroupBox;
    QHBoxLayout *manipulateLayout;
    QPushButton *SelectPushButton;
    QPushButton *NewPushButton;
    QPushButton *DeletePushButton;
    QPushButton *SearchPushButton;

    /*********************Login Interface********************************/
    QGroupBox *loginGroupBox;
    QLabel *userLabel;
    QLabel *userNameLabel;
    QLabel *priorityLabel;
    QLabel *priorityNameLabel;
    QPushButton *loginOroutButton;
    QGridLayout *loginLayout;

    /*********************Detail view********************************/
    QTextEdit *basicViewTextEdit;
    QVBoxLayout *basicViewLayout;
    QGroupBox *basicViewGroupBox;

    QDialog *adddialog;
    QLineEdit *brainNameLineEdit;
    QComboBox *stateCombox;
    QComboBox *priorityAddCombox;
    QLineEdit *commentsLineEdit;
    QLineEdit *xCooridnateLineEdit;
    QLineEdit *yCoordinateLineEdit;
    QLineEdit *zCoordinateLineEdit;
    QLineEdit *sizeLineEdit;
    QLineEdit *bitLineEdit;

    void Initnumber();

    void setbasicViewText(const QString &showText);

protected:
    void createTabView();
    void createMainMenu();
    void createSubMainMenu();
    void createTableFunction();
    void createLoginMenu();
    void createBasicView();

protected slots:
    void newButton_slot();
    void deleteButton_slot();
    void addbrain_slot();
    void cancelbrain_slot();


signals:
    
public slots:
    void mainItemComboBox_slot();
    void submainItemComboBox_slot();
    void loginButton_slot();
    
};

#endif // DBMSDIALOG_H
