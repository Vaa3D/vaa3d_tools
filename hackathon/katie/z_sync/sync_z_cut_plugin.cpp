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

/**
QString zCut_altTip(QWidget* parent, int v, int minv, int maxv, int offset)
{
    lookPanel* w; //V3dr_GLWidget* w;
    if (parent) //&& (w = ((lookPanel*)parent)->getGLWidget()))
        return w->Cut_altTip(3, v, minv, maxv, offset);
    else
        return "";
}
**/

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

QAbstractSlider *lookPanel::createCutPlaneSlider(int maxval, Qt::Orientation hv) //lookPanel = V3D main window
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

    label_master = new QLabel(QObject::tr("Master-window: "));

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
    QPushButton* update = new QPushButton("Update zed");
    //QPushButton* ok2 = new QPushButton("Show surf_dim");

    zcminSlider = createCutPlaneSlider(140);
    zcmaxSlider = createCutPlaneSlider(140);

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
    gridLayout->addWidget(update, 12,12);
    //gridLayout->addWidget(ok2, 17,8);

    setLayout(gridLayout);
    setWindowTitle(QString("Sync Image and Surface Cut"));
    //gridLayout->addWidget(box_ZCut_Min, 13,0,1,2);
    gridLayout->addWidget(SampleName, 15,0,1,1);
    //gridLayout->addWidget(box_ZCut_Max, 13,4,1,2);
    gridLayout->addWidget(SampleNameTwo, 17,0,1,1);

    gridLayout->addWidget(zcminSlider,15,3,1,6); //15,0,1,6 //maybe add dimensions later
    gridLayout->addWidget(zcmaxSlider,17,3,1,6);

    //connect(ok, SIGNAL(clicked()), this, SLOT(_slot_sync_onetime())); //see zcutmin
    //connect(ok2, SIGNAL(clicked()), this, SLOT(showSWC_min_and_max()));
    //connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(update, SIGNAL(clicked()), this, SLOT(change_zed_min()));
    //connect(check_zed, SIGNAL(stateChanged(int)), this, SLOT(update()));

    /**
        box_ZCut_Min->setMaximum(140);
        box_ZCut_Min->setMinimum(0);
        box_ZCut_Min->setValue(0);

        box_ZCut_Max->setMaximum(140);
        box_ZCut_Max->setMinimum(0);
        box_ZCut_Max->setValue(140);
        **/

        //redundant, I think...
        /**
        zcminSlider->setMaximum(100);
        zcminSlider->setMinimum(0);
        zcminSlider->setValue(0);
        **/

    zcmaxSlider->setValue(140);

    //connect(box_ZCut_Min, SIGNAL(valueChanged(double)), this, SLOT(update()));
    //connect(box_ZCut_Max, SIGNAL(valueChanged(double)), this, SLOT(update()));

    if (zcminSlider)
    {
            //connect(glWidget, SIGNAL(changeZCut0(int)), zcminSlider, SLOT(setValue(int)));
            connect(zcminSlider, SIGNAL(valueChanged(int)), this, SLOT(update())); //SLOT(setZCut0(int))

    }

    if(zcmaxSlider){
        connect(zcmaxSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    }

    //should we check the window close event? // by PHC

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
    win_list_past = win_list; //no idea what this is

//list_anchors = new QListWidget(); //NO
    //gridLayout->addWidget(list_anchors,6,0,5,5); //NO

//list_min_max = new QListWidget();
    //gridLayout->addWidget(list_min_max,6,0,5,5); //moved to list_anchors position

}

lookPanel::~lookPanel()
{
    if (m_pTimer) {delete m_pTimer; m_pTimer=0;}
}

//change zed min updates everything...
void lookPanel::change_zed_min()
{
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

                    //for the QSpinBox; take this out for now
                    /**
                    int  N = box_ZCut_Min->text().toInt(); //this gives z cut info to the viewer
                    view_master->setZCut0(N);

                    int L = box_ZCut_Max->text().toInt();
                    view_master->setZCut1(L); //added
                    **/

                    //for the slider
                    int X = zcminSlider->sliderPosition();
                    view_master->setZCut0(X);

                    int Y = zcmaxSlider->sliderPosition();
                    view_master->setZCut1(Y);

                    dist_MIN = fabs((min_num-((float)X))); //make sure to declare in public: also
                    dist_MAX = fabs(((float)Y)-max_num); //ditto //fabs()

                    //QString dist_MIN_str = QString::number(dist_MIN); //works but don't use zcutmin, that's stupid.
                    //v3d_msg(dist_MIN_str); //tester to see if the subtraction works at all...

                    if ((float)X>min_num){ //zcutmin
                        //show a min z coordinate of min_num + dist_MIN
                         view_master->setZClip0((dist_MIN/(max_num-min_num))*200); //clearly this isn't working...//dist_MIN
                         //this is still in the "surface coordinate system, which WILL NOT work.
                    }

                    if ((float)Y<max_num){ //zcutmax
                        //show a max z coordinate of max_num - dist_MAX
                         view_master->setZClip1(200-((dist_MAX/(max_num-min_num))*200)); //dist_MAX
                    }

                    if((float)X<=min_num){
                        view_master->setZClip0(0);
                    }

                    if((float)Y>=max_num){
                        view_master->setZClip1(200);
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
