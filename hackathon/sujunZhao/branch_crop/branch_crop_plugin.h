/* branch_crop_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2018-12-19 : by YourName
 */
 
#ifndef __BRANCH_CROP_PLUGIN_H__
#define __BRANCH_CROP_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <vector>

class TestPlugin : public QObject, public V3DPluginInterface2_1
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
struct block{
    QString name;
    XYZ small;
    XYZ large;
};
struct NeuronSWC : public BasicSurfObj
{
    int type;			// 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite, 4-Apical_dendrite, 5-Fork_point, 6-End_point, 7-Custom
    float x, y, z;		// point coordinates

    union{
    float r;			// radius
    float radius;
    };

    union{
    V3DLONG pn;				// previous point index (-1 for the first point)
    V3DLONG parent;				// previous point index (-1 for the first point)
    };

    V3DLONG level; //20120217, by PHC. for ESWC format
    QList<float> fea_val; //20120217, by PHC. for ESWC format

    V3DLONG seg_id; //this is reused for ESWC format, 20120217, by PHC
    V3DLONG nodeinseg_id; //090925, 091027: for segment editing

    V3DLONG creatmode;      // creation mode LMG 8/10/2018
    double timestamp;		// timestamp  LMG 27/9/2018

    double tfresindex;         // TeraFly resolution index LMG 13/12/2018

    operator XYZ() const { return XYZ(x, y, z); }
    NeuronSWC () {n=type=pn=0; x=y=z=r=0; seg_id=-1; nodeinseg_id=0; fea_val=QList<float>(); level=-1; creatmode=0; timestamp=0; tfresindex=0;}
};
void get_branches(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QList<int> get_branch_points(NeuronTree nt, bool include_root);
XYZ offset_XYZ(XYZ input, XYZ offset);
void crop_swc(QString input_swc, QString output_swc, block crop_block);
void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
LandmarkList get_missing_branches_menu(V3DPluginCallback2 &callback, QWidget *parent, Image4DSimple * p4DImage);
#endif

