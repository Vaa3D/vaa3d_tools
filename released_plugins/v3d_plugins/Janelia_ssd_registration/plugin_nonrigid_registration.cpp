//plugin_nonrigid_registration.h
//by Lei Qu
//2010-02-26

#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include "../../basic_c_fun/stackutil.h"'

#include "plugin_nonrigid_registration.h"
#include "q_nonrigid_registration_dialogs.h"
#include "q_nonrigid_registration.h"
#include "q_convolve.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(plugin_nonrigid_registration, NonrigidRegistrationPlugin);

const QString title = "NonrigidRegistrationPlugin demo";

void FFDNonrigidRegistration(V3DPluginCallback &callback, QWidget *parent);
void releasememory(long*,long*,unsigned char*,unsigned char*,double*,double*,double*,double*,double*,double*);

QStringList NonrigidRegistrationPlugin::menulist() const
{
    return QStringList()
	<< tr("FFD non-rigid registration...");
}

void NonrigidRegistrationPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if(menu_name==tr("FFD non-rigid registration..."))
	{
		FFDNonrigidRegistration(callback, parent);
	}
}

void FFDNonrigidRegistration(V3DPluginCallback &callback, QWidget *parent)
{
	//get parameters
	QString qs_filename_img_tar,qs_filename_img_sub,qs_filename_img_sub2tar,qs_filename_swc_grid;
	long l_refchannel,l_hierarchlevel,l_gridwndsz;

	NonrigidRegistrationDialog nonrigidregistrationDLG(parent);
	if(nonrigidregistrationDLG.exec()==QDialog::Accepted)
	{
		qs_filename_img_tar=nonrigidregistrationDLG.m_pLineEdit_filepath_tar->text();
		qs_filename_img_sub=nonrigidregistrationDLG.m_pLineEdit_filepath_sub->text();
		qs_filename_img_sub2tar=nonrigidregistrationDLG.m_pLineEdit_filepath_sub2tar->text();
		qs_filename_swc_grid=nonrigidregistrationDLG.m_pLineEdit_filepath_meshgrid->text();

		l_refchannel=nonrigidregistrationDLG.m_pLineEdit_refchannel->text().toInt();
		l_hierarchlevel=nonrigidregistrationDLG.m_pLineEdit_hierarchlevel->text().toInt();
		l_gridwndsz=nonrigidregistrationDLG.m_pLineEdit_gridwndsz->text().toInt();
	}
	else
		return;

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>FFD based non-rigid registration\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
	printf(">>  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
	printf(">>  input reference channel:      %ld\n",l_refchannel);
	printf(">>  input hierarchical level:     %ld\n",l_hierarchlevel);
	printf(">>  input meshgrid window size:   %ld\n",l_gridwndsz);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output sub2tar image:         %s\n",qPrintable(qs_filename_img_sub2tar));
	printf(">>  output meshgrid apo:          %s\n",qPrintable(qs_filename_swc_grid));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");


	//------------------------------------------------------------------------------------------------------------------------------------
	//FFD non-rigid registration
	//------------------------------------------------------------------------------------------------------------------------------------
	//pointers definition (needed to be released)
	long *sz_img_tar=0,*sz_img_sub=0;						//input image size. [0]:width, [1]:height, [2]:z, [3]:nchannel
	unsigned char *p_img_tar_input=0,*p_img_sub_input=0;	//input images pointer
	double *p_img64f_tar=0,*p_img64f_sub=0;					//input images pointer (double type)
	double *p_img64f_sub_bk=0;								//backed nonsmoothed subject image pointer
	double *p_img64f_tar_1c=0,*p_img64f_sub_1c=0;			//image pointers with only reference channel
	double *p_img64f_output_sub=0;							//output warped subject image pointer

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target and subject image. \n");
	//read target image
	int datatype_tar=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar,datatype_tar))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
		return;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);
	printf("\t\tdatatype: %d\n",datatype_tar);

	//read subject image
	int datatype_sub=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub,datatype_sub))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
		return;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
	printf("\t\tdatatype: %d\n",datatype_sub);

	//------------------------------------------------------------------------------------------------------------------------------------
	//check image size
	if(sz_img_tar[0]!=sz_img_sub[0] || sz_img_tar[1]!=sz_img_sub[1] || sz_img_tar[2]!=sz_img_sub[2] || sz_img_tar[3]!=sz_img_sub[3] ||
	   datatype_tar!=datatype_sub)
	{
		printf("ERROR: input target and subject image have different size or datatype!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//convert image data type to double and rescale to [0~1]
	printf("2. Convert image data from uint16 to double and scale to [0~1]. \n");
	p_img64f_tar=new double[sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2]*sz_img_tar[3]];
	p_img64f_sub=new double[sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2]*sz_img_tar[3]];
	if(!p_img64f_tar || !p_img64f_sub)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}

	long l_npixels=sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2]*sz_img_tar[3];
	//scale to [0~1]
	if(datatype_tar==1)
	{
		for(long i=0;i<l_npixels;i++)
		{
			p_img64f_tar[i]=p_img_tar_input[i]/255.0;
			p_img64f_sub[i]=p_img_sub_input[i]/255.0;
		}
	}
	else if(datatype_tar==2)
	{
		for(long i=0;i<l_npixels;i++)
		{
			unsigned short int * p_data16u_tar=(unsigned short int *)p_img_tar_input;
			unsigned short int * p_data16u_sub=(unsigned short int *)p_img_sub_input;
			p_img64f_tar[i]=p_data16u_tar[i]/256.0/255.0;
			p_img64f_sub[i]=p_data16u_sub[i]/256.0/255.0;
		}
	}
	if(p_img_tar_input) 	{delete []p_img_tar_input;		p_img_tar_input=0;}
	if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}

