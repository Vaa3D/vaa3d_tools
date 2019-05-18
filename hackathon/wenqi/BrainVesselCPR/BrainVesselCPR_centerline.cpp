#include "BrainVesselCPR_centerline.h"

#define INF 1E9

using namespace std;



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

NeuronTree construcSwc(vector<Coor3D> path_point)
{
    QList<NeuronSWC> pathPointList;
    pathPointList.clear();
    NeuronSWC S;
//    for (int i = 0; i < path_length; i++) {
//        S.n = i;
//        S.type = 0;
//        S.x = path_point[i] % x_length;
//        S.y = floor(path_point[i] % (x_length * y_length)) / x_length;
//        S.z = floor(path_point[i] / (x_length * y_length));
//        S.r = 1;
//        S.pn = i-1;
//        pathPointList.append(S);
//    }
    Coor3D tmpCoor3D;
    int i = 0;
    while(!path_point.empty())
    {
        tmpCoor3D = path_point.back();
        path_point.pop_back();
        S.n = i;
        S.type = 0;
        S.x =tmpCoor3D.x;
        S.y = tmpCoor3D.y;
        S.z = tmpCoor3D.z;
        S.r = 1;
        S.pn = i-1;
        i++;
        pathPointList.append(S);
    }
    NeuronTree tree;
    tree.listNeuron = pathPointList;
    //writeSWC_file(filename.toStdString().c_str(), tree);

    return tree;

}

//smooth curve. just the average of #winsize neighbour coordinates.
bool smooth_curve(std::vector<Coor3D> & mCoord, int winsize)
{
    //std::cout<<" smooth_curve ";
    if (winsize<2) return true;

    std::vector<Coor3D> mC = mCoord; // a copy
    V3DLONG N = mCoord.size();
    int halfwin = winsize/2;

    for (int i=1; i<N-1; i++) // don't move start & end point
    {
        std::vector<Coor3D> winC;
        std::vector<double> winW;
        winC.clear();
        winW.clear();

        winC.push_back( mC[i] );
        winW.push_back( 1.+halfwin );
        for (int j=1; j<=halfwin; j++)
        {
            int k1 = i+j;	if(k1<0) k1=0;	if(k1>N-1) k1=N-1;
            int k2 = i-j;	if(k2<0) k2=0;	if(k2>N-1) k2=N-1;
            winC.push_back( mC[k1] );
            winC.push_back( mC[k2] );
            winW.push_back( 1.+halfwin-j );
            winW.push_back( 1.+halfwin-j );
        }
        //std::cout<<"winC.size = "<<winC.size()<<"\n";

        double s, x,y,z;
        s = x = y = z = 0;
                for (int i2=0; i2<winC.size(); i2++)
        {
                        x += winW[i2]* winC[i2].x;
                        y += winW[i2]* winC[i2].y;
                        z += winW[i2]* winC[i2].z;
                        s += winW[i2];
        }
        if (s)
        {
            x /= s;
            y /= s;
            z /= s;
        }

        mCoord[i].x = x; // output
        mCoord[i].y = y; // output
        mCoord[i].z = z; // output
    }
    return true;
}

void findPath(V3DLONG start, V3DLONG goal, unsigned short int * image1d, int x_length, int y_length, int z_length, V3DPluginCallback2 &callback, QWidget *parent)
{

    V3DLONG total_pxls = x_length * y_length * z_length;
    V3DLONG * path = new V3DLONG[total_pxls];
    double * cost_so_far = new double[total_pxls];
    bool * isVisited = new bool[total_pxls];

    memset(path, -1, total_pxls);
    memset(cost_so_far,INF,total_pxls);
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
               cost_so_far[next] = INF;
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
    vector<Coor3D> smooth_path;
    Coor3D tmpCoor3D;
    while(tmp != start)
    {
        //cout<< tmp << "->";
        tmp = path[tmp];
        //path_point[point_count] = tmp;
        tmpCoor3D.x = tmp % x_length;
        tmpCoor3D.y = floor(tmp % (x_length * y_length)) / x_length;
        tmpCoor3D.z = floor(tmp / (x_length * y_length));
        smooth_path.push_back(tmpCoor3D);
        point_count++;

        cout << path_point[point_count];
        cout << "x: " << smooth_path.back().x << "y: " << smooth_path.back().y << "z: " << smooth_path.back().z << endl;
    }
    cout << tmp << endl;
    tmpCoor3D.x = tmp % x_length;
    tmpCoor3D.y = floor(tmp % (x_length * y_length)) / x_length;
    tmpCoor3D.z = floor(tmp / (x_length * y_length));
    smooth_path.push_back(tmpCoor3D);

    cout << "path size: " << smooth_path.size() << endl;
    smooth_curve(smooth_path, 15);
//    QString filename("/Users/walker/MyProject/test.swc");
    cout << "path size (after smooth): " << smooth_path.size() << endl;
    //display trace in 3d


    NeuronTree tree = construcSwc(smooth_path);
    //cout << "smooth tree size:" << tree.listNeuron.size() << endl;

    v3dhandle curwin = callback.currentImageWindow();
    callback.open3DWindow(curwin);
    bool test = callback.setSWC(curwin, tree);
    cout << "set swc: " << test <<endl;
    callback.updateImageWindow(curwin);
    callback.pushObjectIn3DWindow(curwin);

    //writeSWC_file("/Users/walker/MyProject/test.swc", tree);

}
