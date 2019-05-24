/*
wirtten by heping
2019/3/26
*/
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "genVirture_plugin.h"
#include <vector>
#include<stdlib.h>
#include<iostream>
#include<queue>
using namespace std;
#define Max_in 500
priority_queue<edge> que;
vector<edge> G[Max_in];//任意两点之间的距离
vector<float> path_cost;//每个点的路径长度
float PATH_LENGTH;

//最小生成树算法
void min_tree_prim(QList <NeuronSWC> &new_listNeuron){//所有点坐标,将所有进行排序按照x,y,z坐标依次升序排列,分叉一个还是两个随机,什么时候终止
	//soma就是list中第一点
	V3DLONG size = new_listNeuron.size();
	cout <<"start distance" << endl;	
	float min_dist1, min_dist2;
	cout << size << endl;
	//初始化G
	for (int i = 0; i < size; i++){
		G[i].clear();
	}
	
	while (que.size())que.pop();
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			if (i == j)break;
			float cost = E_distance(new_listNeuron[i], new_listNeuron[j]);
			G[i].push_back(edge(i+1,j+1, cost));//起点终点是编号
			G[j].push_back(edge(j+1,i+1, cost));
			
		}
	}

	prim(new_listNeuron);
	cout << "end" << endl;
}

//prim_dendrite
void prim(QList <NeuronSWC> &new_listNeuron){
	V3DLONG size = new_listNeuron.size();
	vector<float> wring_cost=vector<float>(size,0.0);
	int count;
	//prim  -------------控制分叉个数-------------
	float sum_cost;
	vector<bool> vis = vector <bool>(size, false);
	vector<int> num = vector<int>(size, 0);
	//将顶点1作为初始点
	vis[0] = true;
	wring_cost[0] = 0;//soma点
	for (int i = 0; i < G[0].size(); i++){
		que.push(G[0][i]);
		cout << G[0][i].cost;
	}
	cout << endl;
	srand(que.size());
	int a = rand() % 4 + 4;
	count = 0;
	//从soma出发找到4-8个子节点相连,后面的prim还会增加一次
	while (que.size()){
		count++;
		if (count > a)break;
		else{
			edge e = que.top();
			que.pop();
			if (vis[e.to - 1])continue;
			vis[e.to - 1] = true;
			new_listNeuron[e.to - 1].parent = e.from;
			//wring_cost[e.to - 1] = wring_cost[e.from - 1] + e.cost;//每个点的,从根节点到该点的路径长度
			//PATH_LENGTH += wring_cost[e.to - 1];//树的路径长度
			cout << e.from << "----" << e.to << endl;
			sum_cost += e.cost;
		}
	}
	//将于soma相连访问过的节点放入V中
	for (int i = 1; i < size; i++){
		if (vis[i]){
			for (int j = 0; j < G[i].size(); j++)
				que.push(G[i][j]);
		}
	}
	//每次选取V与S-V集合中路径最短的边
	while (que.size()){
		edge e = que.top();
		que.pop();
		if (vis[e.to - 1])continue;//已经访问过的，e.to是下一个点的编号，它的位置=编号-1
		if (num[e.from - 1] < 2){//最多只有两个分支
			vis[e.to - 1] = true;
			new_listNeuron[e.to - 1].parent = e.from;
			sum_cost += e.cost;
			num[e.from - 1]++;//该节点的孩子个数
			//wring_cost[e.to - 1] = wring_cost[e.from - 1] + e.cost;
			//PATH_LENGTH += wring_cost[e.to - 1];
			cout << e.from << "----" << e.to << endl;
			cout << e.cost << endl;
			for (int j = 0; j < G[e.to - 1].size(); j++){
				que.push(G[e.to - 1][j]);
			}
		}

	}
	//遍历确定是否所有结点都访问过，图是否连通
	for (int i = 0; i < size; i++){
		cout << num[i] << " ";
		if (!vis[i])
			cout << i << endl;
	}
}

//一对节点之间的单位方向向量
NodeXYZ direction(NeuronSWC &Node1, NeuronSWC &Node2){
	NodeXYZ dire;
	float x1, y1, z1;
	float x2, y2, z2;
	float distance= E_distance(Node1, Node2);
	x1 = Node1.x;
	y1 = Node1.y;
	z1 = Node1.z;
	x2 = Node2.x;
	y2 = Node2.y;
	z2 = Node2.y;
	dire.x = (x2 - x1) / distance;
	dire.y = (y2 - y1) / distance;
	dire.z = (z2 - z1) / distance;
	return dire;

}
//一对节点之间的欧式距离
float E_distance(NeuronSWC &Node1, NeuronSWC &Node2){//两个点之间的欧式距离
	float dist;
	float x1, y1, z1;
	float x2, y2, z2;
	x1 = Node1.x;
	y1 = Node1.y;
	z1 = Node1.z;
	x2 = Node2.x;
	y2 = Node2.y;
	z2 = Node2.y;
	dist = sqrt(pow(x1 - x2, 2)+pow(y1 - y2, 2)+pow(z1 - z2, 2));
	
	return dist;

}

