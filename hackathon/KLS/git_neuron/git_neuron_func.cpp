#include "v3d_message.h"
#include <vector>
#include <map>
#include "git_neuron_func.h"
#include <iostream>
#include <unordered_map>
#include <time.h>
using namespace std;

class Node{
public:
	float x , y , z , p_x , p_y , p_z;
	Node(float a , float b , float c, float d , float e , float f){
		x = a;
		y = b;
		z = c;
		p_x = d;
		p_y = e;
		p_z = f;
	}
	bool operator==(const Node & n) const{
		return (x == n.x && y == n.y && z == n.z && p_x == n.p_x && p_y == n.p_y && p_z == n.p_z);
	}
};


struct hash_name{
	size_t operator()(const Node & n) const{
		return 	((((((((hash<float>()(n.x) ^ hash<float>()(n.y) << 1) >> 1) ^ hash<float>()(n.z) << 1) >> 1) ^ hash<float>()(n.p_x) << 1) >> 1) ^ hash<float>()(n.p_y) << 1) >> 1) ^ hash<float>()(n.p_z) << 1;
	}
};


/* New version of the comparison function, using a hash table to find intersection and union, reduce the time complexity from O(n^m) to O(n+m) .*/

void neuron_compare(NeuronTree* nt,NeuronTree* nt2, QString fileSaveName,QString fileSaveName2){
	cout<<"start compare"<<endl;
	clock_t startTime,endTime;
	startTime = clock();
	int size_1 = nt->listNeuron.size() , size_2 = nt2->listNeuron.size();
	unordered_map<int,int> hash_nt1,hash_nt2;
	unordered_map<Node , int , hash_name> nodes;
	vector<bool> visit_1(size_1,false),visit_2(size_2,false);
	cout<<"start compare1"<<endl;
	//检测交集
	for(int i = 0; i < size_1; i ++)
	{
		NeuronSWC node_1 = nt->listNeuron[i];
		hash_nt1[node_1.pn]++;
		
			int pt_1 = nt->hashNeuron.value(node_1.pn);
			NeuronSWC parent_1 = nt->listNeuron[pt_1];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nodes[Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z)]++;
		
	}


	cout<<"start compare2"<<endl;
	for(int i = 0; i < size_2; i ++)
	{
		NeuronSWC node_2 = nt2->listNeuron[i];
		hash_nt2[node_2.pn]++;
		
			int pt_2 = nt2->hashNeuron.value(node_2.pn);
			NeuronSWC parent_2 = nt2->listNeuron[pt_2];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nodes[Node(node_2.x , node_2.y , node_2.z , parent_2.x , parent_2.y , parent_2.z)]++;
		
	}
	cout<<"start compare3"<<endl;
	//对一号神经元改色
	for(int i = 0; i < size_1; i ++)
	{
		NeuronSWC node_1 = nt->listNeuron[i];
		
			int pt_1 = nt->hashNeuron.value(node_1.pn);
			NeuronSWC parent_1 = nt->listNeuron[pt_1];
			Node n(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			
			//if(nodes[n] != 2 || nodes2[n1] != 2) 
			if(nodes[n] != 2) 
			{
				visit_1[i] = true;
				nt->listNeuron[i].level = 6;
			}
	}
	cout<<"start compare4"<<endl;
	for(int i = 0; i < size_1; i ++)
	{
		NeuronSWC node_1 = nt->listNeuron[i];
		if(node_1.pn != -1)
		{
			int pt_1 = nt->hashNeuron.value(node_1.pn);
			NeuronSWC parent_1 = nt->listNeuron[pt_1];
			Node n(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			if(nodes[n] != 2 && hash_nt1[node_1.n] == 0) 
			{
				nt->listNeuron[i].level = 8;
				visit_1[i] = false;
				while(visit_1[pt_1])
				{
					nt->listNeuron[pt_1].level = 8;
					visit_1[pt_1] = false;
					pt_1 = nt->hashNeuron.value(nt->listNeuron[pt_1].pn);
				}
			}
		}
		
	}
	cout<<"start compare5"<<endl;
	//对二号神经元改色
	for(int i = 0; i < size_2; i ++)
	{
		NeuronSWC node_2 = nt2->listNeuron[i];
		
			int pt_2 = nt2->hashNeuron.value(node_2.pn);
			NeuronSWC parent_2 = nt2->listNeuron[pt_2];
			Node n(node_2.x , node_2.y , node_2.z , parent_2.x , parent_2.y , parent_2.z);
			
			//if(nodes[n] != 2 || nodes2[n1] != 2)
			if(nodes[n] != 2 ) 
			{
				visit_2[i] = true;
				nt2->listNeuron[i].level = 6;
			}
	}

	cout<<"start compare6"<<endl;
	for(int i = 0; i < size_2; i ++)
	{
		NeuronSWC node_2 = nt2->listNeuron[i];
		if(node_2.pn != -1)
		{
			int pt_2 = nt2->hashNeuron.value(node_2.pn);
			NeuronSWC parent_2 = nt2->listNeuron[pt_2];
			Node n(node_2.x , node_2.y , node_2.z , parent_2.x , parent_2.y , parent_2.z);
			if(nodes[n] != 2 && hash_nt2[node_2.n] == 0) 
			{
				nt2->listNeuron[i].level = 7;
				visit_2[i] = false;
				while(visit_2[pt_2])
				{
					nt2->listNeuron[pt_2].level = 7;
					visit_2[pt_2] = false;
					pt_2 = nt2->hashNeuron.value(nt2->listNeuron[pt_2].pn);
				}
				
			}
		}
		
	}
	cout<<"start compare7"<<endl;
	endTime = clock();
	cout << "Totle Time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	writeESWC_file(fileSaveName,*nt);
	writeESWC_file(fileSaveName2,*nt2);

}


void conflict_detection(NeuronTree* nt1,NeuronTree* nt2,NeuronTree* ancestor){
	cout<<"start compare !"<<endl;
	int st_1 = nt1->listNeuron.size(), st_2 = nt2->listNeuron.size(), st_a = ancestor->listNeuron.size();
	unordered_map<Node , int , hash_name> hash_nt1 , hash_nt2;
	
	/*发现neuron_1与祖先文件的交集*/
	for(int i = 0 ; i < st_1 ; i ++)
	{
		NeuronSWC node_1 = nt1->listNeuron[i];
		int pt_1 = nt1->hashNeuron.value(node_1.pn);
		if(pt_1 != -1)
		{
			NeuronSWC parent_1 = nt1->listNeuron[pt_1];
			hash_nt1[Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z)]++;
		}
	}

	for(int i = 0 ; i < st_a ; i ++)
	{
		NeuronSWC node_a = ancestor->listNeuron[i];
		int pt_a = ancestor->hashNeuron.value(node_a.pn);
		if(pt_a != -1)
		{
			NeuronSWC parent_a = ancestor->listNeuron[pt_a];
			hash_nt1[Node(node_a.x , node_a.y , node_a.z , parent_a.x , parent_a.y , parent_a.z)]++;
		}
	}

	/*发现neuron_2与祖先文件的交集*/
	for(int i = 0 ; i < st_2 ; i ++)
	{
		NeuronSWC node_1 = nt2->listNeuron[i];
		int pt_1 = nt2->hashNeuron.value(node_1.pn);
		if(pt_1 != -1)
		{
			NeuronSWC parent_1 = nt2->listNeuron[pt_1];
			hash_nt2[Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z)]++;
		}
	}

	for(int i = 0 ; i < st_a ; i ++)
	{
		NeuronSWC node_a = ancestor->listNeuron[i];
		int pt_a = ancestor->hashNeuron.value(node_a.pn);
		if(pt_a != -1)
		{
			NeuronSWC parent_a = ancestor->listNeuron[pt_a];
			hash_nt2[Node(node_a.x , node_a.y , node_a.z , parent_a.x , parent_a.y , parent_a.z)]++;
		}
	}
	

}

