/* plugin_littlequickwarp.cpp
 * 2012-07-16 : by Lei Qu
 */

#include <iostream>
using namespace std;
#include "v3d_message.h"
#include "../../basic_c_fun/stackutil.h"

#include "plugin_littlequickwarp.h"
#include "q_paradialog_littlequickwarp.h"
#include "q_imgwarp_tps_quicksmallmemory.cpp"


Q_EXPORT_PLUGIN2(littlequickwarp, LittleQuickWarpPlugin);

void printHelp();
bool littlequickwarp(QString qs_filename_img_sub,QString qs_filename_marker_sub,QString qs_filename_marker_tar,
		int i_interpmethod_df,int i_interpmethod_img,bool b_padding,bool b_resizeimg,V3DLONG sz_resize_x,V3DLONG sz_resize_y,V3DLONG sz_resize_z,
		QString qs_filename_img_warp);

QStringList LittleQuickWarpPlugin::menulist() const
{
	return QStringList() 
        <<tr("littlequickwarp")
		<<tr("about");
}
QStringList LittleQuickWarpPlugin::funclist() const
{
	return QStringList()
		<<tr("littlequickwarp")
		<<tr("help");
}
void LittleQuickWarpPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("littlequickwarp"))
	{
    	CParaDialog_littlequickwarp DLG_littlequickwarp(callback,parent);
        if(DLG_littlequickwarp.exec()!=QDialog::Accepted)	return;
    	//get parameters
    	QString qs_filename_img_sub=DLG_littlequickwarp.lineEdit_img_sub->text();
    	QString qs_filename_img_warp=DLG_littlequickwarp.lineEdit_img_warp->text();
    	QString qs_filename_marker_sub=DLG_littlequickwarp.lineEdit_marker_sub->text();
    	QString qs_filename_marker_tar=DLG_littlequickwarp.lineEdit_marker_tar->text();
    	int i_interpmethod_df=DLG_littlequickwarp.radioButton_df_interp_bspline->isChecked();
    	int i_interpmethod_img=DLG_littlequickwarp.radioButton_img_interp_nn->isChecked();
    	bool b_padding=DLG_littlequickwarp.checkBox_padding->isChecked();
    	bool b_resizeimg=DLG_littlequickwarp.groupBox_resize->isChecked();
    	V3DLONG sz_resize_x=DLG_littlequickwarp.lineEdit_Xdim->text().toLong();
    	V3DLONG sz_resize_y=DLG_littlequickwarp.lineEdit_Ydim->text().toLong();
    	V3DLONG sz_resize_z=DLG_littlequickwarp.lineEdit_Zdim->text().toLong();
    	//do warping
    	if(!littlequickwarp(qs_filename_img_sub,qs_filename_marker_sub,qs_filename_marker_tar,
    			i_interpmethod_df,i_interpmethod_img,b_padding,b_resizeimg,sz_resize_x,sz_resize_y,sz_resize_z,
    			qs_filename_img_warp))
    	{
    		v3d_msg(tr("ERROR: littlequickwarp() return false!"));
    		return;
    	}
	}
	else
	{
		v3d_msg(tr("Warp image based on given markers (fast and with small memory consumption). "
			"Developed by Lei Qu, 2012-07-16"));
	}
}
bool LittleQuickWarpPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("littlequickwarp"))
	{
		cout<<"============== Welcome to littlequickwarp function ================="<<endl;
		if(input.size()!=2 || output.size()!=1 || ((vector<char*> *)(input.at(1).p))->size()!=7)
		{
			v3d_msg(tr("ERROR: no enough para!"));
			printHelp();
			return false;
		}

		//get paras
		QString qs_filename_img_sub=((vector<char*> *)(input.at(0).p))->at(0);
		QString qs_filename_marker_sub=((vector<char*> *)(input.at(0).p))->at(1);
		QString qs_filename_marker_tar=((vector<char*> *)(input.at(0).p))->at(2);
		QString qs_filename_img_warp=((vector<char*> *)(output.at(0).p))->at(0);
		vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
		int i_interpmethod_df=atoi(paras.at(0));;
		int i_interpmethod_img=atoi(paras.at(1));
		bool b_padding=atoi(paras.at(2));
		bool b_resizeimg=atoi(paras.at(3));
		V3DLONG sz_resize_x=atoi(paras.at(4));
		V3DLONG sz_resize_y=atoi(paras.at(5));
		V3DLONG sz_resize_z=atoi(paras.at(6));

	   	//do warping
		if(!littlequickwarp(qs_filename_img_sub,qs_filename_marker_sub,qs_filename_marker_tar,
				i_interpmethod_df,i_interpmethod_img,b_padding,b_resizeimg,sz_resize_x,sz_resize_y,sz_resize_z,
				qs_filename_img_warp))
		{
			v3d_msg(tr("ERROR: littlequickwarp() return false!"));
			return false;
		}
		return true;
	}
	else
	{
		printHelp();
	}
}

