//plugin_warp_affine_tps.cpp
//by Lei Qu
//2010-03-22

#include <QtGui>
#include <stdio.h>
#include "basic_surf_objs.h"
#include "stackutil.h"
#include "basic_memory.cpp"//note: should not include .h file, since they are template functions

#include "plugin_warp_affine_tps.h"
#include "q_warp_affine_tps_dialogs.h"
#include "q_warp_affine_tps.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(plugin_warp_affin_tps, WarpAffineTPSPlugin);

void WarpPointsetBaseonMatchedPairs(V3DPluginCallback &callback, QWidget *parent);
void WarpImageBaseonMatchedPairs(V3DPluginCallback &callback, QWidget *parent);

const QString title = "WarpAffineTPSPlugin demo";

QStringList WarpAffineTPSPlugin::menulist() const
{
    return QStringList()
	<< tr("warp pointset based on given matched point pairs...")
	<< tr("warp image based on given matched point pairs...");
}

void WarpAffineTPSPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if(menu_name==tr("warp pointset based on given matched point pairs..."))
	{
		WarpPointsetBaseonMatchedPairs(callback, parent);
	}
	else if(menu_name==tr("warp image based on given matched point pairs..."))
	{
		WarpImageBaseonMatchedPairs(callback, parent);
	}
}


