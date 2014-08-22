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

#define CHECK_WINDOWS \
{\
    view=0;curwin=0; \
    list_triview = m_v3d.getImageWindowList();\
    list_3dviewer = m_v3d.getListAll3DViewers();\
    if(!combo_surface || combo_surface->count()<=0) return;\
    if(combo_surface->currentIndex() < list_triview.size())\
{\
    curwin = list_triview[combo_surface->currentIndex()];\
    if(curwin)\
{\
    m_v3d.open3DWindow(curwin);\
    view = m_v3d.getView3DControl(curwin);\
    }\
    else\
    return;\
    }\
    else\
{\
    QString curname = combo_surface->itemText(combo_surface->currentIndex());\
    v3d_msg(QString("current window selected:[%1]").arg(curname), 0);\
    for (int i=0; i<list_3dviewer.count(); i++)\
{\
    if(curname == m_v3d.getImageName(list_3dviewer[i]))\
{\
    surface_win = list_3dviewer[i];\
    if(surface_win)\
{\
    view = m_v3d.getView3DControl_Any3DViewer(surface_win);\
    }\
    else\
    return;\
    break;\
    }\
    }\
    }\
    if (!view) return;\
    }

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
        v3d_msg(tr("This plugin syncs the z-cut of the image and the surface."
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
    win_list = m_v3d.getImageWindowList(); //this is NOT for the 3D window

    //first check to see if there is an image
    list_3dviewer = m_v3d.getListAll3DViewers();

    curwin = m_v3d.currentImageWindow();
    view = m_v3d.getView3DControl(curwin);\

        if (!curwin)
        {
            v3d_msg("You don't have any image open in the main window.");
            return;
        }
        m_v3d.open3DWindow(curwin);

    nt_list = m_v3d.getHandleNeuronTrees_3DGlobalViewer(curwin); //curwin is only for the main window

if(nt_list->size()==0){
        v3d_msg("You have no surface loaded in the 3D window. Cut is only for the image.");
    }

else{

    min_num = 100000000.0;
    max_num = -10000000.0;

      for (int k = 0; k < nt_list->size(); k++){ //changed from 0, idk
      NeuronTree nt1 = nt_list->at(k); //iterates through the QList, creates a new NeuronTree each time

        int m = nt1.listNeuron.count(); //for the current list, creates new value for number of rows in neuron tree

        for(int i = 0; i < m; i++){ //from 0 to the last row in one neuron tree...
            //get minimum z coordinate, save to a variable
            float temp = nt1.listNeuron.at(i).z; //iterate through z coordinates from 0 to the last row
            if(temp<min_num)
            {
                min_num = temp;
            }
        }

    for(int j = 0; j < m; j++){
            //get maximum z coordinate, save to a variable
            float temp2 = nt1.listNeuron.at(j).z;
            if(temp2>max_num)
            {
                max_num = temp2;
            }
        }

      }

}//check for surface, end

    QStringList items;

    for (int i=0; i<win_list.size(); i++)
        items << m_v3d.getImageName(win_list[i]);

    combo_master = new QComboBox(); combo_master->addItems(items);

    label_master = new QLabel(QObject::tr("File: "));
    label_display = new QLabel(QObject::tr("Active: "));

    Image4DSimple* subject = _v3d.getImage(curwin);
    sz2 = subject->getZDim();
    sz22 = (float)sz2;

    zCut0 = view->zCut0();
        _zCut1 = view->zCut1();
        dimz = (_zCut1 - _zCut0)+1;


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
        zcmaxSlider->setValue(255);
        zcminSlider->setValue(0);
    }

    QLabel* SampleName = new QLabel(QObject::tr("Z Cut Min"));
    QLabel* SampleNameTwo = new QLabel(QObject::tr("Z Cut Max"));

    updateSurf = new QPushButton("Get Surface");

    combo_surface = new MyComboBox(&m_v3d);
    combo_surface->updateList();

    gridLayout = new QGridLayout();
    setLayout(gridLayout);

    gridLayout->addWidget(label_display,4,0,1,6);
    gridLayout->addWidget(combo_surface, 4,1,1,6);

    gridLayout->addWidget(label_master, 1,0,1,6);
    gridLayout->addWidget(combo_master,1,1,1,6);
    gridLayout->addWidget(updateSurf, 12,1,1,2);

    setWindowTitle(QString("Sync Image and Surface Cut"));
    gridLayout->addWidget(SampleName, 15,0,1,1);
    gridLayout->addWidget(SampleNameTwo, 17,0,1,1);

    gridLayout->addWidget(zcminSlider,15,3,1,16); //15,0,1,6 //maybe add dimensions later
    gridLayout->addWidget(zcmaxSlider,17,3,1,16);

    zcLock = new QToolButton(); zcLock->setCheckable(true);
    gridLayout->addWidget(zcLock, 20, 1, 1, 3); //NEW

        lockZ = 0; //immediately set

            connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLock(bool)));
            connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLockIcon(bool))); setZCutLockIcon(false); //default
            connect(updateSurf, SIGNAL(clicked()), this, SLOT(update_traces()));

    if (zcminSlider){
            connect(zcminSlider, SIGNAL(valueChanged(int)), this, SLOT(change_z_min())); //SLOT(setZCut0(int))
    }


    if(zcmaxSlider){
        connect(zcmaxSlider, SIGNAL(valueChanged(int)), this, SLOT(change_z_max()));
    }

    //should we check the window close event? // by PHC

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
    win_list_past = win_list;

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

    //need to add a few more lines to get the current highligh file and then always highlight it unless literally changed
    QString lastDisplayfile = currentText();

    // now re-check the currently opened windows
    v3dhandleList cur_list_triview = m_v3d->getImageWindowList();
    QList <V3dR_MainWindow *> cur_list_3dviewer = m_v3d->getListAll3DViewers(); //this should work...

    QStringList items;
    int i;

    for (i=0; i<cur_list_triview.size(); i++)
        items << m_v3d->getImageName(cur_list_triview[i]);

    for (i=0; i<cur_list_3dviewer.count(); i++)
    {
        QString curname = m_v3d->getImageName(cur_list_3dviewer[i]).remove("3D View [").remove("]");
        bool b_found=false;
        for (int j=0; j<cur_list_triview.size(); j++)
            if (curname==m_v3d->getImageName(cur_list_triview[j]))
            {
                b_found=true;
                break;
            }

        if (!b_found)
            items << m_v3d->getImageName(cur_list_3dviewer[i]);
    }

    //update the list now
    clear();
    addItems(items);

    //search if the lastDisplayfile exists, if yes, then highlight it (set as current),
    //otherwise do nothing (i.e. in this case the list will highlight the 1st one which is new)

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


