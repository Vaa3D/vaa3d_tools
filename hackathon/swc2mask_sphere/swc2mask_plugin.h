#ifndef __SWC2MASK_PLUGIN_H__
#define __SWC2MASK_PLUGIN_H__

#include <iostream>
#include <vector>

#include <QtGui>
#include <v3d_interface.h>
#include "src/swc2mask.h"

using namespace std;


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
			return raw_split(, prefix, bs0, bs1, bs2);
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

