#ifndef BIGSCREENDISPALY_FUNC_H
#define BIGSCREENDISPALY_FUNC_H

#include "v3d_interface.h"
#include "../MainDialog/screenwallDialog.h"
#include "basic_surf_objs.h"
#include <vector>

#define MYFLOAT double
#define MAXSIZE 100
#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z))
using namespace std;
struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
    V3DLONG level,seg_id;
    QList<float> fea_val;
};

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;
void resample_path(Segment * seg, double step);
NeuronTree resample(NeuronTree input, double step);
void CheckFileandFloder(DisplayPARA controlPara);
QStringList FindNewSWCFile(QStringList allSWCFile,int displayNumber);
void HelpText();
void MethodForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent,DisplayPARA controlPara);
void MethodFunForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input);
//void MethodForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent);
void MethodForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent,DisplayPARA controlPara);
void MethodFunForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input);
//int ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent);
void ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent,int samplingRate);
//void LoadAnchorFile();
void LoadAnchorFile(DisplayPARA controlPara);
void LoadAnchorFile(const V3DPluginArgList & input);
void printHelpForBigScreenUsage();
//void MethodFunForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent/*,int displayNum=9*/, const V3DPluginArgList & input);
void ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent,const V3DPluginArgList & input);
//NeuronTree resample(NeuronTree input, double step);
void MethodForCombineSWCDisplay(V3DPluginCallback2 &callback, QWidget *parent,QStringList nameList,DisplayPARA controlPara);
void MethodFunForCombineSWCDisplay(V3DPluginCallback2 &callback, QWidget *parent,QStringList nameList);
QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code,int displayNum);
void WriteNewFinishedNeuronsFileName(QStringList nameList);
void angles_to_quaternions(MYFLOAT q[], MYFLOAT xRot, MYFLOAT yRot,MYFLOAT zRot);
void angles_to_quaternions_3DRotation(MYFLOAT q[], MYFLOAT xRot, MYFLOAT yRot,MYFLOAT zRot);
void slerp_zhi(MYFLOAT q1[], MYFLOAT q2[], MYFLOAT t, MYFLOAT q_sample[]);
void quaternions_to_angles(MYFLOAT Rot_current[], MYFLOAT q_sample[]);
void quaternions_to_angles_3DRotation(MYFLOAT Rot_current[], MYFLOAT q[]);
MYFLOAT dot_multi(MYFLOAT q1[], MYFLOAT q2[]);
MYFLOAT dot_multi_normalized(MYFLOAT q1[], MYFLOAT q2[]);
#endif // BIGSCREENDISPALY_FUNC_H
