/* Deep_Learning_Handler_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2017-11-29 : by YourName
 */
 
#ifndef __DEEP_LEARNING_HANDLER_PLUGIN_H__
#define __DEEP_LEARNING_HANDLER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class DL_Handler : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

	void cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);
};

#endif