//************************************************************************************************************************************
void WarpPointsetBaseonMatchedPairs(V3DPluginCallback &callback, QWidget *parent)
{
	//get parameters
	QString qs_filename_marker_tar,qs_filename_marker_sub,qs_filename_sub,qs_filename_matchpairind;
	QString qs_filename_sub2tar_affine,qs_filename_sub2tar_affine_tps;
	bool b_scaleradius=0;

	//WarpAffineTpsDialog_pts warpaffinetpsDLG_pts(parent);
	WarpAffineTpsDialog_pts warpaffinetpsDLG_pts(0);
	if(warpaffinetpsDLG_pts.exec()==QDialog::Accepted)
	{
		//input
		qs_filename_marker_tar			=warpaffinetpsDLG_pts.m_pLineEdit_filepath_tar_ctl->text();
		qs_filename_marker_sub			=warpaffinetpsDLG_pts.m_pLineEdit_filepath_sub_ctl->text();
		qs_filename_sub					=warpaffinetpsDLG_pts.m_pLineEdit_filepath_sub_warp->text();
		qs_filename_matchpairind		=warpaffinetpsDLG_pts.m_pLineEdit_filepath_matchind->text();
		b_scaleradius					=warpaffinetpsDLG_pts.m_pCheckBox_scaleradius->isChecked();
		//output
		qs_filename_sub2tar_affine		=warpaffinetpsDLG_pts.m_pLineEdit_filepath_sub2tar_affine->text();
		qs_filename_sub2tar_affine_tps	=warpaffinetpsDLG_pts.m_pLineEdit_filepath_sub2tar_affine_tps->text();
	}
	else
		return;

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>Pointset and image warp based on the matched control points\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input target control marker file:          %s\n",qPrintable(qs_filename_marker_tar));
	printf(">>  input subject control marker file:         %s\n",qPrintable(qs_filename_marker_sub));
	printf(">>  input to be warped pointset:               %s\n",qPrintable(qs_filename_sub));
	printf(">>  input matched-paris index file:            %s\n",qPrintable(qs_filename_matchpairind));
	printf(">>  input scale radius flag:                   %d\n",b_scaleradius);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output affine sub2tar warped file:         %s\n",qPrintable(qs_filename_sub2tar_affine));
	printf(">>  output affine+tps sub2tar warped file:     %s\n",qPrintable(qs_filename_sub2tar_affine_tps));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target and subject marker files. \n");
	QList<ImageMarker> ql_marker_tar,ql_marker_sub;
	if(qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
    {
		ql_marker_tar=readMarker_file(qs_filename_marker_tar);
		ql_marker_sub=readMarker_file(qs_filename_marker_sub);
    	printf("\t>>Target: read %d markers from [%s]\n",ql_marker_tar.size(),qPrintable(qs_filename_marker_tar));
    	printf("\t>>Subject:read %d markers from [%s]\n",ql_marker_sub.size(),qPrintable(qs_filename_marker_sub));
    }
    else
    {
    	printf("ERROR: at least one marker file is invalid.\n");
    	return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Read matched-pair index file and extract matched pairs. \n");
	vector<Coord3D_PCM> vec_tar,vec_sub;

	//read matched-pair index file
	vector< vector<long> > vec2D_sub2tar_matchind;	//n*2; [*][0]: sub_index; [*][1]: tar_index
	if(qs_filename_matchpairind.endsWith(".match"))	//.match is provided
	{
		if(!q_readMatchInd_file(qs_filename_matchpairind,vec2D_sub2tar_matchind))
		{
			printf("ERROR: q_readMatchInd_file() return false.\n");
			return;
		}

		//print matched-pair index
		for(long i=0;i<vec2D_sub2tar_matchind.size();i++)
		{
			printf("\tsub[%ld]-->tar[%ld]\n",vec2D_sub2tar_matchind[i][0],vec2D_sub2tar_matchind[i][1]);
		}

		//extract matched pairs according to the matched-pair index
		for(long i=0;i<vec2D_sub2tar_matchind.size();i++)
		{
			long tar_ind=vec2D_sub2tar_matchind[i][1]-1;
			long sub_ind=vec2D_sub2tar_matchind[i][0]-1;
			vec_tar.push_back(Coord3D_PCM(ql_marker_tar[tar_ind].x,ql_marker_tar[tar_ind].y,ql_marker_tar[tar_ind].z));
			vec_sub.push_back(Coord3D_PCM(ql_marker_sub[sub_ind].x,ql_marker_sub[sub_ind].y,ql_marker_sub[sub_ind].z));
		}

		printf("\t>>extract matched pairs according to the given .match file. \n");
	}
	else	//.match file isnt provided
	{
		long l_minlength=min(ql_marker_tar.size(),ql_marker_sub.size());
		for(long i=0;i<l_minlength;i++)
		{
			vec_tar.push_back(Coord3D_PCM(ql_marker_tar[i].x,ql_marker_tar[i].y,ql_marker_tar[i].z));
			vec_sub.push_back(Coord3D_PCM(ql_marker_sub[i].x,ql_marker_sub[i].y,ql_marker_sub[i].z));
		}

		printf("\t>>no .match file provided, assume first %ld pairs are matched-pairs. \n",l_minlength);
	}

	//estimate the scale factor (assume isotropic for simplicity)
	double d_scalefactor=1.0;
	{
	//find mass center
	Coord3D_PCM coord_masscenter_before,coord_masscenter_after;
	for(long i=0;i<vec_tar.size();i++)
	{
		coord_masscenter_before.x+=vec_sub[i].x;
		coord_masscenter_before.y+=vec_sub[i].y;
		coord_masscenter_before.z+=vec_sub[i].z;
		coord_masscenter_after.x+=vec_tar[i].x;
		coord_masscenter_after.y+=vec_tar[i].y;
		coord_masscenter_after.z+=vec_tar[i].z;
	}
	coord_masscenter_before.x/=vec_tar.size();
	coord_masscenter_before.y/=vec_tar.size();
	coord_masscenter_before.z/=vec_tar.size();
	coord_masscenter_after.x/=vec_tar.size();
	coord_masscenter_after.y/=vec_tar.size();
	coord_masscenter_after.z/=vec_tar.size();
	//compute the avg dis to mass center
	double d_avgdis2mcenter_before=0.0,d_avgdis2mcenter_after=0.0,tmp,dx,dy,dz;
	for(long i=0;i<vec_tar.size();i++)
	{
		dx=vec_sub[i].x-coord_masscenter_before.x;
		dy=vec_sub[i].y-coord_masscenter_before.y;
		dz=vec_sub[i].z-coord_masscenter_before.z;
		tmp=sqrt(dx*dx+dy*dy+dz*dz);
		d_avgdis2mcenter_before+=tmp;

		dx=vec_tar[i].x-coord_masscenter_after.x;
		dy=vec_tar[i].y-coord_masscenter_after.y;
		dz=vec_tar[i].z-coord_masscenter_after.z;
		tmp=sqrt(dx*dx+dy*dy+dz*dz);
		d_avgdis2mcenter_after+=tmp;
	}
	d_scalefactor=d_avgdis2mcenter_after/d_avgdis2mcenter_before;
	printf("\t>>sub2tar scale factor: %f\n",d_scalefactor);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. estimate the affine matrix. \n");
	Matrix x4x4_affinematrix;
	if(!q_affine_compute_affinmatrix_3D(vec_tar,vec_sub,x4x4_affinematrix))	//B=T*A
	{
		printf("ERROR: q_affine_compute_affinmatrix_2D() return false.\n");
		return;
	}

	//print affine matrix
	for(long row=1;row<=x4x4_affinematrix.nrows();row++)
	{
		printf("\t");
		for(long col=1;col<=x4x4_affinematrix.ncols();col++)
			printf("%.3f\t",x4x4_affinematrix(row,col));
		printf("\n");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. read subject file. \n");
	NeuronTree nt_sub_towarp;
	QList<ImageMarker> ql_marker_sub_towarp;
	if(qs_filename_sub.endsWith(".swc"))
    {
		nt_sub_towarp=readSWC_file(qs_filename_sub);
    	printf("\t>>read %d nodes from swc file [%s]\n",nt_sub_towarp.listNeuron.size(),qPrintable(qs_filename_sub));
    }
	else if(qs_filename_sub.endsWith(".marker"))
    {
		ql_marker_sub_towarp=readMarker_file(qs_filename_sub);
    	printf("\t>>read %d markers from file [%s]\n",ql_marker_sub_towarp.size(),qPrintable(qs_filename_sub));
    }
	else
	{
		printf("ERROR: un-support data type! \n");
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. affine warp subject file. \n");
	NeuronTree nt_sub2tar_affine(nt_sub_towarp);
	QList<ImageMarker> ql_marker_sub_affine(ql_marker_sub_towarp);
	if(qs_filename_sub.endsWith(".swc"))
	{
		long n_pt=nt_sub_towarp.listNeuron.size();
		Matrix x_vec_sub(4,n_pt),x_sub2tar_affine(4,n_pt);
		for(long i=0;i<n_pt;i++)
		{
			x_vec_sub(1,i+1)=nt_sub_towarp.listNeuron[i].x;
			x_vec_sub(2,i+1)=nt_sub_towarp.listNeuron[i].y;
			x_vec_sub(3,i+1)=nt_sub_towarp.listNeuron[i].z;
			x_vec_sub(4,i+1)=1.0;
		}
		x_sub2tar_affine=x4x4_affinematrix.i()*x_vec_sub;
		for(long i=0;i<n_pt;i++)
		{
			nt_sub2tar_affine.listNeuron[i].x=x_sub2tar_affine(1,i+1)/x_sub2tar_affine(4,i+1);
			nt_sub2tar_affine.listNeuron[i].y=x_sub2tar_affine(2,i+1)/x_sub2tar_affine(4,i+1);
			nt_sub2tar_affine.listNeuron[i].z=x_sub2tar_affine(3,i+1)/x_sub2tar_affine(4,i+1);

			//scale radius
			if(b_scaleradius)	nt_sub2tar_affine.listNeuron[i].r*=d_scalefactor;
		}

		if(qs_filename_sub2tar_affine.endsWith(".swc"))
			writeSWC_file(qPrintable(qs_filename_sub2tar_affine),nt_sub2tar_affine);
	}
	else if(qs_filename_sub.endsWith(".marker"))
	{
		long n_pt=ql_marker_sub_towarp.size();
		Matrix x_vec_sub(4,n_pt),x_sub2tar_affine(4,n_pt);
		for(long i=0;i<n_pt;i++)
		{
			x_vec_sub(1,i+1)=ql_marker_sub_towarp[i].x;
			x_vec_sub(2,i+1)=ql_marker_sub_towarp[i].y;
			x_vec_sub(3,i+1)=ql_marker_sub_towarp[i].z;
			x_vec_sub(4,i+1)=1.0;
		}
		x_sub2tar_affine=x4x4_affinematrix.i()*x_vec_sub;
		for(long i=0;i<n_pt;i++)
		{
			ql_marker_sub_affine[i].x=x_sub2tar_affine(1,i+1)/x_sub2tar_affine(4,i+1);
			ql_marker_sub_affine[i].y=x_sub2tar_affine(2,i+1)/x_sub2tar_affine(4,i+1);
			ql_marker_sub_affine[i].z=x_sub2tar_affine(3,i+1)/x_sub2tar_affine(4,i+1);

			//scale radius
			if(b_scaleradius)	ql_marker_sub_affine[i].radius*=d_scalefactor;
		}

		if(qs_filename_sub2tar_affine.endsWith(".marker"))
			writeMarker_file(qPrintable(qs_filename_sub2tar_affine),ql_marker_sub_affine);
	}


	//affine warp subject markers to target
	vector<Coord3D_PCM> vec_sub2tar_affine(vec_sub);
	{
		long n_sub_marker=ql_marker_sub.size();
		Matrix x_vec_sub(4,n_sub_marker),x_sub2tar_affine(4,n_sub_marker);
		for(long i=0;i<n_sub_marker;i++)
		{
			x_vec_sub(1,i+1)=ql_marker_sub[i].x;
			x_vec_sub(2,i+1)=ql_marker_sub[i].y;
			x_vec_sub(3,i+1)=ql_marker_sub[i].z;
			x_vec_sub(4,i+1)=1.0;
		}
		x_sub2tar_affine=x4x4_affinematrix.i()*x_vec_sub;
		QList<ImageMarker> ql_marker_sub2tar_affine(ql_marker_sub);
		for(long i=0;i<n_sub_marker;i++)
		{
			ql_marker_sub2tar_affine[i].x=x_sub2tar_affine(1,i+1);
			ql_marker_sub2tar_affine[i].y=x_sub2tar_affine(2,i+1);
			ql_marker_sub2tar_affine[i].z=x_sub2tar_affine(3,i+1);
			vec_sub2tar_affine[i].x=x_sub2tar_affine(1,i+1);
			vec_sub2tar_affine[i].y=x_sub2tar_affine(2,i+1);
			vec_sub2tar_affine[i].z=x_sub2tar_affine(3,i+1);
		}
//		wirteMarker_file("sub2tar_affine.marker",ql_marker_sub2tar_affine);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. TPS warp affine-warped file to target. \n");

	//compute sub2tar tps warp parameters
	//tps_para_wa=[w;a], where w is a n_marker*1 vector and a is a 3*1 vector
	Matrix x_tpspara_wa_sub2tar;
	if(!q_compute_tps_paras_3D(vec_sub2tar_affine,vec_tar,x_tpspara_wa_sub2tar))
	{
		printf("ERROR: q_compute_tps_paras_3D() return false.\n");
		return;
	}

	//TPS warp subject to target
	NeuronTree nt_sub2tar_affine_tps(nt_sub2tar_affine);
	QList<ImageMarker> ql_marker_sub_affine_tps(ql_marker_sub_affine);
	if(qs_filename_sub.endsWith(".swc"))
	{
		Coord3D_PCM pt_sub,pt_sub_tps;
		for(long i=0;i<nt_sub2tar_affine.listNeuron.size();i++)
		{
			pt_sub.x=nt_sub2tar_affine.listNeuron[i].x;
			pt_sub.y=nt_sub2tar_affine.listNeuron[i].y;
			pt_sub.z=nt_sub2tar_affine.listNeuron[i].z;

			if(!q_compute_ptwarped_from_tpspara_3D(pt_sub,vec_sub2tar_affine,x_tpspara_wa_sub2tar,pt_sub_tps))
			{
				printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
				return;
			}

			nt_sub2tar_affine_tps.listNeuron[i].x=pt_sub_tps.x;
			nt_sub2tar_affine_tps.listNeuron[i].y=pt_sub_tps.y;
			nt_sub2tar_affine_tps.listNeuron[i].z=pt_sub_tps.z;
		}

		if(qs_filename_sub2tar_affine_tps.endsWith(".swc"))
			writeSWC_file(qPrintable(qs_filename_sub2tar_affine_tps),nt_sub2tar_affine_tps);
	}
	else if(qs_filename_sub.endsWith(".marker"))
	{
		Coord3D_PCM pt_sub,pt_sub_tps;
		for(long i=0;i<ql_marker_sub_affine.size();i++)
		{
			pt_sub.x=ql_marker_sub_affine[i].x;
			pt_sub.y=ql_marker_sub_affine[i].y;
			pt_sub.z=ql_marker_sub_affine[i].z;

			if(!q_compute_ptwarped_from_tpspara_3D(pt_sub,vec_sub2tar_affine,x_tpspara_wa_sub2tar,pt_sub_tps))
			{
				printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
				return;
			}

			ql_marker_sub_affine_tps[i].x=pt_sub_tps.x;
			ql_marker_sub_affine_tps[i].y=pt_sub_tps.y;
			ql_marker_sub_affine_tps[i].z=pt_sub_tps.z;
		}

		if(qs_filename_sub2tar_affine_tps.endsWith(".marker"))
			writeMarker_file(qPrintable(qs_filename_sub2tar_affine_tps),ql_marker_sub_affine_tps);
	}

	//TPS warp subject markers to target
	vector<Coord3D_PCM> vec_sub2tar_affineTPS;
	{
		Coord3D_PCM pt_sub_tps;
		for(long i=0;i<vec_sub2tar_affine.size();i++)
		{
			if(!q_compute_ptwarped_from_tpspara_3D(vec_sub2tar_affine[i],vec_sub2tar_affine,x_tpspara_wa_sub2tar,pt_sub_tps))
			{
				printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
				return;
			}
			vec_sub2tar_affineTPS.push_back(pt_sub_tps);
		}

		QList<ImageMarker> ql_marker_sub2tar_affineTPS(ql_marker_sub);
		for(long i=0;i<ql_marker_sub.size();i++)
		{
			ql_marker_sub2tar_affineTPS[i].x=vec_sub2tar_affineTPS[i].x;
			ql_marker_sub2tar_affineTPS[i].y=vec_sub2tar_affineTPS[i].y;
			ql_marker_sub2tar_affineTPS[i].z=vec_sub2tar_affineTPS[i].z;
		}
//		wirteMarker_file("sub2tar_affine_tps.marker",ql_marker_sub2tar_affineTPS);
	}

	//save affine and TPS warped markers to apo file
	{
		char *filename_apo="sub2tar_affine_tps.apo";

		//save to apo file
		FILE *fp;
		fp=fopen(filename_apo,"w");
		if(fp==NULL)
		{
			fprintf(stderr,"ERROR: Failed to open file to write! \n");
			return;
		}
		int n=0;
		double volsize=10;	//size of ball
		for(int i=0;i<ql_marker_tar.size();i++)	//tar-red
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,255,0,0\n",n,n,ql_marker_tar[i].z,ql_marker_tar[i].x,ql_marker_tar[i].y,volsize);
			n++;
		}
		for(int i=0;i<ql_marker_sub.size();i++)	//sub_affine-green
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,255,0\n",n,n,vec_sub2tar_affine[i].z,vec_sub2tar_affine[i].x,vec_sub2tar_affine[i].y,volsize);
			n++;
		}
		for(int i=0;i<ql_marker_sub.size();i++)	//sub_affine-blue
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,0,255\n",n,n,vec_sub2tar_affineTPS[i].z,vec_sub2tar_affineTPS[i].x,vec_sub2tar_affineTPS[i].y,volsize);
			n++;
		}
		fclose(fp);

		printf("done with saving file: %s\n",filename_apo);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//display output info dialog
	QStringList qsl_outputinfo;
	qsl_outputinfo.push_back("Warped results were saved to:");
	if(qs_filename_sub2tar_affine.endsWith(".swc") || qs_filename_sub2tar_affine.endsWith(".marker"))
		qsl_outputinfo.push_back(QString("[%1]").arg(qs_filename_sub2tar_affine));
	if(qs_filename_sub2tar_affine_tps.endsWith(".swc") || qs_filename_sub2tar_affine_tps.endsWith(".marker"))
		qsl_outputinfo.push_back(QString("[%1]").arg(qs_filename_sub2tar_affine_tps));

	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle(QObject::tr("Output information"));
	msgBox.setText(qsl_outputinfo.join("\n"));
	msgBox.exec();

}

//************************************************************************************************************************************
void WarpImageBaseonMatchedPairs(V3DPluginCallback &callback, QWidget *parent)
{
	//get parameters
	QString qs_filename_marker_tar,qs_filename_marker_sub,qs_filename_sub;
	QString qs_filename_sub2tar_affine,qs_filename_sub2tar_tps;

	WarpAffineTpsDialog_img WarpAffineTpsDialog_img(parent);
	if(WarpAffineTpsDialog_img.exec()==QDialog::Accepted)
	{
		//input
		qs_filename_marker_tar			=WarpAffineTpsDialog_img.m_pLineEdit_filepath_tar_ctl->text();
		qs_filename_marker_sub			=WarpAffineTpsDialog_img.m_pLineEdit_filepath_sub_ctl->text();
		qs_filename_sub					=WarpAffineTpsDialog_img.m_pLineEdit_filepath_sub_warp->text();
		//output
		qs_filename_sub2tar_affine		=WarpAffineTpsDialog_img.m_pLineEdit_filepath_sub2tar_affine->text();
		qs_filename_sub2tar_tps			=WarpAffineTpsDialog_img.m_pLineEdit_filepath_sub2tar_tps->text();
	}
	else
		return;

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>Pointset and image warp based on the matched control points\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input target control marker file:          %s\n",qPrintable(qs_filename_marker_tar));
	printf(">>  input subject control marker file:         %s\n",qPrintable(qs_filename_marker_sub));
	printf(">>  input to be warped image:                  %s\n",qPrintable(qs_filename_sub));
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output affine sub2tar warped file:         %s\n",qPrintable(qs_filename_sub2tar_affine));
	printf(">>  output affine+tps sub2tar warped file:     %s\n",qPrintable(qs_filename_sub2tar_tps));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	bool b_saveaffine=0,b_savetps=0;
	if(qs_filename_sub2tar_affine.endsWith(".raw") || qs_filename_sub2tar_affine.endsWith(".tif")) b_saveaffine=1;
	if(qs_filename_sub2tar_tps.endsWith(".raw") || qs_filename_sub2tar_tps.endsWith(".tif")) b_savetps=1;

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target and subject marker files. \n");
	QList<ImageMarker> ql_marker_tar,ql_marker_sub;
	if(qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
    {
		ql_marker_tar=readMarker_file(qs_filename_marker_tar);
		ql_marker_sub=readMarker_file(qs_filename_marker_sub);
    	printf("\t>>Target: read %d markers from [%s]\n",ql_marker_tar.size(),qPrintable(qs_filename_marker_tar));
    	printf("\t>>Subject:read %d markers from [%s]\n",ql_marker_sub.size(),qPrintable(qs_filename_marker_sub));
    }
    else
    {
    	printf("ERROR: at least one marker file is invalid.\n");
    	return;
	}

	//re-formate to vector
	vector<Coord3D_PCM> vec_tar,vec_sub;
	long l_minlength=min(ql_marker_tar.size(),ql_marker_sub.size());
	for(long i=0;i<l_minlength;i++)
	{
		vec_tar.push_back(Coord3D_PCM(ql_marker_tar[i].x,ql_marker_tar[i].y,ql_marker_tar[i].z));
		vec_sub.push_back(Coord3D_PCM(ql_marker_sub[i].x,ql_marker_sub[i].y,ql_marker_sub[i].z));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Read subject image. \n");
	unsigned char *p_img_sub=0;
	long *sz_img_sub=0;
	int datatype_sub=0;
	if(!loadImage((char *)qPrintable(qs_filename_sub),p_img_sub,sz_img_sub,datatype_sub))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_sub));
		return;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
	printf("\t\tdatatype: %d\n",datatype_sub);

	if(datatype_sub!=1)
	{
    	printf("ERROR: Input image datatype is not UINT8.\n");
    	return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. Affine or TPS transform the subject image to target. \n");
	QStringList qsl_outputinfo;
	qsl_outputinfo.push_back("Warped results were saved to:");

	unsigned char *p_img_sub2tar=0;

	//do affine warping
	if(b_saveaffine)
	{
		if(!q_imagewarp_affine(vec_tar,vec_sub,
				p_img_sub,sz_img_sub,
				p_img_sub2tar))
		{
			printf("ERROR: q_imagewarp_affine return false!\n");
			if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
			if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
			return;
		}

		//save image
		saveImage(qPrintable(qs_filename_sub2tar_affine),p_img_sub2tar,sz_img_sub,1);
		qsl_outputinfo.push_back(QString("[%1]").arg(qs_filename_sub2tar_affine));
		if(p_img_sub2tar) 			{delete []p_img_sub2tar;			p_img_sub2tar=0;}
	}

	//do tps warping
	if(b_savetps)
	{
		if(!q_imagewarp_tps(vec_tar,vec_sub,
				p_img_sub,sz_img_sub,
				p_img_sub2tar))
		{
			printf("ERROR: q_imagewarp_tps() return false!\n");
			if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
			if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
			return;
		}

		//save image
		saveImage(qPrintable(qs_filename_sub2tar_tps),p_img_sub2tar,sz_img_sub,1);
		qsl_outputinfo.push_back(QString("[%1]").arg(qs_filename_sub2tar_tps));
		if(p_img_sub2tar) 			{delete []p_img_sub2tar;			p_img_sub2tar=0;}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//display output info dialog
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle(QObject::tr("Output information"));
	msgBox.setText(qsl_outputinfo.join("\n"));
	msgBox.exec();

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. free memory. \n");
	if(p_img_sub2tar) 		{delete []p_img_sub2tar;		p_img_sub2tar=0;}
	if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
	if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}

}
