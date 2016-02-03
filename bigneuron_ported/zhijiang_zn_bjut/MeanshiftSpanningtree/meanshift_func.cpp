/* example_func.cpp
* This file contains the functions used in plugin domenu and dufunc, you can use it as a demo.
* 2012-02-13 : by Yinan Wan
*/

#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "meanshift_func.h"
#include "smooth_curve.h"
#include "hierarchy_prune.h"

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
#define INF 1000000


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
struct sphere_model{
    //the number of nodes which were included by eight directions
    V3DLONG up_right_up;
    V3DLONG up_left_up;
    V3DLONG up_left_bottom;
    V3DLONG up_right_bottom;
    V3DLONG down_right_up;
    V3DLONG down_left_up;
    V3DLONG down_left_bottom;
    V3DLONG down_right_bottom;
    //define the node with maximum pixval value in the eight directions and corresponding pixal
    Node node_up_right_up;
    Node node_up_left_up;
    Node node_up_left_bottom;
    Node node_up_right_bottom;
    Node node_down_right_up;
    Node node_down_left_up;
    Node node_down_left_bottom;
    Node node_down_right_bottom;

    QList<Node> QList_up_right_up;
    QList<Node> QList_up_left_up;
    QList<Node> QList_up_left_bottom;
    QList<Node> QList_up_right_bottom;
    QList<Node> QList_down_right_up;
    QList<Node> QList_down_left_up;
    QList<Node> QList_down_left_bottom;
    QList<Node> QList_down_right_bottom;


    double pixal_up_right_up;
    double pixal_up_left_up;
    double pixal_up_left_bottom;
    double pixal_up_right_bottom;
    double pixal_down_right_up;
    double pixal_down_left_up;
    double pixal_down_left_bottom;
    double pixal_down_right_bottom;

    double sum_pixal_up_right_up;
    double sum_pixal_up_left_up;
    double sum_pixal_up_left_bottom;
    double sum_pixal_up_right_bottom;
    double sum_pixal_down_right_up;
    double sum_pixal_down_left_up;
    double sum_pixal_down_left_bottom;
    double sum_pixal_down_right_bottom;

};

struct sphere_model_four_directions{
    //the number of nodes which were included by eight directions
    V3DLONG up_right;
    V3DLONG up_left;
    V3DLONG down_right;
    V3DLONG down_left;

    //define the node with maximum pixval value in the eight directions and corresponding pixal
    Node node_up_right;
    Node node_up_left;
    Node node_down_right;
    Node node_down_left;

    QList<Node> QList_up_right;
    QList<Node> QList_up_left;
    QList<Node> QList_down_right;
    QList<Node> QList_down_left;

    double pixal_up_right;
    double pixal_up_left;
    double pixal_down_right;
    double pixal_down_left;

    double sum_pixal_up_right;
    double sum_pixal_up_left;
    double sum_pixal_down_right;
    double sum_pixal_down_left;

};

struct sphere_model_two_directions{
    //the number of nodes which were included by eight directions
    V3DLONG up;
    V3DLONG down;

    //define the node with maximum pixval value in the eight directions and corresponding pixal
    Node node_up;
    Node node_down;

    QList<Node> QList_up;
    QList<Node> QList_down;

    double pixal_up;
    double pixal_down;

    double sum_pixal_up;
    double sum_pixal_down;

};

int *flag;

QMap<int,QList<Node*> > Map_finalnode_list;
QMap<int,Node* > Map_rootnode;
QMap<int,Node > Map_rootnode_n;
QMultiMap<int,Node* > Map_allnodes;
QMap<int,QList<Node*> > finalclass_node;
QList<QList<NeuronSWC> > result_tree_part;
QList<NeuronSWC > result_final_tree;
QMap<int,Node* > root;
QMap<int,QMap<int,QList<Node*> > > number_path;
QMap<int,QMap<int,QList<Node> > > number_path_vn2;
QMultiMap<V3DLONG,QMap<V3DLONG,QList<Node> > > number_path_vn3;
QList<NeuronSWC > final_listNeuron;
QMap<V3DLONG,int > cp_SWC;

QMap<int,QList<Node*> > unit_path;
QMap<int,QList<Node> > unit_path_vn2;//keep the shortest path of all nodes

QList<QList<Node*> > final_path;
QList<QList<Node> > final_path_vn2;

QMap<V3DLONG, NeuronSWC > original_tree;//keep the original tree and insert the shortest path into each two nodes

QMap<V3DLONG,int > Map_finalnode;
QMap<V3DLONG,int > Map_nodes;
QMap<V3DLONG,V3DLONG > Map_allnode;
QList <NeuronSWC > result_list;
QList <NeuronSWC > con_tree;
vector<QList<Node*> > v_List;
QMap<V3DLONG,QList<Node*> > root_class;
QList<Node*> QTest;
int cluster_number=0;
QMap<V3DLONG,QList<V3DLONG> > covered_relation;

QMap<V3DLONG,Node*> node_afterMerge;
QMap<V3DLONG,QMap<V3DLONG,QList<Node> > > arrive_path;
QMap<V3DLONG,int> seed_pos_number;


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
    FILE * fp = fopen(path, "wt");
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
void printSWCByQMap_QMap(char* path,QMap<int,QMap<int,QList<Node*> > > nodeMap)
{
    V3DLONG number=0;
    FILE * fp = fopen(path, "wt");
    if (!fp) return;

    fprintf(fp, "#name\n");
    fprintf(fp, "#comment\n");
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");


    for(QMap<int,QMap<int,QList<Node*> > >::iterator iter1 = nodeMap.begin(); iter1 != nodeMap.end(); iter1++)
    {
        QMap<int,QList<Node*> > temp1=iter1.value();
        for(QMap<int,QList<Node*> >::iterator iter2=temp1.begin();iter2!=temp1.end();iter2++)
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

void printSWCByQMap_QMap_vn2(char* path,QMap<V3DLONG,QMap<V3DLONG,Node > > nodeMap)
{
    V3DLONG number=0;
    FILE * fp = fopen(path, "wt");
    if (!fp) return;

    fprintf(fp, "#name\n");
    fprintf(fp, "#comment\n");
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");


    for(QMap<V3DLONG,QMap<V3DLONG,Node > >::iterator iter1 = nodeMap.begin(); iter1 != nodeMap.end(); iter1++)
    {
        QMap<V3DLONG,Node > temp1=iter1.value();
        for(QMap<V3DLONG,Node >::iterator iter2=temp1.begin();iter2!=temp1.end();iter2++)
        {
            Node temp2=iter2.value();


            fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",number, 1,  temp2.x,  temp2.y,  temp2.z, temp2.r, -1);
            number++;


        }
    }
    fclose(fp);
    return;
}



void printSWCByQList_QList(QList<QList <NeuronSWC> > result_list,char* path)
{
    FILE * fp = fopen(path, "wt");
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
            //QList<Gradient*> grad=gradient(seeds,img1d, sz_x, sz_y, sz_z);//calculate the gradient of every seed
            QList<Gradient*> grad;
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
        fprintf(fp, "%ld %ld %ld %ld %ld %5.3f %d\n",
                number, 30,  elem->x,  elem->y,  elem->z, 1.0, -1);
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
                number, 1,  temp_node->x,  temp_node->y,  temp_node->z, 1.0, -1);
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

#define cal_core(cur,center,radius) exp(-0.1*((cur-center)*(cur-center))/radius*radius)


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




QList <NeuronSWC> construct_tree_vn5(QMap<int,Node* > roots,unsigned char * &img1d,unsigned char * &img2d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG *in_sz)
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
    printf("start breath search!\n");
    //markEdge=bf( root,img1d, average_dis,sz_x, sz_y, sz_z);
    //bf_vn2( root,markEdge,img2d, average_dis,sz_x, sz_y, sz_z);
    bf_vn3( root,markEdge,img2d, average_dis,sz_x, sz_y, sz_z);


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
            QMultiMap<V3DLONG,QMap<V3DLONG,QList<Node> > >::iterator begin=number_path_vn3.lowerBound(loc1);
            QMultiMap<V3DLONG,QMap<V3DLONG,QList<Node> > >::iterator end=number_path_vn3.upperBound(loc1);
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

                //S_vn2=connect_shortest_path_vn4(img1d,temp22,seeds.at(indexi),seeds.at(indexj),sz_x,sz_y,sz_z);
                S_vn2=connect_shortest_path_vn5(img1d,temp22,seeds.at(indexi),seeds.at(indexj),sz_x,sz_y,sz_z,in_sz);
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

QList<NeuronSWC> connect_shortest_path_vn5(unsigned char * &img1d,QList<Node> path,Node* begin,Node* end,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG *in_sz)
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
            //  next_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,temp,sz_x, sz_y, sz_z);

            Node* temp_n=new Node(temp.x,temp.y,temp.z);
            next_S.r=markerRadiusXY(img1d, in_sz, * temp_n, 20);
            delete temp_n;
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
            //end_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(end),sz_x, sz_y, sz_z);

            end_S.r=markerRadiusXY(img1d, in_sz, *end, 20);

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
            // end_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(end),sz_x, sz_y, sz_z);

            end_S.r=markerRadiusXY(img1d, in_sz, * end, 20);

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
                // begin_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(begin),sz_x, sz_y, sz_z);

                begin_S.r=markerRadiusXY(img1d, in_sz, * begin, 20);

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
            //   end_S.r=enlarge_radiusof_single_node_xy_vn2(img1d,getnode(end),sz_x, sz_y, sz_z);

            end_S.r=markerRadiusXY(img1d, in_sz, * end, 20);

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

    V3DLONG step_x=ABS(con_startX-con_endX);
    V3DLONG step_y=ABS(con_startY-con_endY);
    V3DLONG step_z=ABS(con_startZ-con_endZ);

    V3DLONG condition_Xa=ABS(current_x-con_startX);
    V3DLONG condition_Xb=ABS(current_x-con_endX);
    V3DLONG condition_Ya=ABS(current_y-con_startY);
    V3DLONG condition_Yb=ABS(current_y-con_endY);
    V3DLONG condition_Za=ABS(current_z-con_startZ);
    V3DLONG condition_Zb=ABS(current_z-con_endZ);

    if((condition_Xa<=step_x)&&(condition_Xb<=step_x)&&(condition_Ya<=step_y)&&(condition_Yb<=step_y)&&(condition_Za<=step_z)&&(condition_Zb<=step_z))
    {
        return true;


    }else
    {
        return false;

    }

}

bool decide_samebox(V3DLONG offset_x,V3DLONG offset_y,V3DLONG offset_z,Node* node,int step)
{
    V3DLONG start_x=offset_x;
    V3DLONG start_y=offset_y;
    V3DLONG start_z=offset_z;

    V3DLONG end_x=node->x;
    V3DLONG end_y=node->y;
    V3DLONG end_z=node->z;

    V3DLONG step_x=ABS(start_x-end_x);
    V3DLONG step_y=ABS(start_y-end_y);
    V3DLONG step_z=ABS(start_z-end_z);

    for(int times=0;times<26;times++)
    {

        if((step_x==ABS(node_table[times].i*step))&&(step_y==ABS(node_table[times].j*step))&&(step_z==ABS(node_table[times].k*step)))
        {
            return true;

        }
    }
    return false;




}

QList<NeuronSWC> spanning_tree_algorithm(double** markEdge,QList<Node> seeds){
    V3DLONG marknum = seeds.size();
    printf("%ld\n",marknum);

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
    S.x 	= seeds.at(0).x;
    S.y 	= seeds.at(0).y;
    S.z 	= seeds.at(0).z;
    S.r 	= 1;
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
        double min = INF;
        for(int i = 0; i<marknum; i++)//the aim of this is to find the node which flag is 1
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
            S.x 	= seeds.at(indexj).x;
            S.y 	= seeds.at(indexj).y;
            S.z 	= seeds.at(indexj).z;
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
    writeSWC_file("C:\\result\\final_vn5.swc",listNeuron);

    //if(markEdge) {delete []markEdge, markEdge = 0;}
    delete []pi;
    //  writeSWC_file("mst.swc",marker_MST);
    QList<NeuronSWC> marker_MST_sorted;
    if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
        return marker_MST_sorted;
    //  outimg = pImage;
}
QList<NeuronSWC> spanning_without_bf(QMap<V3DLONG,QMap<V3DLONG,Node> >  roots){

    QList<Node> seeds;

    for(QMap<V3DLONG,QMap<V3DLONG,Node> > ::iterator iter1=roots.begin();iter1!=roots.end();iter1++){
        QMap<V3DLONG,Node> temp1=iter1.value();
        for(QMap<V3DLONG,Node>::iterator iter2=temp1.begin();iter2!=temp1.end();iter2++){
            Node temp2=iter2.value();
            seeds.append(temp2);


        }

        V3DLONG marknum = seeds.size();

        double** markEdge = new double*[marknum];
        for(int i = 0; i < marknum; i++)
        {
            markEdge[i] = new double[marknum];

        }

        double x1,y1,z1;
        for (int i=0;i<marknum;i++)
        {
            x1 = seeds.at(i).x;
            y1 = seeds.at(i).y;
            z1 = seeds.at(i).z;
            for (int j=0;j<marknum;j++)
            {
                markEdge[i][j] = sqrt(double(x1-seeds.at(j).x)*double(x1-seeds.at(j).x) + double(y1-seeds.at(j).y)*double(y1-seeds.at(j).y) + double(z1-seeds.at(j).z)*double(z1-seeds.at(j).z));
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
        S.x 	= seeds.at(0).x;
        S.y 	= seeds.at(0).y;
        S.z 	= seeds.at(0).z;
        S.r 	= 1;
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
                            indexj = j;
                        }
                    }
                }

            }
            if(indexi>=0)
            {
                S.n 	= indexj+1;
                S.type 	= 7;
                S.x 	= seeds.at(indexj).x;
                S.y 	= seeds.at(indexj).y;
                S.z 	= seeds.at(indexj).z;
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
        //  writeSWC_file("mst.swc",marker_MST);
        QList<NeuronSWC> marker_MST_sorted;
        if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
            return marker_MST_sorted;
        //  outimg = pImage;
    }
}

