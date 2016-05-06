/* meanshift_func.cpp
* This file contains the functions used in plugin domenu and dufunc, you can use it as a demo.
* 2015-03-18 : by Zhijiang Wan
*/
#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "meanshift_func.h"
#include "D:\Vaa3D\v3d_main\neurontracing_vn21\app2/heap.h"
#include "D:\Vaa3D\v3d_main\neuron_editing\v_neuronswc.h"
#include "D:\Vaa3D/sort_neuron_swc/sort_swc.h"
#include <vector>
#include <iostream>

using namespace std;

/* function used in DOMENU typically takes  inputs:
* "callback" - provide information from the plugin interface, and 
* "parent"   - the parent widget of the Vaa3D main window
*/
QMap<int,QList<Node*>> Map_finalnode_list;//键值是种子点的类别，QList是对应种子点类别的节点列
QMap<int,Node*> Map_rootnode;//int是节点的编号，也是类别的编号
QMultiMap<int,Node*> Map_allnodes;//给图像中的每一个像素点都分配mark

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


void enlarge_radiusof_single_node_xy(unsigned char * &img1d,Node * &node,V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z)
{
	//基于xy平面4领域的半径增长

	int allnodes=0;
	int back_nodes=0;
	int max=0;//找最大半径
	double threshold=30;
	QQueue<Node*> *queue=new QQueue<Node*>();
	queue->clear();
	queue->enqueue(node);
	while(!(queue->isEmpty()))//基于根节点以1为步长，一周一周往外扩，把范围内的点都加到队列里面
	{

		Node* head=queue->head();
		Node* up=new Node(head->x,head->y-1,head->z);
		Node* down=new Node(head->x,head->y+1,head->z);
		Node* left=new Node(head->x-1,head->y,head->z);
		Node* right=new Node(head->x+1,head->y,head->z);

		//if(!queue->contains(up))//队列里面没有包括该节点
		if(!contain(queue,up->x,up->y,up->z))
		{

			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(up->x,up->y,up->z)]);
			//printf("%ld   %ld   %ld\n",up->x,up->y,up->z);
			if(img1d[GET_IND(up->x,up->y,up->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}

			queue->enqueue(up);

		}

		//if(!queue->contains(down))
		if(!contain(queue,down->x,down->y,down->z))
		{
			//printf("no down\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(down->x,down->y,down->z)]);
			//printf("%ld   %ld   %ld\n",down->x,down->y,down->z);
			if(img1d[GET_IND(down->x,down->y,down->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}

			queue->enqueue(down);

		}

		//if(!queue->contains(left))
		if(!contain(queue,left->x,left->y,left->z))
		{
			//printf("no left\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(left->x,left->y,left->z)]);
			//printf("%ld   %ld   %ld\n",left->x,left->y,left->z);
			if(img1d[GET_IND(left->x,left->y,left->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}

			queue->enqueue(left);

		}

		//if(!queue->contains(right))
		if(!contain(queue,right->x,right->y,right->z))
		{
			//printf("no right\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(right->x,right->y,right->z)]);
			//printf("%ld   %ld   %ld\n",right->x,right->y,right->z);
			if(img1d[GET_IND(right->x,right->y,right->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}

			queue->enqueue(right);

		}
		//printf("size:::%d   \n",queue.size());
		//printf("%d   %d\n",allnodes,back_nodes);

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
			//node->r=max;
			node->r=(up_r+down_r+left_r+right_r)/4;
			//printf("per:::%f   r:::%lf\n",per,node->r);
			break;

		}

		//出队
		queue->dequeue();

	}
	delete queue;


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
void meanshift_plugin_vn4(V3DPluginCallback2 &callback, QWidget *parent)
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
	V3DLONG r=10;//meanshift用到的半径
	V3DLONG count=0;
	int times=100;
	int num_mark=1;//用于在初始化的时候对种子点进行分类标号
	//QList<Node*> list;
	printf("### Initializing...  ###\n");//20150315，一开始就对整幅图像进行初始化操作，寻找不动点是没有必要的，可以一边寻找终止点，一边给经过的节点进行标记，以后再做修改
	

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
				/*	if(img1d[ind]<30)//像素强度超过某一阈值的坐标点不予考虑，只对具有像素强度的点进行meanshift
				continue;*/
				if(!found_final(img1d,ii,jj,kk,sz_x, sz_y, sz_z, r))//没有动
				{
					Node* final=new Node(ii,jj,kk);
					//QList<Node*> *list=new QList<Node*>();
					final->class_mark=num_mark;
					final->parent=-1;//根节点标记为-1
					final->number=0;//每个根节点的序号标记为0
					
					Map_finalnode.insert(ind,num_mark);//给根节点分配类别标号,ind是二维坐标,num_mark是标号
					enlarge_radiusof_single_node_xy(img1d,final,sz_x, sz_y, sz_z);//计算根节点的半径
					Map_rootnode.insert(num_mark,final);//给根节点分配类别标号，num_mark是类别，final对应相应的节点
					num_mark++;
					

				}

			}
		}
	}

	//printSwcByList( final_nodeList1,"D:\\meanshift_vn3_finalnode.swc");
	printSwcByMap(Map_rootnode,"D:\\meanshift_vn3_finalnode.swc");
	printf("###   initializing finished   ###\n");
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
			//20150318,先找种子点然后聚类的做法是没有必要的，所有点等于计算meanshift了两遍，可以一边shift一边找终点
				meanshift_vn5(img1d,i,j,k,sz_x,sz_y,sz_z,r,times);//通过这个函数，所有的节点都被分门别类的放到了Map_finalnode_list<int,QList<Node*>>中
			}

		}

	}
	//上一步的结果是将所有点按照根节点的类别分门别类的放到了Map_finalnode_list里面
	printSwcByMap(Map_rootnode,"D:\\result\\meanshift_vn3_finalnode.swc");
	printSwcByMap(Map_allnodes,"D:\\result\\meanshift_allnode.swc");
	printSWCByMap_List(Map_finalnode_list,"D:\\result\\meanshift_allnode_List.swc");

	merge_rootnode(Map_rootnode,img1d,sz_x,sz_y,sz_z);//这个函数的输出结果是一个新的Map_rootnode,比之前的根节点数少，并且任意根节点之间不会重合
	//20150312,前面的步骤是实现了对所有像素点的分类，下一步是对每一类采用最小生成树进行,finalclass_node保存了最终点和像素点之间的对应关系
	QList<QList <NeuronSWC>> MST_sorted;
	NeuronTree nt_result;
	//下一步是对融合后的种子点用spanning tree进行神经元重构
	//20150317,对种子点进行重构
	//construct_tree_vn2(finalclass_node);//基于找到的种子点

	//construct_tree(finalclass_node, sz_x, sz_y, sz_z);//得到了所有的子生成树
	//nt_result=merge_spanning_tree(result_tree_part);
	//merge_spanning_tree(result_tree_part);//这个函数用于将生成的所有spanning tree连接成一棵树
	//printSWCByQList_Neuron(result_list,"D:\\result\\meanshift_result.swc");
	//writeSWC_file("D:\\result\\meanshift_result_tree_part.swc",result_final_tree);
	//printSWCByQList_QList( result_tree_part,"D:\\result\\meanshift_result_tree_part.swc");
	

	delete []flag;
}

