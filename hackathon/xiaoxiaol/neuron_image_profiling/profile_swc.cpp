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
#include "eswc_core.h"
#include "math.h"
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



	QString swcFileName = openDlg->file_name;
        QString output_csv_file = swcFileName + QString("out.csv");

        float dilate_radius = 0.0;

    if (!intensity_profile(nt, image, dilate_radius, output_csv_file,callback))
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

    NeuronTree  neuronTree;
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
        {
            neuronTree = readSWC_file(swcFileName);

        }
        else
        {
                cout<<"The file type you specified is not supported."<<endl;
                return false;
        }

    Image4DSimple *image = callback.loadImage((char * )imageFileName.toStdString().c_str());

    if (!intensity_profile(neuronTree, image, dilate_radius, output_csv_file, callback))
	{
		cout<<"Error in intensity_profile() !"<<endl;
		return false;
	}
	return true;

}



//return singal to noise ratio
IMAGE_METRICS  compute_metrics(Image4DSimple *image,  QList<NeuronSWC> neuronSegment,V3DPluginCallback2 &callback){

    IMAGE_METRICS metrics;
    metrics.snr = 0.0;
    metrics.dy = 0.0;

    V3DLONG min_x = INFINITY, min_y = INFINITY, max_x = 0, max_y = 0;
    for (V3DLONG i =0 ; i < neuronSegment.size() ; i++)
    {
       NeuronSWC node = neuronSegment.at(i);
       if(min_x > node.x) min_x = node.x;
       if(min_y > node.y) min_y = node.y;
       if(max_x < node.x) max_x = node.x;
       if(max_y < node.y) max_y = node.y;
    }

    unsigned char * fg_1d;
    unsigned char * bg_1d;
    unsigned char * roi_1d;

    int width = max_x-min_x+1;
    int length = max_y-min_y+1;
    int size_1d = width * length;
    roi_1d = new unsigned char[size_1d];
    V3DLONG d = 0;
    for(V3DLONG i = min_y; i < max_y; i++)
        for(V3DLONG j = min_x; j < max_x; j++)
        {
            roi_1d [d]= image->getRawData()[i*image->getXDim()+j];
            d++;
        }
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)roi_1d, width, length, 1, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "cropped.result");
    callback.updateImageWindow(newwin);


    return metrics;

}

bool intensity_profile(NeuronTree neuronTree, Image4DSimple * image, float dilate_radius, QString output_csv_file, V3DPluginCallback2 &callback)
{
    // parse swc, divide into segments
    vector<V3DLONG> segment_id;
    vector<V3DLONG> segment_layer; //not used

    swc2eswc(neuronTree, segment_id, segment_layer);
    // Assume the neuron tree nodes are ordered by segments (sorted neurons),
    // so the last node has the biggest segmentent label and
    // the nodes are sorted by the segment id, in increasing order.

    V3DLONG num_segments =  segment_id.at(segment_id.size()-1);

    QList<NeuronSWC> neuronSWCs =  neuronTree.listNeuron;
    V3DLONG pre_id = 1;
    QList<NeuronSWC> neuronSegment;
    for (V3DLONG i = 0 ; i < neuronSWCs.size() ; i++)
    {
         if (segment_id[i] == pre_id)
         {
             neuronSegment.push_back( neuronSWCs.at(i) );
         }
         else
         {
            compute_metrics( image, neuronSegment,callback );
            neuronSegment.clear();
            pre_id = segment_id[i];
         }
    }

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