lookPanel::~lookPanel()
{
    if (m_pTimer) {delete m_pTimer; m_pTimer=0;}
}

void lookPanel::update_traces()
{

    curwin = m_v3d.currentImageWindow();
    nt_list = m_v3d.getHandleNeuronTrees_3DGlobalViewer(curwin); //curwin is only for the main window

    if(nt_list->size()==0){
        v3d_msg("You have no surface loaded in the 3D window. Cut is only for the image.");
    }

    else{

    min_num = 100000000.0;
    max_num = -10000000.0;

     for (int k = 0; k < nt_list->size(); k++){ //changed from 0, idk
      NeuronTree nt1 = nt_list->at(k); //iterates through the QList, creates a new NeuronTree each time
        int m = nt1.listNeuron.count(); //for the current list, creates new value for number of rows in neuron tree

        for(int i = 0; i < m; i++){ //from 0 to the last row in one neuron tree...
            //get minimum z coordinate, save to a variable
            float temp = nt1.listNeuron.at(i).z; //iterate through z coordinates from 0 to the last row
            if(temp<min_num)
            {
                min_num = temp;
            }
        }

        for(int j = 0; j < m; j++){
            //get maximum z coordinate, save to a variable
            float temp2 = nt1.listNeuron.at(j).z;
            if(temp2>max_num)
            {
                max_num = temp2;
            }
        }

      }

    }
}

void lookPanel::setZCutLock(bool b)
{

    if (b) {
        dzCut = Y - X;
    }
    else dzCut = 0;
    lockZ = b? 1:0;
}

void lookPanel::setZCutLockIcon(bool b){

    if (! zcLock)  return;
        if (b==1){
            zcLock->setIcon(QIcon(":/pic/Lockon.png"));
        }
        if (b==0){
            zcLock->setIcon(QIcon(":/pic/Lockoff.png"));
        }
}

