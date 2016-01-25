/* color_render_ESWC_features_plugin.h
 * This plugin will render the feature values in ESWC file.
 * 2015-6-22 : by Hanbo Chen
 */
 
#ifndef __COLOR_RENDER_ESWC_FEATURES_PLUGIN_H__
#define __COLOR_RENDER_ESWC_FEATURES_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class neuron_render_ESWC_features : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

    void docolor(V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

