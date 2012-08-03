#ifndef _SUPERPLUGIN_H_
#define _SUPERPLUGIN_H_

#include <QObject>
#include <v3d_interface.h>


class Superplugin: public QObject, public V3DPluginInterface2
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2)
public:
    QStringList menulist()const;
    QStringList funclist()const;
    void domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget* parent);
    bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif
