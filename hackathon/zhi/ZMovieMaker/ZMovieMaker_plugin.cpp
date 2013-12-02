/* ZMovieMaker_plugin.cpp
 * This plugin can be used to generate a smooth movie by several points
 * 2013-11-21 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "ZMovieMaker_plugin.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>

using namespace std;
const double pi = 3.1415926535897;
Q_EXPORT_PLUGIN2(ZMovieMaker, ZMovieMaker);

void MovieFromPoints(V3DPluginCallback2 &v3d, QWidget *parent);
static lookPanel *panel = 0;

void angles_to_quaternions(float q[], float xRot, float yRot,float zRot);
void slerp_zhi(float q1[], float q2[],float alpha,float q_sample[]);
void quaternions_to_angles(float Rot_current[], float q_sample[]);

float dot_multi(float q1[], float q2[]);
QStringList ZMovieMaker::menulist() const
{
	return QStringList() 
        <<tr("Generate a movie by multiple anchor points")
		<<tr("about");
}

QStringList ZMovieMaker::funclist() const
{
    return QStringList()
        <<tr("func1")
        <<tr("func2")
        <<tr("help");
}

void ZMovieMaker::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Generate a movie by multiple anchor points"))
	{
        MovieFromPoints(callback, parent);
	}else
	{
        v3d_msg(tr("This plugin can be used to generate a smooth movie by multiple anchor points. "
			"Developed by Zhi Zhou, 2013-11-21"));
	}
}

bool ZMovieMaker::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("func1"))
    {
        v3d_msg("To be implemented.");
    }
    else if (func_name == tr("func2"))
    {
        v3d_msg("To be implemented.");
    }
    else if (func_name == tr("help"))
    {
        v3d_msg("To be implemented.");
    }
    else return false;

    return true;
}

void MovieFromPoints(V3DPluginCallback2 &v3d, QWidget *parent)
{
    v3dhandle curwin = v3d.currentImageWindow();
    if (curwin)
        v3d.open3DWindow(curwin);
    else
    {
        QList <V3dR_MainWindow *> windowList = v3d.getListAll3DViewers();
        if(windowList.count()<1)
        {
            v3d_msg("You don't have any image open in the main window or any surface object in the 3D view window.");
            return;
        }
        else if(windowList.count()>1)
            v3d_msg("Please choose a surface object");
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
            panel->setAttribute(Qt::WA_QuitOnClose);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->raise();
            panel->move(100,100);
            panel->activateWindow();
        }
    }
}

lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    QPushButton* Record     = new QPushButton("Record Anchor Points");
    QPushButton* Preview = new QPushButton("Preview and Save Movie");
    QPushButton* Show = new QPushButton("Show Selected Anchor Point");
    QPushButton* Delete = new QPushButton("Delete Selected Anchor Point");
    QPushButton* Upload = new QPushButton("Upload to Youtube");
    QPushButton* Save = new QPushButton("Save file");
    QPushButton* Load = new QPushButton("Load file");

    v3dhandle curwin = m_v3d.currentImageWindow();
    if (!curwin)
        windowList = m_v3d.getListAll3DViewers();

    QStringList items;
    for (int i=0; i<windowList.count(); i++)
        items << m_v3d.getImageName(windowList[i]);

    combo_surface = new QComboBox(); combo_surface->addItems(items);
    label_surface = new QLabel(QObject::tr("Surface List: "));

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_surface, 1,0,1,6);
    gridLayout->addWidget(combo_surface, 2,0,2,6);
    gridLayout->addWidget(Record, 4,0);
    gridLayout->addWidget(Preview,5,6);
    gridLayout->addWidget(Show,6,0);
    gridLayout->addWidget(Delete,7,0);
    gridLayout->addWidget(Save,6,6);
    gridLayout->addWidget(Load,7,6);
    gridLayout->addWidget(Upload,8,0);

    listWidget = new QListWidget();
    gridLayout->addWidget(listWidget,5,0);

    if(curwin)
        combo_surface->setEnabled(false);

    setLayout(gridLayout);
    setWindowTitle(QString("ZMovieMaker"));

    connect(Show, SIGNAL(clicked()), this, SLOT(_slot_show()));
    connect(Delete, SIGNAL(clicked()), this, SLOT(_slot_delete()));
    connect(Record,     SIGNAL(clicked()), this, SLOT(_slot_record()));
    connect(Preview, SIGNAL(clicked()), this, SLOT(_slot_preview()));
    connect(Save, SIGNAL(clicked()), this, SLOT(_slot_save()));
    connect(Load, SIGNAL(clicked()), this, SLOT(_slot_load()));
    connect(Upload, SIGNAL(clicked()), this, SLOT(_slot_upload()));
}

lookPanel::~lookPanel()
{
    panel=0;
}

void lookPanel::_slot_record()
{
    curwin = m_v3d.currentImageWindow();
    View3DControl *view;
    if(curwin)
    {
        m_v3d.open3DWindow(curwin);
        view = m_v3d.getView3DControl(curwin);
    }
    else
    {
        windowList = m_v3d.getListAll3DViewers();
        QString surfaceName = m_v3d.getImageName(windowList[combo_surface->currentIndex()]);
        V3dR_MainWindow *surface_win = m_v3d.find3DViewerByName(surfaceName);
       // view = m_v3d.getView3DControl(surface_win);
        return;

    }
    view->absoluteRotPose();
    float xRot = view->xRot();
    float yRot = view->yRot();
    float zRot = view->zRot();
    float xShift = view->xShift();
    float yShift = view->yShift();
    float zShift = view->zShift();
    float zoom = view->zoom();
    float xCut0 = view->xCut0();
    float xCut1 = view->xCut1();
    float yCut0 = view->yCut0();
    float yCut1 = view->yCut1();
    float zCut0 = view->zCut0();
    float zCut1 = view->zCut1();
    bool  channelB = view->channelB();
    bool  channelR = view->channelR();
    bool  channelG = view->channelG();
    int   showSurf = 0;
   // if(m_v3d.getSWC(curwin).listNeuron.count()>0 && view->isShowSurfObjects() ==2)
     //   showSurf = 2;
    int xClip0 = view->xClip0();
    int xClip1 = view->xClip1();
    int yClip0 = view->yClip0();
    int yClip1 = view->yClip1();
    int zClip0 = view->zClip0();
    int zClip1 = view->zClip1();


    listWidget->addItem(new QListWidgetItem(QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21,%22,%23").arg(xRot).arg(yRot).arg(zRot).arg(xShift).arg(yShift).arg(zShift).arg(zoom).arg(xCut0).arg(xCut1).arg(yCut0).arg(yCut1).arg(zCut0).arg(zCut1).arg(channelR).arg(channelG).arg(channelB).arg(showSurf).arg(xClip0).arg(xClip1).arg(yClip0).arg(yClip1).arg(zClip0).arg(zClip1)));
    gridLayout->addWidget(listWidget,5,0);
  //  NeuronTree nt = m_v3d.getSWC(curwin);
  //  printf("\n\nsurface number is %d,%d\n\n", nt.listNeuron.count(),view->isShowSurfObjects());
  //  printf("\n\n surfacue cut is (%d,%d,%d,%d,%d,%d)\n\n",view->xClip0(),view->xClip1(),view->yClip0(),view->yClip1(),view->zClip0(),view->zClip1());

}

#define SET_3DVIEW \
    { \
        view->resetRotation();\
        view->doAbsoluteRot(xRot,yRot,zRot);\
        view->setXShift(xShift);\
        view->setYShift(yShift);\
        view->setZShift(zShift);\
        view->setZoom(zoom);\
        view->setXCut0(xCut0);\
        view->setXCut1(xCut1);\
        view->setYCut0(yCut0);\
        view->setYCut1(yCut1);\
        view->setZCut0(zCut0);\
        view->setZCut1(zCut1);\
        view->setChannelR(channelR);\
        view->setChannelG(channelG);\
        view->setChannelB(channelB);\
        view->setShowSurfObjects(showSurf);\
        view->setXClip0(xClip0);\
        view->setXClip1(xClip1);\
        view->setYClip0(yClip0);\
        view->setYClip1(yClip1);\
        view->setZClip0(zClip0);\
        view->setZClip1(zClip1);\
        m_v3d.updateImageWindow(curwin);\
   }

#define UPDATE_PARA \
    { \
        xRot_last = xRot;\
        yRot_last = yRot;\
        zRot_last = zRot;\
        xShift_last = xShift;\
        yShift_last = yShift;\
        zShift_last = zShift;\
        zoom_last = zoom;\
        xCut0_last = xCut0;\
        xCut1_last = xCut1;\
        yCut0_last = yCut0;\
        yCut1_last = yCut1;\
        zCut0_last = zCut0;\
        zCut1_last = zCut1;\
        channelR_last = channelR;\
        channelG_last = channelG;\
        channelB_last = channelB;\
        showSurf_last = showSurf;\
        xClip0_last = xClip0;\
        xClip1_last = xClip1;\
        yClip0_last = yClip0;\
        yClip1_last = yClip1;\
        zClip0_last = zClip0;\
        zClip1_last = zClip1;\
   }

#define GET_PARA \
    { \
        xRot = currentParas.at(0).toFloat();\
        yRot = currentParas.at(1).toFloat();\
        zRot = currentParas.at(2).toFloat();\
        xShift = currentParas.at(3).toFloat();\
        yShift = currentParas.at(4).toFloat();\
        zShift = currentParas.at(5).toFloat();\
        zoom = currentParas.at(6).toFloat();\
        xCut0 = currentParas.at(7).toFloat();\
        xCut1 = currentParas.at(8).toFloat();\
        yCut0 = currentParas.at(9).toFloat();\
        yCut1 = currentParas.at(10).toFloat();\
        zCut0 = currentParas.at(11).toFloat();\
        zCut1 = currentParas.at(12).toFloat();\
        channelR = currentParas.at(13).toInt();\
        channelG = currentParas.at(14).toInt();\
        channelB = currentParas.at(15).toInt();\
        showSurf = currentParas.at(16).toInt();\
        xClip0 = currentParas.at(17).toInt();\
        xClip1 = currentParas.at(18).toInt();\
        yClip0 = currentParas.at(19).toInt();\
        yClip1 = currentParas.at(20).toInt();\
        zClip0 = currentParas.at(21).toInt();\
        zClip1 = currentParas.at(22).toInt();\
   }

#define INTERPOLATION_PARA \
    { \
        view->resetRotation();\
        angles_to_quaternions(q1,xRot_last,yRot_last,zRot_last);\
        angles_to_quaternions(q2,xRot,yRot,zRot);\
        slerp_zhi(q1, q2,(float)i/N,q_sample);\
        quaternions_to_angles(Rot_current,q_sample);\
        view->doAbsoluteRot(Rot_current[0],Rot_current[1],Rot_current[2]);\
        view->setXShift(xShift_last + i*(xShift-xShift_last)/N);\
        view->setYShift(yShift_last + i*(yShift-yShift_last)/N);\
        view->setZShift(zShift_last + i*(zShift-zShift_last)/N);\
        view->setZoom(zoom_last + i*(zoom-zoom_last)/N);\
        view->setXCut0(xCut0_last + i*(xCut0-xCut0_last)/N);\
        view->setXCut1(xCut1_last + i*(xCut1-xCut1_last)/N);\
        view->setYCut0(yCut0_last + i*(yCut0-yCut0_last)/N);\
        view->setYCut1(yCut1_last + i*(yCut1-yCut1_last)/N);\
        view->setZCut0(zCut0_last + i*(zCut0-zCut0_last)/N);\
        view->setZCut1(zCut1_last + i*(zCut1-zCut1_last)/N);\
        view->setXClip0(xClip0_last + i*(xClip0-xClip0_last)/N);\
        view->setXClip1(xClip1_last + i*(xClip1-xClip1_last)/N);\
        view->setYClip0(yClip0_last + i*(yClip0-yClip0_last)/N);\
        view->setYClip1(yClip1_last + i*(yClip1-yClip1_last)/N);\
        view->setZClip0(zClip0_last + i*(zClip0-zClip0_last)/N);\
        view->setZClip1(zClip1_last + i*(zClip1-zClip1_last)/N);\
        if((float)i/N < 0.5)\
        {\
            view->setChannelR(channelR_last);\
            view->setChannelG(channelG_last);\
            view->setChannelB(channelB_last);\
            view->setShowSurfObjects(showSurf_last);\
        }\
        else\
        {\
            view->setChannelR(channelR);\
            view->setChannelG(channelG);\
            view->setChannelB(channelB);\
            view->setShowSurfObjects(showSurf);\
        }\
        m_v3d.updateImageWindow(curwin);\
   }

#define SCREENSHOT_SAVEFRAMES \
    { \
        QString BMPfilename = selectedFile + QString("/%1").arg(framenum);\
        m_v3d.screenShot3DWindow(curwin, BMPfilename);\
        framenum++;\
    }

void lookPanel::_slot_preview()
{
    if(!listWidget->count())
    {
        v3d_msg("Please define at least one archor point.");
        return;
    }

    bool rate;
    int  N;
    N = QInputDialog::getInteger(this, "Sample Rate",
                                       "Enter Sample Rate:",
                                       10, 1, 1000, 1, &rate);

    if(!rate)
        return;

    curwin = m_v3d.currentImageWindow();
    m_v3d.open3DWindow(curwin);
    View3DControl *view = m_v3d.getView3DControl(curwin);
    float xRot, yRot,zRot,xShift,yShift,zShift,zoom,xCut0,xCut1,yCut0,yCut1,zCut0,zCut1;
    int showSurf,showSurf_last;
    bool channelR,channelG,channelB,channelR_last,channelG_last,channelB_last;
    float xRot_last, yRot_last,zRot_last,xShift_last,yShift_last,zShift_last,zoom_last,xCut0_last,xCut1_last,yCut0_last,yCut1_last,zCut0_last,zCut1_last;
    int xClip0,xClip1,yClip0,yClip1,zClip0,zClip1;
    int xClip0_last,xClip1_last,yClip0_last,yClip1_last,zClip0_last,zClip1_last;

    float q1[4],q2[4],q_sample[4];
    float Rot_current[3];
    QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
    for(int row = 0; row < listWidget->count(); row++)
    {
        QString currentPoint = listWidget->item(row)->text();
        QStringList currentParas = currentPoint.split(rx);
        GET_PARA

       if(row>0)
       {
           for (int i =1; i<N+1;i++)
           {
               INTERPOLATION_PARA
           }
       }
       else
       {
          SET_3DVIEW

       }

         UPDATE_PARA
   }

   if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Save as movie frames?"), QMessageBox::Yes, QMessageBox::No))
   {

       QFileDialog d(this);
       d.setWindowTitle(tr("Choose output dir:"));
       d.setFileMode(QFileDialog::Directory);
       if(d.exec())
       {
           QString selectedFile=(d.selectedFiles())[0];
           int framenum =0;
           for(int row = 0; row < listWidget->count(); row++)
           {
               QString currentPoint = listWidget->item(row)->text();
               QStringList currentParas = currentPoint.split(rx);
               GET_PARA

              if(row>0)
              {
                  for (int i =1; i<N+1;i++)
                  {
                      INTERPOLATION_PARA
                      SCREENSHOT_SAVEFRAMES
                  }
              }
              else
              {
                  SET_3DVIEW
                  SCREENSHOT_SAVEFRAMES
              }

                 UPDATE_PARA

          }

       }
   }
   else
       return;

}

void lookPanel::_slot_delete()
{
    if(listWidget->currentRow()==-1)
    {
        v3d_msg("Please select a valid archor point.");
        return;
    }

    listWidget->takeItem(listWidget->currentRow());
    curwin = m_v3d.currentImageWindow();
    m_v3d.open3DWindow(curwin);
    View3DControl *view = m_v3d.getView3DControl(curwin);
    view->resetRotation();
    view->resetZoomShift();
    m_v3d.updateImageWindow(curwin);
}

void lookPanel::_slot_show()
{
    float xRot, yRot,zRot,xShift,yShift,zShift,zoom,xCut0,xCut1,yCut0,yCut1,zCut0,zCut1;
    bool channelR,channelG,channelB;
    int showSurf;
    int xClip0,xClip1,yClip0,yClip1,zClip0,zClip1;

    if(listWidget->currentRow()==-1)
    {
        v3d_msg("Please select a valid archor point.");
        return;
    }

    QString currentPoint = listWidget->currentItem()->text();
    QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
    QStringList currentParas = currentPoint.split(rx);
    GET_PARA


    curwin = m_v3d.currentImageWindow();
    m_v3d.open3DWindow(curwin);
    View3DControl *view = m_v3d.getView3DControl(curwin);

    SET_3DVIEW

}

void lookPanel::_slot_upload()
{
    v3d_msg("To be implemented!");
}

void lookPanel::_slot_save()
{
    if(!listWidget->count())
    {
        v3d_msg("Please define at least one archor point.");
        return;
    }

    QFileDialog d(this);
    d.setWindowTitle(QObject::tr("Choose output anchor point filename"));
    d.setAcceptMode(QFileDialog::AcceptSave);
    if (!d.exec()) return;
    QString textfilename = (d.selectedFiles())[0] + ".txt";
    v3d_msg(textfilename);
    ofstream myfile;
    myfile.open (textfilename.toLatin1(),ios::out | ios::app );
    QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
    for(int row = 0; row < listWidget->count(); row++)
    {
        QString currentPoint = listWidget->item(row)->text();
        QStringList currentParas = currentPoint.split(rx);
        myfile << currentParas.at(0).toFloat();myfile << "  ";
        myfile << currentParas.at(1).toFloat();myfile << "  ";
        myfile << currentParas.at(2).toFloat();myfile << "  ";
        myfile << currentParas.at(3).toFloat();myfile << "  ";
        myfile << currentParas.at(4).toFloat();myfile << "  ";
        myfile << currentParas.at(5).toFloat();myfile << "  ";
        myfile << currentParas.at(6).toFloat();myfile << "  ";
        myfile << currentParas.at(7).toFloat();myfile << "  ";
        myfile << currentParas.at(8).toFloat();myfile << "  ";
        myfile << currentParas.at(9).toFloat();myfile << "  ";
        myfile << currentParas.at(10).toFloat();myfile << "  ";
        myfile << currentParas.at(11).toFloat();myfile << "  ";
        myfile << currentParas.at(12).toFloat();myfile << "  ";
        myfile << currentParas.at(13).toFloat();myfile << "  ";
        myfile << currentParas.at(14).toFloat();myfile << "  ";
        myfile << currentParas.at(15).toFloat();myfile << "  ";
        myfile << currentParas.at(16).toFloat();myfile << "  ";
        myfile << currentParas.at(17).toFloat();myfile << "  ";
        myfile << currentParas.at(18).toFloat();myfile << "  ";
        myfile << currentParas.at(19).toFloat();myfile << "  ";
        myfile << currentParas.at(20).toFloat();myfile << "  ";
        myfile << currentParas.at(21).toFloat();myfile << "  ";
        myfile << currentParas.at(22).toFloat();
        myfile << "\n";
    }
    myfile.close();

}

void lookPanel::_slot_load()
{
    QString fileOpenName = QFileDialog::getOpenFileName(this, QObject::tr("Open File"),
            "",
            QObject::tr("Supported file (*.txt)"
                ));
    if(fileOpenName.isEmpty())
    {
        return;
    }

    if (fileOpenName.size()>0)
    {
       listWidget->clear();
       ifstream ifs(fileOpenName.toLatin1());
       string points;
       float xRot, yRot,zRot,xShift,yShift,zShift,zoom,xCut0,xCut1,yCut0,yCut1,zCut0,zCut1;
       bool channelR,channelG,channelB;
       int showSurf;
       int xClip0,xClip1,yClip0,yClip1,zClip0,zClip1;
       while(ifs && getline(ifs, points))
       {
         std::istringstream iss(points);
         iss >> xRot >> yRot >> zRot >> xShift >> yShift >> zShift >> zoom >> xCut0 >> xCut1 >> yCut0 >> yCut1 >> zCut0 >> zCut1 >> channelR >> channelG >> channelB >> showSurf >> xClip0 >> xClip1 >> yClip0 >> xClip1 >> zClip0 >> zClip1;
         listWidget->addItem(new QListWidgetItem(QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21,%22,%23").arg(xRot).arg(yRot).arg(zRot).arg(xShift).arg(yShift).arg(zShift).arg(zoom).arg(xCut0).arg(xCut1).arg(yCut0).arg(yCut1).arg(zCut0).arg(zCut1).arg(channelR).arg(channelG).arg(channelB).arg(showSurf).arg(xClip0).arg(xClip1).arg(yClip0).arg(yClip1).arg(zClip0).arg(zClip1)));
         gridLayout->addWidget(listWidget,5,0);

       }
    }
    return;
}

void angles_to_quaternions(float q[], float xRot, float yRot,float zRot)
{
    float xRot_Rad = xRot * (pi/180.0); //if(xRot_Rad>pi) xRot_Rad -= 2*pi;
    float yRot_Rad = yRot * (pi/180.0); //if(yRot_Rad>pi) yRot_Rad -= 2*pi;
    float zRot_Rad = zRot * (pi/180.0); //if(zRot_Rad>pi) zRot_Rad -= 2*pi;

    q[0] = cos(xRot_Rad/2)*cos(yRot_Rad/2)*cos(zRot_Rad/2)+sin(xRot_Rad/2)*sin(yRot_Rad/2)*sin(zRot_Rad/2);
    q[1] = sin(xRot_Rad/2)*cos(yRot_Rad/2)*cos(zRot_Rad/2)-cos(xRot_Rad/2)*sin(yRot_Rad/2)*sin(zRot_Rad/2);
    q[2] = cos(xRot_Rad/2)*sin(yRot_Rad/2)*cos(zRot_Rad/2)+sin(xRot_Rad/2)*cos(yRot_Rad/2)*sin(zRot_Rad/2);
    q[3] = cos(xRot_Rad/2)*cos(yRot_Rad/2)*sin(zRot_Rad/2)-sin(xRot_Rad/2)*sin(yRot_Rad/2)*cos(zRot_Rad/2);

    return;

}


void slerp_zhi(float q1[], float q2[],float t,float q_sample[])
{
    float cos_t = dot_multi(q1,q2);
    float theta,beta,alpha;
    int flag = 0;
    if(cos_t<0)
    {
        cos_t = -cos_t;
        flag =1;
    }
    theta = acosf(cos_t);
    if ((1.0 - fabsf(cos_t)) < 1e-7)
    {
        beta = 1.0 - t;
        alpha = t;
    }
    else
    {

        beta = sinf(theta - t*theta)/sinf(theta);
        alpha = sinf(t*theta)/sinf(theta);
    }
    if(flag ==1)
        alpha = -alpha;

    printf("slerp result is (%f,%f,%f,%f,%f)\n\n",cos_t,theta,beta,alpha,t);

    for(int i= 0; i<4;i++)
    {
        q_sample[i] = beta*q1[i] + alpha*q2[i];

    }

}

void quaternions_to_angles(float Rot_current[], float q_sample[])
{

    float rot_x = atan2f(2*(q_sample[0]*q_sample[1]+q_sample[2]*q_sample[3]),1-2*(q_sample[1]*q_sample[1]+q_sample[2]*q_sample[2]));
    float rot_y = asinf(2*(q_sample[0]*q_sample[2]-q_sample[3]*q_sample[1]));
    float rot_z = atan2f(2*(q_sample[0]*q_sample[3]+q_sample[1]*q_sample[2]),1-2*(q_sample[2]*q_sample[2]+q_sample[3]*q_sample[3]));

    Rot_current[0] = rot_x * (180.0/pi);  //if( Rot_current[0]<0)  Rot_current[0] = 360.0 - Rot_current[0];
    Rot_current[1] = rot_y * (180.0/pi);  //if( Rot_current[1]<0)  Rot_current[1] = 360.0 - Rot_current[1];
    Rot_current[2] = rot_z * (180.0/pi);  //if( Rot_current[2]<0)  Rot_current[2] = 360.0 - Rot_current[2];

}


float dot_multi(float q1[], float q2[])
{
    float result = 0;

    for(int i= 0; i<4;i++)
    {
        result += q1[i] * q2[i];

    }

    return result;
}









