/* example_func.cpp
* This file contains the functions used in plugin domenu and dufunc, you can use it as a demo.
* 2012-02-13 : by Yinan Wan
*/

#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "meanshift_func.h"
#include "D:\Vaa3D\v3d_main\neurontracing_vn21\app2/heap.h"
#include "D:\Vaa3D\v3d_main\neuron_editing\v_neuronswc.h"
#include <vector>
#include <iostream>

using namespace std;

/* function used in DOMENU typically takes 2 inputs:
* "callback" - provide information from the plugin interface, and 
* "parent"   - the parent widget of the Vaa3D main window
*/
const QString title="Image Thresholding";

#define GET_IND(x,y,z) (x) + (y) * sz_x + (z) * sz_x* sz_y
#define GET_X(ind) ind % sz_x
#define GET_Y(ind) (ind / sz_x) % sz_y
#define GET_Z(ind) (ind / (sz_x*sz_y)) % sz_z
#define ABS(a) (a) > 0 ? (a) : -(a)

int *flag;//用于标记每个像素点是否被MeanShift过；
QList<Node*> final_nodeList;
QList<Node*> final_nodeList1;//用于在初始化的时候找到概率密度最大的点
QMap<V3DLONG,QList<Node*>> Map_finalnode_list;//根节点二维坐标对应根节点本身
QMap<V3DLONG,Node*> Map_allnodes;
QMap<V3DLONG,int> Map_finalnode;
vector<QList<Node*>> v_List;//用于保存每个像素点找到的路径
QMap<V3DLONG,QList<Node*>> root_class;//根节点二维坐标对应和根节点类别相同的点
QList<Node*> temp_list;//for testing


void printSWCByVector_List(vector<QList<Node*>> vector_List,char* path)
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
void printSwcByMap(QMap<V3DLONG,Node*> nodeMap,char* path)
{
	FILE * fp = fopen(path, "wt");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");


	for(QMap<V3DLONG,Node*>::iterator iter = nodeMap.begin(); iter != nodeMap.end(); iter++)
	{
		Node* elem = iter.value();
		fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
			elem->number, 1, 1.0 * elem->x, 1.0 * elem->y, 1.0 * elem->z, 1.0 * elem->r, elem->parent == -2 ? -1 : elem->parent);
	}


	fclose(fp);

	return;
}

void printSWCByMap_List(QList<QMap<V3DLONG,QList<Node*>>>  List,char * filename)
{//这个函数是将QMap<V3DLONG,QList<Node*>>类型写到SWC文件中
	FILE * fp = fopen(filename, "a");//追加
	if (!fp) return;
	V3DLONG number=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for(int j=0;j<List.size();j++)
	{
		QMap<V3DLONG,QList<Node*>> Map=List.at(j);
		for(QMap<V3DLONG,QList<Node*>>::iterator iter2 =Map.begin(); iter2 != Map.end();  iter2++)
		{
			QList<Node*> List2=iter2.value();
			for(int i=0;i<List2.size();i++)
			{
				Node* elem = List2.at(i);
				fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
					number, 1,  elem->x,  elem->y,  elem->z, 1.0, -1);
				number++;

			}
		}
	}
	fclose(fp);

	return;
}


int meanshift_plugin_vn2(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if(!curwin)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	Image4DSimple *p4d = callback.getImage(curwin);
	V3DLONG sz_x = p4d->getXDim();
	V3DLONG sz_y = p4d->getYDim();
	V3DLONG sz_z = p4d->getZDim();
	unsigned char* img1d = p4d->getRawDataAtChannel(1);
	flag=new int [sz_x*sz_y*sz_z];
	
	QList<Node*> temp1;//用于临时保存像素点找到的路径，完成后都放到MapList里面；这里不能定义指针，定义指针的话会出错，因为指针指向的是地址，地址改变了，内容也会变；
	V3DLONG r=10;
	V3DLONG count=0;
	int times=100;
	int num_mark=1;//用于在初始化的时候对种子点进行分类标号
	printf("### Initializing...  ###\n");
	
	for(V3DLONG ii=0;ii<sz_x;ii++)
	{
		for(V3DLONG jj=0;jj<sz_y;jj++)
		{
			for(V3DLONG kk=0;kk<sz_z;kk++)
			{
				V3DLONG ind=GET_IND(ii,jj,kk);//从三维坐标转换成二维坐标，通过二维坐标获得像素强度
				//flag[ind]=0;
				if(img1d[ind]==0)//像素强度为0的坐标点不予考虑，只对具有像素强度的点进行meanshift
					continue;
				if(!found_final(img1d,ii,jj,kk,sz_x, sz_y, sz_z, r))//没有动
				{
					Node* temp=new Node(ii,jj,kk);
					temp->class_mark=num_mark;
					temp->parent=-1;//根节点标记为-1
					temp->number=0;//每个根节点的序号标记为0
					final_nodeList1.append(temp);
					Map_finalnode_list.insert(ind,temp);
					Map_finalnode.insert(ind,num_mark);//给根节点分配类别标号
					num_mark++;

				}
			}
		}
	}
	printSwcByList( final_nodeList1,"D:\\meanshift_vn3_finalnode.swc");
	printf("###  start meanshift   ###\n");
	for(V3DLONG i=0;i<sz_x;i++)
	{
		for(V3DLONG j=0;j<sz_y;j++)
		{
			for(V3DLONG k=0;k<sz_z;k++)
			{
				V3DLONG ind=GET_IND(i,j,k);//从三维坐标转换成二维坐标，通过二维坐标获得像素强度
				if(img1d[ind]==0)//像素强度为0的坐标点不予考虑，只对具有像素强度的点进行meanshift
					continue;
			/*	if(img1d[ind]<30)//像素强度超过某一阈值的坐标点不予考虑，只对具有像素强度的点进行meanshift
					continue;*/
				if(flag[ind]==1)
				{
					//printf("被标记\n");
					//printf("%d",flag[ind]);
					continue;//被处理过的像素点不再被处理

				}
				
					
				//if(count<=3)
				//{
					//printf("0:::%ld   %ld   %ld   %d\n",i,j,k,flag[ind]);
					//每次meanshift过后，针对每个像素点都会得到一路路径通往一个点，这个点就是概率密度最大的点
					//所以，应该得到一个Map，int表示概率密度最大点的二维序号，QList表示这条路径
					//需要注意的是：已经被包含在某一条路径上的像素点应该有所标记，目的是保证被处理过的点不再进行MeanShift计算
					
					temp1=meanshift_vn4(img1d,flag,i,j,k,sz_x,sz_y,sz_z,r,times);//r表示这个球的半径,times表示迭代的次数
					v_List.push_back(temp1);
					//count++;

				//}

			}

		}

	}
	//上一步出来的额结果看上去存在过分割的现象
	//接下来需要对每个nodeList构建父与子的关系，甚至会对nodeList之间构建父与子的关系
	//想法1.先合并根节点，以面积最大的根节点作为父节点，每个根节点都会有一群像素点作为它的子节点，考虑采用结构张量的权重计算方式
	//保留生长方向和根节点一致的像素点
	//哪些点属于哪些根节点？
	QMap<V3DLONG,vector<QList<Node*>>> MapList;
	QList<Node*> test_list;//test_list里面存放的是分类好了的点
	QList<Node*> test;//test里面存放的是计算好半径，并且节点被合并的点
	vector<QList<Node*>> v_test;//存放根据根节点分类好的test
	int ii=0;
	for(QMap<V3DLONG,Node*>::iterator iter =Map_finalnode_list.begin(); iter != Map_finalnode_list.end(); iter++)
	{//依次根据每个根节点，对其他像素点按照每个根节点的类别进行聚类
		test_list.clear();
		test.clear();
		Node* elem = iter.value();
		int num_node=0;
		if(ii<=15)
		{
			int mark=elem->class_mark;
			//printf("%d\n",mark);
			for(int jj=0;jj<v_List.size();jj++)
			{
				for(int kk=0;kk<v_List.at(jj).size();kk++)
				{
					
					if((mark==v_List.at(jj).at(kk)->class_mark)&&((elem->x!=v_List.at(jj).at(kk)->x)||((elem->y!=v_List.at(jj).at(kk)->y))||((elem->z!=v_List.at(jj).at(kk)->z))))//保证找到的点不是根节点
					{
						num_node=num_node+1;
						v_List.at(jj).at(kk)->number=num_node;//对每个像素点编号
						test_list.append(v_List.at(jj).at(kk));//对test_list中包含的父节点的信息建立树结构，打印到SWC文件中
						//可以对每个根节点建立一个QMap<V3DLONG,QList<Node*>>,然后针对每个Map建立树结构

					}else if(((elem->x==v_List.at(jj).at(kk)->x)&&((elem->y==v_List.at(jj).at(kk)->y))&&((elem->z==v_List.at(jj).at(kk)->z))))
					{
						
						v_List.at(jj).at(kk)->number=0;//根节点标号为0，父节点标号为-1
						v_List.at(jj).at(kk)->parent=-1;
						test_list.append(v_List.at(jj).at(kk));
						printf("%ld  %ld  %ld\n",v_List.at(jj).at(kk)->x,v_List.at(jj).at(kk)->y,v_List.at(jj).at(kk)->z);

					}

				}
			}
		
		V3DLONG index=GET_IND(elem->x,elem->y,elem->z);
		root_class[index]=test_list;
		//先依次计算所有点的半径，合并某些点，将剩下的点用最小生成树的方法构造一棵树
		test=enlarge_radiusof_allnodes_xy(img1d,root_class[index],sz_x,sz_y,sz_z);
		v_test.push_back(test);
		//enlarge_radiusof_root_xy(img1d,root_class[index],elem,sz_x,sz_y,sz_z);
	//	construct_tree(root_class[index],elem,sz_x,sz_y,sz_z);
	//	QList<Node*> test=post_process(root_class[index]);//删掉那些直接到根节点，又没有孩子的叶子节点
		//printSwcByList( root_class[index],"D:\\meanshift_test_finalnode.swc");
		//printSwcByList( test,"D:\\meanshift_test_finalnode.swc");
		}
		ii++;		
	}
	//上一步的结果是将所有点按照根节点的类别分门别类的放到了root_class里面，现在是依次按照每个根节点和点的父与子的关系构建树

	//printSWCByMap_List(MapList,"D:\\meanshift_vn2.swc");
	printSwcByList( temp_list,"D:\\mark_0_.swc");
	//printSWCByVector_List(v_List,"D:\\meanshift_vn2.swc");
	printSWCByVector_List(v_test,"D:\\meanshift_vn2.swc");
	printSwcByList( final_nodeList,"D:\\meanshift_vn2_finalnode.swc");
	delete []flag;
	

}

