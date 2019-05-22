/* SteerableFilter3D.cxx
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

#include "steerablefilter3Dplugin.h"
#include "steerableDetector3D.h"

using namespace std;

#define INF 1E9

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(SteerableFilter3D, SteerableFilter3DPlugin)

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);


const QString title = QObject::tr("Steerable Filter 2D Plugin");
QStringList SteerableFilter3DPlugin::menulist() const
{
    return QStringList() << tr("Steerable Filter 2D") << tr("About");
}

void SteerableFilter3DPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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

QStringList SteerableFilter3DPlugin::funclist() const
{
	return QStringList()
		<<tr("gf")
		<<tr("help");
}


bool SteerableFilter3DPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
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

    SteerableFilter3DDialog dialog(callback, parent);
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
    double zfactor = dialog.zfactor;

    cout<<"void_process Order = "<<order<<endl;
    cout<<"void_process sigma = "<<sigma<<endl;
    cout<<"void_process zfactor = "<<zfactor<<endl;
    cout<<"void_process ch = "<<c<<endl;
    
    // gaussian_filter
     V3DLONG in_sz[4];
     in_sz[0] = nx; in_sz[1] = ny; in_sz[2] = nz; in_sz[3] = sc;

    double * doubleimage = new double [nx*ny*nz];
    cout<<"void_process: doubleimage initialized"<<endl;

    double * extIter = doubleimage;
    for(V3DLONG index = 0; index< nx*ny*nz; index++){
        *(extIter++) = data1d[index];           
    }
    cout<<"void_process: doubleimage copied"<<endl;
          
    Filter filter = Filter(doubleimage, nx, ny, nz, order, sigma, zfactor);
    double *res = filter.getResponse();
    //double **orientation = filter.getOrientation();
    //double *nms = filter.getNMS();


    // Steerable filter Response convert from double to float
    float * outputimageResponse = new float [nx*ny*nz];
    cout<<"void_process outputimage Response initialized"<<endl;

    float * extIter2 = outputimageResponse;
    for(V3DLONG index = 0; index< nx*ny*nz; index++){
        
       // if(res[index]<0)
        //{
        //    *(extIter2++)=0;
        //}
        //else  
        //{
        *(extIter2++) = res[index]/100;     
        //}
        //cout<<"outputimageResponse["<<index<<"]="<<outputimageResponse[index]<<endl;



    }
    cout<<"void_process outputimage Response copied"<<endl;


     // Steerable filter Response display
    const QString titleresponse = QObject::tr("Steerable Filter 3D Response");
    Image4DSimple * new4DImageResponse = new Image4DSimple();
    new4DImageResponse->setData((unsigned char *)outputimageResponse, nx, ny, nz, 1, V3D_FLOAT32);
    cout<<"void_process outputimage Response set data"<<endl;

    v3dhandle newwinResponse = callback.newImageWindow();
    cout<<"void_process v3dhandle newwinResponse"<<endl;

    callback.setImage(newwinResponse, new4DImageResponse);
    cout<<"void_process outputimage Response set image"<<endl;
    
    callback.setImageName(newwinResponse, titleresponse);
    cout<<"void_process outputimage Response set imagename"<<endl;
    
    callback.updateImageWindow(newwinResponse);
    cout<<"void_process outputimage Response update"<<endl;

    /*
    // Steerable filter orientation convert from double to float
    float * outputimageOrientation = new float [nx*ny];
    cout<<"void_process outputimage Response initialized"<<endl;

    float * extIter4 = outputimageOrientation;
    for(V3DLONG index = 0; index< nx*ny; index++){
        *(extIter4++) = orientation[index];           
    }
    cout<<"void_process outputimage Orientation copied"<<endl;


    // Steerable filter Orientation display
    const QString titleOrientation = QObject::tr("Steerable Filter 3D Orientation");
    Image4DSimple * new4DImageOrientation = new Image4DSimple();
    new4DImageOrientation->setData((unsigned char *)outputimageOrientation, nx, ny, nz, 1, V3D_FLOAT32);
    v3dhandle newwinOrientation = callback.newImageWindow();
    callback.setImage(newwinOrientation, new4DImageOrientation);
    callback.setImageName(newwinOrientation, titleOrientation);
    callback.updateImageWindow(newwinOrientation);
    */

/*

    // Steerable filter NMS convert from double to float 
    float * outputimageNMS = new float [nx*ny*nz];
    cout<<"void_process outputimage NMS initialized"<<endl;

    float * extIter3 = outputimageNMS;
    for(V3DLONG index = 0; index< nx*ny*nz; index++){
        *(extIter3++) = nms[index];           
    }
    cout<<"void_process outputimage NMS copied"<<endl;

    // Steerable filter NMS display
    const QString titlenms = QObject::tr("Steerable Filter 3D NMS");
    Image4DSimple * new4DImageNMS = new Image4DSimple();
    new4DImageNMS->setData((unsigned char *)outputimageNMS, nx, ny, nz, 1, V3D_FLOAT32);
    v3dhandle newwinNMS = callback.newImageWindow();
    callback.setImage(newwinNMS, new4DImageNMS);
    callback.setImageName(newwinNMS, titlenms);
    callback.updateImageWindow(newwinNMS);
*/

	return;
}



