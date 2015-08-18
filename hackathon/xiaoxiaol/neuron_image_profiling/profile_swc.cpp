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

static void cutoff_outliers(vector<double> & x)
{
    //remove the top and bottom 10% data, to be more robust
    sort(x.begin(), x.end());
    int num_to_remove = x.size()*0.1;

    // erase the top and bottom N elements:
    x.erase( x.begin(), x.begin()+ num_to_remove);
    x.erase( x.end()-num_to_remove, x.end());
}

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

static double mean(vector<double> x)
{
     double x_mean  = accumulate( x.begin(), x.end(), 0.0 )/ x.size();
     return x_mean;
}


static double standard_dev(vector<double> x)
{
    double x_mean  = mean(x);

    double sum2 = 0;
    for ( V3DLONG i = 0; i < x.size(); i++ )
    {
        sum2 += pow(x[i]-x_mean,2);
    }
    double x_std = sqrt(sum2/x.size());
    return x_std;
}

static ENSEMBLE_METRICS stats_ensemble(QList<IMAGE_METRICS> result_metrics)
{  // aggregate all segment stats into one stats
    ENSEMBLE_METRICS stats; // average stats over all segments
    vector <double> cnrs;
    vector <double> dys;
    vector <double> tubus;
    vector <double> bgs;
    vector <double> fgs;



    for (int i  = 0; i < result_metrics.size() ; i++)
    {
        cnrs.push_back( result_metrics[i].cnr);
        dys.push_back(result_metrics[i].dy);
        tubus.push_back(result_metrics[i].tubularity_mean);
        bgs.push_back(result_metrics[i].bg_mean);
        fgs.push_back(result_metrics[i].fg_mean);
    }

    cutoff_outliers(cnrs);
    cutoff_outliers(dys);
    cutoff_outliers(tubus);
    cutoff_outliers(bgs);
    cutoff_outliers(fgs);


    stats.mean_cnr = mean(cnrs);
    stats.mean_dy = mean(dys);
    stats.mean_tubularity = mean(tubus);
    stats.mean_bg = mean(bgs);
    stats.mean_fg = mean(fgs);


    stats.std_bg = standard_dev(bgs);
    stats.std_fg = standard_dev(fgs);
    stats.std_dy = standard_dev(dys);
    stats.std_cnr = standard_dev(cnrs);
    stats.std_tubularity = standard_dev(tubus);

    return stats;

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
        stream<< "segment_type,dynamic_range,cnr,snr,tubularity_mean,tubularity_std,fg_mean,fg_std,bg_mean,bg_std"<<"\n";

        for (int i  = 0; i < result_metrics.size() ; i++)
        {
            stream << result_metrics[i].type       <<","
                   << result_metrics[i].dy         <<","
                   << result_metrics[i].cnr        <<","
                   << result_metrics[i].snr        <<","
                   << result_metrics[i].tubularity_mean <<","
                   << result_metrics[i].tubularity_std <<","
                   << result_metrics[i].fg_mean    <<","
                   << result_metrics[i].fg_std     <<","
                   << result_metrics[i].bg_mean    <<","
                   << result_metrics[i].bg_std     <<"\n";
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
    IMAGE_METRICS m_stats = result_metrics[0];


    QString disp_text = "";
    disp_text += "Contrast-to-Background Ratio = " + QString::number(m_stats.cnr) + ";\n";
    disp_text += "Dynamic Range = " + QString::number(m_stats.dy) + ";\n";
    disp_text += "Mean FG Intensity = " + QString::number (m_stats.fg_mean)  + ", STD = "    + QString::number(m_stats.fg_std) + ";\n";
    disp_text += "Mean BG Intensity = " + QString::number (m_stats.bg_mean)  + ", STD = "    + QString::number(m_stats.bg_std) + ";\n";
    disp_text += "Mean Tubularity = " + QString::number(m_stats.tubularity_mean) + ", STD = " + QString::number(m_stats.tubularity_std) + ".\n";


    disp_text += "\n Segment type-specific screening metrics are exported in: \n "+ output_csv_file +"\n";


    /*
    ENSEMBLE_METRICS m_stats = stats_ensemble(result_metrics);


    //display metrics to the msg window
    QString disp_text = "";
    disp_text += "Mean FG Intensity = " + QString::number (m_stats.mean_fg)  + ", STD = "    + QString::number(m_stats.std_fg) + ";\n";
    disp_text += "Mean BG Intensity = " + QString::number(m_stats.mean_bg )  + ", STD = "    + QString::number(m_stats.std_bg) + ";\n";
    disp_text += "Mean Contrast-to-Background Ratio = " + QString::number(m_stats.mean_cnr)  + ", STD = "    + QString::number(m_stats.std_cnr) + ";\n";
    disp_text += "Mean Dynamic Range = " + QString::number(m_stats.mean_dy)   + ", STD = "    + QString::number(m_stats.std_dy) + ";\n";
    disp_text += "Mean Tubularity = " + QString::number(m_stats.mean_tubularity) + ", STD = " + QString::number(m_stats.std_tubularity) + ".\n";


    disp_text += "\n  Screening Metrics of each segment can be found in: \n "+ output_csv_file +"\n";
    */


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
    IMAGE_METRICS m_stats = result_metrics[0];

    /*
    ENSEMBLE_METRICS m_stats = stats_ensemble(result_metrics);
    */

    cout << "Overall Contrast-to-Background Ratio:" << m_stats.cnr << "\n"
         << "Overall Dynamic Range:"<< m_stats.dy << "\n"
         << "Mean BG Intensity:" << m_stats.bg_mean << ", with std = "    << m_stats.bg_std << "\n"
         << "Mean FG Intensity:" << m_stats.fg_mean << ", with std = "    << m_stats.fg_std << "\n"
         << "Mean Tubularity:"   << m_stats.tubularity_mean << ", with std = " << m_stats.tubularity_std << "\n"
         << endl;

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
    cout << "Segment type-specific screening metrics are exported in:  " << output_csv_file.toStdString() <<endl;

	return true;

}



IMAGE_METRICS   compute_metrics(Image4DSimple *image,  QList<NeuronSWC> neuronSegment, float dilate_ratio, V3DPluginCallback2 &callback)
{

    IMAGE_METRICS metrics;
    metrics.type = neuronSegment.at(0).type;
    metrics.cnr = 0.0;
    metrics.snr = 0.0;
    metrics.dy = 0.0;
    metrics.tubularity_mean = 0.0;
    metrics.fg_mean = 0.0;
    metrics.bg_mean = 0.0;
    metrics.tubularity_std = 0.0;
    metrics.fg_std = 0.0;
    metrics.bg_std = 0.0;

    V3DLONG min_x = image->getXDim() , min_y = image->getYDim() ,  min_z = image->getZDim() , max_x = 0, max_y = 0, max_z= 0;

    //get the bounding box of ROI
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

    //cout << "min: "<< min_x<<" "<< min_y <<" " <<min_z<<endl;
    //cout << "max: "<< max_x<<" "<< max_y <<" " <<max_z<<endl;
    //cout << "size:" << width<<" x" <<height <<" x"<<depth<<endl;


    vector <double> fg_1d;
    vector <double> bg_1d;

    unsigned char  * roi_1d_visited = new  unsigned char [size_1d];
    int FG = 255;
    int BG = 100;
    int FUZZY = 10;



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
        xb = boundValue(node.x - r + 0.5, 0,image->getXDim()-1 );
        xe = boundValue(node.x + r + 0.5, 0,image->getXDim()-1 );
        yb = boundValue(node.y - r + 0.5, 0,image->getYDim()-1 );
        ye = boundValue(node.y + r + 0.5, 0,image->getYDim()-1 );
        zb = boundValue(node.z - r + 0.5, 0,image->getZDim()-1 );
        ze = boundValue(node.z + r + 0.5, 0,image->getZDim()-1 );
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
                    }
                }

            }

        }

        //label fuzzy region (between foreground and background, to avoid including spines, and tolerate underestimated neuron radius
        double fuzzy_r_ratio = 1.5;  //0.5 r
        xb = boundValue(node.x - fuzzy_r_ratio*r + 0.5, 0,image->getXDim()-1 );
        xe = boundValue(node.x + fuzzy_r_ratio*r + 0.5, 0,image->getXDim()-1 );
        yb = boundValue(node.y - fuzzy_r_ratio*r + 0.5, 0,image->getYDim()-1 );
        ye = boundValue(node.y + fuzzy_r_ratio*r + 0.5, 0,image->getYDim()-1 );
        zb = boundValue(node.z - fuzzy_r_ratio*r + 0.5, 0,image->getZDim()-1 );
        ze = boundValue(node.z + fuzzy_r_ratio*r + 0.5, 0,image->getZDim()-1 );
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
                        roi_1d_visited[roi_index] = FUZZY;
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
                    if  ( roi_1d_visited[roi_index] != FG  &&  roi_1d_visited[roi_index] != FUZZY )
                    {
                        roi_1d_visited[roi_index] = BG;
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

    //collect labled pixel data into bg and fg vectors
    for (V3DLONG i = 0; i < size_1d ; i++)
    {
        V3DLONG roi_index = i;

        V3DLONG z = roi_index/(width*height);
        V3DLONG y = (roi_index - z* (width*height) )/width;
        V3DLONG x = roi_index - z* (width*height)  - y*width;
        z += min_z;
        y += min_y;
        x += min_x;
        V3DLONG index_1d =  z * (image->getXDim() * image->getYDim())  + y * image->getXDim() + x;

        if (roi_1d_visited[roi_index] == FG ){
            fg_1d.push_back(double(image->getRawData()[index_1d]));
        }
        if (roi_1d_visited[roi_index] == BG ){
            bg_1d.push_back(double(image->getRawData()[index_1d]));
        }

    }
    cout << "number of fg pixels:" << fg_1d.size() <<endl;
    cout << "number of bg pixels:" << bg_1d.size() <<endl;

   /*for debug purpose only
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *) roi_1d_visited, width, height, depth, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "cropped.result");
    callback.updateImageWindow(newwin);
*/

    // compute metrics
    // remove the top and bottom 5% data to be robust
    cutoff_outliers(fg_1d);
    cutoff_outliers(bg_1d);
    cutoff_outliers(tubularities);

    double max_fg =  *( max_element(fg_1d.begin(), fg_1d.end()));
    double min_fg =  * (min_element(fg_1d.begin(), fg_1d.end()));
    metrics.dy = fabs(max_fg - min_fg);

    double bg_mean  = mean(bg_1d);
    double fg_mean  = mean(fg_1d);


    metrics.bg_std = standard_dev(bg_1d);
    metrics.fg_std = standard_dev(fg_1d);
    metrics.tubularity_std  = standard_dev(tubularities);

    if (metrics.bg_std != 0.0){
        metrics.cnr = fabs(fg_mean - bg_mean)/metrics.bg_std;
        metrics.snr = fg_mean/metrics.bg_std;
    }
    else {
        metrics.cnr  = INFINITY;
        metrics.snr = INFINITY;
        cout<<"warning! background deviation is zero"<<endl;
    }

    //average tubularity
     metrics.tubularity_mean = mean(tubularities);
     metrics.bg_mean = bg_mean;
     metrics.fg_mean = fg_mean;

    /*
     cout<< " dy = "<<metrics.dy
        <<"; fg_mean ="<< fg_mean
        <<"; fg_median =" << median(fg_1d)
        <<"; bg_mean ="<< bg_mean
        <<"; bg_median =" << median(bg_1d)
        <<"; fg_dev = "<<fg_std
        <<"; bg_dev = "<< bg_std
        <<"; snr = "<< metrics.snr
        <<"; cnr = "<< metrics.cnr
        <<"; tubularity = "<< metrics.tubularity <<"\n"<< endl;
     */
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


    QList<NeuronSWC> neuronSWCs =  neuronTree.listNeuron;

    QList<IMAGE_METRICS> result_metrics;
    // all types
    IMAGE_METRICS metrics = compute_metrics( image, neuronSWCs,dilate_ratio, callback );
    metrics.type = -1;  //all types
    result_metrics.push_back(metrics);

    // pool neuron nodes by segment types

    QList<NeuronSWC> neuronSWC_sameType;
    QList <QList<NeuronSWC> > neuronSWC_lists;
    std::map<int,int> mapTypeToId;
    int pre_type = neuronSWCs[0].type;
    int count = 0;
    int j = pre_type;
    for (V3DLONG i = 0 ; i < neuronSWCs.size() ; i++)
    {
        j = neuronSWCs[i].type;

        if (j != pre_type)
        {// not in map yet, add into map
            if (!neuronSWC_sameType.isEmpty()){
                if ( mapTypeToId.count(j) == 0   )
                {
                    neuronSWC_lists.push_back(neuronSWC_sameType);
                    mapTypeToId[pre_type] = count;
                    //cout<<"map "<<pre_type<<" to "<<count<<endl;
                    count++;

                }
                else
                {
                    int jj = mapTypeToId[j];

                    neuronSWC_lists[jj].append(neuronSWC_sameType);;
                }
                neuronSWC_sameType.clear();
            }

            pre_type = j;

        }

        neuronSWC_sameType.push_back(neuronSWCs.at(i));

    }


    //at the end
    if (!neuronSWC_sameType.isEmpty()){
        if ( mapTypeToId.count(j) == 0)
        {
            neuronSWC_lists.push_back(neuronSWC_sameType);
            mapTypeToId[j] = count;
           // cout<<"last: map "<<j<<" to "<<count<<endl;
            count++ ;
        }
        else
        {
            int jj = mapTypeToId[j];
            neuronSWC_lists[jj].append(neuronSWC_sameType);;
        }
    }

    //collect metrics
    cout<< "Profile " << count<< " different segment types" <<endl;
    for (j = 0; j < count; j++)
    {
        if (!neuronSWC_lists[j].isEmpty() )
        {
            IMAGE_METRICS metrics = compute_metrics( image, neuronSWC_lists[j] ,dilate_ratio, callback );
            result_metrics.push_back(metrics);
        }

    }



     /*  Segment by segment stats is not really necessary
    //profile segment by segment
    //parse swc, divide into segments
    vector<V3DLONG> segment_id;
    vector<V3DLONG> segment_layer; //not used

    swc2eswc(neuronTree, segment_id, segment_layer);

    V3DLONG num_segments = 0 ;


    V3DLONG pre_id = segment_id.at(0); // segment id may not in order
    QList<NeuronSWC> neuronSegment;
    for (V3DLONG i = 0 ; i < neuronSWCs.size() ; i++)
    {
        if (segment_id[i] != pre_id)
        {
            if (!neuronSegment.empty())
            {
                //cout<<"Segment # :"<<pre_id<<endl;
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
        //cout<<"Segment # :"<<pre_id<<endl;
        IMAGE_METRICS metrics = compute_metrics( image, neuronSegment,dilate_ratio, callback );
        num_segments++;
        result_metrics.push_back(metrics);
    }

    cout <<"total number of segments = " << num_segments <<endl;
     */

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






