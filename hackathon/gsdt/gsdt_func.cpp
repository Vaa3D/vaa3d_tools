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

using namespace std;

const QString title = QObject::tr("Grayscale Distance Transformation Plugin");

int split(const char *paras, char ** &args)
{
    int argc = 0;
    int len = strlen(paras);
    int posb[200];
    char * myparas = new char[len];
    strcpy(myparas, paras);
    for(int i = 0; i < len; i++)
    {
        if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
        {
            posb[argc++]=i;
        }
        else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') &&
                (myparas[i] != ' ' && myparas[i] != '\t'))
        {
            posb[argc++] = i;
        }
    }

    args = new char*[argc];
    for(int i = 0; i < argc; i++)
    {
        args[i] = myparas + posb[i];
    }

    for(int i = 0; i < len; i++)
    {
        if(myparas[i]==' ' || myparas[i]=='\t')myparas[i]='\0';
    }
    return argc;
}
int gsdt(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	//TestDialog dialog(callback, parent);

	//if (dialog.exec()!=QDialog::Accepted) return -1;

	//dialog.update();
	//int i = dialog.i;
	//int c = dialog.channel;
	//Image4DSimple *p4DImage = callback.getImage(win_list[i]);
	//if(p4DImage->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	//V3DLONG sz[3];
	//sz[0] = p4DImage->getXDim();
	//sz[1] = p4DImage->getYDim();
	//sz[2] = p4DImage->getZDim();

	//unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);

	//v3dhandle newwin;
	//if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
		//newwin = callback.currentImageWindow();
	//else
		//newwin = callback.newImageWindow();

	//p4DImage->setData(inimg1d, sz[0], sz[1], sz[2], sz[3]);
	//callback.setImage(newwin, p4DImage);
	//callback.setImageName(newwin, QObject::tr("gsdt"));
	//callback.updateImageWindow(newwin);
	return 1;
}

bool gsdt(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to gsdt"<<endl;
	if(input.size() != 2 || output.size() != 1) return false;
	int bkg_thresh = 0, cnn_type = 2;
	vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
	if(paras.size() >= 1) bkg_thresh = atoi(paras.at(0));
	if(paras.size() >= 2) cnn_type = atoi(paras.at(1));
	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
	cout<<"bkg_thresh = "<<bkg_thresh<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

	unsigned char * inimg1d = 0,  * outimg1d = 0;
	float * phi;
	V3DLONG * in_sz;
	int datatype;
	if(!loadImage(inimg_file, inimg1d, in_sz, datatype)) {cerr<<"load image "<<inimg_file<<" error!"<<endl; return 1;}

	fastmarching_dt(inimg1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);

	float min_val = phi[0], max_val = phi[0];
	long tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
	outimg1d = new unsigned char[tol_sz];
	for(long i = 0; i < tol_sz; i++) {if(phi[i] == INF) continue; min_val = MIN(min_val, phi[i]); max_val = MAX(max_val, phi[i]);}
	cout<<"min_val = "<<min_val<<" max_val = "<<max_val<<endl;
	max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;
	for(long i = 0; i < tol_sz; i++)
	{
		if(phi[i] == INF) outimg1d[i] = 0;
		else if(phi[i] ==0) outimg1d[i] = 0;
		else
		{
			outimg1d[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
			outimg1d[i] = MAX(outimg1d[i], 1);
		}
	}
	saveImage(outimg_file, outimg1d, in_sz, datatype);
	
	delete [] phi; phi = 0;
	delete [] inimg1d; inimg1d = 0;
	delete [] outimg1d; outimg1d = 0;

	return true;
}


