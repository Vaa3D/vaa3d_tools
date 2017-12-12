/* neuronrecon_func.h
 * a plugin to construct neuron tree(s) from detected signals
 * 09/11/2017 : by Yang Yu
 */
 
#ifndef __NEURONRECON_FUNC_H__
#define __NEURONRECON_FUNC_H__

#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"

// menus
int lineconstruct_menu(V3DPluginCallback2 &callback,QWidget * parent);
int localmaxima_menu(V3DPluginCallback2 &callback,QWidget * parent);
int bigneuron_menu(V3DPluginCallback2 &callback,QWidget * parent);
int deeplearning_menu(V3DPluginCallback2 &callback,QWidget * parent);

// functions
bool convertTrees2Pointcloud_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool samplingtree_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool finetunepoints_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool getbranchpoints_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool processpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool getStatisticsTracedNeurons_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool connectpointstolines_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool anisotropicimagefilter_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool sort_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool mergelines_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool attrace_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool trace_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool samplepc_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool test_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);

// methods in pipelines
bool lmpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool bnpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool dlpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);

// test
bool findpeaks_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);

// help info
void printHelp();

//
class ControlPanel: public QDialog
{
    Q_OBJECT

public:
    ControlPanel(V3DPluginCallback2 &v3d, QWidget *parent, int func);
    ~ControlPanel();

public:
    void lineconstruct(V3DPluginCallback2 &v3d, QWidget *parent); // 1
    void localmaxima(V3DPluginCallback2 &v3d, QWidget *parent); // 2
    void bigneuron(V3DPluginCallback2 &v3d, QWidget *parent); // 3
    void deeplearning(V3DPluginCallback2 &v3d, QWidget *parent); // 4

private slots:
    void _slot_start1();
    void _slot_start2();
    void _slot_start3();
    void _slot_start4();
    void _slot_close();
    void _slots_openFile();

public:
    QLineEdit *m_le_filename;
    QSpinBox *pknn, *pthresh, *pdist;

    QLineEdit *m_le_model, *m_le_train, *m_le_mean;
    QSpinBox *pStep, *pWin; // searching windows (radius)

    V3DPluginCallback2 &m_v3d;
    static ControlPanel *m_controlpanel;
};

#endif // __NEURONRECON_FUNC_H__