void printHelp()
{
    printf("\nUsage: v3d -x <littlequickwarp> -f littlequickwarp -i <input_image_sub> <input_marker_sub> <input_marker_tar> -o <output_image_file> -p interpmethod_df interpmethod_img dopadding doresize newsize_x newsize_y newsize_z\n");
    printf("\t input_image_sub:         input image file to be warped (subject image)\n");
    printf("\t input_marker_sub:        marker file in subject image (markers in this file define the control points in subject image)\n");
    printf("\t input_marker_tar:        marker file in target image (markers in this file define the control points in target image)\n");
    printf("\t output_image_file:       output warped image file(warped image)\n");
    printf("\t interpmethod_df:         displace field interpolation method (0:trilinear, 1:B-spline)\n");
    printf("\t interpmethod_img:        image interpolation method (0:trilinear, 1:nearest neighbor)\n");
    printf("\t dopadding:               pad image before warping? (0:nopadding, 1:padding) - just for generating same results as JBA\n");
    printf("\t doresize:                resize the warped image? (0:warped image size will be same as subject image, 1:resize warped image according to the next 3 paras)\n");
    printf("\t newsize_x:               x dim of resized image (works only if doresize=1)\n");
    printf("\t newsize_y:               y dim of resized image (works only if doresize=1)\n");
    printf("\t newsize_z:               z dim of resized image (works only if doresize=1)\n");
    printf("Demo :\t v3d -x littlequickwarp -f littlequickwarp -i /Users/qul/Desktop/testdata/output_global.tiff /Users/qul/Desktop/testdata/output_subject.marker /Users/qul/Desktop/testdata/output_target.marker -o /Users/qul/Desktop/testdata/output_warp_littlequick.v3draw -p 1 0 0 0 1 1 1\n");
    return;
}