void lookPanel::change_z_min(){
    CHECK_WINDOWS;

    int i1 = combo_master->currentIndex();

    if (i1 < win_list.size() &&
            i1 < win_list_past.size() )
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);
        if (current1==past1)
        {


    win_list = m_v3d.getImageWindowList();

           if (win_list[i1])//ensure the 3d viewer window is open; if not, then open it
           {
                m_v3d.open3DWindow(curwin);

                if (view)
                {
                    int min_dim_z = view->getLocalStartPosZ();
                    int max_dim_z = view->getLocalEndPosZ();
                    int min_max_dim = (max_dim_z - min_dim_z)+1;

                    if(min_max_dim!=0&&min_max_dim!=sz2){
                        v3d_msg("Warning! Cut will not sync for ROI cropped in the z-dimension. Please select an ROI from the triview window.");
                    }

                   // view->absoluteRotPose();

                    X = zcminSlider->sliderPosition();
                    Y = zcmaxSlider->sliderPosition();
                    view->setZCut0(X);

                      if (X>=Y){
                          zcmaxSlider->setSliderPosition(X);
                      }


                    if(sz2<256){

                        dist_MIN = fabs((min_num-(float)X));

                        if ((float)X>min_num){

                         view->setZClip0((dist_MIN/(max_num-min_num))*((float)200.0)); //try 200.0

                          }

                        if((float)X<=min_num){
                         view->setZClip0(0);
                         }

                    }

                   if(sz2>=256){
                         dist_MIN = fabs(((float)(X*((sz22-1.0)/255.0)))-(min_num)); //from 256

                    if (((float)(X*((sz22-1.0)/255.0)+0.5))>(min_num+0.5)){ //559.0
                         view->setZClip0(((dist_MIN+0.5)/((max_num+0.5)-(min_num+0.5)))*200); //- view_master
                    }


                    if(((float)(X*((sz22-1.0)/255.0)+0.5))<=(min_num+0.5)){
                        view->setZClip0(0);
                    }

                   }


                    if (lockZ){ //lockZ
                        X = zcminSlider->sliderPosition();
                        view->setZCut0(X);
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
    CHECK_WINDOWS;

    int i1 = combo_master->currentIndex();

    if (i1 < win_list.size() &&
            i1 < win_list_past.size() )
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);
        if (current1==past1)
        {

    win_list = m_v3d.getImageWindowList();

           if (win_list[i1])//ensure the 3d viewer window is open; if not, then open it
           {
                m_v3d.open3DWindow(curwin);


                if (view)
                {
                    int min_dim_z = view->getLocalStartPosZ();
                    int max_dim_z = view->getLocalEndPosZ();
                    int min_max_dim = (max_dim_z - min_dim_z)+1;

                    if(min_max_dim!=0&&min_max_dim!=sz2){
                        v3d_msg("Warning! Cut will not sync for ROI cropped in the z-dimension. Please select an ROI from the triview window.");
                    }

                    //view->absoluteRotPose();

                    X = zcminSlider->sliderPosition();
                    Y = zcmaxSlider->sliderPosition();
                    view->setZCut1(Y);

                    if (Y<=X){
                        zcminSlider->setSliderPosition(Y);
                    }

             if(sz2<256){

                 dist_MAX = fabs((float)(Y-max_num));

                 if((float)Y<max_num){
                    view->setZClip1(200-((dist_MAX/(max_num-min_num))*200));
                 }

                 if((float)Y>=max_num){
                       view->setZClip1(200);
                 }


             }

             if(sz2>=256){
                dist_MAX = fabs(((float)(Y*((sz22-1.0)/255.0))-(max_num)));
                    if (((float)(Y*((sz22-1.0)/255.0)+0.5))<(max_num+0.5)){

                         view->setZClip1(200-(((dist_MAX+0.5)/((max_num+0.5)-(min_num+0.5)))*((float)200.0)));
                    }

                    if(((float)((Y*((sz22-1.0)/255.0))+0.5))>=(max_num+0.5)){
                        view->setZClip1(200);

                    }

             }
                   // }


                    //In the locked case

                    if (lockZ){

                        Y = zcmaxSlider->sliderPosition();
                        view->setZCut1(Y);

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
