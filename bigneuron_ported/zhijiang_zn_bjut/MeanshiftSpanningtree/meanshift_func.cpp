/* example_func.cpp
* This file contains the functions used in plugin domenu and dufunc, you can use it as a demo.
* 2012-02-13 : by Yinan Wan
*/

#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "meanshift_func.h"
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

QMap<V3DLONG,int> Map_finalnode; 
QMap<V3DLONG,int> Map_nodes;
QMap<V3DLONG,V3DLONG> Map_allnode;
QList <NeuronSWC> result_list;
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
	int max=0;//ﾕﾒﾗ鋗・ｶ
	double threshold=30;

	list_node->append(getnode(node));
	

	while(1)//ｻﾚｸﾚｵ耡ﾔ1ﾎｪｲｽｳ､｣ｬﾒｻﾖﾜﾒｻﾖﾜﾍ簑ｩ｣ｬｰﾑｷｶﾎｧﾄﾚｵﾄｵ羝ｼｼﾓｵｽｶﾓﾁﾐﾀ・・
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
		
		//if(!queue->contains(up_queue))//ｶﾓﾁﾐﾀ・貪ｻﾓﾐｰ・ｨｸﾃｽﾚｵ・
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
			fprintf(fp, "%ld %d %f %f %f %lf %ld\n",
				temp2.n, temp2.type,  temp2.x,  temp2.y,  temp2.z, temp2.r, temp2.parent);

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
Node* found_direction(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r)
{//shift the original node, found the final node, calculate their slope, shift only once

	Node *cur_center=new Node(x,y,z);
	double intensity=img1d[GET_IND(x,y,z)];
	//if(intensity<30)// regard the node whose intensity is less than 30 as noise
	//nodeList.append(cur_center);

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

				double cur_intensity=img1d[GET_IND(i,j,k)];
				w=cal_weight(i,j,k,cur_center->x,cur_center->y,cur_center->z,cur_intensity,intensity,r);

				double core_z=cal_core(k,cur_center->z,r);
				sum1_z+=core_z*w*k;
				sum2_z+=core_z*w;

				double core_y=cal_core(j,cur_center->y,r);
				sum1_y+=core_y*w*j;
				sum2_y+=core_y*w;

				double core_x=cal_core(i,cur_center->x,r);
				sum1_x+=core_x*w*i;
				sum2_x+=core_x*w;

			}
		}
	}

	V3DLONG temp_x=cur_center->x;
	V3DLONG temp_y=cur_center->y;
	V3DLONG temp_z=cur_center->z;

	//printf("%lf   %lf   %lf\n",sum2_x,sum2_y,sum2_z);
	if ((sum2_x==0)&&(sum2_y==0)&&(sum2_z==0))//avoiding the program crash.
	{
		cur_center->x=temp_x;
		cur_center->y=temp_y;
		cur_center->z=temp_z;

	}else
	{
		V3DLONG next_x=(sum1_x/sum2_x)+0.5;//may shift to the node whose intensity equals zero, and may shfit to the node which was not included in the roots found in the first step
		V3DLONG next_y=(sum1_y/sum2_y)+0.5;
		V3DLONG next_z=(sum1_z/sum2_z)+0.5;
		cur_center->x=next_x;
		cur_center->y=next_y;
		cur_center->z=next_z;

	}
	return cur_center;
}


