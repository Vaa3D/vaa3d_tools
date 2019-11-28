/* CheckSWC_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-11-18 : by YourName
 */
 
#ifndef __CHECKSWC_PLUGIN_H__
#define __CHECKSWC_PLUGIN_H__
#define __SET_MARKER_DEGREE__
#include <QtGui>
#include "v3d_interface.h"



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

int CheckSWC(V3DPluginCallback2 &callback, QWidget *parent);



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

#endif
