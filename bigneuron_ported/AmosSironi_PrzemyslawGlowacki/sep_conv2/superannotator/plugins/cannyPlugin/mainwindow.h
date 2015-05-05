#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <PluginBase.h>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:    
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    double getLowTh();
    double getHighTh();
    double getVar();

    void setPluginServices(const PluginServices* _pluginServices) { mPluginServices = _pluginServices; }
    void setVolumeIdx(unsigned int _idx) { volume_idx = _idx; }

private:
    Ui::MainWindow *ui;

    const PluginServices* mPluginServices;

    unsigned int volume_idx;

public slots:
    void updateEdgeMap();
};

#endif // MAINWINDOW_H
