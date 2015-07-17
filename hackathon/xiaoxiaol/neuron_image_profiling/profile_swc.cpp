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
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include "v3d_message.h"
#include "compute_tubularity.h"

using namespace std;

const QString title = QObject::tr("Image Profile with SWC ROI");

#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif


static V3DLONG boundValue(V3DLONG x, V3DLONG m_min, V3DLONG m_max)
{
    x = MAX(x, m_min);
    x = MIN(x, m_max);
    return x;

}

static double median(vector<double> x)
{
    sort(x.begin(), x.end());
    return  x[x.size()/2];
}

static void cutoff_outliers(vector<double> & x)
{
    //remove the top and bottom 10% data, to be more robust
    sort(x.begin(), x.end());
    int num_to_remove = x.size()*0.1;

    // erase the top and bottom N elements:
    x.erase( x.begin(), x.begin()+ num_to_remove);
    x.erase( x.end()-num_to_remove, x.end());
}


//flip image along the Y direction ( due to the image matrix order convention in Vaa3D)
bool flip_y (Image4DSimple * image)
{
    unsigned char * data1d = image->getRawData();
    V3DLONG in_sz[3];
    in_sz[0] =image->getXDim();
    in_sz[1] = image->getYDim();
    in_sz[2] = image->getZDim();

    V3DLONG hsz1 = floor((double) (in_sz[1]-1)/2.0);
    if (hsz1*2<in_sz[1]-1)
        hsz1+=1;

    for (int j=0;j<hsz1;j++)
        for (int i=0;i<in_sz[0];i++)
        {
            unsigned char tmpv = data1d[(in_sz[1]-j-1)*in_sz[0] + i];
            data1d[(in_sz[1]-j-1)*in_sz[0] + i] = data1d[j*in_sz[0] + i];
            data1d[j*in_sz[0] + i] = tmpv;
        }

    return true;
}




#define MAX_INTENSITY 255
bool invert_intensity (Image4DSimple * image)
{
    unsigned char * data1d = image->getRawData();
    V3DLONG in_sz[3];
    in_sz[0] = image->getXDim();
    in_sz[1] = image->getYDim();
    in_sz[2] = image->getZDim();

    for (V3DLONG z = 0; z < in_sz[2]; z++)
    {
        for ( V3DLONG y = 0; y < in_sz[1]; y++)
        {
            for ( V3DLONG x = 0; x < in_sz[0]; x++)
            {
                V3DLONG index_1d = z * (image->getXDim() * image->getYDim())  + y * image->getXDim() + x;
                data1d[index_1d] = MAX_INTENSITY - data1d[index_1d] ;
            }
        }
    }

    return true;
}


bool writeMetrics2CSV(QList<IMAGE_METRICS> result_metrics, QString output_csv_file)
{
    QFile file(output_csv_file);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<output_csv_file.toStdString().c_str() << endl;
        return false;
    }
    else
    {
        QTextStream stream (&file);
        stream<< "segment_id" <<","<< "segment_type"<<","<<"dynamic_range"<<","<<"cnr" <<","<<"tubularity"<<"\n";
        for (int i  = 0; i < result_metrics.size() ; i++)
        {
            stream << i+1 <<","<<result_metrics[i].type<<","<<result_metrics[i].dy << "," << result_metrics[i].cnr <<","<< result_metrics[i].tubularity<< "\n";
        }

        file.close();
    }
    return true;

}


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
    QString output_csv_file = swcFileName + QString(".csv");


    float dilate_ratio = QInputDialog::getDouble(parent, "dilate_ratio",
                                 "Enter dialte ratio:",
                                 3.0, 1.0, 100.0);
    int flip = QInputDialog::getInteger(parent, "flip in y ?",
                                 "Flip in Y (0/1):",
                                 0, 0, 1);
    int invert = QInputDialog::getInteger(parent, "invert intensity ? (for calculating tubularities, signals should be brighter than background",
                                 "Invert Intensity (0/1):",
                                 0, 0, 1);

    QList<IMAGE_METRICS> result_metrics = intensity_profile(nt, image, dilate_ratio,flip,invert,callback);

    if (result_metrics.isEmpty())
    {
        cout<<"Error in intensity_profile() !"<<endl;
        return false;
    }


    //output
    writeMetrics2CSV(result_metrics, output_csv_file);

    //display metrics to the msg window
    QString disp_text = "Segment ID | Segment Type | Dynamic Range | Contrast-to-Background Ratio | Tubularity \n";
    for (int i  = 0; i < result_metrics.size() ; i++)
    {
     disp_text += QString::number(i+1)+ "            ";
     disp_text += QString::number(result_metrics[i].type) + "             ";;
     disp_text += QString::number(result_metrics[i].dy) + "             ";
     disp_text += QString::number(result_metrics[i].cnr)+ "                          ";
     disp_text += QString::number(result_metrics[i].tubularity)+ "\n";
    }
    disp_text +="Output the metrics into:"+ output_csv_file +"\n";
    v3d_msg(disp_text);


	return true;

}



