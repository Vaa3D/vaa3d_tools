/* sync_swc_terafly_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-8-29 : by Zhi Zhou
 */
#include<iostream>
#include "v3d_message.h"
#include <vector>
#include "sync_swc_terafly_plugin.h"
#include "openSWCDialog.h"
using namespace std;
Q_EXPORT_PLUGIN2(sync_swc_terafly, sync_swc_terafly);
int i=0;
qint64 etime1, etime2;
QElapsedTimer timer1;
QString filename;
bool notFirstTime = false;
bool isAbandoned = false;
QList<V3DLONG> out;
static lookPanel *panel = 0;
bool export_txt(QList<V3DLONG> &out,QString fileSaveName);
 
QStringList sync_swc_terafly::menulist() const
{
	return QStringList() 
        <<tr("show_markers")
		<<tr("about");
}

QStringList sync_swc_terafly::funclist() const
{
	return QStringList()
		<<tr("help");
}

void sync_swc_terafly::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("show_markers"))
    {
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;
        filename = openDlg->file_name;
        if (panel)
        {
            panel->show();
            return;
        }
        else
        {
            panel = new lookPanel(callback, parent);

            if (panel)
            {
                panel->show();
                panel->raise();
                panel->move(100,100);      
                panel->activateWindow();
            }
        }

    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by LXF, 2018-6-22"));
	}
}

lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    gridLayout = new QGridLayout();
    QPushButton* show_markers     = new QPushButton("next pairs of markers");
    QPushButton* give_it_up       = new QPushButton("give up");
    give_it_up->setFixedWidth(60);
//    QPushButton* start            = new QPushButton("     start time      ");
 //   QPushButton* end              = new QPushButton("      end time       ");
    gridLayout->addWidget(show_markers, 0,0);
    gridLayout->addWidget(give_it_up, 0,1);
   // gridLayout->addWidget(start, 0,1);
   // gridLayout->addWidget(end, 0,2);
    setLayout(gridLayout);
    setWindowTitle(QString("pairs_marker_show"));
    connect(show_markers,     SIGNAL(clicked()), this, SLOT(_slot_set_markers()));
    connect(give_it_up,     SIGNAL(clicked()), this, SLOT(_slot_abandon()));
 //   connect(start,     SIGNAL(clicked()), this, SLOT(_start_time()));
 //   connect(end,     SIGNAL(clicked()), this, SLOT(_end_time()));

}

lookPanel::~lookPanel()
{
}


void lookPanel::_slot_abandon()
{
    // if already 180s, abandon current markers
    if (!notFirstTime)
        return;

    qint64 curr_time = timer1.elapsed()/1000;
    if (curr_time > 180)
    {
        isAbandoned = true;
        _slot_set_markers();
    }
    else
    {
        QString qstr;
        qstr = "Only " + QString::number(curr_time) + "s spent. Please at least work for 3 minutes!";
        v3d_msg(qstr);
    }
}

void lookPanel::_slot_set_markers()
{

    if(notFirstTime)
    {
        if (isAbandoned)
        {
            isAbandoned = false;
            out.push_back(-999);
        }
        else
        {
            etime1 = timer1.elapsed();
            etime1 = etime1/1000.0;
            out.push_back(etime1);
        }

    }


    v3dhandle win = m_v3d.currentImageWindow();
    QList<ImageMarker> marker;
    marker = readMarker_file(filename);
    if(i==marker.size())
    {
        QDateTime mytime = QDateTime::currentDateTime();
        QString outfile =filename + "_3D_" + mytime.toString("yyyy_MM_dd_hh_mm")+".txt";
        QString outswcfile =filename + "_3D_"+ mytime.toString("yyyy_MM_dd_hh_mm")+".swc";
//        QString outfile = QCoreApplication::applicationDirPath()+"/["+filename+"]_3D_"+mytime.toString("yyyy_MM_dd_hh_mm")+".txt";
//        QString outswcfile = QCoreApplication::applicationDirPath()+"/["+filename+"]_3D_"+mytime.toString("yyyy_MM_dd_hh_mm")+".swc";
        NeuronTree nt = m_v3d.getSWC(win);
        export_txt(out,outfile);
        writeSWC_file(outswcfile,nt);
        notFirstTime = false;
        v3d_msg("there is no new marker any more!");
        return;
    }
    LandmarkList ll;
    LocationSimple ls1,ls2;

    ls1.x = marker[i].x;
    ls1.y = marker[i].y;
    ls1.z = marker[i].z;
    ls1.color.r = 255;
    ls1.color.g = 0;
    ls1.color.b = 0;
    ls1.color.a = 0;

    ls2.x = marker[i+1].x;
    ls2.y = marker[i+1].y;
    ls2.z = marker[i+1].z;
    ls2.color.r = 255;
    ls2.color.g = 0;
    ls2.color.b = 0;
    ls2.color.a = 0;

    ll.push_back(ls1);
    ll.push_back(ls2);

    m_v3d.setLandmark(win,ll);
    m_v3d.updateImageWindow(win);
    m_v3d.pushObjectIn3DWindow(win);
    i=i+2;

    QString dlgTitle = "now pair ";
    dlgTitle += QString::number(i/2);
    setWindowTitle(dlgTitle);

    timer1.start();
    notFirstTime = true;


}



void lookPanel::_start_time()
{

}
void lookPanel::_end_time()
{
    etime1 = timer1.elapsed();
    qDebug() << " **** connect preprocessing takes [" << etime1 << " milliseconds]"<<endl;
    QString timemsg = "connect preprocessing takes " + QString::number(etime1) + " milliseconds";
    v3d_msg(timemsg);
}
bool sync_swc_terafly::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

bool export_txt(QList<V3DLONG> &out,QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);


    for (int i=0;i<out.size();i++)
    {
        myfile << out[i] <<endl;

    }

    file.close();
    return true;
}
