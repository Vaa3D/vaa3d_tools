
//by Hanchuan Peng
//2009-08-14

// upgrade to Plugin interface 2.1 and add dofunc() interface by Jianlong Zhou 2012-04-18.


#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "canvas_eraser.h"
#include "stackutil.h"

using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(canvas_eraser, CanvasEraserPlugin)

void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int flag);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);



QStringList CanvasEraserPlugin::menulist() const
{
     return QStringList()
          << tr("Change canvas size and fill all 0 values")
          << tr("Change canvas size and fill a specific value")
          << tr("about this plugin")
          ;
}


void CanvasEraserPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
     	//choosing filter
	unsigned int flag = 0;

	if(menu_name == tr("Change canvas size and fill all 0 values"))
     {
		flag = 1;
          processImage(callback,parent, flag);
     }
	else if(menu_name == tr("Change canvas size and fill a specific value"))
	{
          flag = 2;
          processImage(callback,parent, flag);
     }
     else if(menu_name == tr("about this plugin"))
	{
          QString msg = QString(
               "Canvas Eraser version %1 (2009-Aug-14): this demo is developed by Hanchuan Peng to show V3D plugin capability.")
               .arg(getPluginVersion());
		QMessageBox::information(parent, "Version info", msg);
		return;
	}

}


QStringList CanvasEraserPlugin::funclist() const
{
	return QStringList()
		<<tr("cae")
		<<tr("help");
}


bool CanvasEraserPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("cae"))
	{
		return processImage(input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x eraser -f cae -i <inimg_file> -o <outimg_file> -p <xdim> <ydim> <zdim> <vv>"<<endl;
		cout<<endl;
		cout<<"xdim       new x dimension size, default 128,"<<endl;
		cout<<"ydim       new y dimension size, default 128,"<<endl;
		cout<<"zdim       new z dimension size, default 64,"<<endl;
		cout<<"vv         voxel value to be filled, default 0"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x eraser -f cae -i input.raw -o output.raw -p 128 128 64 0"<<endl;
		cout<<endl;
		return true;
	}
}

bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Canvas Eraser"<<endl;
	if (input.size()<1 || output.size() != 1) return false;

     V3DLONG xdim = 128, ydim=128, zdim=64;
     int vv=0; //value to be filled

     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) xdim = atoi(paras.at(0));
          if(paras.size() >= 2) ydim = atoi(paras.at(1));
          if(paras.size() >= 3) zdim = atoi(paras.at(2));
          if(paras.size() >= 4) vv = atoi(paras.at(3));
	}

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
     cout<<"xdim = "<<xdim<<endl;
	cout<<"ydim = "<<ydim<<endl;
     cout<<"zdim = "<<zdim<<endl;
     cout<<"vv   = "<<vv<<endl;

	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

	unsigned char * data1d = 0;

	V3DLONG * in_sz = 0;
	int datatype;
	if(!loadImage(inimg_file, data1d, in_sz, datatype))
     {
          cerr<<"load image "<<inimg_file<<" error!"<<endl;
          return false;
     }


     V3DLONG N = datatype * in_sz[0] * in_sz[1] * in_sz[2] * in_sz[3]; // getTotalBytes
	V3DLONG szx = in_sz[0], szy = in_sz[1], szz = in_sz[2], szc = in_sz[3];

	if (!data1d || szx<=0 || szy<=0 || szz<=0 || szc<=0)
	{
		throw("Your data to the plugin is invalid. Check the program.");
		return false;
	}


     if (szx!=xdim || szy!=ydim || szz!=zdim)
     {
          V3DLONG tb = xdim*ydim*zdim*szc*datatype;
          unsigned char * nm = 0;
          try {nm = new unsigned char [tb];} catch (...) {throw("Fail to allocate memory in Canvas Eraser plugin.");}

          for (V3DLONG i=0;i<tb;i++) nm[i]=vv;

          // save image
          V3DLONG out_sz[]={xdim,ydim,zdim,szc};
          saveImage(outimg_file, (unsigned char *)nm, out_sz, datatype);
     }
     else
     {
          for (V3DLONG i=0;i<N;i++)
               data1d[i] = vv;

          // save image
          saveImage(outimg_file, (unsigned char *)data1d, in_sz, datatype);
     }

     if(data1d) {delete []data1d; data1d=0;}
     if(in_sz)  {delete []in_sz; in_sz=0;}

     return true;
}




void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int flag)
{
     v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
		return;
	}

    Image4DSimple* image = callback.getImage(curwin);

	if (!image)
	{
		QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}


	unsigned char* data1d = image->getRawData();

	V3DLONG N = image->getTotalBytes();
	V3DLONG szx = image->getXDim(), szy = image->getYDim(), szz = image->getZDim(), szc = image->getCDim();

	if (!data1d || szx<=0 || szy<=0 || szz<=0 || szc<=0)
	{
		throw("Your data to the plugin is invalid. Check the program.");
		return;
	}

     if(flag == 1 || flag== 2) // (arg == tr("Change canvas size and fill all 0 values") || arg == tr("Change canvas size and fill a specific value"))
    {
        bool ok;
        V3DLONG xdim = QInputDialog::getInteger(parent, QObject::tr("X dim"),
                                                 QObject::tr("X dim:"),
                                                 szx, 1, szx*2, 1, &ok);
        if (ok)
		{
			V3DLONG ydim = QInputDialog::getInteger(parent, QObject::tr("Y dim"),
                                                 QObject::tr("Y dim:"),
                                                 szy, 1, szy*2, 1, &ok);
			if (ok)
			{
				V3DLONG zdim = QInputDialog::getInteger(parent, QObject::tr("Z dim"),
													 QObject::tr("Z dim:"),
													 szz, 1, szz*2, 1, &ok);
				if (ok)
				{
					int vv = 0;

					if (flag == 2) //(arg == tr("Change canvas size and fill a specific value"))
					{
						vv = QInputDialog::getInteger(parent, QObject::tr("fill in value"),
														  QObject::tr("fill in value (between 0 and 255)"),
														  0, 0, 255, 1, &ok);
						if (!ok)
							return;
					}

					if (szx!=xdim || szy!=ydim || szz!=zdim)
					{
						V3DLONG tb = xdim*ydim*zdim*szc*image->getUnitBytes();
						unsigned char * nm = 0;
						try {nm = new unsigned char [tb];} catch (...) {throw("Fail to allocate memory in Canvas Eraser plugin.");}


						for (V3DLONG i=0;i<tb;i++) nm[i]=vv;

						image->setData(nm, xdim, ydim, zdim, szc, image->getDatatype()); //setData() will free the original memory automatically
					}
					else
					{
						for (V3DLONG i=0;i<N;i++)
							data1d[i] = vv;
					}
				}
			}
		}
    }
	else
		return;
}


