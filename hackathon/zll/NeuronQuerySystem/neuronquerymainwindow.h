#ifndef NEURONQUERYMAINWINDOW_H
#define NEURONQUERYMAINWINDOW_H

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QTextEdit>
#include <QComboBox>
#include <QTableWidget>
#include "NeuronQuerySystem_plugin.h"
#include "basicinfo.h"
namespace Ui {
class NeuronQueryMainWindow;
}

class NeuronQueryMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NeuronQueryMainWindow(V3DPluginCallback2 &callback,QWidget *parent = nullptr);
    ~NeuronQueryMainWindow();

private:
    QWidget *originParent;
    V3DPluginCallback2 *QueryNeuroncallback;
    Handler author;

    /*..............Login or out Dialog............*/
    QToolBar *loginToolbar;
    QDialog *loginDialog;
    QGridLayout *loginMainlayout;
    QAction *loginAction;
    QLineEdit *loginUserIDQLineEdit;
    QLineEdit *loginPasswordQlineedit;
    QPushButton *loginCancelButton;
    QPushButton *loginOkayButton;
    QAction *logoutAction;
    QLabel *userStatusLabel;
    QTextEdit *logtextedit;//for log
    QLineEdit *userID_QLineEdit;

    QAction *quitAction;
    QAction *aboutAction;
    QAction *aboutQtAction;


    QDir currentDir;

    void init();

    void createMenuBar();

    void about();
    void toLogWindow(const QString& logtext);
    static void updateComboBox(QComboBox &comboBox);
    void animateFindClick();
    void animateFindClickAno();
    void showSwcTables(const QStringList &paths);
    void showAnoTables(const QStringList &paths);
    QStringList queryFiles(const QStringList &files, const QString &text);

private slots:
    void browse();
    void openFileOfItem(int row, int column);
    void contextMenuSwc(const QPoint &pos);
    void contextMenuAno(const QPoint &pos);
    void pop3Dview(const QString &fileName);
    void popTerafly(const QString &fileName);

//    void on_ClickToQueryApo_customContextMenuRequested(const QPoint &pos);

//    void on_loadApoButton_customContextMenuRequested(const QPoint &pos);

    void on_querySwcButton_clicked();
    void on_queryAnoButton_clicked();
    void on_queryApoButton_clicked();

    void on_LoadSwcButton_clicked();



    void loginAction_slot();
    void loginOkayButton_slot();
    void loginCancelButton_slot();
    void logoutAction_slot();


//    void selectAll();

//    void dataTabChange(int index);

    void on_querySwcButton_customContextMenuRequested(const QPoint &pos);
//    QTableWidget *table[1+6];
//    for (int i=0; i<=6; i++)  table[i]=0;


protected:
//    void createTables();
//    void clearTables();
//    QTableWidget* currentTableWidget();
//    QTableWidget* createTableSwc();
//    QTableWidget* createTableApo();
//    QTableWidget* createTableAno();

private:
    Ui::NeuronQueryMainWindow *ui;
};

#endif // NEURONQUERYMAINWINDOW_H
