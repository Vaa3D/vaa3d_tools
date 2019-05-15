/* BrainVesselCPR_plugin.cpp
 * This is a plugin for Brain Vessel CPR in MRA&MRI image
 * 2019-5-14 : by Wenqi Huang
 */

#include <QtGui>
#include <queue>
#include <iostream>
#include "v3d_message.h"
#include <vector>
#include "BrainVesselCPR_plugin.h"
//#include "BrainVesselCPR_func.h"
using namespace std;


struct Node
{
    V3DLONG node_id;
    double priority;
    Node(V3DLONG id, double p): node_id(id), priority(p)
    {
    }
};

bool operator > (const Node &n1, const Node &n2)
{
    return n1.priority > n2.priority;
}

double edgeCost(int a_intensity, int b_intensity)
{
    return 4095-(a_intensity+b_intensity)/2.0;
}

double heuristic(V3DLONG next, V3DLONG goal, int x_length)
{
    return 0.5*(floor(labs(next-goal)/x_length)+labs(next%x_length-goal%x_length));
}


void findPath(/*V3DLONG start, V3DLONG goal/*, unsigned short int * image1d, V3DLONG x_length, V3DLONG y_length, V3DLONG z_length,*/ V3DPluginCallback2 &callback, QWidget *parent)
{
    V3DLONG start = 3;
    V3DLONG goal = 19;
    V3DLONG x_length = 5;
    V3DLONG y_length = 4;
    V3DLONG z_length = 1;

    cout << "here!1";
    unsigned short int * image1d;
    memset(image1d, 0, 20);
    int tmp1[20] = {1,2,3,2,1,  1,4,2,1,0,  3,2,4,2,0,  1,2,2,4,2};
    for(int i=0;i<20;i++)
    {
        image1d[i] = tmp1[i];
    }




    V3DLONG total_pxls = x_length * y_length * z_length;
    V3DLONG * path;
    double * cost_so_far;
    bool * isVisited;

    memset(path, -1, total_pxls);
    memset(cost_so_far,9999999,total_pxls);
    memset(isVisited, false, total_pxls);

    cout<< "here2!";
    priority_queue<Node, vector<Node>, greater<Node> > frontier;
    frontier.push(Node(start,0));
    path[start] = -1;
    cost_so_far[start] = 0;
    isVisited[start] = true;

    cout << "start!" << endl;

    while(!frontier.empty())
    {
       Node current = frontier.top();
       frontier.pop();
       if(current.node_id == goal)
           break;

       V3DLONG neighbour[4]={current.node_id-x_length,current.node_id+x_length,current.node_id-1,current.node_id+1};
       if(current.node_id<x_length)
       {
           neighbour[0] = -2;
       }

       if(x_length*y_length - current.node_id <= x_length)
       {
           neighbour[1] = -2;
       }
       if(current.node_id % x_length == 0)
       {
           neighbour[2] = -2;
       }
       if(current.node_id % x_length == x_length-1)
       {
           neighbour[3] = -2;
       }

       cout<<"curr: "<<current.node_id<<" nei: "<<neighbour[0]<<" "<<neighbour[1]<<" "<<neighbour[2]<<" "<<neighbour[3]<<" \n";


       for(int i=0;i<4;i++)
       {
           V3DLONG next = neighbour[i];
           if(next==-2)
               continue;
           //cout<<next<<endl;
           double new_cost = cost_so_far[current.node_id] + edgeCost(image1d[next], image1d[current.node_id]);
           if(!isVisited[next])
               cost_so_far[next] = 999999;
           if(new_cost < cost_so_far[next])
           {
               isVisited[next] = true;
               cost_so_far[next] = new_cost;
               double priority = new_cost + heuristic(goal,next,x_length);
               frontier.push(Node(next,priority));
               path[next] = current.node_id;
           }

       }
    }

    for(int i=0;i<20;i++)
       cout<<path[i]<<endl;

    V3DLONG tmp = goal;
    while(tmp != start)
    {
       cout<< tmp << "->";
       tmp = path[tmp];
    }
}