void neuron_merge_test(NeuronTree* nt,NeuronTree* nt2,NeuronTree* nt3, QString fileSaveName)
{
	cout<<"start test:"<<endl;
	clock_t startTime,endTime;
	startTime = clock();
	int size_1 = nt->listNeuron.size() , size_2 = nt2->listNeuron.size(),size_3 = nt3->listNeuron.size();
	unordered_map<int,int> hash_nt1,hash_nt2,hash_nt3;
	unordered_map<Node , int , hash_name> nodes ,nt1_nodes , nt2_nodes;
	vector<bool> visit_1(size_1,false),visit_2(size_2,false);


	//提取三个神经的公共部分
	for(int i = 0; i < size_1; i ++)
	{
		NeuronSWC node_1 = nt->listNeuron[i];
		nt->listNeuron[i].level = 1;
		hash_nt1[node_1.pn]++;
		int pt_1 = nt->hashNeuron.value(node_1.pn);	
		if(pt_1 != -1)
		{
			NeuronSWC parent_1 = nt->listNeuron[pt_1];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nodes[Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z)]++;
		}
	}

	for(int i = 0; i < size_2; i ++)
	{
		NeuronSWC node_2 = nt2->listNeuron[i];
		nt2->listNeuron[i].level = 1;
		hash_nt2[node_2.pn]++;
		int pt_2 = nt2->hashNeuron.value(node_2.pn);
		if(pt_2 != -1)
		{
			NeuronSWC parent_2 = nt2->listNeuron[pt_2];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nodes[Node(node_2.x , node_2.y , node_2.z , parent_2.x , parent_2.y , parent_2.z)]++;
		}
	}

	for(int i = 0; i < size_3; i ++)
	{
		NeuronSWC node_3 = nt3->listNeuron[i];
		nt3->listNeuron[i].level = 1;
		hash_nt3[node_3.pn]++;
		int pt_3 = nt3->hashNeuron.value(node_3.pn);
		if(pt_3 != -1)
		{
			NeuronSWC parent_3 = nt3->listNeuron[pt_3];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nodes[Node(node_3.x , node_3.y , node_3.z , parent_3.x , parent_3.y , parent_3.z)]++;
		}
	}

	//提取神经1和父神经的相异部分
	for(int i = 0; i < size_1; i ++)
	{
		NeuronSWC node_1 = nt->listNeuron[i];
		int pt_1 = nt->hashNeuron.value(node_1.pn);
		if(pt_1 != -1)
		{
			NeuronSWC parent_1 = nt->listNeuron[pt_1];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nt1_nodes[Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z)]++;
		}

	}

	for(int i = 0; i < size_3; i ++)
	{
		NeuronSWC node_3 = nt3->listNeuron[i];
		int pt_3 = nt3->hashNeuron.value(node_3.pn);
		if(pt_3 != -1)
		{
			NeuronSWC parent_3 = nt3->listNeuron[pt_3];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nt1_nodes[Node(node_3.x , node_3.y , node_3.z , parent_3.x , parent_3.y , parent_3.z)]++;
		}
	}

	//提取神经2和父神经的相异部分
	for(int i = 0; i < size_2; i ++)
	{
		NeuronSWC node_2 = nt2->listNeuron[i];
		int pt_2 = nt2->hashNeuron.value(node_2.pn);
		if(pt_2 != -1)
		{
			NeuronSWC parent_2 = nt2->listNeuron[pt_2];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nt2_nodes[Node(node_2.x , node_2.y , node_2.z , parent_2.x , parent_2.y , parent_2.z)]++;
		}
	}

	for(int i = 0; i < size_3; i ++)
	{
		NeuronSWC node_3 = nt3->listNeuron[i];
		int pt_3 = nt3->hashNeuron.value(node_3.pn);
		if(pt_3 != -1)
		{
			NeuronSWC parent_3 = nt3->listNeuron[pt_3];
			//Node* n = new Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			nt2_nodes[Node(node_3.x , node_3.y , node_3.z , parent_3.x , parent_3.y , parent_3.z)]++;
		}
	}
	//冲突检测：1将删减的和修改的部分赋予黄色
	for(int i = 0 ; i < size_1 ; i++)
	{
		NeuronSWC node = nt->listNeuron[i];
		if(node.pn = -1)
		{
			Node n(node.x , node.y , node.z , 0.0 , 0.0 , 0.0);
			if(nodes[n] == 1)
				nt->listNeuron[i].level = 6;
		}
		else
		{
			int pt = nt->hashNeuron.value(node.pn);
			NeuronSWC parent = nt->listNeuron[pt];
			Node n(node.x , node.y , node.z , parent.x , parent.y , parent.z);
			if(nodes[n] == 1)
				nt->listNeuron[i].level = 6;
		}
	}

	//删除的部分修改为红色
	for(int i = 0; i < size_1; i ++)
	{
		NeuronSWC node_1 = nt->listNeuron[i];
		if(node_1.pn != -1)
		{
			int pt_1 = nt->hashNeuron.value(node_1.pn);
			NeuronSWC parent_1 = nt->listNeuron[pt_1];
			if(node_1.level == 6 && hash_nt1[node_1.n] == 0) 
			{
				nt->listNeuron[i].level = 8;
				//visit_1[i] = false;
				while(nt->listNeuron[pt_1].level == 6)
				{
					nt->listNeuron[pt_1].level = 8;
					pt_1 = nt->hashNeuron.value(nt->listNeuron[pt_1].pn);
				}
			}
		}
		
	}
	//碰撞检测
	for(int i = 0 ; i < size_1 ; i ++)
	{
		NeuronSWC node = nt->listNeuron[i];
		if(node.pn != -1)
		{
			int pt = nt->hashNeuron.value(node.pn);
			NeuronSWC parent = nt->listNeuron[pt];
			if(node.level == 6)
			{
				cout<<"detect conflict"<<endl;
				return;
			}
		}
	}

	//合并
	for(int i = nt3->listNeuron.size()-1; i >= 0; i --)
	{
		NeuronSWC node_1 = nt3->listNeuron[i];
		int pt_1 = nt3->hashNeuron.value(node_1.pn);
		if(pt_1 != -1)
		{
			NeuronSWC parent_1 = nt3->listNeuron[pt_1];
			Node n(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z);
			if(nodes[n] != 3 ) 
			{
				nt3->listNeuron.removeAt(i);
			}
		}

	}
	
	int count_max = 0;

	for(int i = 0 ; i < nt3->listNeuron.size();i++)
	{
		NeuronSWC node = nt3->listNeuron[i];
		if(node.n > count_max) count_max = node.n;
	}

	cout<<count_max<<endl;
	cout<<nt3->listNeuron.size()<<endl;


	for(int i = 0 ; i < size_1 ; i ++)
	{
		NeuronSWC node_1 = nt->listNeuron[i];
		int pt_1 = nt->hashNeuron.value(node_1.pn);
		if(pt_1 != -1)
		{
			NeuronSWC parent_1 = nt->listNeuron[pt_1];
			int pt_p = nt->hashNeuron.value(parent_1.pn);
			NeuronSWC parent_p = nt->listNeuron[pt_p];		
			if(nt1_nodes[Node(node_1.x , node_1.y , node_1.z , parent_1.x , parent_1.y , parent_1.z)] == 1)
			{
					if(nt1_nodes[Node(parent_1.x , parent_1.y , parent_1.z ,parent_p.x , parent_p.y , parent_p.z )] == 2)
				{
					bool tag = false;
					for(int j = 0 ; j < nt3->listNeuron.size() ; j ++)
					{
						NeuronSWC node2 = nt3->listNeuron[j];
						if(fabs(parent_1.x - node2.x) < 0.001 
						&& fabs(parent_1.y - node2.y) < 0.001 && fabs(parent_1.z - node2.z) < 0.001){
							node_1.pn = node2.n;
							//node_1.pn = -1;
							node_1.n = node_1.n + count_max;
							node_1.type = 7;
							nt3->listNeuron.push_back(node_1);
							tag = true;
							break;
						}	
					}
					if(!tag)
					{
						node_1.pn = -1;
						node_1.n = node_1.n + count_max;
						node_1.type = 7;
						nt3->listNeuron.push_back(node_1);
					}	
				}
				else
				{
					node_1.pn = node_1.pn + count_max;
					node_1.n = node_1.n + count_max;
					node_1.type = 7;
					nt3->listNeuron.push_back(node_1);		
				}
			}
		}
	}

	for(int i = 0 ; i < nt3->listNeuron.size();i++)
	{
		NeuronSWC node = nt3->listNeuron[i];
		if(node.n > count_max) count_max = node.n;
	}

	for(int i = 0 ; i < size_2 ; i++)
	{
		NeuronSWC node_2 = nt2->listNeuron[i];
		int pt_2 = nt2->hashNeuron.value(node_2.pn);
		if(pt_2 != -1)
		{
			NeuronSWC parent_2 = nt2->listNeuron[pt_2];
			int pt_p = nt2->hashNeuron.value(parent_2.pn);
			NeuronSWC parent_p = nt2->listNeuron[pt_p];

			if(nt2_nodes[Node(node_2.x , node_2.y , node_2.z , parent_2.x , parent_2.y , parent_2.z)] == 1)
			{
					if(nt2_nodes[Node(parent_2.x , parent_2.y , parent_2.z ,parent_p.x , parent_p.y , parent_p.z )] == 2)
				{
					bool tag = false;
					for(int j = 0 ; j < nt3->listNeuron.size() ; j ++)
					{
						NeuronSWC node2 = nt3->listNeuron[j];
						if(fabs(parent_2.x - node2.x) < 0.001 
						&& fabs(parent_2.y - node2.y) < 0.001 && fabs(parent_2.z - node2.z) < 0.001){
							node_2.pn = node2.n;
							node_2.n = node_2.n + count_max;
							node_2.type = 6;
							nt3->listNeuron.push_back(node_2);
							tag = true;
							break;
						}
					}
					if(!tag)
					{
						node_2.pn = -1;
						node_2.n = node_2.n + count_max;
						node_2.type = 6;
						nt3->listNeuron.push_back(node_2);
					}
				}
				else
				{
					node_2.pn = node_2.pn + count_max;
					node_2.n = node_2.n + count_max;
					node_2.type = 6;
					nt3->listNeuron.push_back(node_2);	
					
				}
			}
		}
	}
	endTime = clock();
	cout << "Totle Time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	writeESWC_file(fileSaveName,*nt3);
}