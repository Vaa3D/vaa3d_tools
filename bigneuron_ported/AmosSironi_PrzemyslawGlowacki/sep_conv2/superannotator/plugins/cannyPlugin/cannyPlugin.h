#ifndef CANNYPLUGIN_H
#define CANNYPLUGIN_H

/** We shouldn't need a .h file
  * but it is necessary for the Q_OBJECT correctness (vtable)
  */

#include <PluginBase.h>
#include <cstdlib>
#include <QMessageBox>

#include <mainwindow.h>

class CannyPlugin : public PluginBase
{
    Q_OBJECT
private:
    const PluginServices* mPluginServices;
    MainWindow* plugin_window;

public:
    CannyPlugin(QObject *parent = 0) : PluginBase(parent) {}

    bool    initializePlugin( const PluginServices &pServices )
    {
        mPluginServices = &pServices;
        plugin_window = 0;

        /** Add a menu item **/
        for (unsigned int i=0; i < mPluginServices->getMaxOverlayVolumes(); i++) {
            QAction *action = mPluginServices->getPluginMenu()->addAction( QString("Run Canny %1").arg(i+1) );
            action->setData( i );
            connect( action, SIGNAL(triggered()), this, SLOT(run()) );
        }

        /** Add a menu item **/
        QAction *action = mPluginServices->getPluginMenu()->addAction( "Show message box" );
        connect( action, SIGNAL(triggered()), this, SLOT(showMsgBoxClicked()) );

        return true;
    }

    // must return the plugin's name
    QString pluginName() {
        return "Canny plugin";
    }

public slots:
    void run();

    void showMsgBoxClicked()
    {
        QMessageBox::information( mPluginServices->getMainWindow(), "Clicked me!", "You have just clicked me." );
    }
};

#endif // CannyPlugin_H