QList<Node*> enlarge_radiusof_allnodes_xy(unsigned char * &img1d,QList<Node*> &class_List,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	QList<Node*> result;
	for(int i=0;i<class_List.size();i++)
	{
		
		QQueue<Node*> *queue=new QQueue<Node*>();
		
		queue->append(class_List.at(i));
		//printf("%d\n",queue->size());
		while(!(queue->isEmpty()))//基于根节点以1为步长，一周一周往外扩，把范围内的点都加到队列里面
		{
			int allnodes=0;
		int back_nodes=0;
		int max=0;//找最大半径
		double threshold=30;

			Node* head=queue->head();
			Node* up=new Node(head->x,head->y-1,head->z);
			Node* down=new Node(head->x,head->y+1,head->z);
			Node* left=new Node(head->x-1,head->y,head->z);
			Node* right=new Node(head->x+1,head->y,head->z);
			if(!queue->contains(up))//队列里面没有包括该节点
			{
				allnodes++;
				//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
				if(img1d[GET_IND(up->x,up->y,up->z)]<=30)
				{
					back_nodes++;
				}

				queue->enqueue(up);

			}

			if(!queue->contains(down))
			{
				allnodes++;
				//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
				if(img1d[GET_IND(down->x,down->y,down->z)]<=30)
				{
					back_nodes++;
				}

				queue->enqueue(down);

			}

			if(!queue->contains(left))
			{
				allnodes++;
				//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
				if(img1d[GET_IND(left->x,left->y,left->z)]<=30)
				{
					back_nodes++;
				}

				queue->enqueue(left);

			}

			if(!queue->contains(right))
			{
				allnodes++;
				//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
				if(img1d[GET_IND(right->x,right->y,right->z)]<=30)
				{
					back_nodes++;
				}

				queue->enqueue(right);

			}
			//printf("size:::%d   \n",queue->size());
			//printf("%d   %d\n",allnodes,back_nodes);

			double up_r=(double)sqrt((double)(class_List.at(i)->x-up->x)*(class_List.at(i)->x-up->x)+(double)(class_List.at(i)->y-up->y)*(class_List.at(i)->y-up->y)+(double)(class_List.at(i)->z-up->z)*(class_List.at(i)->z-up->z));
			double down_r=(double)sqrt((double)(class_List.at(i)->x-down->x)*(class_List.at(i)->x-down->x)+(double)(class_List.at(i)->y-down->y)*(class_List.at(i)->y-down->y)+(double)(class_List.at(i)->z-down->z)*(class_List.at(i)->z-down->z));
			double left_r=(double)sqrt((double)(class_List.at(i)->x-left->x)*(class_List.at(i)->x-left->x)+(double)(class_List.at(i)->y-left->y)*(class_List.at(i)->y-left->y)+(double)(class_List.at(i)->z-left->z)*(class_List.at(i)->z-left->z));
			double right_r=(double)sqrt((double)(class_List.at(i)->x-right->x)*(class_List.at(i)->x-right->x)+(double)(class_List.at(i)->y-right->y)*(class_List.at(i)->y-right->y)+(double)(class_List.at(i)->z-right->z)*(class_List.at(i)->z-right->z));

			max=up_r>down_r?up_r:down_r;
			max=max>left_r?max:left_r;
			max=max>right_r?max:right_r;



			float per=(float)back_nodes/(float)allnodes;

			if(per>0.1)
			{
				//root->r=(up_r+down_r+left_r+right_r)/4;
				class_List.at(i)->r=max;
				//printf("per:::%f   r:::%lf\n",per,class_List.at(i)->r);
				break;

			}

			//出队
			queue->dequeue();

		}
		queue->clear();

	}
	//上个for循环计算了每个点的半径，下面删除相应的点
	for(int j=0;j<class_List.size();j++)
	{
		Node* temp=class_List.at(j);//依次取出每个点，判断该点是否被其他点覆盖或者交叉很大，如果是，则删除这个点，否则保留
		bool flag_con=true;
		for(int k=j+1;k<class_List.size();k++)
		{
			if(temp->r<=class_List.at(k)->r)//这样是为了避免大球被删掉的情况
			{
				double dis=(double)sqrt((double)(class_List.at(k)->x-temp->x)*(class_List.at(k)->x-temp->x)+(double)(class_List.at(k)->y-temp->y)*(class_List.at(k)->y-temp->y)+(double)(class_List.at(k)->z-temp->z)*(class_List.at(k)->z-temp->z));//两点间的距离代表圆心的距离
				double sum_r=abs(temp->r+class_List.at(k)->r);
				double per=dis/sum_r;

				if(per<1.0)//球心的距离除以两球半径之和，结果大于0.8说明两球相交的不厉害或者远离；反之，说明该球和别的球相交，甚至被覆盖；
				{//判断temp是否被其他的点包含
					//printf("%lf\n",per);
					flag_con=false;
					break;
				}
			}		
		}
		if(flag_con)//如果该点没有出现被其他球覆盖的情况，则把该点放到一个QList里面
		{
			result.append(temp);

		}
	}
	return result;
}
void enlarge_radiusof_single_node_xy(unsigned char * &img1d,Node * &node,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	//基于xy平面4领域的半径增长
	int allnodes=0;
	int back_nodes=0;
	int max=0;//找最大半径
	double threshold=30;
	QQueue<Node*> *queue=new QQueue<Node*>();
	queue->append(node);
	while(!(queue->isEmpty()))//基于根节点以1为步长，一周一周往外扩，把范围内的点都加到队列里面
	{
		
		Node* head=queue->head();
		Node* up=new Node(head->x,head->y-1,head->z);
		Node* down=new Node(head->x,head->y+1,head->z);
		Node* left=new Node(head->x-1,head->y,head->z);
		Node* right=new Node(head->x+1,head->y,head->z);
		if(!queue->contains(up))//队列里面没有包括该节点
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(up->x,up->y,up->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(up);

		}
		
		if(!queue->contains(down))
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(down->x,down->y,down->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(down);

		}
		
		if(!queue->contains(left))
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(left->x,left->y,left->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(left);

		}
		
		if(!queue->contains(right))
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(right->x,right->y,right->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(right);

		}
		printf("size:::%d   \n",queue->size());
		printf("%d   %d\n",allnodes,back_nodes);

		double up_r=(double)sqrt((double)(node->x-up->x)*(node->x-up->x)+(double)(node->y-up->y)*(node->y-up->y)+(double)(node->z-up->z)*(node->z-up->z));
		double down_r=(double)sqrt((double)(node->x-down->x)*(node->x-down->x)+(double)(node->y-down->y)*(node->y-down->y)+(double)(node->z-down->z)*(node->z-down->z));
		double left_r=(double)sqrt((double)(node->x-left->x)*(node->x-left->x)+(double)(node->y-left->y)*(node->y-left->y)+(double)(node->z-left->z)*(node->z-left->z));
		double right_r=(double)sqrt((double)(node->x-right->x)*(node->x-right->x)+(double)(node->y-right->y)*(node->y-right->y)+(double)(node->z-right->z)*(node->z-right->z));

		max=up_r>down_r?up_r:down_r;
		max=max>left_r?max:left_r;
		max=max>right_r?max:right_r;

		

		float per=(float)back_nodes/(float)allnodes;
		
		if(per>0.1)
		{
			//root->r=(up_r+down_r+left_r+right_r)/4;
			node->r=max;
			printf("per:::%f   r:::%lf\n",per,node->r);
			break;

		}
		
		//出队
		queue->dequeue();

	}
delete queue;
delete up;
delete down;
delete left;
delete right;

}
void enlarge_radiusof_root_xy(unsigned char * &img1d,QList<Node*> &class_List,Node * &root,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//基于xy平面4领域的半径增长
	int allnodes=0;
	int back_nodes=0;
	int max=0;//找最大半径
	double threshold=30;
	QQueue<Node*> *queue=new QQueue<Node*>();
	queue->append(root);
	while(!(queue->isEmpty()))//基于根节点以1为步长，一周一周往外扩，把范围内的点都加到队列里面
	{
		
		Node* head=queue->head();
		Node* up=new Node(head->x,head->y-1,head->z);
		Node* down=new Node(head->x,head->y+1,head->z);
		Node* left=new Node(head->x-1,head->y,head->z);
		Node* right=new Node(head->x+1,head->y,head->z);
		if(!queue->contains(up))//队列里面没有包括该节点
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(up->x,up->y,up->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(up);

		}
		
		if(!queue->contains(down))
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(down->x,down->y,down->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(down);

		}
		
		if(!queue->contains(left))
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(left->x,left->y,left->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(left);

		}
		
		if(!queue->contains(right))
		{
			allnodes++;
			//printf("%lf\n",img1d[GET_IND(right->x,right->y,right->z)]);
			if(img1d[GET_IND(right->x,right->y,right->z)]<=30)
			{
				back_nodes++;
			}

			queue->enqueue(right);

		}
		printf("size:::%d   \n",queue->size());
		printf("%d   %d\n",allnodes,back_nodes);

		double up_r=(double)sqrt((double)(root->x-up->x)*(root->x-up->x)+(double)(root->y-up->y)*(root->y-up->y)+(double)(root->z-up->z)*(root->z-up->z));
		double down_r=(double)sqrt((double)(root->x-down->x)*(root->x-down->x)+(double)(root->y-down->y)*(root->y-down->y)+(double)(root->z-down->z)*(root->z-down->z));
		double left_r=(double)sqrt((double)(root->x-left->x)*(root->x-left->x)+(double)(root->y-left->y)*(root->y-left->y)+(double)(root->z-left->z)*(root->z-left->z));
		double right_r=(double)sqrt((double)(root->x-right->x)*(root->x-right->x)+(double)(root->y-right->y)*(root->y-right->y)+(double)(root->z-right->z)*(root->z-right->z));

		max=up_r>down_r?up_r:down_r;
		max=max>left_r?max:left_r;
		max=max>right_r?max:right_r;

		

		float per=(float)back_nodes/(float)allnodes;
		
		if(per>0.1)
		{
			//root->r=(up_r+down_r+left_r+right_r)/4;
			root->r=max;
			printf("per:::%f   r:::%lf\n",per,root->r);
			break;

		}
		
		//出队
		queue->dequeue();

	}

}
void construct_tree(QList<Node*> &class_List,Node *root,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	int num;
	for(int i=0;i<class_List.size();i++)
	{//这个循环应该不包括种子点
		//Node* temp=class_List.at(i);
		
		if (class_List.at(i)->parent==-1)
		{
			class_List.at(i)->r=root->r;
			
			printf("found root,r=%lf   number=%ld\n",class_List.at(i)->r,class_List.at(i)->number);
			continue;

		}
		
		V3DLONG parent_x=GET_X(class_List.at(i)->parent);
		V3DLONG parent_y=GET_Y(class_List.at(i)->parent);
		V3DLONG parent_z=GET_Z(class_List.at(i)->parent);
		

		for(int j=0;j<class_List.size();j++)
		{
			if((parent_x==class_List.at(j)->x)&&(parent_y==class_List.at(j)->y)&&(parent_z==class_List.at(j)->z))
			{
				num=class_List.at(j)->number;
				break;
			}
		}
		class_List.at(i)->r=0.5;
		class_List.at(i)->parent=num;
	}

}

