/* neuron_tile_display_plugin.cpp
 * This Plugin will tile neuron to display
 * 2014-10-28 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_tile_display_plugin.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"

using namespace std;
Q_EXPORT_PLUGIN2(neuron_tile_display, neuron_tile_display);
 
QStringList neuron_tile_display::menulist() const
{
	return QStringList() 
		<<tr("neuron_tile_display")
		<<tr("about");
}

QStringList neuron_tile_display::funclist() const
{
	return QStringList()
		<<tr("help");
}

void neuron_tile_display::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neuron_tile_display"))
	{
        dotile(callback, parent);
	}
	else
	{
		v3d_msg(tr("This Plugin will tile neuron to display. "
			"Developed by Hanbo Chen, 2014-10-28"));
	}
}

bool neuron_tile_display::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void neuron_tile_display::dotile(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>1)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to tile in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Tile Viewer"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    NeuronTileDisplayDialog * myDialog = NULL;
    myDialog = new NeuronTileDisplayDialog(&callback, v3dwin);
    myDialog->show();
}

NeuronTileDisplayDialog::NeuronTileDisplayDialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
{

    v3dwin=inwin;
    callback=cb;

    //create
    check_tile = new QCheckBox("tile neurons");
    check_tile->setChecked(false);
    connect(check_tile, SIGNAL(stateChanged(int)), this, SLOT(checktile(int)));

    spin_x = new QDoubleSpinBox();
    spin_x->setRange(0,100000); spin_x->setValue(0);
    connect(spin_x, SIGNAL(valueChanged(double)), this, SLOT(tile(double)));
    spin_y = new QDoubleSpinBox();
    spin_y->setRange(0,100000); spin_y->setValue(100);
    connect(spin_y, SIGNAL(valueChanged(double)), this, SLOT(tile(double)));
    spin_z = new QDoubleSpinBox();
    spin_z->setRange(0,100000); spin_z->setValue(0);
    connect(spin_z, SIGNAL(valueChanged(double)), this, SLOT(tile(double)));

    btn_quit = new QPushButton("Quit");
    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));

    //layout
    QGridLayout * gridLayout = new QGridLayout();
    gridLayout->addWidget(check_tile,0,0,1,1);
    QLabel* label_1 = new QLabel("X direction steps:");
    gridLayout->addWidget(label_1,1,0,1,1);
    gridLayout->addWidget(spin_x,1,1,1,1);
    QLabel* label_2 = new QLabel("Y direction steps:");
    gridLayout->addWidget(label_2,2,0,1,1);
    gridLayout->addWidget(spin_y,2,1,1,1);
    QLabel* label_3 = new QLabel("Z direction steps:");
    gridLayout->addWidget(label_3,3,0,1,1);
    gridLayout->addWidget(spin_z,3,1,1,1);
    gridLayout->addWidget(btn_quit,4,1,1,1);

    setLayout(gridLayout);

    //load neuron tree
    ntList=callback->getHandleNeuronTrees_Any3DViewer(v3dwin);

    cur_x = cur_y = cur_z = 0;
}

void NeuronTileDisplayDialog::reject()
{
    reset();
    QDialog::reject();
}

void NeuronTileDisplayDialog::reset()
{
    if(cur_x>0 || cur_y>0 || cur_z>0){
        double move_x, move_y, move_z;
        for(int i=1; i<ntList->size(); i++){
            move_x = -cur_x*i;
            move_y = -cur_y*i;
            move_z = -cur_z*i;
            NeuronTree* p = (NeuronTree*)&(ntList->at(i));
            proc_neuron_add_offset(p, move_x, move_y, move_z);
        }
    }
    cur_x = cur_y = cur_z = 0;

    if(v3dwin){
        callback->update_3DViewer(v3dwin);
    }
}

void NeuronTileDisplayDialog::tile(double dis)
{
    tile();
}

void NeuronTileDisplayDialog::checktile(int c)
{
    if(c == Qt::Unchecked){
        reset();
    }else{
        tile();
    }
}

void NeuronTileDisplayDialog::tile()
{
    if(check_tile->isChecked() ){
        double move_x, move_y, move_z;
        for(int i=1; i<ntList->size(); i++){
            move_x = (spin_x->value()-cur_x)*i;
            move_y = (spin_y->value()-cur_y)*i;
            move_z = (spin_z->value()-cur_z)*i;
            NeuronTree* p = (NeuronTree*)&(ntList->at(i));
            proc_neuron_add_offset(p, move_x, move_y, move_z);
        }
        cur_x = spin_x->value();
        cur_y = spin_y->value();
        cur_z = spin_z->value();
    }

    if(v3dwin){
        callback->update_3DViewer(v3dwin);
    }
}
