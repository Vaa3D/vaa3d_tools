#ifndef MAINDIALOG_H
#define MAINDIALOG_H
#include <QDialog>
#include <QtGui>
#include <QWidget>
#include "../BasicInfo.h"
using namespace std;
class MainDialog: public QDialog
{
    Q_OBJECT
private:
    QString dbpath;
    QGridLayout *mainLayout;

    AnnotationProtocol ApforthisDialog;
    Annotator author;

    ReconstructionInfo curNeuron;
    ReconstructionInfo nextNeuron;
public:
    explicit MainDialog(const QString &path,QWidget *parent = 0);
    MainDialog(QWidget *parent);
    ~MainDialog();
    QString choseFuction;
    void MainInit();
    void updateMainView();
    void clearMainView();
    void setFunction(const QString& func);
    void setCurNeuron(ReconstructionInfo inputNeuron);
    void setAnnotator(Annotator inputauthor);
    void setupDBpath(const QString &path);
    void setupAnnotationProtocol(AnnotationProtocol inputAP);
    void generateNextNeuron();

    /*..............Protocol functions............*/
    void InitofAnnotationProtocol();
    QStringList getNextlevelList();
public:
    QStringList dstpath;//commit path, back path and finish path.
    //gui

    QLabel *functionlistQLabel;
    QStringList functionlist;
    QComboBox *functionlistQComboBox;
    QLineEdit *sdataidLineEdit;
    QLineEdit *somaidLineEdit;
    QLineEdit *authorLineEdit;
    QPushButton *changeAuthorNameButton;
    QLineEdit *checkersLineEdit;
    QPushButton *addcheckerName;
    QLineEdit *nowlevelLineEdit;
    QLabel* nextlevelLabel;
    QLineEdit *nextlevel_WorkingSpace_LineEdit;
    QCheckBox *nextlevel_WorkingSpace_CheckBox;
    QLineEdit *nextlevel_NeuronArchives_LineEdit;
    QCheckBox *nextlevel_NeuronArchives_CheckBox;
    QCheckBox *nextlevel_Finished_CheckBox;
    QLineEdit *nextlevel_Finished_LineEdit;

    QLineEdit* removeLevel_WorkingSpace_LineEdit;
    QLineEdit* removeLevel_Finished_LineEdit;
    QLineEdit* removeLevel_Archives_LineEdit;
    QLineEdit* recovery_WorkingSpace_LineEdit;

    QPushButton *okayButton;
    QPushButton *cancelButton;

    void createMainView();

protected slots:
    void changeAuthorNameButton_slot();
    void addcheckerName_slot();
    void OnfunctionlistQComboBox_slot(int index);
    void okayButton_slot();
    void cancelButton_slot();

};

#endif // MAINDIALOG_H
