/* plugin_anisodiff_littlequick.cpp
 * 2015-03-18 : by Lei Qu
 */

#include <iostream>
using namespace std;

#include "v3d_message.h"
#include "../../basic_c_fun/stackutil.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions

#include "plugin_anisodiff_littlequick.h"
#include "q_AnisoDiff3D.h"


Q_EXPORT_PLUGIN2(anisodiff_littlequick, AnisoDiffPlugin_littlequick);

struct input_PARA
{
	QString inimg_file;
	V3DLONG channel;
};


void printHelp();
bool anisodiff_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

QStringList AnisoDiffPlugin_littlequick::menulist() const
{
	return QStringList() 
        <<tr("anisodiff_littlequick_menu")
		<<tr("about anisodiff_littlequick_menu");
}
QStringList AnisoDiffPlugin_littlequick::funclist() const
{
	return QStringList()
		<<tr("anisodiff_littlequick_func")
		<<tr("help");
}
void AnisoDiffPlugin_littlequick::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("anisodiff_littlequick_menu"))
	{
    	//do diffusion
		cout<<"============== Welcome to anisodiff_littlequick ================="<<endl;

		bool bmenu = true;
		input_PARA PARA;
    	if(!anisodiff_func(callback,parent,PARA,bmenu))
    	{
    		v3d_msg(tr("ERROR: anisodiff_func() return false!"));
    		return;
    	}
	}
	else
	{
		v3d_msg(tr("3D anisotropic diffusion for neuron reconstruction.\n "
			"Developed by Lei Qu, 2015-03-18 during Bigneuron Beijing hackathon.\n\n"
			"NOTE: This plugin only works for Bigneuron first2000 data, it is more fast and memory efficient than the above one!"));
	}
}
bool AnisoDiffPlugin_littlequick::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("anisodiff_littlequick_menu"))
	{
		cout<<"============== Welcome to anisodiff function ================="<<endl;

		bool bmenu = false;
		input_PARA PARA;

		vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
		vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();

		if(infiles.empty())
		{
			fprintf (stderr, "Need input image. \n");
			return false;
		}
		else
			PARA.inimg_file = infiles[0];

		anisodiff_func(callback,parent,PARA,bmenu);
	}
	else
	{
		printHelp();
	}

	return true;
}

