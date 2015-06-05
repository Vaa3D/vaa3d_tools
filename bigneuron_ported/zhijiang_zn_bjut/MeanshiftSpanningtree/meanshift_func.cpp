/* example_func.cpp
* This file contains the functions used in plugin domenu and dufunc, you can use it as a demo.
* 2012-02-13 : by Yinan Wan
*/

#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "meanshift_func.h"
#include "smooth_curve.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/heap.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/v_neuronswc.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

#include <vector>
#include <iostream> 

using namespace std;

/* function used in DOMENU typically takes 2 inputs:
* "callback" - provide information from the plugin interface, and 
* "parent"   - the parent widget of the Vaa3D main window
*/


#define GET_IND(x,y,z) (x) + (y) * sz_x + (z) * sz_x* sz_y
#define GET_X(ind) ind % sz_x
#define GET_Y(ind) (ind / sz_x) % sz_y
#define GET_Z(ind) (ind / (sz_x*sz_y)) % sz_z
#define ABS(a) (a) > 0 ? (a) : -(a)
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

static struct adj_node{
	long i,j,k; double dist;
}node_table[26]={
	{-1,1,1,sqrt(3.0)},
	{0,1,1,sqrt(2.0)},
	{1,1,1,sqrt(3.0)},
	{-1,0,1,sqrt(2.0)},
	{0,0,1,1},
	{1,0,1,sqrt(2.0)},
	{-1,-1,1,sqrt(3.0)},
	{0,-1,1,sqrt(2.0)},
	{1,-1,1,sqrt(3.0)},

	{-1,1,0,sqrt(2.0)},
	{0,1,0,1},
	{1,1,0,sqrt(2.0)},
	{-1,0,0,1},
	{1,0,0,1},
	{-1,-1,0,sqrt(2.0)},
	{0,-1,0,1},
	{1,-1,0,sqrt(2.0)},

	{-1,1,-1,sqrt(3.0)},
	{0,1,-1,sqrt(2.0)},
	{1,1,-1,sqrt(3.0)},
	{-1,0,-1,sqrt(2.0)},
	{0,0,-1,1},
	{1,0,-1,sqrt(2.)},
	{-1,-1,-1,sqrt(3.0)},
	{0,-1,-1,sqrt(2.0)},
	{1,-1,-1,sqrt(3.0)}
};

int *flag;

QMap<int,QList<Node*> > Map_finalnode_list;
QMap<int,Node*> Map_rootnode;
QMap<int,Node> Map_rootnode_n;
QMultiMap<int,Node*> Map_allnodes;
QMap<int,QList<Node*> > finalclass_node;
QList<QList<NeuronSWC> > result_tree_part;
QList<NeuronSWC> result_final_tree;
QMap<int,Node*> root;
QMap<int,QMap<int,QList<Node*>>> number_path;
QMap<int,QMap<int,QList<Node>>> number_path_vn2;
QMultiMap<V3DLONG,QMap<V3DLONG,QList<Node>>> number_path_vn3;
QList<NeuronSWC> final_listNeuron;
QMap<V3DLONG,int> cp_SWC;

QMap<int,QList<Node*>> unit_path;
QMap<int,QList<Node>> unit_path_vn2;//keep the shortest path of all nodes

QList<QList<Node*>> final_path;
QList<QList<Node>> final_path_vn2;

QMap<V3DLONG, NeuronSWC> original_tree;//keep the original tree and insert the shortest path into each two nodes

QMap<V3DLONG,int> Map_finalnode; 
QMap<V3DLONG,int> Map_nodes;
QMap<V3DLONG,V3DLONG> Map_allnode;
QList <NeuronSWC> result_list;
QList <NeuronSWC> con_tree;
vector<QList<Node*> > v_List;
QMap<V3DLONG,QList<Node*> > root_class;

Node getnode(Node *node)
{

	Node result;
	result.x=node->x;
	result.y=node->y;
	result.z=node->z;
	result.r=node->r;
	result.class_mark=node->class_mark;
	result.number=node->number;
	result.parent=node->parent;
	return result;


}