void bf_vn3(QMap<int,Node*> roots,double **weight_result,unsigned char *&img1d,double average_dis,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    //found the shortest path between one node and other nodes using breadth first algorithm


    QQueue<double> queue_distance;//keep the unit distance of two nodes
    V3DLONG up_limit;
    V3DLONG down_limit;
    int marknum=roots.size();
    int row=-1;
    int iter_step=2;
    QQueue<Node> queue1;
    QMap<V3DLONG,bool> node_searched;
    QMap<V3DLONG,Node> parent_path_vn2;
    QList<Node> path_shortest_vn2;
    QMap<V3DLONG,QList<Node> > unit_path_vn3;

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
            double stop_distance_vn2=stop_distance-node2->r-node1->r;
            if((stop_distance_vn2>=(30))||stop_distance==0)//stop_distance can be input in the dialogue
            {

                weight_result[row][cloumn]=100000000;

                unit_path_vn3.insert(GET_IND(node2->x,node2->y,node2->z),path_shortest_vn2);

                number_path_vn3.insert(GET_IND(node1->x,node1->y,node1->z),unit_path_vn3);
                continue;

            }else
            {

                V3DLONG node_number=(ABS(start_x-end_x)+1)*(ABS(start_y-end_y)+1)*(ABS(start_z-end_z)+1);
                V3DLONG count=0;
                V3DLONG offset_x=start_x;
                V3DLONG offset_y=start_y;
                V3DLONG offset_z=start_z;
                V3DLONG offset_dis=0;


                queue1.append(getnode(node1));



                //need to acclerate, change to a function which can tell the current and the end node belong to a same box
                //here exists arguement!

                while((!decide_samebox(offset_x,offset_y,offset_z,node2,iter_step)&&(queue1.size()!=0)&&(count<=node_number)))
                {

                    Node temp1=queue1.head();//get the first node in the queue every loop

                    //need to set a flag here means the node was searched
                    for(int times=0;times<26;times++)
                    {
                        //printf("%lf",node_table[times].dist);
                        offset_x=temp1.x+node_table[times].i*iter_step;
                        offset_y=temp1.y+node_table[times].j*iter_step;
                        offset_z=temp1.z+node_table[times].k*iter_step;
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
                            //if(node_searched[GET_IND(offset_x,offset_y,offset_z)]!=true)//change the code here
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

                            }else{
                                continue;

                            }

                            if(decide_samebox(offset_x,offset_y,offset_z,node2,iter_step))
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
                    weight_result[row][cloumn]=stop_distance_vn2;
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
    QMap<V3DLONG,QList<Node> > unit_path_vn3;

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


                V3DLONG node_number=(ABS(start_x-end_x)+1)*(ABS(start_y-end_y)+1)*(ABS(start_z-end_z)+1);
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

void change_NeuronSWCTo_MyMarker(QList<NeuronSWC> swc_tree, vector<MyMarker*> &tmpswc)
{

    QMap<int,MyMarker*> Map_index;
    QList<NeuronSWC> tree;
    int parent_n;

    for(int i=0;i<swc_tree.size();i++)
    {
        NeuronSWC temp=swc_tree.at(i);
        MyMarker* subnode=new MyMarker(temp.x,temp.y,temp.z);
        subnode->radius=temp.r;
        subnode->type=temp.type;
        //tmpswc.push_back(subnode);
        Map_index.insert(i+1,subnode);

    }
    for(int j=0;j<swc_tree.size();j++)
    {

        NeuronSWC temp1=swc_tree.at(j);
        if(temp1.parent==-1)
        {
            tmpswc.push_back(Map_index.value(j+1));

        }else
        {
            for(int k=0;k<swc_tree.size();k++)
            {
                NeuronSWC temp2=swc_tree.at(k);
                if(temp1.parent==temp2.n)
                {
                    parent_n=temp2.n;
                    break;

                }

            }
            Map_index.value(j+1)->parent=Map_index.value(parent_n);
            tmpswc.push_back(Map_index.value(j+1));

        }


    }


}


int meanshift_plugin_vn4(V3DPluginCallback2 &callback, QWidget *parent,unsigned char* img1d,V3DLONG *in_sz, QString &image_name,bool bmenu,input_PARA &PARA)
{
    ////////////////////////////////////////////////////////////
    //这里需要进行第一次去噪，去掉一些像素小的点，但其他点的像素基本上不改变
    V3DLONG sz_x = in_sz[0];
    V3DLONG sz_y = in_sz[1];
    V3DLONG sz_z = in_sz[2];
    flag=new int [sz_x*sz_y*sz_z];
    //unsigned char* nData1=new unsigned char[sz_x*sz_y*(sz_z)];

    V3DLONG r=5;
    V3DLONG count=0;
    int times=50;
    //this method should be optimized
    int iteration=100;//this variable is used to find the path between roots
    int num_mark=1;
    int nstep=1;

    double threshold=PARA.threshold;
    double percentage_rate=PARA.percentage;
    double prim_distance=PARA.prim_distance;
    //printf("%lf %lf %lf \n",threshold,percentage_rate,prim_distance);
    QMultiMap<V3DLONG,Node> cluster_result;
    QMap<V3DLONG,QList<Node> > final_cluster_result;

    // printf("### find out rootnode  ###\n");


    /* for(V3DLONG ii=0;ii<sz_x;ii=ii+nstep)
    {
        for(V3DLONG jj=0;jj<sz_y;jj=jj+nstep)
        {
            for(V3DLONG kk=0;kk<sz_z;kk=kk+nstep)
            {
                V3DLONG ind=GET_IND(ii,jj,kk);

                if((double)img1d[ind]<30)
                    continue;//take the node whose intensity more than 30 and cannot shift anymore as rootnode
                if(!found_final_vn3(img1d,ii,jj,kk,sz_x, sz_y, sz_z, r))
                {
                    Node* final=new Node(ii,jj,kk);

                    final->class_mark=num_mark;
                    final->parent=-1;
                    final->number=0;
                    Map_finalnode.insert(ind,num_mark);
                    // printf("%ld\n",ind);
                    //enlarge_radiusof_single_node_xy(img1d,final,sz_x, sz_y, sz_z);
                    final->r= markerRadiusXY(img1d, in_sz, *final, 40);
                    Map_rootnode.insert(num_mark,final);
                    //Map_rootnode_n.insert(num_mark,getnode(final));
                    if((double)img1d[GET_IND(ii,jj,kk)]==0)
                        printf("%lf\n",(double)img1d[GET_IND(ii,jj,kk)]);
                    //Map_rootnode_n.insert(num_mark,getnode(final));
                    num_mark++;
                    //delete final;

                }

            }
        }
    }*/


    //  printf("###  rootnode found   ###\n");
    printf("###  cluster nodes using meanshift   ###\n");

    for(V3DLONG i=0;i<sz_x;i++)
    {
        for(V3DLONG j=0;j<sz_y;j++)
        {
            for(V3DLONG k=0;k<sz_z;k++)
            {
                V3DLONG ind=GET_IND(i,j,k);


                // if((double)img1d[ind]<30)//here can design a method of determining noisy node by check the pixal value of connected area
                // continue;//try to cluster every node whose intensity is more than 0,but if the intensity is less than 30, we regard it as noise node;

                if(!determine_noisy(img1d,i,j,k,sz_x,sz_y,sz_z,threshold,percentage_rate,nstep)||((double)img1d[ind]==0)){
                    continue;
                }
                /* if(flag[ind]==1)
                {
                    continue;
                }*/

                //meanshift_vn4(img1d,i,j,k,sz_x,sz_y,sz_z,r,times);
                // meanshift_vn5(img1d,i,j,k,sz_x,sz_y,sz_z,r,times,in_sz);
                long cluster_node= meanshift_vn7(img1d,i,j,k,sz_x,sz_y,sz_z,r,times,in_sz);//cluster_node equals -1 or the position number got by GET_IND in the map
                Node* single_node=new Node(i,j,k);
                cluster_result.insert(cluster_node,getnode(single_node));
                delete single_node;

            }
        }
    }
    printSwcByMap(Map_rootnode,"C:\\result\\final.swc");
    //delete []flag;
    //printSWCByMap_List(Map_finalnode_list,"D:\\result\\compare.swc");
    printf("###  cluster finished   ###\n");
    // printf("%d  %d\n",Map_finalnode.count(),Map_rootnode.count());
    printf("###  merge and delete roots according to radius and distance    ###");

    root= merge_rootnode_vn2(Map_rootnode,img1d,sz_x,sz_y,sz_z,prim_distance);//delete some root which covered by others
    printf("###  merge and delete roots finished    ###\n");
    printf("###  begin to cluster the non-roots    ###\n");
    final_cluster_result=cluster2newroot(covered_relation,cluster_result);//assemble every node(not noisy) to the remaining new root

    // printf("%d\n",final_cluster_result.count());
    printSwcByMap(root,"C:\\result\\final_vn2.swc");
    //printf("root::%d\n",root.size());
    printf("###  begin to delete the     ###\n");
    QMap<int,Node*> after_merge_final=delete_cluster_node(img1d,final_cluster_result,sz_x,sz_y,sz_z,in_sz,prim_distance);//for every new root, delete the corresponding suboridinate nodes which could be covered by each other
    printf("###   connect all nodes using an improved spanning tree which using a sphere model to calculate distance\n");
    //printf("after_merge_final::%d\n",after_merge_final.size());
    printSwcByMap(after_merge_final,"C:\\result\\final_vn3.swc");
    QMap<V3DLONG,QMap<V3DLONG,QList<Node> > > path_between_each_roots;
    int threshold_node_number=200;//this is a bug, not very reasonable, fix it later-20160203
    if(after_merge_final.size()<threshold_node_number){
        path_between_each_roots=sphere_search(img1d,after_merge_final,sz_x,sz_y,sz_z,2.0,iteration);
        // printf("%d\n",path_between_each_roots.count());
        //we need to calculate the distance based on path_between_each_roots
        printf("###  path finding finished     ###\n");
        // QMap<V3DLONG,QList<Node> >  node_adj_node=build_adj_matrix(path_between_each_roots,node_con_node);//output:QMap<V3DLONG,QList<Node> >  the adjecent node of node A should be included in QList
        //should define a connect algorithm
        printf("###  calculate the distance between nodes and connect them     ###\n");
        con_tree=calculate_distance_between_roots(path_between_each_roots,after_merge_final,sz_x,sz_y,sz_z);
        //printf("2222222222222222222222\n");
        printf("###   nodes connected   ###\n");

    }else{
        path_between_each_roots=sphere_search(img1d,root,sz_x,sz_y,sz_z,2.0,iteration);
        // printf("%d\n",path_between_each_roots.count());
        //we need to calculate the distance based on path_between_each_roots
        printf("###  path finding finished     ###\n");
        // QMap<V3DLONG,QList<Node> >  node_adj_node=build_adj_matrix(path_between_each_roots,node_con_node);//output:QMap<V3DLONG,QList<Node> >  the adjecent node of node A should be included in QList
        //should define a connect algorithm
        printf("###  calculate the distance between nodes and connect them     ###\n");
        con_tree=calculate_distance_between_roots(path_between_each_roots,root,sz_x,sz_y,sz_z);
        //printf("2222222222222222222222\n");
        printf("###   nodes connected   ###\n");

    }

    QList <NeuronSWC > con_tree_final=fulfill_path_between_roots(con_tree,path_between_each_roots,sz_x,sz_y,sz_z);

    writeSWC_file("C:\\result\\final_vn5.swc",con_tree);
    vector<MyMarker*>  inswc;
    change_NeuronSWCTo_MyMarker(con_tree_final,inswc);

    QString initswc_file = image_name + "init_meanshift.swc";
    saveSWC_file(initswc_file.toStdString(), inswc);

    cout<<"Pruning neuron tree"<<endl;

    vector<MyMarker*> outswc;
    v3d_msg("start to use happ.\n", 0);
    happ(inswc, outswc, img1d, in_sz[0], in_sz[1], in_sz[2],1, 1, 0.11111);
    std::vector<NeuronSWC *> target;

    QString outswc_file = image_name + "_meanshift.swc";
    saveSWC_file(outswc_file.toStdString(), outswc);

    printf("%s\n",outswc_file.toStdString().c_str());
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outswc_file.toStdString().c_str()),bmenu);
    return -1;

}

QList<NeuronSWC> fulfill_path_between_roots(QList<NeuronSWC> con_tree,QMap<V3DLONG,QMap<V3DLONG,QList<Node> > > path_between_each_roots,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    V3DLONG temp_child;
    V3DLONG temp_pn;
    QMap<V3DLONG,V3DLONG> child_parent;
    QList<NeuronSWC> result;
    QMap<V3DLONG,NeuronSWC> con_tree_Map;
    QMap<V3DLONG,int> order_con_tree;
    FILE * fp = fopen("C:\\result\\final6.swc", "wt");
    if (!fp) return result;

    for(int i=0;i<con_tree.size();i++){
        NeuronSWC child_node=con_tree.at(i);
        V3DLONG child_node_index=GET_IND(child_node.x,child_node.y,child_node.z);
        if(child_node.pn==-1){
            child_parent.insert(child_node_index,-1);

        }else{
            NeuronSWC parent_node=con_tree.at((int)child_node.pn-1);
            V3DLONG parent_node_index=GET_IND(parent_node.x,parent_node.y,parent_node.z);
            child_parent.insert(child_node_index,parent_node_index);
        }
        con_tree_Map.insert(child_node_index,child_node);
        order_con_tree.insert(child_node_index,i+1);

    }
    printf("con_tree::%d\n",con_tree.size());
    printf("child_parent::%d\n",child_parent.size());
    int count_number=1;
    QMap<int,NeuronSWC> path_SWC_roots;
    QMap<V3DLONG,int> root_index;

    /* for(int i=0;i<con_tree.size();i++){
        //seed_pos_number;
        NeuronSWC temp_child=con_tree.at(i);
        V3DLONG temp_index_child=GET_IND(temp_child.x,temp_child.y,temp_child.z);
        if(temp_child.pn==-1)
            continue;
        NeuronSWC temp_parent=con_tree.at((int)temp_child.pn-1);
        V3DLONG temp_index_parent=GET_IND(temp_parent.x,temp_parent.y,temp_parent.z);
        //path_between_each_roots.contains(temp_index);
        if(arrive_path.value(temp_index_child).contains(temp_index_parent)||arrive_path.value(temp_index_parent).contains(temp_index_child))
        {//have a path
            if(seed_pos_number.value(temp_index_child)<seed_pos_number.value(temp_index_parent)){
                QList<NeuronSWC> temp_list=arrive_path.value(temp_index_child).value(temp_index_parent);
                for(int i=0;i<temp_list.size();i++)
                {
                    //write
                    NeuronSWC temp2=temp_list.at(i);
                    V3DLONG temp2_index=GET_IND(temp2.x,temp2.y,temp2.z);
                    if(temp2.pn==-1){
                        temp2.n=count_number;
                        path_SWC_roots.insert(count_number,temp2);
                        root_index.insert(temp2_index,count_number);
                        //fprintf(fp, "%ld %d %f %f %f %f %ld\n",count_number, temp2.type,  temp2.x,  temp2.y,  temp2.z, 1.0, -1);

                    }else{
                        temp2.n=count_number;
                        temp2.pn=count_number-1;
                        path_SWC_roots.insert(count_number,temp2);
                        root_index.insert(temp2_index,count_number);
                        // fprintf(fp, "%ld %d %f %f %f %f %ld\n",count_number, temp2.type,  temp2.x,  temp2.y,  temp2.z, 1.0, count_number-1);
                    }
                    count_number++;
                }


            }else{
                QList<NeuronSWC> temp_list1=arrive_path.value(temp_index_parent).value(temp_index_child);
                for(int j=0;j<temp_list1.size();j++){
                    //write
                    NeuronSWC temp1=temp_list1.at(j);
                    V3DLONG temp1_index=GET_IND(temp1.x,temp1.y,temp1.z);
                    if(temp1.pn==-1){
                        temp1.n=count_number;
                        path_SWC_roots.insert(count_number,temp1);
                        root_index.insert(temp1_index,count_number);
                        //fprintf(fp, "%ld %d %f %f %f %f %ld\n",count_number, temp1.type,  temp1.x,  temp1.y,  temp1.z, 1.0, -1);

                    }else{
                        temp1.n=count_number;
                        temp1.pn=count_number-1;
                        path_SWC_roots.insert(count_number,temp1);
                        root_index.insert(temp1_index,count_number);
                        //fprintf(fp, "%ld %d %f %f %f %f %ld\n",count_number, temp1.type,  temp1.x,  temp1.y,  temp1.z, 1.0, count_number-1);

                    }

                    count_number++;
                }


            }

        }else{//have no pa
            temp_child.n=count_number;
            temp_child.pn=-1;
            path_SWC_roots.insert(count_number,temp_child);
            root_index.insert(temp_index_child,count_number);
            count_number++;
            //printf("something wrong!");
        }


    }*/
    int child_order;
    int parent_order;
    // NeuronSWC temp_node;
    /*  for(QMap<V3DLONG,V3DLONG>::iterator iter1=child_parent.begin();iter1!=child_parent.end();iter1++)
    {
        V3DLONG parent_index=iter1.value();
        V3DLONG child_index=iter1.key();
        if(parent_index==-1)
        {
            child_order=root_index.value(child_index);
           NeuronSWC temp_node1=path_SWC_roots.value(child_order);
            temp_node1.pn=-1;
            path_SWC_roots.remove(child_order);
            path_SWC_roots.insert(child_order,temp_node1);
        }else
        {
            child_order=root_index.value(child_index);
            parent_order=root_index.value(parent_index);
            NeuronSWC temp_node=path_SWC_roots.value(child_order);
            if(temp_node.pn==-1){
                path_SWC_roots.remove(child_order);
                temp_node.pn=parent_order;
                path_SWC_roots.insert(child_order,temp_node);
            }

        }

    }*/
    int length=con_tree.size();
    Node node_parent;
    Node node_child;
    V3DLONG index_child;
    V3DLONG index_parent;
    for(int i=0;i<length;i++)
    {
        NeuronSWC temp_child=con_tree.at(i);
        V3DLONG index_child=GET_IND(temp_child.x,temp_child.y,temp_child.z);
        if(temp_child.pn==-1){
            continue;
        }
        NeuronSWC temp_parent=con_tree.at((int) temp_child.pn-1);
        V3DLONG index_parent=GET_IND(temp_parent.x,temp_parent.y,temp_parent.z);\
        if(arrive_path.value(index_child).contains(index_parent)||arrive_path.value(index_parent).contains(index_child))
        {
            if(seed_pos_number.value(index_child)<seed_pos_number.value(index_parent)){//from child to parent
                QList<Node> temp_list=arrive_path.value(index_child).value(index_parent);
                if(temp_list.size()==0){
                    printf("8888888888888\n");
                    continue;
                }
                if((GET_IND(temp_list.first().x,temp_list.first().y,temp_list.first().z)==index_child)&&(GET_IND(temp_list.last().x,temp_list.last().y,temp_list.last().z)==index_parent))
                {


                }else{
                    printf("1:something wrong\n");

                }
                for(int i=temp_list.size()-1;i>=0;i--)
                {

                    if(i==temp_list.size()-1){
                        continue;
                    }
                    node_parent=temp_list.at(i+1);
                    node_child=temp_list.at(i);
                    index_child=GET_IND(node_child.x,node_child.y,node_child.z);
                    index_parent=GET_IND(node_parent.x,node_parent.y,node_parent.z);
                    if(con_tree_Map.contains(index_child)){
                        NeuronSWC temp_child=con_tree_Map.value(index_child);
                        temp_child.pn=order_con_tree.value(index_parent);
                        con_tree_Map.insert(index_child,temp_child);

                    }else{

                        NeuronSWC S;
                        S.n=con_tree.size()+1;
                        S.x=node_child.x;
                        S.y=node_child.y;
                        S.z=node_child.z;
                        S.type=20;
                        S.pn=order_con_tree.value(index_parent);
                        con_tree.append(S);
                        con_tree_Map.insert(index_child,S);
                        order_con_tree.insert(index_child,S.n);
                        child_parent.insert(index_child,index_parent);
                    }

                }


            }else{
                QList<Node> temp_list1=path_between_each_roots.value(index_parent).value(index_child);//from parent to child
                if(temp_list1.size()==0){
                    printf("99999999999999999999\n");
                    continue;
                }
                if((GET_IND(temp_list1.first().x,temp_list1.first().y,temp_list1.first().z)==index_parent)&&(GET_IND(temp_list1.last().x,temp_list1.last().y,temp_list1.last().z)==index_child))
                {


                }else{
                    printf("2:something wrong\n");

                }
                for(int i=0;i<temp_list1.size();i++)
                {
                    if(i==0){
                        continue;
                    }
                    node_parent=temp_list1.at(i-1);
                    node_child=temp_list1.at(i);
                    index_child=GET_IND(node_child.x,node_child.y,node_child.z);
                    index_parent=GET_IND(node_parent.x,node_parent.y,node_parent.z);
                    if(con_tree_Map.contains(index_child)){
                        NeuronSWC temp_child1=con_tree_Map.value(index_child);
                        temp_child1.pn=order_con_tree.value(index_parent);
                        con_tree_Map.insert(index_child,temp_child1);

                    }else{

                        NeuronSWC S1;
                        S1.n=con_tree.size()+1;
                        S1.x=node_child.x;
                        S1.y=node_child.y;
                        S1.z=node_child.z;
                        S1.type=20;
                        S1.pn=order_con_tree.value(index_parent);
                        con_tree.append(S1);
                        con_tree_Map.insert(index_child,S1);
                        order_con_tree.insert(index_child,S1.n);
                        child_parent.insert(index_child,index_parent);
                    }

                }

            }
        }
    }


    //write path_SWC_roots here
    /*for(QMap<int,NeuronSWC>::iterator iter2=path_SWC_roots.begin();iter2!=path_SWC_roots.end();iter2++){
        NeuronSWC write_node=iter2.value();
        fprintf(fp, "%ld %d %f %f %f %f %ld\n",write_node.n, write_node.type,  write_node.x,  write_node.y,  write_node.z, 1.0, write_node.pn);
    }*/
    for(QMap<V3DLONG,NeuronSWC>::iterator iter2=con_tree_Map.begin();iter2!=con_tree_Map.end();iter2++)
    {
        NeuronSWC write_node=iter2.value();
        fprintf(fp, "%ld %d %f %f %f %f %ld\n",write_node.n, write_node.type,  write_node.x,  write_node.y,  write_node.z, 1.0, write_node.pn);
    }
    for(int ii=0;ii<con_tree.size();ii++){
        NeuronSWC temp_result=con_tree.at(ii);
        V3DLONG temp_result_index=GET_IND(temp_result.x,temp_result.y,temp_result.z);
        //temp_node.r= markerRadiusXY(img1d, in_sz, *new_temp_node, 20);
        result.append(con_tree_Map.value(temp_result_index));

    }
    QList<NeuronSWC> marker_MST_sorted;
    SortSWC(result, marker_MST_sorted ,1, 0);
    //return marker_MST_sorted;


    fclose(fp);

    return marker_MST_sorted;

}



QMap<V3DLONG,QList<Node> >  check_node_conection(QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > path_between_each_roots,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    QMap<V3DLONG,QList<Node> >  result;
    for(QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > >::iterator iter1=path_between_each_roots.begin();iter1!=path_between_each_roots.end();iter1++){
        QMultiMap<V3DLONG,QList<Node> >  temp=iter1.value();
        QList<Node> connect_list;
        connect_list.clear();
        for(QMultiMap<V3DLONG,QList<Node> > ::iterator iter2=temp.begin();iter2!=temp.end();iter2++){
            QList<Node> temp_list=iter2.value();
            V3DLONG node_first=GET_IND(temp_list.first().x,temp_list.first().y,temp_list.first().z);
            V3DLONG node_terminal=GET_IND(temp_list.last().x,temp_list.last().y,temp_list.last().z);
            if(node_first==iter1.key()&&node_terminal==iter2.key()){
                connect_list.append(temp_list.last());
            }
        }
        result.insert(iter1.key(),connect_list);
    }
    return result;
}

void printf_path_between_roots(QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > root_path,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    FILE * fp = fopen("C:\\result\\final_vn4.swc", "wt");
    if (!fp) return;

    fprintf(fp, "#name\n");
    fprintf(fp, "#comment\n");
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    for(QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > >::iterator iter1=root_path.begin();iter1!=root_path.end();iter1++){
        QMultiMap<V3DLONG,QList<Node> >  temp1=iter1.value();
        int number=0;
        for(QMultiMap<V3DLONG,QList<Node> > ::iterator iter2=temp1.begin();iter2!=temp1.end();iter2++){
            QList<Node> temp2=iter2.value();
            if((iter1.key()==GET_IND(temp2.first().x,temp2.first().y,temp2.first().z))&&(iter2.key()==GET_IND(temp2.last().x,temp2.last().y,temp2.last().z))){
                for(int i=0;i<temp2.size();i++){
                    fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",number, 1,  temp2.at(i).x,  temp2.at(i).y,  temp2.at(i).z, 1.0, -1);
                    number++;//for test
                }
            }

        }


    }
    fclose(fp);
    return;

}

QList<Node> change_type_to_QList(QMap<V3DLONG,QMap<V3DLONG,Node> >  roots){
    QList<Node> seeds;
    for(QMap<V3DLONG,QMap<V3DLONG,Node> > ::iterator iter1=roots.begin();iter1!=roots.end();iter1++){
        QMap<V3DLONG,Node> temp1=iter1.value();
        for(QMap<V3DLONG,Node>::iterator iter2=temp1.begin();iter2!=temp1.end();iter2++){
            Node temp2=iter2.value();
            seeds.append(temp2);

        }
    }
    return seeds;

}

QList <NeuronSWC > calculate_distance_between_roots(QMap<V3DLONG,QMap<V3DLONG,QList<Node> > > path_between_each_roots,QMap<int,Node* > roots,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    //should define a QList which include all roots or nodes, and calculate the distance in order based on  path_between_each_roots
    int marknum=path_between_each_roots.count();
    //printf("%d\n",marknum);
    double** markEdge = new double*[marknum];
    for(int i = 0; i < marknum; i++)
    {
        markEdge[i] = new double[marknum];

    }
    QList<Node> seeds_temp;
    //delete the node which can not arrive others
    for(QMap<V3DLONG,QMap<V3DLONG,QList<Node> > >::iterator iter_item1 =path_between_each_roots.begin(); iter_item1 != path_between_each_roots.end(); iter_item1++){
        QMap<V3DLONG,QList<Node> >  root_begin=iter_item1.value();
        bool flag=false;
        for(QMap<V3DLONG,QList<Node> > ::iterator iter_item2=root_begin.begin();iter_item2!=root_begin.end();iter_item2++){
            QList<Node> path=iter_item2.value();
            if((iter_item1.key()==GET_IND(path.first().x,path.first().y,path.first().z))&&(iter_item2.key()==GET_IND(path.last().x,path.last().y,path.last().z))){
                flag=true;

            }

        }
        if(!flag){
            root_begin.clear();
            path_between_each_roots.insert(iter_item1.key(),root_begin);
        }

    }
    for(QMap<int,Node* >::iterator iter1 =roots.begin(); iter1 != roots.end(); iter1++){
        Node* temp=iter1.value();
        V3DLONG temp_index=GET_IND(temp->x,temp->y,temp->z);
        if(path_between_each_roots.value(temp_index).size()==0){//delete those node which cannot arrive others, this method may not correct, fix it later
            //printf("6666666666666\n");
            continue;

        }else{
            seeds_temp.append(getnode(temp));
        }


    }
    for(int i=0;i<seeds_temp.size()-1;i++)
    {
        V3DLONG temp1_v3dlong=GET_IND(seeds_temp.at(i).x,seeds_temp.at(i).y,seeds_temp.at(i).z);
        Node temp1=seeds_temp.at(i);
        seed_pos_number.insert(temp1_v3dlong,i);
        QMap<V3DLONG,QList<Node> >  node_path;
        node_path.clear();
        for(int j=i+1;j<seeds_temp.size();j++)
        {
            V3DLONG temp2_v3dlong=GET_IND(seeds_temp.at(j).x,seeds_temp.at(j).y,seeds_temp.at(j).z);
            Node temp2=seeds_temp.at(j);
            QList<Node> distance_list=path_between_each_roots.value(temp1_v3dlong).value(temp2_v3dlong);
            markEdge[i][j]=0;
            if(distance_list.size()==1){
                printf("1111111111111:%ld  %ld\n",temp1,temp2);

            }
            if((temp1_v3dlong==GET_IND(distance_list.first().x,distance_list.first().y,distance_list.first().z))&&(temp2_v3dlong==GET_IND(distance_list.last().x,distance_list.last().y,distance_list.last().z))){
                // printf("11111111111111111111111111111111111111111\n");
                //QList<Node> listNeuron;
                //listNeuron.clear();
                for(int k=0;k<distance_list.size()-1;k++){
                    Node temp1=distance_list.at(k);
                    Node temp2=distance_list.at(k+1);
                    double distance=(double)sqrt((double)(temp1.x-temp2.x)*(temp1.x-temp2.x)+(temp1.y-temp2.y)*(temp1.y-temp2.y)+(temp1.z-temp2.z)*(temp1.z-temp2.z));
                    markEdge[i][j]=markEdge[i][j]+distance;

                }
                node_path.insert(temp2_v3dlong,distance_list);
            }else{
                //if we cannot find the path between two roots in limited steps, we define the distance between two roots as the direct space distance or INF;
                //markEdge[i][j]=INF;//one choice
                markEdge[i][j]=double(sqrt((double)((temp1.x-temp2.x)*(temp1.x-temp2.x)+(temp1.y-temp2.y)*(temp1.y-temp2.y)+(temp1.z-temp2.z)*(temp1.z-temp2.z))));

            }
        }
        if(node_path.size()!=0){
            arrive_path.insert(temp1_v3dlong,node_path);

        }


    }
    // printf("%d\n",marknum);


    /*  for(QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > >::iterator iter1 =path_between_each_roots.begin(); iter1 != path_between_each_roots.end(); iter1++)
    {
        QMultiMap<V3DLONG,QList<Node> >  elem1=iter1.value();
        //  printf("sub node list::%d\n",elem1.count());
        indexj=0;
        for(QMultiMap<V3DLONG,QList<Node> > ::iterator iter2 =elem1.begin(); iter2!= elem1.end(); iter2++)
        {
            QList<Node> elem2=iter2.value();

            markEdge[indexi][indexj]=0;
            if((iter1.key()==GET_IND(elem2.first().x,elem2.first().y,elem2.first().z))&&(iter2.key()==GET_IND(elem2.last().x,elem2.last().y,elem2.last().z))){
                //printf("11111111111111111111111111111111111111111\n");
                for(int i=0;i<elem2.size()-1;i++){
                    Node temp1=elem2.at(i);
                    Node temp2=elem2.at(i+1);
                    double distance=(double)sqrt((double)(temp1.x-temp2.x)*(temp1.x-temp2.x)+(temp1.y-temp2.y)*(temp1.y-temp2.y)+(temp1.z-temp2.z)*(temp1.z-temp2.z));
                    markEdge[indexi][indexj]=markEdge[indexi][indexj]+distance;
                }


            }else{
                //if we cannot find the path between two roots in limited steps, we define the distance between two roots as the direct space distance or INF;
                markEdge[indexi][indexj]=INF;//one choice

                continue;
            }
            indexj++;


        }
        indexi++;
        printf("%d %d\n",indexi,indexj);

    }*/



    for(int i=0;i<marknum;i++){
        for(int j=0;j<marknum;j++){
            if(i==j){
                markEdge[i][j]=0;

            }else if(i>j){
                markEdge[i][j]=markEdge[j][i];

            }

        }

    }
    path_between_each_roots.clear();
    //write DFS algorithm here to construct a SWC tree
    //con_tree=DFS_construct(markEdge,seeds_temp,sz_x,sz_y,sz_z);

    con_tree=spanning_tree_algorithm(markEdge,seeds_temp);
    if(markEdge) {delete []markEdge, markEdge = 0;}

    return con_tree;

}
QList <NeuronSWC> DFS_construct(double** markEdge,QList<Node> seeds,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    int **adj_mark=new int*[seeds.size()];
    int *visit_mark=new int[seeds.size()];
    bool **path_mark=new bool*[seeds.size()];

    QMap<V3DLONG,int> new_seeds;
    new_seeds.clear();
    QMap<V3DLONG,int> root_number;//every root has a number in order (line number in SWC)
    QMap<int,QMap<int,bool> >  path_visited;
    root_number.clear();
    NeuronTree marker_MST;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    for(int i=0;i<seeds.size();i++){
        adj_mark[i]=new int[seeds.size()];
        path_mark[i]=new bool[seeds.size()];
        new_seeds.insert(GET_IND(seeds.at(i).x,seeds.at(i).y,seeds.at(i).z),i);
        visit_mark[i]=-1;//initialize visit mark

    }
    for(int i=0;i<seeds.size();i++){

        for(int j=0;j<seeds.size();j++){

            path_mark[i][j]=false;

            if(markEdge[i][j]>0){
                adj_mark[i][j]=1;//means there is a path between two nodes
            }else{
                adj_mark[i][j]=0;//means there is no node between two nodes
            }
        }

    }
    QStack<Node> stack;
    // QMap<int,QList<int> >  connectship;//represent ith node connect with other jth nodes
    QList<int> connected_node;
    NeuronSWC S_root;
    S_root.n 	= 1;
    S_root.type 	= 3;
    S_root.x 	= seeds.at(0).x;
    S_root.y 	= seeds.at(0).y;
    S_root.z 	= seeds.at(0).z;
    S_root.r 	= 1;
    S_root.pn 	= -1;
    listNeuron.append(S_root);
    hashNeuron.insert(S_root.n, listNeuron.size()-1);
    root_number.insert(GET_IND(S_root.x,S_root.y,S_root.z),S_root.n);

    for(int k=0;k<seeds.size();k++){
        stack.push(seeds.at(k));
        if(!root_number.contains(GET_IND(seeds.at(k).x,seeds.at(k).y,seeds.at(k).z))){
            NeuronSWC S1;
            S1.n 	= listNeuron.size()+1;
            S1.type 	= 3;
            S1.x 	= seeds.at(k).x;
            S1.y 	= seeds.at(k).y;
            S1.z 	= seeds.at(k).z;
            S1.r 	= 1;
            S1.pn 	= listNeuron.size()+1;
            listNeuron.append(S1);
            hashNeuron.insert(S1.n, listNeuron.size()-1);
            root_number.insert(GET_IND(S1.x,S1.y,S1.z),S1.n);
            visit_mark[new_seeds.value(GET_IND(S1.x,S1.y,S1.z))]=1;
        }
        printf("222222222222222222222222\n");
        int pre_node=-1;
        while(!stack.empty()){
            Node first=stack.pop();
            int begin_node=new_seeds.value(GET_IND(first.x,first.y,first.z));
            visit_mark[begin_node]=1;
            connected_node.clear();
            //visit_mark[begin_node]=1;//means visited
            for(int j=0;j<seeds.size();j++)
            {
                if(adj_mark[begin_node][j]==0){//here can be insted by markEdge
                    continue;
                }else{
                    connected_node.append(j);
                }
            }
            //connectship.insert(begin_node,connected_node);
            if(connected_node.size()==0){
                continue;
            }
            int num=-1;
            double minimum=INF;
            for(int i=0;i<connected_node.size();i++){
                int terminal_node=connected_node.at(i);
                if((minimum>markEdge[begin_node][terminal_node])){//need to garantee the selected node is not the former node
                    minimum=markEdge[begin_node][terminal_node];
                    num=terminal_node;
                }
            }
            printf("111111111111111111111111111111:%d\n",num);
            if(num==-1){
                continue;
            }else if((path_mark[begin_node][num]==false)){
                path_mark[begin_node][num]=true;
                path_mark[num][begin_node]=true;
                visit_mark[num]=1;

                stack.push(seeds.at(num));
                NeuronSWC S2;
                S2.n 	= listNeuron.size()+1;
                S2.type 	= 3;
                S2.x 	= seeds.at(num).x;
                S2.y 	= seeds.at(num).y;
                S2.z 	= seeds.at(num).z;
                S2.r 	= 1;
                S2.pn 	= root_number.value(GET_IND(first.x,first.y,first.z));
                listNeuron.append(S2);
                hashNeuron.insert(S2.n, listNeuron.size()-1);
                root_number.insert(GET_IND(S2.x,S2.y,S2.z),S2.n);

            }
            /* else if(path_visited.value(begin_node).value(num)==true){
                NeuronSWC S;
                S.n 	= listNeuron.size()+1;
                S.type 	= 3;
                S.x 	= seeds.at(begin_node).x;
                S.y 	= seeds.at(begin_node).y;
                S.z 	= seeds.at(begin_node).z;
                S.r 	= 1;
                S.pn 	= root_number.value(GET_IND(seeds.at(num).x,seeds.at(num).y,seeds.at(num).z));
                listNeuron.append(S);
                hashNeuron.insert(S.n, listNeuron.size()-1);
                root_number.insert(GET_IND(S.x,S.y,S.z),S.n);

            }*/
            pre_node=begin_node;
        }

    }
    if(adj_mark) {delete []adj_mark, adj_mark = 0;}
    if(visit_mark) {delete visit_mark;}
    if(path_mark) {delete [] path_mark,path_mark=0;}
    return listNeuron;

}

double Map_coordinate(Node current_center,Node target_node,V3DLONG relate_x,V3DLONG relate_y,V3DLONG relate_z)
{

    Node* new_y_axis=new Node(target_node.x-current_center.x,target_node.y-current_center.y,target_node.z-current_center.z);
    //Node new_y_axis=new Node(current_center.x-target_node.x,current_center.y-target_node.y,current_center.z-target_node.z);
    //double unit_length_x=1/sqrt((double)(target_node.x-current_center.x)*(target_node.x-current_center.x)+(target_node.y-current_center.y)*(target_node.y-current_center.y)+(target_node.z-current_center.z)*(target_node.z-current_center.z));
    // Node* new_y_axis_unit=new Node((target_node.x-current_center.x)*unit_length_x,(target_node.y-current_center.y)*unit_length_x,(target_node.z-current_center.z)*unit_length_x);
    //rotating order:first x then z
    //projection of the new Y axis in the yoz panel
    // printf("%ld %ld %ld\n",new_y_axis->x,new_y_axis->y,new_y_axis->z);
    double angle_y_rotate=0;
    if(new_y_axis->y==0){//rotate Y axis to X axis
        angle_y_rotate=0;

    }else{
        angle_y_rotate=(1*new_y_axis->y)/sqrt((double)(new_y_axis->y*new_y_axis->y+new_y_axis->z*new_y_axis->z));//the cos(angle) between old y axis and projection of the new Y axis in the yoz panel

    }

    //printf("%lf %ld\n",angle_y_rotate,new_y_axis->y);
    //first rotation
    double first_rotate_x=(double)relate_x;
    double first_rotate_y=(double)(relate_y*angle_y_rotate-relate_z*sin(acos(angle_y_rotate)));
    double first_rotate_z=(double)(relate_y*sin(acos(angle_y_rotate))+relate_z*angle_y_rotate);
    //printf("if 1::%lf\n",angle_y_rotate*angle_y_rotate+sin(acos(angle_y_rotate))*sin(acos(angle_y_rotate)));

    //second rotation
    double angle_z_rotate=0;
    if(new_y_axis->y==0&&new_y_axis->z==0){
        angle_z_rotate=0;

    }else if(new_y_axis->x==0){
        angle_z_rotate=1;
    }else{
        angle_z_rotate=(double)(new_y_axis->y*new_y_axis->y+new_y_axis->z*new_y_axis->z)/(sqrt((double)(new_y_axis->y*new_y_axis->y+new_y_axis->z*new_y_axis->z))*sqrt((double)(new_y_axis->x*new_y_axis->x+new_y_axis->y*new_y_axis->y+new_y_axis->z*new_y_axis->z)));
    }
    //double angle_z_rotate_vn2=
    double second_rotate_x=(double)(first_rotate_x*angle_z_rotate-first_rotate_y*sin(acos(angle_z_rotate)));
    double second_rotate_y=(double)(first_rotate_x*sin(acos(angle_z_rotate))+first_rotate_y*angle_z_rotate);
    double second_rotate_z=first_rotate_z;
    // printf("%lf\n",angle_z_rotate);
    // printf("if 2::%lf\n",angle_z_rotate*angle_z_rotate+sin(acos(angle_z_rotate))*sin(acos(angle_z_rotate)));
    //for two directions
    if((sqrt((double)second_rotate_x*second_rotate_x+second_rotate_y*second_rotate_y+second_rotate_z*second_rotate_z)*sqrt((double)new_y_axis->x*new_y_axis->x+new_y_axis->y*new_y_axis->y+new_y_axis->z*new_y_axis->z))==0)
        printf("3333333333\n");
    double angle_result=(second_rotate_x*new_y_axis->x+second_rotate_y*new_y_axis->y+second_rotate_z*new_y_axis->z)/(sqrt((double)second_rotate_x*second_rotate_x+second_rotate_y*second_rotate_y+second_rotate_z*second_rotate_z)*sqrt((double)new_y_axis->x*new_y_axis->x+new_y_axis->y*new_y_axis->y+new_y_axis->z*new_y_axis->z));
    delete new_y_axis;
    // printf("%lf\n",angle_result);
    return angle_result;

}

//define a coordinate system which Y axis is the vector between source node and target node, origin of coordinate is source node
QMap<V3DLONG,QMap<V3DLONG,QList<Node> > > sphere_search(unsigned char * &img1d,QMap<int,Node* > cluster_root,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,double r,int iteration){
    //first calculate the distance between roots using bf based on sphere model
    //second, after get the distance, input it into mst algorithm and get a connection relation
    //finally fix the route between parent and child in SWC based on the bf route got in first step
    QMap<V3DLONG,QMap<V3DLONG,QList<Node> > > path_between_roots;
    path_between_roots.clear();
    int count_time=0;

    for(QMap<int,Node*>::iterator iter1 =cluster_root.begin(); iter1 != cluster_root.end(); iter1++)
    {
        Node* elem1=iter1.value();
        V3DLONG index1=GET_IND(elem1->x,elem1->y,elem1->z);
        QMap<V3DLONG,QList<Node> >  path_to_target;
        path_to_target.clear();

        for(QMap<int,Node*>::iterator iter2 =cluster_root.begin(); iter2 != cluster_root.end(); iter2++)
        {

            Node* elem2=iter2.value();
            Node current_center=getnode(elem1);
            V3DLONG index2=GET_IND(elem2->x,elem2->y,elem2->z);
            if(index1==index2)
            {
                continue;
            }

            QList<Node> path;
            path.clear();
            // printf("original size::%d\n",path.size());
            path.append(current_center);
            //QQueue<Node> queue_candidate;
            //queue_candidate.clear();
            int times=0;

            //printf("current_center in the beginning::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
            while(times<iteration)
            {

                //initialize a sphere
                struct sphere_model_two_directions sphere;
                sphere.up=0;
                sphere.down=0;

                sphere.pixal_up=0;
                sphere.pixal_down=0;

                sphere.QList_up.clear();
                sphere.QList_down.clear();

                sphere.sum_pixal_up=0;
                sphere.sum_pixal_down=0;
                //first count the average pixal in the region between begin node and terminal node, if got a low pixal, enlarge the search radius
                //realize the function later,20160126
                // if average<30 r=5.0;else r=2.0(the selection method of r should be according to the prim distance)

                //construct a sphere which radius is r
                V3DLONG left_x=current_center.x-r;
                V3DLONG right_x=current_center.x+r;
                V3DLONG left_y=current_center.y-r;
                V3DLONG right_y=current_center.y+r;
                V3DLONG left_z=current_center.z-r;
                V3DLONG right_z=current_center.z+r;
                left_x=(left_x<0)?0:left_x;
                right_x=(right_x>sz_x)?sz_x:right_x;
                left_y=(left_y<0)?0:left_y;
                right_y=(right_y>sz_y)?sz_y:right_y;
                left_z=(left_z<0)?0:left_z;
                right_z=(right_z>sz_z)?sz_z:right_z;
                for(V3DLONG i=left_x;i<=right_x;i++){
                    for(V3DLONG j=left_y;j<=right_y;j++){
                        for(V3DLONG k=left_z;k<=right_z;k++){
                            if(img1d[GET_IND(i,j,k)]==0){//set a threshold here
                                continue;

                            }
                            if(GET_IND(i,j,k)==GET_IND(current_center.x,current_center.y,current_center.z)){
                                continue;
                            }
                            V3DLONG relate_x=i-current_center.x;
                            V3DLONG relate_y=j-current_center.y;
                            V3DLONG relate_z=k-current_center.z;

                            double distance_node=(double)sqrt((double)(relate_x*relate_x+relate_y*relate_y+relate_z*relate_z));
                            if(distance_node>r){//means the node is not in the sphere
                                continue;
                            }
                            //design a function here to map the node into the new coordinate
                            //double Map_node_direction=Map_coordinate(current_center,getnode(elem2),relate_x,relate_y,relate_z);
                            //calculate the angle between new node and new y axis,if >0 means same direction, otherwise opposite
                            // printf("Map_node_direction:%lf\n",Map_node_direction);
                            V3DLONG beg_ter_x=elem2->x-current_center.x;
                            V3DLONG beg_ter_y=elem2->y-current_center.y;
                            V3DLONG beg_ter_z=elem2->z-current_center.z;
                            double Map_node_direction=(beg_ter_x*relate_x+beg_ter_y*relate_y+beg_ter_z*relate_z)/(sqrt((double)beg_ter_x*beg_ter_x+beg_ter_y*beg_ter_y+beg_ter_z*beg_ter_z)*sqrt((double)relate_x*relate_x+relate_y*relate_y+relate_z*relate_z));
                            if((sqrt((double)beg_ter_x*beg_ter_x+beg_ter_y*beg_ter_y+beg_ter_z*beg_ter_z)*sqrt((double)relate_x*relate_x+relate_y*relate_y+relate_z*relate_z))==0){
                                printf("denominator equals 0\n");

                            }
                            //statistical the node number in eight directions
                            if((Map_node_direction>0)){
                                // if(img1d[GET_IND(i,j,k)]>5){//the threshold can be considered as 1
                                sphere.up++;
                                sphere.sum_pixal_up=sphere.sum_pixal_up+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up=new Node(i,j,k);
                                if(sphere.pixal_up<img1d[GET_IND(i,j,k)])
                                {
                                    sphere.pixal_up=img1d[GET_IND(i,j,k)];
                                    sphere.node_up=getnode(temp_node_up);
                                    // }
                                }
                                sphere.QList_up.append(getnode(temp_node_up));
                                delete temp_node_up;

                            }else if((Map_node_direction<0)){
                                //if(img1d[GET_IND(i,j,k)]>5){
                                sphere.down++;
                                sphere.sum_pixal_down=sphere.sum_pixal_down+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down=new Node(i,j,k);
                                if(sphere.pixal_down<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down=img1d[GET_IND(i,j,k)];
                                    sphere.node_down=getnode(temp_node_down);
                                    //  }
                                }
                                sphere.QList_down.append(getnode(temp_node_down));
                                delete temp_node_down;
                            }
                        }

                    }

                }
                /* printf("%ld\n",sphere.up);
                printf("%ld\n",sphere.down);*///for test

                //determine whether the current node and target is in the same sphere
                double dis_cur_target_x=current_center.x-elem2->x;
                double dis_cur_target_y=current_center.y-elem2->y;
                double dis_cur_target_z=current_center.z-elem2->z;
                double dis_cur_target=(double)sqrt(dis_cur_target_x*dis_cur_target_x+dis_cur_target_y*dis_cur_target_y+dis_cur_target_z*dis_cur_target_z);
                if((dis_cur_target<=r)&&(dis_cur_target!=0)){
                    // printf("222222222222222222222222222\n");
                    path.append(getnode(elem2));
                    break;
                }else if(dis_cur_target==0){
                    break;
                }
                //printf("beginning node::%ld  %ld  %ld\n",elem1->x,elem1->y,elem1->z);

                // printf("first,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                //printf("first\n");
                current_center=choose_region_vn4(img1d,sphere,current_center,getnode(elem2),sz_x,sz_y,sz_z);//the node which pixal is the maximum in the eight regions have been chose in the above,then choose the region using a weight formula
                //current_center=sphere.node_up;
                path.append(current_center);
                //printf("second\n");
                // printf("second,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                // printf("terminal node::%ld  %ld  %ld\n",elem2->x,elem2->y,elem2->z);
                //need to use current_node to form a path which indicate the path between two roots
                if(current_center.x==-1)
                {
                    //printf("111111111111111111\n");
                    break;
                }

                times++;

            }
            //printf("after iteration:: %d\n",path.size());


            path_to_target.insert(index2,path);


        }
        count_time++;
        //printf("count_time::%d\n",count_time);

        path_between_roots.insert(index1,path_to_target);

    }

    return path_between_roots;

}

//two directions
QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > spanning_combined_bf_vn3(unsigned char * &img1d,QMap<int,Node* > cluster_root,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,double r,int iteration){
    //first calculate the distance between roots using bf based on sphere model
    //second, after get the distance, input it into mst algorithm and get a connection relation
    //finally fix the route between parent and child in SWC based on the bf route got in first step
    QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > path_between_roots;
    path_between_roots.clear();
    int count_time=0;

    for(QMap<int,Node*>::iterator iter1 =cluster_root.begin(); iter1 != cluster_root.end(); iter1++)
    {
        Node* elem1=iter1.value();
        V3DLONG index1=GET_IND(elem1->x,elem1->y,elem1->z);
        QMultiMap<V3DLONG,QList<Node> >  path_to_target;
        path_to_target.clear();


        for(QMap<int,Node*>::iterator iter2 =cluster_root.begin(); iter2 != cluster_root.end(); iter2++)
        {


            Node* elem2=iter2.value();
            Node current_center=getnode(elem1);
            V3DLONG index2=GET_IND(elem2->x,elem2->y,elem2->z);
            if(index1==index2){
                continue;

            }

            QList<Node> path;
            path.clear();
            // printf("original size::%d\n",path.size());
            path.append(current_center);
            //QQueue<Node> queue_candidate;
            //queue_candidate.clear();
            int times=0;
            // printf("beginning node::%ld  %ld  %ld\n",elem1->x,elem1->y,elem1->z);
            // printf("terminal node::%ld  %ld  %ld\n",elem2->x,elem2->y,elem2->z);
            // printf("current_center in the beginning::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
            while(times<iteration)
            {

                //initialize a sphere
                struct sphere_model_two_directions sphere;
                sphere.up=0;
                sphere.down=0;

                sphere.pixal_up=0;
                sphere.pixal_down=0;

                sphere.QList_up.clear();
                sphere.QList_down.clear();

                sphere.sum_pixal_up=0;
                sphere.sum_pixal_down=0;


                //construct a sphere which radius is r
                V3DLONG left_x=current_center.x-r;
                V3DLONG right_x=current_center.x+r;
                V3DLONG left_y=current_center.y-r;
                V3DLONG right_y=current_center.y+r;
                V3DLONG left_z=current_center.z-r;
                V3DLONG right_z=current_center.z+r;
                left_x=(left_x<0)?0:left_x;
                right_x=(right_x>sz_x)?sz_x:right_x;
                left_y=(left_y<0)?0:left_y;
                right_y=(right_y>sz_y)?sz_y:right_y;
                left_z=(left_z<0)?0:left_z;
                right_z=(right_z>sz_z)?sz_z:right_z;
                for(V3DLONG i=left_x;i<=right_x;i++){
                    for(V3DLONG j=left_y;j<=right_y;j++){
                        for(V3DLONG k=left_z;k<=right_z;k++){
                            if(img1d[GET_IND(i,j,k)]==0){//set a threshold here
                                continue;

                            }
                            if(GET_IND(i,j,k)==GET_IND(current_center.x,current_center.y,current_center.z)){
                                continue;
                            }
                            V3DLONG relate_x=i-current_center.x;
                            V3DLONG relate_y=j-current_center.y;
                            V3DLONG relate_z=k-current_center.z;

                            double distance_node=(double)sqrt((double)(relate_x*relate_x+relate_y*relate_y+relate_z*relate_z));
                            if(distance_node>r){//means the node is not in the sphere
                                continue;

                            }
                            //statistical the node number in eight directions
                            if((relate_z>0)){

                                sphere.up++;
                                sphere.sum_pixal_up=sphere.sum_pixal_up+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up=new Node(i,j,k);
                                /*if(sphere.pixal_up<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_up=img1d[GET_IND(i,j,k)];
                                    sphere.node_up=getnode(temp_node_up);

                                }*/
                                sphere.QList_up.append(getnode(temp_node_up));
                                delete temp_node_up;

                            }else if((relate_z<0)){

                                sphere.down++;
                                sphere.sum_pixal_down=sphere.sum_pixal_down+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down=new Node(i,j,k);
                                /*if(sphere.pixal_down<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down=img1d[GET_IND(i,j,k)];
                                    sphere.node_down=getnode(temp_node_down);

                                }*/
                                sphere.QList_down.append(getnode(temp_node_down));
                                delete temp_node_down;
                            }
                        }

                    }

                }
                /* printf("%ld\n",sphere.up);
                printf("%ld\n",sphere.down);*///for test


                //determine whether the current node and target is in the same sphere
                double dis_cur_target_x=current_center.x-elem2->x;
                double dis_cur_target_y=current_center.y-elem2->y;
                double dis_cur_target_z=current_center.z-elem2->z;
                double dis_cur_target=(double)sqrt(dis_cur_target_x*dis_cur_target_x+dis_cur_target_y*dis_cur_target_y+dis_cur_target_z*dis_cur_target_z);
                if((dis_cur_target<=r)&&(dis_cur_target!=0)){
                    // printf("222222222222222222222222222\n");
                    path.append(getnode(elem2));
                    break;
                }else if(dis_cur_target==0){
                    break;
                }
                // printf("first,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                current_center=choose_region_vn4(img1d,sphere,current_center,getnode(elem2),sz_x,sz_y,sz_z);//the node which pixal is the maximum in the eight regions have been chose in the above,then choose the region using a weight formula
                path.append(current_center);
                // printf("second,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                //need to use current_node to form a path which indicate the path between two roots
                if(current_center.x==-1){
                    printf("111111111111111111\n");
                    break;
                }

                times++;

            }
            //printf("after iteration:: %d\n",path.size());
            if(path.size()<5){
                printf("0000000000000000000000000000000\n");

            }
            if(GET_IND(current_center.x,current_center.y,current_center.z)==index2){
                printf("0000000000000000000000000000000\n");

            }
            path_to_target.insert(index2,path);


        }
        count_time++;
        printf("count_time::%d\n",count_time);

        path_between_roots.insert(index1,path_to_target);

    }

    return path_between_roots;

}
//four directions
QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > spanning_combined_bf_vn2(unsigned char * &img1d,QMap<int,Node* > cluster_root,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,double r,int iteration){
    //first calculate the distance between roots using bf based on sphere model
    //second, after get the distance, input it into mst algorithm and get a connection relation
    //finally fix the route between parent and child in SWC based on the bf route got in first step
    QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > path_between_roots;
    path_between_roots.clear();
    int count_time=0;

    for(QMap<int,Node*>::iterator iter1 =cluster_root.begin(); iter1 != cluster_root.end(); iter1++)
    {
        Node* elem1=iter1.value();
        V3DLONG index1=GET_IND(elem1->x,elem1->y,elem1->z);
        QMultiMap<V3DLONG,QList<Node> >  path_to_target;
        path_to_target.clear();


        for(QMap<int,Node*>::iterator iter2 =cluster_root.begin(); iter2 != cluster_root.end(); iter2++)
        {


            Node* elem2=iter2.value();
            Node current_center=getnode(elem1);
            V3DLONG index2=GET_IND(elem2->x,elem2->y,elem2->z);
            if(index1==index2){
                continue;

            }

            QList<Node> path;
            path.clear();
            // printf("original size::%d\n",path.size());
            path.append(current_center);
            //QQueue<Node> queue_candidate;
            //queue_candidate.clear();
            int times=0;
            // printf("beginning node::%ld  %ld  %ld\n",elem1->x,elem1->y,elem1->z);
            // printf("terminal node::%ld  %ld  %ld\n",elem2->x,elem2->y,elem2->z);
            // printf("current_center in the beginning::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
            while(times<iteration)
            {
                //define node number of eight directions
                /* V3DLONG up_right_up=0;
                V3DLONG up_left_up=0;
                V3DLONG up_left_bottom=0;
                V3DLONG up_right_bottom=0;
                V3DLONG down_right_up=0;
                V3DLONG down_left_up=0;
                V3DLONG down_left_bottom=0;
                V3DLONG down_right_bottom=0;
                //define the node with maximum pixval value in the eight directions and corresponding pixal
                Node node_up_right_up=0;
                Node node_up_left_up=0;
                Node node_up_left_bottom=0;
                Node node_up_right_bottom=0;
                Node node_down_right_up=0;
                Node node_down_left_up=0;
                Node node_down_left_bottom=0;
                Node node_down_right_bottom=0;

                double pixal_up_right_up=0;
                double pixal_up_left_up=0;
                double pixal_up_left_bottom=0;
                double pixal_up_right_bottom=0;
                double pixal_down_right_up=0;
                double pixal_down_left_up=0;
                double pixal_down_left_bottom=0;
                double pixal_down_right_bottom=0;*/
                //initialize a sphere
                struct sphere_model_four_directions sphere;
                sphere.up_right=0;
                sphere.up_left=0;
                sphere.down_right=0;
                sphere.down_left=0;

                sphere.pixal_up_right=0;
                sphere.pixal_up_left=0;
                sphere.pixal_down_right=0;
                sphere.pixal_down_left=0;

                sphere.QList_up_right.clear();
                sphere.QList_up_left.clear();
                sphere.QList_down_right.clear();
                sphere.QList_down_left.clear();

                sphere.sum_pixal_up_right=0;
                sphere.sum_pixal_up_left=0;
                sphere.sum_pixal_down_right=0;
                sphere.sum_pixal_down_left=0;

                //construct a sphere which radius is r
                V3DLONG left_x=current_center.x-r;
                V3DLONG right_x=current_center.x+r;
                V3DLONG left_y=current_center.y-r;
                V3DLONG right_y=current_center.y+r;
                V3DLONG left_z=current_center.z-r;
                V3DLONG right_z=current_center.z+r;
                left_x=(left_x<0)?0:left_x;
                right_x=(right_x>sz_x)?sz_x:right_x;
                left_y=(left_y<0)?0:left_y;
                right_y=(right_y>sz_y)?sz_y:right_y;
                left_z=(left_z<0)?0:left_z;
                right_z=(right_z>sz_z)?sz_z:right_z;
                for(V3DLONG i=left_x;i<=right_x;i++){
                    for(V3DLONG j=left_y;j<=right_y;j++){
                        for(V3DLONG k=left_z;k<=right_z;k++){
                            if(img1d[GET_IND(i,j,k)]==0){//set a threshold here
                                continue;

                            }
                            if(GET_IND(i,j,k)==GET_IND(current_center.x,current_center.y,current_center.z)){
                                continue;
                            }
                            V3DLONG relate_x=i-current_center.x;
                            V3DLONG relate_y=j-current_center.y;
                            V3DLONG relate_z=k-current_center.z;

                            double distance_node=(double)sqrt((double)(relate_x*relate_x+relate_y*relate_y+relate_z*relate_z));
                            if(distance_node>r){//means the node is not in the sphere
                                continue;

                            }
                            //statistical the node number in eight directions
                            if((relate_x>0)&&(relate_z>0)){
                                // if(img1d[GET_IND(i,j,k)]>5){//the threshold can be consider as 1
                                sphere.up_right++;
                                sphere.sum_pixal_up_right=sphere.sum_pixal_up_right+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up_right=new Node(i,j,k);
                                if(sphere.pixal_up_right<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_up_right=img1d[GET_IND(i,j,k)];
                                    sphere.node_up_right=getnode(temp_node_up_right);
                                    // }
                                }
                                sphere.QList_up_right.append(getnode(temp_node_up_right));
                                delete temp_node_up_right;

                            }else if((relate_x<0)&&(relate_z>0)){
                                // if(img1d[GET_IND(i,j,k)]>5){
                                sphere.up_left++;
                                sphere.sum_pixal_up_left=sphere.sum_pixal_up_left+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up_left=new Node(i,j,k);
                                if(sphere.pixal_up_left<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_up_left=img1d[GET_IND(i,j,k)];
                                    sphere.node_up_left=getnode(temp_node_up_left);
                                    // }
                                }
                                sphere.QList_up_left.append(getnode(temp_node_up_left));
                                delete temp_node_up_left;

                            }else if((relate_x>0)&&(relate_z<0)){
                                //if(img1d[GET_IND(i,j,k)]>5){
                                sphere.down_right++;
                                sphere.sum_pixal_down_right=sphere.sum_pixal_down_right+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down_right=new Node(i,j,k);
                                if(sphere.pixal_down_right<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down_right=img1d[GET_IND(i,j,k)];
                                    sphere.node_down_right=getnode(temp_node_down_right);
                                    //  }
                                }
                                sphere.QList_down_right.append(getnode(temp_node_down_right));
                                delete temp_node_down_right;
                            }else if((relate_x<0)&&(relate_z<0)){
                                // if(img1d[GET_IND(i,j,k)]>5){
                                sphere.down_left++;
                                sphere.sum_pixal_down_left=sphere.sum_pixal_down_left+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down_left=new Node(i,j,k);
                                if(sphere.pixal_down_left<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down_left=img1d[GET_IND(i,j,k)];
                                    sphere.node_down_left=getnode(temp_node_down_left);
                                    // }
                                }
                                sphere.QList_down_left.append(getnode(temp_node_down_left));
                                delete temp_node_down_left;

                            }
                        }

                    }

                }
                /* printf("%ld\n",sphere.up_right);
                printf("%ld\n",sphere.up_left);
                printf("%ld\n",sphere.down_right);
                printf("%ld\n",sphere.down_left);*///for test


                //determine whether the current node and target is in the same sphere
                double dis_cur_target_x=current_center.x-elem2->x;
                double dis_cur_target_y=current_center.y-elem2->y;
                double dis_cur_target_z=current_center.z-elem2->z;
                double dis_cur_target=(double)sqrt(dis_cur_target_x*dis_cur_target_x+dis_cur_target_y*dis_cur_target_y+dis_cur_target_z*dis_cur_target_z);
                if((dis_cur_target<=r)&&(dis_cur_target!=0)){
                    // printf("222222222222222222222222222\n");
                    path.append(getnode(elem2));
                    break;
                }else if(dis_cur_target==0){
                    break;
                }
                // printf("first,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                current_center=choose_region_vn3(img1d,sphere,current_center,getnode(elem2),sz_x,sz_y,sz_z);//the node which pixal is the maximum in the eight regions have been chose in the above,then choose the region using a weight formula
                path.append(current_center);
                // printf("second,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                //need to use current_node to form a path which indicate the path between two roots
                if(current_center.x==-1){
                    printf("111111111111111111\n");
                    break;
                }

                times++;

            }
            //printf("after iteration:: %d\n",path.size());
            if(path.size()<5){
                printf("0000000000000000000000000000000\n");

            }
            if(GET_IND(path.last().x,path.last().y,path.last().z)==index2){
                printf("0000000000000000000000000000000\n");

            }
            path_to_target.insert(index2,path);


        }
        count_time++;
        printf("count_time::%d\n",count_time);

        path_between_roots.insert(index1,path_to_target);

    }

    return path_between_roots;

}


//should define a QMap<V3DLONG,QMap<V3DLONG,QList<Node> > > which indicate that the QList is the nodes between two roots
QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > spanning_combined_bf(unsigned char * &img1d,QMap<int,Node* > cluster_root,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,double r,int iteration){
    //first calculate the distance between roots using bf based on sphere model
    //second, after get the distance, input it into mst algorithm and get a connection relation
    //finally fix the route between parent and child in SWC based on the bf route got in first step
    QMultiMap<V3DLONG,QMultiMap<V3DLONG,QList<Node> > > path_between_roots;
    path_between_roots.clear();
    int count_time=0;

    for(QMap<int,Node*>::iterator iter1 =cluster_root.begin(); iter1 != cluster_root.end(); iter1++)
    {
        Node* elem1=iter1.value();
        V3DLONG index1=GET_IND(elem1->x,elem1->y,elem1->z);
        QMultiMap<V3DLONG,QList<Node> >  path_to_target;
        path_to_target.clear();


        for(QMap<int,Node*>::iterator iter2 =cluster_root.begin(); iter2 != cluster_root.end(); iter2++)
        {


            Node* elem2=iter2.value();
            Node current_center=getnode(elem1);
            V3DLONG index2=GET_IND(elem2->x,elem2->y,elem2->z);
            if(index1==index2){
                continue;

            }

            QList<Node> path;
            path.clear();
            // printf("original size::%d\n",path.size());
            path.append(current_center);
            //QQueue<Node> queue_candidate;
            //queue_candidate.clear();
            int times=0;
            // printf("beginning node::%ld  %ld  %ld\n",elem1->x,elem1->y,elem1->z);
            // printf("terminal node::%ld  %ld  %ld\n",elem2->x,elem2->y,elem2->z);
            // printf("current_center in the beginning::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
            while(times<iteration)
            {
                //define node number of eight directions
                /* V3DLONG up_right_up=0;
                V3DLONG up_left_up=0;
                V3DLONG up_left_bottom=0;
                V3DLONG up_right_bottom=0;
                V3DLONG down_right_up=0;
                V3DLONG down_left_up=0;
                V3DLONG down_left_bottom=0;
                V3DLONG down_right_bottom=0;
                //define the node with maximum pixval value in the eight directions and corresponding pixal
                Node node_up_right_up=0;
                Node node_up_left_up=0;
                Node node_up_left_bottom=0;
                Node node_up_right_bottom=0;
                Node node_down_right_up=0;
                Node node_down_left_up=0;
                Node node_down_left_bottom=0;
                Node node_down_right_bottom=0;

                double pixal_up_right_up=0;
                double pixal_up_left_up=0;
                double pixal_up_left_bottom=0;
                double pixal_up_right_bottom=0;
                double pixal_down_right_up=0;
                double pixal_down_left_up=0;
                double pixal_down_left_bottom=0;
                double pixal_down_right_bottom=0;*/
                //initialize a sphere
                struct sphere_model sphere;
                sphere.up_right_up=0;
                sphere.up_left_up=0;
                sphere.up_left_bottom=0;
                sphere.up_right_bottom=0;
                sphere.down_right_up=0;
                sphere.down_left_up=0;
                sphere.down_left_bottom=0;
                sphere.down_right_bottom=0;

                sphere.pixal_up_right_up=0;
                sphere.pixal_up_left_up=0;
                sphere.pixal_up_left_bottom=0;
                sphere.pixal_up_right_bottom=0;
                sphere.pixal_down_right_up=0;
                sphere.pixal_down_left_up=0;
                sphere.pixal_down_left_bottom=0;
                sphere.pixal_down_right_bottom=0;

                sphere.QList_up_right_up.clear();
                sphere.QList_up_left_up.clear();
                sphere.QList_up_left_bottom.clear();
                sphere.QList_up_right_bottom.clear();
                sphere.QList_down_right_up.clear();
                sphere.QList_down_left_up.clear();
                sphere.QList_down_left_bottom.clear();
                sphere.QList_down_right_bottom.clear();

                sphere.sum_pixal_up_right_up=0;
                sphere.sum_pixal_up_left_up=0;
                sphere.sum_pixal_up_left_bottom=0;
                sphere.sum_pixal_up_right_bottom=0;
                sphere.sum_pixal_down_right_up=0;
                sphere.sum_pixal_down_left_up=0;
                sphere.sum_pixal_down_left_bottom=0;
                sphere.sum_pixal_down_right_bottom=0;

                //construct a sphere which radius is r
                V3DLONG left_x=current_center.x-r;
                V3DLONG right_x=current_center.x+r;
                V3DLONG left_y=current_center.y-r;
                V3DLONG right_y=current_center.y+r;
                V3DLONG left_z=current_center.z-r;
                V3DLONG right_z=current_center.z+r;
                left_x=(left_x<0)?0:left_x;
                right_x=(right_x>sz_x)?sz_x:right_x;
                left_y=(left_y<0)?0:left_y;
                right_y=(right_y>sz_y)?sz_y:right_y;
                left_z=(left_z<0)?0:left_z;
                right_z=(right_z>sz_z)?sz_z:right_z;
                for(V3DLONG i=left_x;i<=right_x;i++){
                    for(V3DLONG j=left_y;j<=right_y;j++){
                        for(V3DLONG k=left_z;k<=right_z;k++){
                            if(img1d[GET_IND(i,j,k)]==0){//set a threshold here
                                continue;

                            }
                            if(GET_IND(i,j,k)==GET_IND(current_center.x,current_center.y,current_center.z)){
                                continue;
                            }
                            V3DLONG relate_x=i-current_center.x;
                            V3DLONG relate_y=j-current_center.y;
                            V3DLONG relate_z=k-current_center.z;

                            double distance_node=(double)sqrt((double)(relate_x*relate_x+relate_y*relate_y+relate_z*relate_z));
                            if(distance_node>r){//means the node is not in the sphere
                                continue;

                            }
                            //statistical the node number in eight directions
                            if((relate_x>0)&&(relate_y>0)&&(relate_z>0)){
                                // if(img1d[GET_IND(i,j,k)]>5){//the threshold can be consider as 1
                                sphere.up_right_up++;
                                sphere.sum_pixal_up_right_up=sphere.sum_pixal_up_right_up+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up_right_up=new Node(i,j,k);
                                if(sphere.pixal_up_right_up<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_up_right_up=img1d[GET_IND(i,j,k)];
                                    sphere.node_up_right_up=getnode(temp_node_up_right_up);
                                    // }
                                }
                                sphere.QList_up_right_up.append(getnode(temp_node_up_right_up));
                                delete temp_node_up_right_up;

                            }else if((relate_x<0)&&(relate_y>0)&&(relate_z>0)){
                                // if(img1d[GET_IND(i,j,k)]>5){
                                sphere.up_left_up++;
                                sphere.sum_pixal_up_left_up=sphere.sum_pixal_up_left_up+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up_left_up=new Node(i,j,k);
                                if(sphere.pixal_up_left_up<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_up_left_up=img1d[GET_IND(i,j,k)];
                                    sphere.node_up_left_up=getnode(temp_node_up_left_up);
                                    // }
                                }
                                sphere.QList_up_left_up.append(getnode(temp_node_up_left_up));
                                delete temp_node_up_left_up;

                            }else if((relate_x<0)&&(relate_y<0)&&(relate_z>0)){
                                // if(img1d[GET_IND(i,j,k)]>5){
                                sphere.up_left_bottom++;
                                sphere.sum_pixal_up_left_bottom=sphere.sum_pixal_up_left_bottom+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up_left_bottom=new Node(i,j,k);
                                if(sphere.pixal_up_left_bottom<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_up_left_bottom=img1d[GET_IND(i,j,k)];
                                    sphere.node_up_left_bottom=getnode(temp_node_up_left_bottom);
                                    // }
                                }
                                sphere.QList_up_left_bottom.append(getnode(temp_node_up_left_bottom));
                                delete temp_node_up_left_bottom;

                            }else if((relate_x>0)&&(relate_y<0)&&(relate_z>0)){
                                // if(img1d[GET_IND(i,j,k)]>5){
                                sphere.up_right_bottom++;
                                sphere.sum_pixal_up_right_bottom=sphere.sum_pixal_up_right_bottom+img1d[GET_IND(i,j,k)];
                                Node* temp_node_up_right_bottom=new Node(i,j,k);
                                if(sphere.pixal_up_right_bottom<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_up_right_bottom=img1d[GET_IND(i,j,k)];
                                    sphere.node_up_right_bottom=getnode(temp_node_up_right_bottom);
                                    // }
                                }
                                sphere.QList_up_right_bottom.append(getnode(temp_node_up_right_bottom));
                                delete temp_node_up_right_bottom;

                            }else if((relate_x>0)&&(relate_y>0)&&(relate_z<0)){
                                //if(img1d[GET_IND(i,j,k)]>5){
                                sphere.down_right_up++;
                                sphere.sum_pixal_down_right_up=sphere.sum_pixal_down_right_up+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down_right_up=new Node(i,j,k);
                                if(sphere.pixal_down_right_up<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down_right_up=img1d[GET_IND(i,j,k)];
                                    sphere.node_down_right_up=getnode(temp_node_down_right_up);
                                    //  }
                                }
                                sphere.QList_down_right_up.append(getnode(temp_node_down_right_up));
                                delete temp_node_down_right_up;
                            }else if((relate_x<0)&&(relate_y>0)&&(relate_z<0)){
                                // if(img1d[GET_IND(i,j,k)]>5){
                                sphere.down_left_up++;
                                sphere.sum_pixal_down_left_up=sphere.sum_pixal_down_left_up+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down_left_up=new Node(i,j,k);
                                if(sphere.pixal_down_left_up<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down_left_up=img1d[GET_IND(i,j,k)];
                                    sphere.node_down_left_up=getnode(temp_node_down_left_up);
                                    // }
                                }
                                sphere.QList_down_left_up.append(getnode(temp_node_down_left_up));
                                delete temp_node_down_left_up;

                            }else if((relate_x<0)&&(relate_y<0)&&(relate_z<0)){
                                // if(img1d[GET_IND(i,j,k)]>5){
                                sphere.down_left_bottom++;
                                sphere.sum_pixal_down_left_bottom=sphere.sum_pixal_down_left_bottom+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down_left_bottom=new Node(i,j,k);
                                if(sphere.pixal_down_left_bottom<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down_left_bottom=img1d[GET_IND(i,j,k)];
                                    sphere.node_down_left_bottom=getnode(temp_node_down_left_bottom);
                                    // }
                                }
                                sphere.QList_down_left_bottom.append(getnode(temp_node_down_left_bottom));
                                delete temp_node_down_left_bottom;

                            }else if((relate_x>0)&&(relate_y<0)&&(relate_z<0)){
                                //if(img1d[GET_IND(i,j,k)]>5){
                                sphere.down_right_bottom++;
                                sphere.sum_pixal_down_right_bottom=sphere.sum_pixal_down_right_bottom+img1d[GET_IND(i,j,k)];
                                Node* temp_node_down_right_bottom=new Node(i,j,k);
                                if(sphere.pixal_down_right_bottom<img1d[GET_IND(i,j,k)]){
                                    sphere.pixal_down_right_bottom=img1d[GET_IND(i,j,k)];
                                    sphere.node_down_right_bottom=getnode(temp_node_down_right_bottom);
                                    // }
                                }
                                sphere.QList_down_right_bottom.append(getnode(temp_node_down_right_bottom));
                                delete temp_node_down_right_bottom;

                            }


                        }

                    }

                }
                /* printf("%ld\n",sphere.up_right_up);
                printf("%ld\n",sphere.up_left_up);
                printf("%ld\n",sphere.up_left_bottom);
                printf("%ld\n",sphere.up_right_bottom);
                printf("%ld\n",sphere.down_right_up);
                printf("%ld\n",sphere.down_left_up);
                printf("%ld\n",sphere.down_left_bottom);
                printf("%ld\n",sphere.down_right_bottom);*///for test


                //determine whether the current node and target is in the same sphere
                double dis_cur_target_x=current_center.x-elem2->x;
                double dis_cur_target_y=current_center.y-elem2->y;
                double dis_cur_target_z=current_center.z-elem2->z;
                double dis_cur_target=(double)sqrt(dis_cur_target_x*dis_cur_target_x+dis_cur_target_y*dis_cur_target_y+dis_cur_target_z*dis_cur_target_z);
                if((dis_cur_target<=r)&&(dis_cur_target!=0)){
                    // printf("222222222222222222222222222\n");
                    path.append(getnode(elem2));
                    break;
                }else if(dis_cur_target==0){
                    break;
                }
                // printf("first,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                current_center=choose_region_vn2(img1d,sphere,current_center,getnode(elem2),sz_x,sz_y,sz_z);//the node which pixal is the maximum in the eight regions have been chose in the above,then choose the region using a weight formula
                path.append(current_center);
                // printf("second,current_center::%ld  %ld  %ld\n",current_center.x,current_center.y,current_center.z);
                //need to use current_node to form a path which indicate the path between two roots
                if(current_center.x==-1){
                    printf("111111111111111111\n");
                    break;
                }

                times++;

            }
            //printf("after iteration:: %d\n",path.size());
            if(path.size()<5){
                printf("0000000000000000000000000000000\n");

            }
            if(GET_IND(current_center.x,current_center.y,current_center.z)==index2){
                printf("0000000000000000000000000000000\n");

            }
            path_to_target.insert(index2,path);


        }
        count_time++;
        printf("count_time::%d\n",count_time);

        path_between_roots.insert(index1,path_to_target);

    }

    return path_between_roots;

}

Node choose_region_vn4(unsigned char * &img1d,struct sphere_model_two_directions sphere_m,Node source_node,Node target_node,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    //two directions
    QList<V3DLONG> direction_node_number;
    //QList<double> direction_average_pixal;
    //QList<double> direction_node_pixal;
    QList<QList<Node> >  direction_all_nodes;
    QList<Node> direction_node;
    Node result_pseu;
    result_pseu.x=-1;

    //direction_node_pixal.append(sphere_m.pixal_up);
    //direction_node_pixal.append(sphere_m.pixal_down);

    direction_node.append(sphere_m.node_up);
    direction_node.append(sphere_m.node_down);

    direction_node_number.append(sphere_m.up);
    direction_node_number.append(sphere_m.down);

    direction_all_nodes.append(sphere_m.QList_up);
    direction_all_nodes.append(sphere_m.QList_down);

    /* double average_pixval_1=(sphere_m.up==0)?0:sphere_m.sum_pixal_up/sphere_m.up;
    direction_average_pixal.append(average_pixval_1);
    double average_pixval_2=(sphere_m.down==0)?0:sphere_m.sum_pixal_down/sphere_m.down;
    direction_average_pixal.append(average_pixval_2);*/

    double weight=-INF;
    double weight_temp=-INF;
    int node_number=0;
    double angle_final=0;
    V3DLONG node_numbers=0;
    // printf("direction_node.size::%d\n",direction_node.size());

    for(int i=0;i<direction_node.size();i++)
    {
        if(direction_node_number.at(i)==0){//means there is no suitable node
            continue;
        }
        QList<Node> sub_all_nodes=direction_all_nodes.at(i);
        weight_temp=0;
        for(int j=0;j<sub_all_nodes.size();j++)
        {
            Node sub_node=sub_all_nodes.at(j);
            V3DLONG vector1_x=source_node.x-sub_node.x;
            V3DLONG vector1_y=source_node.y-sub_node.y;
            V3DLONG vector1_z=source_node.z-sub_node.z;

            V3DLONG vector2_x=source_node.x-target_node.x;
            V3DLONG vector2_y=source_node.y-target_node.y;
            V3DLONG vector2_z=source_node.z-target_node.z;
            double angle=(vector1_x*vector2_x+vector1_y*vector2_y+vector1_z*vector2_z)/((double)sqrt((double)(vector1_x*vector1_x+vector1_y*vector1_y+vector1_z*vector1_z))*(double)sqrt((double)(vector2_x*vector2_x+vector2_y*vector2_y+vector2_z*vector2_z)));
            if(((double)sqrt((double)(vector1_x*vector1_x+vector1_y*vector1_y+vector1_z*vector1_z))*(double)sqrt((double)(vector2_x*vector2_x+vector2_y*vector2_y+vector2_z*vector2_z)))==0){
                printf("denominator equals 0 (2)\n");

            }
            double distance_between_can_tar=(double)sqrt((double)(sub_node.x-target_node.x)*(sub_node.x-target_node.x)+(sub_node.y-target_node.y)*(sub_node.y-target_node.y)+(sub_node.z-target_node.z)*(sub_node.z-target_node.z));
            weight_temp=weight_temp+angle*(img1d[GET_IND(sub_node.x,sub_node.y,sub_node.z)]*img1d[GET_IND(target_node.x,target_node.y,target_node.z)])/(distance_between_can_tar*distance_between_can_tar);

        }
        if(sub_all_nodes.size()==0){
            printf("denominator equals 0 (1)\n");

        }
        weight_temp=weight_temp/sub_all_nodes.size();

        if(weight<weight_temp)
        {
            weight=weight_temp;
            //angle_final=angle;
            node_number=i;
            // node_numbers=direction_node_number.at(i);

        }
    }//choose the region
    // printf("node_number::%d\n",node_number);
    double distance_minimum=INF;
    int minimum_number=0;

    for(int k=0;k<direction_all_nodes.at(node_number).size();k++)
    {//choose the node in the area
        Node temp=direction_all_nodes.at(node_number).at(k);
        double dis=(double)sqrt((double)(temp.x-target_node.x)*(temp.x-target_node.x)+(temp.y-target_node.y)*(temp.y-target_node.y)+(temp.z-target_node.z)*(temp.z-target_node.z));
        if(distance_minimum>dis)
        {
            distance_minimum=dis;
            minimum_number=k;

        }

    }


    if(weight==-INF){//this maybe means that there is only one center node in the sphere
        // printf("minimum_number::%d %d\n",minimum_number,direction_all_nodes.at(node_number).size());
        direction_node_number.clear();
        direction_all_nodes.clear();
        direction_node.clear();
        return result_pseu;
    }else{
        //printf("minimum_number::%d %d\n",minimum_number,direction_all_nodes.at(node_number).size());
        Node result_return=direction_all_nodes.at(node_number).at(minimum_number);
        // printf("minimum_number11::%d\n",minimum_number);
        // printf("select_angle::%lf\n",angle_final);
        //printf("select_direction::%d\n",node_number);
        direction_node_number.clear();
        // direction_average_pixal.clear();
        //direction_node_pixal.clear();
        direction_all_nodes.clear();
        direction_node.clear();
        return result_return;
    }

}

Node choose_region_vn3(unsigned char * &img1d,struct sphere_model_four_directions sphere_m,Node source_node,Node target_node,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    //four directions
    QList<V3DLONG> direction_node_number;
    QList<double> direction_average_pixal;
    QList<double> direction_node_pixal;
    QList<QList<Node> >  direction_all_nodes;
    QList<Node> direction_node;

    direction_node_pixal.append(sphere_m.pixal_up_right);
    direction_node_pixal.append(sphere_m.pixal_up_left);
    direction_node_pixal.append(sphere_m.pixal_down_right);
    direction_node_pixal.append(sphere_m.pixal_down_left);

    direction_node.append(sphere_m.node_up_right);
    direction_node.append(sphere_m.node_up_left);
    direction_node.append(sphere_m.node_down_right);
    direction_node.append(sphere_m.node_down_left);

    direction_node_number.append(sphere_m.up_right);
    direction_node_number.append(sphere_m.up_left);
    direction_node_number.append(sphere_m.down_right);
    direction_node_number.append(sphere_m.down_left);

    direction_all_nodes.append(sphere_m.QList_up_right);
    direction_all_nodes.append(sphere_m.QList_up_left);
    direction_all_nodes.append(sphere_m.QList_down_right);
    direction_all_nodes.append(sphere_m.QList_down_left);

    double average_pixval_1=(sphere_m.up_right==0)?0:sphere_m.sum_pixal_up_right/sphere_m.up_right;
    direction_average_pixal.append(average_pixval_1);
    double average_pixval_2=(sphere_m.up_left==0)?0:sphere_m.sum_pixal_up_left/sphere_m.up_left;
    direction_average_pixal.append(average_pixval_2);
    double average_pixval_3=(sphere_m.down_right==0)?0:sphere_m.sum_pixal_down_right/sphere_m.down_right;
    direction_average_pixal.append(average_pixval_3);
    double average_pixval_4=(sphere_m.down_left==0)?0:sphere_m.sum_pixal_down_left/sphere_m.down_left;
    direction_average_pixal.append(average_pixval_4);
    double weight=-INF;
    double weight_temp=-INF;
    int node_number=0;
    double angle_final=0;
    V3DLONG node_numbers=0;

    for(int i=0;i<direction_node.size();i++)
    {
        if(direction_node_number.at(i)==0){//means there is no suitable node
            continue;
        }
        QList<Node> sub_all_nodes=direction_all_nodes.at(i);
        weight_temp=0;
        for(int j=0;j<sub_all_nodes.size();j++){
            Node sub_node=sub_all_nodes.at(j);
            V3DLONG vector1_x=source_node.x-sub_node.x;
            V3DLONG vector1_y=source_node.y-sub_node.y;
            V3DLONG vector1_z=source_node.z-sub_node.z;

            V3DLONG vector2_x=source_node.x-target_node.x;
            V3DLONG vector2_y=source_node.y-target_node.y;
            V3DLONG vector2_z=source_node.z-target_node.z;
            double angle=(vector1_x*vector2_x+vector1_y*vector2_y+vector1_z*vector2_z)/((double)sqrt((double)(vector1_x*vector1_x+vector1_y*vector1_y+vector1_z*vector1_z))*(double)sqrt((double)(vector2_x*vector2_x+vector2_y*vector2_y+vector2_z*vector2_z)));
            double distance_between_can_tar=(double)sqrt((double)(sub_node.x-target_node.x)*(sub_node.x-target_node.x)+(sub_node.y-target_node.y)*(sub_node.y-target_node.y)+(sub_node.z-target_node.z)*(sub_node.z-target_node.z));
            weight_temp=weight_temp+angle*(img1d[GET_IND(sub_node.x,sub_node.y,sub_node.z)]*img1d[GET_IND(target_node.x,target_node.y,target_node.z)])/(distance_between_can_tar*distance_between_can_tar);

        }
        weight_temp=weight_temp/sub_all_nodes.size();

        if(weight<weight_temp)
        {
            weight=weight_temp;
            //angle_final=angle;
            node_number=i;
            // node_numbers=direction_node_number.at(i);

        }
    }//choose the region
    double distance_minimum=INF;
    int minimum_number=0;

    for(int k=0;k<direction_all_nodes.at(node_number).size();k++){//choose the node in the area
        Node temp=direction_all_nodes.at(node_number).at(k);
        double dis=(double)sqrt((double)(temp.x-target_node.x)*(temp.x-target_node.x)+(temp.y-target_node.y)*(temp.y-target_node.y)+(temp.z-target_node.z)*(temp.z-target_node.z));
        if(distance_minimum>dis)
        {
            distance_minimum=dis;
            minimum_number=k;

        }

    }
    Node result_return=direction_all_nodes.at(node_number).at(minimum_number);
    // printf("select_angle::%lf\n",angle_final);
    //printf("select_direction::%d\n",node_number);
    direction_node_number.clear();
    direction_average_pixal.clear();
    direction_node_pixal.clear();
    direction_all_nodes.clear();
    direction_node.clear();

    if(weight==-INF){//this maybe means that there is only one center node in the sphere
        Node result;
        result.x=-1;
        return result;
    }else{
        return result_return;
    }

}

Node choose_region_vn2(unsigned char * &img1d,struct sphere_model sphere_m,Node source_node,Node target_node,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z){
    //eight directions
    QList<V3DLONG> direction_node_number;
    QList<double> direction_average_pixal;
    QList<double> direction_node_pixal;
    QList<QList<Node> >  direction_all_nodes;
    QList<Node> direction_node;

    direction_node_pixal.append(sphere_m.pixal_up_right_up);
    direction_node_pixal.append(sphere_m.pixal_up_left_up);
    direction_node_pixal.append(sphere_m.pixal_up_left_bottom);
    direction_node_pixal.append(sphere_m.pixal_up_right_bottom);
    direction_node_pixal.append(sphere_m.pixal_down_right_up);
    direction_node_pixal.append(sphere_m.pixal_down_left_up);
    direction_node_pixal.append(sphere_m.pixal_down_left_bottom);
    direction_node_pixal.append(sphere_m.pixal_down_right_bottom);

    direction_node.append(sphere_m.node_up_right_up);
    direction_node.append(sphere_m.node_up_left_up);
    direction_node.append(sphere_m.node_up_left_bottom);
    direction_node.append(sphere_m.node_up_right_bottom);
    direction_node.append(sphere_m.node_down_right_up);
    direction_node.append(sphere_m.node_down_left_up);
    direction_node.append(sphere_m.node_down_left_bottom);
    direction_node.append(sphere_m.node_down_right_bottom);

    direction_node_number.append(sphere_m.up_right_up);
    direction_node_number.append(sphere_m.up_left_up);
    direction_node_number.append(sphere_m.up_left_bottom);
    direction_node_number.append(sphere_m.up_right_bottom);
    direction_node_number.append(sphere_m.down_right_up);
    direction_node_number.append(sphere_m.down_left_up);
    direction_node_number.append(sphere_m.down_left_bottom);
    direction_node_number.append(sphere_m.down_right_bottom);

    direction_all_nodes.append(sphere_m.QList_up_right_up);
    direction_all_nodes.append(sphere_m.QList_up_left_up);
    direction_all_nodes.append(sphere_m.QList_up_left_bottom);
    direction_all_nodes.append(sphere_m.QList_up_right_bottom);
    direction_all_nodes.append(sphere_m.QList_down_right_up);
    direction_all_nodes.append(sphere_m.QList_down_left_up);
    direction_all_nodes.append(sphere_m.QList_down_left_bottom);
    direction_all_nodes.append(sphere_m.QList_down_right_bottom);

    double average_pixval_1=(sphere_m.up_right_up==0)?0:sphere_m.sum_pixal_up_right_up/sphere_m.up_right_up;
    direction_average_pixal.append(average_pixval_1);
    double average_pixval_2=(sphere_m.up_left_up==0)?0:sphere_m.sum_pixal_up_left_up/sphere_m.up_left_up;
    direction_average_pixal.append(average_pixval_2);
    double average_pixval_3=(sphere_m.up_left_bottom==0)?0:sphere_m.sum_pixal_up_left_bottom/sphere_m.up_left_bottom;
    direction_average_pixal.append(average_pixval_3);
    double average_pixval_4=(sphere_m.up_right_bottom==0)?0:sphere_m.sum_pixal_up_right_bottom/sphere_m.up_right_bottom;
    direction_average_pixal.append(average_pixval_4);
    double average_pixval_5=(sphere_m.down_right_up==0)?0:sphere_m.sum_pixal_down_right_up/sphere_m.down_right_up;
    direction_average_pixal.append(average_pixval_5);
    double average_pixval_6=(sphere_m.down_left_up==0)?0:sphere_m.sum_pixal_down_left_up/sphere_m.down_left_up;
    direction_average_pixal.append(average_pixval_6);
    double average_pixval_7=(sphere_m.down_left_bottom==0)?0:sphere_m.sum_pixal_down_left_bottom/sphere_m.down_left_bottom;
    direction_average_pixal.append(average_pixval_7);
    double average_pixval_8=(sphere_m.down_right_bottom==0)?0:sphere_m.sum_pixal_down_right_bottom/sphere_m.down_right_bottom;
    direction_average_pixal.append(average_pixval_8);
    double weight=-INF;
    double weight_temp=-INF;
    int node_number=0;
    double angle_final=0;
    V3DLONG node_numbers=0;

    for(int i=0;i<direction_node.size();i++)
    {
        if(direction_node_number.at(i)==0){//means there is no suitable node
            continue;
        }
        QList<Node> sub_all_nodes=direction_all_nodes.at(i);
        weight_temp=0;
        for(int j=0;j<sub_all_nodes.size();j++){
            Node sub_node=sub_all_nodes.at(j);
            V3DLONG vector1_x=source_node.x-sub_node.x;
            V3DLONG vector1_y=source_node.y-sub_node.y;
            V3DLONG vector1_z=source_node.z-sub_node.z;

            V3DLONG vector2_x=source_node.x-target_node.x;
            V3DLONG vector2_y=source_node.y-target_node.y;
            V3DLONG vector2_z=source_node.z-target_node.z;
            double angle=(vector1_x*vector2_x+vector1_y*vector2_y+vector1_z*vector2_z)/((double)sqrt((double)(vector1_x*vector1_x+vector1_y*vector1_y+vector1_z*vector1_z))*(double)sqrt((double)(vector2_x*vector2_x+vector2_y*vector2_y+vector2_z*vector2_z)));
            double distance_between_can_tar=(double)sqrt((double)(sub_node.x-target_node.x)*(sub_node.x-target_node.x)+(sub_node.y-target_node.y)*(sub_node.y-target_node.y)+(sub_node.z-target_node.z)*(sub_node.z-target_node.z));
            weight_temp=weight_temp+angle*(img1d[GET_IND(sub_node.x,sub_node.y,sub_node.z)]*img1d[GET_IND(target_node.x,target_node.y,target_node.z)])/(distance_between_can_tar*distance_between_can_tar);

        }
        weight_temp=weight_temp/sub_all_nodes.size();

        if(weight<weight_temp)
        {
            weight=weight_temp;
            //angle_final=angle;
            node_number=i;
            // node_numbers=direction_node_number.at(i);

        }
    }//choose the region
    double distance_minimum=INF;
    int minimum_number=0;

    for(int k=0;k<direction_all_nodes.at(node_number).size();k++){//choose the node in the area
        Node temp=direction_all_nodes.at(node_number).at(k);
        double dis=(double)sqrt((double)(temp.x-target_node.x)*(temp.x-target_node.x)+(temp.y-target_node.y)*(temp.y-target_node.y)+(temp.z-target_node.z)*(temp.z-target_node.z));
        if(distance_minimum>dis){
            distance_minimum=dis;
            minimum_number=k;

        }

    }
    Node result_return=direction_all_nodes.at(node_number).at(minimum_number);
    // printf("select_angle::%lf\n",angle_final);
    //printf("select_direction::%d\n",node_number);
    direction_node_number.clear();
    direction_average_pixal.clear();
    direction_node_pixal.clear();
    direction_all_nodes.clear();
    direction_node.clear();

    if(weight==-INF){//this maybe means that there is only one center node in the sphere
        Node result;
        result.x=-1;
        return result;
    }else{
        return result_return;
    }

}

Node choose_region(struct sphere_model sphere_m,Node source_node,Node target_node){

    QList<V3DLONG> direction_node_number;
    QList<double> direction_average_pixal;
    QList<double> direction_node_pixal;
    QList<Node> direction_node;

    direction_node_pixal.append(sphere_m.pixal_up_right_up);
    direction_node_pixal.append(sphere_m.pixal_up_left_up);
    direction_node_pixal.append(sphere_m.pixal_up_left_bottom);
    direction_node_pixal.append(sphere_m.pixal_up_right_bottom);
    direction_node_pixal.append(sphere_m.pixal_down_right_up);
    direction_node_pixal.append(sphere_m.pixal_down_left_up);
    direction_node_pixal.append(sphere_m.pixal_down_left_bottom);
    direction_node_pixal.append(sphere_m.pixal_down_right_bottom);

    direction_node.append(sphere_m.node_up_right_up);
    direction_node.append(sphere_m.node_up_left_up);
    direction_node.append(sphere_m.node_up_left_bottom);
    direction_node.append(sphere_m.node_up_right_bottom);
    direction_node.append(sphere_m.node_down_right_up);
    direction_node.append(sphere_m.node_down_left_up);
    direction_node.append(sphere_m.node_down_left_bottom);
    direction_node.append(sphere_m.node_down_right_bottom);

    direction_node_number.append(sphere_m.up_right_up);
    direction_node_number.append(sphere_m.up_left_up);
    direction_node_number.append(sphere_m.up_left_bottom);
    direction_node_number.append(sphere_m.up_right_bottom);
    direction_node_number.append(sphere_m.down_right_up);
    direction_node_number.append(sphere_m.down_left_up);
    direction_node_number.append(sphere_m.down_left_bottom);
    direction_node_number.append(sphere_m.down_right_bottom);

    double average_pixval_1=(sphere_m.up_right_up==0)?0:sphere_m.sum_pixal_up_right_up/sphere_m.up_right_up;
    direction_average_pixal.append(average_pixval_1);
    double average_pixval_2=(sphere_m.up_left_up==0)?0:sphere_m.sum_pixal_up_left_up/sphere_m.up_left_up;
    direction_average_pixal.append(average_pixval_2);
    double average_pixval_3=(sphere_m.up_left_bottom==0)?0:sphere_m.sum_pixal_up_left_bottom/sphere_m.up_left_bottom;
    direction_average_pixal.append(average_pixval_3);
    double average_pixval_4=(sphere_m.up_right_bottom==0)?0:sphere_m.sum_pixal_up_right_bottom/sphere_m.up_right_bottom;
    direction_average_pixal.append(average_pixval_4);
    double average_pixval_5=(sphere_m.down_right_up==0)?0:sphere_m.sum_pixal_down_right_up/sphere_m.down_right_up;
    direction_average_pixal.append(average_pixval_5);
    double average_pixval_6=(sphere_m.down_left_up==0)?0:sphere_m.sum_pixal_down_left_up/sphere_m.down_left_up;
    direction_average_pixal.append(average_pixval_6);
    double average_pixval_7=(sphere_m.down_left_bottom==0)?0:sphere_m.sum_pixal_down_left_bottom/sphere_m.down_left_bottom;
    direction_average_pixal.append(average_pixval_7);
    double average_pixval_8=(sphere_m.down_right_bottom==0)?0:sphere_m.sum_pixal_down_right_bottom/sphere_m.down_right_bottom;
    direction_average_pixal.append(average_pixval_8);
    double weight=INF;
    double weight_temp=INF;
    int node_number=0;
    double angle_final=0;
    V3DLONG node_numbers=0;

    for(int i=0;i<direction_node.size();i++)
    {
        if(direction_node_number.at(i)==0){//means there is no suitable node
            continue;
        }
        Node temp=direction_node.at(i);
        V3DLONG vector1_x=source_node.x-temp.x;
        V3DLONG vector1_y=source_node.y-temp.y;
        V3DLONG vector1_z=source_node.z-temp.z;

        V3DLONG vector2_x=source_node.x-target_node.x;
        V3DLONG vector2_y=source_node.y-target_node.y;
        V3DLONG vector2_z=source_node.z-target_node.z;
        //calculate the distance between candidate node and target node
        double distance_between_can_tar=(double)sqrt((double)(temp.x-target_node.x)*(temp.x-target_node.x)+(temp.y-target_node.y)*(temp.y-target_node.y)+(temp.z-target_node.z)*(temp.z-target_node.z));

        double angle=(vector1_x*vector2_x+vector1_y*vector2_y+vector1_z*vector2_z)/((double)sqrt((double)(vector1_x*vector1_x+vector1_y*vector1_y+vector1_z*vector1_z))*(double)sqrt((double)(vector2_x*vector2_x+vector2_y*vector2_y+vector2_z*vector2_z)));
        //weight_temp=angle*log((2*direction_node_number.at(i)+direction_average_pixal.at(i)));
        //weight_temp=exp((double)angle*direction_node_number.at(i));
        weight_temp=angle*exp((double)(2*direction_node_number.at(i)+direction_average_pixal.at(i)));
        weight_temp=distance_between_can_tar;
        if(weight>weight_temp)
        {
            weight=weight_temp;
            angle_final=angle;
            node_number=i;
            node_numbers=direction_node_number.at(i);

        }
    }
    // printf("select_angle::%lf\n",angle_final);
    // printf("select_direction::%d\n",node_number);
    direction_node_number.clear();
    direction_average_pixal.clear();
    direction_node_pixal.clear();
    // direction_node.clear();

    if(weight==INF){//this maybe means that there is only one center node in the sphere
        Node result;
        result.x=-1;
        return result;
    }else{
        return direction_node.at(node_number);
    }

}

QMap<int,Node*> delete_cluster_node(unsigned char * &img1d,QMap<V3DLONG,QList<Node> >  final_cluster,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG *in_sz,double prim_distance){//every classification node also contained itself
    QMap<int,Node*> result;
    QMap<V3DLONG,QMap<V3DLONG,Node> >  mid_result;


    for(QMap<V3DLONG,QList<Node> >::iterator iter =final_cluster.begin(); iter != final_cluster.end(); iter++){//clean the final_cluster, change its type to QMap<V3DLONG,QMap<V3DLONG,Node> > for easy process
        QList<Node> cluster_list=iter.value();
        QMap<V3DLONG,Node> temp;
        temp.clear();
        for(int i=0;i<cluster_list.size();i++){
            Node temp_node=cluster_list.at(i);
            Node *new_temp_node=new Node(temp_node.x,temp_node.y,temp_node.z);
            temp_node.r= markerRadiusXY(img1d, in_sz, *new_temp_node, 20);
            temp.insert(GET_IND(temp_node.x,temp_node.y,temp_node.z),temp_node);
            delete new_temp_node;

        }
        mid_result.insert(iter.key(),temp);
    }
    //begin to merge the node for each classification

    int count_number=0;
    for(QMap<V3DLONG,QMap<V3DLONG,Node> > ::iterator iter =mid_result.begin(); iter != mid_result.end(); iter++){

        QMap<V3DLONG,Node> elem_Map=iter.value();
        QMap<V3DLONG,Node> after_merge=merge_cluster_node(elem_Map,prim_distance);
        //printf("%d\n",after_merge.count());
        /* if(after_merge.count()==1){
            V3DLONG key=after_merge.begin().key();
            Node test=after_merge.begin().value();
            //printf("baohan: %d\n",node_afterMerge.contains(key));
            // printf("baohan: %d\n",key==GET_IND(test.x,test.y,test.z));
        }*/
        //change the type of QMap<V3DLONG,Node> to QMap<int,Node*>
        for(QMap<V3DLONG,Node>::iterator iter1=after_merge.begin();iter1!=after_merge.end();iter1++){
            Node temp1=iter1.value();
            Node* temp2=new Node(temp1.x,temp1.y,temp1.z);
            result.insert(count_number,temp2);
            count_number++;

        }
        //result.insert(iter.key(),after_merge);
    }

    return result;


}

QMap<V3DLONG,QList<Node> > cluster2newroot(QMap<V3DLONG,QList<V3DLONG> > covered,QMultiMap<V3DLONG,Node> cluster){
    QMap<V3DLONG,QList<Node> > result;
    for(QMap<V3DLONG,QList<V3DLONG> >::iterator iter =covered.begin(); iter != covered.end(); iter++){
        QList<V3DLONG> subcovered=iter.value();
        if(subcovered.size()==0&&!(node_afterMerge.contains(iter.key())))
            continue;
        QList<Node> temp;
        temp.clear();
        for(int i=0;i<subcovered.size();i++){
            V3DLONG index=subcovered.at(i);
            QMultiMap<V3DLONG,Node>::iterator begin=cluster.lowerBound(index);//equal or greater
            QMultiMap<V3DLONG,Node>::iterator end=cluster.upperBound(index);//greater
            while(begin!=end){
                Node add_node=begin.value();
                //result.value(iter.key()).append(add_node);
                temp.append(add_node);

                begin++;

            }

        }

        QMultiMap<V3DLONG,Node>::iterator begin=cluster.lowerBound(iter.key());//add the node belongs to itself
        QMultiMap<V3DLONG,Node>::iterator end=cluster.upperBound(iter.key());
        while(begin!=end){
            // result.value(iter.key()).append(begin.value());
            temp.append(begin.value());
            begin++;
        }
        Node* temp_root= node_afterMerge.value(iter.key());//add the root itself as its sub-element
        temp.append(getnode(temp_root));

        result.insert(iter.key(),temp);

    }
    return result;
}

bool determine_noisy(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,double threshold,double percentage,int iter_step=1){
    double number_count=0;
    double node_number=0;
    bool result=false;
    for(int times=0;times<26;times++)
    {
        V3DLONG offset_x=x+node_table[times].i*iter_step;
        V3DLONG offset_y=y+node_table[times].j*iter_step;
        V3DLONG offset_z=z+node_table[times].k*iter_step;
        if(offset_x>=sz_x){
            continue;
        }else if(offset_x<0){
            continue;
        }
        if(offset_y>=sz_y){
            continue;
        }else if(offset_y<0){
            continue;
        }
        if(offset_z>=sz_z){
            continue;
        }else if(offset_z<0){
            continue;
        }
        node_number++;
        double pixal_value=img1d[GET_IND(offset_x,offset_y,offset_z)];
        if(pixal_value>threshold){
            number_count++;
        }


    }
    if(node_number==0)
        return false;
    if((number_count/node_number)>=percentage){
        result=true;

    }

    return result;

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
QMap<V3DLONG,Node> merge_cluster_node(QMap<V3DLONG,Node> rootnodes,double distance)
{

    QMap<int,int> evidence2;
    QMap<V3DLONG,Node> result;
    for(QMap<V3DLONG,Node>::iterator iter =rootnodes.begin(); iter != rootnodes.end(); iter++)
    {
        V3DLONG key=iter.key();
        evidence2.insert(key,0);
    }
    for(QMap<V3DLONG,Node>::iterator iter1 =rootnodes.begin(); iter1 != rootnodes.end(); iter1++){

        Node elem=(iter1.value());
        // V3DLONG index1=GET_IND(elem.x,elem.y,elem.z);

        if ((evidence2.value(iter1.key())==1)){

            continue;
        }

        for(QMap<V3DLONG,Node>::iterator iter2 =rootnodes.begin(); iter2 != rootnodes.end(); iter2++){
            if((iter2.key()==iter1.key())||(evidence2.value(iter2.key())==1)){

                //evidence2.insert(iter2.value(),1);
                continue;
            }
            Node elem1=iter2.value();
            // V3DLONG index2=GET_IND(elem1->x,elem1->y,elem1->z);
            double dis=(double)sqrt((double)(elem.x-elem1.x)*(elem.x-elem1.x)+(double)(elem.y-elem1.y)*(elem.y-elem1.y)+(double)(elem.z-elem1.z)*(elem.z-elem1.z));

            if((dis/(elem.r+elem1.r)<distance)&&(!node_afterMerge.contains(iter2.key()))){//if some nodes cover root, do not mark the root as deletion-able;
                evidence2.insert(iter2.key(),1);

            }
        }
    }

    for(QMap<V3DLONG,Node>::iterator iter3 =rootnodes.begin(); iter3 != rootnodes.end(); iter3++){
        Node element=iter3.value();
        if(evidence2.value(iter3.key())==0){
            result.insert(iter3.key(),element);
        }
    }
    return result;

}



QMap<int,Node*> merge_rootnode_vn2(QMap<int,Node*> &rootnodes,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,double distance)
{
    printf("root size before merge:%d\n",rootnodes.size());
    QMultiMap<double, int> r_root;
    QMap<int,int> evidence2;
    QMap<int,Node*> result;

    for(QMap<int,Node*>::iterator iter =rootnodes.begin(); iter != rootnodes.end(); iter++)
    {//check which root include which roots
        Node* elem=iter.value();
        int key=iter.key();
        r_root.insert(elem->r,key);
        evidence2.insert(key,0);
    }
    for(QMap<int,Node*>::iterator iter1 =rootnodes.begin(); iter1 != rootnodes.end(); iter1++){
        QList<V3DLONG> subnode_index;
        Node* elem=(iter1.value());
        V3DLONG index1=GET_IND(elem->x,elem->y,elem->z);
        subnode_index.clear();

        if ((evidence2.value(iter1.key())==1)){
            covered_relation.insert(index1,subnode_index);
            continue;
        }

        for(QMap<int,Node*>::iterator iter2 =rootnodes.begin(); iter2 != rootnodes.end(); iter2++){
            if((iter2.key()==iter1.key())||(evidence2.value(iter2.key())==1)){

                //evidence2.insert(iter2.value(),1);
                continue;
            }
            Node* elem1=iter2.value();
            V3DLONG index2=GET_IND(elem1->x,elem1->y,elem1->z);
            double dis=(double)sqrt((double)(elem->x-elem1->x)*(elem->x-elem1->x)+(double)(elem->y-elem1->y)*(elem->y-elem1->y)+(double)(elem->z-elem1->z)*(elem->z-elem1->z));

            if(dis/(elem->r+elem1->r)<distance){
                evidence2.insert(iter2.key(),1);
                subnode_index.append(index2);
            }

        }
        covered_relation.insert(index1,subnode_index);//in the Map of covered_relation, two situations have been included, one is the root is covered by others, and its related QList contain 0 element, the other is the root did not contain any other roots which means the length of QList is also 0

    }
    /* for(QMultiMap<double,int>::iterator iter1=r_root.end();iter1!=r_root.begin();iter1--){

        if ((evidence2.value(iter1.value())==1)||(iter1==r_root.end()))
            continue;
        Node* elem=rootnodes.value(iter1.value());
        for(QMultiMap<double,int>::iterator iter2=r_root.end();iter2!=r_root.begin();iter2--){

            if((iter2.value()==iter1.value())||(iter2==r_root.end())||(evidence2.value(iter2.value())==1)){

                //evidence2.insert(iter2.value(),1);
                continue;
            }
            Node* elem1=rootnodes.value(iter2.value());
            double dis=(double)sqrt((double)(elem->x-elem1->x)*(elem->x-elem1->x)+(double)(elem->y-elem1->y)*(elem->y-elem1->y)+(double)(elem->z-elem1->z)*(elem->z-elem1->z));
            int p;

            if(dis/(elem->r+elem1->r)<1.0){
                evidence2.insert(iter2.value(),1);

            }
        }

    }*/
    for(QMap<int,Node*>::iterator iter3 =rootnodes.begin(); iter3 != rootnodes.end(); iter3++){
        Node* element=iter3.value();
        if(evidence2.value(iter3.key())==0){
            result.insert(iter3.key(),iter3.value());
            node_afterMerge.insert(GET_IND(element->x,element->y,element->z),element);

        }
    }

    printf("root size after merge:%d\n",result.size());
    return result;

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


    //	printSwcByMap(root,"C:\\Vaa3D\\finalroot.swc");


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

bool found_final_vn3(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r)
{
    double intensity=img1d[GET_IND(x,y,z)];
    double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;

    V3DLONG xe=x-r;if(xe<0) xe=0;
    V3DLONG xb=x+r;if(xb>sz_x) xb=sz_x;
    V3DLONG ye=y-r;if(ye<0) ye=0;
    V3DLONG yb=y+r;if(yb>sz_y) yb=sz_y;
    V3DLONG ze=z-r;if(ze<0) ze=0;
    V3DLONG zb=z+r;if(zb>sz_z) zb=sz_z;
    for(V3DLONG i=xe;i<xb;i++)
    {
        for(V3DLONG j=ye;j<yb;j++)
        {
            for(V3DLONG k=ze;k<zb;k++)
            {

                if(GET_IND(i,j,k)==GET_IND(x,y,z)) continue;
                if(img1d[GET_IND(i,j,k)]<1.0) continue;
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

    V3DLONG next_x=(long)(sum1_x/sum2_x);
    V3DLONG next_y=(long)(sum1_y/sum2_y);
    V3DLONG next_z=(long)(sum1_z/sum2_z);
    if(sqrt((double)(next_x-x)*(next_x-x)+(next_y-y)*(next_y-y)+(next_z-z)*(next_z-z))<1.0){
        return false;

    }else{
        return true;
    }
}

long meanshift_vn7(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration,V3DLONG *in_sz)
{
    int iter=0;
    Node* center=new Node(x,y,z);//get the current node and shift it later until it arrived at a final root
    Node cur_center=getnode(center);
    bool flag_zero;

    // QList<Node*> choose_next;
    Node *pre_node=new Node(x,y,z);
    double intensity=img1d[GET_IND(x,y,z)];
    while(iter<iteration)
    {
        QList<Node> choose_next;
        choose_next.clear();
        flag_zero=false;
        double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
        V3DLONG xe=cur_center.x-r;if(xe<0) xe=0;
        V3DLONG xb=cur_center.x+r;if(xb>sz_x) xb=sz_x;
        V3DLONG ye=cur_center.y-r;if(ye<0) ye=0;
        V3DLONG yb=cur_center.y+r;if(yb>sz_y) yb=sz_y;
        V3DLONG ze=cur_center.z-r;if(ze<0) ze=0;
        V3DLONG zb=cur_center.z+r;if(zb>sz_z) zb=sz_z;

        int count=0;

        for(V3DLONG i=xe;i<xb;i++)
        {
            for(V3DLONG j=ye;j<yb;j++)
            {
                for(V3DLONG k=ze;k<zb;k++)
                {

                    if(GET_IND(i,j,k)==GET_IND(cur_center.x,cur_center.y,cur_center.z)) continue;
                    if((double)img1d[GET_IND(i,j,k)]<1.0) continue;
                    double cur_intensity=img1d[GET_IND(i,j,k)];
                    w=cal_weight(i,j,k,cur_center.x,cur_center.y,cur_center.z,cur_intensity,intensity,r);

                    double core_z=cal_core(k,cur_center.z,r);
                    sum1_z+=core_z*w*k;
                    sum2_z+=core_z*w;
                    //printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

                    double core_y=cal_core(j,cur_center.y,r);
                    sum1_y+=core_y*w*j;
                    sum2_y+=core_y*w;

                    double core_x=cal_core(i,cur_center.x,r);
                    sum1_x+=core_x*w*i;
                    sum2_x+=core_x*w;
                    if((double)img1d[GET_IND(i,j,k)]<30){

                    }else{
                        count++;
                        Node* relative=new Node(i,j,k);
                        choose_next.append(getnode(relative));//collect the candidation node and prepare for deciding the final node
                        delete relative;
                        // printf("%ld %ld %ld %d\n",relative->x,relative->y,relative->z,img1d[GET_IND(relative->x,relative->y,relative->z)]);
                    }
                }
            }
        }
        // final_node=pre_node;
        if(sum2_x==0||sum2_y==0||sum2_z==0||choose_next.size()==0){
            // printf("sum_xyz equals 0,program will crash!\n");
            flag_zero=true;
            break;


        }
        cur_center.x=(long)sum1_x/sum2_x;
        cur_center.y=(long)sum1_y/sum2_y;
        cur_center.z=(long)sum1_z/sum2_z;

        if(sqrt((double)(pre_node->x-cur_center.x)*(pre_node->x-cur_center.x)+(pre_node->y-cur_center.y)*(pre_node->y-cur_center.y)+(pre_node->z-cur_center.z)*(pre_node->z-cur_center.z))<1.0){

            // delete pre_node;
            // delete final_node;
            // qDeleteAll(choose_next);

            choose_next.clear();
            break;
        }


        //get the direction and choose next pixal which in the above box and with an intensity greater then 30
        //printf("%ld %ld %ld \n",cur_center->x,cur_center->y,cur_center->z);
        cur_center=choose_next_node(img1d,pre_node,cur_center.x,cur_center.y,cur_center.z,choose_next,sz_x,sz_y,sz_z);
        intensity=img1d[GET_IND(cur_center.x,cur_center.y,cur_center.z)];
        //printf("%ld %ld %ld %lf\n",cur_center.x,cur_center.y,cur_center.z,intensity);
        // printf("%d %d %d %d %d\n",choose_next.size(),count,sum2_y,sum2_y,sum2_z);
        // pre_node=cur_center;
        // pre_node=new Node(cur_center->x,cur_center->y,cur_center->z);
        pre_node->x=cur_center.x;
        pre_node->y=cur_center.y;
        pre_node->z=cur_center.z;

        iter++;
        //delete pre_node;
        // delete final_node;
        //qDeleteAll(choose_next);
        choose_next.clear();
    }//while
    // printf("%d\n",iter);

    //V3DLONG index=GET_IND(final_node->x,final_node->y,final_node->z);
    //Node* write=new Node(pre_node->x,pre_node->y,pre_node->z);

    //printf("%ld %ld %ld \n",pre_node->x,pre_node->y,pre_node->z);

    if(flag_zero||iter==iteration){
        delete center;
        delete pre_node;
        return -1;//means there is only one node in that box, which means a noisy node

    }else{//get the final nodes
        V3DLONG ind=(V3DLONG)GET_IND(pre_node->x,pre_node->y,pre_node->z);
        pre_node->class_mark=cluster_number;
        pre_node->parent=-1;
        pre_node->number=0;
        if(Map_finalnode.contains(ind)){
            delete pre_node;
            return ind;

        }
        Map_finalnode.insert(ind,cluster_number);
        pre_node->r= markerRadiusXY(img1d, in_sz, *pre_node, 20);
        Map_rootnode.insert(cluster_number, pre_node);
        cluster_number++;
        //delete final;
        delete center;
        return ind;

    }



}
long meanshift_vn6(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration,V3DLONG *in_sz)
{
    int iter=0;
    Node* center=new Node(x,y,z);//get the current node and shift it later until it arrived at a final root
    Node cur_center=getnode(center);
    bool flag_zero;

    // QList<Node*> choose_next;
    Node *pre_node=new Node(x,y,z);
    double intensity=img1d[GET_IND(x,y,z)];
    while(iter<iteration)
    {
        QList<Node> choose_next;
        choose_next.clear();
        flag_zero=false;
        double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
        V3DLONG xe=cur_center.x-r;if(xe<0) xe=0;
        V3DLONG xb=cur_center.x+r;if(xb>sz_x) xb=sz_x;
        V3DLONG ye=cur_center.y-r;if(ye<0) ye=0;
        V3DLONG yb=cur_center.y+r;if(yb>sz_y) yb=sz_y;
        V3DLONG ze=cur_center.z-r;if(ze<0) ze=0;
        V3DLONG zb=cur_center.z+r;if(zb>sz_z) zb=sz_z;

        int count=0;

        for(V3DLONG i=xe;i<xb;i++)
        {
            for(V3DLONG j=ye;j<yb;j++)
            {
                for(V3DLONG k=ze;k<zb;k++)
                {

                    if(GET_IND(i,j,k)==GET_IND(cur_center.x,cur_center.y,cur_center.z)) continue;
                    if((double)img1d[GET_IND(i,j,k)]<1.0) continue;
                    double cur_intensity=img1d[GET_IND(i,j,k)];
                    w=cal_weight(i,j,k,cur_center.x,cur_center.y,cur_center.z,cur_intensity,intensity,r);

                    double core_z=cal_core(k,cur_center.z,r);
                    sum1_z+=core_z*w*k;
                    sum2_z+=core_z*w;
                    //printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

                    double core_y=cal_core(j,cur_center.y,r);
                    sum1_y+=core_y*w*j;
                    sum2_y+=core_y*w;

                    double core_x=cal_core(i,cur_center.x,r);
                    sum1_x+=core_x*w*i;
                    sum2_x+=core_x*w;
                    if((double)img1d[GET_IND(i,j,k)]<30){

                    }else{
                        count++;
                        Node* relative=new Node(i,j,k);
                        choose_next.append(getnode(relative));//collect the candidation node and prepare for deciding the final node
                        delete relative;
                        // printf("%ld %ld %ld %d\n",relative->x,relative->y,relative->z,img1d[GET_IND(relative->x,relative->y,relative->z)]);
                    }
                }
            }
        }


        // final_node=pre_node;
        if(sum2_x==0||sum2_y==0||sum2_z==0||choose_next.size()==0){
            printf("sum_xyz equals 0,program will crash!\n");

            flag_zero=true;
            break;


        }
        cur_center.x=(long)sum1_x/sum2_x;
        cur_center.y=(long)sum1_y/sum2_y;
        cur_center.z=(long)sum1_z/sum2_z;

        if(sqrt((double)(pre_node->x-cur_center.x)*(pre_node->x-cur_center.x)+(pre_node->y-cur_center.y)*(pre_node->y-cur_center.y)+(pre_node->z-cur_center.z)*(pre_node->z-cur_center.z))<1.0){

            // delete pre_node;
            // delete final_node;
            // qDeleteAll(choose_next);

            choose_next.clear();
            break;
        }


        //get the direction and choose next pixal which in the above box and with an intensity greater then 30
        //printf("%ld %ld %ld \n",cur_center->x,cur_center->y,cur_center->z);
        cur_center=choose_next_node(img1d,pre_node,cur_center.x,cur_center.y,cur_center.z,choose_next,sz_x,sz_y,sz_z);
        intensity=img1d[GET_IND(cur_center.x,cur_center.y,cur_center.z)];
        //printf("%ld %ld %ld %lf\n",cur_center.x,cur_center.y,cur_center.z,intensity);
        // printf("%d %d %d %d %d\n",choose_next.size(),count,sum2_y,sum2_y,sum2_z);
        // pre_node=cur_center;
        // pre_node=new Node(cur_center->x,cur_center->y,cur_center->z);
        pre_node->x=cur_center.x;
        pre_node->y=cur_center.y;
        pre_node->z=cur_center.z;

        iter++;
        //delete pre_node;
        // delete final_node;
        //qDeleteAll(choose_next);
        choose_next.clear();
    }//while
    printf("%d\n",iter);

    //V3DLONG index=GET_IND(final_node->x,final_node->y,final_node->z);
    //Node* write=new Node(pre_node->x,pre_node->y,pre_node->z);

    //printf("%ld %ld %ld \n",pre_node->x,pre_node->y,pre_node->z);

    if(flag_zero){
        delete center;
        delete pre_node;
        return -1;//means there is only one node in that box, which means a noisy node

    }else if(Map_finalnode.contains((V3DLONG)GET_IND(pre_node->x,pre_node->y,pre_node->z))){
        V3DLONG index_final=(V3DLONG)GET_IND(pre_node->x,pre_node->y,pre_node->z);
        delete center;
        delete pre_node;
        //  printf("111111111111111111111111111111111111\n");
        // QTest.append(pre_node);
        return index_final;

    }else{//due to some node cannot get a classification node or although the iteration is less then 1000 but still did not be contained by Map_finalnode(reason not clear)
        //pre_node->r= markerRadiusXY(img1d, in_sz, *pre_node, 10);
        /* if(iter==iteration){
            // QTest.append(pre_node);

        }else{
            QTest.append(pre_node);

        }*/

        // printf("%d\n",iter);
        // printf("%ld %ld %ld %ld %lf\n",pre_node->x,pre_node->y,pre_node->z,GET_IND(pre_node->x,pre_node->y,pre_node->z),(double)img1d[GET_IND(pre_node->x,pre_node->y,pre_node->z)]);
        // printf("the final nodes did not contain current node\n");
        delete center;
        delete pre_node;
        return -1;

    }



}

Node choose_next_node(unsigned char * &img1d,Node* pre,V3DLONG next_x,V3DLONG next_y,V3DLONG next_z,QList<Node> choose,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
    Node* cur_vector=new Node(pre->x-next_x,pre->y-next_y,pre->z-next_z);
    //QList<double> angles;
    int a=choose.size();
    double* angle=new double[a];
    //calculate the angle between vectors in box and current vector
    for(int i=0;i<a;i++){
        Node cur=choose.at(i);
        V3DLONG relative_x=pre->x-cur.x;
        V3DLONG relative_y=pre->y-cur.y;
        V3DLONG relative_z=pre->z-cur.z;
        // printf("%ld %ld %ld \n",cur->x,cur->y,cur->z);
        angle[i]=(double)(relative_x*cur_vector->x+relative_y*cur_vector->y+relative_z*cur_vector->z)/(sqrt((double)(relative_x*relative_x+relative_y*relative_y+relative_z*relative_z))*sqrt((double)(cur_vector->x*cur_vector->x+cur_vector->y*cur_vector->y+cur_vector->z*cur_vector->z)));
        // printf("%lf \n",angle[i]);
    }

    //sort the angle and finally choose the next node
    double maximum=angle[0];
    int pos=0;
    for(int i=0;i<a;i++){
        if(angle[i]>maximum){
            maximum=angle[i];
            pos=i;

        }
    }
    // printf("%d\n",pos);
    // printf("%ld %ld %ld %d %d\n",choose.at(pos)->x,choose.at(pos)->y,choose.at(pos)->z,choose.size(),pos);
    // printf("%ld %ld %ld %d\n",choose.at(pos)->x,choose.at(pos)->y,choose.at(pos)->z,img1d[GET_IND(choose.at(pos)->x,choose.at(pos)->y,choose.at(pos)->z)]);

    delete cur_vector;
    delete []angle;
    return choose.at(pos);

}

void meanshift_vn5(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration,V3DLONG *in_sz)
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
            // enlarge_radiusof_single_node_xy(img1d,pre_center,sz_x,sz_y,sz_z);


            pre_center->r= markerRadiusXY(img1d, in_sz, *pre_center, 40);


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

        int new_mark=Map_rootnode.size()+1;
        cur_center->class_mark=new_mark;

        //enlarge_radiusof_single_node_xy(img1d,cur_center,sz_x,sz_y,sz_z);
        if(cur_center->r==0)
            printf("%lf\n",cur_center->r);
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

        int new_mark=Map_rootnode.size()+1;
        cur_center->class_mark=new_mark;

        //enlarge_radiusof_single_node_xy(img1d,cur_center,sz_x,sz_y,sz_z);
        if(cur_center->r==0)
            printf("%lf\n",cur_center->r);
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

void printHelp()
{
    cout<<"\n**** Usage of meanshift tracing ****"<<endl;
    cout<<"\nvaa3d -x BJUT_meanshift -f meanshift -i <inimg_file> -p <channel>"<<endl;
    cout<<"inimg_file       The input image"<<endl;
    cout<<"channel          Data channel for tracing. Start from 1 (default 1)."<<endl;
    cout<<"outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n"<<endl;
    return;
}


