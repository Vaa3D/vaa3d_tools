/* plugin_anisodiff.cpp
 * 2015-03-16 : by Lei Qu
 */
//todo: 
//1.add bad_alloc handler, show output in v3d -->done!
//2.add dofunc, help
//3.add small memory funcs
//end: rebuild v3d and write the note

#include <iostream>
#include "v3d_message.h"
#include "../../basic_c_fun/stackutil.h"
using namespace std;

#include "plugin_anisodiff.h"
#include "q_AnisoDiff3D.h"


Q_EXPORT_PLUGIN2(anisodiff, AnisoDiffPlugin);

void printHelp();
bool anisodiff_func(V3DPluginCallback2 &callback, QWidget *parent, bool bmenu);

QStringList AnisoDiffPlugin::menulist() const
{
	return QStringList() 
        <<tr("anisodiff_menu")
		<<tr("about");
}
QStringList AnisoDiffPlugin::funclist() const
{
	return QStringList()
		<<tr("anisodiff_func")
		<<tr("help");
}
void AnisoDiffPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("anisodiff_menu"))
	{
    	//do diffusion
		cout<<"============== Welcome to anisodiff function ================="<<endl;

		bool bmenu = true;
    	if(!anisodiff_func(callback,parent,bmenu))
    	{
    		v3d_msg(tr("ERROR: anisodiff_func() return false!"));
    		return;
    	}
	}
	else
	{
		v3d_msg(tr("3D anisotropic diffusion for neuron reconstruction. "
			"Developed by Lei Qu, 2015-03-16"));
	}
}
bool AnisoDiffPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("anisodiff_func"))
	{
		cout<<"============== Welcome to anisodiff function ================="<<endl;

	 //  	//do diffusion
		//if(!anisodiff_func())
		//{
		//	v3d_msg(tr("ERROR: anisodiff_func() return false!"));
		//	return false;
		//}
		return true;
	}
	else
	{
		printHelp();
	}
}

bool anisodiff_func(V3DPluginCallback2 &callback, QWidget *parent, bool bmenu)
{
	unsigned char* p_img_input = 0;
	V3DLONG N,M,P,sc,c;
	V3DLONG sz_img_input[4];
	if(bmenu)
	{
		v3dhandle curwin = callback.currentImageWindow();
		if (!curwin)
		{
			QMessageBox::information(0, "", "You don't have any image open in the main window.");
			return false;
		}
		Image4DSimple* p4DImage = callback.getImage(curwin);
		if (!p4DImage)
		{
			QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
			return false;
		}
		if(p4DImage->getDatatype()!=V3D_UINT8)
		{
			QMessageBox::information(0, "", "Please convert the image to be UINT8 and try again!");
			return false;
		}
		if(p4DImage->getCDim()!=1)
		{
			QMessageBox::information(0, "", "The input image is not one channel image!");
			return false;
		}
		p_img_input = p4DImage->getRawData();
		sz_img_input[0] = p4DImage->getXDim();
		sz_img_input[1] = p4DImage->getYDim();
		sz_img_input[2] = p4DImage->getZDim();
		sz_img_input[3] = 1;
	}
	else
	{
		;//todo
	}

	//-----------------------------------------------------------------------------------------
	long l_npixels=sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3];

	//convert image data type to double
	printf("1. Convert image datatype from uint8 to 64f and rescale to [0~255]. \n");
	double *p_img64f=0;
	{
		p_img64f=new(std::nothrow) double[l_npixels]();
		if(!p_img64f)
		{
			printf("ERROR: Fail to allocate memory for p_img64f!\n");
			if(p_img_input) 			{delete []p_img_input;		p_img_input=0;}
			return false;
		}

		//find the maximal intensity value
		double d_maxintensity_input=0.0;
		for(long i=0;i<l_npixels;i++)
			if(p_img_input[i]>d_maxintensity_input)
				d_maxintensity_input=p_img_input[i];
		//convert and rescale
		for(long i=0;i<l_npixels;i++)
			p_img64f[i]=p_img_input[i]/d_maxintensity_input*255.0;
		printf(">>d_maxintensity=%.2f\n",d_maxintensity_input);
	}

	//do anisotropic diffusion
	printf("2. Do anisotropic diffusion... \n");
	double *p_img64f_output=0;
	if(!q_AnisoDiff3D(p_img64f,sz_img_input,p_img64f_output))
	{
		printf("ERROR: q_AnisoDiff3D() return false!\n");
		if(p_img64f) 			{delete []p_img64f;				p_img64f=0;}
		if(p_img64f_output) 	{delete []p_img64f_output;		p_img64f_output=0;}
		return false;
	}
	if(p_img64f) 			{delete []p_img64f;				p_img64f=0;}

	//convert image data type from double to uint8
	unsigned char *p_img8u_output=0;
	p_img8u_output=new(std::nothrow) unsigned char[l_npixels]();
	if(!p_img8u_output)
	{
		printf("ERROR: Fail to allocate memory for p_img8u_output!\n");
		if(p_img64f) 			{delete []p_img64f;				p_img64f=0;}
		if(p_img64f_output) 	{delete []p_img64f_output;		p_img64f_output=0;}
		return false;
	}
	for(long i=0;i<l_npixels;i++)
		p_img8u_output[i]=(unsigned char)(p_img64f_output[i]);
	if(p_img64f_output) 	{delete []p_img64f_output;		p_img64f_output=0;}

	//push result image back to v3d
	v3dhandle newwin=callback.newImageWindow("output");
	Image4DSimple img4D_output;
	img4D_output.setData(p_img8u_output,sz_img_input[0],sz_img_input[1],sz_img_input[2],1,V3D_UINT8);
	callback.setImage(newwin,&img4D_output);
	callback.updateImageWindow(newwin);
	callback.open3DWindow(newwin);

	//free memory
	if(p_img64f) 			{delete []p_img64f;				p_img64f=0;}
	if(p_img64f_output) 	{delete []p_img64f_output;		p_img64f_output=0;}
	//if(p_img8u_output) 		{delete []p_img8u_output;		p_img8u_output=0;}

	printf(">>Program complete success!\n");
	return true;
}


void printHelp()
{
	printf("\nUsage: v3d -x <anisodiff> -f anisodiff -i <input_image> -o <output_image> \n");
	return;
}
