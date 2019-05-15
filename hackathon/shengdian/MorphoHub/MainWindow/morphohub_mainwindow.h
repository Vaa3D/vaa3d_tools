#ifndef MORPHOHUB_MAINWINDOW_H
#define MORPHOHUB_MAINWINDOW_H

#include <QMainWindow>

class MorphoHub_MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MorphoHub_MainWindow(QWidget *parent=0);
    ~MorphoHub_MainWindow();
private:

    QAction *NewDBAction;
private slots:
    void open();
};

#endif // MORPHOHUB_MAINWINDOW_H