Gradient* gradient_node(Node* node,unsigned char* &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//calculate the gradient of single node
	V3DLONG radius=(V3DLONG)node->r;
	double sum=0;
	double intensity=0;
	V3DLONG node_number=0;
	
	Gradient* node_gradient=new Gradient();

	for(V3DLONG i=node->z-radius;i<=node->z+radius;i++)
	{
		if(i<0)
		{
			i=0;

		}else if(i>sz_z)
		{
			i=sz_z;
		}else if(i>=sz_z-1)
		{
			//continue;
			break;

		}
		for(V3DLONG j=node->y-radius;j<=node->y+radius;j++)
		{
			if(j<0)
			{
				j=0;

			}else if(j>sz_y)
			{
				j=sz_y;
			}
			for(V3DLONG k=node->x-radius;k<=node->x+radius;k++)
			{
				if(k<0)
				{
					k=0;

				}else if(k>sz_x)
				{
					k=sz_x;
				}
				sum+=img1d[GET_IND(k,j,i)];//calculate the sum intensity of all nodes
				node_number++;

			}
		}

	}
	intensity=sum/node_number;

	{
		node_gradient->gradient_x=(double)(img1d[GET_IND(node->x,node->y+radius,node->z)]-intensity+img1d[GET_IND(node->x+radius,node->y+radius,node->z)]-img1d[GET_IND(node->x+radius,node->y,node->z)])/2+0.0000001;
		node_gradient->gradient_y=(double)(intensity-img1d[GET_IND(node->x+radius,node->y,node->z)]+img1d[GET_IND(node->x,node->y+radius,node->z)]-img1d[GET_IND(node->x+radius,node->y+radius,node->z)])/2+0.0000001;
		node_gradient->gradient_z=(double)(intensity-img1d[GET_IND(node->x,node->y,node->z+radius)]+img1d[GET_IND(node->x,node->y+radius,node->z)]-img1d[GET_IND(node->x+radius,node->y+radius,node->z+radius)])/2+0.0000001;
	}
	return node_gradient;
	

}
QList<Gradient*> gradient(QList<Node*> nodes,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//calculate the gradient of a list of nodes
	QList<Gradient*> result_grad;
	printf("size:::%d\n",nodes.size());
	for(int i=0;i<nodes.size();i++)
	{
		Node *node=nodes.at(i);
		double radius=ceil(node->r);
		Gradient *grad_list=new Gradient();//result
		Gradient *grad=new Gradient [(2*radius+1)*(2*radius+1)*(2*radius+1)];//temp
		//printf("radius:::%lf\n",radius);
		int count=0;
		double grad_sum=0;
		double ax_sum=0;
		double ay_sum=0;
		double az_sum=0;
		double x_sum=0;
		double y_sum=0;
		double z_sum=0;
		for(V3DLONG i=node->z-radius;i<=node->z+radius;i++)
		{
			if(i<0)
			{
				i=0;

			}else if(i>sz_z)
			{
				i=sz_z;
			}else if(i>=sz_z-1)
			{
				//continue;
				break;

			}
			for(V3DLONG j=node->y-radius;j<=node->y+radius;j++) 
			{
				if(j<0)
				{
					j=0;

				}else if(j>sz_y)
				{
					j=sz_y;
				}
				for(V3DLONG k=node->x-radius;k<=node->x+radius;k++)
				{
					
					if(k<0)
					{
						k=0;

					}else if(k>sz_x)
					{
						k=sz_x;
					}
					if(img1d[GET_IND(k,j,i)]==0)
					{
						continue;
					}else if((k+1>sz_x)||(j+1>sz_y)||(i+1>sz_z))
					{
						continue;
					}
					//printf("2:::kji:::%ld   %ld   %ld   %d   %lf\n",k,j,i,count,radius);
					grad[count].gradient_x=(double)(img1d[GET_IND(k,j+1,i)]-img1d[GET_IND(k,j,i)]+img1d[GET_IND(k+1,j+1,i)]-img1d[GET_IND(k+1,j,i)])/2+0.0000001;
					//printf("%lf  \n",grad[count].gradient_x);
					grad[count].gradient_y=(double)(img1d[GET_IND(k,j,i)]-img1d[GET_IND(k+1,j,i)]+img1d[GET_IND(k,j+1,i)]-img1d[GET_IND(k+1,j+1,i)])/2+0.0000001;
					//printf("y:::%lf  \n",grad[count].gradient_y);
					//printf("%ld \n",GET_IND(k+1,j+1,i+1));
					//printf("%ld \n",GET_IND(k,j,i+1));
					//printf("   %lf \n",(double)img1d[GET_IND(k+1,j+1,i+1)]);
					//printf("   %lf \n",(double)img1d[GET_IND(k,j,i+1)]);
					
					grad[count].gradient_z=(double)(img1d[GET_IND(k,j,i)]-img1d[GET_IND(k,j,i+1)]+img1d[GET_IND(k,j+1,i)]-img1d[GET_IND(k+1,j+1,i+1)])/2+0.0000001;
					//printf("%lf  \n",grad[count].gradient_z);
					grad[count].gradient=(double)sqrt(grad[count].gradient_x*grad[count].gradient_x+grad[count].gradient_y*grad[count].gradient_y+grad[count].gradient_z*grad[count].gradient_z+0.5);//amplitude of gradient
					grad[count].angle_x=(double)atan(grad[count].gradient_y/sqrt(grad[count].gradient_x*grad[count].gradient_x+grad[count].gradient_z*grad[count].gradient_z+0.0000001))*57.3;//angle with x panel
					grad[count].angle_y=(double)atan(grad[count].gradient_z/sqrt(grad[count].gradient_x*grad[count].gradient_x+grad[count].gradient_y*grad[count].gradient_y+0.0000001))*57.3;//angle with y panel
					grad[count].angle_z=(double)atan(grad[count].gradient_x/sqrt(grad[count].gradient_y*grad[count].gradient_y+grad[count].gradient_z*grad[count].gradient_z+0.0000001))*57.3;//angle with z panel
					//fprintf(fp,"%lf   %lf   %lf   \n",angle_x[k+j*step_y+i*step_y*step_z],angle_y[k+j*step_y+i*step_y*step_z],angle_z[k+j*step_y+i*step_y*step_z]);
					//printf("%lf   %lf   %lf  %lf \n",grad[count].gradient,grad[count].angle_x,grad[count].angle_y,grad[count].angle_z);
					grad_sum+=grad[count].gradient;
					ax_sum+=grad[count].angle_x;
					ay_sum+=grad[count].angle_y;
					az_sum+=grad[count].angle_z;
					x_sum+=grad[count].gradient_x;
					y_sum+=grad[count].gradient_y;
					z_sum+=grad[count].gradient_z;
					
					count++;
					//printf("%lf   %lf   %lf  %lf %lf   %lf  %lf\n",grad_sum,ax_sum,ay_sum,az_sum,x_sum,y_sum,z_sum);
					//printf("1:::kji:::%ld   %ld   %ld   %d   %lf\n",k,j,i,count,radius);
				}
			}
		}
		//printf("count:::%d\n",count);
		grad_list->x=node->x;
		grad_list->y=node->y;
		grad_list->z=node->z;
		grad_list->gradient_x=x_sum/(count+0.0000001);
		grad_list->gradient_y=y_sum/(count+0.0000001);
		grad_list->gradient_z=z_sum/(count+0.0000001);
		grad_list->angle_x=ax_sum/(count+0.0000001);
		grad_list->angle_y=ay_sum/(count+0.0000001);
		grad_list->angle_z=az_sum/(count+0.0000001);
		grad_list->gradient=grad_sum/(count+0.0000001);
		printf("%lf   %lf   %lf\n",grad_list->angle_x,grad_list->angle_y,grad_list->angle_z);
		result_grad.append(grad_list);
		delete[] grad;

	}
	return result_grad;
}
void construct_tree_vn3(QMap<int,Node* > roots,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//combined the gradient and distance as weight of spanning to construct every roots
	double weight_d=1;
	double weight_g=1;
	double nSigma=0.4;
	QList<Node*> seeds;
	for(QMap<int,Node* >::iterator iter=roots.begin();iter!=roots.end();iter++)
	{
		Node* temp=iter.value();
		seeds.append(temp);

	}


	V3DLONG marknum = seeds.size();
	//	printf("111111111111111111111111111111111\n");
	QList<Gradient*> grad=gradient(seeds,img1d, sz_x, sz_y, sz_z);//calculate the gradient of every seed
	//printf("2222222222222222222222222222222\n");

	double** markEdge = new double*[marknum];//distance
	double** markEdge_g = new double*[marknum];//gradient
	double** markEdge_d=new double*[marknum];
	for(int i = 0; i < marknum; i++)
	{
		markEdge[i] = new double[marknum];
		markEdge_g[i]=new double[marknum];
		markEdge_d[i]=new double[marknum];
		//fprintf(debug_fp,"markEdge[i]:%lf\n",markEdge[i]);
	}

	double x1,y1,z1;
	double angle_x1,angle_y1,angle_z1;
	double sum=0;//maximum value of all distance between nodes

	for (int i=0;i<marknum;i++)
	{
		x1 = seeds.at(i)->x;
		y1 = seeds.at(i)->y;
		z1 = seeds.at(i)->z;
		for (int j=0;j<marknum;j++)
		{

			markEdge_d[i][j] = sqrt(double(x1-seeds.at(j)->x)*double(x1-seeds.at(j)->x) + double(y1-seeds.at(j)->y)*double(y1-seeds.at(j)->y) + double(z1-seeds.at(j)->z)*double(z1-seeds.at(j)->z));
			//fprintf(debug_fp,"markEdge[i][j]:%lf\n",markEdge[i][j]);

			/*	if(max<markEdge_d[i][j])
			{
			max=markEdge_d[i][j];

			}*/
			sum+=markEdge_d[i][j];
		}//calculating the weight between nodes using distance
		//Considering the weight which combine the distance and gradient using Gauss equation
		angle_x1=grad.at(i)->angle_x;
		angle_y1=grad.at(i)->angle_y;
		angle_z1=grad.at(i)->angle_z;
		for(int j=0;j<marknum;j++)
		{
			markEdge_g[i][j]=(sqrt((angle_x1*grad.at(j)->angle_x+angle_y1*grad.at(j)->angle_y+angle_z1*grad.at(j)->angle_z)/(sqrt(angle_x1*angle_x1+angle_y1*angle_y1+angle_z1*angle_z1)*sqrt(grad.at(j)->angle_x*grad.at(j)->angle_x+grad.at(j)->angle_y*grad.at(j)->angle_y+grad.at(j)->angle_z*grad.at(j)->angle_z)))+1)/2;

		}//calculating the weight between nodes using angle of gradient vector with x/y/z panel

	}
	for (int ii=0;ii<marknum;ii++)//normalizing operation
	{
		for(int jj=0;jj<marknum;jj++)
		{
			markEdge_d[ii][jj]=markEdge_d[ii][jj]/sum;
			markEdge[ii][jj]=exp((-1/2)*(weight_d*markEdge_d[ii][jj]*markEdge_d[ii][jj]+weight_g*markEdge_g[ii][jj]*markEdge_g[ii][jj])/(nSigma*nSigma))/(2*3.1415926*nSigma*nSigma);
			markEdge[ii][jj]=markEdge_d[ii][jj];
		}
	}





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
	listNeuron.append(S);
	hashNeuron.insert(S.n, listNeuron.size()-1);

	int* pi = new int[marknum];
	for(int i = 0; i< marknum;i++)
		pi[i] = 0;
	pi[0] = 1;
	int indexi,indexj;
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
			S.n 	= indexj+1;
			S.type 	= 7;
			S.x 	= seeds.at(indexj)->x;
			S.y 	= seeds.at(indexj)->y;
			S.z 	= seeds.at(indexj)->z;
			S.r 	= seeds.at(indexj)->r;
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

	QList<NeuronSWC> marker_MST_sorted;
	marker_MST_sorted.clear();
	if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
	{
	}

	prepare_write(marker_MST_sorted);

	if(markEdge) {delete []markEdge, markEdge = 0;}
	if(markEdge_d) {delete []markEdge_d, markEdge_d = 0;}
	if(markEdge_g) {delete []markEdge_g, markEdge_g = 0;}
	delete [] pi;
	seeds.clear();



}


