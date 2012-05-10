//celegans straighten plugin
//by Lei Qu
//2010-08-09

// add dofunc() interface by Jianlong Zhou. 2012-05-02
// reorder code for dofunc() interface by Jianlong Zhou. 2012-05-02

#include <QtGui>
#include <stdio.h>
#include <vector>
using namespace std;
#include "../../basic_c_fun/stackutil.h"
#include "../../basic_c_fun/basic_surf_objs.h"

#include "../../v3d_main/worm_straighten_c/spline_cubic.h"
#include "../../v3d_main/jba/c++/convert_type2uint8.h"
#include "q_skeletonbased_warp_sub2tar.h"

#include "q_principalskeleton_detection.h"
#include "q_celegans_straighten.h"
#include "q_paradialog_straighten.h"
#include "plugin_celegans_straighten.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(celegans_straighten,CElegansStraightenPlugin);

const QString title = "CElegans Straighten demo";
bool CElegansStraighten(V3DPluginCallback2 &callback, QWidget *parent);
bool CElegansStraighten(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);

bool do_straighten(V3DPluginCallback2 &callback, ParaDialog &paraDlg, QString &qs_filename_img, QString &qs_filename_marker_input, CSParas &paras, QString &qs_filename_strimg_output );

QStringList CElegansStraightenPlugin::menulist() const
{
    return QStringList()
	<< tr("celegans_straighten...");
}

void CElegansStraightenPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if(menu_name==tr("celegans_straighten..."))
	{
		CElegansStraighten(callback, parent);
	}
}

QStringList CElegansStraightenPlugin::funclist() const
{
	return QStringList()
		<<tr("cst")
		<<tr("help");
}


bool CElegansStraightenPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("cst"))
	{
		return CElegansStraighten(input, output, callback);
     }
	else if(func_name == tr("help"))
	{
          cout<<"Usage : v3d -x straighten -f cst -i <inimg_file> <marker_file> -o <outimg_file> -p <refchannel> <ratio> <ctnum> <ocradius> <xy_factor> <xz_factor> <cpradius>"<<endl;
		cout<<endl;
		cout<<"inimg_file    file name of input image"<<endl;
          cout<<"marker_file   file name of input marker"<<endl;
          cout<<"outimg_file   file name of output image"<<endl;
		cout<<"refchannel    channel number, r: 0, g: 1, b: 2, all: 9, default 1"<<endl;
		cout<<"ratio         downsampling ratio, default 4.0"<<endl;
		cout<<"ctnum         number of control points, default 10"<<endl;
		cout<<"ocradius      radius of openclosing, default 0"<<endl;
          cout<<"xy_factor     xy fg thresh factor, default 3.0"<<endl;
          cout<<"xz_factor     xz fg thresh factor, default 0.0"<<endl;
          cout<<"cpradius      cutting plane radius, default 100"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x straighten -f cst -i input.raw marker_file.marker -o output.raw -p 1 4.0 10 0 3.0 0.0 100"<<endl;
		return true;
	}
}

