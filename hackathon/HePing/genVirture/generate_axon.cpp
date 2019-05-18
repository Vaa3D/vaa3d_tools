/*
wirtten by heping
2019/3/30
*/
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "genVirture_plugin.h"
#include<iostream>
#include<random>
#include<vector>
#include<queue>
#include<algorithm>

using namespace std;
#define Max_in 50000 
vector<edge> G1[Max_in];
vector<int> order;
priority_queue<edge> que1;
vector<edge> edges;
vector<int> num1;
default_random_engine eg;
//normal_distribution <float> distr();
uniform_real_distribution<double> unf(0.0, 1.0);

//初始化，将所有节点分散并且将它们之间的距离保存成矩阵
void init(QList <NeuronSWC> &listNeuron){
	

}

void random_distr(){
	
}

void gen_axon(QList <NeuronSWC> &listNeuron){
	V3DLONG size = listNeuron.size();
	vector<bool> vis = vector<bool>(size, false);
	//init(listNeuron);
	vis[0] = true;
	num1 = vector<int>(size, 0);
	order = vector<int>(size, 0);
	cout << "generate axon!!!" << endl;
	cout << size << "------------"<<endl;
	QList <NeuronSWC> new_listNeuron;
	V3DLONG id=1;
	for (V3DLONG i = 0; i < size; i++){
		if (listNeuron[i].type == 3){//保留树突
			listNeuron[i].n = id;
		//	listNeuron[i].parent = -1;
			new_listNeuron.append(listNeuron[i]);
			id++;
		}
	}
	listNeuron = new_listNeuron;
	size = listNeuron.size();
	cout << size << endl;
	//初始化将所有点变成随机点
	for (V3DLONG i = 0; i < size; i++){
		listNeuron[i].parent = -1;
	}
	for (int i = 0; i < size; i++){
		G1[i].clear();
	}
	//计算两点之间的距离，形成矩阵
	while (que1.size())que1.pop();
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			if (i == j)break;
			float cost = E_distance(listNeuron[i], listNeuron[j]);
			G1[i].push_back(edge(i + 1, j + 1, cost));//起点终点是编号
			G1[j].push_back(edge(j + 1, i + 1, cost));

		}
	}
	//第一个点作为soma
	for (int i = 0; i < G1[0].size(); i++){
		que1.push(G1[0][i]);
		//cout << G1[0][i].cost;
	}
	cout << G1[0].size();
	cout << "*****************" << endl;
	//下面有问题，闪退
	//随机从soma分叉出5-10个分枝
	srand(1);
	//cout << rand();
	//int r = 0;
	int r = rand() % 6+5;
	cout << r << endl;
	for (int j = 0; j < r; j++){
		if (que1.size()){
			edge e = que1.top();
			que1.pop();
			listNeuron[e.to - 1].parent = e.from;//将点连到soma上
			cout << e.from << "-->" << e.to << endl;
			order[e.to - 1] = order[e.from - 1] + 1;//离心顺序
			vis[e.to - 1] = true;
		}
	}
	cout << "111111111111" << endl;
	//将于soma相连访问过的节点放入V中
	for (int i = 1; i < size; i++){
		if (vis[i]){
			for (int j = 0; j < G1[i].size(); j++)
				que1.push(G1[i][j]);
		}
	}
	cout << "generateing-----------" << endl;
	//
	while (que1.size()){
		edge e = que1.top();
		edge e1;
		bool flag = false;
		que1.pop();
		if (vis[e.to - 1])continue;
		if (num1[e.from - 1] != 0){//已经产生过分叉或者伸长了
			continue;
		}
		srand(order[e.from-1]);
		//float r = unf(e);
		float r = rand() % 100 / 100.0;
		cout << r << endl;
		//int r = un(order[e.from - 1]);
		//分叉
		if (r > 0.6&&r<0.9){
			listNeuron[e.to - 1].parent = e.from;
			vis[e.to - 1] = true;
			order[e.to - 1] = order[e.from - 1] + 1;
//			cout << e.from << "-->" << e.to << endl;
			num1[e.from - 1] = 2;
			while (que1.size()){//
				 e1 = que1.top();
				 que1.pop();
				// cout << e1.from<<" ";
				if (e1.from == e.from&&!vis[e1.to-1]){
					listNeuron[e1.to - 1].parent = e.from;
					order[e1.to - 1] = order[e1.from - 1] + 1;					
					vis[e1.to - 1] = true;
//					cout << e1.from << "-->>" << e1.to << endl;
					flag = true;
					break;
				}
				else{
					edges.push_back(e1);
				}
			}		
			for (int i = 0; i < edges.size(); i++)
				que1.push(edges[i]);
		}	
		else if(r>=0.9){//停止生长
			vis[e.to - 1] = true;
			listNeuron[e.to - 1].parent = e.from;
			order[e.to - 1] = order[e.from - 1] + 1;
			num1[e.from - 1] = 1;
			break;
		}
		//继续生长
		else{
			listNeuron[e.to - 1].parent = e.from;
//			cout << e.from << "-->" << e.to << endl;
			vis[e.to - 1] = true;
			order[e.to - 1] = order[e.from - 1] + 1;
			num1[e.from - 1] = 1;//伸长点
		}
		//将新加入的点相连的边加入候选队列,为什么存在一个点出现多次
		cout << num1[e.from - 1] << endl;
		
		for (int i = 0; i < G1[e.to - 1].size(); i++){
			que1.push(G1[e.to - 1][i]);
		}
		//分叉点加入候选队列
		if (flag){
			for (int j = 0; j < G1[e1.to - 1].size(); j++){
				que1.push(G1[e1.to - 1][j]);
			}

		}
		
	}
	//确定所有点是否都访问过
	for (V3DLONG i = 0; i < size; i++){
		if (vis[i])continue;
		else{
			cout << "error----??" << endl;
			break;
		}
	}

}

void branch(){

}