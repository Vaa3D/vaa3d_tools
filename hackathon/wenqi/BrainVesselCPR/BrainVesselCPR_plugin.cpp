/* BrainVesselCPR_plugin.cpp
 * This is a plugin for Brain Vessel CPR in MRA&MRI image
 * 2019-5-14 : by Wenqi Huang
 */

#include "BrainVesselCPR_plugin.h"
#include "BrainVesselCPR_filter.h"

#define INF 1E9

using namespace std;

struct Node
{
    V3DLONG node_id;
    double priority;
    Node(V3DLONG id, double p): node_id(id), priority(p)
    {
    }
};

struct Coor3D
{
    double x;
    double y;
    double z;
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
template <class T> //should be a struct included members of (x,y,z), like Coord3D
bool smooth_curve(std::vector<T> & mCoord, int winsize)
{
    //std::cout<<" smooth_curve ";
    if (winsize<2) return true;

    std::vector<T> mC = mCoord; // a copy
    V3DLONG N = mCoord.size();
    int halfwin = winsize/2;

    for (int i=1; i<N-1; i++) // don't move start & end point
    {
        std::vector<T> winC;
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



void setWLWW(V3DPluginCallback2 &callback, QWidget *parent)
{
    SetContrastWidget * setWLWW_widget = new SetContrastWidget(callback, parent);
    setWLWW_widget->show();
}






// sync3d
static lookPanel *panel = 0;
static controlPanel *ControlPanel=0;
QString warning_msg = "Oops... The image you selected no longer exists... The file list has been refreshed now and you can try it again.";


void finishSyncPanel()
{
    if (panel)
    {
        delete panel;
        panel=0;
    }
}

void finishSyncControlPanel()
{
    if (ControlPanel)
    {
        delete ControlPanel;
        ControlPanel=0;
    }
}

void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent)
{
    v3dhandleList win_list = v3d.getImageWindowList();
    if (win_list.size()<2)
    {
        v3d_msg("You need at least two opened images to synchronize their 3D views!");
        return;
    }

    if (panel)
    {
        panel->show();
        return;
    }
    else
    {
        panel = new lookPanel(v3d, parent);
        if (panel)
        {
            panel->show();
            panel->raise();
            panel->move(100,100);
            panel->activateWindow();
        }
    }
}


void lookPanel::reject()
{
    finishSyncPanel();
}

lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    win_list = m_v3d.getImageWindowList();
    QStringList items;
    for (int i=0; i<win_list.size(); i++)
        items << m_v3d.getImageName(win_list[i]);

    combo_master = new QComboBox(); combo_master->addItems(items);
    combo_slave = new QComboBox(); combo_slave->addItems(items);

    label_master = new QLabel(QObject::tr("Master-window: "));
    label_slave = new QLabel(QObject::tr("Slave-window: "));
    check_rotation = new QCheckBox(); check_rotation->setText(QObject::tr("Rotation "));check_rotation->setChecked(true);
    check_shift = new QCheckBox(); check_shift->setText(QObject::tr("Shift"));check_shift->setChecked(true);
    check_zoom = new QCheckBox(); check_zoom->setText(QObject::tr("Zoom"));check_zoom->setChecked(true);
    QPushButton* ok     = new QPushButton("Sync (one shot)");
    QPushButton* cancel = new QPushButton("Close");
    syncAuto     = new QPushButton("Start Sync (real time)");

    b_autoON = false;

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_master, 1,0,1,6);
    gridLayout->addWidget(combo_master,1,1,1,6);
    gridLayout->addWidget(label_slave, 2,0,1,6);
    gridLayout->addWidget(combo_slave,2,1,1,6);
    gridLayout->addWidget(check_rotation, 4,0,1,1);
    gridLayout->addWidget(check_shift,4,1,1,1);
    gridLayout->addWidget(check_zoom, 4,2,1,1);
    gridLayout->addWidget(ok, 5,0);
    gridLayout->addWidget(cancel,5,6);
    gridLayout->addWidget(syncAuto,5,1);
    setLayout(gridLayout);
    setWindowTitle(QString("Synchronize two 3D views"));