bool contain(QList<Node> *queue,V3DLONG x,V3DLONG y,V3DLONG z)
{
	bool flag=false;
	for(int i=0;i<queue->size();i++)
	{
		//Node* temp=new Node();
		if((queue->at(i).x==x)&&(queue->at(i).y==y)&&(queue->at(i).z==z))
		{
			flag=true;
			break;

		}
	} 

	return flag;

}
void enlarge_radiusof_single_node_xy(unsigned char * &img1d,Node * &node,V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z)
{

	QList<Node> *list_node=new QList<Node>();
	int allnodes=0;
	int back_nodes=0;
	int max=0;
	double threshold=30;

	list_node->append(getnode(node));


	while(1)
	{

		//Node* head=queue->head();
		//Node* head=queue->first();
		//V3DLONG head=list.begin();

		Node head=list_node->first();
		Node* up_queue=new Node(head.x,head.y-1,head.z);
		Node* down_queue=new Node(head.x,head.y+1,head.z);
		Node* left_queue=new Node(head.x-1,head.y,head.z);
		Node* right_queue=new Node(head.x+1,head.y,head.z);
		/*Node *up_queue=up;
		Node *down_queue=down;
		Node *left_queue=left;
		Node *right_queue=right;*/

		//if(!queue->contains(up_queue))
		if(!contain(list_node,up_queue->x,up_queue->y,up_queue->z)) 
			//if(!queue->contains(GET_IND(up->x,up->y,up->z)))
		{
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(up->x,up->y,up->z)]);
			//printf("%ld   %ld   %ld\n",up->x,up->y,up->z);
			if(img1d[GET_IND(up_queue->x,up_queue->y,up_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(up->x,up->y,up->z));
			//queue->enqueue(up_queue);
			//queue->append(up_queue);
			list_node->append(getnode(up_queue));
			//delete up;

		}

		//if(!queue->contains(down_queue))
		if(!contain(list_node,down_queue->x,down_queue->y,down_queue->z))
			//if(!queue->contains(GET_IND(down->x,down->y,down->z)))
		{
			//printf("no down\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(down->x,down->y,down->z)]);
			//printf("%ld   %ld   %ld\n",down->x,down->y,down->z);
			if(img1d[GET_IND(down_queue->x,down_queue->y,down_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(down->x,down->y,down->z));
			//queue->enqueue(down_queue);
			//queue->append(down_queue);
			list_node->append(getnode(down_queue));
			//delete down;

		}

		//if(!queue->contains(left_queue))
		//if(!queue->contains(GET_IND(left->x,left->y,left->z)))
		if(!contain(list_node,left_queue->x,left_queue->y,left_queue->z))
		{
			//printf("no left\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(left->x,left->y,left->z)]);
			//printf("%ld   %ld   %ld\n",left->x,left->y,left->z);
			if(img1d[GET_IND(left_queue->x,left_queue->y,left_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(left->x,left->y,left->z));
			//queue->enqueue(left_queue);
			//queue->append(left_queue);
			list_node->append(getnode(left_queue));
			//delete left;

		}

		//if(!queue->contains(right_queue))
		if(!contain(list_node,right_queue->x,right_queue->y,right_queue->z))
			//if(!queue->contains(GET_IND(right->x,right->y,right->z)))
		{
			//printf("no right\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(right->x,right->y,right->z)]);
			//printf("%ld   %ld   %ld\n",right->x,right->y,right->z);
			if(img1d[GET_IND(right_queue->x,right_queue->y,right_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(right->x,right->y,right->z));
			//queue->enqueue(right_queue);
			//queue->append(right_queue);
			list_node->append(getnode(right_queue));
			//queue->
			//delete right;

		}
		//printf("size:::%d   \n",queue.size());
		//printf("%d   %d\n",allnodes,back_nodes);

		double up_r=(double)sqrt((double)(node->x-up_queue->x)*(node->x-up_queue->x)+(double)(node->y-up_queue->y)*(node->y-up_queue->y)+(double)(node->z-up_queue->z)*(node->z-up_queue->z));
		double down_r=(double)sqrt((double)(node->x-down_queue->x)*(node->x-down_queue->x)+(double)(node->y-down_queue->y)*(node->y-down_queue->y)+(double)(node->z-down_queue->z)*(node->z-down_queue->z));
		double left_r=(double)sqrt((double)(node->x-left_queue->x)*(node->x-left_queue->x)+(double)(node->y-left_queue->y)*(node->y-left_queue->y)+(double)(node->z-left_queue->z)*(node->z-left_queue->z));
		double right_r=(double)sqrt((double)(node->x-right_queue->x)*(node->x-right_queue->x)+(double)(node->y-right_queue->y)*(node->y-right_queue->y)+(double)(node->z-right_queue->z)*(node->z-right_queue->z));
		float per=(float)back_nodes/(float)allnodes;


		if(per>0.05)
		{

			node->r=(up_r+down_r+left_r+right_r)/4;	
			//printf(" radius::%lf\n",node->r);
			delete up_queue;
			delete down_queue;
			delete left_queue;
			delete right_queue;


			break;
		}else{
			//ｳﾓ
			//queue->pop_front();
			list_node->pop_front();
			//list.detach();
			//list.pop_front();
			delete up_queue;
			delete down_queue;
			delete left_queue;
			delete right_queue;
		}
	}
	//queue->clear();
	//qDeleteAll(list_node);
	list_node->clear();
	delete list_node;
	/*for(int i=0;i<queue->size();i++)
	{
	Node *temp=queue->at(i);

	//delete (temp);
	//printf("%ld\n",queue->at(i)->x);
	//printf("temp::%ld\n",temp->x);


	}*/

}


double enlarge_radiusof_single_node_xy_vn2(unsigned char * &img1d,Node node,V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z)
{

	QList<Node> *list_node=new QList<Node>();
	int allnodes=0;
	int back_nodes=0;
	int max=0;
	double threshold=30;

	list_node->append(node);


	while(1)
	{

		Node head=list_node->first();
		Node* up_queue=new Node(head.x,head.y-1,head.z);
		Node* down_queue=new Node(head.x,head.y+1,head.z);
		Node* left_queue=new Node(head.x-1,head.y,head.z);
		Node* right_queue=new Node(head.x+1,head.y,head.z);
		if(!contain(list_node,up_queue->x,up_queue->y,up_queue->z)) 
		{
			allnodes=allnodes+1;
			if(img1d[GET_IND(up_queue->x,up_queue->y,up_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
			}
			list_node->append(getnode(up_queue));

		}


		if(!contain(list_node,down_queue->x,down_queue->y,down_queue->z))
		{

			allnodes=allnodes+1;

			if(img1d[GET_IND(down_queue->x,down_queue->y,down_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
			}
			list_node->append(getnode(down_queue));

		}

		if(!contain(list_node,left_queue->x,left_queue->y,left_queue->z))
		{
			allnodes=allnodes+1;
			if(img1d[GET_IND(left_queue->x,left_queue->y,left_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
			}
			list_node->append(getnode(left_queue));

		}

		if(!contain(list_node,right_queue->x,right_queue->y,right_queue->z))
		{

			allnodes=allnodes+1;
			if(img1d[GET_IND(right_queue->x,right_queue->y,right_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
			}
			list_node->append(getnode(right_queue));

		}

		double up_r=(double)sqrt((double)(node.x-up_queue->x)*(node.x-up_queue->x)+(double)(node.y-up_queue->y)*(node.y-up_queue->y)+(double)(node.z-up_queue->z)*(node.z-up_queue->z));
		double down_r=(double)sqrt((double)(node.x-down_queue->x)*(node.x-down_queue->x)+(double)(node.y-down_queue->y)*(node.y-down_queue->y)+(double)(node.z-down_queue->z)*(node.z-down_queue->z));
		double left_r=(double)sqrt((double)(node.x-left_queue->x)*(node.x-left_queue->x)+(double)(node.y-left_queue->y)*(node.y-left_queue->y)+(double)(node.z-left_queue->z)*(node.z-left_queue->z));
		double right_r=(double)sqrt((double)(node.x-right_queue->x)*(node.x-right_queue->x)+(double)(node.y-right_queue->y)*(node.y-right_queue->y)+(double)(node.z-right_queue->z)*(node.z-right_queue->z));
		float per=(float)back_nodes/(float)allnodes;


		if(per>0.05)
		{

			node.r=(up_r+down_r+left_r+right_r)/4;	
			delete up_queue;
			delete down_queue;
			delete left_queue;
			delete right_queue;


			break;
		}else{
			list_node->pop_front();
			delete up_queue;
			delete down_queue;
			delete left_queue;
			delete right_queue;
		}
	}
	list_node->clear();
	delete list_node;
	return node.r;

}


void writeSWC_file(char* path,QList<NeuronSWC> listNeuron)
{
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for(int i=0;i<listNeuron.size();i++)
	{
		NeuronSWC temp=listNeuron.at(i);
		fprintf(fp, "%ld %d %f %f %f %f %ld\n",
			temp.n, temp.type,  temp.x,  temp.y,  temp.z, temp.r, temp.parent);

	}
	fclose(fp);
	return;

}
void printSWCByQMap_QMap(char* path,QMap<int,QMap<int,QList<Node*>>> nodeMap)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "wt");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");


	for(QMap<int,QMap<int,QList<Node*>>>::iterator iter1 = nodeMap.begin(); iter1 != nodeMap.end(); iter1++)
	{
		QMap<int,QList<Node*>> temp1=iter1.value();
		for(QMap<int,QList<Node*>>::iterator iter2=temp1.begin();iter2!=temp1.end();iter2++)
		{
			QList<Node*> temp2=iter2.value();
			for(int i=0;i<temp2.length();i++)
			{
				Node* elem=temp2.at(i);
				fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",number, 1,  elem->x,  elem->y,  elem->z, 1.0, -1);
				number++;
			}

		}
	}
	fclose(fp);
	return;
}



void printSWCByQList_QList(QList<QList <NeuronSWC> > result_list,char* path)
{
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for(int i=0;i<result_list.size();i++)
	{
		QList<NeuronSWC> list1=result_list.at(i); 
		for(int j=0;j<list1.size();j++)
		{
			NeuronSWC temp2=list1.at(j);
			fprintf(fp, "%ld %d %f %f %f %f %ld\n",
				temp2.n, temp2.type,  temp2.x,  temp2.y,  temp2.z, temp2.r, temp2.parent);
		}
	}
	fclose(fp);
	return;
}

void printSWCByQList_QList1(QList<QList <Node*> > result_list,char* path)
{
	FILE * fp = fopen(path, "a");
	if (!fp) return;
	V3DLONG number=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for(int i=0;i<result_list.size();i++)
	{
		QList<Node*> list1=result_list.at(i); 
		for(int j=0;j<list1.size();j++)
		{
			Node* temp2=list1.at(j);
			fprintf(fp, "%ld %d %ld %ld %ld %f %ld\n",
				number, 1,  temp2->x,  temp2->y,  temp2->z, 1.0, -1);
			number++;
		}
	}
	fclose(fp);
	return;
}


void printSWCByQList_QList1_vn2(QList<QList <Node> > result_list,char* path)
{
	printf("size:::%d\n",result_list.size());
	FILE * fp = fopen(path, "a");
	if (!fp) return;
	V3DLONG number=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for(int i=0;i<result_list.size();i++)
	{
		QList<Node> list1=result_list.at(i); 
		for(int j=0;j<list1.size();j++)
		{
			Node temp2=list1.at(j);
			fprintf(fp, "%ld %d %ld %ld %ld %f %ld\n",
				number, 1,  temp2.x,  temp2.y,  temp2.z, 1.0, -1);
			//printf("%ld %d %ld %ld %ld %f %ld\n",number, 1,  temp2.x,  temp2.y,  temp2.z, 1.0, -1);
			number++;
		}
	}
	fclose(fp);
	return;
}


void printSWCByQList_Neuron(QList <NeuronSWC> result_list,const char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "a");
	if (!fp) return; 

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	//for(int i=0;i<result_list.size();i++)
	{
		//QList<NeuronSWC> temp1=result_list.at(i);
		for(int j=0;j< result_list.size();j++)
		{
			NeuronSWC temp2= result_list.at(j);
			//if (j == 0); temp2.parent = -1;
			//printf(" %lf \n",temp2.r);
			//printf( "%ld %d %f %f %f %lf %ld\n",temp2.n, temp2.type,  temp2.x,  temp2.y,  temp2.z, temp2.r, temp2.parent);
			fprintf(fp, "%ld %d %f %f %f %lf %ld\n",
				temp2.n, temp2.type,  temp2.x,  temp2.y,  temp2.z, temp2.r, temp2.parent);

		}
	}

	fclose(fp);
	return;


}

void printSWCByQList_Neuron_pointer(std::vector <NeuronSWC*> result_list,const char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "a");
	if (!fp) return; 

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	//for(int i=0;i<result_list.size();i++)
	{
		//QList<NeuronSWC> temp1=result_list.at(i);
		for(int j=0;j< result_list.size();j++)
		{
			NeuronSWC* temp2= result_list.at(j);
			//if (j == 0); temp2.parent = -1;
			//printf(" %lf \n",temp2.r);
			
			fprintf(fp, "%ld %d %f %f %f %lf %ld\n",
				temp2->n, temp2->type,  temp2->x,  temp2->y,  temp2->z, temp2->r, temp2->parent);

		}
	}

	fclose(fp);
	return;


}
void printSWCByVector_List(vector<QList<Node*> > vector_List,char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	for(int i=0;i<vector_List.size();i++)
	{
		QList<Node*> temp=vector_List.at(i);
		for(int j=0;j<temp.size();j++)
		{
			Node* temp_node=temp.at(j);
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, 1,  temp_node->x,  temp_node->y,  temp_node->z, temp_node->r, -1);
			number++;
		}

	}
	fclose(fp);
	return;


}

void printSwcByList(QList<Node*> nodeList,char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	for(int i = 0; i < nodeList.size(); i++)
	{
		Node* elem = nodeList.at(i);
		fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
			number, 1,  elem->x,  elem->y,  elem->z, elem->r, -1);
		number++;
	}

	fclose(fp);

	return;
}
void printSwcByMap(QMap<int,Node*> nodeMap,char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "wt");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");


	for(QMap<int,Node*>::iterator iter = nodeMap.begin(); iter != nodeMap.end(); iter++)
	{
		Node* temp_node=iter.value();;
		fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
			number, 1,  temp_node->x,  temp_node->y,  temp_node->z, temp_node->r, -1);
		number++;
	}


	fclose(fp);

	return;
}

void printSWCByMultiMap(QMultiMap<int,int> multimap,char *filename)
{
	FILE * fp = fopen(filename, "a");//ﾗｷｼﾓ
	if (!fp) return;
	V3DLONG number=0;
	int front=0;
	int next=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	for(QMultiMap<int,int>::iterator iter2 =multimap.begin(); iter2 != multimap.end();  iter2++)
	{
		next=iter2.value();
		if(front==next)
		{
			continue;

		}else
		{
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, 1,  Map_rootnode[next]->x,  Map_rootnode[next]->y,  Map_rootnode[next]->z, Map_rootnode[next]->r, -1);
			number++;


		}

		front=next;


	}


}

void printSWCByMap_ListInt(QMap<int,QList<int> >  List,char * filename)
{
	FILE * fp = fopen(filename, "a");
	if (!fp) return;
	V3DLONG number=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	for(QMap<int,QList<int> >::iterator iter2 =List.begin(); iter2 != List.end();  iter2++)
	{
		QList<int> List2=iter2.value();
		for(int i=0;i<List2.size();i++)
		{
			int elem = List2.at(i);
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, 1,  Map_rootnode[elem]->x,  Map_rootnode[elem]->y,  Map_rootnode[elem]->z, Map_rootnode[elem]->r, -1);
			number++;

		}

	}
	fclose(fp);

	return;

}

void printSWCByMap_List(QMap<int,QList<Node*> >  List,char * filename)
{
	FILE * fp = fopen(filename, "a");
	if (!fp) return;
	V3DLONG number=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");


	for(QMap<int,QList<Node*> >::iterator iter2 =List.begin(); iter2 != List.end();iter2++)
	{
		QList<Node*> List2=iter2.value();
		for(int i=0;i<List2.size();i++)
		{
			Node* elem = List2.at(i);
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, iter2.key(),  elem->x,  elem->y,  elem->z, elem->r, -1);
			number++;

		}
	}

	fclose(fp);

	return;
}
double cal_weight(V3DLONG curi,V3DLONG curj,V3DLONG curk, V3DLONG x,V3DLONG y,V3DLONG z,double inte_nd,double inte_cen,V3DLONG r)//MeanShiftﾋ羚ｨﾖﾐｼﾆﾋ羞耨・ｭｵ聊ｮｼ莊ﾄﾈｨﾖｵ｣ｬﾀ・ｭｵ耿ｽｽ・ﾄﾈｨﾖｵﾔｽｴｬﾏﾘｺﾍﾔｭｵ耿ｽｽ・ﾄﾈｨﾖｵﾔｽｴ・
{
	//double r=100;
	double I=255;
	double weight=0;
	double distance=(curi-x)/r*(curi-x)/r+(curj-y)/r*(curj-y)/r+(curk-z)/r*(curk-z)/r;
	double inten_sim=(inte_nd-inte_cen)/I*(inte_nd-inte_cen)/I;
	weight=exp(-1.0*sqrt(2*inten_sim+distance))+0.000001;
	//printf("%lf  %lf  %lf \n",distance,inten_sim,weight);
	return weight;
}

#define cal_core(cur,center,radius) exp(-0.1*(abs(cur-center)))+0.000001;
bool compare_NeuronSWC(NeuronSWC node1,NeuronSWC node2)
{
	if((node1.x==node2.x)&&(node1.y==node2.y)&&(node1.z==node2.z))
	{
		return true;

	}else {
		return false;

	}



}
unsigned char* Gauss_filter(unsigned char * &img1d,unsigned char* nData1,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DPluginCallback2 &callback, QWidget *parent)
{
	//printf("11111111111111111111111111111111111111111111\n");
	//根据三维高斯滤波系数对神经元图像进行滤波
	float nSigma=0.4;
	int nWindowSize=1+2*ceil(3*nSigma);
	int nCenter=(nWindowSize)/2;
	int i,j,k;
	//printf("11111111111111111111111111111111111111111111\n");
	unsigned char* nGauss=new unsigned char[sz_x*sz_y*sz_z];//allocate the memory for the filtered image

	//save the filtering result of Gauss in the x direction
	//float* nData2  =new float[sz_x*sz_y*sz_z];//save the filtering result of Gauss in the y direction
	float *Gkernal=new float[nWindowSize];//define the Guass array of three dimension
	float sum=0;
	for(i=0;i<nWindowSize;i++)//generate the Gauss coefficient 
	{

		int nDis=i-nCenter;				
		Gkernal[i]=exp(-(1/2)*(nDis*nDis)/(nSigma*nSigma))/(sqrt(2*3.1415924)*nSigma);	
		sum+=Gkernal[i];
	}
	for(i=0;i<nWindowSize;i++)
	{
		Gkernal[i]/=sum;//normailized

	}
	//filtering the nodes in the X direction
	for(i=0;i<sz_z;i++)
	{
		for(j=0;j<sz_y;j++)
		{
			for(k=0;k<sz_x;k++)
			{
				float dSum=0;
				float dFilter=0;
				for(int nLimit=(-nCenter);nLimit<=nCenter;nLimit++)
				{
					if((k+nLimit)>=0&&(k+nLimit)<sz_x)
					{
						dFilter+=(float)img1d[k+nLimit+j*sz_x+i*sz_x*sz_y]*Gkernal[nCenter+nLimit];
						dSum+=Gkernal[nCenter+nLimit];
					}
				}
				nData1[k+j*sz_x+i*sz_x*sz_y]=(unsigned char)(int)dFilter/dSum;
			}

		}

	}
	//filtering the nodes in the Y direction
	for(i=0;i<sz_z;i++)
	{
		for(j=0;j<sz_x;j++)
		{
			for(k=0;k<sz_y;k++)
			{
				float dSum=0;
				float dFilter=0;
				for(int nLimit=(-nCenter);nLimit<=nCenter;nLimit++)
				{
					if((k+nLimit)>=0&&(k+nLimit)<sz_y)
					{
						dFilter+=(float)nData1[k+nLimit+j*sz_y+i*sz_x*sz_y]*Gkernal[nCenter+nLimit];
						dSum+=Gkernal[nCenter+nLimit];
					}
				}
				nData1[k+j*sz_y+i*sz_x*sz_y]=(unsigned char)(int)dFilter/dSum;
			}

		}

	}
	//filtering the nodes in the Z direction
	for(i=0;i<sz_y;i++)
	{
		for(j=0;j<sz_x;j++)
		{
			for(k=0;k<sz_z;k++)
			{
				float dSum=0;
				float dFilter=0;
				for(int nLimit=(-nCenter);nLimit<=nCenter;nLimit++)
				{
					if((k+nLimit)>=0&&(k+nLimit)<sz_z)
					{
						dFilter+=(float)nData1[k+nLimit+j*sz_z+i*sz_z*sz_x]*Gkernal[nCenter+nLimit];
						dSum+=Gkernal[nCenter+nLimit];
					}
				}
				//int pixel=(int)dFilter/dSum;
				//if(pixel<30)
				//{
				//	nGauss[k+j*sz_z+i*sz_x*sz_z]=0;

				//}else
				//{
				nGauss[k+j*sz_z+i*sz_x*sz_z]=(unsigned char)(int)dFilter/dSum;

				//}

			}

		}

	}


	if(Gkernal) {delete []Gkernal,Gkernal = 0;}


	return nGauss;


}




QList <NeuronSWC> construct_tree_vn5(QMap<int,Node* > roots,unsigned char * &img1d,unsigned char * &img2d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//combined the gradient and distance as weight of spanning to construct every roots
	QList<Node*> seeds;

	for(QMap<int,Node* >::iterator iter=roots.begin();iter!=roots.end();iter++)
	{
		Node* temp=iter.value();
		seeds.append(temp);

	}

	V3DLONG marknum = seeds.size();

	double** markEdge = new double*[marknum];//distance

	//double** markEdge_d=new double*[marknum];
	for(int i = 0; i < marknum; i++)
	{
		markEdge[i] = new double[marknum];

		//markEdge_d[i]=new double[marknum];
		//fprintf(debug_fp,"markEdge[i]:%lf\n",markEdge[i]);
	}
	double x1,y1,z1;
	double sum=0;//maximum value of all distance between nodes
	int count=0;


	double average_dis=0;
	printf("start breath search!%lf\n");
	//markEdge=bf( root,img1d, average_dis,sz_x, sz_y, sz_z);
	bf_vn2( root,markEdge,img2d, average_dis,sz_x, sz_y, sz_z);

	printf(" breath search finished!\n");
	//NeutronTree structure
	NeuronTree marker_MST;
	QList <NeuronSWC> listNeuron;
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
	S.r 	= seeds.at(0)->r;
	S.pn 	= -1;
	//listNeuron.append(S);
	original_tree.insert(GET_IND(seeds.at(0)->x,seeds.at(0)->y,seeds.at(0)->z),S);
	hashNeuron.insert(S.n, listNeuron.size()-1);
	final_listNeuron.append(S);
	listNeuron.append(S);
	cp_SWC.insert(GET_IND(seeds.at(0)->x,seeds.at(0)->y,seeds.at(0)->z),S.pn);

	int* pi = new int[marknum];
	for(int i = 0; i< marknum;i++)
		pi[i] = 0;
	pi[0] = 1;
	int indexi,indexj;
	NeuronSWC pre_S=S;
	QList<NeuronSWC> S_vn2;
	for(int loop = 0; loop<marknum;loop++)
	{
		double min = 100000000;
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
						indexj = j;
					}
				}
			}

		}
		if(indexi>=0)
		{
			//get n and pn of NeuronSWC in the listNeuronSWC every time
			S.n 	= indexj+1;
			S.type 	= 7;
			S.x 	= seeds.at(indexj)->x;
			S.y 	= seeds.at(indexj)->y;
			S.z 	= seeds.at(indexj)->z;
			S.r 	= seeds.at(indexj)->r;
			S.pn 	= indexi+1;

			//printf("",markEdge[i][j]);
			if(markEdge[indexi][indexj]>1000000)
			{
				printf("444444444444444444444444444444444444\n");
			}
			//here record the each shortest node from every node, wirte a function here to select shortest path from "number_path"  
			//I need to find the node which number is indexi and indexj every time and plug the path between them
			//the begin node is seeds.at(indexi) and the end node is seeds.at(indexj)

			V3DLONG loc1=GET_IND(seeds.at(indexi)->x,seeds.at(indexi)->y,seeds.at(indexi)->z);
			V3DLONG loc2=GET_IND(seeds.at(indexj)->x,seeds.at(indexj)->y,seeds.at(indexj)->z);
			QList<Node> temp22;
			temp22.clear();
			QMultiMap<V3DLONG,QMap<V3DLONG,QList<Node>>>::iterator begin=number_path_vn3.lowerBound(loc1);
			QMultiMap<V3DLONG,QMap<V3DLONG,QList<Node>>>::iterator end=number_path_vn3.upperBound(loc1);
			//printf("times:::%d\n",times);
			while((begin!=end))
			{
				
				if(begin.value().contains(loc2))
				{
					temp22=begin.value().value(loc2);
					break;

				}else
				{


				}
				begin++;

			}

			final_path_vn2.append(temp22);

			//connect the shortest path with the begin node and end node
			//	if(temp2.size()!=0)
			{
				S_vn2.clear();
			
				S_vn2=connect_shortest_path_vn4(img1d,temp22,seeds.at(indexi),seeds.at(indexj),sz_x,sz_y,sz_z);
				//insert the root node into listNeuron first and check the next node is also root or not
			
				for(int i=0;i<S_vn2.length();i++)
				{
						
					if(compare_NeuronSWC(listNeuron.first(),S_vn2.first()))
					{
						continue;

					}
					
					listNeuron.append(S_vn2.at(i));
					/*	if(!original_tree.contains(GET_IND(S_vn2.at(i).x,S_vn2.at(i).y,S_vn2.at(i).z)))
					original_tree.insert(GET_IND(S_vn2.at(i).x,S_vn2.at(i).y,S_vn2.at(i).z),S_vn2.at(i));*/

				}
			
			}
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
	//printSWCByQList_Neuron(listNeuron,"C:\\Vaa3D\\list.SWC");

	QList<NeuronSWC> marker_MST_sorted;
	marker_MST_sorted.clear();
	if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
	{
	}

	prepare_write(marker_MST_sorted);

	if(markEdge) {delete []markEdge, markEdge = 0;}
	//if(markEdge_d) {delete []markEdge_d, markEdge_d = 0;}
	number_path_vn3.clear();
	delete [] pi;
	seeds.clear();
	cp_SWC.clear();
	final_listNeuron.clear();
	original_tree.clear();

	return listNeuron;
}




QList<NeuronSWC> connect_shortest_path_vn4(unsigned char * &img1d,QList<Node> path,Node* begin,Node* end,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	QList<NeuronSWC> result;
	result.clear();

	if(path.size()!=0)
	{

		///////////////////////////////////////for testing
		if((begin->x==path.last().x)&&(begin->y==path.last().y)&&(begin->z==path.last().z))
		{
		}else{
			printf("11111111111111111111111111111111\n");
			printf("path size::%d\n",path.size());
			printf("%ld   %ld   %ld \n",begin->x,begin->y,begin->z);
			printf("%ld   %ld   %ld \n",path.last().x,path.last().y,path.last().z);

			//return result;

		}
		if((end->x==path.first().x)&&(end->y==path.first().y)&&(end->z==path.first().z))
		{
		}else
		{
			printf("222222222222222222222222222222222222\n");

		}
		//////////////////////////////////////////////////


		for(int i=path.length()-1;i>=0;i--)
		{

			Node temp=path.at(i);
			NeuronSWC next_S;

			if(cp_SWC.value(GET_IND(temp.x,temp.y,temp.z))==-1)
			{
				continue;

			}else if((final_listNeuron.last().x==temp.x)&&(final_listNeuron.last().y==temp.y)&&(final_listNeuron.last().z==temp.z))
			{
				continue;

			}


			//next_S.n=original_tree.size()+1;
			next_S.n=final_listNeuron.size()+1;
			if(i==path.length()-1)
			{
				if(original_tree.contains(GET_IND(path.at(i).x,path.at(i).y,path.at(i).z)))
				{
					//next_S.pn=original_tree[GET_IND(path.at(i).x,path.at(i).y,path.at(i).z)].pn;
					continue;

				}else
				{
					next_S.pn=original_tree[GET_IND(final_listNeuron.last().x,final_listNeuron.last().y,final_listNeuron.last().z)].n;

				}
			}else
			{
					next_S.pn=original_tree[GET_IND(path.at(i+1).x,path.at(i+1).y,path.at(i+1).z)].n;
			}



			next_S.x=temp.x;
			next_S.y=temp.y;
			next_S.z=temp.z;
			next_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,temp,sz_x, sz_y, sz_z);
			next_S.type=7;
			result.append(next_S);
			final_listNeuron.append(next_S);
			original_tree.insert(GET_IND(temp.x,temp.y,temp.z),next_S);
			cp_SWC.insert(GET_IND(temp.x,temp.y,temp.z),next_S.pn);


		}
	}else
	{
		if(cp_SWC.value(GET_IND(begin->x,begin->y,begin->z))==-1)
		{
			NeuronSWC end_S;
			end_S.x=end->x;
			end_S.y=end->y;
			end_S.z=end->z;
			end_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(end),sz_x, sz_y, sz_z);
			end_S.type=7;
			//end_S.n=original_tree.size()+1;
			end_S.n=final_listNeuron.size()+1;
			end_S.pn=original_tree[GET_IND(begin->x,begin->y,begin->z)].n;
			result.append(end_S);
			final_listNeuron.append(end_S);
			original_tree.insert(GET_IND(end_S.x,end_S.y,end_S.z),end_S);
			cp_SWC.insert(GET_IND(end->x,end->y,end->z),end_S.pn);



		}else if((final_listNeuron.last().x==begin->x)&&(final_listNeuron.last().y==begin->y)&&(final_listNeuron.last().z==begin->z))
		{
			NeuronSWC end_S;
			end_S.x=end->x;
			end_S.y=end->y;
			end_S.z=end->z;
			end_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(end),sz_x, sz_y, sz_z);
			end_S.type=7;
			//end_S.n=original_tree.size()+1;
			end_S.n=final_listNeuron.size()+1;
			end_S.pn=original_tree[GET_IND(begin->x,begin->y,begin->z)].n;
			result.append(end_S);
			final_listNeuron.append(end_S);
			original_tree.insert(GET_IND(end_S.x,end_S.y,end_S.z),end_S);
			cp_SWC.insert(GET_IND(end->x,end->y,end->z),end_S.pn);


		}else
		{
			
			NeuronSWC begin_S;
			begin_S.x=begin->x;
			begin_S.y=begin->y;
			begin_S.z=begin->z;
			//begin_S.n=original_tree[GET_IND(begin_S.x,begin_S.y,begin_S.z)].n;
			if(original_tree.contains(GET_IND(begin_S.x,begin_S.y,begin_S.z)))
			{//if original_tree contain the node which means it has already exist in the tree and meanless to add it again
				begin_S.pn=original_tree[GET_IND(begin_S.x,begin_S.y,begin_S.z)].pn;

			}else
			{
				begin_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(begin),sz_x, sz_y, sz_z);
				begin_S.type=7;
				begin_S.n=final_listNeuron.size()+1;
				begin_S.pn=original_tree[GET_IND(final_listNeuron.last().x,final_listNeuron.last().y,final_listNeuron.last().z)].n;
				result.append(begin_S);
				final_listNeuron.append(begin_S);
				original_tree.insert(GET_IND(begin_S.x,begin_S.y,begin_S.z),begin_S);
				cp_SWC.insert(GET_IND(begin->x,begin->y,begin->z),begin_S.pn);

			}
			
			
			

			NeuronSWC end_S;
			end_S.x=end->x;
			end_S.y=end->y;
			end_S.z=end->z;
			end_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(end),sz_x, sz_y, sz_z);
			end_S.type=7;
			//end_S.n=original_tree.size()+1;
			end_S.n=final_listNeuron.size()+1;
			end_S.pn=original_tree[GET_IND(begin_S.x,begin_S.y,begin_S.z)].n;
			result.append(end_S);
			final_listNeuron.append(end_S);
			original_tree.insert(GET_IND(end_S.x,end_S.y,end_S.z),end_S);
			cp_SWC.insert(GET_IND(end->x,end->y,end->z),end_S.pn);
			

		}

	}

	return result;
}


