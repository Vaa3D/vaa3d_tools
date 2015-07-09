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
#include <math.h>
#include <numeric>
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

        float dilate_ratio = 3.0;

    if (!intensity_profile(nt, image, dilate_ratio, output_csv_file,callback))
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


    float  dilate_ratio = 3.0;

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
            dilate_ratio = 1.0;
		}
		else if (paralist->size()==1)
		{
            dilate_ratio  = atof(paralist->at(0));
            cout<<"dialation radius: "<< dilate_ratio<<endl;
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

    if (!intensity_profile(neuronTree, image, dilate_ratio, output_csv_file, callback))
	{
		cout<<"Error in intensity_profile() !"<<endl;
		return false;
	}
	return true;

}



IMAGE_METRICS  compute_metrics(Image4DSimple *image,  QList<NeuronSWC> neuronSegment, float dilate_ratio, V3DPluginCallback2 &callback){

    IMAGE_METRICS metrics;
    metrics.snr = 0.0;
    metrics.dy = 0.0;

    V3DLONG min_x = INFINITY, min_y = INFINITY,  min_z = INFINITY, max_x = 0, max_y = 0, max_z= 0;

    //get the  bounding box of ROI include the background defined by the dilate_ratio
    for (V3DLONG i =0 ; i < neuronSegment.size() ; i++)
    {
       NeuronSWC node = neuronSegment.at(i);
       float r;
       if (node.radius < 0){
           r = 1;
       }
       else{
           r = node.radius;
       }

       float dilate_radius = dilate_ratio * r;
       V3DLONG node_x_min = node.x - r - dilate_radius + 0.5; // with rounding
       V3DLONG node_y_min = node.y - r - dilate_radius + 0.5;
       V3DLONG node_z_min = node.z - r - dilate_radius + 0.5;

       V3DLONG node_x_max = node.x + r + dilate_radius + 0.5;
       V3DLONG node_y_max = node.y + r + dilate_radius + 0.5;
       V3DLONG node_z_max = node.z + r + dilate_radius + 0.5;

       if(min_x > node_x_min) min_x = node_x_min;
       if(min_y > node_y_min) min_y = node_y_min;
       if(min_z > node_z_min) min_z = node_z_min;
       if(max_x < node_x_max) max_x = node_x_max;
       if(max_y < node_y_max) max_y = node_y_max;
       if(max_z < node_z_max) max_z = node_z_max;
    }


    min_x = min_x < 0 ? 0 : min_x;
    min_y = min_y < 0 ? 0 : min_y;
    min_z = min_z < 0 ? 0 : min_z;
    max_x = max_x > (image->getXDim()-1) ? (image->getXDim() - 1) : max_x;
    max_y = max_y > (image->getYDim()-1) ? (image->getYDim() - 1) : max_y;
    max_z = max_z > (image->getZDim()-1) ? (image->getZDim() - 1) : max_z;

    int width =  max_x - min_x + 1;
    int height = max_y - min_y + 1;
    int depth =  max_z - min_z + 1;

    int size_1d = width * height *depth;

    cout << "min: "<< min_x<<" "<< min_y <<" " <<min_z<<endl;
    cout << "max: "<< max_x<<" "<< max_y <<" " <<max_z<<endl;
    cout << "size:" << width<<" x" <<height <<" x"<<depth<<endl;


    vector <double> fg_1d;
    vector <double> bg_1d;

    unsigned char  * roi_1d_visited = new  unsigned char [size_1d];
    for (V3DLONG i = 0; i < size_1d ; i++){ roi_1d_visited[i] = 0;} //not visited = 0


    unsigned char FG = 255; //foreground
    unsigned char BG = 100; //background

    for (V3DLONG i = 0; i < neuronSegment.size() ; i++)
    {
        NeuronSWC node = neuronSegment.at(i);
        float r;
        if (node.radius < 0 ){
            r = 1;
        }
        else{
            r = node.radius;
        }
        float dilate_radius = dilate_ratio * r;

        V3DLONG xb,xe,yb,ye,zb,ze;



        //label foreground
        xb = node.x - r +0.5; if(xb<0) xb = 0;
        xe = node.x + r +0.5; if(xe>image->getXDim()-1) xe = image->getXDim()-1;
        yb = node.y - r +0.5; if(yb<0) yb = 0;
        ye = node.y + r +0.5; if(ye>image->getYDim()-1) ye = image->getYDim()-1;
        zb = node.z - r +0.5; if(zb<0) zb = 0;
        ze = node.z + r +0.5; if(ze>image->getZDim()-1) ze = image->getZDim()-1;
        for (V3DLONG z = zb; z <= ze; z++)
        {
            for ( V3DLONG y = yb; y <= ye; y++)
            {
                for ( V3DLONG x = xb; x <= xe; x++)
                {
                    V3DLONG index_1d = z * (image->getXDim() * image->getYDim())  + y * image->getXDim() + x;
                    V3DLONG roi_index =  (z - min_z) * (width * height)  + (y - min_y) * width + (x - min_x);
                    if  (roi_1d_visited[roi_index] != FG)
                    {
                        roi_1d_visited[roi_index] = FG;
                        fg_1d.push_back(double(image->getRawData()[index_1d]));
                    }
                }

            }

        }

        //label background
         xb = node.x - r - dilate_radius + 0.5; if(xb<0) xb = 0;
         xe = node.x + r + dilate_radius + 0.5; if(xe>image->getXDim()-1) xe = image->getXDim()-1;
         yb = node.y - r - dilate_radius + 0.5; if(yb<0) yb = 0;
         ye = node.y + r + dilate_radius + 0.5; if(ye>image->getYDim()-1) ye = image->getYDim()-1;
         zb = node.z - r - dilate_radius + 0.5; if(zb<0) zb = 0;
         ze = node.z + r + dilate_radius + 0.5; if(ze>image->getZDim()-1) ze = image->getZDim()-1;
        for (V3DLONG z = zb; z <= ze; z++)
        {
            for ( V3DLONG y = yb; y <= ye; y++)
            {
                for ( V3DLONG x = xb; x <= xe; x++)
                {
                    V3DLONG index_1d = z * (image->getXDim() * image->getYDim())  + y * image->getXDim() + x;
                    V3DLONG roi_index =  (z - min_z) * (width * height)  + (y - min_y) * width + (x - min_x);
                    if  (roi_1d_visited[roi_index] == 0)
                    {
                        roi_1d_visited[roi_index] = BG;
                        bg_1d.push_back(double(image->getRawData()[index_1d]));
                    }
                }

            }

        }
////

    }

    //for debug purpose
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *) roi_1d_visited, width, height, depth, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "cropped.result");
    callback.updateImageWindow(newwin);


    // compute metrics
    double max_fg =  *( max_element(fg_1d.begin(), fg_1d.end()));
    double min_fg =  * (min_element(fg_1d.begin(), fg_1d.end()));
    metrics.dy = max_fg - min_fg;

    double bg_mean  = accumulate( bg_1d.begin(), bg_1d.end(), 0.0 )/ bg_1d.size();
    double fg_mean  = accumulate( fg_1d.begin(), fg_1d.end(), 0.0 )/ fg_1d.size();
    double sum2;

    for ( V3DLONG i = 0; i < bg_1d.size(); i++ )
    {
        sum2 += pow(bg_1d[i]-bg_mean,2);
    }
    double bg_deviation= sqrt(sum2/(bg_1d.size()-1));

    if (bg_deviation != 0.0){
        metrics.snr = fabs(fg_mean - bg_mean)/bg_deviation;
    }
    else {
        metrics.snr  = INFINITY;
        cout<<"warning! background deviation is zero"<<endl;
    }


    cout<<"\n\n\n My Segment "<< ":dy = "<<metrics.dy <<"; snr = "<<metrics.snr <<"\n\n\n\n\n"<< endl;
    return metrics;

}