    connect(ok,     SIGNAL(clicked()), this, SLOT(_slot_sync_onetime()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(syncAuto, SIGNAL(clicked()), this, SLOT(_slot_syncAuto()));
    connect(check_rotation, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(check_shift, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(check_zoom, SIGNAL(stateChanged(int)), this, SLOT(update()));

    //should we check the window close event? // by PHC

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
    win_list_past = win_list;

}

lookPanel::~lookPanel()
{
    if (m_pTimer) {delete m_pTimer; m_pTimer=0;}
}


void lookPanel::_slot_sync_onetime()
{
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i1 = combo_master->currentIndex();
    int i2 = combo_slave->currentIndex();

    if (i1==i2)
    {
        v3d_msg("You have selected the same image. You need to specify and sync two different images. Try again!");
        return;
    }

    if (i1 <  win_list.size() &&
            i2 < win_list.size() &&
            i1 < win_list_past.size() &&
            i2  < win_list_past.size())
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString current2 = m_v3d.getImageName(win_list[i2]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);
        QString past2 = m_v3d.getImageName(win_list_past[i2]);
        if (current1==past1  && current2==past2)
        {
            if (win_list[i1] && win_list[i2])//ensure the 3d viewer window is open; if not, then open it
            {
                m_v3d.open3DWindow(win_list[i1]);
                View3DControl *view_master = m_v3d.getView3DControl(win_list[i1]);
                m_v3d.open3DWindow(win_list[i2]);
                View3DControl *view_slave = m_v3d.getView3DControl(win_list[i2]);
                if (view_master && view_slave)
                {
                    view_master->absoluteRotPose();
                    int xRot = view_master->xRot();
                    int yRot = view_master->yRot();
                    int zRot = view_master->zRot();

                    int xShift = view_master->xShift();
                    int yShift = view_master->yShift();
                    int zShift = view_master->zShift();

                    int zoom = view_master->zoom();

                    if (check_rotation->isChecked())
                    {
                        view_slave->resetRotation();
                        view_slave->doAbsoluteRot(xRot,yRot,zRot);
                    }
                    if (check_shift->isChecked())
                    {
                        view_slave->setXShift(xShift);
                        view_slave->setYShift(yShift);
                        view_slave->setZShift(zShift);
                    }
                    if (check_zoom->isChecked())
                        view_slave->setZoom(zoom);
                }
            }
        }
        else
        {
            v3d_msg(warning_msg);
            QStringList items;
            for (int i=0; i<win_list.size(); i++)
                items << m_v3d.getImageName(win_list[i]);
            combo_master->clear(); combo_master->addItems(items);
            combo_slave->clear(); combo_slave->addItems(items);
            win_list_past = win_list;
            return;

        }
    }
    else
    {
        v3d_msg(warning_msg);
        QStringList items;
        for (int i=0; i<win_list.size(); i++)
            items << m_v3d.getImageName(win_list[i]);
        combo_master->clear(); combo_master->addItems(items);
        combo_slave->clear(); combo_slave->addItems(items);
        win_list_past = win_list;
        return;
    }
    return;
}

void lookPanel::resetSyncAutoState()
{
    if (m_pTimer)
        m_pTimer->stop();
    b_autoON = false;
    if (syncAuto)
        syncAuto->setText("Start Sync (real time)");
    if (combo_master) combo_master->setEnabled(true);
    if (combo_slave) combo_slave->setEnabled(true);
}

void lookPanel::_slot_syncAuto()
{
    v3dhandleList win_list = m_v3d.getImageWindowList();

    bool b_filelistchanged = (win_list.size()==win_list_past.size()) ? false : true;
    if (!b_filelistchanged)
    {
        for (int i=0; i<win_list.size(); i++)
        {
            if (win_list.at(i) != win_list_past.at(i))
            {
                b_filelistchanged = true;
                break;
            }
        }
    }
    if (b_filelistchanged)
    {
        v3d_msg(warning_msg);
        QStringList items;
        for (int i=0; i<win_list.size(); i++)
            items << m_v3d.getImageName(win_list[i]);
        combo_master->clear(); combo_master->addItems(items);
        combo_slave->clear(); combo_slave->addItems(items);
        win_list_past = win_list;

        return;
    }

    //only execute the sync if the file list has not changed

    int i_master = combo_master->currentIndex();
    int i_slave = combo_slave->currentIndex();
    if (i_master==i_slave)
    {
        v3d_msg("You have selected the same image. You need to specify and sync two different images. Try again!");
        resetSyncAutoState();

        QStringList items;
        for (int i=0; i<win_list.size(); i++)
            items << m_v3d.getImageName(win_list[i]);
        if (combo_master)
        {
            combo_master->clear(); combo_master->addItems(items);
            combo_master->setEnabled(true);
        }

        if (combo_slave)
        {
            combo_slave->clear(); combo_slave->addItems(items);
            combo_slave->setEnabled(true);
        }

        return;
    }

    b_autoON = !b_autoON; // a simple bi-state switch

    if (b_autoON)
    {
        syncAuto->setText("Stop Sync (real time)");
        xRot_past = -1;
        yRot_past = -1;
        zRot_past = -1;
        xShift_past = -1;
        yShift_past = -1;
        zShift_past = -1;
        zoom_past = -1;

        if (!(view_master = m_v3d.getView3DControl(win_list[i_master])))
        {
            m_v3d.open3DWindow(win_list[i_master]);
            view_master = m_v3d.getView3DControl(win_list[i_master]);
        }

        if (!(view_slave = m_v3d.getView3DControl(win_list[i_slave])))
        {
            m_v3d.open3DWindow(win_list[i_slave]);
            view_slave = m_v3d.getView3DControl(win_list[i_slave]);
        }

        combo_master->setEnabled( false );
        combo_slave->setEnabled( false );

        long interval = 0.2 * 1000;
        m_pTimer->start(interval);
    }
    else
    {
        resetSyncAutoState();
        return;
    }
    return;
}

void lookPanel::_slot_timerupdate()
{
    int i_master = combo_master->currentIndex();
    int i_slave = combo_slave->currentIndex();
    if (i_master==i_slave)
    {
        v3d_msg("Somehow you have specified the same image for both master and slave views. Do nothing!", 0);
        resetSyncAutoState();
        return;
    }

    if (!(view_master = m_v3d.getView3DControl(win_list[i_master])))
    {
        m_v3d.open3DWindow(win_list[i_master]);
        view_master = m_v3d.getView3DControl(win_list[i_master]);
    }

    if (!(view_slave = m_v3d.getView3DControl(win_list[i_slave])))
    {
        m_v3d.open3DWindow(win_list[i_slave]);
        view_slave = m_v3d.getView3DControl(win_list[i_slave]);
    }

    if (view_master && view_slave)
    {
        if (check_rotation->isChecked())
        {
            view_master->absoluteRotPose();
            int xRot = view_master->xRot();
            int yRot = view_master->yRot();
            int zRot = view_master->zRot();

            if (xRot!=xRot_past || yRot!=yRot_past || zRot!=zRot_past)
            {
                view_slave->resetRotation();
                view_slave->doAbsoluteRot(xRot,yRot,zRot);
                xRot_past = xRot;
                yRot_past = yRot;
                zRot_past = zRot;
            }
        }

        if (check_shift->isChecked())
        {
            int xShift = view_master->xShift();
            int yShift = view_master->yShift();
            int zShift = view_master->zShift();

            if (xShift!=xShift_past || yShift!=yShift_past || zShift!=zShift_past)
            {
                view_slave->setXShift(xShift);
                view_slave->setYShift(yShift);
                view_slave->setZShift(zShift);
                xShift_past = xShift;
                yShift_past = yShift;
                zShift_past = zShift;
            }
        }

        if (check_zoom->isChecked())
        {
            int zoom = view_master->zoom();
            if (zoom!=zoom_past)
            {
                view_slave->setZoom(zoom);
                zoom_past = zoom;
            }
        }
    }
}

void MyComboBox::enterEvent(QEvent *e)
{
    updateList();
    QComboBox::enterEvent(e);
}

void MyComboBox::updateList()
{
    if (!m_v3d)
        return;

    QString lastDisplayfile = currentText();

    QList <V3dR_MainWindow *> cur_list_3dviewer = m_v3d->getListAll3DViewers();

    QStringList items;
    int i;

    for (i=0; i<cur_list_3dviewer.count(); i++)
    {
         items << m_v3d->getImageName(cur_list_3dviewer[i]);
    }

    //update the list now
    clear();
    addItems(items);

    int curDisplayIndex=-1; //-1 for invalid index
    for (i=0; i<items.size(); i++)
        if (items[i]==lastDisplayfile)
        {
            curDisplayIndex = i;
            break;
        }

    if (curDisplayIndex>=0)
        setCurrentIndex(curDisplayIndex);

    //
    update();

    return;
}


controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    //potential bugs for the following two sentences
    list_triview = m_v3d.getImageWindowList();
    list_3dviewer = m_v3d.getListAll3DViewers();

    combo_surface = new MyComboBox(&m_v3d);
    combo_surface->updateList();

    label_surface = new QLabel(QObject::tr("Master-window: "));
    check_rotation = new QCheckBox(); check_rotation->setText(QObject::tr("Rotation "));check_rotation->setChecked(true);
    check_shift = new QCheckBox(); check_shift->setText(QObject::tr("Shift"));check_shift->setChecked(true);
    check_zoom = new QCheckBox(); check_zoom->setText(QObject::tr("Zoom"));check_zoom->setChecked(true);
    QPushButton* ok     = new QPushButton("Sync (one shot)");
    QPushButton* cancel = new QPushButton("Close");
    syncAuto     = new QPushButton("Start Sync (real time)");

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_surface, 1,0,1,5);
    gridLayout->addWidget(combo_surface, 2,0,1,5);
    gridLayout->addWidget(check_rotation, 3,0,1,1);
    gridLayout->addWidget(check_shift,3,1,1,1);
    gridLayout->addWidget(check_zoom, 3,2,1,1);
    gridLayout->addWidget(ok, 4,0);
    gridLayout->addWidget(cancel,4,6);
    gridLayout->addWidget(syncAuto,4,1);
    setLayout(gridLayout);
    setWindowTitle(QString("Synchronize multiple 3D views"));


    connect(ok,     SIGNAL(clicked()), this, SLOT(_slot_sync_onetime()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(syncAuto, SIGNAL(clicked()), this, SLOT(_slot_syncAuto()));
    connect(check_rotation, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(check_shift, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(check_zoom, SIGNAL(stateChanged(int)), this, SLOT(update()));

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
}

controlPanel::~controlPanel()
{
     if(panel){delete panel; panel=0;}

}

void controlPanel::reject()
{
    finishSyncControlPanel();

}

void controlPanel::_slot_sync_onetime()
{
    if(combo_surface->currentIndex()<0) return;
    list_3dviewer = m_v3d.getListAll3DViewers();
    curwin = list_triview[combo_surface->currentIndex()];
    QString curname = combo_surface->itemText(combo_surface->currentIndex());
    int xRot,yRot,zRot,xShift,yShift,zShift,zoom;

    view_master = m_v3d.getView3DControl_Any3DViewer(list_3dviewer[combo_surface->currentIndex()]);
    if(view_master)
    {
        view_master->absoluteRotPose();
        xRot = view_master->xRot();
        yRot = view_master->yRot();
        zRot = view_master->zRot();

        xShift = view_master->xShift();
        yShift = view_master->yShift();
        zShift = view_master->zShift();
        zoom = view_master->zoom();
    }

    for (int i=0; i<list_3dviewer.count(); i++)
    {
        if(curname != m_v3d.getImageName(list_3dviewer[i]))
        {
            V3dR_MainWindow * slave_win = list_3dviewer[i];
            if(slave_win)
            {
                view_slave = m_v3d.getView3DControl_Any3DViewer(slave_win);
                if(view_slave)
                {
                    if (check_rotation->isChecked())
                    {
                        view_slave->resetRotation();
                        view_slave->doAbsoluteRot(xRot,yRot,zRot);
                    }
                    if (check_shift->isChecked())
                    {
                        view_slave->setXShift(xShift);
                        view_slave->setYShift(yShift);
                        view_slave->setZShift(zShift);
                    }
                    if (check_zoom->isChecked())
                        view_slave->setZoom(zoom);
                }
            }
        }
    }
    return;
}

void controlPanel::resetSyncAutoState()
{
    if (m_pTimer)
        m_pTimer->stop();
    b_autoON = false;
    if (syncAuto)
        syncAuto->setText("Start Sync (real time)");
    if (combo_surface) combo_surface->setEnabled(true);
}

void controlPanel::_slot_syncAuto()
{

    if(combo_surface->currentIndex()<0) return;

    b_autoON = !b_autoON; // a simple bi-state switch

    if (b_autoON)
    {
        syncAuto->setText("Stop Sync (real time)");
        xRot_past = -1;
        yRot_past = -1;
        zRot_past = -1;
        xShift_past = -1;
        yShift_past = -1;
        zShift_past = -1;
        zoom_past = -1;

        view_master = m_v3d.getView3DControl_Any3DViewer(list_3dviewer[combo_surface->currentIndex()]);
        combo_surface->setEnabled( false );
        long interval = 0.2 * 1000;
        m_pTimer->start(interval);
    }
    else
    {
        resetSyncAutoState();
        return;
    }
    return;
}

void controlPanel::_slot_timerupdate()
{
    list_3dviewer = m_v3d.getListAll3DViewers();
    curwin = list_triview[combo_surface->currentIndex()];
    QString curname = combo_surface->itemText(combo_surface->currentIndex());
    int xRot,yRot,zRot,xShift,yShift,zShift,zoom;

    view_master = m_v3d.getView3DControl_Any3DViewer(list_3dviewer[combo_surface->currentIndex()]);
    if(view_master)
    {
        view_master->absoluteRotPose();

        xRot = view_master->xRot();
        yRot = view_master->yRot();
        zRot = view_master->zRot();

        xShift = view_master->xShift();
        yShift = view_master->yShift();
        zShift = view_master->zShift();

        zoom = view_master->zoom();
    }

    for (int i=0; i<list_3dviewer.count(); i++)
    {
        if(curname != m_v3d.getImageName(list_3dviewer[i]))
        {
            V3dR_MainWindow * slave_win = list_3dviewer[i];
            if(slave_win)
            {
                view_slave = m_v3d.getView3DControl_Any3DViewer(slave_win);
                if(view_slave)
                {
                    if (check_rotation->isChecked())
                    {

                        if (xRot!=xRot_past || yRot!=yRot_past || zRot!=zRot_past)
                        {
                            view_slave->resetRotation();
                            view_slave->doAbsoluteRot(xRot,yRot,zRot);
                        }
                    }
                    if (check_shift->isChecked())
                    {

                        if (xShift!=xShift_past || yShift!=yShift_past || zShift!=zShift_past)
                        {
                            view_slave->setXShift(xShift);
                            view_slave->setYShift(yShift);
                            view_slave->setZShift(zShift);
                        }
                    }
                    if (check_zoom->isChecked())
                    {
                        if (zoom!=zoom_past)
                        {
                            view_slave->setZoom(zoom);
                        }
                    }
                }
            }
        }
    }

    xRot_past = xRot;
    yRot_past = yRot;
    zRot_past = zRot;
    xShift_past = xShift;
    yShift_past = yShift;
    zShift_past = zShift;
    zoom_past = zoom;
}

// end sync3d



Q_EXPORT_PLUGIN2(BrainVesselCPR, BrainVesselCPRPlugin);
 
QStringList BrainVesselCPRPlugin::menulist() const
{
	return QStringList() 
		<<tr("Start CPR")
        <<tr("Set MRI WL/WW")
        <<tr("synchronize 3D viewers")
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
    else if (menu_name == tr("synchronize 3D viewers"))
    {
        SynTwoImage(callback, parent);
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

    float * data1d_gausssian;
    V3DLONG *in_sz;
    in_sz = new V3DLONG[4];
    in_sz[0] = x_length;
    in_sz[1] = y_length;
    in_sz[2] = z_length;
    in_sz[3] = 1;
    gaussian_filter(data1d, in_sz, 7, 7, 7, 1, 100, data1d_gausssian);
    unsigned short int * data1d_gaussian_uint16;
    for(int i = 0; i < x_length*y_length*z_length; i++)
    {
        data1d_gaussian_uint16[i] = (unsigned short int)data1d_gausssian[i];
    }
    findPath(start, goal, data1d_gaussian_uint16, x_length, y_length, z_length, callback, parent);
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

