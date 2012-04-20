#ifndef __SWC2MASK_PLUGIN_H__
#define __SWC2MASK_PLUGIN_H__

#include <iostream>
#include <vector>

#include <QtGui>
#include <v3d_interface.h>
#include "stackutil.h"
#include "src/swc2mask.h"
#include "swc_convert.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
//#include "vaa3d_neurontoolbox_para.h"

using namespace std;

static string basename(string para)
{
	int pos1 = para.find_last_of("/");
	int pos2 = para.find_last_of(".");
	if(pos1 == string::npos) pos1 = -1;
	if(pos2 == string::npos) pos2 = para.size();
	return para.substr(pos1+1, pos2 - pos1 -1);
}
class SWC2MaskPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const
	{
		return QStringList()
			<< tr("swc2mask")
			<< tr("about");
	}

	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
	{
		if(menu_name == "swc2mask")
		{
		}
		else if(menu_name == "about")
		{
			QMessageBox::information(0, "SWC2Mask Plugin", \
					QObject::tr("This is swc2mask plugin"));
		}
	}

	QStringList funclist() const 
	{
		return QStringList() 
			<< tr("swc2mask")
			<< tr("help");
	}

	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
	{
		if(func_name == "swc2mask")
		{
			vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
			vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
			vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0; 

			vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
			vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
			vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

			if(infiles.size() != 1) return false;
			if(paras.size() != 0 && paras.size() != 3) return false;

			string inswc_file = infiles[0];
			vector<MyMarker*> inswc = readSWC_file(inswc_file);
			string outimg_file = outfiles.empty() ? basename(inswc_file) + "_out.raw" : outfiles[0];
			V3DLONG sz0 = 0, sz1 = 0, sz2 = 0;
			if(paras.empty())
			{
				MyMarker * marker = inswc[0];
				V3DLONG x = marker->x + 0.5;
				V3DLONG y = marker->y + 0.5;
				V3DLONG z = marker->z + 0.5;
				V3DLONG mx = x, my = y, mz = z, Mx = x, My = y, Mz = z;
				for(int i = 1; i < inswc.size(); i++)
				{
					marker = inswc[i];
					x = marker->x + 0.5;
					y = marker->y + 0.5;
					z = marker->z + 0.5;
					mx = MIN(x, mx);
					my = MIN(y, my);
					mz = MIN(z, mz);
					Mx = MAX(x, Mx);
					My = MAX(y, My);
					Mz = MAX(z, Mz);
				}
				sz0 = Mx - mx + 1;
				sz1 = My - my + 1;
				sz2 = Mz - mz + 1;
				for(int i = 0; i < inswc.size(); i++)
				{
					marker = inswc[i];
					x = marker->x + 0.5;
					y = marker->y + 0.5;
					z = marker->z + 0.5;
					marker->x = x - mx;
					marker->y = y - my;
					marker->z = z - mz;
				}
			}
			else
			{
				sz0 = atoi(paras[0]);
				sz1 = atoi(paras[1]);
				sz2 = atoi(paras[2]);
			}
			cout<<"size : "<<sz0<<"x"<<sz1<<"x"<<sz2<<endl;
			
			unsigned char * outimg1d = 0;
			if(!swc2mask(outimg1d, inswc, sz0, sz1, sz2)) return false;
			V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};
			if(!saveImage(outimg_file.c_str(), outimg1d, out_sz, V3D_UINT8))
			{
				cerr<<"Unable to save image to file "<<outimg_file<<endl; 
				return false;
			}
			return true;
		}
		else if(func_name == "TOOLBOXswc2mask")
		{
			vaa3d_neurontoolbox_paras * toolbox_paras = (vaa3d_neurontoolbox_paras *)input.at(0).p;
			NeuronTree nt = toolbox_paras->nt;
			vector<MyMarker*> inswc = swc_convert(nt);

			string outimg_file = basename(nt.file.toStdString())+"_out.raw";
			cout<<"outimg_file = "<<outimg_file<<endl;

			V3DLONG sz0 = 0, sz1 = 0, sz2 = 0;
			v3dhandle curwin = callback.currentImageWindow();
			if(curwin == 0)
			{
				MyMarker * marker = inswc[0];
				V3DLONG x = marker->x + 0.5;
				V3DLONG y = marker->y + 0.5;
				V3DLONG z = marker->z + 0.5;
				V3DLONG mx = x, my = y, mz = z, Mx = x, My = y, Mz = z;
				V3DLONG margin = 0;
				for(int i = 1; i < inswc.size(); i++)
				{
					marker = inswc[i];
					x = marker->x + 0.5;
					y = marker->y + 0.5;
					z = marker->z + 0.5;
					mx = MIN(x, mx);
					my = MIN(y, my);
					mz = MIN(z, mz);
					Mx = MAX(x, Mx);
					My = MAX(y, My);
					Mz = MAX(z, Mz);
					margin = MAX(margin, (V3DLONG)(marker->radius+0.5));
				}
				mx -= margin;
				my -= margin;
				mz -= margin;
				Mx += margin;
				My += margin;
				Mz += margin;

				sz0 = Mx - mx + 1;
				sz1 = My - my + 1;
				sz2 = Mz - mz + 1;
				for(int i = 0; i < inswc.size(); i++)
				{
					marker = inswc[i];
					x = marker->x + 0.5;
					y = marker->y + 0.5;
					z = marker->z + 0.5;
					marker->x = x - mx;
					marker->y = y - my;
					marker->z = z - mz;
				}
			}
			else
			{
				Image4DSimple * p4dImage = callback.getImage(curwin);
				sz0 = p4dImage->getXDim();
				sz1 = p4dImage->getYDim();
				sz2 = p4dImage->getZDim();
			}
			cout<<"size : "<<sz0<<"x"<<sz1<<"x"<<sz2<<endl;
			
			unsigned char * outimg1d = 0;
			if(!swc2mask(outimg1d, inswc, sz0, sz1, sz2)) return false;
			/*V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};
			if(!saveImage(outimg_file.c_str(), outimg1d, out_sz, V3D_UINT8))
			{
				cerr<<"Unable to save image to file "<<outimg_file<<endl; 
				return false;
			}*/
			v3dhandle newwin = callback.newImageWindow();
			Image4DSimple * new4dImage = new Image4DSimple();
			//NeuronTree new_nt = swc_convert(inswc);
			new4dImage->setData(outimg1d, sz0, sz1, sz2, 1, V3D_UINT8);
			//callback.setSWC(newwin, new_nt);
			callback.setImage(newwin, new4dImage);
			callback.updateImageWindow(newwin);
			callback.open3DWindow(newwin);
			return true;
		}
		else if(func_name == "help")
		{
			cout<<"Usage : v3d -x swc2mask -f swc2mask -i <swc_file> [-p <sz0> <sz1> <sz2>] [-o <outimg_file>]"<<endl;
			cout<<endl;
			cout<<"Produce mask image from swc structure. The outimg size will be the bounding box of swc structure or provided size."<<endl;
		}
		return false;
	}
};

Q_EXPORT_PLUGIN2(swc2mask, SWC2MaskPlugin);

#endif