void construct_tree_vn2(QMap<int,QList<Node*> > finalclass_node,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//based on the every roots which cluster pixel nodes, using spanning tree to construct subtrees with weights combined distance and gradient
	double weight_d=1;
	double weight_g=1;
	double nSigma=0.4;
    for(QMap<int,QList<Node*> >::iterator iter=finalclass_node.begin();iter!=finalclass_node.end();iter++)
	{
		{
			if(iter.value().size()<=1)
				continue;

			QList<Node*> seed=iter.value();
				
			QList<Node*> seeds=trim_nodes(seed, sz_x, sz_y, sz_z);
			
			V3DLONG marknum = seeds.size();
		//	printf("111111111111111111111111111111111\n");
			QList<Gradient*> grad=gradient(seeds,img1d, sz_x, sz_y, sz_z);//calculate the gradient of every seed
			//printf("2222222222222222222222222222222\n");

			double** markEdge = new double*[marknum];//distance
			double** markEdge_g = new double*[marknum];//gradient
			double** markEdge_d=new double*[marknum];
			for(int i = 0; i < marknum; i++)
			{
				markEdge[i] = new double[marknum];
				markEdge_g[i]=new double[marknum];
				markEdge_d[i]=new double[marknum];
				//fprintf(debug_fp,"markEdge[i]:%lf\n",markEdge[i]);
			}

			double x1,y1,z1;
			double angle_x1,angle_y1,angle_z1;
			double sum=0;//maximum value of all distance between nodes

			for (int i=0;i<marknum;i++)
			{
				x1 = seeds.at(i)->x;
				y1 = seeds.at(i)->y;
				z1 = seeds.at(i)->z;
				for (int j=0;j<marknum;j++)
				{
			
					markEdge_d[i][j] = sqrt(double(x1-seeds.at(j)->x)*double(x1-seeds.at(j)->x) + double(y1-seeds.at(j)->y)*double(y1-seeds.at(j)->y) + double(z1-seeds.at(j)->z)*double(z1-seeds.at(j)->z));
					//fprintf(debug_fp,"markEdge[i][j]:%lf\n",markEdge[i][j]);
					
				/*	if(max<markEdge_d[i][j])
					{
						max=markEdge_d[i][j];

					}*/
					sum+=markEdge_d[i][j];
				}//calculating the weight between nodes using distance
				//Considering the weight which combine the distance and gradient using Gauss equation
				angle_x1=grad.at(i)->angle_x;
				angle_y1=grad.at(i)->angle_y;
				angle_z1=grad.at(i)->angle_z;
				for(int j=0;j<marknum;j++)
				{
					markEdge_g[i][j]=(sqrt((angle_x1*grad.at(j)->angle_x+angle_y1*grad.at(j)->angle_y+angle_z1*grad.at(j)->angle_z)/(sqrt(angle_x1*angle_x1+angle_y1*angle_y1+angle_z1*angle_z1)*sqrt(grad.at(j)->angle_x*grad.at(j)->angle_x+grad.at(j)->angle_y*grad.at(j)->angle_y+grad.at(j)->angle_z*grad.at(j)->angle_z)))+1)/2;
					
				}//calculating the weight between nodes using angle of gradient vector with x/y/z panel
				
			}
			for (int ii=0;ii<marknum;ii++)//normalizing operation
			{
				for(int jj=0;jj<marknum;jj++)
				{
					markEdge_d[ii][jj]=markEdge_d[ii][jj]/sum;
					markEdge[ii][jj]=exp((-1/2)*(weight_d*markEdge_d[ii][jj]*markEdge_d[ii][jj]+weight_g*markEdge_g[ii][jj]*markEdge_g[ii][jj])/(nSigma*nSigma))/(2*3.1415926*nSigma*nSigma);
				}
			}

			

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
			listNeuron.append(S);
			hashNeuron.insert(S.n, listNeuron.size()-1);

			int* pi = new int[marknum];
			for(int i = 0; i< marknum;i++)
				pi[i] = 0;
			pi[0] = 1;
			int indexi,indexj;
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
					S.n 	= indexj+1;
					S.type 	= 7;
					S.x 	= seeds.at(indexj)->x;
					S.y 	= seeds.at(indexj)->y;
					S.z 	= seeds.at(indexj)->z;
					S.r 	= seeds.at(indexj)->r;
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

			QList<NeuronSWC> marker_MST_sorted;
			marker_MST_sorted.clear();
			if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
			{
			}

			prepare_write(marker_MST_sorted);

			if(markEdge) {delete []markEdge, markEdge = 0;}
			if(markEdge_d) {delete []markEdge_d, markEdge_d = 0;}
			if(markEdge_g) {delete []markEdge_g, markEdge_g = 0;}
			delete [] pi;
			seeds.clear();

		}
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
	printf("11111111111111111111111111111111111111111111\n");
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
	
	 // 4 - Set and show the Gauss filtering image in a new window
	v3dhandle curwin = callback.currentImageWindow();
	if(!curwin)
	{
		//QMessageBox::information(0, title, QObject::tr("No image is open."));
		//return -1;
	}
        v3dhandle newwin = callback.newImageWindow();
		if(! newwin)
	{
		//QMessageBox::information(0, title, QObject::tr("No image is open."));
		//return -1;
	}
		/*Image4DSimple *p4DImage = callback.getImage(curwin);
        p4DImage->setData(nGauss, sz_x, sz_y, sz_z, 1, p4DImage->getDatatype());//setData() will free the original memory automatically
        callback.setImage(newwin, p4DImage);
        callback.setImageName(newwin, QObject::tr("Gauss filtering"));*/
       //callback.updateImageWindow(newwin);
		//if(nData1) {delete []nData1,nData1 = 0;}
		//if(nData2) {delete []nData2,nData1 = 0;}
		if(Gkernal) {delete []Gkernal,Gkernal = 0;}
		
			
        return nGauss;


}



void construct_tree_vn4(QMap<int,Node* > roots,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	
	QList<Node*> result_shift;
	QList<Node*> shift_vector;
	QList<Node*> original_roots;

	for(QMap<int,Node* >::iterator iter=roots.begin();iter!=roots.end();iter++)
	{
		Node* temp=iter.value();
		Node* temp2=new Node();
		//temp=gradient_node(temp,sz_x,sz_y,sz_z);
		Node* temp1=found_direction(img1d,temp->x,temp->y,temp->z,sz_x,sz_y,sz_z,10);//some nodes cannot shift

		result_shift.append(temp1);
		//printf("%ld  %ld  %ld\n",temp1->x,temp1->y,temp1->z);
		V3DLONG A_x=temp1->x-temp->x;
		V3DLONG A_y=temp1->y-temp->y;
		V3DLONG A_z=temp1->z-temp->z;

		temp2->x=A_x;
		temp2->y=A_y;
		temp2->z=A_z;
		shift_vector.append(temp2);
		original_roots.append(temp);

	}

	V3DLONG marknum = original_roots.size();

	double** markEdge = new double*[marknum];
	double** markEdge_g = new double*[marknum];//shift vector
	double** markEdge_d=new double*[marknum];//distance
	for(int i = 0; i < marknum; i++)
	{
		markEdge[i] = new double[marknum];
		markEdge_g[i]=new double[marknum];
		markEdge_d[i]=new double[marknum];
		//fprintf(debug_fp,"markEdge[i]:%lf\n",markEdge[i]);
	}
//	printf("1111111111111111111111111\n");

	V3DLONG x1,y1,z1;
	V3DLONG A_x,A_y,A_z;
	double sum=0;//maximum value of all distance between nodes

	for (int i=0;i<marknum;i++)
	{
		x1 = original_roots.at(i)->x;
		y1 = original_roots.at(i)->y;
		z1 = original_roots.at(i)->z;
		for (int j=0;j<marknum;j++)
		{
			markEdge_d[i][j] = sqrt(double(x1-original_roots.at(j)->x)*double(x1-original_roots.at(j)->x) + double(y1-original_roots.at(j)->y)*double(y1-original_roots.at(j)->y) + double(z1-original_roots.at(j)->z)*double(z1-original_roots.at(j)->z));
			sum+=markEdge_d[i][j];
		}//calculating the weight between nodes using distance

		A_x=shift_vector.at(i)->x;
		A_y=shift_vector.at(i)->y;
		A_z=shift_vector.at(i)->z;
		for(int j=0;j<marknum;j++)
		{
			double q1=A_x*shift_vector.at(j)->x+A_y*shift_vector.at(j)->y+A_z*shift_vector.at(j)->z;
			double q2=shift_vector.at(j)->x*shift_vector.at(j)->x+shift_vector.at(j)->y*shift_vector.at(j)->y+shift_vector.at(j)->z*shift_vector.at(j)->z;
			double q3=A_x*A_x+A_y*A_y+A_z*A_z;
			markEdge_g[i][j]=sqrt(q1)/(sqrt(q2)*sqrt(q3)+0.0000001);
			//printf("%lf\n",q3);

		}//calculating the weight between nodes using angle of gradient vector with x/y/z panel

	}
	for (int ii=0;ii<marknum;ii++)//normalizing operation and calculate the weight
	{
		for(int jj=0;jj<marknum;jj++)
		{
			markEdge_d[ii][jj]=markEdge_d[ii][jj]/sum;
			//markEdge[ii][jj]=(markEdge_d[ii][jj])*markEdge_g[ii][jj];
			markEdge[ii][jj]=(markEdge_d[ii][jj]);
			//printf("%lf\n",markEdge[ii][jj]);
		}
	}
	//printf("1111111111111111111111111\n");




	//NeutronTree structure
	NeuronTree marker_MST;
	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();
//printf("1111111111111111111111111\n");
	//set node

	NeuronSWC S;
	S.n 	= 1;
	S.type 	= 3;
	S.x 	= original_roots.at(0)->x;
	S.y 	= original_roots.at(0)->y;
	S.z 	= original_roots.at(0)->z;
	S.r 	= original_roots.at(0)->r;
	S.pn 	= -1;
	listNeuron.append(S);
	hashNeuron.insert(S.n, listNeuron.size()-1);
//printf("1111111111111111111111111\n");
	int* pi = new int[marknum];
	for(int i = 0; i< marknum;i++)
		pi[i] = 0;
	pi[0] = 1;
	int indexi,indexj;
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
			S.n 	= indexj+1;
			S.type 	= 7;
			S.x 	= original_roots.at(indexj)->x;
			S.y 	= original_roots.at(indexj)->y;
			S.z 	= original_roots.at(indexj)->z;
			S.r 	= original_roots.at(indexj)->r;
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
//	printf("1111111111111111111111111\n");

	QList<NeuronSWC> marker_MST_sorted;
	marker_MST_sorted.clear();
	if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
	{
	}

	prepare_write(marker_MST_sorted);

	if(markEdge) {delete []markEdge, markEdge = 0;}
	if(markEdge_d) {delete []markEdge_d, markEdge_d = 0;}
	if(markEdge_g) {delete []markEdge_g, markEdge_g = 0;}
	delete [] pi;
	original_roots.clear();

}


void construct_tree_vn5(QMap<int,Node* > roots,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//combined the gradient and distance as weight of spanning to construct every roots
	QList<Node*> seeds;
	
	for(QMap<int,Node* >::iterator iter=roots.begin();iter!=roots.end();iter++)
	{
		Node* temp=iter.value();
		seeds.append(temp);

	}

	V3DLONG marknum = seeds.size();

	double** markEdge = new double*[marknum];//distance
	
	double** markEdge_d=new double*[marknum];
	for(int i = 0; i < marknum; i++)
	{
		markEdge[i] = new double[marknum];
	
		markEdge_d[i]=new double[marknum];
		//fprintf(debug_fp,"markEdge[i]:%lf\n",markEdge[i]);
	}
	double x1,y1,z1;
	double sum=0;//maximum value of all distance between nodes
	int count=0;

	for (int i=0;i<marknum;i++)
	{
		x1 = seeds.at(i)->x;
		y1 = seeds.at(i)->y;
		z1 = seeds.at(i)->z;
		for (int j=0;j<marknum;j++)
		{
			
			markEdge_d[i][j] = sqrt(double(x1-seeds.at(j)->x)*double(x1-seeds.at(j)->x) + double(y1-seeds.at(j)->y)*double(y1-seeds.at(j)->y) + double(z1-seeds.at(j)->z)*double(z1-seeds.at(j)->z));

			sum+=markEdge_d[i][j];
			count++;
		}//Calculating the weight between nodes using distance
		//Considering the weight which combine the distance and gradient using Gauss equation
		
	}

	double average_dis=sum/count;
	printf("start breath search!%lf\n",average_dis);
	markEdge=bf( root,img1d, average_dis,sz_x, sz_y, sz_z);

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
	listNeuron.append(S);
	hashNeuron.insert(S.n, listNeuron.size()-1);

	int* pi = new int[marknum];
	for(int i = 0; i< marknum;i++)
		pi[i] = 0;
	pi[0] = 1;
	int indexi,indexj;
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
			S.n 	= indexj+1;
			S.type 	= 7;
			S.x 	= seeds.at(indexj)->x;
			S.y 	= seeds.at(indexj)->y;
			S.z 	= seeds.at(indexj)->z;
			S.r 	= seeds.at(indexj)->r;
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

	QList<NeuronSWC> marker_MST_sorted;
	marker_MST_sorted.clear();
	if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
	{
	}

	prepare_write(marker_MST_sorted);

	if(markEdge) {delete []markEdge, markEdge = 0;}
	if(markEdge_d) {delete []markEdge_d, markEdge_d = 0;}
	
	delete [] pi;
	seeds.clear();



}



double** bf(QMap<int,Node* > roots,unsigned char * &img1d,double average_dis,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//found the shortest path between one node and other nodes using breadth first algorithm
	
	
	QQueue<double> queue_distance;//keep the unit distance of two nodes
	//QMultiMap<int,QMap<int,QList<Node*>*>*> result_double;//keep the shortest path between A node and other nodes
	
	V3DLONG up_limit;
	V3DLONG down_limit;
	int marknum=roots.size();

	double ** weight_result=new double*[marknum];
	for(int i = 0; i < marknum; i++)
	{
		weight_result[i] = new double[marknum];
	}

	int row=-1;
	QQueue<Node*> queue;
	QMap<V3DLONG,Node*> path_map;
	QMap<V3DLONG,bool> node_searched;
	QMap<V3DLONG,Node*> parent_path;
	QList<Node*> path_shortest;//delete it in the end of loop
	

	
	for(QMap<int,Node* >::iterator iter=roots.begin();iter!=roots.end();iter++)
	{
		int cloumn=-1;
		row++;
		//printf("key1::%d\n",iter.key());
		int key1=iter.key();
		
		
		//QList<bool> end_mark=new QList<bool>();
		//double stop_distance=0;//set this parameter so as to stop the while loop if a long distance between two nodes appeared
		Node* node1=iter.value();//start node
		V3DLONG start_x=node1->x;
		V3DLONG start_y=node1->y;
		V3DLONG start_z=node1->z;
		V3DLONG limit1=GET_IND(start_x,start_y,start_z);
		//Node* temp=new Node(start_x,start_y,start_z);
		for(QMap<int,Node* >::iterator iter1=roots.begin();iter1!=roots.end();iter1++)
		{
			cloumn++;
			//printf("key2::%d\n",iter1.key());
			int key2=iter1.key();
			
			Node* node2=iter1.value();//terminal node
			V3DLONG end_x=node2->x;
			V3DLONG end_y=node2->y;
			V3DLONG end_z=node2->z;
			V3DLONG limit2=GET_IND(end_x,end_y,end_z);
			
			queue.clear();
			QList<Node*> path;//keep the shortest path temporally;
			path.clear();
			
			path_map.clear();
			
			node_searched.clear();
			
			parent_path.clear();
			
			path_shortest.clear();
			bool flag=false;//mark the terminal condition which shows the program found the end node from start node
			
			//QMap<int,QList<Node*>*> *result_single=new QMap<int,QList<Node*>*>();
			//result_single->clear();//keep the new one clean

			double stop_distance=(double)(sqrt(double(node2->x-node1->x)*(node2->x-node1->x)+double(node2->y-node1->y)*(node2->y-node1->y)+double(node2->z-node1->z)*(node2->z-node1->z)));
			//printf("stop distance calculated:%lf\n",stop_distance);
			//if((stop_distance>=(average_dis/10))||stop_distance==0)
			if((stop_distance>=(7))||stop_distance==0)//stop_distance can be input in the dialogue
			{
				//printf("size:%d\n",roots.size());
				weight_result[row][cloumn]=stop_distance;
			//	printf("continue\n");
				continue;

			}else
			{

				if(limit1<=limit2)
				{
					up_limit=limit2;
					down_limit=limit1;
				}else
				{
					up_limit=limit1;
					down_limit=limit2;
				}

				//printf("%ld  %ld  %ld  %ld  %ld  %ld\n",start_x,start_y,start_z,end_x,end_y,end_z);

				V3DLONG node_number=(abs(start_x-end_x)+1)*(abs(start_y-end_y)+1)*(abs(start_z-end_z)+1);
				V3DLONG count=1;
				V3DLONG offset_x=start_x;
				V3DLONG offset_y=start_y;
				V3DLONG offset_z=start_z;
				V3DLONG offset_dis=0;

				queue.append(node1);

				//queue_intensity(img1d[GET_IND(start_x,start_y,start_z)]);
				//while((offset_x==end_x)&&(offset_y==end_y)&&(offset_z==end_z))
			//	printf("start while:%ld\n",node_number);
				/*if(node_number<=1)
				{
					weight_result[key1-1][key2-1]=stop_distance;
					continue;//two roots have the same location

				}else*/
				{
					//while((count<=node_number)&&(!(offset_x==end_x)&&(offset_y==end_y)&&(offset_z==end_z))&&(queue.size()!=0))
					while(!((offset_x==end_x)&&(offset_y==end_y)&&(offset_z==end_z))&&(queue.size()!=0))//here exists arguement!
					{//condition 1 gaurantee the number of searched node will not more than the nodes in the block,because maybe there is a gap between nodes
						//condition 2 gaurantee the loop will be stoped when the terminal node was found
						//condition 3 gaurantee the loop will be stoped when there is a gap between two nodes, it means that A node will never get to B node
						//printf("looping...\n");
						Node *temp1=queue.head();//get the first node in the queue every loop
						//node_searched[GET_IND(offset_x,offset_y,offset_z)]=false;//initialization
						//need to set a flag here means the node was searched
						for(int times=0;times<26;times++)
						{
							//printf("%lf",node_table[times].dist);
							offset_x=temp1->x+node_table[times].i;
							offset_y=temp1->y+node_table[times].j;
							offset_z=temp1->z+node_table[times].k;
							//offset_dis=node_table[times].dist;
							V3DLONG index=GET_IND(offset_x,offset_y,offset_z);

							if((index<down_limit)||(index>up_limit))//outrange
							{
							//	printf("%ld  %ld  %ld\n",index,down_limit,up_limit);
								continue;

							}else if(img1d[index]==0)//not connected ,also means no edge
							{
								count++;
							//	printf("%lf\n",(double) img1d[index]);
								continue;
							}else
							{

							//	printf("%d\n",node_searched[GET_IND(offset_x,offset_y,offset_z)]); 
							//considering other method about how to 
								if(node_searched[GET_IND(offset_x,offset_y,offset_z)]!=true)//if true, which  means the node was searched before
								{
									//printf("node_searched\n");
									parent_path.insert(GET_IND(offset_x,offset_y,offset_z),temp1);
									count++;//if count equals with node number, that mean all nodes were put in the queue
									Node* temp2=new Node(offset_x,offset_y,offset_z);//consider change it to intensity
									node_searched[GET_IND(offset_x,offset_y,offset_z)]=true;
									queue.append(temp2);
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
						//	printf("out \n");
						//record the distance, pixal intensity, waiting...20150511
						//consider how to stop when a long distance between two nodes, because the two nodes cannot ever be connected
						//familiar with the spanning tree algorithm
						//path.append(queue.head());//keep the first node in every loop so that we can record the path from start to end
						//path_map.insert(GET_IND(queue.head()->x,queue.head()->y,queue.head()->z),queue.head());
						//end_mark.append(flag);

						/*if(A_distance>stop_distance)
						{
						break;//loop will be stopped when the two nodes have a long distance
						//20150512,this idea cannot meet the running condition of spanning tree, give up

						}*/

						queue.dequeue();

					}

				}

				//printf("out while:%d\n",flag);

				
				if(flag)
				{
					//path_shortest=found_path(path,path_map,node1,node2,sz_x,sz_y,sz_z);// if get the terminal node, found the shortest path using backward method
					path_shortest=found_path_vn2(parent_path,node1,node2,sz_x,sz_y,sz_z);
					//printf("shortest path found\n");
					weight_result[row][cloumn]=distance_calculate(img1d,path_shortest,sz_x,sz_y,sz_z);
					//printf("calculated weight_result:%lf\n",weight_result[row][cloumn]);
					if(path_shortest.size()==0)
					{
						v3d_msg(QString("the shortest path is empty, program crash."));

					}

				}else
				{
					path_shortest.clear();//if not found the terminal node that means maybe there is a gap between two nodes and A node cannot ever get to B node
					weight_result[row][cloumn]=stop_distance;
				}

				//result_single->insert(key2,path_shortest);
				//result_double.insert(key1,result_single);
				//calculate weight_result[key1-1][key2-1] here 
				//printf("quit if\n");


			}
			queue.clear();
			//queue_distance.clear();
			path.clear();
			path_map.clear();
			node_searched.clear();
			path_shortest.clear();
			parent_path.clear();
			//delete path_shortest;
		}
	}

	//calculate markEdge matrix,found the shortest path between two nodes
	return weight_result;
}

QList<Node*> found_path_vn2( QMap<V3DLONG,Node*> path_map, Node* temp,Node* temp1,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//considering the distance and pixal intensity as weight so as to calculate the markEdge
	bool flag=false;
	QList<Node*> result;
	result.append(temp1);//temp1 represent the terminal node
	//printf("length is correct:%ld\n",current_node->x);
	V3DLONG index;

	V3DLONG current_x=result.first()->x;
	V3DLONG current_y=result.first()->y;
	V3DLONG current_z=result.first()->z;
	while(!((current_x==temp->x)&&(current_y==temp->y)&&(current_z==temp->z)))
	{
		//printf("looping1...\n");
		if(path_map.contains(GET_IND(current_x,current_y,current_z)))
		{
			//printf("contained,%ld   %ld   %ld\n",path_map[GET_IND(current_x,current_y,current_z)]->x,path_map[GET_IND(current_x,current_y,current_z)]->y,path_map[GET_IND(current_x,current_y,current_z)]->z);

		}
		V3DLONG x1=path_map[GET_IND(current_x,current_y,current_z)]->x;
		V3DLONG y1=path_map[GET_IND(current_x,current_y,current_z)]->y;
		V3DLONG z1=path_map[GET_IND(current_x,current_y,current_z)]->z;

		current_x=x1;
		current_y=y1;
		current_z=z1;
		//printf("looping2...\n");
		if(((current_x==temp->x)&&(current_y==temp->y)&&(current_z==temp->z)))
		{
			result.append(temp);
			break;
			

		}else
		{
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

int meanshift_plugin_vn4(V3DPluginCallback2 &callback, QWidget *parent,unsigned char* img1d,V3DLONG *in_sz, QString &image_name,bool bmenu)
{
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
	
	printf("### finding rootnode  ###\n");
	

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

	printf("###  cluster nodes using meanshift and connect subtrees by spanning tree   ###\n");

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
	

	merge_rootnode(Map_rootnode,img1d,sz_x,sz_y,sz_z);

	
	//merge_rootnode(Map_rootnode,img1d,sz_x,sz_y,sz_z);
	printf("###   denosing the image using Gauss method   ###\n");//这里作为第二次去噪，采用distance transform计算中心线
	img1d=Gauss_filter(img1d,nData1,sz_x,sz_y,sz_z,callback, parent);//for calculate the gradient of nodes better

	printf("###   denoising finished   ###\n");
	//construct_tree_vn2(finalclass_node, img1d,sz_x, sz_y, sz_z);
	
	//construct_tree_vn4(root, img1d,sz_x, sz_y, sz_z);//call bf method to find the shortest path between nodes
	construct_tree_vn5(root,img1d,sz_x,sz_y,sz_z);
		
        QString outswc_file = image_name + "_meanshift.swc";
        printSWCByQList_Neuron(result_list,outswc_file.toStdString().c_str());
		printSWCByQList_Neuron(result_list,"D:\\result\\result11.swc");

        V3DPluginArgItem arg;
        V3DPluginArgList input_sort;
        V3DPluginArgList output;

        arg.type = "random";std::vector<char*> arg_input_sort;
        std:: string fileName_Qstring(outswc_file.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
        arg_input_sort.push_back(fileName_string);
        arg.p = (void *) & arg_input_sort; input_sort<< arg;
        arg.type = "random";std::vector<char*> arg_sort_para; arg.p = (void *) & arg_sort_para; input_sort << arg;
        arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(fileName_string); arg.p = (void *) & arg_output; output<< arg;

        QString full_plugin_name_sort = "sort_neuron_swc"; 
        QString func_name_sort = "sort_swc";
        callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

        vector<MyMarker*> temp_out_swc = readSWC_file(outswc_file.toStdString());
        saveSWC_file(outswc_file.toStdString(), temp_out_swc);
        v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outswc_file.toStdString().c_str()),bmenu);
		if(nData1) {delete []nData1,nData1 = 0;}
        delete []flag;
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

	
     //printSwcByMap(root,"D:\\result\\finalroot.swc");


}

void construct_tree(QMap<int,QList<Node*> > finalclass_node,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
    for(QMap<int,QList<Node*> >::iterator iter=finalclass_node.begin();iter!=finalclass_node.end();iter++)
	{
		{
			if(iter.value().size()<=1)
				continue;

			QList<Node*> seed=iter.value();
				
			QList<Node*> seeds=trim_nodes(seed, sz_x, sz_y, sz_z);
			
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
			}

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
			listNeuron.append(S);
			hashNeuron.insert(S.n, listNeuron.size()-1);

			int* pi = new int[marknum];
			for(int i = 0; i< marknum;i++)
				pi[i] = 0;
			pi[0] = 1;
			int indexi,indexj;
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
					S.n 	= indexj+1;
					S.type 	= 7;
					S.x 	= seeds.at(indexj)->x;
					S.y 	= seeds.at(indexj)->y;
					S.z 	= seeds.at(indexj)->z;
					S.r 	= seeds.at(indexj)->r;
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

			QList<NeuronSWC> marker_MST_sorted;
			marker_MST_sorted.clear();
			if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
			{
			}

			prepare_write(marker_MST_sorted);

			if(markEdge) {delete []markEdge, markEdge = 0;}
			delete [] pi;
			seeds.clear();

		}
	}
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
	//ﾖﾘﾐﾂﾐｴﾒｻｸ盪ｹ｣ｬｰﾑﾋﾐｵ羝ｼｼﾓｽ･
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


