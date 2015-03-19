/* example_func.h
 * This is an example plugin, the header file of plugin functions.
 * 2012-02-10 : by Yinan Wan
 */
 
#ifndef __MY_FUNC_H__
#define __MY_FUNC_H__

#include <v3d_interface.h>
#include "node.h"
#include "tree.h"
#include "graph.h"


bool proc(V3DPluginCallback2 &callback,QWidget* parent,unsigned char* data1d,V3DLONG *in_sz,QString imagename);//窗口模式下的插件入口
template<class T>
void mst(QMap<V3DLONG,Graph<T>*> map,QList<Tree<T>*> &rootList,QMap<V3DLONG,Tree<T>*> &treeMap);//最小生成树
void findNode(unsigned char* img1d, QMap<V3DLONG,Graph<Node*>*> &nodeMap);//获取前景色点
void initPath(QMap<V3DLONG,Graph<Node*>*> &nodeMap);//初始化图结构
void calculateRadius_old(QMap<V3DLONG,Graph<Node*>*> &nodeMap);//抛弃使用的计算半径方法
void calculateRadius(QMap<V3DLONG,Graph<Node*>*> &nodeMap);//计算半径
void prundNodeByRadius(QMap<V3DLONG,Graph<Node*>*> &nodeMap);//根据半径合并点
void calculateWeight(QMap<V3DLONG,Graph<Node*>*> &nodeMap);//计算边权重
void calculateTreeWeight(QList<Tree<Node*>*> rootList
                         , QMap<V3DLONG,Graph<Tree<Node*>*>*> &rootMap);
void reCreate(QMap<V3DLONG,Graph<Tree<Node*>*>*> rootMap
                , QMap<V3DLONG, Tree<Tree<Node *> *> *> treeRootMap
                , QMap<V3DLONG,Tree<Node*>*> &treeMap);
void reCreateTree(Tree<Node*>* curNode,QMap<V3DLONG,Tree<Node*>*> &treeMap);
void myplugin_proc(unsigned char* img1d);//算法入口
#endif