//将SWC文件中所有节点的孩子节点个数保存下来
void calculate_nodes(NeuronTree ntree, vector<V3DLONG> &nodes){
//	QMap <int, int>  mapDenrite;
//	QMap <int, int>  mapAxon;	
	V3DLONG neuron_size = ntree.listNeuron.size();
	//对应位置孩子的个数
	nodes = vector<V3DLONG>(neuron_size, 0);
	cout << neuron_size;
	//统计所有节点的孩子节点数量
	cout << "start calculate nodes number---" << endl;
//	int did=1, aid=1;
	//V3DLONG no = 0;
//	mapAxon.clear();
//	mapDenrite.clear();
	for (V3DLONG i = 1; i < neuron_size; i++){
		
		if (ntree.listNeuron[i].type == 2 || ntree.listNeuron[i].type == 3){//轴突，对于不同的结构生成算法是否需要不同--------
			V3DLONG nid = ntree.hashNeuron.value(ntree.listNeuron[i].parent);//将在list中的该节点的父节点在list中的位置id保存下来
			nodes[nid]++;			

		}
		else continue;
		
	}
//	cout << "axon nodes num"<<nodes_axon.size();
//	cout <<"denrite nodes num"<< nodes_denr.size();
	cout << "finish it!" << endl;
	
}

//保存关键节点
void save_import_nodes(NeuronTree &ntree, vector<V3DLONG> &nodes, QList <NeuronSWC> &new_listNeuron_denr, QList <NeuronSWC> &new_listNeuron_axon){
	
	V3DLONG size = nodes.size();
	int noa = 0;
	int nod = 0;
	for (V3DLONG i = 0; i < size; i++){
		cout << nodes[i] << "-";
		if (nodes[i] != 1){//终端节点，保留
			if (ntree.listNeuron.at(i).type == 2){//轴突,无soma
				V3DLONG pn = ntree.listNeuron.at(i).parent;
				while (nodes[pn-1] == 1){
					pn = ntree.listNeuron.at(pn-1).parent;
				}
				
				new_listNeuron_axon.append(ntree.listNeuron.at(i));
				//new_listNeuron_axon[noa].n = noa + 1;//节点重新编号,编号从1开始	
				new_listNeuron_axon[noa].parent = pn;
				
				//new_listNeuron_axon[noa].parent = -1;//全部初始化无父节点
				cout << new_listNeuron_axon[noa].n << "---";
				noa++;
			}
			else{//树突,包含soma
				V3DLONG pn = ntree.listNeuron.at(i).parent;
				while (nodes[pn - 1] == 1){
					pn = ntree.listNeuron.at(pn - 1).parent;
					if (pn == 1)break;
				}
				new_listNeuron_denr.append(ntree.listNeuron.at(i));
				//new_listNeuron_denr[nod].n = nod + 1;//节点重新编号，编号从1开始
				//new_listNeuron_denr[nod].parent = -1;//全部初始化无父节点
				new_listNeuron_denr[nod].parent = pn;
				cout << new_listNeuron_denr[nod].n << "---";
				nod++;
			}			
			
		}		
		else {//分叉点，无论它分叉成几个子节点，暂时不删除大于两个分支的
			continue;
		}
		cout << endl;
	}
	

}

void save_point_nodes(NeuronTree &ntree, vector<V3DLONG> &nodes, QList <NeuronSWC> &new_listNeuron){
	V3DLONG size = nodes.size();
	int no = 0;
	for (V3DLONG i = 0; i < size; i++){
		if (nodes[i] != 1){//终端节点，保留
			V3DLONG pn = ntree.listNeuron.at(i).parent;
			while (nodes[pn - 1] == 1)
				pn = ntree.listNeuron.at(pn - 1).parent;
			new_listNeuron.append(ntree.listNeuron.at(i));
			new_listNeuron[no].parent = pn;
			no++;

		}
		else continue;
	}
}

NeuronTree ntree1;//新的树
//路径成本,从根节点到该点的路径长度
float cost_path(NeuronSWC Node){
	NeuronSWC node = Node;
	V3DLONG pn = node.parent;
	//路径计算
	if (pn = -1)
		return 0;
	else{
		NeuronSWC parent = ntree1.listNeuron.at(pn);
		return cost_path(parent) + E_distance(parent, node);
	}
}