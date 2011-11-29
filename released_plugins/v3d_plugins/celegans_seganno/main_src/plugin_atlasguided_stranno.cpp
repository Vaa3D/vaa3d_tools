//plugin_atlasguided_stranno.cpp
// by Lei Qu
//2010-11-01

#include <QtGui>

#include "stackutil.h"
//#include "q_paradialog.h"
#include "plugin_atlasguided_stranno.h"
#include "../celegans_straighten/q_celegans_straighten.h"
#include "../../v3d_main/worm_straighten_c/spline_cubic.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"
#include "q_atlasguided_seganno.h"
#include "q_paradialog_stranno.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(plugin_atlasguided_stranno, AtlasGuidedStrAnnoPartialPlugin);

const QString title = "AtlasGuidedStrAnnoPlugin demo";

void AtlasGuidedStrAnno(V3DPluginCallback &callback, QWidget *parent);
bool readCelloi_file(const QString &qs_filename,QList<QString> &ql_celloi);

//************************************************************************************************************************************
QStringList AtlasGuidedStrAnnoPartialPlugin::menulist() const
{
    return QStringList()
	<< tr("atlasguided_stranno...");
}

void AtlasGuidedStrAnnoPartialPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if(menu_name==tr("atlasguided_stranno..."))
	{
		AtlasGuidedStrAnno(callback,parent);
	}
}