QList<Node*> post_process(QList<Node*> List)
{
	QList<Node*> temp;
	int num_parent;
	bool flag_child;
	for(int i=0;i<List.size();i++)
	{
		if(List.at(i)->parent==-1)
		{
			num_parent=List.at(i)->number;
			printf("%ld\n",num_parent);
			break;

		}
	}

	for(int j=0;j<List.size();j++)
	{
		Node *temp1=new Node();

		temp1=List.at(j);
		if(temp1->parent==num_parent)//父节点是根节点
		{
			for(int k=0;k<List.size();k++)
			{
				if(List.at(k)->parent==temp1->number)//判断该节点是否有孩子
				{
					
						temp.append(temp1);
						printf("1111111111111111111111");

				}
			}

		}else
		{
			temp.append(temp1);

		}

	}
	return temp;

}
//MeanShift算法中定义的高斯核函数
#define cal_core(cur,center,radius) exp(-0.1*(abs(cur-center)))

double cal_weight(V3DLONG curi,V3DLONG curj,V3DLONG curk, V3DLONG x,V3DLONG y,V3DLONG z,double inte_nd,double inte_cen,V3DLONG r)//MeanShift算法中计算点与原点之间的权值，离原点越近的权值越大，像素和原点越近的权值越大
{
	//double r=100;
	double I=255;//像素最大值
	double weight=0;
	double distance=(curi-x)/r*(curi-x)/r+(curj-y)/r*(curj-y)/r+(curk-z)/r*(curk-z)/r;
	double inten_sim=(inte_nd-inte_cen)/I*(inte_nd-inte_cen)/I;
	weight=exp(-1.0*sqrt(2*inten_sim+distance));
	//printf("%lf  %lf  %lf \n",distance,inten_sim,weight);
	return weight;
}