void meanshift_vn5(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration)
{
	int iter=0;
	int same_times=0;
	int situation=0;//用于观察找到的最终点属于哪种类型

	QList<Node*> nodeList;
	nodeList.clear();//每次都clear
	
	Node *cur_center=new Node(x,y,z);//当前圆的中心点，记录圆中心点的三维坐标
	double intensity=img1d[GET_IND(x,y,z)];//通过圆中心的三维坐标计算它的信号强度
	//cur_center->intensity=intensity;

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
			intensity=img1d[GET_IND(cur_center->x,cur_center->y,cur_center->z)];//更新当前圆心的像素强度


			//方式2
			Node *pre_center=new Node(temp_x,temp_y,temp_z);
			if(!nodeList.contains(pre_center))//判断上一个圆心是否在List里面，如果在的话说明当前节点和前一节点相同，前一节点并未移动
			{	//计算点的半径
				enlarge_radiusof_single_node_xy(img1d,pre_center,sz_x,sz_y,sz_z);//计算每一个点的半径，可能会对根节点进行重复计算
				//printf("%lf\n",pre_center->r);
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

	//bool f=found_final(img1d,cur_center->x,cur_center->y,cur_center->z,sz_x,sz_y,sz_z,r);//用于判断该点是否能移动，也表示这个点是根节点还是路径上的点
	V3DLONG ind2=GET_IND(cur_center->x,cur_center->y,cur_center->z);

	double temp_dis=1000000;

	//if((f==false))//说明没有移动，是根节点，需要把这条路径上的点都赋予根节点的标记

	{//找到了新的概率密度最大的点
		//situation=1;//用于标记找到的最终点属于哪种情况
		int mark=0;
		if(Map_finalnode.contains(ind2))//20150311,如果在Map_finalnode里面，说明不能被移动了，是终点
		{//如果Map_finalnode里面包括这个根节点，则将这个List放到相应的Map_finalnode_List中
			for(int iii=0;iii<nodeList.size();iii++)
			{
				V3DLONG node_x=nodeList.at(iii)->x;
				V3DLONG node_y=nodeList.at(iii)->y;
				V3DLONG node_z=nodeList.at(iii)->z;
				Map_nodes[GET_IND(node_x,node_y,node_z)]=Map_finalnode[ind2];//对nodeList里面的节点赋予相应根节点的标记
				Map_allnodes.insert(Map_finalnode[ind2],nodeList.at(iii));
				Map_finalnode_list[Map_finalnode[ind2]].append(nodeList.at(iii));
				

			}
			
		}else if(Map_nodes.contains(GET_IND(cur_center->x,cur_center->y,cur_center->z)))//终点能移动，并且被处理过
		{
			//有这个点的坐标，找到这个点的mark,给nodeList赋值
			situation=2;

			int mark3=Map_nodes[GET_IND(cur_center->x,cur_center->y,cur_center->z)];

			for(int iii=0;iii<nodeList.size();iii++)
			{
				V3DLONG node_x2=nodeList.at(iii)->x;
				V3DLONG node_y2=nodeList.at(iii)->y;
				V3DLONG node_z2=nodeList.at(iii)->z;
				Map_nodes[GET_IND(node_x2,node_y2,node_z2)]=mark3;//对nodeList里面的节点赋予相应根节点的标记
				Map_allnodes.insert(mark3,nodeList.at(iii));
				Map_finalnode_list[mark3].append(nodeList.at(iii));
				//Map_allnode.insert(GET_IND(node_x2,node_y2,node_z2),mark4);//键值是每个点的二维坐标，mark4是终点的二维坐标；
				//Map_finalnode_list[mark3].append(nodeList.at(iii));

			}
			//mark_nodeList(mark3,nodeList,situation);
			printf("1111111111111111111111111111111111111111\n");

		}
		else
		{
			//printf("not found node in the Map_finalnode");
			//担心会出现没有出现在Map_finalnode，如果没有，则将该点分为Map_finalnode中和它距离最近的根节点的class_mark
			//20150306,以前的方法有问题，可以将在Map_finalnode中不包括的点重新给个序号，将nodeList里面的点都归并到这个序号下面
			printf("22222222222222222222222222222222222222\n");
			int new_mark=Map_rootnode.size()+1;
			cur_center->class_mark=new_mark;

			enlarge_radiusof_single_node_xy(img1d,cur_center,sz_x,sz_y,sz_z);//计算根节点的半径
			//if(!Map_rootnode.contains(cur_center))
			{
				Map_rootnode.insert(new_mark,cur_center);

			}


			for(int iii=0;iii<nodeList.size();iii++)
			{
				V3DLONG node_x1=nodeList.at(iii)->x;
				V3DLONG node_y1=nodeList.at(iii)->y;
				V3DLONG node_z1=nodeList.at(iii)->z;

				Map_nodes[GET_IND(node_x1,node_y1,node_z1)]=new_mark;//对nodeList里面的节点赋予相应根节点的标记
				Map_allnodes.insert(new_mark,nodeList.at(iii));
				Map_finalnode_list[new_mark].append(nodeList.at(iii));
				//Map_allnode.insert(GET_IND(node_x1,node_y1,node_z1),ind2);//键值是每个点的二维坐标，ind2是终点的二维坐标；
				//Map_finalnode_list[Map_finalnode[ind2]].append(nodeList.at(iii));

			}
			//mark_nodeList(mark,nodeList,situation);
		}

	}
	nodeList.free();
	delete cur_center;
	
	

}









