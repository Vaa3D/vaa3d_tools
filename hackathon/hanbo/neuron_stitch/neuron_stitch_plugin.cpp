/* neuron_stitch_plugin.cpp
 * This plugin is for link neuron segments across stacks.
 * 2014-10-07 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_stitch_plugin.h"
#include "neuron_stitch_func.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"

using namespace std;

Q_EXPORT_PLUGIN2(neuron_stitch, neuron_stitch);

QList<NeuronGeometryDialog* > dialogList;

QStringList neuron_stitch::menulist() const
{
	return QStringList() 
        <<tr("adjust_neurons")
        <<tr("affine_transform_neurons_by_matrix")
		<<tr("about");
}

QStringList neuron_stitch::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neuron_stitch::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("adjust_neurons"))
    {
        doadjust(callback, parent);
    }
    else if (menu_name == tr("affine_transform_neurons_by_matrix"))
    {
        dotransform_swc(callback, parent);
    }
	else
	{
		v3d_msg(tr("This plugin is for link neuron segments across stacks.. "
			"Developed by Hanbo Chen, 2014-10-07"));
	}
}

bool neuron_stitch::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void neuron_stitch::doadjust(V3DPluginCallback2 &callback, QWidget *parent)
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
        if(ntTreeList->size()==2)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with only 2 SWC file. Please load the two SWC file you want to stitch in the same 3D view");
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

    //call dialog
    //NeuronGeometryDialog myDialog(&callback, v3dwin);
    //int res=myDialog.exec();
    NeuronGeometryDialog * myDialog = NULL;
//    qDebug("dialog size %d",dialogList.size());
//    for(int i=0; i<dialogList.size(); i++){
//        if(callback.getImageName(dialogList[i]->v3dwin) == callback.getImageName(v3dwin)){
//            myDialog = dialogList[i];
//            break;
//        }
//    }
    if(myDialog == NULL)
    {
        myDialog = new NeuronGeometryDialog(&callback, v3dwin);
//        dialogList.append(myDialog);
    }
    myDialog->show();
}

int neuron_stitch::dotransform_swc(V3DPluginCallback2 &callback, QWidget *parent)
{
    //input file name
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
            "",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fileOpenName.isEmpty())
        return 0;
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        nt = readSWC_file(fileOpenName);
    }

    QString fileMatName = QFileDialog::getOpenFileName(0, QObject::tr("Open Affine Matrix File"),
            "",
            QObject::tr("Supported file (*.txt)"
                ";;Affine Matrix    (*.txt)"
                ));

    double amat[16]={0};
    if (!readAmat(fileMatName.toStdString().c_str(),amat));

    proc_neuron_affine(&nt, amat);

    QString fileDefaultName = fileOpenName+QString("_affine.swc");
    //write new SWC to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.swc)"
                ";;Neuron structure	(*.swc)"
                ));
    if (!export_list2file(nt.listNeuron,fileSaveName,fileOpenName))
    {
        v3d_msg("fail to write the output swc file.");
        return 0;
    }

    return 1;
}