QList<NeuronSWC> connect_shortest_path_vn3(unsigned char * &img1d,QList<Node> path,Node* begin,Node* end,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	QList<NeuronSWC> result;
	result.clear();

	if(path.size()!=0)
	{
		///////////////////////////////////////for testing
		if((begin->x==path.last().x)&&(begin->y==path.last().y)&&(begin->z==path.last().z))
		{
		}else{
			printf("11111111111111111111111111111111\n");
			printf("path size::%d\n",path.size());
			printf("%ld   %ld   %ld \n",begin->x,begin->y,begin->z);
			printf("%ld   %ld   %ld \n",path.last().x,path.last().y,path.last().z);

			//return result;

		}
		if((end->x==path.first().x)&&(end->y==path.first().y)&&(end->z==path.first().z))
		{
		}else
		{
			printf("222222222222222222222222222222222222\n");

		}
		//////////////////////////////////////////////////
		NeuronSWC head_S;
		head_S.x=path.last().x;
		head_S.y=path.last().y;
		head_S.z=path.last().z;
		head_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,path.last(),sz_x, sz_y, sz_z);
		head_S.type=7;
		//head_S.n=original_tree[GET_IND(head_S.x,head_S.y,head_S.z)].n;
		head_S.n=final_listNeuron.size();
		head_S.pn=original_tree[GET_IND(head_S.x,head_S.y,head_S.z)].pn;
		result.append(head_S);
		//final_listNeuron.append(head_S);
		original_tree.insert(GET_IND(head_S.x,head_S.y,head_S.z),head_S);

		for(int i=path.length()-2;i>=0;i--)
		{
			Node temp=path.at(i);
			NeuronSWC next_S;
			//next_S.n=original_tree.size()+1;
			next_S.n=final_listNeuron.size()+1;

			next_S.pn=original_tree[GET_IND(path.at(i+1).x,path.at(i+1).y,path.at(i+1).z)].n;

			next_S.x=temp.x;
			next_S.y=temp.y;
			next_S.z=temp.z;
			next_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,temp,sz_x, sz_y, sz_z);
			next_S.type=7;
			result.append(next_S);
			final_listNeuron.append(next_S);

			original_tree.insert(GET_IND(temp.x,temp.y,temp.z),next_S);



		}
	}else
	{
		printf("the size of shortest path is zero\n");
		NeuronSWC begin_S;
		begin_S.x=begin->x;
		begin_S.y=begin->y;
		begin_S.z=begin->z;
		begin_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(begin),sz_x, sz_y, sz_z);
		begin_S.type=7;
		//begin_S.n=original_tree[GET_IND(begin_S.x,begin_S.y,begin_S.z)].n;
		begin_S.n=final_listNeuron.size();
		begin_S.pn=original_tree[GET_IND(begin_S.x,begin_S.y,begin_S.z)].pn;
		result.append(begin_S);
		final_listNeuron.append(begin_S);
		original_tree.insert(GET_IND(begin_S.x,begin_S.y,begin_S.z),begin_S);

		NeuronSWC end_S;
		end_S.x=end->x;
		end_S.y=end->y;
		end_S.z=end->z;
		end_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(end),sz_x, sz_y, sz_z);
		end_S.type=7;
		//end_S.n=original_tree.size()+1;
		end_S.n=final_listNeuron.size()+1;
		end_S.pn=original_tree[GET_IND(begin_S.x,begin_S.y,begin_S.z)].n;
		result.append(end_S);
		final_listNeuron.append(end_S);
		original_tree.insert(GET_IND(end_S.x,end_S.y,end_S.z),end_S);


	}

	return result;
}

