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
vector<edge> G[Max_in];
//最小生成树算法
void min_tree_prim(QList <NeuronSWC> &new_listNeuron){//所有点坐标,将所有进行排序按照x,y,z坐标依次升序排列,分叉一个还是两个随机,什么时候终止
	//soma就是list中第一点
	V3DLONG size = new_listNeuron.size();
	cout <<"start distance" << endl;	
	float min_dist1, min_dist2;
//	vector <float> dista;	
//	int flag1,flag2=0;
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

	//分开树突和轴突的部分，轴突只有一根主干到cluster，随机选择最近的点作为轴突与树突连接的点
//	for (V3DLONG i = 0; i < size; i++){//----------
//		cout << "start distance" << endl;
//		for (int j = i; j <size; j++){
//			cout << (2 * size - i + 1)*i / 2 + j - i << "!!";
//			if (i == j){
//				dista.push_back(0);//只能用push_back在VECTOR末尾添加数值，不能直接用没有分配空间的地址引用[]来赋值
				
//			}				
//			else{
//				dista.push_back(E_distance(new_listNeuron[i], new_listNeuron[j]));				
//			}				
//		}
//	}


	
	//选择最近的一个点或者两个点，若距离一直增加，循环100次结束
	
//	vector<V3DLONG> d;
//	d.push_back(0);
//	for (V3DLONG k = 0; k < size-1; k++){
		
//		cout << "start recurrent----"<<k << endl;
//		min_dist1 = dista[(2*size-k+1)*k/2 + 2];//取最近的连两个点初始化
//		min_dist2 = dista[(2 * size - k + 1)*k / 2 + 3];
//		cout << min_dist1 << "--" << min_dist2 << endl;
//		V3DLONG j;
//		flag1 = k+1;
//		//flag2 = k+2;
//		vis[flag1] = true;
		//vis[flag2] = true;
//		count = 0;
//		for (j = k+1; j < size; j++){
//			if (count > 50) break;
// 	 	    if (dista[(2 * size - k + 1)*k / 2 + j - k] < min_dist1&&vis[j]==false){
//				min_dist1 = dista[(2 * size - k + 1)*k / 2 + j - k];
//				cout << min_dist1<<"--";
//				vis[flag1] = false;
//				flag1 = j;				
//				vis[j] = true;
//			}								
//			else { 
//				count++;
//				continue; }
//		}
		
		//srand(k);
		//float ra=rand() % 100 / double(100);//产生0-1范围内的随机数
		//if (ra > 0.6){//大于一定阈值分叉，现在不需要----目前只包含分叉点
//
		//}
//		cout << endl;
///		cout << flag1 << "#" << endl;
//		new_listNeuron[flag1].parent = k+1;
	//	new_listNeuron[flag2].parent = k + 1;
//	}
	
	cout << "end" << endl;
}

//prim_dendrite
void prim(QList <NeuronSWC> &new_listNeuron){
	V3DLONG size = new_listNeuron.size();
	int count;
	//prim  -------------控制分叉个数-------------
	float sum_cost;
	vector<bool> vis = vector <bool>(size, false);
	vector<int> num = vector<int>(size, 0);
	//将顶点1作为初始点
	vis[0] = true;
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

//将SWC文件中所有节点的孩子节点个数保存下来,并将轴突和树突分开
void calculate_nodes(NeuronTree ntree, vector<V3DLONG> &nodes){
//	QMap <int, int>  mapDenrite;
//	QMap <int, int>  mapAxon;	
	V3DLONG neuron_size = ntree.listNeuron.size();
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
		//	if (mapAxon.value(nid)==0){
		//		mapAxon.insert(nid, aid);				
				
	//		}
	//		else{
	//			aid = mapAxon.value(nid);
	//		}
			//保存nodes_axon下标与listNeuron对应的节点下标
	//		nodes_axon[aid]++;
	//		cout << "parent:"<<nid <<"aid:"<< aid <<"num:"<< nodes_axon[aid] << endl;
	//		aid++;			

		}
		
	//		if (mapDenrite.value(nid) == 0){
	//			mapDenrite.insert(nid, did);
	//			
	//		}
	//		else{
	//			did = mapDenrite.value(nid);
	//		}
			//保存nodes_axon下标与listNeuron对应的节点下标
	//		nodes_denr[did]++;
	//		cout << "parent:"<<nid <<"did:"<< did <<"num:"<< nodes_denr[did] << endl;
	//		did++;
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
			if (ntree.listNeuron.at(i).type == 2){//轴突
				new_listNeuron_axon.append(ntree.listNeuron.at(i));
				new_listNeuron_axon[noa].n = noa + 1;//节点重新编号,编号从1开始
				new_listNeuron_axon[noa].parent = -1;//全部初始化无父节点
				cout << new_listNeuron_axon[noa].n << "---";
				noa++;
			}
			else{//树突
				new_listNeuron_denr.append(ntree.listNeuron.at(i));
				new_listNeuron_denr[nod].n = nod + 1;//节点重新编号，编号从1开始
				new_listNeuron_denr[nod].parent = -1;//全部初始化无父节点
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


//路径成本
void cost_path(){

}