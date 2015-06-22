/* multi_channel_swc_plugin.cpp
 * This plugin will creat swc files based on multiple channel information in the neuron image.
 * 2015-6-18 : by Sumit and Hanbo
 */
 
#include "v3d_message.h"
#include <vector>
#include "multi_channel_swc_plugin.h"
#include "multi_channel_swc_dialog.h"
#include "multi_channel_swc_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(multi_channel_swc, MultiChannelSWC);
 
QStringList MultiChannelSWC::menulist() const
{
	return QStringList() 
        <<tr("multi_channel_compute")
       <<tr("multi_channel_render")
		<<tr("about");
}

QStringList MultiChannelSWC::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void MultiChannelSWC::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("multi_channel_compute"))
    {
        multi_channel_swc_dialog dialog(&callback);
        dialog.exec();
	}
    else if (menu_name == tr("multi_channel_render"))
    {
        do_calculate_render_eswc();
    }
	else
	{
        v3d_msg(tr("This plugin will create swc files based on multiple channel information in the neuron image.."
			"Developed by Sumit and Hanbo, 2015-6-18"));
	}
}

bool MultiChannelSWC::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
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

void do_calculate_render_eswc()
{
    QSettings settings("V3D plugin","multiChannelSWC");
    QString fname_neuronTree;
    if(settings.contains("fname_swc"))
        fname_neuronTree=settings.value("fname_swc").toString();

    //read swc
    QString fileOpenName=fname_neuronTree;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    NeuronTree nt;
    if(!fileOpenName.isEmpty()){
        nt = readSWC_file_multichannel(fileOpenName);
    }else{
        return;
    }
    if(nt.listNeuron.size()>0){
        fname_neuronTree=fileOpenName;
    }else{
        v3d_msg("Error: cannot read file "+fileOpenName);
        return;
    }
    if(nt.listNeuron.at(0).fea_val.size()==0){
        v3d_msg("Error: could not find feature informations from the file "+fileOpenName);
    }
    settings.setValue("fname_swc",fname_neuronTree);

    //do the calculation
    NeuronTree nt_new = convert_SWC_to_render_ESWC(nt);

    //save eswc
    QString fileSaveName=fname_neuronTree+"_render.eswc";
    fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                    fileSaveName,
                QObject::tr("Supported file (*.eswc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
    if(fileOpenName.isEmpty()){
        return;
    }
    writeESWC_file(fileSaveName,nt_new);
}

