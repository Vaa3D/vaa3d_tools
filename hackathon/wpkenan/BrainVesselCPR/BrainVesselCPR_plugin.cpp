/* BrainVesselCPR_plugin.cpp
 * This is a plugin for Brain Vessel CPR in MRA&MRI image
 * 2019-5-14 : by Wenqi Huang
 */

#include <QtGui>
#include <queue>
#include <iostream>
#include <string>
#include "v3d_message.h"
#include <vector>
#include "BrainVesselCPR_plugin.h"
#include "basic_surf_objs.h"
#include <stdlib.h>

#include "BrainVesselCPR_gui.h"


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

double heuristic(V3DLONG next, V3DLONG goal, int x_length, int y_length)
{
    int next_x = next % x_length;
    int next_y = floor((next % (x_length * y_length)) / x_length);
    int next_z = floor(next / (x_length * y_length));

    int goal_x = next % x_length;
    int goal_y = floor((next % (x_length * y_length)) / x_length);
    int goal_z = floor(next / (x_length * y_length));

    return 0.2*(abs(next_x - goal_x) + abs(next_y - goal_y) + abs(next_z - goal_z));
}

NeuronTree construcSwc(V3DLONG * path_point, V3DLONG path_length, int x_length, int y_length, int z_length/*, QString filename*/)
{
    QList<NeuronSWC> pathPointList;
    pathPointList.clear();
    NeuronSWC S;
    for (int i = 0; i < path_length; i++) {
        S.n = i;
        S.type = 0;
        S.x = path_point[i] % x_length;
        S.y = floor(path_point[i] % (x_length * y_length)) / x_length;
        S.z = floor(path_point[i] / (x_length * y_length));
        S.r = 1;
        S.pn = i-1;
        pathPointList.append(S);
    }
    NeuronTree tree;
    tree.listNeuron = pathPointList;
    //writeSWC_file(filename.toStdString().c_str(), tree);

    return tree;

}


void findPath(V3DLONG start, V3DLONG goal, unsigned short int * image1d, int x_length, int y_length, int z_length, V3DPluginCallback2 &callback, QWidget *parent)
{

    V3DLONG total_pxls = x_length * y_length * z_length;
    V3DLONG * path = new V3DLONG[total_pxls];
    double * cost_so_far = new double[total_pxls];
    bool * isVisited = new bool[total_pxls];

    memset(path, -1, total_pxls);
    memset(cost_so_far,9999999,total_pxls);
    memset(isVisited, false, total_pxls);

    priority_queue<Node, vector<Node>, greater<Node> > frontier;
    frontier.push(Node(start,0));
    path[start] = -1;
    cost_so_far[start] = 0;
    isVisited[start] = true;

    //cout << "start!" << endl;

    while(!frontier.empty())
    {
       Node current = frontier.top();
       frontier.pop();
       if(current.node_id == goal)
           break;

       // neighbours: up, down, left, right, front, back.
       V3DLONG neighbour[6]={current.node_id - x_length         , current.node_id + x_length, \
                             current.node_id - 1                , current.node_id + 1,        \
                             current.node_id - x_length*y_length, current.node_id + x_length*y_length};

       //check if current point is on the surface.
       if((current.node_id % (x_length * y_length)) < x_length)                         //up surface
           neighbour[0] = -2;
       if(x_length * y_length - (current.node_id % (x_length * y_length)) <= x_length)  //down surface
           neighbour[1] = -2;
       if((current.node_id % (x_length * y_length)) % x_length == 0)                      //left surface
           neighbour[2] = -2;
       if((current.node_id % (x_length * y_length)) % x_length == x_length-1)             //right surface
           neighbour[3] = -2;
       if(current.node_id < x_length * y_length)                                        //front surface
           neighbour[4] = -2;
       if(total_pxls - current.node_id <= x_length * y_length)                          //back surface
           neighbour[5] = -2;


       for(int i=0;i<6;i++)
       {
           V3DLONG next = neighbour[i];
           if(next==-2)
               continue;

           double new_cost = cost_so_far[current.node_id] + edgeCost(image1d[next], image1d[current.node_id]);
           if(!isVisited[next])
               cost_so_far[next] = 999999;
           if(new_cost < cost_so_far[next])
           {
               isVisited[next] = true;
               cost_so_far[next] = new_cost;
               double priority = new_cost + heuristic(goal, next, x_length, y_length);
               frontier.push(Node(next, priority));
               path[next] = current.node_id;
           }
       }
    }


    //output path in console.
    V3DLONG tmp = goal;
    V3DLONG * path_point = new V3DLONG[x_length * y_length * int(floor(z_length/10))];
    V3DLONG point_count = 0;
    while(tmp != start)
    {
        cout<< tmp << "->";
        tmp = path[tmp];
        path_point[point_count] = tmp;
        point_count++;
    }
    cout << tmp << endl;
//    QString filename("/Users/walker/MyProject/test.swc");

    //display trace in 3d
    NeuronTree tree = construcSwc(path_point, point_count, x_length, y_length, z_length);
    v3dhandle curwin = callback.currentImageWindow();
    callback.open3DWindow(curwin);
    callback.setSWC(curwin, tree);
    callback.updateImageWindow(curwin);
    callback.pushObjectIn3DWindow(curwin);

}



