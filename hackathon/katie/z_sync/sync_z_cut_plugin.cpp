/* sync_z_cut_plugin.cpp
 * This plugin syncs the z-cut of the image and the surface.
 * 2014-07-03 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "sync_z_cut_plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <QAbstractSlider>
#include <iostream>
#include <sstream>

using namespace std;

Q_EXPORT_PLUGIN2(sync_z_cut, SyncZ)

static lookPanel *panel = 0;

void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent);

void finishSyncPanel()
{
    if (panel)
    {
        delete panel;
        panel=0;
    }
}

//generic construction
QStringList SyncZ::menulist() const
{
    return QStringList()
        <<tr("sync z cut");
}

QString warning_msg = "Oops...that is not a valid z slice";

void SyncZ::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("sync z cut"))
    {
        SynTwoImage(callback, parent);
    }
    else
    {
        v3d_msg(tr("This plugin syncs the z-cut of the image and the surface.. "
            "Developed by Katie Lin, 2014-07-03"));
    }
}

//main functions
void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent)
{
    v3dhandleList win_list = v3d.getImageWindowList();
    if (win_list.size()<1)
    {
        v3d_msg("You need one opened image to get 3D view data!");
        return;
    }

//I added this if bracket
    if (win_list.size()>1)
    {
        v3d_msg("Cannot analyze multiple windows. Close all windows but the desired one and try again.");
        return;
    }

    if (panel)
    {
        panel->show();
        return;
    }
    else
    {
        panel = new lookPanel(v3d, parent); //this shouldn't happen.

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

QAbstractSlider *lookPanel::createCutPlaneSlider(int maxval, Qt::Orientation hv)
{
    QScrollBar *slider = new QScrollBar(hv);
    slider->setRange(0, maxval);
    slider->setSingleStep(1);
    slider->setPageStep(10);

    return slider;
}

lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) : //do it here!!
    QDialog(parent), m_v3d(_v3d)
{
    win_list = m_v3d.getImageWindowList();
    //QObject *test = m_v3d.currentImageWindow();
    v3dhandle curwin = m_v3d.currentImageWindow();
        if (!curwin)
        {
            v3d_msg("You don't have any image open in the main window.");
            return;
        }
        m_v3d.open3DWindow(curwin);

    QList<NeuronTree> * nt_list = m_v3d.getHandleNeuronTrees_3DGlobalViewer(curwin);
    NeuronTree nt1 = nt_list->first(); //this fixed the getting rid of the surface problem

    //need to expand this sometime to account for multiple SWC files?
    int m = nt1.listNeuron.count();

    //float inf = 1.0/0.0;
    min_num = 100000000.0;
    max_num = -10000000.0;

    for(int i = 0; i < m; i++){
            //get minimum z coordinate, save to a variable
            float temp = nt1.listNeuron.at(i).z;
            if(temp<min_num)
            {
                min_num = temp;
            }
    }

    for(int i = 0; i < m; i++){
            //get maximum z coordinate, save to a variable
            float temp2 = nt1.listNeuron.at(i).z;
            if(temp2>max_num)
            {
                max_num = temp2;
            }
        }

    QStringList items;
    for (int i=0; i<win_list.size(); i++)
        items << m_v3d.getImageName(win_list[i]);

    combo_master = new QComboBox(); combo_master->addItems(items);

    label_master = new QLabel(QObject::tr("Display: "));

    Image4DSimple* subject = _v3d.getImage(curwin);
    sz2 = subject->getZDim();
    sz22 = (float)sz2;

    if(sz2<256){
        zcminSlider = createCutPlaneSlider(sz2); //-1
        zcmaxSlider = createCutPlaneSlider(sz2); //-1
        zcmaxSlider->setMinimum(0);
        zcminSlider->setMinimum(0);
        zcmaxSlider->setValue(sz2); //-1
        zcminSlider->setValue(0);
    }

    if(sz2>=256){
        zcminSlider = createCutPlaneSlider(255);
        zcmaxSlider = createCutPlaneSlider(255); //NEW
        zcmaxSlider->setMinimum(0);
        zcminSlider->setMinimum(0);
        zcmaxSlider->setValue(255); //this helped...
        zcminSlider->setValue(0);
    }

    QLabel* SampleName = new QLabel(QObject::tr("Z Cut Min"));
    QLabel* SampleNameTwo = new QLabel(QObject::tr("Z Cut Max"));
    zcLock = new QToolButton(); zcLock->setCheckable(true);

    b_autoON = false; //no idea what this means

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_master, 1,0,1,6);
    gridLayout->addWidget(combo_master,1,1,1,6);

    setLayout(gridLayout);
    setWindowTitle(QString("Sync Image and Surface Cut"));
    gridLayout->addWidget(SampleName, 15,0,1,1);
    gridLayout->addWidget(SampleNameTwo, 17,0,1,1);

    gridLayout->addWidget(zcminSlider,15,3,1,16); //15,0,1,6 //maybe add dimensions later
    gridLayout->addWidget(zcmaxSlider,17,3,1,16);
    gridLayout->addWidget(zcLock, 20, 1, 1, 3); //NEW

    if (zcminSlider)
    {
            connect(zcminSlider, SIGNAL(valueChanged(int)), this, SLOT(change_z_min())); //SLOT(setZCut0(int))
    }


    if(zcmaxSlider){
        connect(zcmaxSlider, SIGNAL(valueChanged(int)), this, SLOT(change_z_max()));
    }

    if (zcLock) {
            connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLock(bool))); //setZCutLock(false);//3. glWidget
            connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLockIcon(bool))); setZCutLockIcon(false); //default
     }



    //should we check the window close event? // by PHC

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
    win_list_past = win_list; //no idea what this is

}

lookPanel::~lookPanel()
{
    if (m_pTimer) {delete m_pTimer; m_pTimer=0;}
}

void lookPanel::setZCutLock(bool b)
{
    //change this first part??
    //seems to work even though I haven't rescaled X or Y.

    //b is successfully being returned.
    /**
    stringstream ss (stringstream::in | stringstream::out);
    ss << b;
    string test = ss.str();
    v3d_msg(test.c_str());
    **/

    if (b) {
        dzCut = Y - X;
    }
    else dzCut = 0;
    lockZ = b? 1:0; //this is right
}

