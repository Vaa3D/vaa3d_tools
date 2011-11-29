//by Hanchuan Peng
//2009-05-30

#include <QtGui>
#include <math.h>
#include <stdlib.h>
#include "v3d_message.h"
#include "edge_of_maskimg.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(edge_of_maskimg, EdgeOfMaskImgPlugin);

void findedgeimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code);

QStringList EdgeOfMaskImgPlugin::menulist() const
{
    return QStringList()
	<< tr("Label edge of a mask image using the original label values")
	<< tr("Label edge of a mask image using intensity 255")
	<< tr("about this plugin")
	;
}

void EdgeOfMaskImgPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Label edge of a mask image using the original label values"))
		findedgeimg(callback, parent, 0);
	else if (menu_name == tr("Label edge of a mask image using intensity 255"))
		findedgeimg(callback, parent, 1);
	else
		v3d_msg("Label the edge of a mask image (label field image)'s edge using either the original label values or white (255). Developed by Hanchuan Peng, 2010.");
}

void findedgeimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	
	if (method_code!=0 && method_code!=1)
	{
		v3d_msg("Invalid internal method code. You should never see this message. Report this bug to the developer");
		return;
	}
	
	int start_t = clock(); // record time point
	
	Image4DSimple* image = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!image) 
	{
		v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}

	unsigned char* data1d = image->getRawData();

	V3DLONG N = image->getTotalBytes();
	V3DLONG szx = image->getXDim(), szy = image->getYDim(), szz = image->getZDim(), szc = image->getCDim();

	if (!data1d || szx<=0 || szy<=0 || szz<=0 || szc<=0)
	{
		throw("Your data to the plugin is invalid. Check the data and try again.");
		return;
	}

	//create the output buffer
	unsigned char *outputData = 0;
	try
	{
		outputData = new unsigned char [N]; 
		for (V3DLONG tmpi=0;tmpi<N;++tmpi) outputData[tmpi] = 0; //preset to be all 0
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory.");
		if (outputData) {delete []outputData; outputData=0;}
		return;
	}
	Image4DSimple outputImage;
	outputImage.setData((unsigned char*)outputData, szx, szy, szz, szc, V3D_UINT8);
	Image4DProxy<Image4DSimple> outputIProxy(&outputImage);

	//now do computation
    {
		bool bset255 = (method_code==0) ? false : true;
			
		Image4DProxy<Image4DSimple> p(image);
		Image4DProxy_foreach(p, ix, iy, iz, ic)
		{
			double v = p.value_at(ix, iy, iz, ic);
			V3DLONG cx, cy, cz;
			
			bool bb=false;
			for (cz = iz-1; cz<iz+2; ++cz)
			{
				for (cy = iy-1; cy<iy+2; ++cy)
				{
					for (cx = ix-1; cx<ix+2; ++cx)
					{
						if (!p.is_inner(cx, cy, cz, ic))
							continue;
						if (v!=p.value_at(cx, cy, cz, ic))
						{
							*outputIProxy.at(ix, iy, iz, ic) = (bset255) ? 255 : v; 
							bb = true; 
							break;
						}
					}
					if (bb)	break;
				}
				if (bb) break;
			}

			//note that all value had been preset as 0, thus no need to set as the background color in case not an edge point
		}
    }


	int end_t = clock();
	printf("time eclapse %d s for edge detecting.\n", (end_t-start_t)/1000000);
	
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &outputImage);
	callback.setImageName(newwin, QString("edge map of the image"));
	callback.updateImageWindow(newwin);
	
	return;
}

