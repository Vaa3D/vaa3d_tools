/* NeuronPopulator_plugin.cpp
 * Use this plugin to simulate a group of neurons with desired density. This simulator also counts the contacts between axons and dendrites.
 * 2015-8-5 : by Xiaoxiao Liu
 */
 
#include "v3d_message.h"
#include <vector>
#include "NeuronPopulator_plugin.h"
#include "populate_neurons.h"
#include <iostream>
#include "../neuron_image_profiling/openSWCDialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(NeuronPopulator, NeuronPopulator);



 
QStringList NeuronPopulator::menulist() const
{
	return QStringList() 
		<<tr("populate")
		<<tr("detect")
		<<tr("about");
}

QStringList NeuronPopulator::funclist() const
{
	return QStringList()
		<<tr("populate")
		<<tr("detect")
		<<tr("help");
}

void NeuronPopulator::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("populate"))
	{
        menu_populate(callback,parent);
	}
	else if (menu_name == tr("detect"))
	{
        menu_detect(callback,parent);
	}
	else
	{
        v3d_msg(tr("Use this plugin to simulate a group of neurons with desired density. This simulator also detects and counts the contacts between axons and dendrites."
			"Developed by Xiaoxiao Liu, 2015-8-5"));
	}
}

void NeuronPopulator::menu_populate( V3DPluginCallback2 &callback, QWidget *parent)
{


    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

     QString swc_file_name = openDlg->file_name;
     NeuronTree sampleNeuron = openDlg->nt;

     float maxRotation = QInputDialog::getDouble(parent, "max rotation",
                                          "Enter max rotation degrees:",
                                          15.0, 1.0, 360.0);
     int  num_neurons = QInputDialog::getInteger(parent, "number of neurons",
                                          "Enter the number of neurons you want to populate:",
                                          10, 1, 10000);

     int  siz_x = QInputDialog::getInteger(parent, "bounding box size in x-axis",
                                          "Enter bounding box size in x-axis:",
                                          100, 10, 10000);

     int  siz_y = QInputDialog::getInteger(parent, "bounding box size in y-axis",
                                          "Enter bounding box size in y-axis:",
                                          100, 10, 10000);

     int  siz_z = QInputDialog::getInteger(parent, "bounding box size in z-axis",
                                          "Enter bounding box size in z-axis:",
                                          100, 10, 10000);


     //----------------
     QList<NeuronTree> neuronTreeList = populate_neurons( sampleNeuron, num_neurons, maxRotation,  siz_x,  siz_y,  siz_z);


     //visualize neuronTreeList

     //------------------------

     QString  output_ano_file = swc_file_name.left(swc_file_name.size()-4)+'.ano';
     v3d_msg("output ano file can be found at:" + output_ano_file);

     QStringList swc_file_list;
     for (int i = 0; i< neuronTreeList.size(); i++)
     {
         QString fn = swc_file_name.left(swc_file_name.size()-4)+'_' + QString::number(i) +'.swc';
         writeSWC_file(fn, neuronTreeList[i]);
         swc_file_list.push_back(fn);
     }

     P_ObjectFileType fnList;
     fnList.swc_file_list = swc_file_list;
     saveAnoFile(output_ano_file,fnList);

}



void NeuronPopulator::menu_detect(V3DPluginCallback2 &callback, QWidget *parent)
{
   //To be implemented

    /*OpenANODialog * openDlg = new OpenANODialog(0, &callback);
    if (!openDlg->exec())
        return;

     QList<NeuronTree> neuronTreeList = openDlg->nts;
     QString ano_file_name = openDlg->fn;


     //----------------
     QList<ImageMarker> markers =  detect_contacts(neuronTreeList, 2, 3);  // 2--axon  3--dendrite


     //Visualize markers


     //------------------------

     QString  output_marker_file = ano_file_name.left(ano_file_name.size()-4)+'.marker';
     writeMarker_file(output_marker_file, markers);

     v3d_message("output contacts(landmarks) into:" + output_marker_file);*/
}




