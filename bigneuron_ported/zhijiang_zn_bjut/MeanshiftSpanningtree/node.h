#ifndef NODE_H
#define NODE_H
#include <v3d_interface.h>
class Node
{
public:
    V3DLONG x;//x轴坐标
    V3DLONG y;//y轴坐标
    V3DLONG z;//z轴坐标
    V3DLONG ind;//在hashmap中的key
    V3DLONG parent;//父亲节点
    V3DLONG number;//swc中编号
	int containByroot;//flag用于判断是否被其他的根节点包含
	double intensity;
	int class_mark;//类别标记
    QMap<V3DLONG,Node*> connect;//连接点集合
    V3DLONG count;
    double r;
public:
    Node();
	Node(V3DLONG i,V3DLONG j,V3DLONG k)
	{
		x=i;
		y=j;
		z=k;
	};
};

#endif // NODE_H


