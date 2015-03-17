/* NeuroGPSTree_plugin.h
 * Trace dense network with multi-Neuron
 * Permit to provide soma swc file for the position of somas
 * 2015-3-16 : by ZhouHang
 * Main algorithm: Quan Tingwei, Zhou Hang, Zeng Shaoqun
 */
 
#ifndef __NEUROGPSTREE_PLUGIN_H__
#define __NEUROGPSTREE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <Eigen/Core>
#include <ngtypes/basetypes.h>
#include <ngtypes/volume.h>
#include <ngtypes/soma.h>
#include <ngtypes/tree.h>
typedef unsigned char UChar;
class BinarySettingsDockWidget;
class NeuroGLWidget;
class INeuronDataObject;
class BinaryFilter;
typedef std::shared_ptr<BinaryFilter> NGBinaryFilter;
class TraceFilter;
typedef std::shared_ptr<TraceFilter> NGTraceFilter;
class BridgeBreaker;
typedef std::shared_ptr<BridgeBreaker> NGBridgeBreaker;
class NeuroTreeCluster;
typedef std::shared_ptr<NeuroTreeCluster> NGNeuronTreeCluster;


class NeuroGPSTreePlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:

	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

private:
    struct input_PARA
    {
        QString inimg_file;
        V3DLONG channel;
        double xRes_, yRes_, zRes_;
        double binaryThreshold;
        std::string swcfile;
        int threadNum;//TODO:for openmp accelerate
    };

protected:

    void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

    bool ConvertVaa3dImg2NGImg(UChar*, int, int, int, double, double, double,
                               std::shared_ptr<INeuronDataObject>);

    bool SaveSeperateTree(const QString& filename, const QStringList &infostring);

    void AutoBinaryImage();

    void AutoTrace();
private:
    int X, Y, Z;
    double xres_, yres_, zres_;

    input_PARA PARA;

    NGBinaryFilter filter;
    NGTraceFilter traceFilter;
    NGBridgeBreaker breaker;
    NGNeuronTreeCluster treeCluster;
    std::shared_ptr<INeuronDataObject> OrigImage;
    std::shared_ptr<INeuronDataObject> BinImage;
    std::shared_ptr<INeuronDataObject> BackImage;
    std::shared_ptr<VectorVec3i> binPtSet ;
    std::shared_ptr<INeuronDataObject> soma;
    std::shared_ptr<INeuronDataObject> tree;
    std::shared_ptr<INeuronDataObject> treeConInfo;
    std::shared_ptr<INeuronDataObject> seperateTree;

};

#endif