bool found_final(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r)
{
	double intensity=img1d[GET_IND(x,y,z)];
	double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
	//定义范围，以半径为r的立方区域进行移动
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
					{//圆心点需要跟这个区域中的每个点都计算权值

						if(GET_IND(i,j,k)==GET_IND(x,y,z)) continue;//圆心点不计算
						if(img1d[GET_IND(i,j,k)]==0) continue;//像素为0的点不计算
						double cur_intensity=img1d[GET_IND(i,j,k)];
						w=cal_weight(i,j,k,x,y,z,cur_intensity,intensity,r);//计算区域中每个点和圆心的权重，主要是基于像素强度和距离

						double core_z=cal_core(k,z,r);//计算核函数，z坐标
						sum1_z+=core_z*w*k;//计算Z轴坐标，累加后作为MeanShift主要计算公式的分子
						sum2_z+=core_z*w;//累加后作为MeanShift主要计算公式的分母
						//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

						double core_y=cal_core(j,y,r);//y轴
						sum1_y+=core_y*w*j;
						sum2_y+=core_y*w;

						double core_x=cal_core(i,x,r);//x轴
						sum1_x+=core_x*w*i;
						sum2_x+=core_x*w;
						//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

					}
				}
			}
			
			V3DLONG next_x=(sum1_x/sum2_x)+0.5;
			V3DLONG next_y=(sum1_y/sum2_y)+0.5;
			V3DLONG next_z=(sum1_z/sum2_z)+0.5;
			if((next_x==x)&&(next_y==y)&&(next_z==z))
			{
				return false;//没有动

			}else
			{
				return true;//有移动

			}


}

int found_finalnode_mark(V3DLONG x,V3DLONG y,V3DLONG z,QList<Node*> final_List)
{
	int result=0;
	for(int i=0;i<final_List.size();i++)
	{
		Node* temp=final_List.at(i);
		if((x==temp->x)&&(y==temp->y)&&(z==temp->z))
		{
			result=temp->class_mark;
		}

	}
	if(result==0)
	printf("not found the final node,something wrong!!!\n");
	return result;


}

int found_nodefrom_VectorList(V3DLONG x,V3DLONG y,V3DLONG z,vector<QList<Node*>> vector_List)
{
	int result=0;
	for(int i=0;i<vector_List.size();i++)
	{
		for(int j=0;j<vector_List.at(i).size();j++)
		{
			Node* temp=vector_List.at(i).at(j);
			//printf("%d\n",temp->class_mark);
			if((x==temp->x)&&(y==temp->y)&&(z==temp->z))
			{
				result=temp->class_mark;
				//printf("not found the final node in the Vector,something wrong!!!\n");
			}
		}
	}
	
	if(result==0){

	//printf("not found the final node in the Vector,something wrong!!!\n");
	}
	return result;
}

void print_nodeList_mark(QList<Node*> nodeList)
{
	
	for(int i=0;i<nodeList.size();i++)
	{
		Node* temp=nodeList.at(i);
		if(temp->class_mark==0)
		{
		printf("testing:::%d\n",temp->class_mark);
		temp_list.append(temp);
		}

	}



}

QList<Node*> meanshift_vn4(unsigned char * &img1d,int *flag,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration)
{//和vn3相比，整理了vn3相关的逻辑
	int iter=0;
	int same_times=0;
	int situation=0;//用于观察找到的最终点属于哪种类型

	QList<Node*> nodeList;
	nodeList.clear();//每次都clear
	Node *cur_center=new Node(x,y,z);//圆的中心点，记录圆中心点的三维坐标
	double intensity=img1d[GET_IND(x,y,z)];//通过圆中心的三维坐标计算它的信号强度
	cur_center->intensity=intensity;
	
	while(1)
	{
		if(iter==iteration)
		{
			break;

		}else
		{
			double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
			//定义范围，以半径为r的立方区域进行移动
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
					{//圆心点需要跟这个区域中的每个点都计算权值

						if(GET_IND(i,j,k)==GET_IND(cur_center->x,cur_center->y,cur_center->z)) continue;//圆心点不计算
						if(img1d[GET_IND(i,j,k)]==0) continue;//像素为0的点不计算
						double cur_intensity=img1d[GET_IND(i,j,k)];
						w=cal_weight(i,j,k,cur_center->x,cur_center->y,cur_center->z,cur_intensity,intensity,r);//计算区域中每个点和圆心的权重，主要是基于像素强度和距离

						double core_z=cal_core(k,cur_center->z,r);//计算核函数，z坐标
						sum1_z+=core_z*w*k;//计算Z轴坐标，累加后作为MeanShift主要计算公式的分子
						sum2_z+=core_z*w;//累加后作为MeanShift主要计算公式的分母
						//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

						double core_y=cal_core(j,cur_center->y,r);//y轴
						sum1_y+=core_y*w*j;
						sum2_y+=core_y*w;

						double core_x=cal_core(i,cur_center->x,r);//x轴
						sum1_x+=core_x*w*i;
						sum2_x+=core_x*w;
						//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

					}
				}
			}
			V3DLONG temp_x=cur_center->x;//3个temp放置前一个圆心点x\y\z坐标
			V3DLONG temp_y=cur_center->y;
			V3DLONG temp_z=cur_center->z;
			flag[GET_IND(temp_x,temp_y,temp_z)]=1;//对上一个圆心点标记为1，表示该点已经被处理过

			V3DLONG next_x=(sum1_x/sum2_x)+0.5;
			V3DLONG next_y=(sum1_y/sum2_y)+0.5;
			V3DLONG next_z=(sum1_z/sum2_z)+0.5;
			cur_center->x=next_x;//把新的下一个圆心点赋给作为当前圆心
			cur_center->y=next_y;
			cur_center->z=next_z;
			

			//方式2
			Node *pre_center=new Node(temp_x,temp_y,temp_z);
			if(!nodeList.contains(pre_center))//判断上一个圆心是否在List里面，如果在的话说明当前节点和前一节点相同，前一节点并未移动
				{//在这里计算这个点的半径，
					//enlarge_radiusof_single_node_xy(img1d,pre_center,sz_x,sz_y,sz_z);
					nodeList.append(pre_center);
				}	

			if(flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]==1)
			{//计算出来的当前中心点已经被计算过，跳出while循环，不再进行计算
			//被计算过说明以前就被计算过一次，再计算的话还是会被移动到终点，再做无意义
				break;
			}

			iter++;//保险措施，迭代100次后，不再迭代
			
		}
	}
	
	bool f=found_final(img1d,cur_center->x,cur_center->y,cur_center->z,sz_x,sz_y,sz_z,r);//用于判断该点是否能移动，也表示这个点是根节点还是路径上的点
	V3DLONG ind2=GET_IND(cur_center->x,cur_center->y,cur_center->z);
	double temp_dis=1000000;
	
	if((f==false))//说明没有移动，是根节点，需要把这条路径上的点都赋予根节点的标记
	{//找到了新的概率密度最大的点
		situation=1;//用于标记找到的最终点属于哪种情况
		int mark=0;
		if(Map_finalnode.contains(ind2))
		{
			mark_nodeList(Map_finalnode[ind2],nodeList,situation);//对nodeList里面的每一个节点都给标mark
			//每一个根节点都会有若干个像素点作为它的子节点

		}else
		{
			//担心会出现没有出现在Map_finalnode，如果没有，则将该点分为Map_finalnode中和它距离最近的根节点的class_mark
			for(QMap<V3DLONG,Node*>::iterator iter =Map_finalnode_list.begin(); iter != Map_finalnode_list.end(); iter++)
			{
				Node* elem = iter.value();
				
				double dis=(double)sqrt((double)(cur_center->x-elem->x)*(cur_center->x-elem->x)+(double)(cur_center->y-elem->y)*(cur_center->y-elem->y)+(double)(cur_center->z-elem->z)*(cur_center->z-elem->z));
				if(dis<temp_dis)
				{
					temp_dis=dis;
					mark=elem->class_mark;
				}else
				{

				}
			}
			if(mark==0)
			printf("mark:::%d\n",mark);
			mark_nodeList(mark,nodeList,situation);
		}
		
	}else//终点能移动，并且被处理过
	{
		//有这个点的坐标，找到这个点的mark,给nodeList赋值
		situation=2;
		//printf("%d\n",nodeList.size());
		int mark3=found_nodefrom_VectorList(cur_center->x,cur_center->y,cur_center->z,v_List);
		if(mark3==0)
		{//会出现不明原因的在VECTOR里面找不到该点的情况，老办法,为什么会在Vector里面找不到？
			for(QMap<V3DLONG,Node*>::iterator iter =Map_finalnode_list.begin(); iter != Map_finalnode_list.end(); iter++)
			{
				Node* elem = iter.value();
				
				double dis=(cur_center->x-elem->x)*(cur_center->x-elem->x)+(cur_center->y-elem->y)*(cur_center->y-elem->y)+(cur_center->z-elem->z)*(cur_center->z-elem->z);
				if(dis<temp_dis)
				{
					temp_dis=dis;
					mark3=elem->class_mark;
				}else
				{

				}
			}

		}
		mark_nodeList(mark3,nodeList,situation);
	
	}
	if(situation==0)
	{
		printf("situation=0::something wrong\n");

	}
	print_nodeList_mark(nodeList);//for testing	
	return nodeList;

}





