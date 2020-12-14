/* UnsortedPlugin_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2020-8-29 : by Shengdian
 */
 
#ifndef __UNSORTEDPLUGIN_PLUGIN_H__
#define __UNSORTEDPLUGIN_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
using namespace std;
class UnsortedPlugin : public QObject, public V3DPluginInterface2_1
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

void getSWCIntensityInTerafly(V3DPluginCallback2 &callback, string imgPath, QString inswc_file);
void getTipComponent(QString inswc_file, QString outpath, int cropx, int cropy, int cropz);
//void getSomaFeature(V3DPluginCallback2 &callback, string imgPath, QString inapo_file);
void getSomaBlock(V3DPluginCallback2 &callback, string imgPath, QString inapo_file, QString outpath, int cropx, int cropy, int cropz);
void getMarkerRadius(unsigned char *&inimg1d, long in_sz[], NeuronSWC& s);
void getTipBlock(V3DPluginCallback2 &callback, string imgPath, QString inswc_file, QString outpath, int cropx, int cropy, int cropz);
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath, QString inapo, QString outpath, int cropx, int cropy, int cropz);
NeuronSWC nodeRefine(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size=4);
void getTeraflyBlock(V3DPluginCallback2 &callback, string imgPath, QList<CellAPO> apolist, string outpath, int cropx, int cropy, int cropz);
#endif

