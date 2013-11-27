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
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }
    v3d.open3DWindow(curwin);

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
    QPushButton* Preview = new QPushButton("Preview");
    QPushButton* Show = new QPushButton("Show Anchor Points");
    QPushButton* Delete = new QPushButton("Delete");
    QPushButton* Upload = new QPushButton("Upload to Youtube");
    QPushButton* Save = new QPushButton("Save All Anchor Points");
    QPushButton* Load = new QPushButton("Load file");

    gridLayout = new QGridLayout();
    gridLayout->addWidget(Record, 1,0);
    gridLayout->addWidget(Preview,1,6);
    gridLayout->addWidget(Show,4,0);
    gridLayout->addWidget(Delete,4,6);
    gridLayout->addWidget(Save,5,0);
    gridLayout->addWidget(Load,5,6);
    gridLayout->addWidget(Upload,6,0);

    listWidget = new QListWidget();
    gridLayout->addWidget(listWidget,3,0);

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
    m_v3d.open3DWindow(curwin);
    View3DControl *view = m_v3d.getView3DControl(curwin);
    m_v3d.open3DWindow(curwin);
    view->absoluteRotPose();
    float xRot = view->xRot();
    float yRot = view->yRot();
    float zRot = view->zRot();
    float xShift = view->xShift();
    float yShift = view->yShift();
    float zShift = view->zShift();
    float zoom = view->zoom();


    listWidget->addItem(new QListWidgetItem(QString("%1,%2,%3,%4,%5,%6,%7").arg(xRot).arg(yRot).arg(zRot).arg(xShift).arg(yShift).arg(zShift).arg(zoom)));
    gridLayout->addWidget(listWidget,3,0);
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
    m_v3d.open3DWindow(curwin);
    float xRot, yRot,zRot,xShift,yShift,zShift,zoom;
    float xRot_last, yRot_last,zRot_last,xShift_last,yShift_last,zShift_last,zoom_last;
    float q1[4],q2[4],q_sample[4];
    float Rot_current[3];
    QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
    for(int row = 0; row < listWidget->count(); row++)
    {
        QString currentPoint = listWidget->item(row)->text();
        QStringList currentParas = currentPoint.split(rx);
        xRot = currentParas.at(0).toFloat();
        yRot = currentParas.at(1).toFloat();
        zRot = currentParas.at(2).toFloat();
        xShift = currentParas.at(3).toFloat();
        yShift = currentParas.at(4).toFloat();
        zShift = currentParas.at(5).toFloat();
        zoom = currentParas.at(6).toFloat();
       if(row>0)
       {
           for (int i =1; i<N+1;i++)
           {
               view->resetRotation();
               angles_to_quaternions(q1,xRot_last,yRot_last,zRot_last);
               angles_to_quaternions(q2,xRot,yRot,zRot);
               slerp_zhi(q1, q2,(float)i/N,q_sample);
               quaternions_to_angles(Rot_current,q_sample);

               view->doAbsoluteRot(Rot_current[0],Rot_current[1],Rot_current[2]);
               view->setXShift(xShift_last + i*(xShift-xShift_last)/N);
               view->setYShift(yShift_last + i*(yShift-yShift_last)/N);
               view->setZShift(zShift_last + i*(zShift-zShift_last)/N);
               view->setZoom(zoom_last + i*(zoom-zoom_last)/N);
               m_v3d.updateImageWindow(curwin);
           }
       }
       else
       {
           view->resetRotation();
           view->doAbsoluteRot(xRot,yRot,zRot);
           view->setXShift(xShift);
           view->setYShift(yShift);
           view->setZShift(zShift);
           view->setZoom(zoom);
           m_v3d.updateImageWindow(curwin);
       }

       xRot_last = xRot;
       yRot_last = yRot;
       zRot_last = zRot;
       xShift_last = xShift;
       yShift_last = yShift;
       zShift_last = zShift;
       zoom_last = zoom;
   }

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
    m_v3d.open3DWindow(curwin);
    view->resetRotation();
    view->resetZoomShift();
    m_v3d.updateImageWindow(curwin);
}

void lookPanel::_slot_show()
{
    float xRot, yRot,zRot,xShift,yShift,zShift,zoom;

    if(listWidget->currentRow()==-1)
    {
        v3d_msg("Please select a valid archor point.");
        return;
    }

    QString currentPoint = listWidget->currentItem()->text();
    QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
    QStringList currentParas = currentPoint.split(rx);
    xRot = currentParas.at(0).toFloat();
    yRot = currentParas.at(1).toFloat();
    zRot = currentParas.at(2).toFloat();
    xShift = currentParas.at(3).toFloat();
    yShift = currentParas.at(4).toFloat();
    zShift = currentParas.at(5).toFloat();
    zoom = currentParas.at(6).toFloat();

    curwin = m_v3d.currentImageWindow();
    m_v3d.open3DWindow(curwin);
    View3DControl *view = m_v3d.getView3DControl(curwin);
    m_v3d.open3DWindow(curwin);

    view->resetRotation();
    view->doAbsoluteRot(xRot,yRot,zRot);
    view->setXShift(xShift);
    view->setYShift(yShift);
    view->setZShift(zShift);
    view->setZoom(zoom);
    m_v3d.updateImageWindow(curwin);

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
        myfile << currentParas.at(6).toFloat();
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
       float xRot, yRot,zRot,xShift,yShift,zShift,zoom;
       while(ifs && getline(ifs, points))
       {
         std::istringstream iss(points);
         iss >> xRot >> yRot >> zRot >> xShift >> yShift >> zShift >> zoom;
         listWidget->addItem(new QListWidgetItem(QString("%1,%2,%3,%4,%5,%6,%7").arg(xRot).arg(yRot).arg(zRot).arg(xShift).arg(yShift).arg(zShift).arg(zoom)));
         gridLayout->addWidget(listWidget,3,0);

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









