//plugin_atlasguided_seganno_prior.cpp
// by Lei Qu
//2010-09-27

#include <QtGui>

#include "stackutil.h"
#include "q_paradialog_prior.h"
#include "plugin_atlasguided_seganno_prior.h"
#include "q_atlasguided_seganno.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(plugin_atlasguided_seganno_prior, AtlasGuidedSegAnnoPriorPlugin);

const QString title = "AtlasGuidedSegAnnoPriorPlugin demo";

void AtlasGuidedSegAnnoPrior(V3DPluginCallback &callback, QWidget *parent);

//************************************************************************************************************************************
QStringList AtlasGuidedSegAnnoPriorPlugin::menulist() const
{
    return QStringList()
	<< tr("atlasguided_seganno_prior...");
}

void AtlasGuidedSegAnnoPriorPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if(menu_name==tr("atlasguided_seganno_prior..."))
	{
		AtlasGuidedSegAnnoPrior(callback,parent);
	}
}

//************************************************************************************************************************************
void AtlasGuidedSegAnnoPrior(V3DPluginCallback &callback, QWidget *parent)
{
	ParaDialog paraDlg(callback,parent);
	if(paraDlg.exec()==QDialog::Accepted)
	{
		CParas paras;
		//image and atlas
		paras.b_imgfromV3D=paraDlg.radioButton_imgv3d->isChecked();
		QString qs_filename_atals_input=paraDlg.lineEdit_atlasfile->text();
		QString qs_filename_atals_output=paraDlg.lineEdit_atlas_output->text();
		//align paras
		paras.b_atlasupsidedown=paraDlg.checkBox_wormupsidedown->isChecked();
		paras.l_refchannel=paraDlg.lineEdit_refchannel->text().toLong();
		paras.d_downsampleratio=paraDlg.lineEdit_downsampleratio->text().toDouble();
		paras.d_T_min=paraDlg.lineEdit_temp->text().toDouble();
		paras.d_fgthresh_factor=paraDlg.lineEdit_fgthresh->text().toDouble();
		paras.d_ref_minposchange=0.1;
		paras.b_imgfromV3D=1;

		printf("===================================================\n");
		printf("Image & Atlas I/O \n");
		printf(">>qs_filename_atlasinput: 	%s\n",qPrintable(qs_filename_atals_input));
		printf(">>qs_filename_atlasoutput: 	%s\n",qPrintable(qs_filename_atals_output));
		printf("Initial align \n");
		printf(">>b_atlasupsidedown:        %d\n",paras.b_atlasupsidedown);
		printf(">>l_refchannel:             %d\n",paras.l_refchannel);
		printf(">>d_downsampleratio:        %f\n",paras.d_downsampleratio);
		printf(">>d_T_min:                  %f\n",paras.d_T_min);
		printf(">>d_fgthresh_factor:        %f\n",paras.d_fgthresh_factor);
		printf(">>d_ref_minposchange:       %f\n",paras.d_ref_minposchange);
		printf("===================================================\n");

		//------------------------------------------------------------------------------------------------------------------------------------
		//1. Import image and markers
		unsigned char *p_img_input=0;
		long *sz_img_input=0;
		int datatype_input=0;
		LandmarkList ml_makers;

		printf("1. Import image and markers from V3D. \n");
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
		if(datatype_input!=1)
		{
			v3d_msg(QString("Image type need to be UINT8!"));
			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}

		ml_makers=callback.getLandmark(callback.currentImageWindow());
		if(ml_makers.size()<3)
		{
			v3d_msg(QString("Need at least 3 markers!"));
			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}
		//marker coord start from 1 instead of 0
		for(long i=0;i<ml_makers.size();i++)
		{
			ml_makers[i].x-=1;
			ml_makers[i].y-=1;
			ml_makers[i].z-=1;
		}

		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);
		printf("\t\tdatatype: %d\n",datatype_input);
		printf("\t>>n_makers: %d\n",ml_makers.size());

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
		if(!q_atlas2image_prior(paras,callback,
				p_img_input,sz_img_input,ql_atlasapo,ml_makers,
				ql_musclecell_output))
		{
			printf("ERROR: q_atlas2image_prior() return false!\n");
			if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return;
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("4. Save deformed atlas to file. \n");
		writeAPO_file(qPrintable(qs_filename_atals_output),ql_musclecell_output);

		//------------------------------------------------------------------------------------------------------------------------------------
		//show result in V3D
		v3dhandle curwin=callback.currentImageWindow();
		callback.open3DWindow(curwin);

		//now push the data to the 3d viewer's display
		LandmarkList curlist;
		for(int i=0;i<ql_musclecell_output.size();i++)
		{
			LocationSimple s;
			s.x=ql_musclecell_output[i].x+1;//note: marker coord start from 1 instead of 0
			s.y=ql_musclecell_output[i].y+1;//note: marker coord start from 1 instead of 0
			s.z=ql_musclecell_output[i].z+1;//note: marker coord start from 1 instead of 0
			s.name=ql_musclecell_output[i].name.toStdString();
			s.radius=10;

			curlist << s;
		}
		callback.setLandmark(curwin,curlist);
		callback.updateImageWindow(curwin);
		callback.pushObjectIn3DWindow(curwin);


		//------------------------------------------------------------------------------------------------------------------------------------
		printf(">>Free memory\n");
		if(p_img_input && !paras.b_imgfromV3D) 		{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}

		printf(">>Program exit success!\n");
	}
	else
		return;
}
