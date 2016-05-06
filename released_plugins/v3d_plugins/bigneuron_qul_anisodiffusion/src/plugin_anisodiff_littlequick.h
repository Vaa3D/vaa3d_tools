/* plugin_anisodiff_littlequick.h
 * 2015-03-18 : by Lei Qu
 */
 
#ifndef __PLUGIN_ANISODIFF_H__
#define __PLUGIN_ANISODIFF_H__

#include <QtGui>
#include <v3d_interface.h>

class AnisoDiffPlugin_littlequick : public QObject, public V3DPluginInterface2_1
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