bool anisodiff_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
	unsigned char* p_img_input = 0;
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
		int datatype = 0;
		if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), p_img_input, sz_img_input, datatype))
		{
			fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
			return false;
		}
		if(PARA.channel < 1 || PARA.channel > sz_img_input[3])
		{
			fprintf (stderr, "Invalid channel number. \n");
			return false;
		}

		if(datatype !=1)
		{
			fprintf (stderr, "Please convert the image to be UINT8 and try again!\n");
			return false;
		}
	}

	//-----------------------------------------------------------------------------------------
	printf("1. Find the bounding box and crop image. \n");
	long l_boundbox_min[3],l_boundbox_max[3];//xyz
	V3DLONG sz_img_crop[4];
	long l_npixels_crop;
	unsigned char *p_img8u_crop=0;
	{
	//find bounding box
	unsigned char ***p_img8u_3d=0;
	if(!new3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2],p_img_input))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img8u_3d) 				{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
		return false;
	}

	l_boundbox_min[0]=sz_img_input[0];	l_boundbox_min[1]=sz_img_input[1];	l_boundbox_min[2]=sz_img_input[2];
	l_boundbox_max[0]=0;				l_boundbox_max[1]=0;				l_boundbox_max[2]=0;
	for(long X=0;X<sz_img_input[0];X++)
		for(long Y=0;Y<sz_img_input[1];Y++)
			for(long Z=0;Z<sz_img_input[2];Z++)
				if(p_img8u_3d[Z][Y][X]>0.1)
				{
					if(l_boundbox_min[0]>X) l_boundbox_min[0]=X;	if(l_boundbox_max[0]<X) l_boundbox_max[0]=X;
					if(l_boundbox_min[1]>Y) l_boundbox_min[1]=Y;	if(l_boundbox_max[1]<Y) l_boundbox_max[1]=Y;
					if(l_boundbox_min[2]>Z) l_boundbox_min[2]=Z;	if(l_boundbox_max[2]<Z) l_boundbox_max[2]=Z;
				}
	printf(">>boundingbox: x[%ld~%ld],y[%ld~%ld],z[%ld~%ld]\n",l_boundbox_min[0],l_boundbox_max[0],
															   l_boundbox_min[1],l_boundbox_max[1],
															   l_boundbox_min[2],l_boundbox_max[2]);

	//crop image
	sz_img_crop[0]=l_boundbox_max[0]-l_boundbox_min[0]+1;
	sz_img_crop[1]=l_boundbox_max[1]-l_boundbox_min[1]+1;
	sz_img_crop[2]=l_boundbox_max[2]-l_boundbox_min[2]+1;
	sz_img_crop[3]=1;
	l_npixels_crop=sz_img_crop[0]*sz_img_crop[1]*sz_img_crop[2];

	p_img8u_crop=new(std::nothrow) unsigned char[l_npixels_crop]();
	if(!p_img8u_crop)
	{
		printf("ERROR: Fail to allocate memory for p_img32f_crop!\n");
		if(p_img8u_3d) 				{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
		return false;
	}
	unsigned char *p_tmp=p_img8u_crop;
	for(long Z=0;Z<sz_img_crop[2];Z++)
		for(long Y=0;Y<sz_img_crop[1];Y++)
			for(long X=0;X<sz_img_crop[0];X++)
			{
				*p_tmp = p_img8u_3d[Z+l_boundbox_min[2]][Y+l_boundbox_min[1]][X+l_boundbox_min[0]];
				p_tmp++;
			}
	if(p_img8u_3d) 			{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
	}
	//saveImage("d:/SVN/Vaa3D_source_code/v3d_external/released_plugins/v3d_plugins/anisodiffusion_littlequick/crop.raw",p_img8u_crop,sz_img_crop,1);

	//-----------------------------------------------------------------------------------------
	//convert image data type to float
	printf("2. Convert image data to float and scale to [0~255]. \n");
	float *p_img32f_crop=0;
	{
	p_img32f_crop=new(std::nothrow) float[l_npixels_crop]();
	if(!p_img32f_crop)
	{
		printf("ERROR: Fail to allocate memory for p_img32f_crop!\n");
		if(p_img8u_crop) 			{delete []p_img8u_crop;		p_img8u_crop=0;}
		if(p_img32f_crop) 			{delete []p_img32f_crop;			p_img32f_crop=0;}
		return false;
	}
	//find the maximal intensity value
	float d_maxintensity_input=0.0;
	for(long i=0;i<l_npixels_crop;i++)
		if(p_img8u_crop[i]>d_maxintensity_input)
			d_maxintensity_input=p_img8u_crop[i];
	//convert and rescale
	for(long i=0;i<l_npixels_crop;i++)
		p_img32f_crop[i]=p_img8u_crop[i]/d_maxintensity_input*255.0;
	printf(">>d_maxintensity=%.2f\n",d_maxintensity_input);
	//free input image to save memory
	//if(p_img_input) 			{delete []p_img_input;		p_img_input=0;}
	if(p_img8u_crop) 			{delete []p_img8u_crop;		p_img8u_crop=0;}
	}

	//-----------------------------------------------------------------------------------------
	//do anisotropic diffusion
	printf("3. Do anisotropic diffusion... \n");
	float *p_img32f_crop_output=0;
	if(!q_AnisoDiff3D(p_img32f_crop,sz_img_crop,p_img32f_crop_output))
	{
		printf("ERROR: q_AnisoDiff3D() return false!\n");
		if(p_img8u_crop) 			{delete []p_img8u_crop;		p_img8u_crop=0;}
		if(p_img32f_crop) 				{delete []p_img32f_crop;		p_img32f_crop=0;}
		if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
		return false;
	}
	if(p_img32f_crop) 				{delete []p_img32f_crop;		p_img32f_crop=0;}

	//-----------------------------------------------------------------------------------------
	printf("4. Reconstruct processed crop image back to original size. \n");
	unsigned char *p_img8u_output=0;
	long l_npixels=sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3];
	{
	p_img8u_output=new(std::nothrow) unsigned char[l_npixels]();
	if(!p_img8u_output)
	{
		printf("ERROR: Fail to allocate memory for p_img8u_output!\n");
		if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
		return false;
	}
	//copy original image data to output image
	for(long i=0;i<l_npixels;i++)
		p_img8u_output[i]=p_img_input[i];

	unsigned char ***p_img8u_3d=0;
	if(!new3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2],p_img8u_output))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img8u_output) 				{delete []p_img8u_output;	p_img8u_output=0;}
		if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
		return false;
	}

	float *p_tmp=p_img32f_crop_output;
	for(long Z=0;Z<sz_img_crop[2];Z++)
		for(long Y=0;Y<sz_img_crop[1];Y++)
			for(long X=0;X<sz_img_crop[0];X++)
			{
				p_img8u_3d[Z+l_boundbox_min[2]][Y+l_boundbox_min[1]][X+l_boundbox_min[0]]=(unsigned char)(*p_tmp);
				p_tmp++;
			}

	if(p_img8u_3d) 	{delete3dpointer(p_img8u_3d,sz_img_input[0],sz_img_input[1],sz_img_input[2]);}
	if(p_img32f_crop_output) 		{delete []p_img32f_crop_output;	p_img32f_crop_output=0;}
	}

	//-----------------------------------------------------------------------------------------
	//save or display
	if(bmenu)
	{
		printf("5. Display the processed image in Vaa3D. \n");
		//push result image back to v3d
		v3dhandle newwin=callback.newImageWindow("output");
		Image4DSimple img4D_output;
		img4D_output.setData(p_img8u_output,sz_img_input[0],sz_img_input[1],sz_img_input[2],1,V3D_UINT8);
		callback.setImage(newwin,&img4D_output);
		callback.updateImageWindow(newwin);
		callback.open3DWindow(newwin);
	}
	else
	{
		printf("5. Save the processed image to file. \n");
		QString str_outimg_filename = PARA.inimg_file + "_anisodiff.raw";
		saveImage(qPrintable(str_outimg_filename),p_img8u_output,sz_img_input,1);

		if(p_img8u_output) 		{delete []p_img8u_output;		p_img8u_output=0;}
	}

	printf(">>Program complete success!\n");
	return true;
}


void printHelp()
{
	printf("**** Usage of anisodiff  **** \n");
	printf("\nUsage: v3d -x <anisodiff> -f anisodiff_littlequick_menu -i <inimg_file> \n");
	printf("inimg_file       The input image path (input image need to be uint8 single channel)\n");

	printf("outimg_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");
	printf("***************************** \n");
	return;
}
