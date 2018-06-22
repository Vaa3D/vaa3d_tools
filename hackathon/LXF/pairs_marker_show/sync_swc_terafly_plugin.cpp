/* sync_swc_terafly_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-8-29 : by Zhi Zhou
 */
#include<iostream>
#include "v3d_message.h"
#include <vector>
#include "sync_swc_terafly_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(sync_swc_terafly, sync_swc_terafly);
int i=1;
qint64 etime1, etime2;
QElapsedTimer timer1;
static lookPanel *panel = 0;

 
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
    QPushButton* start            = new QPushButton("     start time      ");
    QPushButton* end              = new QPushButton("      end time       ");
    gridLayout->addWidget(show_markers, 0,0);
    gridLayout->addWidget(start, 0,1);
    gridLayout->addWidget(end, 0,2);
    setLayout(gridLayout);
    setWindowTitle(QString("pairs_marker_show"));
    connect(show_markers,     SIGNAL(clicked()), this, SLOT(_slot_set_markers()));
    connect(start,     SIGNAL(clicked()), this, SLOT(_start_time()));
    connect(end,     SIGNAL(clicked()), this, SLOT(_end_time()));

}

lookPanel::~lookPanel()
{
}




void lookPanel::_slot_set_markers()
{

    v3dhandle win = m_v3d.currentImageWindow();
    QString filename = "113.v3draw.marker";
    QList<ImageMarker> marker;
    marker = readMarker_file(filename);
    if(i>marker.size())
    {
        v3d_msg("there is no new marker any more!");
        return;
    }
    LandmarkList ll;
    LocationSimple ls1,ls2;
    bool ok;
    for(V3DLONG j=0;j<marker.size();j++)
    {
        if(marker[j].comment.toInt(&ok,0)==i)
        {
            ls1.x = marker[j].x;
            ls1.y = marker[j].y;
            ls1.z = marker[j].z;
            ls1.color.r = 255;
            ls1.color.g = 0;
            ls1.color.b = 0;
            ls1.color.a = 0;
        }
        else if(marker[j].comment.toInt(&ok,0)==i+1)
        {

            ls2.x = marker[j].x;
            ls2.y = marker[j].y;
            ls2.z = marker[j].z;
            ls2.color.r = 255;
            ls2.color.g = 0;
            ls2.color.b = 0;
            ls2.color.a = 0;
        }
    }
    ll.push_back(ls1);
    ll.push_back(ls2);
    cout<<ls1.x<<"  "<<ls1.y<<"  "<<ls1.z<<"  "<<endl;
    cout<<ls2.x<<"  "<<ls2.y<<"  "<<ls2.z<<"  "<<endl;
    cout<<"ll = "<<ll.size()<<endl;

    m_v3d.setLandmark(win,ll);
    m_v3d.updateImageWindow(win);
    m_v3d.pushObjectIn3DWindow(win);
    i=i+2;










}
void lookPanel::_start_time()
{
    timer1.start();
}
void lookPanel::_end_time()
{
    etime1 = timer1.elapsed();
    qDebug() << " **** connect preprocessing takes [" << etime1 << " milliseconds]";
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