bool decide_outrange(V3DLONG current_x,V3DLONG current_y,V3DLONG current_z,Node* start,Node* end)
{

	V3DLONG con_startX=start->x;
	V3DLONG con_startY=start->y;
	V3DLONG con_startZ=start->z;

	V3DLONG con_endX=end->x;
	V3DLONG con_endY=end->y;
	V3DLONG con_endZ=end->z;

	V3DLONG step_x=abs(con_startX-con_endX);
	V3DLONG step_y=abs(con_startY-con_endY);
	V3DLONG step_z=abs(con_startZ-con_endZ);

	V3DLONG condition_Xa=abs(current_x-con_startX);
	V3DLONG condition_Xb=abs(current_x-con_endX);
	V3DLONG condition_Ya=abs(current_y-con_startY);
	V3DLONG condition_Yb=abs(current_y-con_endY);
	V3DLONG condition_Za=abs(current_z-con_startZ);
	V3DLONG condition_Zb=abs(current_z-con_endZ);

	if((condition_Xa<=step_x)&&(condition_Xb<=step_x)&&(condition_Ya<=step_y)&&(condition_Yb<=step_y)&&(condition_Za<=step_z)&&(condition_Zb<=step_z))
	{
		return true;


	}else
	{
		return false;

	}

}

void bf_vn2(QMap<int,Node* > roots,double **weight_result,unsigned char * &img1d,double average_dis,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//found the shortest path between one node and other nodes using breadth first algorithm


	QQueue<double> queue_distance;//keep the unit distance of two nodes
	V3DLONG up_limit;
	V3DLONG down_limit;
	int marknum=roots.size();
	int row=-1;
	QQueue<Node> queue1;
	QMap<V3DLONG,bool> node_searched;
	QMap<V3DLONG,Node> parent_path_vn2;
	QList<Node> path_shortest_vn2;
	QMap<V3DLONG,QList<Node>> unit_path_vn3;

	for(QMap<int,Node* >::iterator iter=roots.begin();iter!=roots.end();iter++)
	{
		int cloumn=-1;
		row++;
		int key1=iter.key();
		unit_path.clear();
		Node* node1=iter.value();//start node
		V3DLONG start_x=node1->x;
		V3DLONG start_y=node1->y;
		V3DLONG start_z=node1->z;
		if(start_x==sz_x)
		{
			start_x=start_x-1;
		}
		if(start_y==sz_y)
		{
			start_y=start_y-1;

		}
		if(start_z==sz_z)
		{
			start_z=start_z-1;

		}
		V3DLONG limit1=GET_IND(start_x,start_y,start_z);
		for(QMap<int,Node* >::iterator iter1=roots.begin();iter1!=roots.end();iter1++)
		{


			cloumn++;
			int key2=iter1.key();

			Node* node2=iter1.value();//terminal node
			V3DLONG end_x=node2->x;
			V3DLONG end_y=node2->y;
			V3DLONG end_z=node2->z;
			if(end_z==sz_z)
			{
				end_z=end_z-1;

			}
			V3DLONG limit2=GET_IND(end_x,end_y,end_z);

			queue1.clear();
			node_searched.clear();
			parent_path_vn2.clear();
			path_shortest_vn2.clear();
			unit_path_vn3.clear();
			bool flag=false;//mark the terminal condition which shows the program found the end node from start node


			double stop_distance=(double)(sqrt(double(node2->x-node1->x)*(node2->x-node1->x)+double(node2->y-node1->y)*(node2->y-node1->y)+double(node2->z-node1->z)*(node2->z-node1->z)));
			if((stop_distance>=(30))||stop_distance==0)//stop_distance can be input in the dialogue
			{

				weight_result[row][cloumn]=100000000;

				unit_path_vn3.insert(GET_IND(node2->x,node2->y,node2->z),path_shortest_vn2);

				number_path_vn3.insert(GET_IND(node1->x,node1->y,node1->z),unit_path_vn3);
				continue;

			}else
			{


				V3DLONG node_number=(abs(start_x-end_x)+1)*(abs(start_y-end_y)+1)*(abs(start_z-end_z)+1);
				V3DLONG count=0;
				V3DLONG offset_x=start_x;
				V3DLONG offset_y=start_y;
				V3DLONG offset_z=start_z;
				V3DLONG offset_dis=0;


				queue1.append(getnode(node1));


				//while((count<=node_number)&&(!(offset_x==end_x)&&(offset_y==end_y)&&(offset_z==end_z))&&(queue.size()!=0))
				while((!((offset_x==end_x)&&(offset_y==end_y)&&(offset_z==end_z)))&&(queue1.size()!=0)&&(count<=node_number))//here exists arguement!
				{//condition 1 gaurantee the number of searched node will not more than the nodes in the block,because maybe there is a gap between nodes
					//condition 2 gaurantee the loop will be stoped when the terminal node was found
					//condition 3 gaurantee the loop will be stoped when there is a gap between two nodes, it means that A node will never get to B node

					Node temp1=queue1.head();//get the first node in the queue every loop

					//need to set a flag here means the node was searched
					for(int times=0;times<26;times++)
					{
						//printf("%lf",node_table[times].dist);
						offset_x=temp1.x+node_table[times].i;
						offset_y=temp1.y+node_table[times].j;
						offset_z=temp1.z+node_table[times].k;
						//offset_dis=node_table[times].dist;
						V3DLONG index=GET_IND(offset_x,offset_y,offset_z);

						//if((index<down_limit)||(index>up_limit))//outrange, this method is uncorrect
						//need to write a function here to decide whether the node is outrange
						if(!decide_outrange(offset_x,offset_y,offset_z,node1,node2))
						{
							//	printf("%ld  %ld  %ld\n",index,down_limit,up_limit);
							continue;

						}else if(img1d[index]==0)//not connected ,also means no edge
						{//need to deicde whether the node has choosed before
							if(node_searched[GET_IND(offset_x,offset_y,offset_z)]!=true)//change the code here
							{
								node_searched[GET_IND(offset_x,offset_y,offset_z)]=true;
								count++;
							}

							//	printf("%lf\n",(double) img1d[index]);
							continue;
						}else
						{

							if(node_searched[GET_IND(offset_x,offset_y,offset_z)]!=true)//if true, which  means the node was searched before
							{
								//printf("node_searched\n");
								//parent_path.insert(GET_IND(offset_x,offset_y,offset_z),temp1);//change the type of parent_path into <long,Node>
								parent_path_vn2.insert(GET_IND(offset_x,offset_y,offset_z),temp1);
								count++;//if count equals with node number, that mean all nodes were put in the queue
								Node* temp2=new Node(offset_x,offset_y,offset_z);//consider change it to intensity
								node_searched[GET_IND(offset_x,offset_y,offset_z)]=true;

								queue1.append(getnode(temp2));
								delete temp2;
								//printf("%d\n",queue.size());
								//queue_distance.append(offset_dis);//cannot calculate the node distance here,because it is not the shortest path
								//queue_intensity.append(img1d[GET_IND(offset_x,offset_y,offset_z)]);

							}

							if((offset_x==end_x)&&(offset_y==end_y)&&(offset_z==end_z))
							{
								flag=true;//found the terminal node, stop
								//printf("found the terminal node\n");
								break;

							}
						}
					}

					queue1.dequeue();
				}
				queue1.clear();
				if(flag)
				{
					// if get the terminal node, found the shortest path using backward method
					//found out the shortest path between two nodes from terminal node to beginning node
					path_shortest_vn2=found_path_vn3(parent_path_vn2,node1,node2,sz_x,sz_y,sz_z);

					weight_result[row][cloumn]=distance_calculate_vn2(img1d,path_shortest_vn2,sz_x,sz_y,sz_z);
					unit_path_vn3.insert(GET_IND(node2->x,node2->y,node2->z),path_shortest_vn2);
					number_path_vn3.insert(GET_IND(node1->x,node1->y,node1->z),unit_path_vn3);

					if(path_shortest_vn2.size()==0) 
					{
						v3d_msg(QString("the shortest path is empty, program crash."));

					}

				}else
				{
					//if not found the terminal node that means maybe there is a gap between two nodes and A node cannot ever get to B node
					//weight_result[row][cloumn]=100000000;
					parent_path_vn2.clear();
					path_shortest_vn2.clear();

					unit_path_vn3.insert(GET_IND(node2->x,node2->y,node2->z),path_shortest_vn2);
					number_path_vn3.insert(GET_IND(node1->x,node1->y,node1->z),unit_path_vn3);
					weight_result[row][cloumn]=stop_distance*2;
				}
			}

			node_searched.clear();
			path_shortest_vn2.clear();
			parent_path_vn2.clear();
			//qDeleteAll(unit_path_vn3.begin(),unit_path_vn3.end());
			unit_path_vn3.clear();
		}
	}

	return;
}




