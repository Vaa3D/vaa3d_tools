#include "cannyPlugin.h"
#include <QInputDialog>
#include <QLineEdit>

#include <mainwindow.h>

#include <sstream>

extern "C" Q_DECL_EXPORT PluginBase* createPlugin();

PluginBase *createPlugin()
{
    return new CannyPlugin();
}

void CannyPlugin::run()
{
    QAction *action = qobject_cast<QAction *>(sender());
    unsigned int idx = action->data().toUInt();

    qDebug("Overlay %d", idx);

    if(!plugin_window) {
        plugin_window = new MainWindow;
        plugin_window->setPluginServices(mPluginServices);
    }
    plugin_window->setVolumeIdx(idx);
    plugin_window->show();
}
