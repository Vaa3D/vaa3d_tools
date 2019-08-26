/* get_samples_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-4-29 : by Yongzhang
 */
 
#ifndef __GET_SAMPLES_PLUGIN_H__
#define __GET_SAMPLES_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

struct input_PARA
{
    QString inimg_file,swc_file,out_file,soma_file;
    V3DLONG in_sz[3];

};

class get_samplesPlugin : public QObject, public V3DPluginInterface2_1
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

bool get_train_smaples(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input, V3DPluginArgList & output,input_PARA &P);
QStringList importFileList_addnumbersort(const QString & curFilePath);
NeuronTree resize(NeuronTree &nt,double n);
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
bool get_train_smaples_soma(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input, V3DPluginArgList & output,input_PARA &P);
bool sort_with_standard(QList<NeuronSWC>  & neuron1, QList<NeuronSWC> & neuron2,QList<NeuronSWC>  &result);

#endif