QList<Node> found_path_vn3( QMap<V3DLONG,Node> path_map, Node* temp,Node* temp1,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//considering the distance and pixal intensity as weight so as to calculate the markEdge
	bool flag=false;
	QList<Node> result;
	result.append(getnode(temp1));//temp1 represent the terminal node
	//printf("length is correct:%ld\n",current_node->x);
	V3DLONG index;

	V3DLONG current_x=result.first().x;
	V3DLONG current_y=result.first().y;
	V3DLONG current_z=result.first().z;

	while(!((current_x==temp->x)&&(current_y==temp->y)&&(current_z==temp->z)))
	{
		//printf("looping1...\n");
		if(path_map.contains(GET_IND(current_x,current_y,current_z)))
		{
			//printf("contained,%ld   %ld   %ld\n",path_map[GET_IND(current_x,current_y,current_z)]->x,path_map[GET_IND(current_x,current_y,current_z)]->y,path_map[GET_IND(current_x,current_y,current_z)]->z);

		}
		V3DLONG x1=path_map[GET_IND(current_x,current_y,current_z)].x;
		V3DLONG y1=path_map[GET_IND(current_x,current_y,current_z)].y;
		V3DLONG z1=path_map[GET_IND(current_x,current_y,current_z)].z;

		current_x=x1;
		current_y=y1;
		current_z=z1;
		//printf("looping2...\n");
		if(((current_x==temp->x)&&(current_y==temp->y)&&(current_z==temp->z)))
		{
			result.append(getnode(temp));
			break;


		}else
		{
			//Node* node_save=&path_map[GET_IND(current_x,current_y,current_z)];
			result.append(path_map[GET_IND(current_x,current_y,current_z)]);

		}
		//printf("looping3...\n");


	}
	//printf("out while...\n");
	return result;
}

