/* BoutonDectection_plugin.h
 * designed by shengdian
 * 2020-7-29 : by SD-Jiang
 */
 
#ifndef __BOUTONDECTECTION_PLUGIN_H__
#define __BOUTONDECTECTION_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <vector>
using namespace std;
class BoutonDectectionPlugin : public QObject, public V3DPluginInterface2_1
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
void getBoutonInTerafly(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int allnode=0);
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int block_size);
QList <CellAPO> getBouton(NeuronTree nt,int threshold,int allnode=0);
void getBoutonInImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt,int useNeighborArea=0);
void printHelp();
NeuronSWC nodeIntensity(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size);
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size=256);
#endif

