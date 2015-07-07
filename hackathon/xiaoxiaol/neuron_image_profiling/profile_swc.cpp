/*
  profile_swc.cpp
  profile swc plugin functions for domenu and do func 
*/

#include <v3d_interface.h>
#include "v3d_message.h"
#include "openSWCDialog.h"
#include "profile_swc.h"
#include <vector>
#include <iostream>
using namespace std;

const QString title = QObject::tr("Image Profile with SWC ROI");

bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined)
{
        V3DLONG neuronNum = linker.size();
        if (neuronNum<=0)
        {
                cout<<"the linker file is empty, please check your data."<<endl;
                return false;
        }
        V3DLONG offset = 0;
        combined = linker[0];
        for (V3DLONG i=1;i<neuronNum;i++)
        {
                V3DLONG maxid = -1;
                for (V3DLONG j=0;j<linker[i-1].size();j++)
                        if (linker[i-1][j].n>maxid) maxid = linker[i-1][j].n;
                offset += maxid+1;
                for (V3DLONG j=0;j<linker[i].size();j++)
                {
                        NeuronSWC S = linker[i][j];
                        S.n = S.n+offset;
                        if (S.pn>=0) S.pn = S.pn+offset;
                        combined.append(S);
                }
        }
};


bool profile_swc_menu(V3DPluginCallback2 &callback, QWidget *parent)
{

    v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
       		QMessageBox::information(0, "", "you don't have any image open in the main window");
            return false;
	}

    Image4DSimple * image = callback.getImage(curwin);

	OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
	if (!openDlg->exec())
		return false;

	NeuronTree nt = openDlg->nt;
	QList<NeuronSWC> neuron = nt.listNeuron;


	QString swcFileName = openDlg->file_name;
        QString output_csv_file = swcFileName + QString("out.csv");

        float dilate_radius = 0.0;

	if (!intensity_profile(neuron, image, dilate_radius, output_csv_file))
	{
		cout<<"Error in intensity_profile() !"<<endl;
		return false;
	}
    cout<<" output file:" << output_csv_file.toStdString() <<endl;
	return true;

}

bool  profile_swc_func(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;


	float  dilate_radius = 0.0;

	QList<NeuronSWC> neuron;
	bool hasPara, hasOutput;
	if (input.size() < 3)
	{
		cout<<"No new parameter specified.\n";
		hasPara = false;
	}
	else {
		hasPara = true;
		paralist = (vector<char*>*)(input.at(2).p);
	}

	if (inlist->size() < 3)
	{
		cout<<"You must specify both the input image file and input swc file!"<<endl;
		return false;
	}

	if (output.size()==0){
		cout<<"No output file specified.\n";
		hasOutput = false;
	}
	else {  
		hasOutput = true;
		if (output.size()>1)
		{       
			cout<<"You have specified more than 1 output file.\n";
			return false;
		}
		outlist = (vector<char*>*)(output.at(0).p);
	}


	if (hasPara)
	{
		if (paralist->size()==0)
		{
			cout<<"Dilation diameter is set to be 0 by default."<<endl;
			dilate_radius = 0;   
		}
		else if (paralist->size()==1)
		{
			dilate_radius  = atof(paralist->at(0));
			cout<<"dialation radius: "<< dilate_radius<<endl;
		}
		else    
		{
			cout<<"Illegal parameter list."<<endl;
			return false;
		}
	}


        QString swcFileName = QString(inlist->at(0));
        QString imageFileName = QString(inlist->at(1));
        QString output_csv_file = swcFileName + "./out.csv";
        if (hasOutput)
        {
                cout <<"output file: "<< outlist->at(0)<<endl;
                output_csv_file = QString(outlist->at(0));
        }       
        else
        {
                output_csv_file= swcFileName+QString("_sorted.swc");
        }

        if (swcFileName.endsWith(".swc") || swcFileName.endsWith(".SWC"))
                neuron = readSWC_file(swcFileName).listNeuron;
        else if (swcFileName.endsWith(".ano") || swcFileName.endsWith(".ANO"))
        {
                P_ObjectFileType linker_object;
                if (!loadAnoFile(swcFileName,linker_object))
		{
			cout<<"Error in reading the linker file."<<endl;
                        return false;
                }
                QStringList nameList = linker_object.swc_file_list;
                V3DLONG neuronNum = nameList.size();
                vector<QList<NeuronSWC> > nt_list;
                for (V3DLONG i=0;i<neuronNum;i++)
                {
                        QList<NeuronSWC> tmp = readSWC_file(nameList.at(i)).listNeuron;
                        nt_list.push_back(tmp);
                }
                if (!combine_linker(nt_list, neuron))
                {
                        cout<<"Error in combining neurons."<<endl;
                        return false;
                }
        }
        else
        {
                cout<<"The file type you specified is not supported."<<endl;
                return false;
        }

    Image4DSimple *image = callback.loadImage((char * )imageFileName.toStdString().c_str());

    if (!intensity_profile(neuron, image, dilate_radius, output_csv_file))
	{
		cout<<"Error in intensity_profile() !"<<endl;
		return false;
	}
	return true;

}

bool intensity_profile(QList<NeuronSWC> neuron, Image4DSimple * image, float dilate_radius, QString output_csv_file)
{
	return true; 

}

void printHelp(const V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("This plugin is used for profiling images with SWC specified ROIs.");
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"This plugin is used for profiling images with SWC specified ROIs.\n";
    cout<<"usage:\n";
	cout<<"-f<func name>:\t\t profile_swc\n";
	cout<<"-i<file name>:\t\t input .swc or .ano file\n";
	cout<<"-o<file name>:\t\t (not required) output statistics of intensities into a csv file. DEFAUTL: 'ouput.csv'\n";
	cout<<"-p<dilation radius>:\t (not required) the dilation radius to expand the radius sepcified in swc file to exclude from background for ROI calculation"; 
	cout<<"Example:\t ./v3d -x neuron_image_profiling -f profile_swc -i test.swc -o test.swc.output.csv -p 0.0\n";

}