// for dofunc()
bool CElegansStraighten(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{

     cout<<"Welcome to Celegans Straighten"<<endl;
     if (output.size() != 1) return false;

     CSParas paras_str;
     QString qs_filename_img = "";
     QString qs_filename_marker_input = "";

     paras_str.l_refchannel = 1;
     paras_str.d_downsampleratio = 4.0;
     paras_str.l_ctlpts_num = 10;
     paras_str.l_radius_openclose = 0;
     paras_str.d_fgthresh_xy = 3.0;
     paras_str.d_fgthresh_xz = 0.0;
     paras_str.l_radius_cuttingplane = 100;

     if (input.size()>=1)
     {
          qs_filename_img = ((vector<char*> *)(input.at(0).p))->at(0);
          qs_filename_marker_input = ((vector<char*> *)(input.at(0).p))->at(1);

          if(input.size()>=2)
          {
               vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
               if(paras.size() >= 1) paras_str.l_refchannel = atoi(paras.at(0));
               if(paras.size() >= 2) paras_str.d_downsampleratio = atof(paras.at(1));
               if(paras.size() >= 3) paras_str.l_ctlpts_num = atoi(paras.at(2));
               if(paras.size() >= 4) paras_str.l_radius_openclose = atoi(paras.at(3));
               if(paras.size() >= 5) paras_str.d_fgthresh_xy = atof(paras.at(4));
               if(paras.size() >= 6) paras_str.d_fgthresh_xz = atof(paras.at(5));
               if(paras.size() >= 7) paras_str.l_radius_cuttingplane = atoi(paras.at(6));
          }
	}
     else
     {
          v3d_msg("There are not enough parameters.\n", 0);
          return false;
     }

     // ========================================================
     QString qs_filename_strimg_output = ((vector<char*> *)(output.at(0).p))->at(0);

     //image and marker
     paras_str.b_imgfromV3D=false;
     paras_str.b_markerfromV3D=false;
     //visualization
     paras_str.b_showinV3D_pts=false;

     if(qs_filename_img.isEmpty() || qs_filename_marker_input.isEmpty() || qs_filename_strimg_output.isEmpty())
     {
          v3d_msg("Invalid straighten parameters.\n", 0);
          return false;
     }

     ParaDialog paraDlg(callback, NULL); // it is no use for dofunc, but necessary for building
     if(!do_straighten(callback, paraDlg, qs_filename_img, qs_filename_marker_input, paras_str, qs_filename_strimg_output) )
     {
          return false;
     }

     return true;

}

bool CElegansStraighten(V3DPluginCallback2 &callback, QWidget *parent)
{
	ParaDialog paraDlg(callback, parent);
	if(paraDlg.exec()==QDialog::Accepted)
	{
		CSParas paras;
		//image and marker
		paras.b_imgfromV3D=paraDlg.radioButton_imgv3d->isChecked();
		paras.b_markerfromV3D=paraDlg.radioButton_markerv3d->isChecked();
		QString qs_filename_img=paraDlg.lineEdit_imfile->text();
		QString qs_filename_marker_input=paraDlg.lineEdit_markerfile->text();
		QString qs_filename_strimg_output=paraDlg.lineEdit_strimg_output->text();
		//visualization
		paras.b_showinV3D_pts=paraDlg.checkBox_showV3D_pts->isChecked();
		//skeleton detection
		paras.l_refchannel=paraDlg.lineEdit_refchannel->text().toInt();
		paras.d_downsampleratio=paraDlg.lineEdit_downsampleratio->text().toDouble();
		paras.l_ctlpts_num=paraDlg.lineEdit_ctlpts_num->text().toLong();
		paras.l_radius_openclose=paraDlg.lineEdit_radius_openclose->text().toLong();
		paras.d_fgthresh_xy=paraDlg.lineEdit_fgthresh_xy->text().toDouble();
		paras.d_fgthresh_xz=paraDlg.lineEdit_fgthresh_xz->text().toDouble();
		//straighten
		paras.l_radius_cuttingplane=paraDlg.lineEdit_radius_cuttingplane->text().toLong();

          // do_straighten
          if(!do_straighten(callback, paraDlg, qs_filename_img, qs_filename_marker_input, paras, qs_filename_strimg_output) )
          {
               return false;
          }
          v3d_msg(QString("Celegans Strenghten is finished successfully!"));
          return true;
	}

}




bool do_straighten(V3DPluginCallback2 &callback, ParaDialog &paraDlg, QString &qs_filename_img, QString &qs_filename_marker_input, CSParas &paras, QString &qs_filename_strimg_output )
{
     //------------------------------------------------------------------------------------------------------------------------------------

     printf("1. Import image. \n");
     unsigned char *p_img_input=0;
     long *sz_img_input=0;
     int datatype_input=0;
     ROIList roils_roi;

     if(!paras.b_imgfromV3D)
     {
          if(qs_filename_img.isEmpty())
          {
               v3d_msg(QString("invalid image path!"));
               return false;
          }
          if(!loadImage((char *)qPrintable(qs_filename_img),p_img_input,sz_img_input,datatype_input))
          {
               v3d_msg(QString("open file [%1] failed!").arg(qs_filename_img));
               return false;
          }
          printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img));
     }
     else
     {
          printf("\t>>import image from V3D. \n");
          long l_imgind=paraDlg.comboBox_imgv3d->currentIndex();
          if(l_imgind<0)
          {
               v3d_msg(QString("Image index is invalid!\nMake sure there are at least 1 image opend in V3D!"));
               return false;
          }
          Image4DSimple* image=callback.getImage(paraDlg.h_wndlist[l_imgind]);
          p_img_input=image->getRawData();
          sz_img_input=new long[4]();
          sz_img_input[0]=image->getXDim();	sz_img_input[1]=image->getYDim();	sz_img_input[2]=image->getZDim();	sz_img_input[3]=image->getCDim();
          datatype_input=image->getDatatype();

          //import ROI
          roils_roi=callback.getROI(paraDlg.h_wndlist[l_imgind]);//xy, zy, xz
          for(long i=0;i<roils_roi.size();i++)
          {
               printf("\t>>roi-%ld: ",i);
               for(long j=0;j<roils_roi[i].size();j++)
                    printf("[%ld,%ld],",roils_roi[i][j].x(),roils_roi[i][j].y());
               printf("\n");
          }
     }
     printf("\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);
     printf("\tdatatype: %d\n",datatype_input);

     //------------------------------------------------------------------------------------------------------------------------------------
     printf("2. Convert image datatype to uint8 if it is not. \n");
     unsigned char * p_img_8u=0;
     {
		long l_npixels=sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3];
		p_img_8u=new unsigned char[l_npixels];
		if(!p_img_8u)
		{
			printf("ERROR: Fail to allocate memory. Do nothing. \n");
			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return false;
		}
		if(datatype_input==1)
		{
			printf("\t>>convert image data from uint8 to uint8. \n");
			for(long i=0;i<l_npixels;i++)
				p_img_8u[i]=p_img_input[i];
		}
		else if(datatype_input==2)
		{
			printf("\t>>convert image data from uint16 to uint8. \n");
			double min,max;
			if(!rescale_to_0_255_and_copy((unsigned short int *)p_img_input,l_npixels,min,max,p_img_8u))
			{
				printf("ERROR: rescale_to_0_255_and_copy() return false.\n");
				if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
				if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
				if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
				return false;
			}
		}
		else if(datatype_input==3)
		{
			printf("\t>>convert image data from float to uint8. \n");
			double min,max;
			if(!rescale_to_0_255_and_copy((float *)p_img_input,l_npixels,min,max,p_img_8u))
			{
				printf("ERROR: rescale_to_0_255_and_copy() return false.\n");
				if(p_img_8u)								{delete []p_img_8u;			p_img_8u=0;}
				if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
				if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
				return false;
			}
		}
		else
		{
			v3d_msg(QString("Unknown datatype!\n"));
			if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return false;
		}
     }

     //------------------------------------------------------------------------------------------------------------------------------------
     printf("3. Import markers. \n");
     //get initial head and tail marker position from V3D
     vector< vector<double> > vec2d_markers;
     if(!paras.b_markerfromV3D)
     {
          if(qs_filename_marker_input.isEmpty())
          {
               v3d_msg(QString("Invalid marker path!"));
               return false;
          }
          QList<ImageMarker> ql_markers=readMarker_file(qs_filename_marker_input);
          printf("\t>>read %d markers from file: %s.\n",ql_markers.size(),qPrintable(qs_filename_marker_input));
          if(ql_markers.size()<2)
          {
               v3d_msg(QString("Need at least 2 markers!"));
               if(p_img_8u) 								{delete []p_img_8u;				p_img_8u=0;}
               if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
               if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
               return false;
          }
          vector<double> vec_marker(3,0);
          for(long i=0;i<ql_markers.size();i++)
          {
               vec_marker[0]=ql_markers[i].x-1;
               vec_marker[1]=ql_markers[i].y-1;
               vec_marker[2]=ql_markers[i].z-1;
               vec2d_markers.push_back(vec_marker);
          }
     }
     else
     {
          printf("\t>>import markers from V3D. \n");
          long l_imgind=paraDlg.comboBox_imgv3d->currentIndex();
          if(l_imgind<0)
          {
               v3d_msg(QString("Image index is invalid!\nMake sure there are at least 1 image opend in V3D!"));
               return false;
          }
          LandmarkList ml_makers=callback.getLandmark(callback.currentImageWindow());
          if(ml_makers.size()<2)
          {
               v3d_msg(QString("Need at least 2 markers!"));
               if(p_img_8u) 								{delete []p_img_8u;				p_img_8u=0;}
               if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
               if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
               return false;
          }
          vector<double> vec_marker(3,0);
          for(long i=0;i<ml_makers.size();i++)
          {
               vec_marker[0]=ml_makers[i].x;
               vec_marker[1]=ml_makers[i].y;
               vec_marker[2]=ml_makers[i].z;
               vec2d_markers.push_back(vec_marker);
          }
     }

     //------------------------------------------------------------------------------------------------------------------------------------
     printf("4. Crop the ROI region and modify the marker position accordingly. \n");
     long sz_img_roi[4];
     unsigned char *p_img_roi=0;
     {
		//get the size of roi region (union of bounding box)
		QList<QRect> ql_boundingrect;	//xy,xz,zy
		ql_boundingrect.push_back(QRect(0,0,sz_img_input[0],sz_img_input[1]));	//xy
		ql_boundingrect.push_back(QRect(0,0,sz_img_input[2],sz_img_input[1]));	//zy
		ql_boundingrect.push_back(QRect(0,0,sz_img_input[0],sz_img_input[2]));	//xz

          // changed by ZJL for dofunc(). 2012-05-09
          if(paras.b_imgfromV3D)
          {
               for(long i=0;i<3;i++)
                    if(roils_roi[i].size()!=0)
                         ql_boundingrect[i]=roils_roi[i].boundingRect();
                    else
                         roils_roi[i]=ql_boundingrect[i];
          }
          else
          {
               for(long i=0;i<3;i++)
                   roils_roi.push_back(ql_boundingrect[i]);
          }

          long x_min=max(ql_boundingrect[0].left(),ql_boundingrect[2].left());
		long x_max=min(ql_boundingrect[0].right(),ql_boundingrect[2].right());
		long y_min=max(ql_boundingrect[0].top(),ql_boundingrect[1].top());
		long y_max=min(ql_boundingrect[0].bottom(),ql_boundingrect[1].bottom());
		long z_min=max(ql_boundingrect[1].left(),ql_boundingrect[2].top());
		long z_max=min(ql_boundingrect[1].right(),ql_boundingrect[2].bottom());
		sz_img_roi[0]=x_max-x_min+1;
		sz_img_roi[1]=y_max-y_min+1;
		sz_img_roi[2]=z_max-z_min+1;
		sz_img_roi[3]=sz_img_input[3];
		printf("\t>>ROI size: [w=%d, h=%d, z=%d, c=%d]\n",sz_img_roi[0],sz_img_roi[1],sz_img_roi[2],sz_img_roi[3]);
		//crop roi region
		long n_pixels=sz_img_roi[0]*sz_img_roi[1]*sz_img_roi[2]*sz_img_roi[3];
		p_img_roi=new unsigned char[n_pixels]();
		if(!p_img_roi)
		{
			printf("ERROR:Fail to allocate memory for the roi image. \n");
			if(p_img_8u) 							{delete []p_img_8u;				p_img_8u=0;}
			if(p_img_input && !paras.b_imgfromV3D) 	{delete []p_img_input;			p_img_input=0;}
			if(sz_img_input) 						{delete []sz_img_input;			sz_img_input=0;}
			return false;
		}

		long pgsz_x=sz_img_input[0];
		long pgsz_xy=sz_img_input[0]*sz_img_input[1];
		long pgsz_xyz=sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
		long pgsz_x_roi=sz_img_roi[0];
		long pgsz_xy_roi=sz_img_roi[0]*sz_img_roi[1];
		long pgsz_xyz_roi=sz_img_roi[0]*sz_img_roi[1]*sz_img_roi[2];
		for(long x_roi=0;x_roi<sz_img_roi[0];x_roi++)
			for(long y_roi=0;y_roi<sz_img_roi[1];y_roi++)
				for(long z_roi=0;z_roi<sz_img_roi[2];z_roi++)
				{
					long x=x_roi+x_min;
					long y=y_roi+y_min;
					long z=z_roi+z_min;
					if(roils_roi[0].containsPoint(QPoint(x,y),Qt::OddEvenFill) &&
                              roils_roi[1].containsPoint(QPoint(z,y),Qt::OddEvenFill) &&
                              roils_roi[2].containsPoint(QPoint(x,z),Qt::OddEvenFill))
					{
						for(long c=0;c<sz_img_roi[3];c++)
						{
							long ind=pgsz_xyz*c+pgsz_xy*z+pgsz_x*y+x;
							long ind_roi=pgsz_xyz_roi*c+pgsz_xy_roi*z_roi+pgsz_x_roi*y_roi+x_roi;
							p_img_roi[ind_roi]=p_img_8u[ind];
						}
					}
				}
          //		saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/roi.raw",(unsigned char *)p_img_roi,sz_img_roi,1);

		//modify the marker position according to the new roi definition
		for(long i=0;i<vec2d_markers.size();i++)
		{
			vec2d_markers[i][0]-=x_min;
			vec2d_markers[i][1]-=y_min;
			vec2d_markers[i][2]-=z_min;
		}
     }

     //------------------------------------------------------------------------------------------------------------------------------------
     printf("5. Straighten. \n");
     unsigned char *p_img_output=0;
     long *sz_img_output=0;
     if(!q_celegans_straighten(callback,paras,
               p_img_roi,sz_img_roi,
               vec2d_markers,
               p_img_output,sz_img_output))
     {
          v3d_msg(QString("q_celegans_straighten() return false!"));
          if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
          if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
          return false;
     }
     //		if(!q_celegans_straighten_manual(callback,paras,
     //				p_img_roi,sz_img_roi,
     //				vec2d_markers,
     //				p_img_output,sz_img_output))
     //		{
     //			v3d_msg(QString("q_celegans_straighten_manual() return false!"));
     //			if(p_img_roi) 								{delete []p_img_roi;			p_img_roi=0;}
     //			if(p_img_8u) 								{delete []p_img_8u;				p_img_8u=0;}
     //			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
     //			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
     //			return false;
     //		}

     //------------------------------------------------------------------------------------------------------------------------------------
     printf("6. Save straightened image. \n");
     saveImage(qPrintable(qs_filename_strimg_output),(unsigned char *)p_img_output,sz_img_output,1);
     v3d_msg(QString("Strenghtened image is saved to:\n %1").arg(qs_filename_strimg_output),0);

     //------------------------------------------------------------------------------------------------------------------------------------
     //free memory
     printf(">>Free memory\n");
     if(p_img_output) 						{delete []p_img_output;			p_img_output=0;}
     if(p_img_roi) 							{delete []p_img_roi;			p_img_roi=0;}
     if(p_img_8u) 							{delete []p_img_8u;				p_img_8u=0;}
     if(p_img_input && !paras.b_imgfromV3D) 	{delete []p_img_input;			p_img_input=0;}
     if(sz_img_input) 						{delete []sz_img_input;			sz_img_input=0;}

     printf(">>Program exit successful!\n");
     return true;

}