void setWLWW(V3DPluginCallback2 &callback, QWidget *parent)
{
    SetContrastWidget * setWLWW_widget = new SetContrastWidget(callback, parent);
    setWLWW_widget->show();

//    v3dhandle curwin = callback.currentImageWindow();
//    Image4DSimple* p4DImage = callback.getImage(curwin);
//    if (!p4DImage)
//    {
//        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
//        return;
//    }
//    unsigned short int * data1d = (unsigned short int *) p4DImage->getRawData();
//    V3DLONG totalpxls = p4DImage->getTotalBytes();
//    for(int i=0; i<totalpxls; i++)
//    {

//    }

}




void testfunc(V3DPluginCallback2 &callback, QWidget *parent)
{


        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* p4DImage = callback.getImage(curwin);
        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        unsigned short int * data1d = (unsigned short int *) p4DImage->getRawData();
        V3DLONG totalpxls = p4DImage->getTotalBytes();
        V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

        V3DLONG x_length = p4DImage->getXDim();
        V3DLONG y_length = p4DImage->getYDim();
        V3DLONG z_length = p4DImage->getZDim();
        V3DLONG channels = p4DImage->getCDim();

        V3DLONG UnitBytes  = p4DImage->getUnitBytes();

        cout << "total bytes: " << totalpxls << endl << "width: " << x_length << endl \
             << "height: " << y_length << endl << "slice num: " << z_length \
             << endl << "channel: " << channels << endl << "unit bytes: " << UnitBytes << endl;

        unsigned short int * map = new unsigned short int[20];
        memset(map, 0, 20);
        int tmp[20] = {1,2,3,2,1,  1,4,2,1,0,  3,2,4,2,0,  1,2,2,4,2};
        for(int i=0;i<20;i++)
        {
            map[i] = tmp[i];
        }

        V3DLONG start;
        V3DLONG goal;


        cout << "begin find path!" << endl;

        //test start (113, 123,138), id: , goal (138, 198, 76), id:
        start = x_length * y_length * 138 + 123 * x_length + 113;
        goal = x_length * y_length * 76 + 198 * x_length + 138;
        findPath(start, goal, data1d, x_length, y_length, z_length, callback, parent);
        cout << "find path finished!" << endl;


        LandmarkList curlist;
        LocationSimple s;
        s.x= 113;
        s.y= 123;
        s.z= 138;
        s.name=std::string("point 1");
        s.radius=3;
        curlist << s;
        s.x= 138;
        s.y= 198;
        s.z= 76;
        s.name=std::string("point 2");
        s.radius=3;
        curlist << s;

        callback.setLandmark(curwin,curlist);
        callback.updateImageWindow(curwin);
        callback.pushObjectIn3DWindow(curwin);

}

Q_EXPORT_PLUGIN2(BrainVesselCPR, BrainVesselCPRPlugin);
 
QStringList BrainVesselCPRPlugin::menulist() const
{
	return QStringList() 
		<<tr("Start CPR")
        <<tr("Set MRI WL/WW")
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
    else if(menu_name == tr("Set MRI WL/WW"))
    {
        setWLWW(callback,parent);
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

    //get 2 landmarks as start and end point
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
            arg(landmark_list[0].x).arg(landmark_list[0].y).arg(landmark_list[0].z).\
            arg(landmark_list[1].x).arg(landmark_list[1].y).arg(landmark_list[1].z));


    // get 3d image info & 1d data vector
    Image4DSimple* p4DImage = callback.getImage(curwin);
    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned short int * data1d = (unsigned short int *) p4DImage->getRawData();
    V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
    V3DLONG x_length = p4DImage->getXDim();
    V3DLONG y_length = p4DImage->getYDim();
    V3DLONG z_length = p4DImage->getZDim();
    V3DLONG channels = p4DImage->getCDim();
    V3DLONG UnitBytes  = p4DImage->getUnitBytes();

    cout << "total bytes: " << totalpxls << endl << "width: " << x_length << endl \
         << "height: " << y_length << endl << "slice num: " << z_length \
         << endl << "channel: " << channels << endl << "unit bytes: " << UnitBytes << endl;

    //convert landmark to 1d data index
    V3DLONG start;
    V3DLONG goal;
//    test start (113, 123,138), id: , goal (138, 198, 76), id:
//    start = x_length * y_length * 138 + 123 * x_length + 113;
//    goal = x_length * y_length * 76 + 198 * x_length + 138;
    start = V3DLONG(landmark_list[0].x) + V3DLONG(landmark_list[0].y) * x_length + V3DLONG(landmark_list[0].z) * x_length * y_length;
    goal = V3DLONG(landmark_list[1].x) + V3DLONG(landmark_list[1].y) * x_length + V3DLONG(landmark_list[1].z) * x_length * y_length;

    //find path begins!
    cout << "begin find path!" << endl;
    findPath(start, goal, data1d, x_length, y_length, z_length, callback, parent);
    cout << "find path finished!" << endl;



    //sync 3d view of MRA and MRI



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

