#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
//#include <QApplication>
#include "morphohub_mainwindow.h"

MorphoHub_MainWindow::MorphoHub_MainWindow(QWidget *parent)
    :QMainWindow(parent)
{
    setWindowTitle(tr("MorphoHub"));
    NewDBAction = new QAction(/*QIcon(":/images/doc-open"), */tr("&New DB..."), this);
    NewDBAction->setShortcuts(QKeySequence::Open);
    NewDBAction->setStatusTip(tr("Setup a new DB"));
    connect(NewDBAction, SIGNAL(triggered()), this, SLOT(open()));

    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(NewDBAction);
    QToolBar *toolBar = addToolBar(tr("&File"));
    toolBar->addAction(NewDBAction);

    statusBar() ;
}
void MorphoHub_MainWindow::open()
{
    QMessageBox::information(this, tr("Information"), tr("Open"));
}
