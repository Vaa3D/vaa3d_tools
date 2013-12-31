
//by Hanchuan Peng
//2009-06-26

#ifndef __example_reset_xyz_resolution_PLUGIN_H__
#define __example_reset_xyz_resolution_PLUGIN_H__


#include <v3d_interface.h>

class example_reset_xyz_resolutionPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1)

public:
     float getPluginVersion() const {return 1.2f;}

     QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
     bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

};

#endif