//************************************************************************************************************************************
void AtlasGuidedStrAnno(V3DPluginCallback &callback, QWidget *parent)
{
	CParaDialog_stranno DLG_stranno(callback,parent);
	if(DLG_stranno.exec()!=QDialog::Accepted)
		return;

	CSParas paras_str;
	//image and marker
	paras_str.b_imgfromV3D=1;
	paras_str.b_markerfromV3D=1;
	QString qs_filename_img="";
	QString qs_filename_marker_input="";
	QString qs_filename_strimg_output="";
	//visualization
	paras_str.b_showinV3D_pts=0;
	//skeleton detection
	paras_str.l_refchannel=1;
	paras_str.d_downsampleratio=2;
	paras_str.l_ctlpts_num=0;
	paras_str.l_radius_openclose=0;
	paras_str.d_fgthresh_xy=0;
	paras_str.d_fgthresh_xz=0;
	//straighten
	paras_str.l_radius_cuttingplane=50;

	CParas paras_anno;
	//image and atlas
	QString qs_filename_atals_input=DLG_stranno.lineEdit_atlasfile->text();
	QString qs_filename_celloi_input=DLG_stranno.lineEdit_celloifile->text();
	QString qs_filename_atals_output=DLG_stranno.lineEdit_atlas_output->text();
	//visualization
	paras_anno.b_showinV3D_img=0;
	paras_anno.b_showinV3D_pts=0;
	//initial align
	paras_anno.b_atlasupsidedown=0;
	paras_anno.l_refchannel=DLG_stranno.lineEdit_refchannel->text().toLong()-1;
	paras_anno.d_downsampleratio=DLG_stranno.lineEdit_downsampleratio->text().toDouble();
	paras_anno.d_T=DLG_stranno.lineEdit_temp_ini->text().toDouble();
	paras_anno.d_T_min=DLG_stranno.lineEdit_temp_min->text().toDouble();
	paras_anno.d_annealingrate=DLG_stranno.lineEdit_annealingrate->text().toDouble();
	paras_anno.l_niter_pertemp=DLG_stranno.lineEdit_niter_pertemp->text().toLong();
	//refine align
	paras_anno.b_ref_simplealign=1;
	paras_anno.d_ref_T=0.1;
	paras_anno.l_ref_cellradius=8;
	paras_anno.l_ref_maxiter=100;
	paras_anno.d_ref_minposchange=1.0;


	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Import image. \n");
	unsigned char *p_img_input=0;
	long *sz_img_input=0;
	int datatype_input=0;
	if(!paras_str.b_imgfromV3D)
	{
		if(qs_filename_img.isEmpty())
		{
			v3d_msg(QString("invalid image path!"));
			return;
		}
		if(!loadImage((char *)qPrintable(qs_filename_img),p_img_input,sz_img_input,datatype_input))
		{
			v3d_msg(QString("open file [%1] failed!").arg(qs_filename_img));
			return;
		}
		printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img));
	}
	else
	{
		printf("\t>>import image from V3D. \n");
		v3dhandleList h_wndlist=callback.getImageWindowList();
		if(h_wndlist.size()<1)
		{
			v3d_msg(QString("Make sure there are at least 1 image in V3D!"));
			return;
		}
		Image4DSimple* image=callback.getImage(callback.currentImageWindow());
		p_img_input=image->getRawData();
		sz_img_input=new long[4]();
		sz_img_input[0]=image->getXDim();	sz_img_input[1]=image->getYDim();	sz_img_input[2]=image->getZDim();	sz_img_input[3]=image->getCDim();
		datatype_input=image->getDatatype();
	}
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);
	printf("\t\tdatatype: %d\n",datatype_input);
	if(datatype_input!=1)
	{
		v3d_msg(QString("Need image datatype==1(Uint8) !"));
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Import markers. \n");
	//get initial head and tail marker position from V3D
	vector< vector<double> > vec2d_markers;
	if(!paras_str.b_markerfromV3D)
	{
		if(qs_filename_marker_input.isEmpty())
		{
			v3d_msg(QString("Invalid marker path!"));
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}
		QList<ImageMarker> ql_markers=readMarker_file(qs_filename_marker_input);
		printf("\t>>read %d markers from file: %s.\n",ql_markers.size(),qPrintable(qs_filename_marker_input));
		if(ql_markers.size()<2)
		{
			v3d_msg(QString("Need right at least 2 markers!"));
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}
		vector<double> vec_marker(3,0);
		for(long i=0;i<ql_markers.size();i++)
		{
			vec_marker[0]=ql_markers[i].x;
			vec_marker[1]=ql_markers[i].y;
			vec_marker[2]=ql_markers[i].z;
			vec2d_markers.push_back(vec_marker);
		}
	}
	else
	{
		printf("\t>>import markers from V3D. \n");
		v3dhandleList h_wndlist=callback.getImageWindowList();
		if(h_wndlist.size()<1)
		{
			v3d_msg(QString("Make sure there are at least 1 image in V3D!"));
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}
		LandmarkList ml_makers=callback.getLandmark(callback.currentImageWindow());
		if(ml_makers.size()<2)
		{
			v3d_msg(QString("Need right at least 2 markers!"));
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
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
	printf("3. Read atlas apo file. \n");
	QList<CellAPO> ql_atlasapo;
	ql_atlasapo=readAPO_file(qs_filename_atals_input);
	printf("\t>>read %d points from [%s]\n",ql_atlasapo.size(),qPrintable(qs_filename_atals_input));
	if(ql_atlasapo.size()<=0)
	{
		v3d_msg(QString("Given atlas file is empty or invalid!"));
		if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;			p_img_input=0;}
		if(sz_img_input) 							{delete []sz_img_input;			sz_img_input=0;}
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. Read interesting cell file. \n");
	QList<QString> ql_celloi;
	if(!readCelloi_file(qs_filename_celloi_input,ql_celloi))
	{
		printf("ERROR: readCelloi_file() return false! \n");
		if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
		return;
	}

	printf("\t>>interesting cell:\n");
	for(long i=0;i<ql_celloi.size();i++)
		printf("\t\t%s\n",qPrintable(ql_celloi[i]));


	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. Do Straightening. \n");
	unsigned char *p_strimg=0;
	long *sz_strimg=0;
	vector< vector< vector< vector<long> > > > vec4d_mappingfield_str2ori;

	long l_width=paras_str.l_radius_cuttingplane*2+1;
	QList<ImageMarker> ql_marker;
	for(long i=0;i<vec2d_markers.size();i++)
	{
		ImageMarker tmp;
		tmp.x=vec2d_markers[i][0];
		tmp.y=vec2d_markers[i][1];
		tmp.z=vec2d_markers[i][2];
		ql_marker.push_back(tmp);
	}

	if(!q_celegans_restacking_xy(
			p_img_input,sz_img_input,
			ql_marker,l_width,
			p_strimg,sz_strimg,
			vec4d_mappingfield_str2ori))
	{
		printf("ERROR: q_celegans_restacking_xy() return false! \n");
		if(p_img_input && !paras_str.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
		return;
	}


	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. Do Annotation. \n");
	QList<CellAPO> ql_musclecell_output;
	if(!q_atlas2image(paras_anno,callback,
			p_strimg,sz_strimg,ql_atlasapo,ql_celloi,
			ql_musclecell_output))
	{
		printf("ERROR: q_atlas2image() return false!\n");
		if(p_strimg)								{delete []p_strimg; 			p_strimg=0;}
		if(sz_strimg) 								{delete []sz_strimg;			sz_strimg=0;}
		if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;			p_img_input=0;}
		if(sz_img_input) 							{delete []sz_img_input;			sz_img_input=0;}
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("7. Map the annotated cell back to ori image. \n");
	QList<CellAPO> ql_musclecell_output_ori(ql_musclecell_output);
	for(long i=0;i<ql_musclecell_output.size();i++)
	{
		long x=ql_musclecell_output[i].x;
		long y=ql_musclecell_output[i].y;
		long z=ql_musclecell_output[i].z;
		ql_musclecell_output_ori[i].x=vec4d_mappingfield_str2ori[y][x][z][0];
		ql_musclecell_output_ori[i].y=vec4d_mappingfield_str2ori[y][x][z][1];
		ql_musclecell_output_ori[i].z=vec4d_mappingfield_str2ori[y][x][z][2];
	}
	writeAPO_file(qPrintable(qs_filename_atals_output),ql_musclecell_output_ori);

	//show deformed atlas pts in V3D
	v3dhandle curwin=callback.currentImageWindow();
	callback.open3DWindow(curwin);
	LandmarkList curlist;
	for(int i=0;i<ql_musclecell_output_ori.size();i++)
	{
		LocationSimple s;
		s.x=ql_musclecell_output_ori[i].x+1;//note: marker coord start from 1 instead of 0
		s.y=ql_musclecell_output_ori[i].y+1;//note: marker coord start from 1 instead of 0
		s.z=ql_musclecell_output_ori[i].z+1;//note: marker coord start from 1 instead of 0
		s.name=ql_musclecell_output_ori[i].name.toStdString();
		s.radius=10;

		curlist << s;
	}
	callback.setLandmark(curwin,curlist);
	callback.updateImageWindow(curwin);
	callback.pushObjectIn3DWindow(curwin);


	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	printf(">>Free memory\n");
	if(p_strimg)								{delete []p_strimg; 			p_strimg=0;}
	if(sz_strimg) 								{delete []sz_strimg;			sz_strimg=0;}
	if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;			p_img_input=0;}
	if(sz_img_input) 							{delete []sz_img_input;			sz_img_input=0;}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf(">>Program exit successful!\n");
	return;
}

bool readCelloi_file(const QString &qs_filename,QList<QString> &ql_celloi)
{
	//check paras
	if(qs_filename.isEmpty())
	{
		printf("ERROR: Invalid input file name.\n");
		return false;
	}
	if(!ql_celloi.isEmpty())
	{
		printf("WARNING: ql_celloi is not empty, original data will be deleted.\n");
		ql_celloi.clear();
	}

	QFile qf_file(qs_filename);
	if(!qf_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		printf("ERROR: open file error!\n");
		return false;
	}

    while(!qf_file.atEnd())
    {
		char curline[2000];
        qf_file.readLine(curline, sizeof(curline));

		QString qs_cellname=QString(curline).simplified().toUpper();
		if(qs_cellname.isEmpty())   	continue;
		if(qs_cellname.contains("#")) 	continue;

		ql_celloi.push_back(qs_cellname);
    }

	return true;
}
