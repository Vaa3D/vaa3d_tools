/* image_save_channel_plugin.h
 * This plugin will save the channel of interest.
 * 2015-2-18 : by Hanbo Chen
 */
 
#ifndef __IMAGE_SAVE_CHANNEL_PLUGIN_H__
#define __IMAGE_SAVE_CHANNEL_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

#define V3DLONG long

class image_channel_io : public QObject, public V3DPluginInterface2_1
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