bool littlequickwarp(QString qs_filename_sub,QString qs_filename_marker_sub,QString qs_filename_marker_tar,
		int i_interpmethod_df,int i_interpmethod_img,bool b_padding_img,bool b_resizeimg,V3DLONG sz_resize_x,V3DLONG sz_resize_y,V3DLONG sz_resize_z,
		QString qs_filename_warp)
{
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>Quick small image warping:\n");
	printf(">> 	quick <- TPS+linear\n");
	printf(">>	small <- warp block by block\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input subject image:           %s\n",qPrintable(qs_filename_sub));
	printf(">>  input target  marker:          %s\n",qPrintable(qs_filename_marker_tar));
	printf(">>  input subject marker:          %s\n",qPrintable(qs_filename_marker_sub));
	printf(">>  DF  interp method:             %d\n",i_interpmethod_df);
	printf(">>  img interp method:             %d\n",i_interpmethod_img);
	printf(">>  padding image?:                %d\n",b_padding_img);
	printf(">>  resize image?:                 %d\n",b_resizeimg);
	printf(">>  new image size:               [%d,%d,%d]\n",sz_resize_x,sz_resize_y,sz_resize_z);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output warped image:           %s\n",qPrintable(qs_filename_warp));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	V3DLONG time_total_start,time_warp_start;
	time_total_start=clock();
	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target and subject marker files. \n");
	QList<ImageMarker> ql_marker_tar,ql_marker_sub;
	if(qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
    {
		ql_marker_tar=readMarker_file(qs_filename_marker_tar);
		ql_marker_sub=readMarker_file(qs_filename_marker_sub);
    	printf("\t>>Target: read %d markers from [%s]\n",ql_marker_tar.size(),qPrintable(qs_filename_marker_tar));
    	printf("\t>>Subject:read %d markers from [%s]\n",ql_marker_sub.size(),qPrintable(qs_filename_marker_sub));

    	if(ql_marker_tar.size()==0 || ql_marker_tar.size()!=ql_marker_sub.size())
        {
        	printf("ERROR: marker number not equal or has zero marker.\n");
        	return false;
    	}
    }
    else
    {
    	printf("ERROR: at least one marker file is invalid.\n");
    	return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Read subject image. \n");
	unsigned char *p_img_sub=0;
	V3DLONG *sz_img_sub=0;
	int datatype_sub=0;
	if(!loadImage((char *)qPrintable(qs_filename_sub),p_img_sub,sz_img_sub,datatype_sub))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_sub));
		return false;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
	printf("\t\tdatatype: %d\n",datatype_sub);

	if(datatype_sub!=1 && datatype_sub!=2 && datatype_sub!=4)
	{
    	printf("ERROR: Input image datatype is not valid, return.\n");
    	if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
    	if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}
    	return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. warp the subject image block by block. \n");
	time_warp_start=clock();
	unsigned char *p_img_warp=0;
	V3DLONG szBlock_x,szBlock_y,szBlock_z;
	szBlock_x=szBlock_y=szBlock_z=4;

	bool b_status=false;
    if(datatype_sub==1)
    {
    	if(!b_padding_img)
    		b_status=imgwarp_smallmemory(p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_df,i_interpmethod_img,p_img_warp);
    	else
    		b_status=imgwarp_smallmemory_padding(p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,p_img_warp);
    }
    else if(datatype_sub==2)
    {
    	if(!b_padding_img)
    		b_status=imgwarp_smallmemory((unsigned short int *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_df,i_interpmethod_img,(unsigned short int *&)p_img_warp);
    	else
    		b_status=imgwarp_smallmemory_padding((unsigned short int *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,(unsigned short int *&)p_img_warp);
    }
    else if(datatype_sub==4)
    {
    	if(!b_padding_img)
    		b_status=imgwarp_smallmemory((float *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_df,i_interpmethod_img,(float *&)p_img_warp);
    	else
    		b_status=imgwarp_smallmemory_padding((float *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,(float *&)p_img_warp);
    }
    if(!b_status)
	{
		printf("ERROR: imgwarp_smallmemory() return false.\n");
		if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
		if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}
		return false;
	}
	printf(">>>>The warping process took %f seconds\n",((float)(clock()-time_warp_start))/CLOCKS_PER_SEC);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. Resize the warped image: [%d]\n",b_resizeimg);
	V3DLONG sz_img_warp[4];
	for(int i=0;i<4;i++) sz_img_warp[i]=sz_img_sub[i];
	if(b_resizeimg)
	{
		//check paras
		if(sz_resize_x==0 || sz_resize_y==0 || sz_resize_z==0)
		{
	    	printf("ERROR: Input image size is not valid, return.\n");
	    	return false;
		}
		if(!(sz_resize_x==sz_img_sub[0] && sz_resize_y==sz_img_sub[1] && sz_resize_z==sz_img_sub[2]))
		{
			b_status=false;
			if(datatype_sub==1)
				b_status=imgresize_padding(p_img_warp,sz_img_warp,sz_resize_x,sz_resize_y,sz_resize_z);
			else if(datatype_sub==2)
				b_status=imgresize_padding((unsigned short int *&)p_img_warp,sz_img_warp,sz_resize_x,sz_resize_y,sz_resize_z);
			else if(datatype_sub==4)
				b_status=imgresize_padding((float *&)p_img_warp,sz_img_warp,sz_resize_x,sz_resize_y,sz_resize_z);
			if(!b_status)
			{
				printf("ERROR: imgresize_padding() return false.\n");
				if(p_img_warp) 			{delete []p_img_warp;		p_img_warp=0;}
			   	if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
				if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}
				return false;
			}
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. save warped image to file: [%s]\n",qPrintable(qs_filename_warp));
	if(!saveImage(qPrintable(qs_filename_warp),p_img_warp,sz_img_warp,datatype_sub))
	{
		printf("ERROR: saveImage() return false.\n");
		if(p_img_warp) 			{delete []p_img_warp;		p_img_warp=0;}
	   	if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
		if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. free memory. \n");
	if(p_img_warp) 			{delete []p_img_warp;		p_img_warp=0;}
	if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
	if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}

	printf(">>>>The whole process took %f seconds\n",((float)(clock()-time_total_start))/CLOCKS_PER_SEC);
	printf("Program exit success.\n");
	return true;
}
