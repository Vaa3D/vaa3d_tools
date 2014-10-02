/* node_connect_plugin.cpp
 * This is a plugin that takes two marker point inputs, and connects the two swc nodes corresponding to the same location.
 * 2014-08-08 : by SurobhiGanguly
 */
 
#include "v3d_message.h"
#include <vector>
#include <v3d_interface.h>
#include "my_surf_objs.h"
#include "openSWCDialog.h"
#include "node_connect_plugin.h"
#include <QObject>
#include <iostream>
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include "node_connect_func.h"


using namespace std;

Q_EXPORT_PLUGIN2(node_connect, TestPlugin)

static controlPanel *panel = 0;

void NodeConnectWindow(V3DPluginCallback2 &v3d, QWidget *parent);

#define MYFLOAT double

void finishSyncPanel()
{
    if (panel)
    {
        delete panel;
        panel=0;
    }
}

QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("node_connect")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("node_connect")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("node_connect"))
	{
        panel = new controlPanel(callback,parent);
        NodeConnectWindow(callback,parent);
    }

	else
	{
        v3d_msg(tr("This is a plugin that takes two marker point inputs, and connects the two swc nodes corresponding to the same location. "
			"Developed by SurobhiGanguly, 2014-08-08"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("node_connect"))
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

void NodeConnectWindow(V3DPluginCallback2 &v3d, QWidget *parent)
{
    if (panel)
    {
        panel->show();
        return;
    }
    else
    {
        panel = new controlPanel(v3d, parent); //this shouldn't happen.

        if (panel)
        {
            panel->show();
            panel->raise();
            panel->move(100,100);
            panel->activateWindow();
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

    // now re-check the currently opened windows
    v3dhandleList cur_list_triview = m_v3d->getImageWindowList();
    QList <V3dR_MainWindow *> cur_list_3dviewer = m_v3d->getListAll3DViewers();

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

controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    //need 2 buttons, 2 labels, and 2 fill-in-the-blank type areas

    QPushButton* btn_Load = new QPushButton("Load Markers"); //loads markers from image or file if image not open
    QPushButton* btn_Connect = new QPushButton("Connect"); //connects nodes every two marker locations
//    QPushButton* btn_SaveFinal = new QPushButton("Save Latest Version as Final");

    QLabel* status_load_label = new QLabel(QObject::tr("Marker Loading Status: "));
    QLabel* status_connect_label = new QLabel(QObject::tr("Connecting Status: "));

    status_load = new QLabel("plugin not run yet");
    status_connect = new QLabel("plugin not run yet");

    gridLayout = new QGridLayout();
    gridLayout->addWidget(btn_Load, 1,0,1,3);
    gridLayout->addWidget(status_load_label, 2,0,1,3);
    gridLayout->addWidget(status_load, 2,3,1,3);
    gridLayout->addWidget(btn_Connect, 3,0,1,3);
    gridLayout->addWidget(status_connect_label, 4,0,1,3);
    gridLayout->addWidget(status_connect, 4,3,1,3);

    setLayout(gridLayout);
    setWindowTitle(QString("Connect Nodes"));

    connect(btn_Load, SIGNAL(clicked()), this, SLOT(_slot_load())); //loads marker file
    connect(btn_Connect, SIGNAL(clicked()), this, SLOT(_slot_connect())); //connects marker points and loads new swc file
    //connect(status_load, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(_slot_show_item(QListWidgetItem *)));
    //connect(status_connect, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(_slot_show_item(QListWidgetItem *)));

    //connect(btn_SaveFinal, SIGNAL(clicked()), this, SLOT(_slot_save()));
}

controlPanel::~controlPanel()
{
     if(panel){delete panel; panel=0;}

}

static QList<ImageMarker> marker_list;
static bool markers_loaded = false;

void controlPanel::_slot_load() //loads markers.
{
    marker_list = get_markers(m_v3d);
    status_load->clear();

    if (marker_list.size() < 1)
    {
        status_load->setText("no markers loaded");
    }
    else if (marker_list.size()%2==1) //is odd
    {
        status_load->setText("odd number of markers");
    }
    else
    {
        status_load->setText("markers loaded");
        markers_loaded = true;
    }
}

void controlPanel::_slot_connect()//V3DPluginCallback2 &_v3d, QWidget *parent)
{
    if (markers_loaded)
    {
        connect_swc(m_v3d, marker_list);
        status_connect->setText("connect function complete");
        status_load->setText("load new set of markers");

    }
    else
    {
        v3d_msg("must have an even number of markers to use connect function");
    }
}