bool intensity_profile(NeuronTree neuronTree, Image4DSimple * image, float dilate_ratio, QString output_csv_file, V3DPluginCallback2 &callback)
{
    // parse swc, divide into segments
    vector<V3DLONG> segment_id;
    vector<V3DLONG> segment_layer; //not used

    swc2eswc(neuronTree, segment_id, segment_layer);
    // Assume the neuron tree nodes are ordered by segments (sorted neurons),
    // so the last node has the biggest segmentent label and
    // the nodes are sorted by the segment id, in increasing order.

    V3DLONG num_segments =  segment_id.at(segment_id.size()-1);
    cout<<"\n\n\n\n\n TotalSegment number :"<<num_segments<<endl;

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
            if (!neuronSegment.empty())
            {
              cout<<"Segment # :"<<pre_id<<endl;
              IMAGE_METRICS metrics = compute_metrics( image, neuronSegment,dilate_ratio, callback );
            }
            neuronSegment.clear();
            pre_id = segment_id[i];
         }
    }

    if (!neuronSegment.empty())
    {
      cout<<"Segment # :"<<segment_id[segment_id.size()-1]<<endl;
      IMAGE_METRICS metrics = compute_metrics( image, neuronSegment,dilate_ratio,callback );
     }

	return true; 
}

void printHelp(const V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("This plugin is used for profiling images with SWC specified ROIs.");
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"This plugin is used for profiling 2D images with SWC specified ROIs.\n";
    cout<<"usage:\n";
	cout<<"-f<func name>:\t\t profile_swc\n";
	cout<<"-i<file name>:\t\t input .swc or .ano file\n";
	cout<<"-o<file name>:\t\t (not required) output statistics of intensities into a csv file. DEFAUTL: 'ouput.csv'\n";
    cout<<"-p<dilation radius>:\t (not required) the dilation ratio to expand the radius for background ROI extraction";
	cout<<"Example:\t ./v3d -x neuron_image_profiling -f profile_swc -i test.swc -o test.swc.output.csv -p 0.0\n";

}






