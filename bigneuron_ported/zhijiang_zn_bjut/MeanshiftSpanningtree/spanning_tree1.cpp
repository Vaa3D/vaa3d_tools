
#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "meanshift_func.h"
#include "node.h"
#include "D:\Vaa3D\v3d_main\neurontracing_vn21\app2/heap.h"
#include "D:\Vaa3D\v3d_main\neuron_editing\v_neuronswc.h"
#include "D:\Vaa3D/sort_neuron_swc/sort_swc.h"
#include "D:\Vaa3D\v3d_main\basic_c_fun\basic_surf_objs.h"
#include "D:\Vaa3D/neurontracing_vn21/app2/my_surf_objs.h"
#include <vector>
#include <iostream>
#include "stackutil.h"

 void construct_tree(QList<Node*> &seeds)
{
	
	//for(QMap<int,QList<Node*>>::iterator iter=finalclass_node.begin();iter!=finalclass_node.end();iter++)
	{
		//if(iter==finalclass_node.begin())
		{
			QList<Node*> seeds=iter.value();
			V3DLONG marknum = seeds.size();

			double** markEdge = new double*[marknum];
			for(int i = 0; i < marknum; i++)
			{
				markEdge[i] = new double[marknum];
				//fprintf(debug_fp,"markEdge[i]:%lf\n",markEdge[i]);
			}

			double x1,y1,z1;
			for (int i=0;i<marknum;i++)
			{
				x1 = seeds.at(i)->x;
				y1 = seeds.at(i)->y;
				z1 = seeds.at(i)->z;
				for (int j=0;j<marknum;j++)
				{
					markEdge[i][j] = sqrt(double(x1-seeds.at(j)->x)*double(x1-seeds.at(j)->x) + double(y1-seeds.at(j)->y)*double(y1-seeds.at(j)->y) + double(z1-seeds.at(j)->z)*double(z1-seeds.at(j)->z));
					//fprintf(debug_fp,"markEdge[i][j]:%lf\n",markEdge[i][j]);
				}
			}//这个是计算种子点之间的距离，应该是在构造边，即计算边的长度

			//NeutronTree structure
			NeuronTree marker_MST;
			QList <NeuronSWC> listNeuron;//NeuronSWC应该是节点的意思，多个节点构成一棵树
			QHash <int, int>  hashNeuron;
			listNeuron.clear();
			hashNeuron.clear();

			//set node

			NeuronSWC S;
			S.n 	= 1;
			S.type 	= 3;
			S.x 	= seeds.at(0)->x;
			S.y 	= seeds.at(0)->y;
			S.z 	= seeds.at(0)->z;
			S.r 	= 1;
			S.pn 	= -1;
			listNeuron.append(S);
			hashNeuron.insert(S.n, listNeuron.size()-1);

			int* pi = new int[marknum];
			for(int i = 0; i< marknum;i++)
				pi[i] = 0;
			pi[0] = 1;
			int indexi,indexj;
			for(int loop = 0; loop<marknum;loop++)//这个循环貌似在找最短路径，先从第1个点开始，找离他最近的一个点，然后从这个点开始，找最近点，以此类推。这个应该是最小生成树的实现代码
			{
				double min = INF;
				for(int i = 0; i<marknum; i++)
				{
					if (pi[i] == 1)
					{
						for(int j = 0;j<marknum; j++)
						{
							if(pi[j] == 0 && min > markEdge[i][j])
							{
								min = markEdge[i][j];
								indexi = i;
								indexj = j;//这两个值应该表示距离最短的额两个点
							}
						}
					}

				}
				if(indexi>=0)//在这个if语句中就形成了一棵树，节点的父节点是离他距离最近的节点，也是在他之前计算的一个节点
				{
					S.n 	= indexj+1;
					S.type 	= 7;
					S.x 	= seeds.at(indexj)->x;
					S.y 	= seeds.at(indexj)->y;
					S.z 	= seeds.at(indexj)->z;
					S.r 	= 1;
					S.pn 	= indexi+1;
					listNeuron.append(S);
					hashNeuron.insert(S.n, listNeuron.size()-1);

				}else
				{
					break;
				}
				pi[indexj] = 1;
				indexi = -1;
				indexj = -1;
			}
			marker_MST.n = -1;
			marker_MST.on = true;
			marker_MST.listNeuron = listNeuron;
			marker_MST.hashNeuron = hashNeuron;

			if(markEdge) {delete []markEdge, markEdge = 0;}
			//writeSWC_file("mst.swc",marker_MST);
			QList<NeuronSWC> marker_MST_sorted;
			if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))//这个排序应该是不会影响父节点与子节点之间的关系，应该只是将这些节点按照一定的顺序写到SWC文件中
			//return marker_MST_sorted;


		}

	}

}