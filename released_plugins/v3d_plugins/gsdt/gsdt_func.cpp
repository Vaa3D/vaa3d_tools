/* gsdt_func.cpp
 * Perform distance transformation on grayscale image.
 * 2012-03-02 : by Hang Xiao
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "gsdt_func.h"
#include <vector>
#include <iostream>

#include "stackutil.h"
#include "fastmarching_dt.h"
#include "common_dialog.h"

using namespace std;

const QString title = QObject::tr("Grayscale Distance Transformation Plugin");

bool gsdt(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	v3dhandle curwin = callback.currentImageWindow();
	Image4DSimple * p4DImage = callback.getImage(curwin);
    if (!p4DImage || !p4DImage->valid())
    {
        v3d_msg("The image is not valid. Do nothing.");
        return false;
    }

	V3DLONG sz0 = p4DImage->getXDim();
	V3DLONG sz1 = p4DImage->getYDim();
	V3DLONG sz2 = p4DImage->getZDim();
	V3DLONG sz3 = p4DImage->getCDim();

	vector<string> items;
	items.push_back("Background Threshold (0 ~ 255)");
	items.push_back("Connection Type (1 ~ 3)");
	items.push_back("Channel (0 ~ )");
	items.push_back("Z_thickness");
	CommonDialog dialog(items);
	dialog.setWindowTitle(title);
	if(dialog.exec() != QDialog::Accepted) return 0;

	int bkg_thresh = 0, cnn_type = 2, channel = 0, z_thickness = 1.0;
	dialog.get_num("Background Threshold (0 ~ 255)", bkg_thresh);
	dialog.get_num("Connection Type (1 ~ 3)", cnn_type);
	dialog.get_num("Channel (0 ~ )", channel);
	dialog.get_num("Z_thickness", z_thickness);
	if(bkg_thresh < 0) bkg_thresh = 0;
	if(z_thickness == 0.0) z_thickness = 1.0;
	if(cnn_type < 1 || cnn_type > 3 || channel < 0 || channel >= sz3)
	{
		v3d_msg(QObject::tr("Connection type or channel value is out of range").arg(sz3-1));
        return false;
	}

	cout<<"bkg_thresh = "<<bkg_thresh<<endl;
	cout<<"cnn_type = "<<cnn_type<<endl;
	cout<<"channel = "<<channel<<endl;
	cout<<"z_thickness = "<<z_thickness<<endl;

	unsigned char * inimg1d = p4DImage->getRawDataAtChannel(channel);
    float * phi = 0;

    switch(p4DImage->getDatatype())
    {
    case V3D_UINT8:
        fastmarching_dt(inimg1d, phi, sz0, sz1, sz2, cnn_type, bkg_thresh, z_thickness);
        break;
    case V3D_UINT16:
        fastmarching_dt((unsigned short int *)inimg1d, phi, sz0, sz1, sz2, cnn_type, bkg_thresh, z_thickness);
        break;
    case V3D_FLOAT32:
        fastmarching_dt((float *)inimg1d, phi, sz0, sz1, sz2, cnn_type, bkg_thresh, z_thickness);
        break;
    default:
        v3d_msg("You should have never seen this warning in GSDT.");
        return false;
    }

	float min_val = phi[0], max_val = phi[0];
    V3DLONG tol_sz = sz0 * sz1 * sz2;

	unsigned char * outimg1d = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) {if(phi[i] == INF) continue; min_val = MIN(min_val, phi[i]); max_val = MAX(max_val, phi[i]);}
	cout<<"min_val = "<<min_val<<" max_val = "<<max_val<<endl;
	max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;
    for(V3DLONG i = 0; i < tol_sz; i++)
	{
		if(phi[i] == INF) outimg1d[i] = 0;
		else if(phi[i] ==0) outimg1d[i] = 0;
		else
		{
			outimg1d[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
			outimg1d[i] = MAX(outimg1d[i], 1);
		}
	}
	delete [] phi; phi = 0;

	Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData(outimg1d, sz0, sz1, sz2, 1, V3D_UINT8);
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, new4DImage);
	callback.setImageName(newwin, title);
	callback.updateImageWindow(newwin);

    return true;
}

bool gsdt(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to gsdt!"<<endl;
	if(input.size() != 2 || output.size() != 1) return false;
	int bkg_thresh = 0, cnn_type = 2, channel = 0, z_thickness = 1.0;
	vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
	if(paras.size() >= 1) bkg_thresh = atoi(paras.at(0));
	if(paras.size() >= 2) cnn_type = atoi(paras.at(1));
    if(paras.size() >= 3) channel = atoi(paras.at(2));
	if(paras.size() >= 4) z_thickness = atof(paras.at(3));

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
	cout<<"bkg_thresh = "<<bkg_thresh<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;
	cout<<"channel = "<<channel<<endl;
	cout<<"z_thickness = "<<z_thickness<<endl;

	unsigned char * inimg1d = 0,  * outimg1d = 0;
    float * phi = 0;
	V3DLONG * in_sz = 0;
	int datatype;
	if(!loadImage(inimg_file, inimg1d, in_sz, datatype, channel)) {cerr<<"load image "<<inimg_file<<" error!"<<endl; return 1;}

     if(datatype == 1)
     {
          if(!fastmarching_dt(inimg1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh, z_thickness)) return false;
     }
     else if(datatype == 2)
     {
          if(!fastmarching_dt((unsigned short int*)inimg1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh, z_thickness)) return false;
     }
     else if(datatype == 4)
     {
          if(!fastmarching_dt((float*)inimg1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh, z_thickness)) return false;
     }
	float min_val = phi[0], max_val = phi[0];
    V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
	outimg1d = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) {if(phi[i] == INF) continue; min_val = MIN(min_val, phi[i]); max_val = MAX(max_val, phi[i]);}
	cout<<"min_val = "<<min_val<<" max_val = "<<max_val<<endl;
	max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;
    for(V3DLONG i = 0; i < tol_sz; i++)
	{
		if(phi[i] == INF) outimg1d[i] = 0;
		else if(phi[i] ==0) outimg1d[i] = 0;
		else
		{
			outimg1d[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
			outimg1d[i] = MAX(outimg1d[i], 1);
		}
	}
     in_sz[3]=1;
	saveImage(outimg_file, outimg1d, in_sz, 1);

	delete [] phi; phi = 0;
	delete [] inimg1d; inimg1d = 0;
	delete [] outimg1d; outimg1d = 0;
	return true;
}


