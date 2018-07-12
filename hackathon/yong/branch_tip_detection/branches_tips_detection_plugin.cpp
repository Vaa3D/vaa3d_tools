/* branches_tips_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-7-4 : by Yongzhang
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
//<<<<<<< HEAD
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"
#include "branches_tips_detection_plugin.h"
#include "branch_tip_detection.h"

Q_EXPORT_PLUGIN2(branches_tips_detection, TestPlugin);


//=======
////#include "basic_surf_objs.h"
////#include "my_surf_objs.h"
//#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"
//#include "branches_tips_detection_plugin.h"
//#include "branch_tip_detection.h"
//Q_EXPORT_PLUGIN2(branches_tips_detection, TestPlugin);

//>>>>>>> af416b85e5de165d26027b426cefde182d44c722
using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("branch_tip_detection")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("branch_tip_detection")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("branch_tip_detection"))
	{
//        bool bmenu = true;
//        OpenSWCDialog * openDlg=new OpenSWCDialog(0,&callback);
//        if(!openDlg->exec())
//            return;
//        NeuronTree nt= openDlg->nt;
//        QString filename = openDlg->file_name;
//        cout<<filename.toStdString()<<endl;
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                    "",
//<<<<<<< HEAD
                                                    QObject::tr("Supported file (*.swc *.SWC *.eswc)"
//=======
//                                                    QObject::tr("Supported file (*.swc *.SWC)"
//>>>>>>> af416b85e5de165d26027b426cefde182d44c722
                                                        ));

        NeuronTree nt=readSWC_file(fileOpenName);
        QList<NeuronSWC> neuron=nt.listNeuron;
        cout<<fileOpenName.toStdString()<<endl;

//<<<<<<< HEAD
        QList<CellAPO> markers;
        V3DLONG rootid,thres,root_dist_thres;

        bool ok;
        rootid = QInputDialog::getInteger(0, "Would you like to specify new root number?","New root number:(If you select 'cancel', the first root in file is set as default)",1,1,2147483647,1,&ok);
        if (!ok)
            rootid = VOID;
        thres = QInputDialog::getDouble(0, "Would you like to set a threshold for the newly generated link?","threshold:(If you select 'cancel', all the points will be connected automated; If you set '0', no new link will be generated)",0,0,2147483647,1,&ok);
        if (!ok)
            thres = VOID;
        root_dist_thres = QInputDialog::getDouble(0, "Would you like to set a threshold for connecting roots to soma?","threshold:(If you set '0', no new link will be generated. If set, all the roots within the distance of the new root id will be directly connected to it)",0,0,2147483647,1,&ok);
        if (!ok)
            root_dist_thres = 0;

        QList<NeuronSWC> result=SortSWC(neuron,rootid,thres,root_dist_thres,markers,fileOpenName);

        //QList<NeuronSWC> result=removedupSWC(neuron,fileOpenName);
//=======
//        QList<NeuronSWC> result=removedupSWC(neuron,fileOpenName);

//        //cout<<"neuron.size="<<neuron.size()<<endl;
//        //cout<<"result.size="<<result.size()<<endl;
//>>>>>>> af416b85e5de165d26027b426cefde182d44c722

        branch_tip_detection(callback,result,fileOpenName,parent);

	}

	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by Yongzhang, 2018-7-4"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("branch_tip_detection"))
    {
        v3d_msg("To be implemented.");
    }
//<<<<<<< HEAD

//=======
//    if (func_name == tr("func2"))
//    {
//        v3d_msg("To be implemented.");
//    }
//>>>>>>> af416b85e5de165d26027b426cefde182d44c722
    else if (func_name == tr("help"))
    {

       v3d_msg("To be implemented.");

    }
    else return false;

    return true;
}
