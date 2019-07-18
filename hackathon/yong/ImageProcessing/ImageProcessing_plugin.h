/* ImageProcessing_plugin.h
 * This is a image processing plugin, you can use it as a demo.
 * 2018-10-27 : by Yongzhang
 */
 
#ifndef __IMAGEPROCESSING_PLUGIN_H__
#define __IMAGEPROCESSING_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "v3d_message.h"
#include <vector>
#include "iostream"
#include "basic_surf_objs.h"
#include "v3d_interface.h"

using namespace std;

class ImageProcessingPlugin : public QObject, public V3DPluginInterface2_1
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


struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};


#endif