QList<Node*> meanshift_vn3(unsigned char * &img1d,int *flag,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration)
{
	int iter=0;
	int same_times=0;
	int situation=0;//用于观察找到的最终点属于哪种类型

	QList<Node*> nodeList;
	nodeList.clear();//每次都clear
	Node *cur_center=new Node(x,y,z);//圆的中心点，记录圆中心点的三维坐标
	double intensity=img1d[GET_IND(x,y,z)];//通过圆中心的三维坐标计算它的信号强度
	cur_center->intensity=intensity;
	//Node *pre_center=new Node(x,y,z);//前一个中心点，用于判断当前中心点和前一个中心点是否一样，一样的话停止均值偏移
	//nodeList.append(pre_center);
	
	Node *temp=new Node();
	//printf("%d\n",flag[GET_IND(pre_center->x,pre_center->y,pre_center->z)]);
	//printf("%ld  %ld   %ld   %ld  %lf\n",r,cur_center->x,cur_center->y,cur_center->z,intensity);
	while(1)
	{
		if(iter==iteration)
		{
			break;

		}else
		{
			double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
			//定义范围，以半径为r的立方区域进行移动
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
					{//圆心点需要跟这个区域中的每个点都计算权值

						if(GET_IND(i,j,k)==GET_IND(cur_center->x,cur_center->y,cur_center->z)) continue;//圆心点不计算
						if(img1d[GET_IND(i,j,k)]==0) continue;//像素为0的点不计算
						double cur_intensity=img1d[GET_IND(i,j,k)];
						w=cal_weight(i,j,k,cur_center->x,cur_center->y,cur_center->z,cur_intensity,intensity,r);//计算区域中每个点和圆心的权重，主要是基于像素强度和距离

						double core_z=cal_core(k,cur_center->z,r);//计算核函数，z坐标
						sum1_z+=core_z*w*k;//计算Z轴坐标，累加后作为MeanShift主要计算公式的分子
						sum2_z+=core_z*w;//累加后作为MeanShift主要计算公式的分母
						//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

						double core_y=cal_core(j,cur_center->y,r);//y轴
						sum1_y+=core_y*w*j;
						sum2_y+=core_y*w;

						double core_x=cal_core(i,cur_center->x,r);//x轴
						sum1_x+=core_x*w*i;
						sum2_x+=core_x*w;
						//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

					}
				}
			}
			V3DLONG temp_x=cur_center->x;//3个temp放置前一个圆心点x\y\z坐标
			V3DLONG temp_y=cur_center->y;
			V3DLONG temp_z=cur_center->z;
			flag[GET_IND(temp_x,temp_y,temp_z)]=1;//对上一个圆心点标记为1，表示该点已经被处理过

			V3DLONG next_x=(sum1_x/sum2_x)+0.5;
			V3DLONG next_y=(sum1_y/sum2_y)+0.5;
			V3DLONG next_z=(sum1_z/sum2_z)+0.5;
			cur_center->x=next_x;//把新的下一个圆心点赋给作为当前圆心
			cur_center->y=next_y;
			cur_center->z=next_z;
			

			//方式2
				if(!exist(temp->x,temp->y,temp->z,nodeList))//在List里面不存在就放进去
				{
				//nodeList.append(temp);//这样每个像素点都知道它的父节点是谁
					Node *next_center=new Node(temp_x,temp_y,temp_z);
					next_center->parent=GET_IND(cur_center->x,cur_center->y,cur_center->z);//记录父节点的二维坐标
					//if(next_center->parent==0)
					//printf("%d\n",next_center->parent);
					nodeList.append(next_center);

				}	
			
			
			if(flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]==1)
			{//计算出来的当前中心点已经被计算过，跳出while循环，不再进行计算
				//printf("1:::%ld   %ld   %ld   %d\n",cur_center->x,cur_center->y,cur_center->z,flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]);
				//Node* next_2_center=new Node(cur_center->x,cur_center->y,cur_center->z);
				//nodeList.append(next_2_center);
				break;
			}

			iter++;//保险措施，迭代100次后，不再迭代
			
		}
	}
	
	bool f=found_final(img1d,cur_center->x,cur_center->y,cur_center->z,sz_x,sz_y,sz_z,r);//用于判断该点是否能移动，也表示这个点是根节点还是路径上的点
	bool exist_b=exist(cur_center->x,cur_center->y,cur_center->z,final_nodeList);
	V3DLONG ind2=GET_IND(cur_center->x,cur_center->y,cur_center->z);
	double temp_dis=1000000;
	
	if((f==false))//说明没有移动，是根节点，需要把这条路径上的点都赋予根节点的标记
	{//找到了新的概率密度最大的点
		situation=1;//用于标记找到的最终点属于哪种情况
		
		int mark=0;
		

		if(Map_finalnode.contains(ind2))
		{
			mark_nodeList(Map_finalnode[ind2],nodeList,situation);//对nodeList里面的每一个节点都给标mark
			//每一个根节点都会有若干个像素点作为它的子节点

		}else
		{
			//担心会出现没有出现在Map_finalnode，如果没有，则将该点分为Map_finalnode中和它距离最近的根节点的class_mark
			for(QMap<V3DLONG,Node*>::iterator iter =Map_finalnode_list.begin(); iter != Map_finalnode_list.end(); iter++)
			{
				Node* elem = iter.value();
				
				double dis=(cur_center->x-elem->x)*(cur_center->x-elem->x)+(cur_center->y-elem->y)*(cur_center->y-elem->y)+(cur_center->z-elem->z)*(cur_center->z-elem->z);
				if(dis<temp_dis)
				{
					temp_dis=dis;
					mark=elem->class_mark;
				}else
				{

				}
			}
			mark_nodeList(mark,nodeList,situation);
		}
		
	}else//终点能移动，并且被处理过
	{
		//有这个点的坐标，找到这个点的mark,给nodeList赋值
		situation=2;
		//printf("%d\n",nodeList.size());
		int mark3=found_nodefrom_VectorList(cur_center->x,cur_center->y,cur_center->z,v_List);
		if(mark3==0)
		{//会出现不明原因的在VECTOR里面找不到该点的情况，老办法
			for(QMap<V3DLONG,Node*>::iterator iter =Map_finalnode_list.begin(); iter != Map_finalnode_list.end(); iter++)
			{
				Node* elem = iter.value();
				
				double dis=(cur_center->x-elem->x)*(cur_center->x-elem->x)+(cur_center->y-elem->y)*(cur_center->y-elem->y)+(cur_center->z-elem->z)*(cur_center->z-elem->z);
				if(dis<temp_dis)
				{
					temp_dis=dis;
					mark3=elem->class_mark;
				}else
				{

				}
			}

		}
		mark_nodeList(mark3,nodeList,situation);
	
	}
	if(situation==0)
	{
		printf("situation=0::something wrong\n");

	}
	print_nodeList_mark(nodeList);//for testing	
	return nodeList;

}