//	q_save64f12_image(p_img64f_tar,sz_img_tar,"tar.tif");
//	q_save64f12_image(p_img64f_sub,sz_img_tar,"sub.tif");

	//------------------------------------------------------------------------------------------------------------------------------------
	//backup the subject image before smoothing
	printf("3. Backup the subject image before smoothing. \n");
	//allocate memory
	p_img64f_sub_bk=new double[l_npixels];
	if(!p_img64f_sub_bk)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}
	//backup
	for(long i=0;i<l_npixels;i++)
		p_img64f_sub_bk[i]=p_img64f_sub[i];

	//------------------------------------------------------------------------------------------------------------------------------------
	//extract reference channel
	printf("4. Extract reference channel:[%ld]. \n",l_refchannel);
	long l_npixels_1c=sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2];
	long l_ind_start=l_refchannel*sz_img_tar[0]*sz_img_tar[1];
	p_img64f_tar_1c=new double[l_npixels];
	p_img64f_sub_1c=new double[l_npixels];
	if(!p_img64f_tar_1c || !p_img64f_sub_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_tar_1c or p_img64f_sub_1c!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}
	for(long i=0;i<l_npixels_1c;i++)
	{
		p_img64f_tar_1c[i]=p_img64f_tar[i+l_ind_start];
		p_img64f_sub_1c[i]=p_img64f_sub[i+l_ind_start];
	}
	if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
	if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}

	//------------------------------------------------------------------------------------------------------------------------------------
	//Gaussian smooth (optional)
	printf("4. Gaussian smooth input images. \n");
	long l_radius_x=3,l_radius_y=3,l_radius_z=3;
	double d_sigma=0.5;

	vector< vector< vector<double> > > vec3D_kernel;
	if(!q_kernel_gaussian(l_radius_x,l_radius_y,l_radius_z,d_sigma,vec3D_kernel))
	{
		printf("ERROR: q_kernel_gaussian() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}
	printf("\tsmoothing target image...\n");
	if(!q_convolve_img64f_3D(p_img64f_tar_1c,sz_img_tar,vec3D_kernel))
	{
		printf("ERROR: q_convolve64f_3D() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}
	printf("\tsmoothing subject image...\n");
	if(!q_convolve_img64f_3D(p_img64f_sub_1c,sz_img_tar,vec3D_kernel))
	{
		printf("ERROR: q_convolve64f_3D() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}
//	q_save64f12_image(p_img64f_tar,sz_img_tar,"tar_smooth.tif");
//	q_save64f12_image(p_img64f_sub,sz_img_tar,"sub_smooth.tif");

	//------------------------------------------------------------------------------------------------------------------------------------
	//FFD based non-rigid registration
	printf("############################################################################################################################\n");
	printf("5. Enter FFD based non-rigid registration. \n");
	printf("############################################################################################################################\n");
	vector< vector< vector< vector<double> > > > vec4D_grid;
	if(!q_nonrigid_registration_FFD(p_img64f_tar_1c,p_img64f_sub_1c,sz_img_tar,l_hierarchlevel,l_gridwndsz,vec4D_grid))
	{
		printf("ERROR: q_nonrigid_registration_FFD() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//prepare the final output (warp and trim the input subject image base on the deformed meshgrid)
	printf("6. prepare the final output (warp and trim the subject image base on the deformed meshgrid). \n");
	if(!q_warpimage_baseongrid(p_img64f_sub_bk,sz_img_sub,vec4D_grid,l_gridwndsz,p_img64f_output_sub))
	{
		printf("ERROR: q_warpimage_baseongrid() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	QStringList qsl_outputinfo;
	qsl_outputinfo.push_back("Registration results were saved to:");

	//save warped image
	printf("7. save warped subject image to file:[%s]. \n",qPrintable(qs_filename_img_sub2tar));
	if(!q_save64f01_image(p_img64f_output_sub,sz_img_sub,qPrintable(qs_filename_img_sub2tar)))
	{
		printf("ERROR: q_save64f12_image() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return;
	}
	if(p_img64f_output_sub) {delete []p_img64f_output_sub;		p_img64f_output_sub=0;}
	qsl_outputinfo.push_back(QString("[%1]").arg(qs_filename_img_sub2tar));

	//------------------------------------------------------------------------------------------------------------------------------------
	//save deformed meshgrid to swc file
	if(qs_filename_swc_grid!=NULL)
	{
		printf("8. save warped subject image to swc file:[%s]. \n",qPrintable(qs_filename_swc_grid));
		if(!q_savegrid_swc(vec4D_grid,l_gridwndsz,qPrintable(qs_filename_swc_grid)))
		{
			printf("ERROR: q_savegrid_swc() return false!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return;
		}
		qsl_outputinfo.push_back(QString("[%1]").arg(qs_filename_swc_grid));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	printf(">>Free memory\n");
	releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
	printf(">>Program exit success!\n");

	//display output info dialog
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle(QObject::tr("Output information"));
	msgBox.setText(qsl_outputinfo.join("\n"));
	msgBox.exec();

	return;
}


//release all allocated memory
void releasememory(long *sz_img_tar,long *sz_img_sub,
		unsigned char *p_img_tar_input,unsigned char *p_img_sub_input,
		double *p_img64f_tar,double *p_img64f_sub,
		double *p_img64f_sub_bk,
		double *p_img64f_tar_1c,double *p_img64f_sub_1c,
		double *p_img64f_output_sub)
{
	if(p_img_tar_input) 	{delete []p_img_tar_input;		p_img_tar_input=0;}
	if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}
	if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
	if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}
	if(p_img64f_sub_bk)		{delete []p_img64f_sub_bk;		p_img64f_sub_bk=0;}
	if(p_img64f_tar_1c)		{delete []p_img64f_tar_1c;		p_img64f_tar_1c=0;}
	if(p_img64f_sub_1c)		{delete []p_img64f_sub_1c;		p_img64f_sub_1c=0;}
	if(p_img64f_output_sub) {delete []p_img64f_output_sub;	p_img64f_output_sub=0;}
	if(sz_img_tar) 			{delete []sz_img_tar;			sz_img_tar=0;}
	if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
	printf("Release all memory!\n");
}
