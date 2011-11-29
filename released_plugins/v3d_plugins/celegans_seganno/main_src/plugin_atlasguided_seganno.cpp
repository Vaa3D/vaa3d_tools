//plugin_atlasguided_seganno.cpp
// by Lei Qu
//2010-07-31

#include <QtGui>

#include "stackutil.h"
#include "q_paradialog.h"
#include "plugin_atlasguided_seganno.h"
#include "q_atlasguided_seganno.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(plugin_atlasguided_seganno, AtlasGuidedSegAnnoPlugin);

const QString title = "AtlasGuidedSegAnnoPlugin demo";

void AtlasGuidedSegAnno(V3DPluginCallback &callback, QWidget *parent);

//************************************************************************************************************************************
QStringList AtlasGuidedSegAnnoPlugin::menulist() const
{
    return QStringList()
	<< tr("atlasguided_seganno...");
}

void AtlasGuidedSegAnnoPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if(menu_name==tr("atlasguided_seganno..."))
	{
		AtlasGuidedSegAnno(callback,parent);
	}
}

//************************************************************************************************************************************
void AtlasGuidedSegAnno(V3DPluginCallback &callback, QWidget *parent)
{
	ParaDialog paraDlg(callback,parent);
	if(paraDlg.exec()==QDialog::Accepted)
	{
		CParas paras;
		//image and atlas
		paras.b_imgfromV3D=paraDlg.radioButton_imgv3d->isChecked();
		QString qs_filename_img=paraDlg.lineEdit_imfile->text();
		QString qs_filename_atals_input=paraDlg.lineEdit_atlasfile->text();
		QString qs_filename_atals_output=paraDlg.lineEdit_atlas_output->text();
		//visualization
		paras.b_showinV3D_img=paraDlg.checkBox_showV3D_img->isChecked();
		paras.b_showinV3D_pts=paraDlg.checkBox_showV3D_pts->isChecked();
		//initial align
		paras.b_atlasupsidedown=paraDlg.checkBox_wormupsidedown->isChecked();
		paras.l_refchannel=paraDlg.lineEdit_refchannel->text().toLong();
		paras.d_downsampleratio=paraDlg.lineEdit_downsampleratio->text().toDouble();
		paras.d_T=paraDlg.lineEdit_temp_ini->text().toDouble();
		paras.d_T_min=paraDlg.lineEdit_temp_min->text().toDouble();
		paras.d_annealingrate=paraDlg.lineEdit_annealingrate->text().toDouble();
		paras.l_niter_pertemp=paraDlg.lineEdit_niter_pertemp->text().toLong();
		//refine align
		paras.b_ref_simplealign=paraDlg.checkBox_refinealign_simple->isChecked();
		paras.d_ref_T=paraDlg.lineEdit_ref_temperature->text().toDouble();
		paras.l_ref_cellradius=paraDlg.lineEdit_ref_cellradius->text().toLong();
		paras.l_ref_maxiter=paraDlg.lineEdit_ref_maxiter->text().toLong();
		paras.d_ref_minposchange=paraDlg.lineEdit_ref_minposchange->text().toDouble();

//		CParas paras;
//		bool b_imgfromv3d=0;
//		QString qs_filename_img;
//		for(long i=0;i<9;i++)
//		{
//		if(i==0){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1213061_crop_straight.raw";
//		paras.b_atlasupsidedown=0;}
//		else if(i==1){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1228061_crop_straight.raw";
//		paras.b_atlasupsidedown=0;}
//		else if(i==2){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1229061_crop_straight.raw";
//		paras.b_atlasupsidedown=0;}
//		else if(i==3){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1229062_crop_straight.raw";
//		paras.b_atlasupsidedown=1;}
//		else if(i==4){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1229063_crop_straight.raw";
//		paras.b_atlasupsidedown=1;}
//		else if(i==5){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904072_crop_straight.raw";
//		paras.b_atlasupsidedown=1;}
//		else if(i==6){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904073_crop_straight.raw";
//		paras.b_atlasupsidedown=1;}
//		else if(i==7){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904074_crop_straight.raw";
//		paras.b_atlasupsidedown=0;}
//		else if(i==8){
//		qs_filename_img="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904075_crop_straight.raw";
//		paras.b_atlasupsidedown=0;}
//
//		QString qs_filename_atals_input="/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/atlas.apo";
//		QString qs_filename_atals_output="";
//		paras.l_refchannel=1;
//		paras.d_downsampleratio=4;
//		paras.d_T=5;
//		paras.d_T_min=0.2;
//		paras.d_annealingrate=0.95;

		printf("===================================================\n");
		printf("Image & Atlas I/O \n");
		printf(">>b_imgfromv3d:             %d\n",paras.b_imgfromV3D);
		printf(">>qs_filename_img:          %s\n",qPrintable(qs_filename_img));
		printf(">>qs_filename_atlasinput: 	%s\n",qPrintable(qs_filename_atals_input));
		printf(">>qs_filename_atlasoutput: 	%s\n",qPrintable(qs_filename_atals_output));
		printf("Visualization \n");
		printf(">>b_showinV3D_img:          %d\n",paras.b_showinV3D_img);
		printf(">>b_showinV3D_pts:          %d\n",paras.b_showinV3D_pts);
		printf("Initial align \n");
		printf(">>b_atlasupsidedown:        %d\n",paras.b_atlasupsidedown);
		printf(">>l_refchannel:             %d\n",paras.l_refchannel);
		printf(">>d_downsampleratio:        %f\n",paras.d_downsampleratio);
		printf(">>d_T_ini:                  %f\n",paras.d_T);
		printf(">>d_T_min:                  %f\n",paras.d_T_min);
		printf(">>d_annealingrate:          %f\n",paras.d_annealingrate);
		printf(">>l_niter_pertemp:          %d\n",paras.l_niter_pertemp);
		printf("Refined align \n");
		printf(">>b_ref_simplealign:       %d\n",paras.b_ref_simplealign);
		printf(">>d_ref_T:                 %f\n",paras.d_ref_T);
		printf(">>l_ref_cellradius:        %d\n",paras.l_ref_cellradius);
		printf(">>l_ref_maxiter:           %d\n",paras.l_ref_maxiter);
		printf(">>d_ref_minposchange:      %f\n",paras.d_ref_minposchange);
		printf("===================================================\n");

		//------------------------------------------------------------------------------------------------------------------------------------
		//1. Import image
		unsigned char *p_img_input=0;
		long *sz_img_input=0;
		int datatype_input=0;
		if(!paras.b_imgfromV3D)
		{
			if(qs_filename_img.isEmpty())
			{
				v3d_msg(QString("invalid image path!"));
				return;
			}
			printf("1. Read image from file. \n");
			if(!loadImage((char *)qPrintable(qs_filename_img),p_img_input,sz_img_input,datatype_input))
			{
				v3d_msg(QString("open file [%1] failed!").arg(qs_filename_img));
				return;
			}
		}
		else
		{
			printf("1. Import image from V3D. \n");
			long l_imgind=paraDlg.comboBox_imgv3d->currentIndex();
			if(l_imgind<0)
			{
				v3d_msg(QString("Image index is invalid!\nMake sure there are at least 1 image opend in V3D!"));
				return;
			}
			Image4DSimple* image=callback.getImage(paraDlg.h_wndlist[l_imgind]);
			p_img_input=image->getRawData();
			sz_img_input=new long[4]();
			sz_img_input[0]=image->getXDim();	sz_img_input[1]=image->getYDim();	sz_img_input[2]=image->getZDim();	sz_img_input[3]=image->getCDim();
			datatype_input=image->getDatatype();
		}
		printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);
		printf("\t\tdatatype: %d\n",datatype_input);

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("2. Read atlas apo file. \n");
		QList<CellAPO> ql_atlasapo;
		ql_atlasapo=readAPO_file(qs_filename_atals_input);
		printf("\t>>read %d points from [%s]\n",ql_atlasapo.size(),qPrintable(qs_filename_atals_input));
		if(ql_atlasapo.size()<=0)
		{
			v3d_msg(QString("Given atlas file is empty or invalid!"));
			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("3. Align the cells onto image (currently only align the muscle cells). \n");
		QList<CellAPO> ql_musclecell_output;
		if(!q_atlas2image(paras,callback,
				p_img_input,sz_img_input,ql_atlasapo,
				ql_musclecell_output))
		{
			printf("ERROR: q_atlas2image() return false!\n");
			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("4. Save deformed atlas to file. \n");
		writeAPO_file(qPrintable(qs_filename_atals_output),ql_musclecell_output);
//		qs_filename_atals_output=QString("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/%1.apo").arg(i);
//		writeAPO_file(qPrintable(qs_filename_atals_output),ql_musclecell_output);

		//------------------------------------------------------------------------------------------------------------------------------------
		printf(">>Free memory\n");
		if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}

		printf(">>Program exit success!\n");
//		}
	}
	else
		return;
}