QList<Node*> meanshift_vn2(unsigned char * &img1d,int *flag,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration)
{
	//QMap<V3DLONG,QList<Node*>> road;
	//V_NeuronSWC_unit *unit;//这里的unit实际上就是一个像素点；
	//vector<V_NeuronSWC_unit> *unit;//由像素点组成的向量
	
	
	int iter=0;
	int same_times=0;
	int situation=0;//用于观察找到的最终点属于哪种类型

	QList<Node*> nodeList;
	Node *cur_center=new Node(x,y,z);//圆的中心点，记录圆中心点的三维坐标
	double intensity=img1d[GET_IND(x,y,z)];//通过圆中心的三维坐标计算它的信号强度
	cur_center->intensity=intensity;
	//Node *pre_center=new Node(x,y,z);//前一个中心点，用于判断当前中心点和前一个中心点是否一样，一样的话停止均值偏移
	//nodeList.append(pre_center);
	
	Node *temp=new Node();
	//printf("%d\n",flag[GET_IND(pre_center->x,pre_center->y,pre_center->z)]);
	//printf("%ld  %ld   %ld   %ld  %lf\n",r,cur_center->x,cur_center->y,cur_center->z,intensity);
	while(1)
	{
		if(iter==iteration)
		{
			break;

		}else
		{
			double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
			//定义范围，以半径为r的立方区域进行移动
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
					{//圆心点需要跟这个区域中的每个点都计算权值

						if(GET_IND(i,j,k)==GET_IND(cur_center->x,cur_center->y,cur_center->z)) continue;//圆心点不计算
						if(img1d[GET_IND(i,j,k)]==0) continue;//像素为0的点不计算
						double cur_intensity=img1d[GET_IND(i,j,k)];
						w=cal_weight(i,j,k,cur_center->x,cur_center->y,cur_center->z,cur_intensity,intensity,r);//计算区域中每个点和圆心的权重，主要是基于像素强度和距离

						double core_z=cal_core(k,cur_center->z,r);//计算核函数，z坐标
						sum1_z+=core_z*w*k;//计算Z轴坐标，累加后作为MeanShift主要计算公式的分子
						sum2_z+=core_z*w;//累加后作为MeanShift主要计算公式的分母
						//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

						double core_y=cal_core(j,cur_center->y,r);//y轴
						sum1_y+=core_y*w*j;
						sum2_y+=core_y*w;

						double core_x=cal_core(i,cur_center->x,r);//x轴
						sum1_x+=core_x*w*i;
						sum2_x+=core_x*w;
						//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

					}
				}
			}
			temp->x=cur_center->x;
			temp->y=cur_center->y;
			temp->z=cur_center->z;
			flag[GET_IND(temp->x,temp->y,temp->z)]=1;//对上一个圆心点标记为1，表示该点已经被处理过

			V3DLONG next_x=(sum1_x/sum2_x)+0.5;
			V3DLONG next_y=(sum1_y/sum2_y)+0.5;
			V3DLONG next_z=(sum1_z/sum2_z)+0.5;
			cur_center->x=next_x;
			cur_center->y=next_y;
			cur_center->z=next_z;
			//intensity=img1d[GET_IND(cur_center->x,cur_center->y,cur_center->z)];//得到下一个像素点的像素强度
			//temp->parent=GET_IND(cur_center->x,cur_center->y,cur_center->z);//记录父节点
			//每次都把前一个像素点放到nodeList里面

			//方式2
				if(!exist(temp->x,temp->y,temp->z,nodeList))//在List里面不存在就放进去
				{
				//nodeList.append(temp);//这样每个像素点都知道它的父节点是谁
					Node *next_center=new Node(temp->x,temp->y,temp->z);
					next_center->parent=GET_IND(cur_center->x,cur_center->y,cur_center->z);//记录父节点的二维坐标
					nodeList.append(next_center);

				}	
			
			
			if(flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]==1)
			{//计算出来的当前中心点已经被计算过，跳出while循环，不再进行计算
				//printf("1:::%ld   %ld   %ld   %d\n",cur_center->x,cur_center->y,cur_center->z,flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]);
				break;
			}else
			{//当前中心点和上一个中心点不一样，就把当前中心点放到QList里面
				//方式1和方式2效果一样
				/*if(!exist(cur_center->x,cur_center->y,cur_center->z,nodeList))
				{
					//printf("2:::%ld   %ld   %ld   %d\n",cur_center->x,cur_center->y,cur_center->z,flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]);
			
					Node *next_center=new Node(cur_center->x,cur_center->y,cur_center->z);
					//next_center->intensity=img1d[GET_IND(cur_center->x,cur_center->y,cur_center->z)];
					//printf("%lf\n",next_center->intensity);
					//if(next_center->intensity!=0)
					nodeList.append(next_center);
				}*/
				
				
			}

			iter++;//保险措施，迭代100次后，不再迭代
			//printf("  %d::: %ld   %ld   %ld\n",iter,cur_center->x,cur_center->y,cur_center->z);

		}
	}
	//在这里用Map结构存每个像素点到最终点的路径貌似会存在问题，
	//1.我想知道哪些点进行MeanShift计算后会偏移到哪些点上？  解决！！！
	//2.通过这种均值偏移的方式是否会得到一个父与子的关系，比如说最终点作为根节点，下一个节点作为父节点？  解决！！！
	//3.这种方式处理过后，如何对图像进行裁剪？
	//4.处理完成后，会有断裂的地方，如何处理？   这种情况少，几乎没有！！！
	//找根节点的方法可以将每个根节点标记为0，从而观察根节点的位置，对找到的最终节点再做一次MeanShift，如果还能移动，则不是根节点，如果不能则标记为-1；
	//road[GET_IND(cur_center->x,cur_center->y,cur_center->z)]=nodeList;//这种处理方法有问题
	//printf("%ld\n",GET_IND(cur_center->x,cur_center->y,cur_center->z));
	//在这里写一个把最终节点写到文件中的函数，观察最终点都找到了哪些点
	bool f=found_final(img1d,cur_center->x,cur_center->y,cur_center->z,sz_x,sz_y,sz_z,r);//用于判断该点是否能移动，也表示这个点是根节点还是路径上的点
	bool exist_b=exist(cur_center->x,cur_center->y,cur_center->z,final_nodeList);
	
	if((f==false)&&(exist_b==false))//说明没有移动，是根节点，加到final_node里面;同时，需要判断当前圆点是否已经在final_nodeList里面
	{//找到了新的概率密度最大的点
		situation=1;//用于标记找到的最终点属于哪种情况
		Node *final_node=new Node(cur_center->x,cur_center->y,cur_center->z);
		final_node->parent=-1;
		final_node->class_mark=ClassMark(final_nodeList);//对根节点进行标记，每个根节点都赋予不同的标记符
		//printf("%d\n",final_node->class_mark);
		
		final_nodeList.append(final_node);//final_nodeList里面的点都是进行MeanShift不会移动的点，把这些点都作为根节点
		mark_nodeList(final_node->class_mark,nodeList,situation);//对nodeList里面的每一个节点都给标mark
		//每一个根节点都会有若干个像素点作为它的子节点

	}else if((f==false)&&(exist_b==true))//终点不能移动，但在终点队列里面
	{
		//先从final_node里面找到这个点，取出标记，再给nodeList赋值
		situation=2;
		int mark2=found_finalnode_mark(cur_center->x,cur_center->y,cur_center->z,final_nodeList);
		mark_nodeList(mark2,nodeList,situation);

	}else if((f==true)&&(flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]==1))//终点能移动，并且被处理过
	{
		//有这个点的坐标，找到这个点的mark,给nodeList赋值
		situation=3;
		//printf("%d\n",nodeList.size());
		int mark3=found_nodefrom_VectorList(cur_center->x,cur_center->y,cur_center->z,v_List);
		mark_nodeList(mark3,nodeList,situation);
	
	}
	if(situation==0)
	{
		printf("situation=0::something wrong\n");

	}

	print_nodeList_mark(nodeList);//for testing
	
	
	return nodeList;
}