bool  profile_swc_func(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.size() != 2 && infiles.size() != 3)
    {
        cerr<<"Invalid input"<<endl;
        return false;
    }
    QString imageFileName = QString(infiles[0]);
    QString swcFileName = QString(infiles[1]);
    QString output_csv_file;
    if(!outfiles.empty())
        output_csv_file = QString(outfiles[0]);
    else
        output_csv_file = swcFileName + ".csv";

    float  dilate_ratio = (inparas.size() >= 1) ? atof(inparas[0]) : 3.0;
    int  flip = (inparas.size() >= 2) ? atoi(inparas[1]) : 1;
    int  invert = (inparas.size() >= 3) ? atoi(inparas[2]) : 1;

    cout<<"inimg_file = "<< imageFileName.toStdString()<<endl;
    cout<<"inswc_file = "<< swcFileName.toStdString()<<endl;
    cout<<"output_file = "<< output_csv_file.toStdString()<<endl;
    cout<<"dilate_ratio = "<< dilate_ratio<<endl;
    cout<<"flip y = "<< flip <<endl;
    cout<<"invert intensity = "<< invert <<endl;

    NeuronTree  neuronTree;

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

    QList<IMAGE_METRICS> result_metrics = intensity_profile(neuronTree, image, dilate_ratio,flip,invert, callback);

    if (result_metrics.isEmpty())
    {
        cout<<"Error in intensity_profile() !"<<endl;
        return false;
    }
    else{
        if (!writeMetrics2CSV(result_metrics, output_csv_file))
        {
            cout<< "error in writeMetrics2CSV()" <<endl;
        }
    }


	return true;

}



