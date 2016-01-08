/* ENT_plugin.h
 * The plugin is called ensemble neuron tracer.
 * Developers: Ching-Wei Wang, Hilmil Pradana, Cheng-Ta Huang 
 * Institution: National Taiwan University of Science and Technology
 * Website: http://www-o.ntust.edu.tw/~cweiwang/
 * Email: cweiwang@mail.ntust.edu.tw
 * Release date: 2016-1-8
 */
 
#ifndef __APP2_PORTED_PLUGIN_H__
#define __APP2_PORTED_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class APP2_ported : public QObject, public V3DPluginInterface2_1
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

