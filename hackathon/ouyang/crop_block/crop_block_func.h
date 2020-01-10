#ifndef CROP_BLOCK_FUNC_H
#define CROP_BLOCK_FUNC_H

#include "basic_surf_objs.h"
#include "v3d_interface.h"
#include <vector>
#include <string>
#include "v3d_message.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include <sstream>
using namespace std;
struct TBlock   //定义block的坐标
{
    long t;
    long tx;
    long ty;
    long tz;
    friend bool operator<(const TBlock & x,const TBlock & y)

    {
        return x.t<y.t;   //因为插入的类型是自定义的，不是基本类型（基本类型有默认的排序准则），因此需要重载 < 运算符。（相当于给自定义类型一个排序准则）。
    }

};


void crop_bt_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
void crop_defined_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QList<int> get_branch(NeuronTree nt);
QList<int> get_tips(NeuronTree nt, bool include_root);
int CheckSWC(V3DPluginCallback2 &callback, QWidget *parent);
bool writeMarker_file(const QString & filename, LandmarkList & listMarker);
bool readSWC_file(string swc_file, vector<MyMarker*> & swc);
#endif // CROP_BLOCK_FUNC_H