/**
void lookPanel::setZCutLock(bool b)
{
    //change this first part??
    //seems to work even though I haven't rescaled X or Y.
    if (b) {
        dzCut = Y - X;
    }
    else dzCut = 0;
    lockZ = b? 1:0;
}
 */

/**
void lookPanel::setZCutLockIcon(bool b){
    if (! zcLock)  return;
        if (b)
            zcLock->setIcon(QIcon(":/pic/Lockon.png"));
        else
            zcLock->setIcon(QIcon(":/pic/Lockoff.png"));
}
**/

//this did not make any difference
void lookPanel::setZCutLockIcon(bool b){
    //if (! zcLock)  return;
        if (b==1){
            zcLock->setIcon(QIcon(":/pic/Lockon.png"));
        }
        if (b==0){
            zcLock->setIcon(QIcon(":/pic/Lockoff.png"));
        }
}

void lookPanel::change_z_min(){
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i1 = combo_master->currentIndex();

    if (i1 <  win_list.size() &&
            i1 < win_list_past.size() )
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);
        if (current1==past1)
        {
           if (win_list[i1])//ensure the 3d viewer window is open; if not, then open it (took out  && win_list[i2])
           {
                m_v3d.open3DWindow(win_list[i1]);
                View3DControl *view_master = m_v3d.getView3DControl(win_list[i1]);

                if (view_master)
                {

                    view_master->absoluteRotPose();

                    X = zcminSlider->sliderPosition();
                    view_master->setZCut0(X);


                    if(sz2<256){
                        dist_MIN = fabs((min_num-((float)X)));

                        if ((float)X>min_num){

                         view_master->setZClip0((dist_MIN/(max_num-min_num))*200);

                          }

                        if((float)X<=min_num){
                         view_master->setZClip0(0);
                         }

                    }

                    if(sz2>=256){
                         dist_MIN = fabs(((float)(X*((sz22-1.0)/255.0)))-(min_num)); //from 256

                    if (((float)(X*((sz22-1.0)/255.0)+0.5))>(min_num+0.5)){ //559.0
                         view_master->setZClip0(((dist_MIN+0.5)/((max_num+0.5)-(min_num+0.5)))*200);
                    }


                    if(((float)(X*((sz22-1.0)/255.0)+0.5))<=(min_num+0.5)){ //- =
                        view_master->setZClip0(0);
                    }

                    }


                    if (lockZ){
                        zcmaxSlider->setValue(X+dzCut);

                        if((zcmaxSlider->value())==(zcmaxSlider->maximum())){
                            if(sz2<256){
                                if((zcminSlider->value())==0){
                                    zcmaxSlider->setValue(sz2);
                                    zcminSlider->setValue(0);
                                }
                                else{
                                save_z_min = ((zcmaxSlider->maximum())-dzCut)-1; //take out the -1?
                                zcminSlider->setValue(save_z_min);
                                }
                            }
                            if(sz2>=256){
                                if((zcminSlider->value())==0){
                                    zcmaxSlider->setValue(255);
                                    zcminSlider->setValue(0);
                                }
                                else{
                                save_z_min = ((zcmaxSlider->maximum())-dzCut)-1; //take out the -1?
                                zcminSlider->setValue(save_z_min);
                                }
                            }

                        }
                    }

                    /**
                    if (lockZ){
                        zcminSlider->setValue(Y-dzCut);

                        if((zcminSlider->value())==0){
                        if(sz2<256){

                            if((zcmaxSlider->value())==sz2){
                                zcmaxSlider->setValue(sz2);
                                zcminSlider->setValue(0);
                            }
                            save_z_max = dzCut+1; //why +1...I forget
                            zcmaxSlider->setValue(save_z_max);
                        }

                        if(sz2>=256){
                            if((zcmaxSlider->value())==255){
                                zcmaxSlider->setValue(255);
                                zcminSlider->setValue(0);
                            }
                            save_z_max = dzCut+1; //why +1...I forget
                            zcmaxSlider->setValue(save_z_max);
                        }

                        }
                    }
                    **/

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
        win_list_past = win_list;
        return;
    }
    return;
}

void lookPanel::change_z_max(){
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i1 = combo_master->currentIndex();

    if (i1 <  win_list.size() &&
            i1 < win_list_past.size() )
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);
        if (current1==past1)
        {
           if (win_list[i1])//ensure the 3d viewer window is open; if not, then open it (took out  && win_list[i2])
           {
                m_v3d.open3DWindow(win_list[i1]);
                View3DControl *view_master = m_v3d.getView3DControl(win_list[i1]);

                if (view_master)
                {
                    view_master->absoluteRotPose();

                    Y = zcmaxSlider->sliderPosition();
                    view_master->setZCut1(Y);

             if(sz2<256){
                 dist_MAX = fabs((float)Y-max_num);

                 if((float)Y<max_num){
                    view_master->setZClip1(200-((dist_MAX/(max_num-min_num))*200));
                 }

                 if((float)Y>=max_num){
                       view_master->setZClip1(200);
                 }


             }

             if(sz2>=256){
                dist_MAX = fabs(((float)(Y*((sz22-1.0)/255.0))-(max_num))); //from 256
                    if (((float)(Y*((sz22-1.0)/255.0)+0.5))<(max_num+0.5)){ //zcutmax //+ =
                        //show a max z coordinate of max_num - dist_MAX
                         view_master->setZClip1(200-(((dist_MAX+0.5)/((max_num+0.5)-(min_num+0.5)))*200)); //dist_MAX
                    }

                    if(((float)((Y*((sz22-1.0)/255.0))+0.5))>=(max_num+0.5)){ //- =
                        view_master->setZClip1(200);

                    }

             }

             /**
                    if (lockZ){
                        zcminSlider->setValue(Y-dzCut);

                        if((zcminSlider->value())==0){
                            save_z_max = dzCut+1; //why +1...I forget
                            zcmaxSlider->setValue(save_z_max);
                        }
                    }

               **/
                    //In the locked case

                    if (lockZ){
                        zcminSlider->setValue(Y-dzCut);

                        if((zcminSlider->value())==0){
                        if(sz2<256){

                            if((zcmaxSlider->value())==sz2){
                                zcmaxSlider->setValue(sz2);
                                zcminSlider->setValue(0);
                            }
                            else{
                            save_z_max = dzCut+1; //why +1...I forget
                            zcmaxSlider->setValue(save_z_max);
                            }
                        }

                        if(sz2>=256){
                            if((zcmaxSlider->value())==255){
                                zcmaxSlider->setValue(255);
                                zcminSlider->setValue(0);
                            }
                            else{
                            save_z_max = dzCut+1; //why +1...I forget
                            zcmaxSlider->setValue(save_z_max);
                            }
                        }

                        }
                    }


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
        win_list_past = win_list;
        return;
    }
    return;
}