bool NeuronPopulator::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{


	if (func_name == tr("populate"))
	{
        func_populate( input, output,callback);

	}
	else if (func_name == tr("detect"))
	{
        func_detect( input, output,callback);
	}
	else if (func_name == tr("help"))
	{
        cout <<"This plugin is used for 1) populate neurons with specified density ( number of neurons, and bounding box size). 2) detect axon and dendrite contacts.\n";
        cout<<"usage:\n";
        cout<<"v3d -x neuron_populator -f populate -i <inswc_file> -o <out_ano_file> -p max_z_rotation num_of_neurons size_x size_y size_z "<<endl;
        cout<<"inswc_file:\t\t input swc file\n";
        cout<<"out_ano_file:\t\t output ano file that lists simulated swc files\n";
        cout<<"num_of_neurons:\t\t output ano file that lists simulated swc files\n";
        cout<<"size_x:\t\t bounding box size in x\n";
        cout<<"size_y:\t\t bounding box size in y\n";
        cout<<"size_z:\t\t bounding box size in z\n";
        cout <<"\n\n\n";

        cout<<"v3d -x neuron_populator -f detect -i <in_swc_ano_file> -o <list_of_landmarks>" <<endl;
        cout<<"in_swc_ano_file:\t\t input ano file that lists simulated swc files\n";
        cout<<"list_of_landmarks:\t\t output axon and dendrite contacting landmarks into a file\n";

	}
	else return false;

	return true;
}



bool NeuronPopulator::func_populate(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.size() < 1 )
    {
        cerr<<"Invalid input"<<endl;
        return false;
    }
    QString swc_file_name = QString(infiles[0]);

    float  maxRotation = (inparas.size() >= 1) ? atof(inparas[0]) : 15;
    int  num_neurons = (inparas.size() >= 2) ? atoi(inparas[1]) : 10;
    int  siz_x = (inparas.size() >= 3) ? atoi(inparas[2]) : 100;
    int  siz_y = (inparas.size() >= 4) ? atoi(inparas[3]) : 100;
    int  siz_z = (inparas.size() >= 5) ? atoi(inparas[4]) : 100;

    cout<< "max rotation = " << maxRotation << endl;
    cout<< "siz_x =:" << siz_x << endl;
    cout<< "siz_y =:" << siz_y << endl;
    cout<< "siz_z =:" << siz_z << endl;


    QString output_ano_file;
    if(!outfiles.empty()){
        output_ano_file = QString(outfiles[0]);}
    else{
        output_ano_file = swc_file_name.left(swc_file_name.size()-4) + ".ano";
    }

    NeuronTree sampleNeuron =  readSWC_file(swc_file_name);

    //----------------
    QList<NeuronTree> neuronTreeList = populate_neurons( sampleNeuron, num_neurons, maxRotation,  siz_x,  siz_y,  siz_z);

    QStringList swc_file_list;
    for (int i = 0; i< neuronTreeList.size(); i++)
    {
        QString fn = swc_file_name.left(swc_file_name.size()-4)+'_' + QString::number(i) +'.swc';
        writeSWC_file(fn, neuronTreeList[i]);
        swc_file_list.push_back(fn);
    }

    P_ObjectFileType fnList;
    fnList.swc_file_list = swc_file_list;
    saveAnoFile(output_ano_file,fnList);

    return true;


}


bool NeuronPopulator::func_detect(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.size() <1)
    {
        cerr<<"Invalid input"<<endl;
        return false;
    }

    QString anoFileName = QString(infiles[0]);
    QString output_marker_file;
    if(!outfiles.empty())
        output_marker_file = QString(outfiles[0]);
    else
        output_marker_file = anoFileName + ".marker";

    P_ObjectFileType fnList;
    loadAnoFile(anoFileName,fnList);
    QStringList swc_file_list = fnList.swc_file_list;
    QList<NeuronTree> neuronTreeList;

    for (int i = 0; i< swc_file_list.size(); i++)
    {
        NeuronTree tree = readSWC_file(swc_file_list[i]);
        neuronTreeList.push_back(tree);

    }

    //----------------
    QList<ImageMarker> markers =  detect_contacts(neuronTreeList, 2, 3);  // 2--axon  3--dendrite


    writeMarker_file(output_marker_file, markers);

    return true;
}