double distance_calculate(unsigned char * &img1d,QList<Node*> path,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//Temporally, just take the distance as weight
	double sum_distance=0;
	double varience_intensity;
	for(int i=0;i<path.length()-1;i++)
	{
		Node* first=path.at(i);
		Node* second=path.at(i+1);
		double distance=sqrt((double)(second->x-first->x)*(second->x-first->x)+(double)(second->y-first->y)*(second->y-first->y)+(double)(second->z-first->z)*(second->z-first->z));
		sum_distance+=distance;
	}


	return sum_distance;


}

double distance_calculate_vn2(unsigned char * &img1d,QList<Node> path,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//Temporally, just take the distance as weight
	double sum_distance=0;
	double varience_intensity;
	for(int i=0;i<path.length()-1;i++)
	{
		Node first=path.at(i);
		Node second=path.at(i+1);
		double distance=sqrt((double)(second.x-first.x)*(second.x-first.x)+(double)(second.y-first.y)*(second.y-first.y)+(double)(second.z-first.z)*(second.z-first.z));
		sum_distance+=distance;
	}


	return sum_distance;


}




int meanshift_plugin_vn4(V3DPluginCallback2 &callback, QWidget *parent,unsigned char* img1d,V3DLONG *in_sz, QString &image_name,bool bmenu)
{
	/////////////////////////////////////////////////////////
	v3dhandle curwin = callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No image is open.");
		return 1;
	}

	Image4DSimple *p4d = callback.getImage(curwin);
	unsigned char* img2d = p4d->getRawDataAtChannel(1);//used for keeping the Gauss denoising result

	////////////////////////////////////////////////////////////
	//这里需要进行第一次去噪，去掉一些像素小的点，但其他点的像素基本上不改变
	V3DLONG sz_x = in_sz[0];
	V3DLONG sz_y = in_sz[1];
	V3DLONG sz_z = in_sz[2];
	flag=new int [sz_x*sz_y*sz_z];
	unsigned char* nData1=new unsigned char[sz_x*sz_y*(sz_z)];

	V3DLONG r=10;
	V3DLONG count=0;
	int times=100;
	int num_mark=1;

	printf("### find out rootnode  ###\n");


	for(V3DLONG ii=0;ii<sz_x;ii++)
	{
		for(V3DLONG jj=0;jj<sz_y;jj++)
		{
			for(V3DLONG kk=0;kk<sz_z;kk++)
			{
				V3DLONG ind=GET_IND(ii,jj,kk);

				/*if(img1d[ind]==0)
				continue;*/
				if((double)img1d[ind]<30)
					continue;//take the node whose intensity more than 30 and cannot shift anymore as rootnode
				if(!found_final(img1d,ii,jj,kk,sz_x, sz_y, sz_z, r))
				{
					Node* final=new Node(ii,jj,kk);

					final->class_mark=num_mark;
					final->parent=-1;
					final->number=0;
					Map_finalnode.insert(ind,num_mark);
					enlarge_radiusof_single_node_xy(img1d,final,sz_x, sz_y, sz_z);
					Map_rootnode.insert(num_mark,final);
					if((double)img1d[GET_IND(ii,jj,kk)]==0)
						printf("%lf\n",(double)img1d[GET_IND(ii,jj,kk)]);
					//Map_rootnode_n.insert(num_mark,getnode(final));
					num_mark++;
					//delete final;

				}

			}
		}
	}
	//printSwcByMap(Map_rootnode,"D:\\result\\final.swc");

	printf("###  rootnode found   ###\n");
	//在这里将找到的root打印出来

	printf("###  cluster nodes using meanshift   ###\n");

	for(V3DLONG i=0;i<sz_x;i++)
	{
		for(V3DLONG j=0;j<sz_y;j++)
		{
			for(V3DLONG k=0;k<sz_z;k++)
			{
				V3DLONG ind=GET_IND(i,j,k);
				/*if((double)img1d[ind]<1)
				continue;
				*/
				if(img1d[ind]<30)
					continue;//try to cluster every node whose intensity is more than 0,but if the intensity is less than 30, we regard it as noise node;
				if(flag[ind]==1)
				{

					continue;

				}
				//printf("111111111111111111111111111111111\n");
				meanshift_vn4(img1d,i,j,k,sz_x,sz_y,sz_z,r,times);
			}

		}

	}
	//printSWCByMap_List(Map_finalnode_list,"D:\\result\\compare.swc");
	printf("###  cluster finished   ###\n");

	merge_rootnode(Map_rootnode,img1d,sz_x,sz_y,sz_z);


	//merge_rootnode(Map_rootnode,img1d,sz_x,sz_y,sz_z);
	printf("###   Smooth the image using Gauss method   ###\n");//这里作为第二次去噪，采用distance transform计算中心线
	img2d=Gauss_filter(img1d,nData1,sz_x,sz_y,sz_z,callback, parent);//for calculate the gradient of nodes better

	printf("###   Smooth finished   ###\n");
	//construct_tree_vn2(finalclass_node, img1d,sz_x, sz_y, sz_z);

	//construct_tree_vn4(root, img1d,sz_x, sz_y, sz_z);//call bf method to find the shortest path between nodes
	printf("###   connect all roots using an improved spanning tree which adopt bf method to calculate distance\n");
	con_tree=construct_tree_vn5(root,img1d,img2d,sz_x,sz_y,sz_z);
	printf("###   roots connected   ###\n");
	//printSWCByQMap_QMap("C:\\Vaa3D\\shortest_path.swc",number_path);
	//printSWCByQList_QList1(final_path,"C:\\Vaa3D\\shortest_path.swc");
	//printSWCByQList_QList1_vn2(final_path_vn2,"C:\\Vaa3D\\shortest_path.swc");
	
	printf("fix the radius of nodes and trim some covering nodes\n");
	std::vector<NeuronSWC *> target;

	//change_type(target,con_tree);
	//printf("target size:::%d\n",target.size());
	//smooth_curve_and_radius(target, 5);
	con_tree=smooth_SWC_radius(con_tree,sz_x,sz_y,sz_z);
	QString outswc_file = image_name + "_meanshift.swc";
	
printSWCByQList_Neuron(con_tree,outswc_file.toStdString().c_str());
	//printSWCByQList_Neuron_pointer(target,outswc_file.toStdString().c_str());
	printf("%s\n",outswc_file.toStdString().c_str());
	v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outswc_file.toStdString().c_str()),bmenu);
	if(nData1) {delete []nData1,nData1 = 0;}
	delete []flag;
}

QList<NeuronSWC> smooth_SWC_radius(QList<NeuronSWC> target,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	QList<NeuronSWC> result;
	QHash<V3DLONG,NeuronSWC> neuron;//for searching the node using V3DLONG
	QList<V3DLONG> childs;
	for(int j=0;j<target.size();j++)
	{
		NeuronSWC temp=target.at(j);
		V3DLONG index=GET_IND(temp.x,temp.y,temp.z);
		neuron.insert(index,temp);
	}
	QHash<V3DLONG,V3DLONG> cp=Child_Parent_Wan(target,sz_x,sz_y,sz_z);
	QHash<V3DLONG,V3DLONG> pc=Parent_Child_Wan(target,sz_x,sz_y,sz_z);

	for(int i=0;i<target.size();i++)
	{
		if(i==0||i==target.size()-1)
		{
			result.append(target.at(i));

		}else
		{
			childs.clear();
			NeuronSWC element=target.at(i);
			V3DLONG parent=cp.value(GET_IND(element.x,element.y,element.z));
			childs=pc.values(GET_IND(element.x,element.y,element.z));
			element.r=fix_radius(neuron,parent,childs);
			result.append(element);
		}
	}
	return result;
}

double fix_radius(QHash<V3DLONG,NeuronSWC> neuron,V3DLONG Parent,QList<V3DLONG> Childs )
{
	double result;
	result=neuron.value(Parent).r;
	for(int i=0;i<Childs.size();i++)
	{
		result+=neuron.value(Childs.at(i)).r;

	}
	return result/(Childs.size()+1);
}
QHash<V3DLONG,V3DLONG> Child_Parent_Wan(QList<NeuronSWC> target,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	QHash<V3DLONG,V3DLONG> result;
	
	for(int i=1;i<target.size()-1;i++)//ignore the first and end node
	{
		NeuronSWC element_child=target.at(i);
		V3DLONG child=GET_IND(element_child.x,element_child.y,element_child.z);
		NeuronSWC element_parent=target.at(element_child.parent-1);
		V3DLONG parent=GET_IND(element_parent.x,element_parent.y,element_parent.z);
		result.insert(child,parent);//every node have a single parent
		//neuron.insert(child,element_child);


	}

	return result;
}