void mark_nodeList(int mark,QList<Node*> &List,int s)
{
	
	if(s==1)
	{
	
		//printf("situation::%d  List size::%d \n",s,List.size());
	
		for(int i=0;i<List.size();i++)
		{
			//Node* temp=List.at(i);
			//temp->class_mark=mark;
			List.at(i)->class_mark=mark;
			//printf("%d\n",temp->class_mark);

		}
		

	}else if(s==2)
	{
		//printf("situation::%d  List size::%d\n",s,List.size());
		
		for(int i=0;i<List.size();i++)
		{
			//Node* temp=List.at(i);
			//temp->class_mark=mark;
			List.at(i)->class_mark=mark;

		}


	}else if(s==3)
	{
		//printf("situation::%d  List size::%d\n",s,List.size());
		
		for(int i=0;i<List.size();i++)
		{
			//Node* temp=List.at(i);
			//temp->class_mark=mark;
			List.at(i)->class_mark=mark;

		}

	}
}

int ClassMark(QList<Node *> final_List)
{
	
		return final_List.size()+1;

}

bool exist(V3DLONG x,V3DLONG y,V3DLONG z,QList<Node*> List)//判断一个点是否在一个List里面
{
	bool flag=false;
	for(int i=0;i<List.size();i++)
	{
		Node *temp=List.at(i);
		if((temp->x==x)&&(temp->y==y)&&(temp->z==z))
		{
			flag=true;
			break;

		}

	}
	return flag;


}

int meanshift_plugin(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if(!curwin)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	Image4DSimple *p4d = callback.getImage(curwin);
	V3DLONG sz_x = p4d->getXDim();
	V3DLONG sz_y = p4d->getYDim();
	V3DLONG sz_z = p4d->getZDim();
	unsigned int ws = 5;
	unsigned char* img1d = p4d->getRawDataAtChannel(1);
	printf("find seed\n");
	QMap<V3DLONG,Node*> nodeMap = searchAndConnectByWs(img1d,sz_x,sz_y,sz_z,ws);
	printSwcByMap(nodeMap,"meanshift_seed_node.swc");
	printf("find %d seed\n",nodeMap.size());
	printf("create tree\n");
	Node* root = nodeMap.begin().value();
	root->parent = -1;
	createTree(img1d,root,sz_x,sz_y,sz_z,ws);
	prundUnConnectNode(nodeMap);
	printf("create tree over,now print this tree\n");
	printSwcByMap(nodeMap,"meanshift_init_tree.swc");
	printf("finish\n");
}
void prundUnConnectNode(QMap<V3DLONG,Node*> nodeMap)
{
	for(QMap<V3DLONG,Node*>::iterator iter = nodeMap.begin(); iter != nodeMap.end(); iter++)
	{
		V3DLONG key = iter.key();
		if(nodeMap[key]->parent == -2) nodeMap.remove(key);
	}
}

//建立节点之间的树结构
void createTree(unsigned char* &img1d,Node* curNode,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,unsigned int ws)
{
	QList<Node*> childList;
	for(QMap<V3DLONG,Node*>::iterator iter = curNode->connect.begin(); iter != curNode->connect.end(); iter++)
	{
		//       printf("key:%d\n",iter.key());
		Node* tmp = curNode->connect[iter.key()];
		tmp->connect.remove(curNode->ind);
		if(checkConnect(img1d,curNode,tmp,ws,sz_x,sz_y,sz_z))
		{
			if(tmp->parent == -2)
			{
				tmp->parent = curNode->number;
				childList.append(tmp);
				curNode->connect[tmp->ind] = tmp;
			}
		}
		else
			curNode->connect.remove(tmp->ind);

	}
	printf("childList size %d\n",childList.size());
	for(int i = 0; i < childList.size(); i++)
	{
		createTree(img1d,childList.at(i),sz_x,sz_y,sz_z,ws);
	}
}
bool checkConnect(unsigned char* &img1d,Node* begin,Node* end,unsigned int ws,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
	V3DLONG sz_xy = sz_x * sz_y;
	//通过fast marching判断是否存在通路
	V3DLONG min_x = begin->x > end->x ? end->x : begin->x;
	V3DLONG max_x = begin->x > end->x ? begin->x : end->x;
	V3DLONG min_y = begin->y > end->y ? end->y : begin->y;
	V3DLONG max_y = begin->y > end->y ? begin->y : end->y;
	V3DLONG min_z = begin->z > end->z ? end->z : begin->z;
	V3DLONG max_z = begin->z > end->z ? begin->z : end->z;
	enum{ALIVE = -1,TRIAL = 0,FAR = 1};
	QMap<V3DLONG,char> state;
	QMap<V3DLONG,int> phi;
	for(int i = min_x; i <= max_x; i++)
	{
		for(int j = min_y; j <= max_y; j++)
		{
			for(int k = min_z; k <= max_z; k++)
			{
				V3DLONG ind = GET_IND(i,j,k);
				if(img1d[ind] == 0) state[ind] = ALIVE;
				else state[ind] = FAR;
				phi[ind] = 1000;
			}
		}
	}
	V3DLONG begin_ind = GET_IND(begin->x,begin->y,begin->z);
	phi[begin_ind] = 0;
	HeapElemX* beginNode = new HeapElemX(begin_ind,phi[begin_ind]);
	BasicHeap<HeapElemX> heap;
	heap.insert(beginNode);
	V3DLONG end_ind = GET_IND(end->x,end->y,end->z);
	int mistake = 2;
	while(!heap.empty())
	{
		HeapElemX* min_elem = heap.delete_min();
		V3DLONG min_ind = min_elem->img_ind;
		state[min_ind] = ALIVE;

		V3DLONG x = GET_X(min_ind);V3DLONG y = GET_Y(min_ind);V3DLONG z = GET_Z(min_ind);
		for(int i = x - mistake - 1; i <= x + mistake + 1; i++)
		{
			if(i < min_x || i > max_x) continue;
			for(int j = y - mistake - 1; j <= y + mistake + 1; j++)
			{
				if(j < min_y || j > max_y) continue;
				for(int k = z - mistake - 1; k <= z + mistake + 1; k++)
				{
					if(k < min_z || k > max_z) continue;
					V3DLONG ind = GET_IND(i,j,k);
					if(ind == end_ind) return true;
					if(state[ind] != ALIVE)
					{
						double dist = phi[min_ind] + 1;

						if(state[ind] != TRIAL)
						{
							phi[ind] = dist;
							state[ind] = TRIAL;
							HeapElemX* tmp = new HeapElemX(ind,phi[ind]);
							heap.insert(tmp);
						}
					}
				}
			}
		}
	}
	return false;
}

