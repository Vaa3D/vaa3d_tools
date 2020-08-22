/* CheckSWC_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-11-18 : by YourName
 */
 
#ifndef __CHECKSWC_PLUGIN_H__
#define __CHECKSWC_PLUGIN_H__
//#define __SET_MARKER_DEGREE__
#include <QtGui>
#include "v3d_interface.h"
#include "my_surf_objs.h"
#include <set>
#include "main.h"
#include "v3d_message.h"


class TestPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.1f;}

    QStringList menulist() const;
    void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

    QStringList funclist() const ;
    bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif
