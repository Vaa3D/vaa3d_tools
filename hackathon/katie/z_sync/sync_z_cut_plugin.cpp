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

    //QString minstr = QString::number(min_num);

    //v3d_msg(minstr);

    QStringList items;
    for (int i=0; i<win_list.size(); i++)
        items << m_v3d.getImageName(win_list[i]);

    combo_master = new QComboBox(); combo_master->addItems(items);
    //new_combo_master = new QComboBox(); new_combo_master->addItems(items);

    label_master = new QLabel(QObject::tr("Display: "));

    //check_zed = new QCheckBox(); check_zed->setText(QObject::tr("zcutmin"));check_zed->setChecked(true);

    //SliderTipFilter *zVsliderTip = new SliderTipFilter(this, "", "", 1, zCut_altTip);
    //zcminSlider->installEventFilter(zVsliderTip);
    //zcmaxSlider->installEventFilter(zVsliderTip);

    //test this part later
    /**
    SliderTipFilter *SsliderTip = new SliderTipFilter(this, "", "%", -100);
    zSminSlider->installEventFilter(SsliderTip);
    zSmaxSlider->installEventFilter(SsliderTip);
    **/

    //QPushButton* ok     = new QPushButton("Show zed");
    //QPushButton* cancel = new QPushButton("Close");
    //QPushButton* update = new QPushButton("Update zed");
    //QPushButton* ok2 = new QPushButton("Show surf_dim");

    zcminSlider = createCutPlaneSlider(255);
    zcmaxSlider = createCutPlaneSlider(255); //NEW

    zcLock = new QToolButton(); zcLock->setCheckable(true);

    //box_ZCut_Min = new QSpinBox();
    //box_ZCut_Max = new QSpinBox();
    QLabel* SampleName = new QLabel(QObject::tr("Z Cut Min"));
    QLabel* SampleNameTwo = new QLabel(QObject::tr("Z Cut Max"));

    b_autoON = false; //no idea what this means

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_master, 1,0,1,6);
    gridLayout->addWidget(combo_master,1,1,1,6);
    //gridLayout->addWidget(new_combo_master,17,1,1,6);

    //gridLayout->addWidget(check_zed, 4,2,1,1);
    //gridLayout->addWidget(ok, 5,0); //needed?
    //gridLayout->addWidget(cancel,12,6); //needed?
    //gridLayout->addWidget(update, 12,12);
    //gridLayout->addWidget(ok2, 17,8);

    setLayout(gridLayout);
    setWindowTitle(QString("Sync Image and Surface Cut"));
    //gridLayout->addWidget(box_ZCut_Min, 13,0,1,2);
    gridLayout->addWidget(SampleName, 15,0,1,1);
    //gridLayout->addWidget(box_ZCut_Max, 13,4,1,2);
    gridLayout->addWidget(SampleNameTwo, 17,0,1,1);

    gridLayout->addWidget(zcminSlider,15,3,1,16); //15,0,1,6 //maybe add dimensions later
    gridLayout->addWidget(zcmaxSlider,17,3,1,16);
    gridLayout->addWidget(zcLock, 20, 1, 1, 3); //NEW


    //connect(ok, SIGNAL(clicked()), this, SLOT(_slot_sync_onetime())); //see zcutmin
    //connect(ok2, SIGNAL(clicked()), this, SLOT(showSWC_min_and_max()));
    //connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    //connect(update, SIGNAL(clicked()), this, SLOT(change_zed_min())); //OUT
    //connect(check_zed, SIGNAL(stateChanged(int)), this, SLOT(update()));

    //v3dhandleList win_list = m_v3d.getImageWindowList();



    zcmaxSlider->setMinimum(0);
    zcminSlider->setMinimum(0);

    zcmaxSlider->setValue(255); //this helped...
    zcminSlider->setValue(0); //from 0

    //connect(box_ZCut_Min, SIGNAL(valueChanged(double)), this, SLOT(update()));
    //connect(box_ZCut_Max, SIGNAL(valueChanged(double)), this, SLOT(update()));



    if (zcminSlider)
    {
            //connect(glWidget, SIGNAL(changeZCut0(int)), zcminSlider, SLOT(setValue(int)));
            connect(zcminSlider, SIGNAL(valueChanged(int)), this, SLOT(change_z_min())); //SLOT(setZCut0(int))
            //connect(zcminSlider, SIGNAL(valueChanged(int)), parent, SLOT(setZClip0(float)));

    }


    if(zcmaxSlider){

        connect(zcmaxSlider, SIGNAL(valueChanged(int)), this, SLOT(change_z_max()));
        //connect(zcmaxSlider, SIGNAL(valueChanged(int)), parent, SLOT(setZClip1(float)));

    }

    if (zcLock) {
            connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLock(bool))); //3. glWidget
            //connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(showZCutLock()));
        //I need to rewrite setZCutLock for sure
            connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLockIcon(bool))); setZCutLockIcon(false); //default
        }
    //right now just setting to false. a blank icon. this is ok.
    //this isn't actually locking the slider!
    //still not working...argh



    //should we check the window close event? // by PHC

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
    win_list_past = win_list; //no idea what this is

    Image4DSimple* subject = _v3d.getImage(curwin);
    sz2 = subject->getZDim();

      stringstream ss (stringstream::in | stringstream::out);

      ss << sz2;

      string test = ss.str();

      v3d_msg(test.c_str());

}

lookPanel::~lookPanel()
{
    if (m_pTimer) {delete m_pTimer; m_pTimer=0;}
}

//Not working, yet
void lookPanel::setZCutLockIcon(bool b){
    if (! zcLock)  return;
        if (b)
            zcLock->setIcon(QIcon(":/icons/Lockon.png"));
        else
            zcLock->setIcon(QIcon(":/icons/Lockoff.png"));
}


void lookPanel::setZCutLock(bool b)
{
    //change this first part??
    if (b) {

        //if(sz2<256)
        //{
        dzCut = Y - X;
        //}

        /**
        if(sz2>=256)
        {
            dzCut = Y_rescaled-X_rescaled;
        }
        **/
        //it's not locking because X and Y are changing?
        //zcmaxSlider->sliderPosition()==(zcminSlider->sliderPosition()+dzCut);
    //zcmaxSlider->setValue(50); //this works. Sets it to this as soon as I hit the button
    //zcmaxSlider->setValue(dzCut); //is decreasing the value of the zcmaxSlider by 26 each time clicked, seems to work though
        //zcmaxSlider->setValue(X+dzCut); //now try this //the problem is it doesn't update until I click the button, then it doesn't stick
    }
    else dzCut = 0; //I think I can keep this?
    lockZ = b? 1:0; //I should keep this
}


void lookPanel::change_z_min(){
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i1 = combo_master->currentIndex();
    //V3DPluginCallback2 &_v3d

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

                    //max_num is the "true" coordinate. 553 something. Huh.


                    X = zcminSlider->sliderPosition();
                    view_master->setZCut0(X); //works on scale from 1 - 256
                    //view_master->setZCut0((int)(X*(560/256)));
                    //it is not cutting the image at all!

                    //dist_MIN = fabs(((float)X*(560/256))-(min_num*(560/256)));
                    dist_MIN = fabs(((float)(X*(559.0/255.0)))-(min_num)); //from 256
                    X_rescaled = X*(559.0/255.0);

                    int x_coord = view_master->zCut0();

                    if(x_coord==50||x_coord==100||x_coord==150||x_coord==200||x_coord==250){

                        stringstream ss (stringstream::in | stringstream::out);

                        ss << X_rescaled;

                        string test = ss.str();

                        v3d_msg(test.c_str());

                    }


                    //if(sz2<256){

                    //the surface cut lags behind the image cut
                    //surf cut stops at 83, should stop at 1001
                    if (((float)(X*(559.0/255.0)+0.5))>(min_num+0.5)){ //+ =

                         view_master->setZClip0(((dist_MIN+0.5)/((max_num+0.5)-(min_num+0.5)))*200);
                    }


                    if(((float)(X*(559.0/255.0)+0.5))<=(min_num+0.5)){ //- =
                        view_master->setZClip0(0);



                    }



                    if (lockZ){
                        zcmaxSlider->setValue(X+dzCut);
                        //update();
                        //zcmaxSlider->setValue(50); //this works. Sets it to this as soon as I hit the button
                        if((zcmaxSlider->value())==(zcmaxSlider->maximum())){
                            save_z_min = ((zcmaxSlider->maximum())-dzCut)-1; //take out the -1?
                            //zcminSlider->setValue(120); //"jumps" to 120 then ...
                            zcminSlider->setValue(save_z_min);
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
                    //view_master->setZCut1((int)(Y*(256/560)));

                     //dist_MAX = fabs(((float)Y*(560/256))-(max_num*(560/256)));
                    dist_MAX = fabs(((float)(Y*(559.0/255.0))-(max_num))); //from 256

                    //dimensions of the slider are indeed correct. if 560?

                //if(sz2<256){
                    if (((float)(Y*(559.0/255.0)+0.5))<(max_num+0.5)){ //zcutmax //+ =
                        //show a max z coordinate of max_num - dist_MAX
                         view_master->setZClip1(200-(((dist_MAX+0.5)/((max_num+0.5)-(min_num+0.5)))*200)); //dist_MAX
                    }

                    if(((float)((Y*(559.0/255.0))+0.5))>=(max_num+0.5)){ //- =
                        view_master->setZClip1(200); //pull max all the way to the right and it clips at 84
                        //v3d_msg("Larger than max_num!"); //this has yet to show up...


                    }
                //}


                    //In the locked case
                    if (lockZ){
                        zcminSlider->setValue(Y-dzCut);
                        //update();
                        //zcmaxSlider->setValue(50); //this works. Sets it to this as soon as I hit the button

                        if((zcminSlider->value())==0){
                            save_z_max = dzCut+1; //why +1...I forget
                            zcmaxSlider->setValue(save_z_max);
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