QMap<V3DLONG,Node*> searchAndConnectByWs(unsigned char* &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,unsigned int ws)
{
	QMap<V3DLONG,Node*> rstMap;
	V3DLONG sz_xy = sz_x * sz_y;
	for(int i = 0; i < sz_x; i += ws)
	{
		for(int j = 0; j < sz_y; j += ws)
		{
			for(int k = 0; k < sz_z; k += ws)
			{
				V3DLONG ind = meanshift(img1d,i,j,k,sz_x,sz_y,sz_z,ws);
				if(ind < 0) continue;
				Node* node = new Node();

				node->number = rstMap.size() + 1;
				node->parent = -2;
				node->r = 1;
				node->x = GET_X(ind);
				node->y = GET_Y(ind);
				node->z = GET_Z(ind);
				node->ind = GET_IND(i,j,k);
				rstMap[GET_IND(i,j,k)] = node;
			}
		}
	}
	for(QMap<V3DLONG,Node*>::iterator iter = rstMap.begin(); iter != rstMap.end(); iter++)
	{
		V3DLONG ind = iter.key();
		V3DLONG x = GET_X(ind);
		V3DLONG y = GET_Y(ind);
		V3DLONG z = GET_Z(ind);

		V3DLONG xb = x - ws >= 0 ? x - ws : 0;
		V3DLONG xe = x + ws < sz_x  ? x + ws : sz_x - 1;
		V3DLONG yb = y - ws >= 0 ? y - ws : 0;
		V3DLONG ye = y + ws < sz_y  ? y + ws : sz_y - 1;
		V3DLONG zb = z - ws >= 0 ? z - ws : 0;
		V3DLONG ze = z + ws < sz_z  ? z + ws : sz_z - 1;

		for(V3DLONG i = xb; i <= xe; i+=ws)
		{
			for(V3DLONG j = yb; j <= ye; j+=ws)
			{
				for(V3DLONG k = zb; k <= ze; k+=ws)
				{
					V3DLONG ot_ind = GET_IND(i,j,k);
					if(ot_ind == ind) continue;
					if(rstMap.contains(ot_ind))
					{
						rstMap[ind]->connect[ot_ind] = rstMap[ot_ind];
					}
				}
			}
		}
		if(rstMap[ind]->connect.size() == 0) rstMap.remove(ind);
	}
	return rstMap;
}

#define Radial_Basis_Function(v) exp(-1.0*((v)*(v)))
#define Similar(v1,v2) exp(-1.0*(v1-v2)*(v1-v2))
V3DLONG meanshift(unsigned char* &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,unsigned int ws)
{
	V3DLONG sz_xy = sz_x * sz_y;
	V3DLONG max_ind = GET_IND(x,y,z);//将三维平面的坐标转换成二维平面的坐标
	double max = img1d[max_ind];
	//寻找最大点
	for(int i = x; i < x + ws && i < sz_x; i++)
	{
		for(int j = y; j < y + ws && j < sz_y; j++)
		{
			for(int k = z; k < z + ws && k < sz_z; k++)
			{
				V3DLONG ind = GET_IND(i,j,k);
				if(max < img1d[ind])
				{
					max = img1d[ind];
					max_ind = ind;
				}
			}
		}
	}
	//进行meanshift
	int count = 0;
	V3DLONG curCenter = max_ind;
	if(img1d[max_ind] == 0) return -1;
	int r = ws / 2;

	while(count < 50)
	{

		V3DLONG cx = GET_X(curCenter);
		V3DLONG cy = GET_Y(curCenter);
		V3DLONG cz = GET_Z(curCenter);

		V3DLONG xb = cx - r > x ? cx - r : x;
		V3DLONG xe = cx + r + 1 < x + ws  ? cx + r + 1 : x + ws;
		V3DLONG yb = cy - r > y ? cy - r : y;
		V3DLONG ye = cy + r + 1 < y + ws  ? cy + r + 1 : y + ws;
		V3DLONG zb = cz - r > z ? cz - r : z;
		V3DLONG ze = cz + r + 1 < z + ws  ? cz + r + 1 : z + ws;



		V3DLONG oldCenter = curCenter;
		double sumX=0,sumY=0,sumZ=0;
		double wX=0,wY=0,wZ=0;
		for(V3DLONG i = xb; i < xe; i++)
		{
			for(V3DLONG j = yb; j < ye; j++)
			{
				for(V3DLONG k = zb; k < ze; k++)
				{
					V3DLONG curInd = GET_IND(i,j,k);
					if(img1d[curInd] == 0) continue;
					double w = Similar(img1d[curInd],img1d[curCenter]);
					wX += Radial_Basis_Function(i - cx) * w * i;
					sumX += Radial_Basis_Function(i - cx) * w;

					wY += Radial_Basis_Function(j - cy) * w * j;
					sumY += Radial_Basis_Function(j - cy) * w;

					wZ += Radial_Basis_Function(k - cz) * w * k;
					sumZ += Radial_Basis_Function(k - cz) * w;
				}
			}
		}
		if(sumX == 0 || sumY == 0 || sumZ == 0) return img1d[curCenter] == 0 ? -1 : curCenter;
		cx = (wX / sumX) + 0.5;
		cy = (wY / sumY) + 0.5;
		cz = (wZ / sumZ) + 0.5;
		curCenter = cx + cy * sz_x + cz * sz_xy;
		if(curCenter == oldCenter)
		{
			if(r <= 1) return img1d[curCenter] == 0 ? -1 : curCenter;
			r--;
		}
		count++;
	}
	return curCenter;
}


/* functions in DOFUNC takes 2 parameters
* "input" arglist has 2 positions reserved for input and parameter:
*            input.at(0).p returns a pointer to vector<char*> that pass the arglist following the input option '-i'
*                                  items are splitted by ' ', which is often used as input data 
*                                  [required]
*            input.at(1).p returns a pointer to vector<char*> that pass the arglist following the input option '-p'. 
*                                  items are splitted by ' ', it is reserved for you to define your own parameters 
*                                  [not required, if '-p' is not specified, input only contains one member]
* "output" arglist has a size of 1:
*            output.at(0).p returns a pointer to vector<char*> that pass the arglist following the input option '-o' 
*                                  items are splitted by ' ', which is often used as output data 
*                                  [required]
*/
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
