/* Mean_Shift_Center_plugin.h
 * Search for center using mean-shift
 * 2015-3-4 : by Yujie Li
 */
 
#ifndef __MEAN_SHIFT_CENTER_PLUGIN_H__
#define __MEAN_SHIFT_CENTER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "mean_shift_dialog.h"



class mean_shift_plugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
    void mean_shift_center(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output);

    QList <LocationSimple> readPosFile_usingMarkerCode(const char * posFile);
    QList <ImageMarker> readMarker_file(const QString & filename);
    void printHelp();

private:
    V3DLONG sz_img[4];
    unsigned char *image_data;
};

#endif

