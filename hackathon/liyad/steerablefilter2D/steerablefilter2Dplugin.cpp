/* steerablefilter2D.cxx
 * 2009-06-03: create this program by Yang Yu
 * 2009-08-14: change into plugin by Yang Yu
 */

// Adapted and upgraded to V3DPluginInterface2_1 by Jianlong Zhou, 2012-04-05
// add dofunc() by Jianlong Zhou, 2012-04-08

#include <QtGui>
#include <v3d_interface.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string> //memcpy

#include "v3d_message.h"
#include "stackutil.h"

#include "steerablefilter2Dplugin.h"
#include "steerableDetector.h"

using namespace std;

#define INF 1E9

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(steerablefilter2D, SteerableFilter2DPlugin)

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);


const QString title = QObject::tr("Steerable Filter 2D Plugin");
QStringList SteerableFilter2DPlugin::menulist() const
{
    return QStringList() << tr("Steerable Filter 2D") << tr("About");
}

void SteerableFilter2DPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Steerable Filter 2D"))
	{
		processImage(callback,parent);
	}
	else if (menu_name == tr("About"))
	{
		v3d_msg("Gaussian filter.");
	}
}

QStringList SteerableFilter2DPlugin::funclist() const
{
	return QStringList()
		<<tr("gf")
		<<tr("help");
}


bool SteerableFilter2DPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("gf"))
	{
        return processImage(callback, input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x gaussian -f gf -i <inimg_file> -o <outimg_file> -p <wx> <wy> <wz> <channel> <sigma>"<<endl;
		cout<<endl;
		cout<<"worder      order of Steerable Filter"<<endl;
		cout<<"channel     the input channel value, default 1 and start from 1"<<endl;
		cout<<"sigma       filter sigma, default 1.0"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x gaussian -f gf -i input.raw -o output.raw -p 3 3 3 1 1.0"<<endl;
		cout<<endl;
		return true;
	}
}

bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Steerable filter"<<endl;
	if (output.size() != 1) return false;

	unsigned int order=2, c=1;
     float sigma = 1.0;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) order = atoi(paras.at(0));
          if(paras.size() >= 2) c = atoi(paras.at(1));
          if(paras.size() >= 3) sigma = atof(paras.at(2));
	}

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
	cout<<"bool_processOrder = "<<order<<endl;
    cout<<"bool_processc = "<<c<<endl;
    cout<<"bool_processsigma = "<<sigma<<endl;
	cout<<"bool_processinimg_file = "<<inimg_file<<endl;
	cout<<"bool_processoutimg_file = "<<outimg_file<<endl;

     double sigma_s2 = 0.5/(sigma*sigma);

    Image4DSimple *inimg = callback.loadImage(inimg_file);
    unsigned char* data1d = inimg->getRawData();
    
    if (!inimg || !inimg->valid())
    {
        v3d_msg("Fail to open the image file.", 0);
        return false;
    }

     if(c > inimg->getCDim())// check the input channel number range
     {
          v3d_msg("The input channel number is out of real channel range.\n", 0 );
          return false;
     }

	//input
    // float* outimg = 0; //no need to delete it later as the Image4DSimple variable "outimg" will do the job
     double* response = 0;
     double* orientation = 0;
     double* nms = 0;
 
     V3DLONG in_sz[4];
     in_sz[0] = inimg->getXDim();
     in_sz[1] = inimg->getYDim();
     in_sz[2] = inimg->getZDim();
     in_sz[3] = inimg->getCDim();

    int nx=in_sz[0];
    int ny=in_sz[1];
    
    double* doubleimage = new double[nx*ny];
    cout<<"bool_process: doubleimage initialized"<<endl;
	
    double * extIter = doubleimage;
    for(V3DLONG index = 0; index< nx*ny; index++){
        *(extIter++) = data1d[index]+20;           
    }
     //switch (inimg->getDatatype())
     //{
        //  case V3D_UINT8: 
    steerablefilter2Dcore(doubleimage, in_sz, order, sigma, response, orientation, nms); //break;
         // case V3D_UINT16: steerablefilter2Dcore((unsigned short int*)(inimg->getRawData()), in_sz, order, c, sigma, response,orientation,nms); break;
          //case V3D_FLOAT32: steerablefilter2Dcore((float *)(inimg->getRawData()), in_sz, order, c, sigma, response,orientation,nms); break;
      //    default:
    //           v3d_msg("Invalid datatype in steerable fileter.", 0);
        //       if (inimg) {delete inimg; inimg=0;}
          //     return false;
     //}

     // save image
     Image4DSimple outimg1;
     outimg1.setData((unsigned char *)response, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);

     callback.saveImage(&outimg1, outimg_file);

     if(inimg) {delete inimg; inimg =0;}

     return true;
}


void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d = p4DImage->getRawData();
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

     V3DLONG nx = p4DImage->getXDim();
     V3DLONG ny = p4DImage->getYDim();
     V3DLONG nz = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();

    cout<<"void_process nx = "<<nx<<endl;
    cout<<"void_process ny = "<<ny<<endl;
    cout<<"void_process nz = "<<nz<<endl;
    cout<<"void_process sc = "<<sc<<endl;

     //add input dialog

    SteerableFilter2DDialog dialog(callback, parent);
    if (!dialog.image)
        return;

    if (dialog.exec()!=QDialog::Accepted)
        return;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return;
    ROIList pRoiList = dialog.pRoiList;

    int order = dialog.order;
    int c = dialog.ch;
    double sigma = dialog.sigma;

    cout<<"void_process Order = "<<order<<endl;
    cout<<"void_process sigma = "<<sigma<<endl;
    cout<<"void_process ch = "<<c<<endl;
    
    // gaussian_filter
     V3DLONG in_sz[4];
     in_sz[0] = nx; in_sz[1] = ny; in_sz[2] = nz; in_sz[3] = sc;

    double* response = 0;
    double* orientation = 0;
    double* nms = 0;
    
    double * doubleimage = new double [nx*ny];
    cout<<"void_process: doubleimage initialized"<<endl;

    double * extIter = doubleimage;
    for(V3DLONG index = 0; index< nx*ny; index++){
        *(extIter++) = data1d[index]+20;           
    }
          
    
	//switch (p4DImage->getDatatype())
    //{
      //  case V3D_UINT8: 
    steerablefilter2Dcore(doubleimage, in_sz, order, sigma,response, orientation, nms);// break;
       // case V3D_UINT16: steerablefilter2Dcore((unsigned short int *)data1d,in_sz, M, sigma,response,orientation,nms); break;
       // case V3D_FLOAT32: steerablefilter2Dcore((float *)data1d, in_sz, M, sigma,response,orientation,nms);break;
       // default: v3d_msg("Invalid data type. Do nothing."); return;
    //}

    for(V3DLONG index = 0; index< nx*ny; index++){
        cout<<"index="<<index<<"response[index]: "<<response[index]<<endl;         
    }

    float * outputimage = new float [nx*ny];
    cout<<"void_process outputimage initialized"<<endl;

    float * extIter2 = outputimage;
    for(V3DLONG index = 0; index< nx*ny; index++){
        *(extIter2++) = response[index];           
    }
    cout<<"void_process outputimage copied"<<endl;


     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outputimage, nx, ny, nz, 1, V3D_FLOAT32);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, title);
     callback.updateImageWindow(newwin);
	return;
}
