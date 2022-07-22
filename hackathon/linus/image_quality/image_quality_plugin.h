/* image_quality_plugin.h
 * Obtain image quality features
 * 2021-08-04 : by Linus Manubens-Gil
 */
 
#ifndef __IMAGEQUALITY_PLUGIN_H__
#define __IMAGEQUALITY_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class ImageQualityPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);
    Q_PLUGIN_METADATA(IID"com.janelia.v3d.V3DPluginInterface/2.1")

public:
    float getPluginVersion() const {return 1.12f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