IMAGE_METRICS  compute_metrics(Image4DSimple *image,  QList<NeuronSWC> neuronSegment, float dilate_ratio, V3DPluginCallback2 &callback)
{

    IMAGE_METRICS metrics;
    metrics.type = neuronSegment.at(0).type; // one segment is one type ( all it's nodes should have the same type)
    metrics.cnr = 0.0;
    metrics.dy = 0.0;
    metrics.tubularity = 0.0;

    V3DLONG min_x = INFINITY, min_y = INFINITY,  min_z = INFINITY, max_x = 0, max_y = 0, max_z= 0;

    //get the  bounding box of ROI include the background defined by the dilate_ratio
    for (V3DLONG i =0 ; i < neuronSegment.size() ; i++)
    {
       NeuronSWC node = neuronSegment.at(i);
       float r;
       if (node.radius < 0){
           r = 1;
           cout <<" warning: node radius is negtive?! " <<endl;
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


    min_x = boundValue(min_x, 0,image->getXDim()-1 );
    min_y = boundValue(min_y, 0,image->getYDim()-1 );
    min_z = boundValue(min_z, 0,image->getZDim()-1 );

    max_x = boundValue(max_x, 0,image->getXDim()-1 );
    max_y = boundValue(max_y, 0,image->getYDim()-1 );
    max_z = boundValue(max_z, 0,image->getZDim()-1 );


    int width =  max_x - min_x + 1;
    int height = max_y - min_y + 1;
    int depth =  max_z - min_z + 1;


    if (image->getZDim() == 1)
    {
        depth = 1;
        min_z = 0;
        max_z = 0;
    }

    int size_1d = width * height *depth;

    cout << "min: "<< min_x<<" "<< min_y <<" " <<min_z<<endl;
    cout << "max: "<< max_x<<" "<< max_y <<" " <<max_z<<endl;
    cout << "size:" << width<<" x" <<height <<" x"<<depth<<endl;


    vector <double> fg_1d;
    vector <double> bg_1d;

    unsigned char  * roi_1d_visited = new  unsigned char [size_1d];
    int FG = 255;
    int BG = 100;



    for (V3DLONG i = 0; i < size_1d ; i++)
    {
        roi_1d_visited[i] = 0;  //unvisited tag
    }


    vector <double> tubularities;
    for (V3DLONG i = 0; i < neuronSegment.size() ; i++)
    {
        NeuronSWC node = neuronSegment.at(i);
        float r;
        if (node.radius < 0 ){
            r = 1;
        }
        else
        {
            r = node.radius;
        }

        float dilate_radius = dilate_ratio * r;

        V3DLONG xb,xe,yb,ye,zb,ze;

        // for each node
        //label foreground
        xb = boundValue(node.x - r +0.5, 0,image->getXDim()-1 );
        xe = boundValue(node.x + r +0.5, 0,image->getXDim()-1 );
        yb = boundValue(node.y - r +0.5, 0,image->getYDim()-1 );
        ye = boundValue(node.y + r +0.5, 0,image->getYDim()-1 );
        zb = boundValue(node.z - r +0.5, 0,image->getZDim()-1 );
        ze = boundValue(node.z + r +0.5, 0,image->getZDim()-1 );
        for (V3DLONG z = zb; z <= ze; z++)
        {
            for ( V3DLONG y = yb; y <= ye; y++)
            {
                for ( V3DLONG x = xb; x <= xe; x++)
                {
                    V3DLONG index_1d = z * (image->getXDim() * image->getYDim())  + y * image->getXDim() + x;
                    V3DLONG roi_index =  (z - min_z) * (width * height)  + (y - min_y) * width + (x - min_x);
                    if  ( roi_1d_visited[roi_index] != FG )
                    {
                        roi_1d_visited[roi_index] = FG;
                        fg_1d.push_back(double(image->getRawData()[index_1d]));
                    }
                }

            }

        }

        //label background
        xb = boundValue(node.x - r - dilate_radius + 0.5, 0,image->getXDim()-1 );
        xe = boundValue(node.x + r + dilate_radius + 0.5, 0,image->getXDim()-1 );
        yb = boundValue(node.y - r - dilate_radius + 0.5, 0,image->getYDim()-1 );
        ye = boundValue(node.y + r + dilate_radius + 0.5, 0,image->getYDim()-1 );
        zb = boundValue(node.z - r - dilate_radius + 0.5, 0,image->getZDim()-1 );
        ze = boundValue(node.z + r + dilate_radius + 0.5, 0,image->getZDim()-1 );



        for (V3DLONG z = zb; z <= ze; z++)
        {
            for ( V3DLONG y = yb; y <= ye; y++)
            {
                for ( V3DLONG x = xb; x <= xe; x++)
                {
                    V3DLONG index_1d = z * (image->getXDim() * image->getYDim())  + y * image->getXDim() + x;
                    V3DLONG roi_index =  (z - min_z) * (width * height)  + (y - min_y) * width + (x - min_x);
                    if  ( roi_1d_visited[roi_index] == 0)
                    {
                        roi_1d_visited[roi_index] = BG;
                        bg_1d.push_back(double(image->getRawData()[index_1d]));
                    }
                }

            }
        }

        // compute tubularity for each node
        V3DLONG xx = boundValue(node.x, 0,image->getXDim()-1 );
        V3DLONG yy = boundValue(node.y, 0,image->getYDim()-1 );
        V3DLONG zz = boundValue(node.z, 0,image->getZDim()-1 );
        double tubuV = compute_anisotropy_sphere(image->getRawData(), image->getXDim(), image->getYDim(), image->getZDim(), 0, xx,yy,zz, r + dilate_radius);

        tubularities.push_back(tubuV);

    }

   /*for debug purpose only
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *) roi_1d_visited, width, height, depth, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "cropped.result");
    callback.updateImageWindow(newwin);
   */

    // compute metrics
    //remove the top and bottom 5% data to be robust
    cutoff_outliers(fg_1d);
    cutoff_outliers(bg_1d);
    cutoff_outliers(tubularities);

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
        metrics.cnr = fabs(fg_mean - bg_mean)/bg_deviation;
        //double contrast = median(fg_1d) - median(bg_1d);
        //metrics.cnr = fabs(contrast)/bg_deviation;
    }
    else {
        metrics.cnr  = INFINITY;
        cout<<"warning! background deviation is zero"<<endl;
    }

    //average tubularity
     metrics.tubularity = accumulate( tubularities.begin(), tubularities.end(), 0.0 )/ tubularities.size();

    //median tubularity
   // metrics.tubularity = median(tubularities);
    cout<< "Segment "<< ":dy = "<<metrics.dy <<"; fg_mean="<<fg_mean<<";fg_median=" <<median(fg_1d)<<"; bg_mean="<<bg_mean <<";bg_median=" <<median(bg_1d)
        <<"; bg_dev = "<<bg_deviation<<"; cnr = "<<metrics.cnr <<"; tubularity = "<<metrics.tubularity <<"\n"<< endl;

    return metrics;

}

QList<IMAGE_METRICS> intensity_profile(NeuronTree neuronTree, Image4DSimple * image, float dilate_ratio, int flip, int invert, V3DPluginCallback2 &callback)
{


    if(flip > 0)
    {
      flip_y(image);
      cout<<"warning: the image is flipped in Y by default, to be consistent with other image readers, e.g. ImageJ."<<endl;
    }

    if (invert > 0)
    {
        invert_intensity(image);
        cout<<"warning: the image is flipped in Y by default, to be consistent with other image readers, e.g. ImageJ."<<endl;
    }

    QList<IMAGE_METRICS> result_metrics;
    result_metrics.clear();

    if (image->getDatatype() != V3D_UINT8)
    {
        cout << "This plugin only support 8 bit images." <<endl;
        return result_metrics;
    }

    //parse swc, divide into segments
    vector<V3DLONG> segment_id;
    vector<V3DLONG> segment_layer; //not used

    swc2eswc(neuronTree, segment_id, segment_layer);

    V3DLONG num_segments = 0 ;

    QList<NeuronSWC> neuronSWCs =  neuronTree.listNeuron;
    V3DLONG pre_id = segment_id.at(0); // segment id may not in order
    QList<NeuronSWC> neuronSegment;
    for (V3DLONG i = 0 ; i < neuronSWCs.size() ; i++)
    {

        if (segment_id[i] != pre_id)
        {
            if (!neuronSegment.empty())
            {
                cout<<"Segment # :"<<pre_id<<endl;
                IMAGE_METRICS metrics = compute_metrics( image, neuronSegment,dilate_ratio, callback );
                num_segments++;
                result_metrics.push_back(metrics);
            }
            neuronSegment.clear();

            //start a new segment
            pre_id = segment_id[i];
        }

        neuronSegment.push_back( neuronSWCs.at(i) );

    }

    if (!neuronSegment.empty())
    {
        cout<<"Segment # :"<<pre_id<<endl;
        IMAGE_METRICS metrics = compute_metrics( image, neuronSegment,dilate_ratio, callback );
        num_segments++;
        result_metrics.push_back(metrics);
    }

    cout <<"total number of segments = " << num_segments <<endl;
    return result_metrics;
}

void printHelp(const V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("This plugin is used for profiling images with SWC specified ROIs.");
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"This plugin is used for profiling 2D images with SWC specified ROIs.\n";
    cout<<"usage:\n";
    cout<<"v3d -x neuron_image_profiling -f profile_swc -i <inimg_file> <inswc_file> -o <out_file> -p <dilation_ratio>  <flip>"
    <<endl;
    cout<<"inimg_file:\t\t input image file\n";
    cout<<"inswc_file:\t\t input .swc file\n";
    cout<<"out_file:\t\t (not required) output statistics of intensities into a csv file. DEFAUTL: '<inswc_file>.csv'\n";
    cout<<"dilation_radius :\t (not required) the dilation ratio to expand the radius for background ROI extraction\n";
    cout<<"flip in y [0 or 1]\t (not required)\n";
    cout<<"invert intensity [0 or 1]\t (not required, signal should have higher intensties than background, for tubuliarty calculation)"<<endl;
}