QHash<V3DLONG,V3DLONG> Parent_Child_Wan(QList<NeuronSWC> target,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	QHash<V3DLONG,V3DLONG> result;
	for(int i=1;i<target.size()-1;i++)//ignore the first and end node
	{
		NeuronSWC element_child=target.at(i);
		V3DLONG child=GET_IND(element_child.x,element_child.y,element_child.z);
		NeuronSWC element_parent=target.at(element_child.parent-1);
		V3DLONG parent=GET_IND(element_parent.x,element_parent.y,element_parent.z);
		result.insertMulti(parent,child);//one parent may have zero or multi childs
		
	}

	return result;
}


void change_type(std::vector<NeuronSWC *> &target,QList<NeuronSWC> source)
{
	for(int i=0;i<source.size();i++)
	{
		NeuronSWC *element=new NeuronSWC();
		element->n=source.at(i).n;
		element->type=source.at(i).type;
		element->x=source.at(i).x;
		element->y=source.at(i).y;
		element->z=source.at(i).z;
		element->r=source.at(i).r;
		element->radius=source.at(i).radius;
		element->parent=source.at(i).parent;
		element->pn=source.at(i).pn;
		target.push_back(element);
	}

}






void merge_rootnode(QMap<int,Node*> &rootnodes,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{

	QMultiMap<double, int> r_root;

	QMultiMap<int,int> parent_root;


	QMap<int,int> evidence2;
	printf("rootnodes:::%d\n",rootnodes.size());
	for(QMap<int,Node*>::iterator iter =rootnodes.begin(); iter != rootnodes.end(); iter++)
	{//check which root include which roots
		Node* elem=iter.value();
		int key=iter.key();
		r_root.insert(elem->r,key);
		evidence2.insert(key,0);
		parent_root.insert(key,key);

		for(QMap<int,Node*>::iterator iter1 =rootnodes.begin(); iter1 != rootnodes.end(); iter1++)
		{
			Node* elem1=iter1.value();
			int key1=iter1.key();
			if((elem1->x==elem->x)&&(elem1->y==elem->y)&&(elem1->z==elem->z))
				continue;
			double dis=(double)sqrt((double)(elem->x-elem1->x)*(elem->x-elem1->x)+(double)(elem->y-elem1->y)*(elem->y-elem1->y)+(double)(elem->z-elem1->z)*(elem->z-elem1->z));

			if(dis/(elem->r+elem1->r)<1.5)
			{
				parent_root.insert(key,key1);

			}
		}

		if(!parent_root.contains(key))
		{
			parent_root.insert(key,key);

		}
	}

	QMap<int,QList<int> > Map_finallist_new;

	printf("parent_root:::%d\n",parent_root.size());
	printf("r_root:::%d\n",r_root.size());

	for(QMultiMap<double,int>::iterator iter3=r_root.end();iter3!=r_root.begin();iter3--)
	{
		int times=0;
		int root_key=iter3.value();
		if(iter3==r_root.end())
		{

			continue;

		}

		if(evidence2[root_key]==1)
		{// it means the root was already included by others and need not to cluster the nodes for it 

			continue;

		}else
		{
			root.insert(root_key,Map_rootnode[root_key]);
			QMultiMap<int,int>::iterator begin=parent_root.lowerBound(root_key);
			QMultiMap<int,int>::iterator end=parent_root.upperBound(root_key);
			while(begin!=end)
			{
				//printf("begin:::%d\n",begin.value());
				Map_finallist_new[root_key].append(begin.value());
				evidence2[begin.value()]=1;
				begin++;
				times++;//for testing
			}
			//printf("times:::%d\n",Map_finallist_new[root_key].size());
		}

	}



	for(QMap<int,QList<int> >::iterator iter3 =Map_finallist_new.begin(); iter3!=Map_finallist_new.end(); iter3++)
	{//cluster the nodes for every roots
		int first_index=iter3.key();

		for(int i=0;i<Map_finallist_new[first_index].size();i++)
		{
			int second_index=Map_finallist_new[first_index].at(i);

			for(int j=0;j<Map_finalnode_list[second_index].size();j++)
			{
				Node* elem3=Map_finalnode_list[second_index].at(j);
				if(Map_nodes[GET_IND(elem3->x,elem3->y,elem3->z)]!=-1)
				{
					finalclass_node[first_index].append(Map_finalnode_list[second_index].at(j));
				}
				Map_nodes[GET_IND(elem3->x,elem3->y,elem3->z)]=-1;
			}
		}
	}


	printSwcByMap(root,"C:\\Vaa3D\\finalroot.swc");


}


QList<Node*> trim_nodes(QList<Node*> seed,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	QList<Node*> result_seeds;
	QMultiMap<double,Node*> seed_radius;
	QMultiMap<V3DLONG,V3DLONG> parent_node;
	QMap<V3DLONG,int> flag;

	result_seeds.clear();
	seed_radius.clear();
	parent_node.clear();
	flag.clear();
	for(int i=0;i<seed.size();i++)
	{
		Node* elem1=seed.at(i);
		V3DLONG key=GET_IND(elem1->x,elem1->y,elem1->z);
		seed_radius.insert(elem1->r,elem1);
		flag.insert(GET_IND(elem1->x,elem1->y,elem1->z),0);
		for(int j=0;j<seed.size();j++)
		{
			Node* elem=seed.at(j);
			V3DLONG key1=GET_IND(elem->x,elem->y,elem->z);
			if((elem1->x==elem->x)&&(elem1->y==elem->y)&&(elem1->z==elem->z))
				continue;
			double dis=(double)sqrt((double)(elem->x-elem1->x)*(elem->x-elem1->x)+(double)(elem->y-elem1->y)*(elem->y-elem1->y)+(double)(elem->z-elem1->z)*(elem->z-elem1->z));
			if(dis/(elem->r+elem1->r)<1.2)
			{
				parent_node.insert(key,key1);

			}
		}

	}

	for(QMultiMap<double,Node*>::iterator iter=seed_radius.end();iter!=seed_radius.begin();iter--)
	{
		if(iter==seed_radius.end())
			continue; 
		Node* temp=iter.value();
		V3DLONG key2=GET_IND(temp->x,temp->y,temp->z);
		if(flag[GET_IND(temp->x,temp->y,temp->z)]==0)
		{
			result_seeds.append(temp);
			QMultiMap<V3DLONG,V3DLONG>::iterator begin=parent_node.lowerBound(key2);
			QMultiMap<V3DLONG,V3DLONG>::iterator end=parent_node.upperBound(key2);
			//printf("times:::%d\n",times);
			while((begin!=end))
			{
				flag[begin.value()]=1;
				begin++;

			}
			flag[GET_IND(temp->x,temp->y,temp->z)]=1;

		}
	}

	return result_seeds;

}
void prepare_write(QList<NeuronSWC> marker_MST_sorted)
{

	V3DLONG n_temp=0;
	V3DLONG n_parent=0;
	QList<NeuronSWC> temp_marker_sorted;
	temp_marker_sorted.clear();

	if(result_list.size()==0)
	{
		n_temp=0;

	}else
	{
		n_temp=result_list.back().n;
		//printf("n_temp:::%d\n",n_temp);

	}

	for(int ii=0;ii<marker_MST_sorted.size();ii++)
	{
		NeuronSWC neuron=marker_MST_sorted.at(ii);
		neuron.n=n_temp+neuron.n;
		if(neuron.parent!=-1)
		{
			neuron.parent=n_temp+neuron.parent;

		}else
		{
			neuron.parent=n_temp+neuron.parent+2;

		}
		result_list.append(neuron);
		temp_marker_sorted.append(neuron);
	}


}




bool found_final(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r)
{
	double intensity=img1d[GET_IND(x,y,z)];
	double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;

	V3DLONG xe=x-r;if(xe<0) xe=0;
	V3DLONG xb=x+r+1;if(xb>sz_x) xb=sz_x;
	V3DLONG ye=y-r;if(ye<0) ye=0;
	V3DLONG yb=y+r+1;if(yb>sz_y) yb=sz_y;
	V3DLONG ze=z-r;if(ze<0) ze=0;
	V3DLONG zb=z+r+1;if(zb>sz_z) zb=sz_z;
	for(V3DLONG i=xe;i<xb;i++) 
	{
		for(V3DLONG j=ye;j<yb;j++)
		{
			for(V3DLONG k=ze;k<zb;k++)
			{

				if(GET_IND(i,j,k)==GET_IND(x,y,z)) continue;
				if(img1d[GET_IND(i,j,k)]==0) continue;
				//if(img1d[GET_IND(i,j,k)]<30) continue;//it is unnecessary to set a threshold here, because this function mainly deal with deciding whether the node is root.
				double cur_intensity=img1d[GET_IND(i,j,k)];
				w=cal_weight(i,j,k,x,y,z,cur_intensity,intensity,r);

				double core_z=cal_core(k,z,r);
				sum1_z+=core_z*w*k;
				sum2_z+=core_z*w;
				//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

				double core_y=cal_core(j,y,r);
				sum1_y+=core_y*w*j;
				sum2_y+=core_y*w;

				double core_x=cal_core(i,x,r);
				sum1_x+=core_x*w*i;
				sum2_x+=core_x*w;
				//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

			}
		}
	}

	if((sum2_x==0)&&(sum2_y==0)&&(sum2_z==0))
	{
		return false;

	}

	V3DLONG next_x=(sum1_x/sum2_x)+0.5;
	V3DLONG next_y=(sum1_y/sum2_y)+0.5;
	V3DLONG next_z=(sum1_z/sum2_z)+0.5;
	if((next_x==x)&&(next_y==y)&&(next_z==z))
	{
		return false;

	}else
	{
		return true;

	}


}

void meanshift_vn4(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration)
{
	int iter=0;
	int same_times=0;
	int situation=0;

	QList<Node*> nodeList;
	nodeList.clear();
	Node *cur_center=new Node(x,y,z);
	double intensity=img1d[GET_IND(x,y,z)];
	//if(intensity<30)// regard the node whose intensity is less than 30 as noise
	nodeList.append(cur_center);

	while(1)
	{
		if(iter==iteration)
		{
			break;

		}else
		{
			double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
			V3DLONG xe=cur_center->x-r;if(xe<0) xe=0;
			V3DLONG xb=cur_center->x+r+1;if(xb>sz_x) xb=sz_x;
			V3DLONG ye=cur_center->y-r;if(ye<0) ye=0;
			V3DLONG yb=cur_center->y+r+1;if(yb>sz_y) yb=sz_y;
			V3DLONG ze=cur_center->z-r;if(ze<0) ze=0;
			V3DLONG zb=cur_center->z+r+1;if(zb>sz_z) zb=sz_z;
			for(V3DLONG i=xe;i<xb;i++) 
			{
				for(V3DLONG j=ye;j<yb;j++)
				{
					for(V3DLONG k=ze;k<zb;k++)
					{

						if(GET_IND(i,j,k)==GET_IND(cur_center->x,cur_center->y,cur_center->z)) continue;
						if((double)img1d[GET_IND(i,j,k)]==0) continue;
						//if((double)img1d[GET_IND(i,j,k)]<30) continue;
						double cur_intensity=img1d[GET_IND(i,j,k)];
						w=cal_weight(i,j,k,cur_center->x,cur_center->y,cur_center->z,cur_intensity,intensity,r);

						double core_z=cal_core(k,cur_center->z,r);
						sum1_z+=core_z*w*k;
						sum2_z+=core_z*w;
						//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

						double core_y=cal_core(j,cur_center->y,r);
						sum1_y+=core_y*w*j;
						sum2_y+=core_y*w;

						double core_x=cal_core(i,cur_center->x,r);
						sum1_x+=core_x*w*i;
						sum2_x+=core_x*w;
						//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

					}
				}
			}

			V3DLONG temp_x=cur_center->x;
			V3DLONG temp_y=cur_center->y;
			V3DLONG temp_z=cur_center->z;
			Node *pre_center=cur_center;
			flag[GET_IND(temp_x,temp_y,temp_z)]=1;
			//printf("%lf   %lf   %lf\n",sum2_x,sum2_y,sum2_z);
			if ((sum2_x==0)&&(sum2_y==0)&&(sum2_z==0))//avoiding the program crash.
			{
				cur_center->x=temp_x;
				cur_center->y=temp_y;
				cur_center->z=temp_z;
				intensity=img1d[GET_IND(cur_center->x,cur_center->y,cur_center->z)];
				//enlarge_radiusof_single_node_xy(img1d,cur_center,sz_x,sz_y,sz_z);

			}else
			{
				V3DLONG next_x=(sum1_x/sum2_x)+0.5;//may shift to the node whose intensity equals zero, and may shfit to the node which was not included in the roots found in the first step
				V3DLONG next_y=(sum1_y/sum2_y)+0.5;
				V3DLONG next_z=(sum1_z/sum2_z)+0.5;
				cur_center->x=next_x;
				cur_center->y=next_y;
				cur_center->z=next_z;
				intensity=img1d[GET_IND(cur_center->x,cur_center->y,cur_center->z)];

			}

			//Node *pre_center=new Node(temp_x,temp_y,temp_z);
			enlarge_radiusof_single_node_xy(img1d,pre_center,sz_x,sz_y,sz_z);

			if(!nodeList.contains(pre_center)&&(intensity>=30))
			{	

				//printf("%lf\n",pre_center->r);
				nodeList.append(pre_center);
			}	

			/*	if(flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]==1)
			{
			break;
			}*/

			if(GET_IND(cur_center->x,cur_center->y,cur_center->z)==GET_IND(pre_center->x,pre_center->y,pre_center->z))
				break;

			iter++;

		}
	}
	//bool f=found_final(img1d,cur_center->x,cur_center->y,cur_center->z,sz_x,sz_y,sz_z,r);
	V3DLONG ind2=GET_IND(cur_center->x,cur_center->y,cur_center->z);//here,we take the cur_center as a final root due to it cannot shift in the region

	double temp_dis=1000000;

	int mark=0;
	if(Map_finalnode.contains(ind2))
	{
		//printf("1111111111111111111111\n");

		for(int iii=0;iii<nodeList.size();iii++)
		{
			V3DLONG node_x=nodeList.at(iii)->x;
			V3DLONG node_y=nodeList.at(iii)->y;
			V3DLONG node_z=nodeList.at(iii)->z;
			Map_nodes[GET_IND(node_x,node_y,node_z)]=Map_finalnode[ind2];
			Map_allnodes.insert(Map_finalnode[ind2],nodeList.at(iii));
			Map_finalnode_list[Map_finalnode[ind2]].append(nodeList.at(iii));

		}

	}else if(Map_nodes.contains(GET_IND(cur_center->x,cur_center->y,cur_center->z)))
	{
		//printf("2222222222222222222222222\n");
		situation=2;
		int mark3=Map_nodes[GET_IND(cur_center->x,cur_center->y,cur_center->z)];

		for(int iii=0;iii<nodeList.size();iii++)
		{
			V3DLONG node_x2=nodeList.at(iii)->x;
			V3DLONG node_y2=nodeList.at(iii)->y;
			V3DLONG node_z2=nodeList.at(iii)->z;
			Map_nodes[GET_IND(node_x2,node_y2,node_z2)]=mark3;
			Map_allnodes.insert(mark3,nodeList.at(iii));
			Map_finalnode_list[mark3].append(nodeList.at(iii));

		}

	}
	else
	{//not figure out why the roots found in the first step did not include ind2?20150417
		//maybe because the first step did not find all roots, if we find all of them, we need to change code, slove it later
		//printf("33333333333333333333333333333\n");
		int new_mark=Map_rootnode.size()+1;
		cur_center->class_mark=new_mark;

		//enlarge_radiusof_single_node_xy(img1d,cur_center,sz_x,sz_y,sz_z);
		if(cur_center->r==0)
			printf("%lf\n",cur_center->r);

		//printf("%lf\n",(double)img1d[ind2]);


		Map_rootnode.insert(new_mark,cur_center);

		for(int iii=0;iii<nodeList.size();iii++)
		{
			V3DLONG node_x1=nodeList.at(iii)->x;
			V3DLONG node_y1=nodeList.at(iii)->y;
			V3DLONG node_z1=nodeList.at(iii)->z;

			Map_nodes[GET_IND(node_x1,node_y1,node_z1)]=new_mark;
			Map_allnodes.insert(new_mark,nodeList.at(iii));
			Map_finalnode_list[new_mark].append(nodeList.at(iii));

		}
	}



	//delete cur_center;//2015.04.15，为什么删除这个程序能跑完，但出来的结果会和不删除时不一样？不删除时结果不正确，会出现长线条

}

int meanshift_plugin(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to image_threshold function"<<endl;
	if(input.size() != 2 || output.size() != 1) 
	{
		cout<<"illegal input!"<<endl;
		printHelp();
		return -1;
	}


	// 1 - Read input image
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	if (inlist->size() != 1)
	{
		cout<<"You must specify 1 input file!"<<endl;
		return -1;
	}
	char * infile = inlist->at(0);
	cout<<"input file: "<<infile<<endl;
	unsigned char * inimg1d = NULL;
	V3DLONG * sz = NULL;
	int datatype;
	if (!loadImage(infile, inimg1d, sz, datatype)) return -1;


	// 2 - Read color channel and threshold parameter
	vector<char*>* paralist = (vector<char*>*)(input.at(1).p);
	if (paralist->size() != 2)
	{
		cout<<"Illegal parameter!"<<endl;
		printHelp();
		return -1;
	}
	int c = atoi(paralist->at(0));
	int thres = atoi(paralist->at(1));
	cout<<"color channel: "<<c<<endl;
	cout<<"threshold : "<<thres<<endl;
	if (c < 0 || c>=sz[3])
	{
		cout<<"The color channel does not exist!"<<endl;
		return -1;
	}
	loadImage(infile, inimg1d, sz, datatype, c);


	// 3 - Read output fileName
	vector<char*>* outlist = (vector<char*>*)(output.at(0).p);
	if (outlist->size() != 1)
	{
		cout<<"You must specify 1 output file!"<<endl;
		return -1;
	}
	char * outfile = outlist->at(0);
	cout<<"output file: "<<outfile<<endl;

	// 4 - Do binary segmentation
	V3DLONG tb = sz[0]*sz[1]*sz[2]*datatype;
	unsigned char * nm = NULL;
	try {
		nm = new unsigned char [tb];
	} catch (...) {
		throw("Fail to allocate memory in Image Thresholding plugin.");
	}
	for (V3DLONG i=0;i<tb;i++)
	{
		if (inimg1d[i]>=thres) nm[i] = 255;
		else nm[i] = 0;
	}

	// 5 - Save file and free memory
	sz[3] = 1;
	saveImage(outfile, nm, sz, datatype);
	if (nm) {delete []nm; nm=NULL;}

	return 1;
}

void printHelp()
{
	cout<<"\nThis is a demo plugin to perform binary thresholding in an image. by Yinan Wan 2012-02"<<endl;
	cout<<"\nUsage: v3d -x <example_plugin_name> -f image_thresholding -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
	cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
	cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
	cout<<"\t -p <subject_color_channel> <threshold>     the channel you want to perform thresholding and the threshold"<<endl;
	cout<<"\t                                            the 2 paras must come in this order"<<endl;
	cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
	return;
}