void sayhello(V3DPluginCallback2 &callback, QWidget *parent){
    cout<<"hello";
}


void testfunc(V3DPluginCallback2 &callback, QWidget *parent){


        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* p4DImage = callback.getImage(curwin);
        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        // TODO： uint16 image data
        unsigned short int * data1d = (unsigned short int *) p4DImage->getRawData();
        V3DLONG totalpxls = p4DImage->getTotalBytes();
        V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

        V3DLONG image_width = p4DImage->getXDim();
        V3DLONG image_height = p4DImage->getYDim();
        V3DLONG image_slice_num = p4DImage->getZDim();
        V3DLONG channels = p4DImage->getCDim();

        V3DLONG UnitBytes  = p4DImage->getUnitBytes();

        cout << "total bytes: " << totalpxls << endl << "width: " << image_width << endl \
             << "height: " << image_height << endl << "slice num: " << image_slice_num \
             << endl << "channel: " << channels << endl << "unit bytes: " << UnitBytes << endl;

        unsigned short int * map;
        memset(map, 0, 20);
        int tmp[20] = {1,2,3,2,1,  1,4,2,1,0,  3,2,4,2,0,  1,2,2,4,2};
        for(int i=0;i<20;i++)
        {
            map[i] = tmp[i];
        }

        cout << "begin find path!" << endl;
        sayhello(callback, parent);
        findPath(/*start, goal/*,map,x_length, y_length, z_length, */callback, parent);
        cout << "find path finished!" << endl;

}

Q_EXPORT_PLUGIN2(BrainVesselCPR, BrainVesselCPRPlugin);
 
QStringList BrainVesselCPRPlugin::menulist() const
{
	return QStringList() 
		<<tr("Start CPR")
        <<tr("test")
		<<tr("about");
}

// menu bar settings
// QStringList BrainVesselCPRPlugin::funclist() const
// {
// 	return QStringList()
// 		<<tr("func1")
// 		<<tr("func2")
// 		<<tr("help");
// }

void BrainVesselCPRPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Start CPR"))
	{
		//v3d_msg("To be implemented.");
		startCPR(callback,parent);
	}
    if(menu_name == tr("test"))
    {
        testfunc(callback,parent);
    }
	else
	{
		v3d_msg(tr("This is a plugin for Brain Vessel CPR in MRA&MRI image. "
			"Developed by Wenqi Huang, 2019-5-14"));
	}
}

void startCPR(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();
    v3dhandle curwin = callback.currentImageWindow();
    if(win_list.size()<1)
    {
        v3d_msg("No MRA Image Opened!");
        return;
    }

  //  callback.open3DWindow(curwin);
    LandmarkList landmark_list = callback.getLandmark(curwin);
    while(landmark_list.size()!=2)
    {
        v3d_msg("Please select TWO landmarks as start point and end point.");
        landmark_list = callback.getLandmark(curwin);
        //TODO: exception
    }
    //marker coord start from 1 instead of 0
    for(long i=0;i<2;i++)
    {
        landmark_list[i].x-=1;
        landmark_list[i].y-=1;
        landmark_list[i].z-=1;
    }
    v3d_msg(QObject::tr("Start point: (%1, %2, %3)\nEnd point: (%4, %5, %6)").\
            arg(landmark_list[0].x).arg(landmark_list[0].y).arg(landmark_list[0].z).arg(landmark_list[1].x).arg(landmark_list[1].y).arg(landmark_list[1].z));

}



// menu bar funtions
// bool BrainVesselCPRPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
// {
// 	vector<char*> infiles, inparas, outfiles;
// 	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
// 	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
// 	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

// 	if (func_name == tr("func1"))
// 	{
// 		v3d_msg("To be implemented.");
// 	}
// 	else if (func_name == tr("func2"))
// 	{
// 		v3d_msg("To be implemented.");
// 	}
// 	else if (func_name == tr("help"))
// 	{
// 		v3d_msg("To be implemented.");
// 	}
// 	else return false;

// 	return true;
// }