/*

void steerablefilter3DCore(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    // check # inputs
    if (nrhs < 3 || nrhs > 4)
        mexErrMsgTxt("Required inputs arguments: image, filter order, sigma. Optional: z-anisotropy factor.");
    if (nlhs > 4)
        mexErrMsgTxt("Too many output arguments.");
    
    // check image
    if (!mxIsDouble(prhs[0]) || mxGetNumberOfDimensions(prhs[0]) != 3)
        mexErrMsgTxt("Input must be a 3D array.");
    const mwSize* dims = mxGetDimensions(prhs[0]);
    
    int ny = (int)dims[0]; // reversed: (m,n) -> (y,x)
    int nx = (int)dims[1];
    int nz = (int)dims[2];
    
    int N = nx*ny*nz;
    double* input = mxGetPr(prhs[0]);
    // check for NaNs in input, as these will result in a crash
    for (int i=0;i<N;++i) {
        if (mxIsNaN(input[i])) {
            mexErrMsgTxt("Input image contains NaNs.");
            break;
        }
    }
    
    // check order
    if (!mxIsDouble(prhs[1]) || mxGetNumberOfElements(prhs[1]) != 1 || *mxGetPr(prhs[1])<1 || *mxGetPr(prhs[1])>2)
        mexErrMsgTxt("The filter type 'M' must be 1 (curve detector) or 2 (surface detector).");
    int M = (int) *mxGetPr(prhs[1]);
    
    // check sigma
    if (!mxIsDouble(prhs[2]) || mxGetNumberOfElements(prhs[2]) != 1 || *mxGetPr(prhs[2]) <= 0.0)
        mexErrMsgTxt("Sigma must be a strictly positive scalar value.");
    double sigma = *mxGetPr(prhs[2]);
    
    double zfactor = 1.0;
    if (nrhs==4) {
        if (!mxIsDouble(prhs[3]) || mxGetNumberOfElements(prhs[3]) != 1 || *mxGetPr(prhs[3]) <= 0.0)
            mexErrMsgTxt("z-anisotropy factor must be strictly positive.");
        zfactor = *mxGetPr(prhs[3]);
    }
    
    int L = 2*(int)(3.0*sigma)+1; // support of the Gaussian kernels
    int Lz = 2*(int)(3.0*sigma/zfactor)+1;
    
    if (L>nx || L>ny || Lz>nz) {
        mexPrintf("Sigma must be smaller than %.2f\n", (min(min(nx,ny), nz)-1.0)/8.0);
        mexErrMsgTxt("Sigma value results in filter support that is larger than image.");
    }
    
    double* voxels = new double[N];
    //Convert volume to row-major frames (Matlab uses column-major)
    int nxy = nx*ny;
    int z;
    div_t divRes;
    for (int i=0;i<N;++i) {
        divRes = div(i, nxy);
        z = divRes.quot;
        divRes = div(divRes.rem, ny);
        voxels[divRes.quot+divRes.rem*nx + z*nxy] = input[i];
    }
    
    Filter filter = Filter(voxels, nx, ny, nz, M, sigma, zfactor);
    double *res = filter.getResponse();
    
    // Switch outputs back to column-major format
    if (nlhs > 0) {
        
        for (int i=0;i<N;++i) {
            divRes = div(i, nxy);
            z = divRes.quot;
            divRes = div(divRes.rem, nx);
            voxels[divRes.quot+divRes.rem*ny + z*nxy] = res[i];
        }
        plhs[0] =  mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
        memcpy(mxGetPr(plhs[0]), voxels, N*sizeof(double));
    }
    
    if (nlhs > 1) { // return orientation map: structure theta, fields .x1, .x2, .x3
        double **orientation = filter.getOrientation();
        
        const char *fieldnames[] = {"x1", "x2", "x3"};
        mwSize sdims[2] = {1, 1};
        plhs[1] = mxCreateStructArray(2, sdims, 3, fieldnames);
        // copy each coordinate to its field
        mxArray *x1 = mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
        mxArray *x2 = mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
        mxArray *x3 = mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
        double* px1 = mxGetPr(x1);
        double* px2 = mxGetPr(x2);
        double* px3 = mxGetPr(x3);
        
        for (int i=0;i<N;++i) {
            divRes = div(i, nxy);
            z = divRes.quot;
            divRes = div(divRes.rem, nx);
            px1[divRes.quot+divRes.rem*ny + z*nxy] = orientation[i][0];
            px2[divRes.quot+divRes.rem*ny + z*nxy] = orientation[i][1];
            px3[divRes.quot+divRes.rem*ny + z*nxy] = orientation[i][2];
        }
        mxSetFieldByNumber(plhs[1], 0, 0, x1);
        mxSetFieldByNumber(plhs[1], 0, 1, x2);
        mxSetFieldByNumber(plhs[1], 0, 2, x3);
    }
    
    if (nlhs > 2) { // return NMS
        double *nms = filter.getNMS();
        plhs[2] =  mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
        double *nmsP = mxGetPr(plhs[2]);
        for (int i=0;i<N;++i) {
            divRes = div(i, nxy);
            z = divRes.quot;
            divRes = div(divRes.rem, nx);
            nmsP[divRes.quot+divRes.rem*ny + z*nxy] = nms[i];
        }
    }
    
    // Free memory
    delete[] voxels;
}


// compiled with:
// export DYLD_LIBRARY_PATH=/Applications/MATLAB_R2012a.app/bin/maci64 && g++ -Wall -g -DARRAY_ACCESS_INLINING -I. -L/Applications/MATLAB_R2012a.app/bin/maci64 -I../../mex/include/ -I/Applications/MATLAB_R2012a.app/extern/include steerableDetector3D.cpp -lmx -lmex -lgsl
// tested with:
// valgrind --tool=memcheck --leak-check=full --show-reachable=yes ./a.out 2>&1 | grep steerable

/*int main(void) {
    int nx = 200;
    int ny = 200;
    int nz = 80;
    int N = nx*ny*nz;
    double* voxels = new double[N];
    for (int i=0;i<N;++i) {
        voxels[i] = rand();
    }
    
    Filter filter = Filter(voxels, nx, ny, nz, 1, 3.0);
    
    delete[] voxels;
}*/


