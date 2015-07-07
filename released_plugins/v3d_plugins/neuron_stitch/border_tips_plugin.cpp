/* border_tips_plugin.cpp
 * This plugin is for link neuron segments across stacks.
 * 2014-10-07 : by Hanbo Chen
 */

#include "v3d_message.h"
#include <vector>
#include "border_tips_plugin.h"
#include "neuron_stitch_func.h"
#include "neuron_tipspicker_dialog.h"

using namespace std;

Q_EXPORT_PLUGIN2(search_border_tips, border_tips)

QStringList border_tips::menulist() const
{
    return QStringList()
        <<tr("find_border_tips_SWC_image")
		<<tr("about");
}

QStringList border_tips::funclist() const
{
    return QStringList()
      //  <<tr("tmp_test")
		<<tr("help");
}

void border_tips::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("find_border_tips_SWC_image"))
    {
        dosearch(callback, parent);
    }
	else
	{
        v3d_msg(tr("This plugin is for searching border tips between adjacent neuron sections.. "
			"Developed by Hanbo Chen, 2014-10-07"));
	}
}

bool border_tips::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("help"))
	{
        qDebug()<<"No do func available.";
    }
	else return false;

	return true;
}

void border_tips::dosearch(V3DPluginCallback2 &callback, QWidget *parent)
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
        if(ntTreeList->size()>0)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC file you want to identify border tips on.");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Stitcher"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
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

    neuron_tipspicker_dialog * myDialog = NULL;
    myDialog = new neuron_tipspicker_dialog(&callback, v3dwin);
    myDialog->show();
}
