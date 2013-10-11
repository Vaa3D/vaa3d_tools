// q_atlasguided_seganno.cpp
// by Lei Qu
// 2010-07-30

#include <math.h>
#include "stackutil.h"

#include "q_imresize.cpp"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "q_atlasguided_seganno.h"

CControlPanel::CControlPanel(V3DPluginCallback &_v3d, QWidget *parent) : QDialog(parent),v3d(_v3d)
{
	panel=this;
	b_stop=1;

	QPushButton *ok     = new QPushButton(QObject::tr("Pause"));
	QPushButton *cancel = new QPushButton(QObject::tr("Resume"));
	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow(ok, cancel);

	//QDialog d(parent);
	setLayout(formLayout);
	setWindowTitle(QString("Control Panel"));
	setWindowFlags(Qt::WindowStaysOnTopHint);

	connect(ok,     SIGNAL(clicked()), this, SLOT(_slot_stop()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(_slot_resume()));
	connect(this,   SIGNAL(_signal_stop()), this, SLOT(_slot_stop()));
}
CControlPanel::~CControlPanel()
{
	panel=0;
}
void CControlPanel::_slot_stop()
{
	b_stop=1;

	while(b_stop)
		QCoreApplication::processEvents();
}
void CControlPanel::_slot_resume()
{
	b_stop=0;
}
void CControlPanel::setstop()
{
	emit _signal_stop();
}

//warp the atlas onto the image
//(currently only aling the muscle cells)
bool q_atlas2image(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const QList<CellAPO> &ql_atlasapo,const QList<QString> &ql_celloi,
		QList<CellAPO> &ql_musclecell_output,unsigned char *&p_img8u_seglabel,COutputInfo &outputinfo)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(ql_atlasapo.size()==0)
	{
		printf("ERROR: Invalid input ql_atlasapo size!\n");
		return false;
	}
	if(ql_celloi.size()==0)
	{
		printf("ERROR: Invalid input ql_celloi size!\n");
		return false;
	}
	if(paras.l_refchannel<=0)
	{
		printf("ERROR: l_refchannel is invalid!\n");
		return false;
	}
	if(ql_musclecell_output.size()!=0)
	{
		printf("WARNNING: Output vec_musclecell_output is not empty, original data will be cleared!\n");
		ql_musclecell_output.clear();
	}
	if(p_img8u_seglabel)
	{
		printf("WARNNING: Output p_img8u_seglabel is not empty, original data will be cleared!\n");
		if(p_img8u_seglabel) 	{delete []p_img8u_seglabel;	p_img8u_seglabel=0;}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(1). Extract the interesting image channel. \n");
	unsigned char *p_img_1c=0;
	V3DLONG sz_img_1c[4]={sz_img[0],sz_img[1],sz_img[2],1};
	V3DLONG l_npixels_1c=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];

	{
	//allocate memeory
	p_img_1c=new unsigned char[l_npixels_1c]();
	if(!p_img_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img_1c!\n");
		return false;
	}
	//extract/combine referece channel(s)
	printf("\t>>extract channel:[%ld]\n",paras.l_refchannel);
	bool arr_validchannel[3]={0,0,0};
	int n_validchannel=0;
	QString qs_refchannel=QString("%1").arg(paras.l_refchannel);
	if(qs_refchannel.contains("1") && sz_img[3]>=1)	{arr_validchannel[0]=1; n_validchannel++;}
	if(qs_refchannel.contains("2") && sz_img[3]>=2)	{arr_validchannel[1]=1; n_validchannel++;}
	if(qs_refchannel.contains("3") && sz_img[3]>=3)	{arr_validchannel[2]=1; n_validchannel++;}

	V3DLONG pgsz_y=sz_img[0];
	V3DLONG pgsz_xy=sz_img[0]*sz_img[1];
	V3DLONG pgsz_xyz=sz_img[0]*sz_img[1]*sz_img[2];
	for(V3DLONG x=0;x<sz_img[0];x++)
		for(V3DLONG y=0;y<sz_img[1];y++)
			for(V3DLONG z=0;z<sz_img[2];z++)
			{
				V3DLONG ind_1c=pgsz_xy*z+pgsz_y*y+x;
				for(V3DLONG c=0;c<3;c++)
					if(arr_validchannel[c])
					{
						V3DLONG ind_ref=pgsz_xyz*c+ind_1c;
						p_img_1c[ind_1c]+=p_img8u[ind_ref]/n_validchannel+0.5;
					}
			}
//	saveImage("img_1c.raw",p_img_1c,sz_img_1c,1);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(2). Resize image and atlas point. \n");
	//resize image and compute actual resize ratio
	V3DLONG sz_img_s[4]={1,1,1,1};//w,h,z,c
	double arr_downratio_actual[3];
	for(V3DLONG i=0;i<3;i++)
	{
		sz_img_s[i]=sz_img_1c[i]/paras.d_downsampleratio+0.5;
		arr_downratio_actual[i]=(double)sz_img_1c[i]/(double)sz_img_s[i];
	}

	unsigned char *p_img_s=0;
	if(!q_imresize_3D(p_img_1c,sz_img_1c,1,sz_img_s,p_img_s))
	{
		printf("ERROR: q_imresize8u_3D() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		return false;
	}
	printf("\t>>sz_img_s:[%ld,%ld,%ld,%ld]\n",sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3]);
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/img_s.raw",p_img_s,sz_img_s,1);

	//resize atlas
	QList<CellAPO> ql_atlasapo_s(ql_atlasapo);
	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
//		ql_atlasapo_s[i].x/=arr_downratio_actual[0];
//		ql_atlasapo_s[i].y/=arr_downratio_actual[1];
//		ql_atlasapo_s[i].z/=arr_downratio_actual[2];
		ql_atlasapo_s[i].x/=arr_downratio_actual[0]*3;//better result
		ql_atlasapo_s[i].y/=arr_downratio_actual[1]*2;
		ql_atlasapo_s[i].z/=arr_downratio_actual[2];
		ql_atlasapo_s[i].volsize=5;
		RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
		ql_atlasapo_s[i].color=color;
	}
//	writeAPO_file("atlas_s.apo",ql_atlasapo);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(3). Extract interesting atlas points. \n");
	QList<CellAPO> ql_musclecell;
	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
		QString qs_cellname=ql_atlasapo_s[i].name;
		qs_cellname=qs_cellname.simplified();
		qs_cellname=qs_cellname.toUpper();
		ql_atlasapo_s[i].name=qs_cellname;

		for(V3DLONG j=0;j<ql_celloi.size();j++)
		{
			if(ql_celloi[j].contains("*"))
			{
				QString qs_cellnamevalid=ql_celloi[j];
				qs_cellnamevalid.remove("*");
				if(qs_cellname.contains(qs_cellnamevalid,Qt::CaseInsensitive))
				{
					ql_musclecell.push_back(ql_atlasapo_s[i]);
					ql_musclecell_output.push_back(ql_atlasapo_s[i]);
				}
			}
			else if(qs_cellname.compare(ql_celloi[j],Qt::CaseInsensitive)==0)
			{
				ql_musclecell.push_back(ql_atlasapo_s[i]);
				ql_musclecell_output.push_back(ql_atlasapo_s[i]);
			}
		}
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/atlas_muscle.apo",ql_musclecell);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(4). Rotate the atlas points upside down if necessary. \n");
	if(paras.b_atlasupsidedown)
	{
		printf("\t>>do rotate \n");
		double d_degree=180/180*3.1415926;
		//compute offset in yz plane
		double d_offset_y=sz_img_s[1]/2.0;
		double d_offset_z=sz_img_s[2]/2.0;
		//rotate
		QList<CellAPO> ql_tmp(ql_musclecell);
		for(V3DLONG i=0;i<ql_musclecell.size();i++)
		{
			ql_tmp[i].y= cos(d_degree)*(ql_musclecell[i].y-d_offset_y)-sin(d_degree)*(ql_musclecell[i].z-d_offset_z)+d_offset_y;
			ql_tmp[i].z=-sin(d_degree)*(ql_musclecell[i].y-d_offset_y)+cos(d_degree)*(ql_musclecell[i].z-d_offset_z)+d_offset_z;
		}
		ql_musclecell.clear();	ql_musclecell=ql_tmp;
	}
	else
		printf("\t>>do not rotate \n");
//	writeAPO_file("atlas_muscle.apo",ql_musclecell);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(5). Extract forground image region. \n");
	vector<V3DLONG> 		vec_fg_ind;
	vector<point3D64F> 	vec_fg_xyz;
	V3DLONG l_npixel_s=sz_img_s[0]*sz_img_s[1]*sz_img_s[2]*sz_img_s[3];

	{
	//compute the mean and std of image
	double d_img_mean=0.0,d_img_std=0.0;
	for(V3DLONG i=0;i<l_npixel_s;i++)
		d_img_mean+=p_img_s[i];
	d_img_mean/=l_npixel_s;
	for(V3DLONG i=0;i<l_npixel_s;i++)
	{
		double temp=p_img_s[i]-d_img_mean;
		d_img_std+=temp*temp;
	}
	d_img_std=sqrt(d_img_std/l_npixel_s);
	printf("\t>>img_mean=%.2f, img_std=%.2f\n",d_img_mean,d_img_std);
	//compute threshold
	double d_thresh_fg=d_img_mean+paras.d_fgthresh_factor*d_img_std;
//	double d_thresh_fg=d_img_mean;
	//extract foreground region
	V3DLONG pgsz_y=sz_img_s[0];
	V3DLONG pgsz_xy=sz_img_s[0]*sz_img_s[1];
	for(V3DLONG x=0;x<sz_img_s[0];x++)
		for(V3DLONG y=0;y<sz_img_s[1];y++)
			for(V3DLONG z=0;z<sz_img_s[2];z++)
			{
				V3DLONG ind=pgsz_xy*z+pgsz_y*y+x;
				if(p_img_s[ind]>d_thresh_fg)
				{
					vec_fg_ind.push_back(ind);
					vec_fg_xyz.push_back(point3D64F(x,y,z));
				}
			}
	printf("\t>>foregroud voxel number: %ld\n",V3DLONG(vec_fg_ind.size()));

//	{
//	unsigned char *p_img_tmp=new unsigned char[l_npixel_s]();
//	for(V3DLONG i=0;i<vec_fg_ind.size();i++)
//		p_img_tmp[vec_fg_ind[i]]=255;
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/img_fg.raw",p_img_tmp,sz_img_s,1);
//	delete []p_img_tmp;
//	}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//creat new image window in V3D and show the downsampled image
	if(paras.b_showatlas || paras.b_showsegmentation)
	{
		v3dhandle newwin=callback.newImageWindow();
		unsigned char* newdata1d=new unsigned char[l_npixel_s]();
		memcpy(newdata1d,p_img_s,l_npixel_s);
		Image4DSimple tmp;
		tmp.setData(newdata1d,sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3],V3D_UINT8);
		callback.setImage(newwin,&tmp);
		callback.open3DWindow(newwin);
		callback.updateImageWindow(newwin);
		callback.pushImageIn3DWindow(newwin);

		//allow user to adjust the property of window before enter into the iteration loop
		//actually within the loop, we have another event mechanism to allow the user interaction, but that is less efficient
		if(CControlPanel::panel)
		{
			CControlPanel::panel->show();
			CControlPanel::panel->setstop();
		}
		else
		{
			CControlPanel *q_controlpanel=new CControlPanel(callback,paras.qw_rootparent);
			q_controlpanel->show();
			q_controlpanel->setstop();
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(6). Initial align atlas with image (deterministic annealing). \n");
	//format convert
	vector<point3D64F> vec_musclecell,vec_musclecell_affine;
	for(V3DLONG i=0;i<ql_musclecell.size();i++)
		vec_musclecell.push_back(point3D64F(ql_musclecell[i].x,ql_musclecell[i].y,ql_musclecell[i].z));

	//affine align the atlas onto image
	vector< vector<double> > vec2d_labelprob;
	if(!q_atlas2image_musclecell_ini_affine(paras,callback,
			p_img_s,sz_img_s,vec_musclecell,
			vec_fg_ind,vec_fg_xyz,
			vec_musclecell_affine,vec2d_labelprob,outputinfo))
	{
		printf("ERROR: q_atlas2image_musclecell_ini_affine() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}

	//generate segmentation label image based on assignment matrix
	{
	p_img8u_seglabel=new unsigned char[l_npixel_s]();
	if(!p_img8u_seglabel)
	{
		printf("ERROR: fail to allocate memory for p_img8u_seglabel!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}

	//fill seglabel image (use the same algorithm as the segmentation visualization)
	vector< vector<unsigned V3DLONG> > vec2d_cellpixelgroups(vec_musclecell.size(),vector<unsigned V3DLONG>());
	double d_celldiameter_max=paras.l_ref_cellradius/paras.d_downsampleratio*2;
	for(unsigned V3DLONG i=0;i<vec_fg_ind.size();i++)
	{
		//for a given pixel, find the cell index with highest prob
		double d_maxprob=0;
		unsigned V3DLONG l_maxprob_ind=0;
		for(unsigned V3DLONG j=0;j<vec_musclecell.size();j++)
		{
			if(vec2d_labelprob[i][j]>d_maxprob)
			{
				d_maxprob=vec2d_labelprob[i][j];
				l_maxprob_ind=j;
			}
		}

		double dis,dif_x,dif_y,dif_z;
		dif_x=vec_fg_xyz[i].x-vec_musclecell_affine[l_maxprob_ind].x;
		dif_y=vec_fg_xyz[i].y-vec_musclecell_affine[l_maxprob_ind].y;
		dif_z=vec_fg_xyz[i].z-vec_musclecell_affine[l_maxprob_ind].z;
		dis=sqrt(dif_x*dif_x+dif_y*dif_y+dif_z*dif_z);
		if(dis<d_celldiameter_max)
			p_img8u_seglabel[vec_fg_ind[i]]=l_maxprob_ind+1;//+1 is for differing the first cell index 0 from background
	}

	//resize seglabel image to original size by using nearest neighbor interpolation
	unsigned char *p_img_tmp=0;
	if(!q_imresize_3D(p_img8u_seglabel,sz_img_s,2,sz_img_1c,p_img_tmp))
	{
		printf("ERROR: q_imresize_3D() return false!\n");
		if(p_img8u_seglabel){delete []p_img8u_seglabel; p_img8u_seglabel=0;}
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}
	if(p_img8u_seglabel) 	{delete []p_img8u_seglabel;	p_img8u_seglabel=0;}
	p_img8u_seglabel=p_img_tmp;	p_img_tmp=0;
	}


//	{
//	QList<CellAPO> ql_tmp(ql_musclecell);
//	RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
//	for(V3DLONG i=0;i<vec_musclecell_affine.size();i++)
//	{
//		ql_tmp[i].x=vec_musclecell_affine[i].x;
//		ql_tmp[i].y=vec_musclecell_affine[i].y;
//		ql_tmp[i].z=vec_musclecell_affine[i].z;
//		ql_tmp[i].volsize=5;
//		ql_tmp[i].color=color;
//	}
//	writeAPO_file("/groups/peng/home/qul/work/straightened/tmp_affine.apo",ql_tmp);
//	}

	//------------------------------------------------------------------------------------------------------------------------------------
//	printf("(7). Refine align the atlas. \n");
//	vector<point3D64F> vec_musclecell_ref;
//	if(paras.b_ref_simplealign)
//	{
//		if(!q_atlas2image_musclecell_ref_DAWC(paras,callback,
//				p_img_s,sz_img_s,vec_musclecell_affine,
//				vec_fg_ind,vec_fg_xyz,
//				vec_musclecell_ref))
////		if(!q_atlas2image_musclecell_ref_DATPS(paras,callback,
////				p_img_s,sz_img_s,vec_musclecell_affine,
////				vec_fg_ind,vec_fg_xyz,
////				vec_musclecell_ref))
//		{
//			printf("ERROR: q_atlas2image_musclecell_ref_DAWC() return false!\n");
//			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
//			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
//			return false;
//		}
//	}
//	else
//	{
//		if(!q_atlas2image_musclecell_ref_LHHV(paras,callback,
//				p_img_s,sz_img_s,vec_musclecell_affine,
//				vec_fg_ind,vec_fg_xyz,
//				vec_musclecell_ref))
//		{
//			printf("ERROR: q_atlas2image_musclecell_ref_LHHV() return false!\n");
//			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
//			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
//			return false;
//		}
//	}
//
////	{
////	QList<CellAPO> ql_tmp(ql_musclecell);
////	RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
////	for(V3DLONG i=0;i<vec_musclecell_affine.size();i++)
////	{
////		ql_tmp[i].x=vec_musclecell_ref[i].x;
////		ql_tmp[i].y=vec_musclecell_ref[i].y;
////		ql_tmp[i].z=vec_musclecell_ref[i].z;
////		ql_tmp[i].volsize=5;
////		ql_tmp[i].color=color;
////	}
////	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/tmp_noaffine.apo",ql_tmp);
////	}

	//------------------------------------------------------------------------------------------------------------------------------------
//	printf("(8). Refine the position of atlas points by mean-shift (on original size image). \n");
//	vector<point3D64F> vec_musclecell_mshift;
//
//	for(V3DLONG i=0;i<vec_musclecell_ref.size();i++)
//	{
//		vec_musclecell_ref[i].x*=arr_downratio_actual[0];
//		vec_musclecell_ref[i].y*=arr_downratio_actual[1];
//		vec_musclecell_ref[i].z*=arr_downratio_actual[2];
//	}
//
//	if(!q_atlas2image_musclecell_ref_mshift(paras,callback,
//			p_img_1c,sz_img_1c,vec_musclecell_ref,
//			vec_musclecell_mshift))
////	if(!q_atlas2image_musclecell_ref_mshift_adaptive(paras,callback,
////			p_img_1c,sz_img_1c,vec_musclecell_ref,d_thresh_fg,
////			vec_musclecell_mshift))
//	{
//		printf("ERROR: q_atlas2image_musclecell_ref_mshift_adaptive() return false!\n");
//		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
//		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
//		return false;
//	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(9). Prepare the final output. \n");
	//rescale the deformed atlas
	//+(arr_downratio_actual[0]+0.5) is for compensate the downsample error (image origin offset error)
	RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
	for(V3DLONG i=0;i<ql_musclecell_output.size();i++)
	{
		ql_musclecell_output[i].x=vec_musclecell_affine[i].x*arr_downratio_actual[0]+(arr_downratio_actual[0]+0.5);
		ql_musclecell_output[i].y=vec_musclecell_affine[i].y*arr_downratio_actual[1]+(arr_downratio_actual[1]+0.5);
		ql_musclecell_output[i].z=vec_musclecell_affine[i].z*arr_downratio_actual[2]+(arr_downratio_actual[2]+0.5);
//		ql_musclecell_output[i].x=vec_musclecell_ref[i].x*arr_downratio_actual[0]+(arr_downratio_actual[0]+0.5);
//		ql_musclecell_output[i].y=vec_musclecell_ref[i].y*arr_downratio_actual[1]+(arr_downratio_actual[1]+0.5);
//		ql_musclecell_output[i].z=vec_musclecell_ref[i].z*arr_downratio_actual[2]+(arr_downratio_actual[2]+0.5);
//			ql_musclecell_output[i].x=vec_musclecell_mshift[i].x;
//			ql_musclecell_output[i].y=vec_musclecell_mshift[i].y;
//			ql_musclecell_output[i].z=vec_musclecell_mshift[i].z;
		ql_musclecell_output[i].volsize=50;
		ql_musclecell_output[i].color=color;
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/tmp_final.apo",ql_musclecell_output);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(10). Post-precess (force the VP constraint). \n");
	//get muscle cell 4 bundle index
	vector< vector<V3DLONG> > vec2d_ind_bundlecell(4,vector<V3DLONG>());	//0:VR, 1:VL, 2:DR, 3:DL
	for(V3DLONG i=0;i<ql_musclecell_output.size();i++)
	{
		QString qs_cellname=ql_musclecell_output[i].name;
		if(qs_cellname.contains("BWMVR",Qt::CaseInsensitive))
			vec2d_ind_bundlecell[0].push_back(i);
		else if(qs_cellname.contains("BWMVL",Qt::CaseInsensitive))
			vec2d_ind_bundlecell[1].push_back(i);
		else if(qs_cellname.contains("BWMDR",Qt::CaseInsensitive))
			vec2d_ind_bundlecell[2].push_back(i);
		else if(qs_cellname.contains("BWMDL",Qt::CaseInsensitive))
			vec2d_ind_bundlecell[3].push_back(i);
	}
	//swap the VP violated cells
	if(vec2d_ind_bundlecell[0].size()!=0 && vec2d_ind_bundlecell[1].size()!=0 && vec2d_ind_bundlecell[2].size()!=0 && vec2d_ind_bundlecell[3].size()!=0)
	{
	bool b_swap=0;
	do
	{
		b_swap=0;
		for(V3DLONG i=0;i<4;i++)
			for(unsigned V3DLONG j=0;j<vec2d_ind_bundlecell[i].size()-1;j++)
			{
				if(ql_musclecell_output[vec2d_ind_bundlecell[i][j]].x > ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].x)
				{
					b_swap=1;
					printf("\t>>swap: %s <-> %s\n",
							qPrintable(ql_musclecell_output[vec2d_ind_bundlecell[i][j]].name),
							qPrintable(ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].name));

					CellAPO tmp;
					tmp.x=ql_musclecell_output[vec2d_ind_bundlecell[i][j]].x;
					tmp.y=ql_musclecell_output[vec2d_ind_bundlecell[i][j]].y;
					tmp.z=ql_musclecell_output[vec2d_ind_bundlecell[i][j]].z;
					ql_musclecell_output[vec2d_ind_bundlecell[i][j]].x=ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].x;
					ql_musclecell_output[vec2d_ind_bundlecell[i][j]].y=ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].y;
					ql_musclecell_output[vec2d_ind_bundlecell[i][j]].z=ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].z;
					ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].x=tmp.x;
					ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].y=tmp.y;
					ql_musclecell_output[vec2d_ind_bundlecell[i][j+1]].z=tmp.z;
				}
			}
	}while(b_swap!=0);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("Destroy control panel dialog\n");
	if(CControlPanel::panel)
	{
		delete CControlPanel::panel;
		CControlPanel::panel=0;
	}
	printf(">>Free memory\n");
	if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
	if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}

	return true;
}

bool q_atlas2image_partial(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const QList<CellAPO> &ql_atlasapo,const QList<QString> &ql_celloi,
		QList<CellAPO> &ql_musclecell_output,COutputInfo &outputinfo)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(ql_atlasapo.size()==0)
	{
		printf("ERROR: Invalid input ql_atlasapo size!\n");
		return false;
	}
	if(ql_celloi.size()==0)
	{
		printf("ERROR: Invalid input ql_celloi size!\n");
		return false;
	}
	if(paras.l_refchannel>sz_img[3] || paras.l_refchannel<=0)
	{
		printf("ERROR: l_refchannel is invalid!\n");
		return false;
	}
	if(ql_musclecell_output.size()!=0)
	{
		printf("WARNNING: Output vec_musclecell_output is not empty, original data will be cleared!\n");
		ql_musclecell_output.clear();
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(1). Extract the interesting image channel. \n");
	V3DLONG sz_img_1c[4]={sz_img[0],sz_img[1],sz_img[2],1};
	V3DLONG l_npixels_1c=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];
	//allocate memeory
	unsigned char *p_img_1c=0;
	p_img_1c=new unsigned char[l_npixels_1c]();
	if(!p_img_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img_1c!\n");
		return false;
	}
	//extract referece channel
	V3DLONG pgsz_y=sz_img[0];
	V3DLONG pgsz_xy=sz_img[0]*sz_img[1];
	V3DLONG pgsz_xyz=sz_img[0]*sz_img[1]*sz_img[2];
	printf("\t>>extract channel:[%ld]\n",paras.l_refchannel);
	for(V3DLONG x=0;x<sz_img[0];x++)
		for(V3DLONG y=0;y<sz_img[1];y++)
			for(V3DLONG z=0;z<sz_img[2];z++)
			{
				V3DLONG ind_1c=pgsz_xy*z+pgsz_y*y+x;
				V3DLONG ind_ref=pgsz_xyz*(paras.l_refchannel-1)+ind_1c;
				p_img_1c[ind_1c]=p_img8u[ind_ref];
			}
//	saveImage("img_1c.raw",p_img_1c,sz_img_1c,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(2). Resize image and atlas point. \n");
	//resize image
	V3DLONG sz_img_s[4]={1,1,1,1};//w,h,z,c
	for(V3DLONG i=0;i<3;i++)
		sz_img_s[i]=sz_img_1c[i]/paras.d_downsampleratio+0.5;

	unsigned char *p_img_s=0;
	if(!q_imresize_3D(p_img_1c,sz_img_1c,1,sz_img_s,p_img_s))
	{
		printf("ERROR: q_imresize_3D() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		return false;
	}
	printf("\t>>sz_img_s:[%ld,%ld,%ld,%ld]\n",sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3]);
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/img_s.raw",p_img_s,sz_img_s,1);

	//resize atlas
	QList<CellAPO> ql_atlasapo_s(ql_atlasapo);
	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
		ql_atlasapo_s[i].x/=paras.d_downsampleratio;
		ql_atlasapo_s[i].y/=paras.d_downsampleratio;
		ql_atlasapo_s[i].z/=paras.d_downsampleratio;
		ql_atlasapo_s[i].volsize=5;
		RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
		ql_atlasapo_s[i].color=color;
	}
//	writeAPO_file("atlas_s.apo",ql_atlasapo);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(3). Extract interesting atlas points. \n");
	QList<CellAPO> ql_musclecell;
	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
		QString qs_cellname=ql_atlasapo_s[i].name;
		qs_cellname=qs_cellname.simplified();
		qs_cellname=qs_cellname.toUpper();

		for(V3DLONG j=0;j<ql_celloi.size();j++)
		{
			if(ql_celloi[j].contains("*"))
			{
				QString qs_cellnamevalid=ql_celloi[j];
				qs_cellnamevalid.remove("*");
				if(qs_cellname.contains(qs_cellnamevalid,Qt::CaseInsensitive))
					ql_musclecell.push_back(ql_atlasapo_s[i]);
			}
			else if(qs_cellname.compare(ql_celloi[j],Qt::CaseInsensitive)==0)
				ql_musclecell.push_back(ql_atlasapo_s[i]);
		}
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/atlas_muscle.apo",ql_musclecell);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(4). Rotate the atlas points upside down if necessary. \n");
	if(paras.b_atlasupsidedown)
	{
		printf("\t>>do rotate \n");
		double d_degree=180/180*3.1415926;
		//compute offset in yz plane
		double d_offset_y=sz_img_s[1]/2.0;
		double d_offset_z=sz_img_s[2]/2.0;
		//rotate
		QList<CellAPO> ql_tmp(ql_musclecell);
		for(V3DLONG i=0;i<ql_musclecell.size();i++)
		{
			ql_tmp[i].y= cos(d_degree)*(ql_musclecell[i].y-d_offset_y)-sin(d_degree)*(ql_musclecell[i].z-d_offset_z)+d_offset_y;
			ql_tmp[i].z=-sin(d_degree)*(ql_musclecell[i].y-d_offset_y)+cos(d_degree)*(ql_musclecell[i].z-d_offset_z)+d_offset_z;
		}
		ql_musclecell.clear();	ql_musclecell=ql_tmp;
	}
	else
		printf("\t>>do not rotate \n");
//	writeAPO_file("atlas_muscle.apo",ql_musclecell);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(5). Extract forground image region. \n");
	V3DLONG l_npixel_s=sz_img_s[0]*sz_img_s[1]*sz_img_s[2]*sz_img_s[3];
	//compute the mean and std of image
	double d_img_mean=0.0,d_img_std=0.0;
	for(V3DLONG i=0;i<l_npixel_s;i++)
		d_img_mean+=p_img_s[i];
	d_img_mean/=l_npixel_s;
	for(V3DLONG i=0;i<l_npixel_s;i++)
	{
		double temp=p_img_s[i]-d_img_mean;
		d_img_std+=temp*temp;
	}
	d_img_std=sqrt(d_img_std/l_npixel_s);
	printf("\t>>img_mean=%.2f, img_std=%.2f\n",d_img_mean,d_img_std);
	//compute threshold
	double d_thresh_fg=d_img_mean+paras.d_fgthresh_factor*d_img_std;
	//extract foreground region
	vector<V3DLONG> 		vec_fg_ind;
	vector<point3D64F> 	vec_fg_xyz;
	pgsz_y=sz_img_s[0];
	pgsz_xy=sz_img_s[0]*sz_img_s[1];
	for(V3DLONG x=0;x<sz_img_s[0];x++)
		for(V3DLONG y=0;y<sz_img_s[1];y++)
			for(V3DLONG z=0;z<sz_img_s[2];z++)
			{
				V3DLONG ind=pgsz_xy*z+pgsz_y*y+x;
				if(p_img_s[ind]>d_thresh_fg)
				{
					vec_fg_ind.push_back(ind);
					vec_fg_xyz.push_back(point3D64F(x,y,z));
				}
			}

//	{
//	unsigned char *p_img_tmp=new unsigned char[l_npixel_s]();
//	for(V3DLONG i=0;i<vec_fg_ind.size();i++)
//		p_img_tmp[vec_fg_ind[i]]=255;
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/img_fg.raw",p_img_tmp,sz_img_s,1);
//	delete []p_img_tmp;
//	}

	//creat new image window in V3D and show the downsampled image
	if(paras.b_showatlas || paras.b_showsegmentation)
	{
		v3dhandle newwin=callback.newImageWindow();
		unsigned char* newdata1d=new unsigned char[l_npixel_s]();
		memcpy(newdata1d,p_img_s,l_npixel_s);
		Image4DSimple tmp;
		tmp.setData(newdata1d,sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3],V3D_UINT8);
		callback.setImage(newwin,&tmp);
		callback.open3DWindow(newwin);
		callback.updateImageWindow(newwin);
		callback.pushImageIn3DWindow(newwin);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(6). Initial align atlas with image. \n");
	//format convert
	vector<point3D64F> vec_musclecell,vec_musclecell_affine;
	for(V3DLONG i=0;i<ql_musclecell.size();i++)
		vec_musclecell.push_back(point3D64F(ql_musclecell[i].x,ql_musclecell[i].y,ql_musclecell[i].z));

	//affine align the atlas onto image
	if(paras.l_mode==2)		//atlas2image_affine
	{
		vector< vector<double> > vec2d_labelprob;
		if(!q_atlas2image_musclecell_ini_affine(paras,callback,
				p_img_s,sz_img_s,vec_musclecell,
				vec_fg_ind,vec_fg_xyz,
				vec_musclecell_affine,vec2d_labelprob,outputinfo))
		{
			printf("ERROR: q_atlas2image_musclecell_ini_affine() return false!\n");
			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return false;
		}
		if(paras.b_stepwise) QMessageBox::information(0,"","Affine align atlas onto image complete!\n");
		//prepare output
		ql_musclecell_output=ql_musclecell;
		RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
		for(V3DLONG i=0;i<ql_musclecell_output.size();i++)
		{
			ql_musclecell_output[i].x=vec_musclecell_affine[i].x*paras.d_downsampleratio;
			ql_musclecell_output[i].y=vec_musclecell_affine[i].y*paras.d_downsampleratio;
			ql_musclecell_output[i].z=vec_musclecell_affine[i].z*paras.d_downsampleratio;
			ql_musclecell_output[i].volsize=50;
			ql_musclecell_output[i].color=color;
		}
		//return
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return true;
	}

	//align grid onto image (get all possible cell position)
	vector<point3D64F> vec_musclecell_mshift;
	if(paras.l_mode==0 || paras.l_mode==1 || paras.l_mode==3)
	{
		//generate regular grid
		vector<point3D64F> vec_grid;
		float nx=50,ny=2,nz=2;
		float step_x,step_y,step_z;
		step_x=sz_img_s[0]/(nx-1);	step_y=sz_img_s[1]/(ny-1);	step_z=sz_img_s[2]/(nz-1);
		for(V3DLONG x=0;x<nx;x++)
			for(V3DLONG y=0;y<ny;y++)
				for(V3DLONG z=0;z<nz;z++)
				{
					point3D64F tmp;
					tmp.x=x*step_x;	tmp.y=y*step_y;	tmp.z=z*step_z;
					vec_grid.push_back(tmp);
				}

		vector<point3D64F> vec_musclecell_ref;
		if(!q_atlas2image_musclecell_ref_DAWC(paras,callback,
				p_img_s,sz_img_s,vec_grid,
				vec_fg_ind,vec_fg_xyz,
				vec_musclecell_ref))
		{
			printf("ERROR: q_atlas2image_musclecell_ref_LHHV() return false!\n");
			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return false;
		}
		if(paras.b_stepwise) QMessageBox::information(0,"","Initial align without constraint complete!\n");

		for(unsigned V3DLONG i=0;i<vec_musclecell_ref.size();i++)
		{
			vec_musclecell_ref[i].x*=paras.d_downsampleratio;
			vec_musclecell_ref[i].y*=paras.d_downsampleratio;
			vec_musclecell_ref[i].z*=paras.d_downsampleratio;
		}
		if(!q_atlas2image_musclecell_ref_mshift(paras,callback,
				p_img_1c,sz_img_1c,vec_musclecell_ref,
				vec_musclecell_mshift))
		{
			printf("ERROR: q_atlas2image_musclecell_ref_mshift_adaptive() return false!\n");
			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return false;
		}
		if(paras.b_stepwise) QMessageBox::information(0,"","Mean-shift refining complete!\n");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(7). Group cells and estimate cell number. \n");
	unsigned V3DLONG l_estcellnum=0;
	if(paras.l_mode==0 || paras.l_mode==1 || paras.l_mode==3)
	{
		//group cells and estimate cell number
		V3DLONG l_cellradius=8;
		double d_mingroupdis=l_cellradius;
		vector< vector<V3DLONG> > vec2d_groupind;
		if(!q_groupcells(vec_musclecell_mshift,d_mingroupdis,vec2d_groupind))
		{
			printf("ERROR: q_groupcells() return false!\n");
			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return false;
		}
		for(unsigned V3DLONG i=0;i<vec2d_groupind.size();i++)
		{
			printf("\tgroup[%ld]: ",i);
			for(unsigned V3DLONG j=0;j<vec2d_groupind[i].size();j++)
				printf("%ld,",vec2d_groupind[i][j]+1);
			printf("\n");
		}
		l_estcellnum=vec2d_groupind.size();
		printf("\t>>estimated cell number: %ld\n",l_estcellnum);

		//compute the average position of grouped cells (optional)
		vector<point3D64F> vec_musclecell_groupavg;
		for(unsigned V3DLONG i=0;i<vec2d_groupind.size();i++)
		{
			point3D64F avg;
			for(unsigned V3DLONG j=0;j<vec2d_groupind[i].size();j++)
			{
				V3DLONG ind=vec2d_groupind[i][j];
				avg.x+=vec_musclecell_mshift[ind].x/vec2d_groupind[i].size();
				avg.y+=vec_musclecell_mshift[ind].y/vec2d_groupind[i].size();
				avg.z+=vec_musclecell_mshift[ind].z/vec2d_groupind[i].size();
			}
			vec_musclecell_groupavg.push_back(avg);
		}
		if(paras.b_showatlas)
		{
			vector<point3D64F> vec_musclecell_tmp(vec_musclecell_groupavg);
			for(unsigned V3DLONG i=0;i<vec_musclecell_tmp.size();i++)
			{
				vec_musclecell_tmp[i].x/=paras.d_downsampleratio;
				vec_musclecell_tmp[i].y/=paras.d_downsampleratio;
				vec_musclecell_tmp[i].z/=paras.d_downsampleratio;
			}
			q_push2V3D_pts(callback,vec_musclecell_tmp,10);
		}
		if(paras.b_stepwise) QMessageBox::information(0,"","Group cells and estimate cell number complete!\n");

		//refine group
		if(!q_atlas2image_musclecell_ref_mshift(paras,callback,
				p_img_1c,sz_img_1c,vec_musclecell_groupavg,
				vec_musclecell_mshift))
		{
			printf("ERROR: q_atlas2image_musclecell_ref_mshift_adaptive() return false!\n");
			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return false;
		}
		vec2d_groupind.clear();
		if(!q_groupcells(vec_musclecell_mshift,d_mingroupdis,vec2d_groupind))
		{
			printf("ERROR: q_groupcells() return false!\n");
			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return false;
		}
		vec_musclecell_groupavg.clear();
		for(unsigned V3DLONG i=0;i<vec2d_groupind.size();i++)
		{
			point3D64F avg;
			for(unsigned V3DLONG j=0;j<vec2d_groupind[i].size();j++)
			{
				V3DLONG ind=vec2d_groupind[i][j];
				avg.x+=vec_musclecell_mshift[ind].x/vec2d_groupind[i].size();
				avg.y+=vec_musclecell_mshift[ind].y/vec2d_groupind[i].size();
				avg.z+=vec_musclecell_mshift[ind].z/vec2d_groupind[i].size();
			}
			vec_musclecell_groupavg.push_back(avg);
		}
		if(paras.b_showatlas)
		{
			vector<point3D64F> vec_musclecell_tmp(vec_musclecell_groupavg);
			for(unsigned V3DLONG i=0;i<vec_musclecell_tmp.size();i++)
			{
				vec_musclecell_tmp[i].x/=paras.d_downsampleratio;
				vec_musclecell_tmp[i].y/=paras.d_downsampleratio;
				vec_musclecell_tmp[i].z/=paras.d_downsampleratio;
			}
			q_push2V3D_pts(callback,vec_musclecell_tmp,10);
		}

		if(paras.b_stepwise) QMessageBox::information(0,"","Group cells and estimate cell number complete!\n");

		if(paras.l_mode==1)	//grid onto image
		{
			//prepare output
			CellAPO tmp;
			RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
			tmp.color=color;
			tmp.volsize=50;
			for(unsigned V3DLONG i=0;i<vec_musclecell_groupavg.size();i++)
			{
				tmp.x=vec_musclecell_groupavg[i].x;
				tmp.y=vec_musclecell_groupavg[i].y;
				tmp.z=vec_musclecell_groupavg[i].z;
				ql_musclecell_output.push_back(tmp);
			}

			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(8). Crop the atlas according to the estimated cell number. \n");
	vector<point3D64F> vec_musclecell_crop;
	vector<V3DLONG> vec_muscleind_crop2ori;
	if(l_estcellnum>vec_musclecell.size())
		l_estcellnum=vec_musclecell.size();

	while(vec_muscleind_crop2ori.size()<l_estcellnum)
	{
		double d_minx=1e+5;
		V3DLONG l_minind;

		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			bool b_existed=0;
			for(unsigned V3DLONG j=0;j<vec_muscleind_crop2ori.size();j++)
				if(i==vec_muscleind_crop2ori[j])
				{
					b_existed=1;
					break;
				}
			if(!b_existed)
			{
				if(vec_musclecell[i].x<d_minx)
				{
					d_minx=vec_musclecell[i].x;
					l_minind=i;
				}
			}
		}

		vec_muscleind_crop2ori.push_back(l_minind);
		vec_musclecell_crop.push_back(vec_musclecell[l_minind]);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(9). Realign the image with croped atlas. \n");
	vec_musclecell_affine.clear();
	//affine align the atlas onto image
	vector< vector<double> > vec2d_labelprob;
	if(!q_atlas2image_musclecell_ini_affine(paras,callback,
			p_img_s,sz_img_s,vec_musclecell_crop,
			vec_fg_ind,vec_fg_xyz,
			vec_musclecell_affine,vec2d_labelprob,outputinfo))
	{
		printf("ERROR: q_atlas2image_musclecell_ini_affine() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}
	if(paras.b_stepwise) QMessageBox::information(0,"","Second affine align complete!\n");

	if(paras.l_mode==3) //atlas2image_affine_aftercrop
	{
		//prepare output
		ql_musclecell_output=ql_musclecell;
		RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
		for(V3DLONG i=0;i<ql_musclecell_output.size();i++)
		{
			ql_musclecell_output[i].x=vec_musclecell_affine[i].x*paras.d_downsampleratio;
			ql_musclecell_output[i].y=vec_musclecell_affine[i].y*paras.d_downsampleratio;
			ql_musclecell_output[i].z=vec_musclecell_affine[i].z*paras.d_downsampleratio;
			ql_musclecell_output[i].volsize=50;
			ql_musclecell_output[i].color=color;
		}
		//return
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return true;
	}

	vector<point3D64F> vec_musclecell_ref;
	if(!q_atlas2image_musclecell_ref_DAWC(paras,callback,
			p_img_s,sz_img_s,vec_musclecell_affine,
			vec_fg_ind,vec_fg_xyz,
			vec_musclecell_ref))
	{
		printf("ERROR: q_atlas2image_musclecell_ref_LHHV() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}
	if(paras.b_stepwise) QMessageBox::information(0,"","Second no constraint align complete!\n");

	vec_musclecell_mshift.clear();
	for(unsigned V3DLONG i=0;i<vec_musclecell_ref.size();i++)
	{
		vec_musclecell_ref[i].x*=paras.d_downsampleratio;
		vec_musclecell_ref[i].y*=paras.d_downsampleratio;
		vec_musclecell_ref[i].z*=paras.d_downsampleratio;
	}
	if(!q_atlas2image_musclecell_ref_mshift(paras,callback,
			p_img_1c,sz_img_1c,vec_musclecell_ref,
			vec_musclecell_mshift))
	{
		printf("ERROR: q_atlas2image_musclecell_ref_mshift_adaptive() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}
	if(paras.b_stepwise) QMessageBox::information(0,"","Second mean-shift refining complete!\n");

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(10). Prepare the final output. \n");
	RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
	ql_musclecell_output.clear();
	for(unsigned V3DLONG i=0;i<vec_muscleind_crop2ori.size();i++)
	{
		ql_musclecell_output.push_back(ql_musclecell[vec_muscleind_crop2ori[i]]);
		ql_musclecell_output[i].x=vec_musclecell_mshift[i].x;
		ql_musclecell_output[i].y=vec_musclecell_mshift[i].y;
		ql_musclecell_output[i].z=vec_musclecell_mshift[i].z;
		ql_musclecell_output[i].volsize=50;
		ql_musclecell_output[i].color=color;
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/tmp_final.apo",ql_musclecell_output);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(11). (Post-Process)Backward push the overlapped anno. \n");
	//group the overlapped annos
	vector< vector<V3DLONG> > vec2d_groupind;
	V3DLONG l_cellradius=5;
	double d_mingroupdis=l_cellradius/2.0;
	if(!q_groupcells(vec_musclecell_mshift,d_mingroupdis,vec2d_groupind))
	{
		printf("ERROR: q_groupcells() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}
	for(unsigned V3DLONG i=0;i<vec2d_groupind.size();i++)
	{
		printf("\tgroup[%ld]: ",i);
		for(unsigned V3DLONG j=0;j<vec2d_groupind[i].size();j++)
			printf("%ld,",vec2d_groupind[i][j]+1);
		printf("\n");
	}

	//build the index vector of each bundle (order: VR,VL,DR,DL)
	vector< vector<V3DLONG> > vec2d_ind_bundle2cell(4,vector<V3DLONG>(0,0));
	vector< vector<V3DLONG> > vec2d_ind_cell2bundle(ql_musclecell_output.size(),vector<V3DLONG>(2,-1));//0:bundle ind, 1: ind in current bundle
	for(V3DLONG i=0;i<ql_musclecell_output.size();i++)
	{
		QString qs_cellname=ql_musclecell_output[i].name;
		qs_cellname=qs_cellname.simplified();
		qs_cellname=qs_cellname.toUpper();

		if(qs_cellname.contains("BWMVR"))
		{
			vec2d_ind_bundle2cell[0].push_back(i);
			vec2d_ind_cell2bundle[i][0]=0;
			vec2d_ind_cell2bundle[i][1]=vec2d_ind_bundle2cell[0].size()-1;
		}
		else if(qs_cellname.contains("BWMVL"))
		{
			vec2d_ind_bundle2cell[1].push_back(i);
			vec2d_ind_cell2bundle[i][0]=1;
			vec2d_ind_cell2bundle[i][1]=vec2d_ind_bundle2cell[1].size()-1;
		}
		else if(qs_cellname.contains("BWMDR"))
		{
			vec2d_ind_bundle2cell[2].push_back(i);
			vec2d_ind_cell2bundle[i][0]=2;
			vec2d_ind_cell2bundle[i][1]=vec2d_ind_bundle2cell[2].size()-1;
		}
		else if(qs_cellname.contains("BWMDL"))
		{
			vec2d_ind_bundle2cell[3].push_back(i);
			vec2d_ind_cell2bundle[i][0]=3;
			vec2d_ind_cell2bundle[i][1]=vec2d_ind_bundle2cell[3].size()-1;
		}
	}
	for(unsigned V3DLONG i=0;i<vec2d_ind_bundle2cell.size();i++)
	{
		if(i==0)	printf("\tbundle[BWMVR]: ");
		if(i==1)	printf("\tbundle[BWMVL]: ");
		if(i==2)	printf("\tbundle[BWMDR]: ");
		if(i==3)	printf("\tbundle[BWMDL]: ");
		for(unsigned V3DLONG j=0;j<vec2d_ind_bundle2cell[i].size();j++)
			printf("%ld, ",vec2d_ind_bundle2cell[i][j]+1);
		printf("\n");
	}

	//sort the group and bundle index vector (do not need for current atlas - already sorted)

	//build the anno change vector (backward push the anno)
	vector< vector<V3DLONG> > vec2d_annochange;
	for(unsigned V3DLONG i=0;i<vec2d_groupind.size();i++)
	{
		for(unsigned V3DLONG j=1;j<vec2d_groupind[i].size();j++)
		{
			V3DLONG l_indbundle=vec2d_ind_cell2bundle[vec2d_groupind[i][j]][0];			//ind of bundle need to change anno
			V3DLONG l_indcellinbundle=vec2d_ind_cell2bundle[vec2d_groupind[i][j]][1];		//cell ind in current bundle
			//backward push the anno behind the current cell in current bundle
			vector<V3DLONG> vec_ind_cur2new(2,-1);	//0: cur ind, 1: new ind
			for(unsigned V3DLONG k=l_indcellinbundle;k<vec2d_ind_bundle2cell[l_indbundle].size();k++)
			{
				vec_ind_cur2new[0]=vec2d_ind_bundle2cell[l_indbundle][k];
				if(k+1==vec2d_ind_bundle2cell[l_indbundle].size())
					vec_ind_cur2new[1]=-1;
				else
					vec_ind_cur2new[1]=vec2d_ind_bundle2cell[l_indbundle][k+1];
				vec2d_annochange.push_back(vec_ind_cur2new);
			}
		}
	}
	for(unsigned V3DLONG i=0;i<vec2d_annochange.size();i++)
		printf("\t[%ld] --> [%ld]\n",vec2d_annochange[i][0]+1,vec2d_annochange[i][1]+1);

	//assemble the refined output
	QList<CellAPO> ql_musclecell_refine(ql_musclecell_output);
	for(unsigned V3DLONG i=0;i<vec2d_annochange.size();i++)
	{
		if(vec2d_annochange[i][1]>=0)
			ql_musclecell_refine.replace(vec2d_annochange[i][0],ql_musclecell_refine[vec2d_annochange[i][1]]);
		else
			ql_musclecell_refine[vec2d_annochange[i][0]].comment="out";
	}
	for(V3DLONG i=0;i<ql_musclecell_refine.size();i++)
	{
		if(i>=ql_musclecell_refine.size()) break;
		if(ql_musclecell_refine[i].comment=="out")
			ql_musclecell_refine.removeAt(i);
	}
	ql_musclecell_output.clear();
	ql_musclecell_output=ql_musclecell_refine;

	//------------------------------------------------------------------------------------------------------------------------------------
	printf(">>Free memory\n");
	if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
	if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}

	return true;
}

//group the cells according to the given distance threshold (used to estimate cell num and post-process the clustered cells)
bool q_groupcells(const vector<point3D64F> &vec_cell,const double d_mingroupdis,
		vector< vector<V3DLONG> > &vec2d_groupind)
{
	//check paras
	if(vec_cell.size()==0)
	{
		printf("ERROR: Invalid input vec_cell size!\n");
		return false;
	}
	if(d_mingroupdis<=0)
	{
		printf("ERROR: Invalid input d_mingroupdis<=0!\n");
		return false;
	}
	if(vec2d_groupind.size()!=0)
	{
		printf("WARNNING: Output vec2d_groupind is not empty, original data will be cleared!\n");
		vec2d_groupind.clear();
	}

	for(unsigned V3DLONG i=0;i<vec_cell.size();i++)
	{
		double d_mindis=1e+5;
		V3DLONG l_groupind_mindis=0;

		for(unsigned V3DLONG g=0;g<vec2d_groupind.size();g++)
			for(unsigned V3DLONG k=0;k<vec2d_groupind[g].size();k++)
			{
				V3DLONG ind=vec2d_groupind[g][k];
				double dx=vec_cell[i].x-vec_cell[ind].x;
				double dy=vec_cell[i].y-vec_cell[ind].y;
				double dz=vec_cell[i].z-vec_cell[ind].z;
				double dis=sqrt(dx*dx+dy*dy+dz*dz);

				if(d_mindis>dis)
				{
					d_mindis=dis;
					l_groupind_mindis=g;
				}
			}

		if(d_mindis>d_mingroupdis)		//current cell do not belong to any group
			vec2d_groupind.push_back(vector<V3DLONG>(1,i));
		else							//current cell belong to group g
			vec2d_groupind[l_groupind_mindis].push_back(i);
	}

	return true;
}

bool q_atlas2image_prior(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const QList<CellAPO> &ql_atlasapo,const LandmarkList &ml_makers,
		QList<CellAPO> &ql_musclecell_output)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(ql_atlasapo.size()==0)
	{
		printf("ERROR: Invalid input ql_atlasapo size!\n");
		return false;
	}
	if(ml_makers.size()<3)
	{
		printf("ERROR: Need at least 3 anchor markers!\n");
		return false;
	}
	if(paras.l_refchannel>=sz_img[3] || paras.l_refchannel<0)
	{
		printf("ERROR: l_refchannel is invalid!\n");
		return false;
	}
	if(ql_musclecell_output.size()!=0)
	{
		printf("WARNNING: Output vec_musclecell_output is not empty, original data will be cleared!\n");
		ql_musclecell_output.clear();
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(1). Extract the interesting image channel. \n");
	unsigned char *p_img_1c=0;
	V3DLONG sz_img_1c[4]={sz_img[0],sz_img[1],sz_img[2],1};
	V3DLONG l_npixels_1c=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];

	{
	//allocate memeory
	p_img_1c=new unsigned char[l_npixels_1c]();
	if(!p_img_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img_1c!\n");
		return false;
	}
	//extract referece channel
	V3DLONG pgsz_xyz=sz_img[0]*sz_img[1]*sz_img[2];
	printf("\t>>extract channel:[%ld]\n",paras.l_refchannel);
	V3DLONG ind_start=pgsz_xyz*(paras.l_refchannel-1);
	for(V3DLONG i=0;i<l_npixels_1c;i++)
	{
		V3DLONG ind_1c=i;
		V3DLONG ind_ref=ind_start+ind_1c;
		p_img_1c[ind_1c]=p_img8u[ind_ref];
	}
//	saveImage("img_1c.raw",p_img_1c,sz_img_1c,1);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(2). Resize image, atlas and anchor points. \n");
	unsigned char *p_img_s=0;
	V3DLONG sz_img_s[4]={1,1,1,1};//w,h,z,c
	QList<CellAPO> ql_atlasapo_s(ql_atlasapo);
	LandmarkList ml_makers_s(ml_makers);

	{
	//resize image
	for(V3DLONG i=0;i<3;i++)
		sz_img_s[i]=sz_img_1c[i]/paras.d_downsampleratio+0.5;

	if(!q_imresize_3D(p_img_1c,sz_img_1c,1,sz_img_s,p_img_s))
	{
		printf("ERROR: q_imresize8u_3D() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		return false;
	}
	printf("\t>>sz_img_s:[%ld,%ld,%ld,%ld]\n",sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3]);
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/img_s.raw",p_img_s,sz_img_s,1);

	//resize atlas
	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
		ql_atlasapo_s[i].x/=paras.d_downsampleratio;
		ql_atlasapo_s[i].y/=paras.d_downsampleratio;
		ql_atlasapo_s[i].z/=paras.d_downsampleratio;
		ql_atlasapo_s[i].volsize=5;
		RGBA8 color;	color.r=0; 	color.g=255;	color.b=0;
		ql_atlasapo_s[i].color=color;
	}
//	writeAPO_file("atlas_s.apo",ql_atlasapo);

	//resize anchor markers
	for(V3DLONG i=0;i<ml_makers.size();i++)
	{
		ml_makers_s[i].x/=paras.d_downsampleratio;
		ml_makers_s[i].y/=paras.d_downsampleratio;
		ml_makers_s[i].z/=paras.d_downsampleratio;
	}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(3). Extract atlas points which correspoinding to the interesting image channel. \n");
	QList<CellAPO> ql_musclecell;

	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
		QString qs_cellname=ql_atlasapo_s[i].name;
		qs_cellname=qs_cellname.simplified();
		qs_cellname=qs_cellname.toUpper();
		if(qs_cellname.contains("BWM",Qt::CaseInsensitive) ||
		   qs_cellname=="SPH" ||
		   qs_cellname=="DEP" ||
		   qs_cellname=="IML")
		{
			ql_musclecell.push_back(ql_atlasapo_s[i]);
		}
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/atlas_muscle.apo",ql_musclecell);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(4). Rotate the atlas points upside down if necessary. \n");
	if(paras.b_atlasupsidedown)
	{
		printf("\t>>do rotate \n");
		double d_degree=180/180*3.1415926;
		//compute offset in yz plane
		double d_offset_y=sz_img_s[1]/2.0;
		double d_offset_z=sz_img_s[2]/2.0;
		//rotate
		QList<CellAPO> ql_tmp(ql_musclecell);
		for(V3DLONG i=0;i<ql_musclecell.size();i++)
		{
			ql_tmp[i].y= cos(d_degree)*(ql_musclecell[i].y-d_offset_y)-sin(d_degree)*(ql_musclecell[i].z-d_offset_z)+d_offset_y;
			ql_tmp[i].z=-sin(d_degree)*(ql_musclecell[i].y-d_offset_y)+cos(d_degree)*(ql_musclecell[i].z-d_offset_z)+d_offset_z;
		}
		ql_musclecell.clear();	ql_musclecell=ql_tmp;
	}
	else
		printf("\t>>do not rotate \n");
//	writeAPO_file("atlas_muscle.apo",ql_musclecell);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(5). Extract forground image region. \n");
	double d_thresh_fg;
	vector<V3DLONG> 		vec_fg_ind;
	vector<point3D64F> 	vec_fg_xyz;

	{
	//compute the mean and std of image
	double d_img_mean=0.0,d_img_std=0.0;
	V3DLONG l_npixel_s=sz_img_s[0]*sz_img_s[1]*sz_img_s[2]*sz_img_s[3];
	for(V3DLONG i=0;i<l_npixel_s;i++)
		d_img_mean+=p_img_s[i];
	d_img_mean/=l_npixel_s;
	for(V3DLONG i=0;i<l_npixel_s;i++)
	{
		double temp=p_img_s[i]-d_img_mean;
		d_img_std+=temp*temp;
	}
	d_img_std=sqrt(d_img_std/l_npixel_s);
	printf("\t>>img_mean=%.2f, img_std=%.2f\n",d_img_mean,d_img_std);
	//compute threshold
	d_thresh_fg=d_img_mean+paras.d_fgthresh_factor*d_img_std;
	//extract foreground region
	V3DLONG pgsz_y=sz_img_s[0];
	V3DLONG pgsz_xy=sz_img_s[0]*sz_img_s[1];
	for(V3DLONG x=0;x<sz_img_s[0];x++)
		for(V3DLONG y=0;y<sz_img_s[1];y++)
			for(V3DLONG z=0;z<sz_img_s[2];z++)
			{
				V3DLONG ind=pgsz_xy*z+pgsz_y*y+x;
				if(p_img_s[ind]>d_thresh_fg)
				{
					vec_fg_ind.push_back(ind);
					vec_fg_xyz.push_back(point3D64F(x,y,z));
				}
			}
//	{
//	unsigned char *p_img_tmp=new unsigned char[l_npixel_s]();
//	for(V3DLONG i=0;i<vec_fg_ind.size();i++)
//		p_img_tmp[vec_fg_ind[i]]=255;
//	saveImage("/groups/peng/home/qul/work/straightened/img_fg.raw",p_img_tmp,sz_img_s,1);
//	delete []p_img_tmp;
//	}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(6). Align the anchor points (rigid). \n");
	QList<CellAPO> ql_musclecell_rigid(ql_musclecell);
	vector<V3DLONG> vec_ind_anchor2atlas(ml_makers_s.size(),-1);

	{
	//find the atals points position that corresponding to the anchor points
	for(V3DLONG i=0;i<ml_makers_s.size();i++)
	{
		QString qs_name_anchor(ml_makers_s[i].name.c_str());
		qs_name_anchor=qs_name_anchor.trimmed();
		qs_name_anchor=qs_name_anchor.toUpper();
		for(V3DLONG j=0;j<ql_musclecell.size();j++)
		{
			QString qs_name_atlas=ql_musclecell[j].name;
			qs_name_atlas=qs_name_atlas.trimmed();
			qs_name_atlas=qs_name_atlas.toUpper();
			if(qs_name_anchor==qs_name_atlas)
			{
				vec_ind_anchor2atlas[i]=j;
				break;
			}
		}
	}
	for(V3DLONG i=0;i<ml_makers_s.size();i++)
		if(vec_ind_anchor2atlas[i]==-1)
		{
			printf("ERROR: can not find corresponding atlas point for marker[%ld]!\n",i);
			if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
			if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
			return false;
		}

	//compute the best fitting rigid transform paras for anchor points
	Matrix x3x4_rigidmatrix(3,4);
	V3DLONG n_anchor=ml_makers_s.size();
	//fill matrix A
	//
	//	  | sx, 0 , 0 , cx |    |x1|
	//	  | 0 , sy, 0 , cy | *  |y1|=|x2,y2,z2| <=>
	//	  | 0 , 0 , sz, cz |    |z1|
	//                          |1 |
	//
	//	  |x1,  1, 0,  0,  0,  0, -x2 |
	//	  | 0,  0, y1, 1,  0,  0, -y2 | * |sx,cx,sy,cy,sz,cz,1|'=0
	//	  | 0,  0, 0,  0, z1,  1, -z2 |
	Matrix A(3*n_anchor,7);
	V3DLONG row=1;
	for(V3DLONG i=0;i<n_anchor;i++)
	{
		CellAPO atlaspt=ql_musclecell[vec_ind_anchor2atlas[i]];
		A(row,1)=atlaspt.x;			A(row,2)=1;				A(row,3)=0;					A(row,4)=0;
		A(row,5)=0;					A(row,6)=0;				A(row,7)=-ml_makers_s[i].x;
		row+=1;
		A(row,1)=0;					A(row,2)=0;				A(row,3)=atlaspt.y;			A(row,4)=1;
		A(row,5)=0;					A(row,6)=0;				A(row,7)=-ml_makers_s[i].y;
		row+=1;
		A(row,1)=0;					A(row,2)=0;				A(row,3)=0;					A(row,4)=0;
		A(row,5)=atlaspt.z;			A(row,6)=1;				A(row,7)=-ml_makers_s[i].z;
		row+=1;
	}
	//compute the T
	DiagonalMatrix D(7);
	Matrix U(3*n_anchor,7),V(7,7);
	try
	{
		SVD(A,D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix h=V.column(7);	//A*h=0
	if(D(6,6)==0)			//degenerate case
	{
		x3x4_rigidmatrix=0.0;	//check with A.is_zero()
		printf("Degenerate singular values in SVD! \n");
		return false;
	}
	//de-homo
	for(int i=1;i<=7;i++)
	{
		h(i,1) /= h(7,1);
	}
	//reshape h:7*1 to 3*4 matrix
	x3x4_rigidmatrix(1,1)=h(1,1);	x3x4_rigidmatrix(1,2)=0;		x3x4_rigidmatrix(1,3)=0;		x3x4_rigidmatrix(1,4)=h(2,1);
	x3x4_rigidmatrix(2,1)=0;		x3x4_rigidmatrix(2,2)=h(3,1);	x3x4_rigidmatrix(2,3)=0;		x3x4_rigidmatrix(2,4)=h(4,1);
	x3x4_rigidmatrix(3,1)=0;		x3x4_rigidmatrix(3,2)=0;		x3x4_rigidmatrix(3,3)=h(5,1);	x3x4_rigidmatrix(3,4)=h(6,1);
	cout << setw(10) << setprecision(5) << x3x4_rigidmatrix << endl;

	//rigid transform atlas
	for(V3DLONG i=0;i<ql_musclecell.size();i++)
	{
		ql_musclecell_rigid[i].x=ql_musclecell[i].x*x3x4_rigidmatrix(1,1)+x3x4_rigidmatrix(1,4);
		ql_musclecell_rigid[i].y=ql_musclecell[i].y*x3x4_rigidmatrix(2,2)+x3x4_rigidmatrix(2,4);
		ql_musclecell_rigid[i].z=ql_musclecell[i].z*x3x4_rigidmatrix(3,3)+x3x4_rigidmatrix(3,4);
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/atlas_rigid.apo",ql_musclecell_rigid);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(7). Align the anchor points (non-rigid). \n");
	vector<point3D64F> vec_pts_atlas_rigid,vec_pts_anchor,vec_pts_atlas_nonrigid;

	{
	//reformat the data
	for(V3DLONG i=0;i<ml_makers_s.size();i++)
	{
		point3D64F tmp;
		tmp.x=ml_makers_s[i].x;	tmp.y=ml_makers_s[i].y;	tmp.z=ml_makers_s[i].z;
		vec_pts_anchor.push_back(tmp);
	}
	for(V3DLONG i=0;i<ql_musclecell_rigid.size();i++)
	{
		point3D64F tmp;
		tmp.x=ql_musclecell_rigid[i].x;	tmp.y=ql_musclecell_rigid[i].y;	tmp.z=ql_musclecell_rigid[i].z;
		vec_pts_atlas_rigid.push_back(tmp);
	}

	vec_pts_atlas_nonrigid=vec_pts_atlas_rigid;
	for(V3DLONG iter=0;iter<5000;iter++)
	{
		//compute the distance matrix of atlas points to anchor points
		vector< vector<double> > vec2d_dismatrix(vec_pts_atlas_rigid.size(),vector<double>(vec_pts_anchor.size(),0.0));
		double d_maxdis=0;
		for(unsigned V3DLONG i=0;i<vec_pts_atlas_rigid.size();i++)
			for(unsigned V3DLONG j=0;j<vec_pts_anchor.size();j++)
			{
				double dif_x=vec_pts_atlas_nonrigid[i].x-vec_pts_anchor[j].x;
				double dif_y=vec_pts_atlas_nonrigid[i].y-vec_pts_anchor[j].y;
				double dif_z=vec_pts_atlas_nonrigid[i].z-vec_pts_anchor[j].z;
				vec2d_dismatrix[i][j]=sqrt(dif_x*dif_x+dif_y*dif_y+dif_z*dif_z);

				if(vec2d_dismatrix[i][j]>d_maxdis)
					d_maxdis=vec2d_dismatrix[i][j];
			}

		//normalize the distance matrix
		vector<double> vec_dissum_row(vec_pts_atlas_rigid.size(),0.0);
		for(unsigned V3DLONG i=0;i<vec_pts_atlas_rigid.size();i++)
			for(unsigned V3DLONG j=0;j<vec_pts_anchor.size();j++)
			{
				vec2d_dismatrix[i][j]=d_maxdis-vec2d_dismatrix[i][j];
				vec_dissum_row[i]+=vec2d_dismatrix[i][j];
			}

		//distance weighted shift average
		vector<point3D64F> vec_pts_atlas_dif(vec_pts_atlas_nonrigid.size(),point3D64F());
		for(unsigned V3DLONG i=0;i<vec_pts_anchor.size();i++)
		{
			double dif_x=vec_pts_atlas_nonrigid[vec_ind_anchor2atlas[i]].x-vec_pts_anchor[i].x;
			double dif_y=vec_pts_atlas_nonrigid[vec_ind_anchor2atlas[i]].y-vec_pts_anchor[i].y;
			double dif_z=vec_pts_atlas_nonrigid[vec_ind_anchor2atlas[i]].z-vec_pts_anchor[i].z;
			for(unsigned V3DLONG j=0;j<vec_pts_atlas_rigid.size();j++)
			{
				vec_pts_atlas_dif[j].x+=vec2d_dismatrix[j][i]*dif_x;
				vec_pts_atlas_dif[j].y+=vec2d_dismatrix[j][i]*dif_y;
				vec_pts_atlas_dif[j].z+=vec2d_dismatrix[j][i]*dif_z;
			}
		}

		//update atlas
		for(unsigned V3DLONG i=0;i<vec_pts_atlas_rigid.size();i++)
		{
			vec_pts_atlas_dif[i].x/=vec_dissum_row[i];
			vec_pts_atlas_dif[i].y/=vec_dissum_row[i];
			vec_pts_atlas_dif[i].z/=vec_dissum_row[i];

			vec_pts_atlas_nonrigid[i].x-=vec_pts_atlas_dif[i].x;
			vec_pts_atlas_nonrigid[i].y-=vec_pts_atlas_dif[i].y;
			vec_pts_atlas_nonrigid[i].z-=vec_pts_atlas_dif[i].z;
		}

		double d_simcost=0;
		for(unsigned V3DLONG i=0;i<vec_pts_anchor.size();i++)
		{
			double dif_x=vec_pts_atlas_nonrigid[vec_ind_anchor2atlas[i]].x-vec_pts_anchor[i].x;
			double dif_y=vec_pts_atlas_nonrigid[vec_ind_anchor2atlas[i]].y-vec_pts_anchor[i].y;
			double dif_z=vec_pts_atlas_nonrigid[vec_ind_anchor2atlas[i]].z-vec_pts_anchor[i].z;
			d_simcost+=sqrt(dif_x*dif_x+dif_y*dif_y+dif_z*dif_z);
		}
		printf("[%ld]: cost=%f\n",iter,d_simcost);

		if(d_simcost<0.1)
			break;
	}

	QList<CellAPO> ql_musclecell_nonrigid(ql_musclecell_rigid);
	for(V3DLONG i=0;i<ql_musclecell_rigid.size();i++)
	{
		ql_musclecell_nonrigid[i].x=vec_pts_atlas_nonrigid[i].x;
		ql_musclecell_nonrigid[i].y=vec_pts_atlas_nonrigid[i].y;
		ql_musclecell_nonrigid[i].z=vec_pts_atlas_nonrigid[i].z;
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/atlas_nonrigid.apo",ql_musclecell_nonrigid);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(8). Refine align the atlas by deterministic annealing no constraint. \n");
	QList<CellAPO> ql_musclecell_ref(ql_musclecell_rigid);
	vector<point3D64F> vec_musclecell_valid,vec_musclecell_valid_ref;
	vector<V3DLONG> vec_ind_valid2atlas;

	{
	//find the valid atlas points (within image area)
	vector<V3DLONG> vec_ind_anchor2valid(ml_makers_s.size(),-1);
	for(unsigned V3DLONG i=0;i<vec_pts_atlas_nonrigid.size();i++)
	{
		if(vec_pts_atlas_nonrigid[i].x>=0 && vec_pts_atlas_nonrigid[i].x<sz_img_s[0] &&
		   vec_pts_atlas_nonrigid[i].y>=0 && vec_pts_atlas_nonrigid[i].y<sz_img_s[1] &&
		   vec_pts_atlas_nonrigid[i].z>=0 && vec_pts_atlas_nonrigid[i].z<sz_img_s[2])
		{
			vec_musclecell_valid.push_back(vec_pts_atlas_nonrigid[i]);
			vec_ind_valid2atlas.push_back(i);

			for(V3DLONG j=0;j<ml_makers_s.size();j++)
				if(i==vec_ind_anchor2atlas[j])
					vec_ind_anchor2valid[j]=i;
		}
	}

	//refine the valid atlas points (do not update the anchor points)
	if(!q_atlas2image_musclecell_ref_DAWC_prior(paras,callback,
			p_img_s,sz_img_s,vec_musclecell_valid,vec_ind_anchor2valid,
			vec_fg_ind,vec_fg_xyz,
			vec_musclecell_valid_ref))
	{
		printf("ERROR: q_atlas2image_musclecell_ref_LHHV() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}

	for(unsigned V3DLONG i=0;i<vec_musclecell_valid_ref.size();i++)
	{
		ql_musclecell_ref[vec_ind_valid2atlas[i]].x=vec_musclecell_valid_ref[i].x;
		ql_musclecell_ref[vec_ind_valid2atlas[i]].y=vec_musclecell_valid_ref[i].y;
		ql_musclecell_ref[vec_ind_valid2atlas[i]].z=vec_musclecell_valid_ref[i].z;
	}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(9). Refine the position of atlas points by mean-shift (on original size image). \n");
	QList<CellAPO> ql_musclecell_ref_mshift(ql_musclecell_ref);
	vector<point3D64F> vec_musclecell_valid_mshift;

	for(unsigned V3DLONG i=0;i<vec_musclecell_valid_ref.size();i++)
	{
		vec_musclecell_valid_ref[i].x*=paras.d_downsampleratio;
		vec_musclecell_valid_ref[i].y*=paras.d_downsampleratio;
		vec_musclecell_valid_ref[i].z*=paras.d_downsampleratio;
	}
	for(V3DLONG i=0;i<ql_musclecell_ref_mshift.size();i++)
	{
		ql_musclecell_ref_mshift[i].x*=paras.d_downsampleratio;
		ql_musclecell_ref_mshift[i].y*=paras.d_downsampleratio;
		ql_musclecell_ref_mshift[i].z*=paras.d_downsampleratio;
	}

////	if(!q_atlas2image_musclecell_ref_mshift(paras,callback,
////			p_img_s,sz_img_s,vec_musclecell_valid_ref,
////			vec_musclecell_valid_mshift))
	if(!q_atlas2image_musclecell_ref_mshift(paras,callback,
			p_img_1c,sz_img_1c,vec_musclecell_valid_ref,
			vec_musclecell_valid_mshift))
//	if(!q_atlas2image_musclecell_ref_mshift_adaptive(paras,callback,
//			p_img_1c,sz_img_1c,vec_musclecell_valid_ref,d_thresh_fg,
//			vec_musclecell_valid_mshift))
	{
		printf("ERROR: q_atlas2image_musclecell_ref_mshift() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
		return false;
	}

	for(unsigned V3DLONG i=0;i<vec_musclecell_valid_mshift.size();i++)
	{
		ql_musclecell_ref_mshift[vec_ind_valid2atlas[i]].x=vec_musclecell_valid_mshift[i].x;
		ql_musclecell_ref_mshift[vec_ind_valid2atlas[i]].y=vec_musclecell_valid_mshift[i].y;
		ql_musclecell_ref_mshift[vec_ind_valid2atlas[i]].z=vec_musclecell_valid_mshift[i].z;
	}


	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(10). Prepare the final output. \n");
//	ql_musclecell_output=ql_musclecell_ref;
	ql_musclecell_output=ql_musclecell_ref_mshift;

	//rescale the deformed atlas
	RGBA8 color;	color.r=0; 	color.g=255;	color.b=0;
//	if(fabs(paras.d_downsampleratio-1.0)>1e-5)
		for(V3DLONG i=0;i<ql_musclecell_output.size();i++)
		{
//			ql_musclecell_output[i].x*=paras.d_downsampleratio;
//			ql_musclecell_output[i].y*=paras.d_downsampleratio;
//			ql_musclecell_output[i].z*=paras.d_downsampleratio;
			ql_musclecell_output[i].volsize=10;
			ql_musclecell_output[i].color=color;
		}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/atlas_ref.apo",ql_musclecell_output);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf(">>Free memory\n");
	if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
	if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}

	return true;
}


//affine warp the atlas onto image by deterministic annealing
bool q_atlas2image_musclecell_ini_affine(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<V3DLONG> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output,vector< vector<double> > &vec2d_labelprob,COutputInfo &outputinfo)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec_musclecell.size()==0)
	{
		printf("ERROR: Invalid input vec_musclecell size!\n");
		return false;
	}
	if(vec_fg_ind.size()!=vec_fg_xyz.size())
	{
		printf("ERROR: vec_fg_ind.size()!=vec_fg_xyz.size()\n");
		return false;
	}
	if(vec_musclecell_output.size()!=0)
	{
		printf("WARNNING: Output vec_musclecell_output is not empty, original data will be cleared!\n");
		vec_musclecell_output.clear();
	}
	if(vec2d_labelprob.size()!=0)
	{
		printf("WARNNING: Output vec2d_labelprob is not empty, original data will be cleared!\n");
		vec2d_labelprob.clear();
	}
	outputinfo.b_rotate90=0;

	double d_T=paras.d_T;
	double d_T_min=paras.d_T_min;
	double d_annealingrate=paras.d_annealingrate;
	V3DLONG n_maxiter_inner=paras.l_niter_pertemp;
	double d_lamda=5000;

	vec2d_labelprob.assign(vec_fg_ind.size(),vector<double>(vec_musclecell.size(),0));
	vector<point3D64F> vec_musclecell_affine(vec_musclecell);
	vector<point3D64F> vec_musclecell_noaffine(vec_musclecell);
	vector<point3D64F> vec_musclecell_last(vec_musclecell);
	double d_disnorm=max(sz_img[0],max(sz_img[1],sz_img[2]));

	Matrix x4x4_affine,xnx4_c,xnxn_K;
	bool b_stopiter=0;
	for(V3DLONG iter=0;iter<1000;iter++)
	{
		for(V3DLONG iter_inner=0;iter_inner<n_maxiter_inner;iter_inner++)
		{
//			vector< vector<double> > vec2d_fitcost(vec_fg_ind.size(),vector<double>(vec_musclecell.size(),0));//for fit and entropy energy
			//------------------------------------------------------------------
			//1). given cellpos update labeling possibility
			double d_dis,d_dif_x,d_dif_y,d_dif_z;
			vector<double> vec_probnorm(vec_fg_ind.size(),0);
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
				for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				{
					//compute the distance from foreground pixel to a certain atlas cell
					d_dif_x=vec_fg_xyz[j].x-vec_musclecell_affine[i].x;
					d_dif_y=vec_fg_xyz[j].y-vec_musclecell_affine[i].y;
					d_dif_z=vec_fg_xyz[j].z-vec_musclecell_affine[i].z;
					d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);

					if(d_dis<1e-10) d_dis=1e-10;	//avoid nan result from denominator=0

					//set anisotropic weight factor (along AP direction )
					double d_attenu_90;
					if(d_T<d_T_min)
						d_attenu_90=8;
					else
						d_attenu_90=1;
					//compute the angle between the vector(pixel to cell) and X axis (degree)
					double d_angle2x=acos(fabs(d_dif_x)/d_dis);
					//compute the angle weighted dis
					double d_disweight_angle=sin(d_angle2x)*((d_attenu_90-1)/2.0)+1+(d_attenu_90-1)/2.0;
					d_dis*=d_disweight_angle;

					//update the labeling possibility
//					double d_cost=(350-p_img8u[vec_fg_ind[j]]) * (d_dis/d_disnorm); //for high intensity images
//					vec2d_labelprob[j][i]=exp(-d_cost/d_T);
					double d_cost=(255-p_img8u[vec_fg_ind[j]]) * (d_dis/d_disnorm);
					vec2d_labelprob[j][i]=exp(-d_cost/d_T-1);
//					vec2d_fitcost[j][i]=d_cost;

					//sum of prob of all pixel to a certain cell (for normalize)
					vec_probnorm[j]+=vec2d_labelprob[j][i];
				}
			//normalize the labeling prob (critical!)
			//(increase the prob of unassigned pixel, decrease the prob of multiassigned pixel -> relocate the prob)
			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
				{
					vec2d_labelprob[j][i]/=vec_probnorm[j];

					if(isnan(vec2d_labelprob[j][i]))
						vec2d_labelprob[j][i]=0;
				}

//			//compute the fit and entropy energy (for convincing Hanchuan!)
//			double E_fit=0,E_entropy=0;
//			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
//				for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
//				{
//					E_fit+=vec2d_labelprob[j][i]*vec2d_fitcost[j][i];
//					double tmp=vec2d_labelprob[j][i]*log(vec2d_labelprob[j][i]);
//					if(tmp!=tmp) continue;//isnan
//					E_entropy-=tmp;
//				}
//			printf("[%4ld]: E_fit=%f, E_entropy=%f, T*E_entropy=%f\n",iter,E_fit,E_entropy,d_T*E_entropy);

			//add cluster energy (give clustered pixels higher prob, for reduce noise affect)

			//------------------------------------------------------------------
			//2). update cellpos (probability weighted mass center)
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
			{
				double d_probsum=0,d_sum_x=0,d_sum_y=0,d_sum_z=0;
				for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				{
					d_probsum+=vec2d_labelprob[j][i];
					d_sum_x+=vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
					d_sum_y+=vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
					d_sum_z+=vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
//					d_probsum+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i];
//					d_sum_x+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
//					d_sum_y+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
//					d_sum_z+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
				}
				vec_musclecell_noaffine[i].x=d_sum_x/d_probsum;
				vec_musclecell_noaffine[i].y=d_sum_y/d_probsum;
				vec_musclecell_noaffine[i].z=d_sum_z/d_probsum;
			}

			//------------------------------------------------------------------
			//3). given cellpos update transform
//			//method 1: (vec_musclecell_tmp=T*vec_musclecell)
//			if(!q_compute_affinmatrix_3D(vec_musclecell,vec_musclecell_noaffine,x4x4_affine))
//			{
//				printf("ERROR: q_compute_affinmatrix_3D() return false!\n");
//				return false;
//			}
			//method 2: (vec_musclecell_tmp=vec_musclecell*T)
			if(!q_TPS_cd(vec_musclecell,vec_musclecell_noaffine,d_lamda,x4x4_affine,xnx4_c,xnxn_K))
			{
				printf("ERROR: q_TPS_cd() return false!\n");
				return false;
			}

			//------------------------------------------------------------------
			//4). generate transform constrainted cellpos
//			//method 1:
//			Matrix x_ori(4,1),x_aff(4,1);
//			for(V3DLONG i=0;i<vec_musclecell_affine.size();i++)
//			{
//				x_ori(1,1)=vec_musclecell[i].x;
//				x_ori(2,1)=vec_musclecell[i].y;
//				x_ori(3,1)=vec_musclecell[i].z;
//				x_ori(4,1)=1.0;
//
//				x_aff=x4x4_affine*x_ori;
//
//				vec_musclecell_affine[i].x=x_aff(1,1)/x_aff(4,1);
//				vec_musclecell_affine[i].y=x_aff(2,1)/x_aff(4,1);
//				vec_musclecell_affine[i].z=x_aff(3,1)/x_aff(4,1);
//			}
			//method 2:
			Matrix x_ori(vec_musclecell_affine.size(),4),x_tps(vec_musclecell_affine.size(),4);
			for(unsigned V3DLONG i=0;i<vec_musclecell_affine.size();i++)
			{
				x_ori(i+1,1)=1.0;
				x_ori(i+1,2)=vec_musclecell[i].x;
				x_ori(i+1,3)=vec_musclecell[i].y;
				x_ori(i+1,4)=vec_musclecell[i].z;
			}
//			x_tps=x_ori*x4x4_affine;				//affine transform
			x_tps=x_ori*x4x4_affine+xnxn_K*xnx4_c;	//tps transform
			for(unsigned V3DLONG i=0;i<vec_musclecell_affine.size();i++)
			{
				vec_musclecell_affine[i].x=x_tps(i+1,2)/x_tps(1,1);
				vec_musclecell_affine[i].y=x_tps(i+1,3)/x_tps(1,1);
				vec_musclecell_affine[i].z=x_tps(i+1,4)/x_tps(1,1);
			}

		}

		//------------------------------------------------------------------
		//5). compute the total pos change (affine)
		double d_totalposchange=0,d_dis,d_dif_x,d_dif_y,d_dif_z;
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			d_dif_x=vec_musclecell_last[i].x-vec_musclecell_affine[i].x;
			d_dif_y=vec_musclecell_last[i].y-vec_musclecell_affine[i].y;
			d_dif_z=vec_musclecell_last[i].z-vec_musclecell_affine[i].z;
			d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);
			d_totalposchange+=d_dis;
		}
		vec_musclecell_last=vec_musclecell_affine;
		printf("[%4ld]: T=%f, lamda=%f, poschange=%f\n",iter,d_T,d_lamda,d_totalposchange);

		//compute current energy (energy=ocupied pixel intensity sum)
//		double d_energy;
//		V3DLONG l_radius=2;
//		q_compute_energy(p_img8u,sz_img,vec_musclecell_noaffine,
//				vec_fg_ind,vec_fg_xyz,
//				l_radius,
//				d_energy);
//		printf("[%4d]: T=%f, poschange=%f, energy=%f\n",iter,d_T,d_totalposchange,d_energy);

		//------------------------------------------------------------------
		//6). jude whether stop iter
		if(d_totalposchange<0.1 && d_T<=d_T_min)
			b_stopiter=1;

		//------------------------------------------------------------------
		// visualization
		if(paras.b_showatlas && !paras.b_showsegmentation)
		{
			//visualize the deformation of atlas
			V3DLONG l_makradius=10;
			q_push2V3D_pts(callback,vec_musclecell_affine,l_makradius);
		}
		else if(paras.b_showsegmentation)
		{
			//build the valid pixel to cell correspondence matrix
			vector< vector<unsigned V3DLONG> > vec2d_cellpixelgroups(vec_musclecell.size(),vector<unsigned V3DLONG>());
			double d_celldiameter_max=paras.l_ref_cellradius/paras.d_downsampleratio*2;
			for(unsigned V3DLONG i=0;i<vec_fg_ind.size();i++)
			{
				//for a given pixel, find the cell index with highest prob
				double d_maxprob=0;
				unsigned V3DLONG l_maxprob_ind=0;
				for(unsigned V3DLONG j=0;j<vec_musclecell.size();j++)
				{
					if(vec2d_labelprob[i][j]>d_maxprob)
					{
						d_maxprob=vec2d_labelprob[i][j];
						l_maxprob_ind=j;
					}
				}

				if(!b_stopiter)//during the deformation, the cell radius constraint is not applied
				{
					vec2d_cellpixelgroups[l_maxprob_ind].push_back(i);
				}
				else//deformation complete, apply pixel to cell distance constraint
				{
					double dis,dif_x,dif_y,dif_z;
					dif_x=vec_fg_xyz[i].x-vec_musclecell_affine[l_maxprob_ind].x;
					dif_y=vec_fg_xyz[i].y-vec_musclecell_affine[l_maxprob_ind].y;
					dif_z=vec_fg_xyz[i].z-vec_musclecell_affine[l_maxprob_ind].z;
					dis=sqrt(dif_x*dif_x+dif_y*dif_y+dif_z*dif_z);

					if(dis<d_celldiameter_max)
						vec2d_cellpixelgroups[l_maxprob_ind].push_back(i);
				}
			}

			//line the cells to all of their corresponding segmentated pixles
			NeuronTree nt_cell2validpix;
			NeuronSWC ns;
			ns.r=1;
			for(unsigned V3DLONG i=0;i<vec2d_cellpixelgroups.size();i++)
			{
				ns.n=nt_cell2validpix.listNeuron.size();
				ns.type=1;
				ns.x=vec_musclecell_affine[i].x;
				ns.y=vec_musclecell_affine[i].y;
				ns.z=vec_musclecell_affine[i].z;
				ns.pn=-1;
				nt_cell2validpix.listNeuron.push_back(ns);

				V3DLONG parentind=nt_cell2validpix.listNeuron.size()-1;
				for(unsigned V3DLONG j=0;j<vec2d_cellpixelgroups[i].size();j++)
				{
					ns.n=nt_cell2validpix.listNeuron.size();
					ns.type=2;
					ns.x=vec_fg_xyz[vec2d_cellpixelgroups[i][j]].x+0.5;
					ns.y=vec_fg_xyz[vec2d_cellpixelgroups[i][j]].y+0.5;
					ns.z=vec_fg_xyz[vec2d_cellpixelgroups[i][j]].z+0.5;
					ns.pn=parentind;
					nt_cell2validpix.listNeuron.push_back(ns);
				}
			}

			//now push swc to 3d viewer
			v3dhandle curwin = callback.currentImageWindow();
			callback.open3DWindow(curwin);
			callback.setSWC(curwin,nt_cell2validpix);
			callback.updateImageWindow(curwin);
			callback.pushObjectIn3DWindow(curwin);
//			//save swc to file (for paper, for better visualization in Mac)
//			QString qs_filename_swc=QString("/groups/peng/home/qul/Desktop/segmentation/seg_%1.swc").arg(iter);
//			writeSWC_file(qs_filename_swc,nt_cell2validpix);
			//push the marker to 3d viewer
			if(paras.b_showatlas)
			{
				V3DLONG l_makradius=3;
				q_push2V3D_pts(callback,vec_musclecell_affine,l_makradius);
//				//save markers to file (for paper, for setting consistent color for different figs)
//				QList <ImageMarker> ql_marker;
//				for(unsigned int i=0;i<vec_musclecell_affine.size();i++)
//				{
//					ImageMarker s;
//					s.x=vec_musclecell_affine[i].x+1;//note: marker coord start from 1 instead of 0
//					s.y=vec_musclecell_affine[i].y+1;//note: marker coord start from 1 instead of 0
//					s.z=vec_musclecell_affine[i].z+1;//note: marker coord start from 1 instead of 0
//					s.radius=l_makradius;
//					ql_marker << s;
//				}
//				QString qs_filename_mak=QString("/groups/peng/home/qul/Desktop/segmentation/mak_%1.marker").arg(iter);
//				wirteMarker_file(qs_filename_mak,ql_marker);
			}
		}

		//force process events during the iteration
		if(paras.b_showatlas || paras.b_showsegmentation)
			for(V3DLONG o=0;o<5;o++)
				QCoreApplication::processEvents();

//		//take screen shot
//		v3dhandle curwin = callback.currentImageWindow();
//		QString BMPfilename = QString("/groups/peng/home/qul/Desktop/segmentation/bbb_%1").arg(iter);
//		callback.screenShot3DWindow(curwin, BMPfilename);

		//------------------------------------------------------------------
		if(b_stopiter)
			break;

		//------------------------------------------------------------------
		//7). decrease temperature
		if(d_T>d_T_min) d_T*=d_annealingrate;
//		if(d_T<=d_T_min) d_lamda*=d_annealingrate;
		d_lamda*=d_annealingrate;

	}
	vec_musclecell_output=vec_musclecell_last;

	//------------------------------------------------------------------
	//judge whether deformed atlas exist wrongly 90 degree rotation
	//if angle between current Y and original Z less than 30 degree, we take it wrongly rotated, since this barely happened in practice
	Matrix A=x4x4_affine.submatrix(2,4,2,4).t();
	DiagonalMatrix D(3);
	Matrix U(3,3),V(3,3);
	try
	{
		SVD(A,D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix UV=U*V.t();
	cout<<D<<endl;
	cout<<V<<endl;
	cout<<UV<<endl;

//	if((fabs(V(3,2))>fabs(V(2,2))) && (fabs(V(3,3))<fabs(V(2,3))))
	if(fabs(atan(UV(2,3)/UV(3,3)))/3.1415926*180 > 45)
	{
		outputinfo.b_rotate90=1;
	}


	return true;
}

//refine the warped atlas (DAWC: deterministic annealing without constraint)
bool q_atlas2image_musclecell_ref_DAWC(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<V3DLONG> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec_musclecell.size()==0)
	{
		printf("ERROR: Invalid input vec_musclecell size!\n");
		return false;
	}
	if(vec_fg_ind.size()!=vec_fg_xyz.size())
	{
		printf("ERROR: vec_fg_ind.size()!=vec_fg_xyz.size()\n");
		return false;
	}
	if(vec_musclecell_output.size()!=0)
	{
		printf("WARNNING: Output vec_musclecell_output is not empty, original data will be cleared!\n");
		vec_musclecell_output.clear();
	}

	double d_T=paras.d_ref_T;
	V3DLONG l_cellradius=paras.l_ref_cellradius/paras.d_downsampleratio+0.5;
	V3DLONG l_maxiter=paras.l_ref_maxiter;
	double d_mintotalposchannge=paras.d_ref_minposchange;

	vector< vector<double> > vec2d_labelprob(vec_fg_ind.size(),vector<double>(vec_musclecell.size(),0));
	vector<point3D64F> vec_musclecell_tmp(vec_musclecell);
	vector<point3D64F> vec_musclecell_last(vec_musclecell);
	double d_disnorm=max(sz_img[0],max(sz_img[1],sz_img[2]));

	for(V3DLONG iter=0;iter<l_maxiter;iter++)
	{
		if(paras.b_showatlas) q_push2V3D_pts(callback,vec_musclecell_tmp,10);

		//------------------------------------------------------------------
		//1). given cellpos update labeling possibility
		double d_dis,d_dif_x,d_dif_y,d_dif_z;
		vector<double> vec_probnorm(vec_fg_ind.size(),0);
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
			{
				//compute the distance from foreground pixel to a certain atlas cell
				d_dif_x=vec_fg_xyz[j].x-vec_musclecell_tmp[i].x;
				d_dif_y=vec_fg_xyz[j].y-vec_musclecell_tmp[i].y;
				d_dif_z=vec_fg_xyz[j].z-vec_musclecell_tmp[i].z;
				d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);

				if(d_dis<1e-10) d_dis=1e-10;	//avoid nan result from denominator=0

//				//compute the angle between the vector(pixel to cell) and X axis (degree)
//				double d_angle2x=acos(fabs(d_dif_x)/d_dis);
//				//compute the angle weighted dis
//				double d_attenu_90=8;
//				double d_disweight_angle=sin(d_angle2x)*((d_attenu_90-1)/2.0)+1+(d_attenu_90-1)/2.0;
//				d_dis*=d_disweight_angle;

				//update the labeling possibility
				double d_cost=(255-p_img8u[vec_fg_ind[j]]) * (d_dis/d_disnorm);
				vec2d_labelprob[j][i]=exp(-d_cost/d_T);

				//sum of prob of all pixel to a certain cell (for normalize)
				vec_probnorm[j]+=vec2d_labelprob[j][i];
			}
		//normalize the labeling prob
		for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
			{
				vec2d_labelprob[j][i]/=vec_probnorm[j];

				if(isnan(vec2d_labelprob[j][i]))
					vec2d_labelprob[j][i]=0;
			}

		//------------------------------------------------------------------
		//2). update cellpos (probability weighted mass center)
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			double d_probsum=0,d_sum_x=0,d_sum_y=0,d_sum_z=0;
			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
			{
//				d_probsum+=vec2d_labelprob[j][i];
//				d_sum_x+=vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
//				d_sum_y+=vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
//				d_sum_z+=vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
				d_probsum+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i];
				d_sum_x+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
				d_sum_y+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
				d_sum_z+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
			}
			vec_musclecell_tmp[i].x=d_sum_x/d_probsum;
			vec_musclecell_tmp[i].y=d_sum_y/d_probsum;
			vec_musclecell_tmp[i].z=d_sum_z/d_probsum;
		}

		//------------------------------------------------------------------
		//3). compute the total pos change (affine)
		double d_totalposchange=0;
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			d_dif_x=vec_musclecell_last[i].x-vec_musclecell_tmp[i].x;
			d_dif_y=vec_musclecell_last[i].y-vec_musclecell_tmp[i].y;
			d_dif_z=vec_musclecell_last[i].z-vec_musclecell_tmp[i].z;
			d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);
			d_totalposchange+=d_dis;
		}
		vec_musclecell_last=vec_musclecell_tmp;
		printf("[%4ld]: T=%f, poschange=%f\n",iter,d_T,d_totalposchange);
//		//compute current energy (energy=ocupied pixel intensity sum)
//		double d_energy;
//		q_compute_energy(p_img8u,sz_img,vec_musclecell_last,
//				vec_fg_ind,vec_fg_xyz,
//				l_cellradius,
//				d_energy);
//		printf("[%4d]: T=%f, poschange=%f, energy=%f\n",iter,d_T,d_totalposchange,d_energy);

		//------------------------------------------------------------------
		//6). jude whether stop iter
		if(d_totalposchange<d_mintotalposchannge)
			break;

//		if(paras.b_showinV3D_pts)
//			for(V3DLONG o=0;o<10;o++)
//				QCoreApplication::processEvents();

//		v3dhandle curwin = callback.currentImageWindow();
//		QString BMPfilename = QString("bbb_%1").arg(iter);
//		callback.screenShot3DWindow(curwin, BMPfilename);
	}

	vec_musclecell_output=vec_musclecell_last;

	if(paras.b_showatlas) q_push2V3D_pts(callback,vec_musclecell_output,10);

	return true;
}

bool q_atlas2image_musclecell_ref_DAWC_prior(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,const vector<V3DLONG> vec_ind_anchor,
		const vector<V3DLONG> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec_musclecell.size()==0)
	{
		printf("ERROR: Invalid input vec_musclecell size!\n");
		return false;
	}
	if(vec_fg_ind.size()!=vec_fg_xyz.size())
	{
		printf("ERROR: vec_fg_ind.size()!=vec_fg_xyz.size()\n");
		return false;
	}
	if(vec_musclecell_output.size()!=0)
	{
		printf("WARNNING: Output vec_musclecell_output is not empty, original data will be cleared!\n");
		vec_musclecell_output.clear();
	}

	double d_T=paras.d_ref_T;
	V3DLONG l_maxiter=paras.l_ref_maxiter;
	double d_mintotalposchannge=paras.d_ref_minposchange;

	vector< vector<double> > vec2d_labelprob(vec_fg_ind.size(),vector<double>(vec_musclecell.size(),0));
	vector<point3D64F> vec_musclecell_tmp(vec_musclecell);
	vector<point3D64F> vec_musclecell_last(vec_musclecell);
	double d_disnorm=max(sz_img[0],max(sz_img[1],sz_img[2]));

	for(V3DLONG iter=0;iter<l_maxiter;iter++)
	{
		//------------------------------------------------------------------
		//1). given cellpos update labeling possibility
		double d_dis,d_dif_x,d_dif_y,d_dif_z;
		vector<double> vec_probnorm(vec_fg_ind.size(),0);
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
			{
				//compute the distance from foreground pixel to a certain atlas cell
				d_dif_x=vec_fg_xyz[j].x-vec_musclecell_tmp[i].x;
				d_dif_y=vec_fg_xyz[j].y-vec_musclecell_tmp[i].y;
				d_dif_z=vec_fg_xyz[j].z-vec_musclecell_tmp[i].z;
				d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);

				//update the labeling possibility
				double d_cost=(255-p_img8u[vec_fg_ind[j]]) * (d_dis/d_disnorm);
				vec2d_labelprob[j][i]=exp(-d_cost/d_T);

				//sum of prob of all pixel to a certain cell (for normalize)
				vec_probnorm[j]+=vec2d_labelprob[j][i];
			}
		//normalize the labeling prob
		for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
				vec2d_labelprob[j][i]/=vec_probnorm[j];

		//------------------------------------------------------------------
		//2). update cellpos (probability weighted mass center)
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			double d_probsum=0,d_sum_x=0,d_sum_y=0,d_sum_z=0;
			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
			{
//				d_probsum+=vec2d_labelprob[j][i];
//				d_sum_x+=vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
//				d_sum_y+=vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
//				d_sum_z+=vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
				d_probsum+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i];
				d_sum_x+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
				d_sum_y+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
				d_sum_z+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
			}
			vec_musclecell_tmp[i].x=d_sum_x/d_probsum;
			vec_musclecell_tmp[i].y=d_sum_y/d_probsum;
			vec_musclecell_tmp[i].z=d_sum_z/d_probsum;
		}
		//do not update the anchor points
		for(unsigned V3DLONG i=0;i<vec_ind_anchor.size();i++)
		{
			vec_musclecell_tmp[vec_ind_anchor[i]].x=vec_musclecell[vec_ind_anchor[i]].x;
			vec_musclecell_tmp[vec_ind_anchor[i]].y=vec_musclecell[vec_ind_anchor[i]].y;
			vec_musclecell_tmp[vec_ind_anchor[i]].z=vec_musclecell[vec_ind_anchor[i]].z;
		}

		//------------------------------------------------------------------
		//3). compute the total pos change (affine)
		double d_totalposchange=0;
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			d_dif_x=vec_musclecell_last[i].x-vec_musclecell_tmp[i].x;
			d_dif_y=vec_musclecell_last[i].y-vec_musclecell_tmp[i].y;
			d_dif_z=vec_musclecell_last[i].z-vec_musclecell_tmp[i].z;
			d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);
			d_totalposchange+=d_dis;
		}
		vec_musclecell_last=vec_musclecell_tmp;
		printf("[%4ld]: T=%f, poschange=%f\n",iter,d_T,d_totalposchange);

		//------------------------------------------------------------------
		//6). jude whether stop iter
		if(d_totalposchange<d_mintotalposchannge)
			break;
	}

	vec_musclecell_output=vec_musclecell_last;

	return true;
}


//refine the warped atlas (deterministic annealing without TPS constraint)
bool q_atlas2image_musclecell_ref_DATPS(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<V3DLONG> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output)
{
	double d_T=paras.d_ref_T;
	double d_annealingrate=paras.d_annealingrate;
	V3DLONG n_maxiter_inner=paras.l_niter_pertemp;
	double d_lamda=1000;

	vector< vector<double> > vec2d_labelprob(vec_fg_ind.size(),vector<double>(vec_musclecell.size(),0));
	vector<point3D64F> vec_musclecell_affine(vec_musclecell);
	vector<point3D64F> vec_musclecell_noaffine(vec_musclecell);
	vector<point3D64F> vec_musclecell_last(vec_musclecell);
	double d_disnorm=max(sz_img[0],max(sz_img[1],sz_img[2]));

//	n_maxiter_inner=5;
	for(V3DLONG iter=0;iter<100;iter++)
	{
		for(V3DLONG iter_inner=0;iter_inner<n_maxiter_inner;iter_inner++)
		{
			//------------------------------------------------------------------
			//1). given cellpos update labeling possibility
			double d_dis,d_dif_x,d_dif_y,d_dif_z,d_angle2x;
			vector<double> vec_probnorm(vec_fg_ind.size(),0);
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
				for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				{
					//compute the distance from foreground pixel to a certain atlas cell
					d_dif_x=vec_fg_xyz[j].x-vec_musclecell_affine[i].x;
					d_dif_y=vec_fg_xyz[j].y-vec_musclecell_affine[i].y;
					d_dif_z=vec_fg_xyz[j].z-vec_musclecell_affine[i].z;
					d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);

					//update the labeling possibility
					double d_cost=(255-p_img8u[vec_fg_ind[j]]) * (d_dis/d_disnorm);
					vec2d_labelprob[j][i]=exp(-d_cost/d_T);

					//sum of prob of all pixel to a certain cell (for normalize)
					vec_probnorm[j]+=vec2d_labelprob[j][i];
				}
			//normalize the labeling prob
			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
					vec2d_labelprob[j][i]/=vec_probnorm[j];

			//------------------------------------------------------------------
			//2). update cellpos (probability weighted mass center)
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
			{
				double d_probsum=0,d_sum_x=0,d_sum_y=0,d_sum_z=0;
				for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				{
//					d_probsum+=vec2d_labelprob[j][i];
//					d_sum_x+=vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
//					d_sum_y+=vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
//					d_sum_z+=vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
					d_probsum+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i];
					d_sum_x+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
					d_sum_y+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
					d_sum_z+=p_img8u[vec_fg_ind[j]]*vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
				}
				vec_musclecell_noaffine[i].x=d_sum_x/d_probsum;
				vec_musclecell_noaffine[i].y=d_sum_y/d_probsum;
				vec_musclecell_noaffine[i].z=d_sum_z/d_probsum;
			}

			//------------------------------------------------------------------
			//3). given cellpos update transform (vec_musclecell_tmp=T*vec_musclecell)
			Matrix x4x4_affine,xnx4_c,xnxn_K;
			if(!q_TPS_cd(vec_musclecell,vec_musclecell_noaffine,d_lamda,x4x4_affine,xnx4_c,xnxn_K))
			{
				printf("ERROR: q_TPS_cd() return false!\n");
				return false;
			}

			//------------------------------------------------------------------
			//4). generate transform constrainted cellpos
			Matrix x_ori(vec_musclecell_affine.size(),4),x_tps(vec_musclecell_affine.size(),4);
			for(unsigned V3DLONG i=0;i<vec_musclecell_affine.size();i++)
			{
				x_ori(i+1,1)=1.0;
				x_ori(i+1,2)=vec_musclecell[i].x;
				x_ori(i+1,3)=vec_musclecell[i].y;
				x_ori(i+1,4)=vec_musclecell[i].z;
			}
			x_tps=x_ori*x4x4_affine+xnxn_K*xnx4_c;
			for(unsigned V3DLONG i=0;i<vec_musclecell_affine.size();i++)
			{
				vec_musclecell_affine[i].x=x_tps(i+1,2);
				vec_musclecell_affine[i].y=x_tps(i+1,3);
				vec_musclecell_affine[i].z=x_tps(i+1,4);
			}

			//------------------------------------------------------------------
			//5). compute the total pos change (affine)
			double d_totalposchange=0;
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
			{
				d_dif_x=vec_musclecell_last[i].x-vec_musclecell_affine[i].x;
				d_dif_y=vec_musclecell_last[i].y-vec_musclecell_affine[i].y;
				d_dif_z=vec_musclecell_last[i].z-vec_musclecell_affine[i].z;
				d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);
				d_totalposchange+=d_dis;
			}
			vec_musclecell_last=vec_musclecell_affine;
			printf("[%4ld]: T=%f, poschange=%f\n",iter,d_T,d_totalposchange);

			//------------------------------------------------------------------
			//6). jude whether stop iter
//			if(d_totalposchange<1)
//				break;

		}

		//------------------------------------------------------------------
		//6). decrease temperature
		d_lamda*=0.9;

		if(paras.b_showatlas) q_push2V3D_pts(callback,vec_musclecell_affine,10);

		if(paras.b_showatlas || paras.b_showsegmentation)
			for(V3DLONG o=0;o<10;o++)
				QCoreApplication::processEvents();

	}

	vec_musclecell_output=vec_musclecell_last;

	return true;
}

//refine the warped atlas (LP: locally horizental 3/4 dulside perturbation)
//1. randomly reposition the cells in 3/4 left and right side (locally horizental 3/4 dulside perturbation))
//2. then refine to its local minimal
//3. take the one with highest energy as the output
//refine the warped atlas (LP: locally horizental 3/4 dulside perturbation)
bool q_atlas2image_musclecell_ref_LHHV(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<V3DLONG> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec_musclecell.size()==0)
	{
		printf("ERROR: Invalid input vec_musclecell size!\n");
		return false;
	}
	if(vec_fg_ind.size()!=vec_fg_xyz.size())
	{
		printf("ERROR: vec_fg_ind.size()!=vec_fg_xyz.size()\n");
		return false;
	}
	if(vec_musclecell_output.size()!=0)
	{
		printf("WARNNING: Output vec_musclecell_output is not empty, original data will be cleared!\n");
		vec_musclecell_output.clear();
	}

	V3DLONG l_cellradius=paras.l_ref_cellradius/paras.d_downsampleratio+0.5;

	//------------------------------------------------------------------------------------------------------------------------------------
	//find the index of cells in each stripe (in increase order)
	vector< vector<V3DLONG> > vec2d_musclecellind_4strip;
	//VL
	V3DLONG tmp_VL[]={3,4,10,14,17,23,27,29,33,37,42,46,50,55,57,58,69,72,75};
	vector<V3DLONG> vec_musclecellind_VL(tmp_VL,tmp_VL+sizeof(tmp_VL)/sizeof(V3DLONG));
	vec2d_musclecellind_4strip.push_back(vec_musclecellind_VL);
	//VR
	V3DLONG tmp_VR[]={0,7,11,12,19,20,24,28,32,39,41,45,51,52,56,59,66,70,74,78};
	vector<V3DLONG> vec_musclecellind_VR(tmp_VR,tmp_VR+sizeof(tmp_VR)/sizeof(V3DLONG));
	vec2d_musclecellind_4strip.push_back(vec_musclecellind_VR);
	//DL
	V3DLONG tmp_DL[]={1,6,9,13,18,21,26,30,34,38,40,47,48,54,61,62,65,68,73,77,80};
	vector<V3DLONG> vec_musclecellind_DL(tmp_DL,tmp_DL+sizeof(tmp_DL)/sizeof(V3DLONG));
	vec2d_musclecellind_4strip.push_back(vec_musclecellind_DL);
	//DR
	V3DLONG tmp_DR[]={2,5,8,15,16,22,25,31,35,36,43,44,49,53,60,63,64,67,71,76,79};
	vector<V3DLONG> vec_musclecellind_DR(tmp_DR,tmp_DR+sizeof(tmp_DR)/sizeof(V3DLONG));
	vec2d_musclecellind_4strip.push_back(vec_musclecellind_DR);

	//compute the distance and avg between adjacent cell which in the same stripe
	vector< vector<double> > vec2d_dis;
	vector<double> vec_dis;
	double d_avgdis=0;	V3DLONG n_dis=0;
	for(unsigned V3DLONG i=0;i<vec2d_musclecellind_4strip.size();i++)
	{
		vec_dis.clear();
		for(unsigned V3DLONG j=0;j<vec2d_musclecellind_4strip[0].size()-1;j++)
		{
			V3DLONG ind1=vec2d_musclecellind_4strip[i][j];
			V3DLONG ind2=vec2d_musclecellind_4strip[i][j+1];
			double dis=fabs(vec_musclecell[ind1].x-vec_musclecell[ind2].x);
			vec_dis.push_back(dis);
			d_avgdis+=dis;
			n_dis++;
		}
		vec2d_dis.push_back(vec_dis);
	}
	d_avgdis/=n_dis;

	//------------------------------------------------------------------------------------------------------------------------------------
	vector<point3D64F> vec_musclecell_best,vec_musclecell_tmp(vec_musclecell),vec_musclecell_ref;
	double d_energy,d_energy_best;

	//compute the initial energy
	if(!q_atlas2image_musclecell_ref_DAWC(paras,callback,
			p_img8u,sz_img,vec_musclecell,
			vec_fg_ind,vec_fg_xyz,
			vec_musclecell_ref))
	{
		printf("ERROR: q_atlas2image_musclecell_ref_DAWC() return false!\n");
		return false;
	}
	if(!q_compute_energy(p_img8u,sz_img,vec_musclecell_ref,
			vec_fg_ind,vec_fg_xyz,
			l_cellradius,
			d_energy))
	{
		printf("ERROR: q_compute_energy() return false!\n");
		return false;
	}
	d_energy_best=d_energy;
	vec_musclecell_best=vec_musclecell_ref;
	printf(">>initial_energy=%f\n",d_energy);

//	{RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
//	CellAPO cellapo;
//	cellapo.color=color;
//	cellapo.volsize=5;
//	QList<CellAPO> ql_tmp;
//	for(V3DLONG i=0;i<vec_musclecell_best.size();i++)
//	{
//		cellapo.x=vec_musclecell_best[i].x;
//		cellapo.y=vec_musclecell_best[i].y;
//		cellapo.z=vec_musclecell_best[i].z;
//		ql_tmp.push_back(cellapo);
//	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/tmp_best.apo",ql_tmp);}

	//------------------------------------------------------------------------------------------------------------------------------------
	//shift every cell to its 3/4 left and right side
	//the shift of the neighbor cells satisfy Gaussian distribution
	for(unsigned V3DLONG strip=0;strip<vec2d_musclecellind_4strip.size();strip++)
	{
		for(unsigned V3DLONG i=1;i<vec2d_musclecellind_4strip[strip].size()-1;i++)
		{
			V3DLONG ind=vec2d_musclecellind_4strip[strip][i];
			for(V3DLONG lr=0;lr<2;lr++)//0:left, 1:right
			{
				vec_musclecell_tmp=vec_musclecell;

				//------------------------------------------------------------------
				//shift current cells 3/4 left and right
				double d_shift_cur;
				if(lr==0)	//left
					d_shift_cur=-vec2d_dis[strip][i-1]/2.0;
				else		//right
					d_shift_cur=vec2d_dis[strip][i]/2.0;

				//shift the other cells accordingly (Gaussian)
				double sigma=d_avgdis;
				double miu=vec_musclecell[ind].x;
//				double pi=3.1415926;
				for(unsigned V3DLONG j=0;j<vec2d_musclecellind_4strip[strip].size();j++)
				{
					V3DLONG ind2=vec2d_musclecellind_4strip[strip][j];
					double x=vec_musclecell[ind2].x;
	//				double y=exp(-(x-miu)*(x-miu)/(2*sigma*sigma))/sqrt(2*pi*sigma*sigma);
					double y=exp(-(x-miu)*(x-miu)/(2*sigma*sigma));
					double d_shift=y*d_shift_cur;
					vec_musclecell_tmp[ind2].x+=d_shift;
				}
//				q_push2V3D_pts(callback,vec_musclecell_tmp);

				//------------------------------------------------------------------
				vec_musclecell_ref.clear();
				//recompute current energy
				if(!q_atlas2image_musclecell_ref_DAWC(paras,callback,
						p_img8u,sz_img,vec_musclecell_tmp,
						vec_fg_ind,vec_fg_xyz,
						vec_musclecell_ref))
				{
					printf("ERROR: q_atlas2image_musclecell_ref_DAWC() return false!\n");
					return false;
				}
				V3DLONG l_radius=paras.l_ref_cellradius/paras.d_downsampleratio+0.5;
				if(!q_compute_energy(p_img8u,sz_img,vec_musclecell_ref,
						vec_fg_ind,vec_fg_xyz,
						l_radius,
						d_energy))
				{
					printf("ERROR: q_compute_energy() return false!\n");
					return false;
				}

				//------------------------------------------------------------------
				//keep the best
				if(d_energy>d_energy_best)
				{
					d_energy_best=d_energy;
					vec_musclecell_best=vec_musclecell_ref;

//					{RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
//					CellAPO cellapo;
//					cellapo.color=color;
//					cellapo.volsize=5;
//					QList<CellAPO> ql_tmp;
//					for(V3DLONG i=0;i<vec_musclecell_best.size();i++)
//					{
//						cellapo.x=vec_musclecell_best[i].x;
//						cellapo.y=vec_musclecell_best[i].y;
//						cellapo.z=vec_musclecell_best[i].z;
//						ql_tmp.push_back(cellapo);
//					}
//					writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/tmp_best.apo",ql_tmp);}
				}
				printf("[%d -%4ld]: energy=%f, energy_best=%f\n",strip,i,d_energy,d_energy_best);
			}
		}
	}

	vec_musclecell_output=vec_musclecell_best;

	if(paras.b_showatlas) q_push2V3D_pts(callback,vec_musclecell_output,10);

	return true;
}


//refine the position of atlas points by mean-shift
bool q_atlas2image_musclecell_ref_mshift(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,
		vector<point3D64F> &vec_musclecell_output)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec_musclecell.size()==0)
	{
		printf("ERROR: Invalid input vec_musclecell size!\n");
		return false;
	}
	if(vec_musclecell_output.size()!=0)
	{
		printf("WARNNING: vec_musclecell_output is not empty, original data is deleted!\n");
		vec_musclecell_output.clear();
	}

	V3DLONG l_wndradius=paras.l_ref_cellradius/2.0+0.5;
	V3DLONG l_maxiter=100;
	double d_mintotalposchannge=0.001;

	//build sphere xyz range array
	vector<point3D64F> vec_sphereoffset;
	for(V3DLONG x=-l_wndradius;x<=l_wndradius;x++)
		for(V3DLONG y=-l_wndradius;y<=l_wndradius;y++)
			for(V3DLONG z=-l_wndradius;z<=l_wndradius;z++)
			{
				double dis=sqrt(double(x*x+y*y+z*z));
				if(dis<=l_wndradius)
					vec_sphereoffset.push_back(point3D64F(x,y,z));
			}

	unsigned char ****p_img_4d=0;
	if(!new4dpointer(p_img_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img8u))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		return false;
	}


	//reposition the cells to the mass center of the corresponding sphere
	vector<point3D64F> vec_musclecell_new(vec_musclecell),vec_musclecell_last(vec_musclecell);
	for(V3DLONG iter=0;iter<l_maxiter;iter++)
	{
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			double d_intensitysum=0;
			point3D64F pt_new;
			for(unsigned V3DLONG j=0;j<vec_sphereoffset.size();j++)
			{
				double x=vec_musclecell_new[i].x+vec_sphereoffset[j].x;
				double y=vec_musclecell_new[i].y+vec_sphereoffset[j].y;
				double z=vec_musclecell_new[i].z+vec_sphereoffset[j].z;

				if(x<0) x=0;	if(x>=sz_img[0]) x=sz_img[0]-1;
				if(y<0) y=0;	if(y>=sz_img[1]) y=sz_img[1]-1;
				if(z<0) z=0;	if(z>=sz_img[2]) z=sz_img[2]-1;

				//linear interpolate the intensity at (x,y,z)
				double d_intensity=0;
				//find 8 neighor pixels boundary
				V3DLONG x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=floor(x);		x_b=ceil(x);
				y_s=floor(y);		y_b=ceil(y);
				z_s=floor(z);		z_b=ceil(z);
				x_b=x_b>=sz_img[0]?sz_img[0]-1:x_b;
				y_b=y_b>=sz_img[1]?sz_img[1]-1:y_b;
				z_b=z_b>=sz_img[2]?sz_img[2]-1:z_b;
				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w,r_w,t_w,b_w;
				l_w=1.0-(x-x_s);	r_w=1.0-l_w;
				t_w=1.0-(y-y_s);	b_w=1.0-t_w;
				//compute weight for higer slice and lower slice
				double u_w,d_w;
				u_w=1.0-(z-z_s);	d_w=1.0-u_w;
				//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
				double higher_slice;
				int c=0;
				higher_slice=t_w*(l_w*p_img_4d[c][z_s][y_s][x_s]+r_w*p_img_4d[c][z_s][y_s][x_b])+
							 b_w*(l_w*p_img_4d[c][z_s][y_b][x_s]+r_w*p_img_4d[c][z_s][y_b][x_b]);
				//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
				double lower_slice;
				lower_slice =t_w*(l_w*p_img_4d[c][z_b][y_s][x_s]+r_w*p_img_4d[c][z_b][y_s][x_b])+
							 b_w*(l_w*p_img_4d[c][z_b][y_b][x_s]+r_w*p_img_4d[c][z_b][y_b][x_b]);
				//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
				d_intensity=u_w*higher_slice+d_w*lower_slice;

				pt_new.x+=d_intensity*x;
				pt_new.y+=d_intensity*y;
				pt_new.z+=d_intensity*z;
				d_intensitysum+=d_intensity;
			}

			vec_musclecell_new[i].x=pt_new.x/d_intensitysum;
			vec_musclecell_new[i].y=pt_new.y/d_intensitysum;
			vec_musclecell_new[i].z=pt_new.z/d_intensitysum;
		}

		//judge whether stop iteration
		double d_change=0;
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			double x_dif=vec_musclecell_new[i].x-vec_musclecell_last[i].x;
			double y_dif=vec_musclecell_new[i].y-vec_musclecell_last[i].y;
			double z_dif=vec_musclecell_new[i].z-vec_musclecell_last[i].z;
			d_change+=sqrt(x_dif*x_dif+y_dif*y_dif+z_dif*z_dif);;
		}
		vec_musclecell_last=vec_musclecell_new;
		printf("[%4ld]: change=%f\n",iter,d_change);

		if(d_change<d_mintotalposchannge)
			break;
	}

	vec_musclecell_output=vec_musclecell_new;

	if(paras.b_showatlas)
	{
		vector<point3D64F> vec_musclecell_tmp(vec_musclecell_output);
		for(unsigned V3DLONG i=0;i<vec_musclecell_output.size();i++)
		{
			vec_musclecell_tmp[i].x/=paras.d_downsampleratio;
			vec_musclecell_tmp[i].y/=paras.d_downsampleratio;
			vec_musclecell_tmp[i].z/=paras.d_downsampleratio;
		}
		q_push2V3D_pts(callback,vec_musclecell_tmp,10);
	}

	if(p_img_4d) 		{delete4dpointer(p_img_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}
//refine the position of atlas points by mean-shift (window radius change adaptively)
bool q_atlas2image_musclecell_ref_mshift_adaptive(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,const double d_thresh_fg,
		vector<point3D64F> &vec_musclecell_output)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec_musclecell.size()==0)
	{
		printf("ERROR: Invalid input vec_musclecell size!\n");
		return false;
	}
	if(vec_musclecell_output.size()!=0)
	{
		printf("WARNNING: vec_musclecell_output is not empty, original data is deleted!\n");
		vec_musclecell_output.clear();
	}

	unsigned char ****p_img_4d=0;
	if(!new4dpointer(p_img_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img8u))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		return false;
	}

	V3DLONG l_maxiter=100;
	double d_mintotalposchannge=0.001;

	V3DLONG l_wndradius=paras.l_ref_cellradius/2+0.5;
	V3DLONG l_wndradius_bk=l_wndradius;
	V3DLONG l_wnddiameter=l_wndradius_bk*2+1;
	V3DLONG l_wndvolume=l_wnddiameter*l_wnddiameter*l_wnddiameter;

	//reposition the cells to the mass center of the corresponding sphere
	vector<point3D64F> vec_musclecell_new(vec_musclecell),vec_musclecell_last(vec_musclecell);
	V3DLONG pgsz_y=sz_img[0];
	V3DLONG pgsz_xy=sz_img[0]*sz_img[1];
	for(V3DLONG i=0;i<vec_musclecell.size();i++)
	{
		V3DLONG l_iter=0;
		while(l_iter<l_maxiter)
		{
			double d_intensitysum=0;
			double d_intensitymax=0;
			V3DLONG l_nfgpts=0;
			point3D64F pt_new;
			for(V3DLONG xx=-l_wndradius;xx<=l_wndradius;xx++)
				for(V3DLONG yy=-l_wndradius;yy<=l_wndradius;yy++)
					for(V3DLONG zz=-l_wndradius;zz<=l_wndradius;zz++)
					{
						double x=vec_musclecell_new[i].x+xx;
						double y=vec_musclecell_new[i].y+yy;
						double z=vec_musclecell_new[i].z+zz;

						if(x<0) x=0;	if(x>=sz_img[0]) x=sz_img[0]-1;
						if(y<0) y=0;	if(y>=sz_img[1]) y=sz_img[1]-1;
						if(z<0) z=0;	if(z>=sz_img[2]) z=sz_img[2]-1;

						//linear interpolate the intensity at (x,y,z)
						double d_intensity=0;
						//find 8 neighor pixels boundary
						V3DLONG x_s,x_b,y_s,y_b,z_s,z_b;
						x_s=floor(x);		x_b=ceil(x);
						y_s=floor(y);		y_b=ceil(y);
						z_s=floor(z);		z_b=ceil(z);
						x_b=x_b>=sz_img[0]?sz_img[0]-1:x_b;
						y_b=y_b>=sz_img[1]?sz_img[1]-1:y_b;
						z_b=z_b>=sz_img[2]?sz_img[2]-1:z_b;
						//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
						double l_w,r_w,t_w,b_w;
						l_w=1.0-(x-x_s);	r_w=1.0-l_w;
						t_w=1.0-(y-y_s);	b_w=1.0-t_w;
						//compute weight for higer slice and lower slice
						double u_w,d_w;
						u_w=1.0-(z-z_s);	d_w=1.0-u_w;
						//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double higher_slice;
						int c=0;
						higher_slice=t_w*(l_w*p_img_4d[c][z_s][y_s][x_s]+r_w*p_img_4d[c][z_s][y_s][x_b])+
									 b_w*(l_w*p_img_4d[c][z_s][y_b][x_s]+r_w*p_img_4d[c][z_s][y_b][x_b]);
						//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double lower_slice;
						lower_slice =t_w*(l_w*p_img_4d[c][z_b][y_s][x_s]+r_w*p_img_4d[c][z_b][y_s][x_b])+
									 b_w*(l_w*p_img_4d[c][z_b][y_b][x_s]+r_w*p_img_4d[c][z_b][y_b][x_b]);
						//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
						d_intensity=u_w*higher_slice+d_w*lower_slice;

						pt_new.x+=d_intensity*x;
						pt_new.y+=d_intensity*y;
						pt_new.z+=d_intensity*z;
						d_intensitysum+=d_intensity;
						if(d_intensity>d_intensitymax)	d_intensitymax=d_intensity;
						if(d_intensity>d_thresh_fg)		l_nfgpts++;
					}
			printf("\tl_wndradius=%ld, d_intensitysum=%f\n",l_wndradius,d_intensitysum);

			vec_musclecell_new[i].x=pt_new.x/d_intensitysum;
			vec_musclecell_new[i].y=pt_new.y/d_intensitysum;
			vec_musclecell_new[i].z=pt_new.z/d_intensitysum;
			l_iter++;

			//judge whether stop iteration
			double d_change=0;
			double x_dif=vec_musclecell_new[i].x-vec_musclecell_last[i].x;
			double y_dif=vec_musclecell_new[i].y-vec_musclecell_last[i].y;
			double z_dif=vec_musclecell_new[i].z-vec_musclecell_last[i].z;
			d_change+=sqrt(x_dif*x_dif+y_dif*y_dif+z_dif*z_dif);;
			vec_musclecell_last[i]=vec_musclecell_new[i];
//			printf("[%4d-%4d]: change=%f\n",i,l_iter,d_change);

			//adjust the window radius adaptively (the thresh is critical!)
			double d_threshold=l_wndvolume*0.7;
			if(l_nfgpts<d_threshold)
				l_wndradius+=1;
			else if(l_nfgpts>=d_threshold && l_wndradius>2)
				l_wndradius-=1;


//			if(d_change<d_mintotalposchannge)
			if(d_change<0.001)
				break;

		}
	}

	vec_musclecell_output=vec_musclecell_new;

	if(p_img_4d) 		{delete4dpointer(p_img_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}


//compute the energy for given deformed atlas (energy=ocupied pixel intensity sum)
bool q_compute_energy(const unsigned char *p_img8u,const V3DLONG sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<V3DLONG> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		const V3DLONG &l_radius,
		double &d_energy)
{
	//check paras
	if(p_img8u==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec_musclecell.size()==0)
	{
		printf("ERROR: Invalid input vec_musclecell size!\n");
		return false;
	}
	if(vec_fg_ind.size()!=vec_fg_xyz.size())
	{
		printf("ERROR: vec_fg_ind.size()!=vec_fg_xyz.size()\n");
		return false;
	}
	d_energy=0;

	double d_dis,d_dif_x,d_dif_y,d_dif_z;
	for(unsigned V3DLONG i=0;i<vec_fg_ind.size();i++)
		for(unsigned V3DLONG j=-0;j<vec_musclecell.size();j++)
		{
			d_dif_x=vec_fg_xyz[i].x-vec_musclecell[j].x;
			d_dif_y=vec_fg_xyz[i].y-vec_musclecell[j].y;
			d_dif_z=vec_fg_xyz[i].z-vec_musclecell[j].z;
			d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);

			if(d_dis<l_radius)
			{
				d_energy+=p_img8u[vec_fg_ind[i]];
				break;	//make sure only count once for every fg fixel
			}
		}

	return true;
}

bool q_normalize_points_3D(const vector<point3D64F> &vec_input,vector<point3D64F> &vec_output,Matrix &x4x4_normalize)
{
	//check parameters
	if(vec_input.size()<=0)
	{
		fprintf(stderr,"ERROR: Input array is null! \n");
		return false;
	}
	if(!vec_output.empty())
		vec_output.clear();
	vec_output=vec_input;
	if(x4x4_normalize.nrows()!=4 || x4x4_normalize.ncols()!=4)
	{
		x4x4_normalize.ReSize(4,4);
	}

	//compute the centriod of input point set
	point3D64F cord_centroid;
	int n_point=vec_input.size();
	for(int i=0;i<n_point;i++)
	{
		cord_centroid.x+=vec_input[i].x;
		cord_centroid.y+=vec_input[i].y;
		cord_centroid.z+=vec_input[i].z;
	}
	cord_centroid.x/=n_point;
	cord_centroid.y/=n_point;
	cord_centroid.z/=n_point;
	//center the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x-=cord_centroid.x;
		vec_output[i].y-=cord_centroid.y;
		vec_output[i].z-=cord_centroid.z;
	}

	//compute the average distance of every point to the origin
	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(vec_output[i].x*vec_output[i].x+vec_output[i].y*vec_output[i].y+vec_output[i].z*vec_output[i].z);
		d_point2o_avg+=d_point2o;
	}
	d_point2o_avg/=n_point;
	//compute the scale factor
	double d_scale_factor=1.0/d_point2o_avg;
	//scale the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x*=d_scale_factor;
		vec_output[i].y*=d_scale_factor;
		vec_output[i].z*=d_scale_factor;
	}

	//compute the transformation matrix
	// 1 row
	x4x4_normalize(1,1)=d_scale_factor;
	x4x4_normalize(1,2)=0;
	x4x4_normalize(1,3)=0;
	x4x4_normalize(1,4)=-d_scale_factor*cord_centroid.x;
	// 2 row
	x4x4_normalize(2,1)=0;
	x4x4_normalize(2,2)=d_scale_factor;
	x4x4_normalize(2,3)=0;
	x4x4_normalize(2,4)=-d_scale_factor*cord_centroid.y;
	// 3 row
	x4x4_normalize(3,1)=0;
	x4x4_normalize(3,2)=0;
	x4x4_normalize(3,3)=d_scale_factor;
	x4x4_normalize(3,4)=-d_scale_factor*cord_centroid.z;
	// 4 row
	x4x4_normalize(4,1)=0;
	x4x4_normalize(4,2)=0;
	x4x4_normalize(4,3)=0;
	x4x4_normalize(4,4)=1;

	return true;
}

//compute the rigid(actually is similar) transform matrix B=T*A
//	B=T*A
bool q_compute_rigidmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_rigidmatrix)
{
	//check parameters
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_rigidmatrix.nrows()!=4 || x4x4_rigidmatrix.ncols()!=4)
	{
		x4x4_rigidmatrix.ReSize(4,4);
	}
	int n_point=vec_A.size();

	//normalize point set
	vector<point3D64F> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

	//format
	Matrix x3xn_A(3,n_point),x3xn_B(3,n_point);
	for(V3DLONG i=0;i<n_point;i++)
	{
		x3xn_A(1,i+1)=vec_A_norm[i].x;	x3xn_A(2,i+1)=vec_A_norm[i].y;	x3xn_A(3,i+1)=vec_A_norm[i].z;
		x3xn_B(1,i+1)=vec_B_norm[i].x;	x3xn_B(2,i+1)=vec_B_norm[i].y;	x3xn_B(3,i+1)=vec_B_norm[i].z;
	}

	//compute rotation matrix
	DiagonalMatrix D(3);
	Matrix U(3,3),V(3,3);
	try
	{
		SVD(x3xn_B*x3xn_A.t(),D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix R=V*U.t();

	//reshape the transformation matrix
	x4x4_rigidmatrix(1,1)=R(1,1);	x4x4_rigidmatrix(1,2)=R(1,2);	x4x4_rigidmatrix(1,3)=R(1,3);	x4x4_rigidmatrix(1,4)=0;
	x4x4_rigidmatrix(2,1)=R(2,1);	x4x4_rigidmatrix(2,2)=R(2,2);	x4x4_rigidmatrix(2,3)=R(2,3);	x4x4_rigidmatrix(2,4)=0;
	x4x4_rigidmatrix(3,1)=R(3,1);	x4x4_rigidmatrix(3,2)=R(3,2);	x4x4_rigidmatrix(3,3)=R(3,3);	x4x4_rigidmatrix(3,4)=0;
	x4x4_rigidmatrix(4,1)=0.0;		x4x4_rigidmatrix(4,2)=0.0;		x4x4_rigidmatrix(4,3)=0.0;		x4x4_rigidmatrix(4,4)=1.0;

	//denormalize
	x4x4_rigidmatrix=x4x4_normalize_B.i()*x4x4_rigidmatrix*x4x4_normalize_A;

	return true;
}

//compute the affine matraix
//	B=T*A
bool q_compute_affinmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_affinematrix)
{
	//check parameters
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}

	//normalize point set
	vector<point3D64F> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

	//fill matrix A
	//
	//	  | h1, h2, h3, h4 |    |x1| |x2|
	//	  | h5, h6, h7, h8 | *  |y1|=|y2| <=>
	//	  | h9, h10,h11,h12|    |z1| |z2|
	//	  | 0 ,  0,  0,  1 |    |1 | |1 |
	//
	//	  |x1, y1, z1, 1,  0,  0,  0,  0,  0,  0,  0,  0, -x2 |
	//	  | 0,  0,  0, 0, x1, y1, z1,  1,  0,  0,  0,  0, -y2 | * |h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11,h12,h13|=0
	//	  | 0,  0,  0, 0, 0, 0, 0, 0,  0, x1, y1, z1,  1, -z2 |
	int n_point=vec_A.size();
	Matrix A(3*n_point,13);
	int row=1;
	for(int i=0;i<n_point;i++)
	{
		A(row,1)=vec_A_norm[i].x;	A(row,2)=vec_A_norm[i].y;	A(row,3)=vec_A_norm[i].z;	A(row,4)=1.0;
		A(row,5)=0.0;				A(row,6)=0.0;				A(row,7)=0.0;				A(row,8)=0.0;
		A(row,9)=0.0;				A(row,10)=0.0;				A(row,11)=0.0;				A(row,12)=0.0;
		A(row,13)=-vec_B_norm[i].x;

		A(row+1,1)=0.0;				A(row+1,2)=0.0;				A(row+1,3)=0.0;				A(row+1,4)=0.0;
		A(row+1,5)=vec_A_norm[i].x;	A(row+1,6)=vec_A_norm[i].y;	A(row+1,7)=vec_A_norm[i].z;	A(row+1,8)=1.0;
		A(row+1,9)=0.0;				A(row+1,10)=0.0;			A(row+1,11)=0.0;			A(row+1,12)=0.0;
		A(row+1,13)=-vec_B_norm[i].y;

		A(row+2,1)=0.0;				A(row+2,2)=0.0;				A(row+2,3)=0.0;				A(row+2,4)=0.0;
		A(row+2,5)=0.0;				A(row+2,6)=0.0;				A(row+2,7)=0.0;				A(row+2,8)=0.0;
		A(row+2,9)=vec_A_norm[i].x;	A(row+2,10)=vec_A_norm[i].y;A(row+2,11)=vec_A_norm[i].z;A(row+2,12)=1.0;
		A(row+2,13)=-vec_B_norm[i].z;

		row+=3;
	}

	//compute T  --> bug? SVD in newmat need row>=col?
	DiagonalMatrix D(13);
	Matrix U(3*n_point,13),V(13,13);
	try
	{
		SVD(A,D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}

	Matrix h=V.column(13);	//A*h=0
	if(D(12,12)==0)			//degenerate case
	{
		x4x4_affinematrix=0.0;	//check with A.is_zero()
		printf("Degenerate singular values in SVD! \n");
		//		return false;
	}

	//de-homo
	for(int i=1;i<=13;i++)
	{
		h(i,1) /= h(13,1);
	}

	//reshape h:13*1 to 4*4 matrix
	x4x4_affinematrix(1,1)=h(1,1);	x4x4_affinematrix(1,2)=h(2,1);	x4x4_affinematrix(1,3)=h(3,1);	x4x4_affinematrix(1,4)=h(4,1);
	x4x4_affinematrix(2,1)=h(5,1);	x4x4_affinematrix(2,2)=h(6,1);	x4x4_affinematrix(2,3)=h(7,1);	x4x4_affinematrix(2,4)=h(8,1);
	x4x4_affinematrix(3,1)=h(9,1);	x4x4_affinematrix(3,2)=h(10,1);	x4x4_affinematrix(3,3)=h(11,1);	x4x4_affinematrix(3,4)=h(12,1);
	x4x4_affinematrix(4,1)=0.0;		x4x4_affinematrix(4,2)=0.0;		x4x4_affinematrix(4,3)=0.0;		x4x4_affinematrix(4,4)=1.0;

	//denormalize
	x4x4_affinematrix=x4x4_normalize_B.i()*x4x4_affinematrix*x4x4_normalize_A;

	return true;
}

//compute the affine term d and nonaffine term c which decomposed from TPS (tar=sub*d+K*c)
//more stable compare to the q_affine_compute_affinmatrix_3D()
bool q_TPS_cd(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_tar,const double d_lamda,
		Matrix &x4x4_d,Matrix &xnx4_c,Matrix &xnxn_K)
{
	//check parameters
	if(vec_sub.size()<4 || vec_sub.size()!=vec_tar.size())
	{
		printf("ERROR: Invalid input parameters! \n");
		return false;
	}
	V3DLONG n_pts=vec_sub.size();
	if(xnx4_c.nrows()!=n_pts || xnx4_c.ncols()!=4)
		xnx4_c.ReSize(n_pts,4);
	if(x4x4_d.nrows()!=4 || xnx4_c.ncols()!=4)
		x4x4_d.ReSize(4,4);
	if(xnxn_K.nrows()!=n_pts || xnxn_K.ncols()!=n_pts)
		xnxn_K.ReSize(n_pts,n_pts);

	//generate TPS kernel matrix K=-r=-|xi-xj|
	if(!q_TPS_k(vec_sub,vec_sub,xnxn_K))
	{
		printf("ERROR: q_TPS_k() return false! \n");
		return false;
	}

	//------------------------------------------------------------------
	//compute the QR decomposition of x
	Matrix X(n_pts,4),Y(n_pts,4);
	Matrix Q(n_pts,n_pts); Q=0.0;
	for(V3DLONG i=0;i<n_pts;i++)
	{
		Q(i+1,1)=X(i+1,1)=1;
		Q(i+1,2)=X(i+1,2)=vec_sub[i].x;
		Q(i+1,3)=X(i+1,3)=vec_sub[i].y;
		Q(i+1,4)=X(i+1,4)=vec_sub[i].z;

		Y(i+1,1)=1;
		Y(i+1,2)=vec_tar[i].x;
		Y(i+1,3)=vec_tar[i].y;
		Y(i+1,4)=vec_tar[i].z;
	}

	UpperTriangularMatrix R;
	QRZ(Q,R);
	extend_orthonormal(Q,4);//otherwise q2=0

	Matrix q1=Q.columns(1,4);
	Matrix q2=Q.columns(5,n_pts);
	Matrix r=R.submatrix(1,4,1,4);

	//------------------------------------------------------------------
	//compute non-affine term c which decomposed from TPS
	Matrix A=q2.t()*xnxn_K*q2+IdentityMatrix(n_pts-4)*d_lamda;
	xnx4_c=q2*(A.i()*q2.t()*Y);
	//compute affine term d (normal)
	x4x4_d=r.i()*q1.t()*(Y-xnxn_K*xnx4_c);
//	//compute affine term d (regulized)
//	//small lamda2 can not avoid flip, big lamba can will prevent rotation(not good!)
//	double d_lamda2=0.01;
//	Matrix xRtR=r.t()*r;
//	x4x4_d=((xRtR+d_lamda2*IdentityMatrix(4)).i()) * (xRtR*x4x4_d-xRtR) + IdentityMatrix(4);

	//------------------------------------------------------------------------------------------------------------------------------------
	//suppress affine reflection and 90 degree rotate
	A.resize(3,3);
	A=x4x4_d.submatrix(2,4,2,4).t();

	DiagonalMatrix D(3);
	Matrix U(3,3),V(3,3);
	try
	{
		SVD(A,D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		cout<<BaseException::what()<<endl;
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix UV=U*V.t();
//	cout<<D<<endl;
//	cout<<UV<<endl;

	//------------------------------------------------------------------
	//suppress affine reflection
	//since the variation along ventral-dorsal dir is big than AP dir (we can initialize the atlas by multiply XYZ with 321 respectly)
	//after SVD of tmp, the dir with biggest sigular value must corresponding to x dir, the second one to y, last to z
	//if flip happened, the eigen vector will flip its direction(since the singular value is always >0)
	//since the affine can be decomposed into two rotation with scaling(T=(UV')*(VDV')), judge the flip dir we can use UV':
	//det(T)=det(UV')*det(VDV')=det(UV')*det(V)*det(D)*det(V')=det(UV')*(det(V))^2*det(D)=sign(det(UV'))*det(D) <=> sign(det(T))=sign(det(UV'))
	//strategy:
	//	1. if upside down flip appear first, the left right flip should be enforces
	//	2. if left right flip appear first,another left right flip should be enforces immediately(for compensating the first one)
	//	3. summary: if flip detected, we set D(3,3)=-D(3,3)
	if(A.determinant()<-1e-10)
	{

		//detect upside-down flip
		bool b_flipdetected=0;
		if(UV(2,2)<0)
		{
			printf("WARNING: ||||||||||||||||||||||||||||||||||upside down flip detected||||||||||||||||||||||||||||||||||||\n\n");
			b_flipdetected=1;
		}
		//detect left-right flip
		if(UV(3,3)<0)
		{
			printf("WARNING: -----------------------------------left right flip detected------------------------------------\n\n");
			b_flipdetected=1;
		}
//		if(!b_flipdetected)
//		{
//			v3d_msg("miss detect flip\n");
//		}

		//surpress reflection
		D(3,3)*=-1;

		A=U*D*V.t();
		x4x4_d.submatrix(2,4,2,4)=A.t();
	}

//	//------------------------------------------------------------------
//	//suppress 90 degree rotation
//	//change along Y should always > which along Z (since we have initialize the atlas by multiply XYZ with 321 respectly)
//	//this result in the second column shoudl alway correponding to Y instead of X
//	//shoud perform rotate, not only swap singular value
//	//90 degree have two possible direction, it is hard to make right choice, we only give warnning here, user will make the choice
//	if((fabs(UV(3,2))>fabs(UV(2,2))) && (fabs(UV(3,3))<fabs(UV(2,3))) &&
//        fabs(V(2,2))>0.86)
//	{
//		printf("WARNING: LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL90 degree rotation detectedLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL\n\n");
//
////		double tmp=D(3,3);
////		D(3,3)=D(2,2);
////		D(2,2)=tmp;
//
////		D(2,2)=D(3,3)*0.98;
////		D(3,3)=D(2,2)*1.02;
//
////		A=U*D*V.t();
////		x4x4_d.submatrix(2,4,2,4)=A.t();
//	}

	return true;
}

//generate TPS kernel matrix
bool q_TPS_k(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_basis,Matrix &xmxn_K)
{
	//check parameters
	if(vec_sub.empty() || vec_basis.empty())
	{
		printf("ERROR: q_TPS_k: The input subject or basis marker vector is empty.\n");
		return false;
	}

	xmxn_K.resize(vec_sub.size(),vec_basis.size());

	//compute K=-r=-|xi-xj|
	double d_x,d_y,d_z;
	for(unsigned V3DLONG i=0;i<vec_sub.size();i++)
		for(unsigned V3DLONG j=0;j<vec_basis.size();j++)
		{
			d_x=vec_sub[i].x-vec_basis[j].x;
			d_y=vec_sub[i].y-vec_basis[j].y;
			d_z=vec_sub[i].z-vec_basis[j].z;
			xmxn_K(i+1,j+1)=-sqrt(d_x*d_x+d_y*d_y+d_z*d_z);
		}

	return true;
}

//show the point set in current 3D viewer window
bool q_push2V3D_pts(V3DPluginCallback &callback,vector<point3D64F> &vec_pts,V3DLONG l_makradius)
{
	//show deformed atlas pts in V3D
	v3dhandle curwin = callback.currentImageWindow();
	callback.open3DWindow(curwin);

	//now push the data to the 3d viewer's display
	LandmarkList curlist;
	for(int i=0;i<vec_pts.size();i++)
	{
		LocationSimple s;
		s.x=vec_pts[i].x+1;//note: marker coord start from 1 instead of 0
		s.y=vec_pts[i].y+1;//note: marker coord start from 1 instead of 0
		s.z=vec_pts[i].z+1;//note: marker coord start from 1 instead of 0
		s.radius=l_makradius;

		curlist << s;
	}
	callback.setLandmark(curwin,curlist);
	callback.updateImageWindow(curwin);
	callback.pushObjectIn3DWindow(curwin);

	return true;
}

Matrix q_pseudoinverse(Matrix inputMat)
{
	Matrix pinv;

	//calculate SVD decomposition
	DiagonalMatrix D;
	Matrix U,V;
	SVD(inputMat,D,U,V);
	Matrix Dinv=D.i();
	pinv=V*Dinv*U.t();
	return pinv;
}

//compute sub2tar tps warp parameters based on given subject and target control points
//tps_para_wa=[w;a]
bool q_compute_tps_paras_3D(
		const vector <point3D64F> &vec_ctlpts_sub,const vector <point3D64F> &vec_ctlpts_tar,
		Matrix &wa)
{
	//check parameters
	if(vec_ctlpts_sub.empty() || vec_ctlpts_tar.empty())
	{
		printf("ERROR: q_compute_tps_paras_3D: The input subject or target marker vector is empty.\n");
		return false;
	}
	if(vec_ctlpts_sub.size()!=vec_ctlpts_tar.size())
	{
		printf("ERROR: q_compute_tps_paras_3D: The number of markers in subject and target vector is different.\n");
		return false;
	}

	int n_marker=vec_ctlpts_sub.size();

	//compute the tps transform parameters
	double tmp,s;

	Matrix wR(n_marker,n_marker);
	for(V3DLONG j=0;j<n_marker;j++)
	  for(V3DLONG i=0;i<n_marker;i++)
	  {
		s=0.0;
		tmp=vec_ctlpts_sub.at(i).x-vec_ctlpts_sub.at(j).x;	s+=tmp*tmp;
		tmp=vec_ctlpts_sub.at(i).y-vec_ctlpts_sub.at(j).y; 	s+=tmp*tmp;
		tmp=vec_ctlpts_sub.at(i).z-vec_ctlpts_sub.at(j).z; 	s+=tmp*tmp;
//	    wR(i+1,j+1)=2*s*log(s+1e-20);	//the control points can not be perfectly aligned
	    wR(i+1,j+1)=s*log(s+1e-20);
	  }

	Matrix wP(n_marker,4);
	for(V3DLONG i=0;i<n_marker;i++)
	{
	   wP(i+1,1)=1;
	   wP(i+1,2)=vec_ctlpts_sub.at(i).x;
	   wP(i+1,3)=vec_ctlpts_sub.at(i).y;
	   wP(i+1,4)=vec_ctlpts_sub.at(i).z;
	}

	Matrix wL(n_marker+4,n_marker+4);
	wL.submatrix(1,n_marker,1,n_marker)=wR;
	wL.submatrix(1,n_marker,n_marker+1,n_marker+4)=wP;
	wL.submatrix(n_marker+1,n_marker+4,1,n_marker)=wP.t();
	wL.submatrix(n_marker+1,n_marker+4,n_marker+1,n_marker+4)=0;

	Matrix wY(n_marker+4,3);
	for(V3DLONG i=0;i<n_marker;i++)
	{
	   wY(i+1,1)=vec_ctlpts_tar.at(i).x;
	   wY(i+1,2)=vec_ctlpts_tar.at(i).y;
	   wY(i+1,3)=vec_ctlpts_tar.at(i).z;
	}
	wY.submatrix(n_marker+1,n_marker+4,1,3)=0;

	Try
	{
//	    wa=wL.i()*wY;

//		wa=wL.i();
		wa=q_pseudoinverse(wL);

		wa=wa*wY;
	}
	CatchAll
	{
		printf("ERROR: q_compute_tps_paras_3d: Fail to find the (pseudo)inverse of the wL matrix, maybe too sigular.\n");
		return false;
	}

	return true;
}

//compute the sub2tar warped postion for give subject point based on given sub2tar tps warping parameters
bool q_compute_ptwarped_from_tpspara_3D(
		const point3D64F &pt_sub,const vector <point3D64F> &vec_ctlpts_sub,const Matrix &wa_sub2tar,
		point3D64F &pt_sub2tar)
{
	//check parameters
	if(vec_ctlpts_sub.size()<3)
	{
		printf("ERROR: q_compute_ptwarped_from_tpspara_3D: Are you sure the input control points are right?.\n");
		return false;
	}

	//allocate memory
    double *Ua=new double[vec_ctlpts_sub.size()+4]();
	if(!Ua)
	{
		printf("ERROR: q_compute_ptwarped_from_tpspara_3D: Fail to allocate memory dist for tps warping.\n");
		return false;
	}

	int n_marker=vec_ctlpts_sub.size();
	double tmp,s;

	//calculate distance vector
	for(V3DLONG n=0;n<n_marker;n++)
	{
		s=0;
		tmp=pt_sub.x-vec_ctlpts_sub.at(n).x;	s+=tmp*tmp;
		tmp=pt_sub.y-vec_ctlpts_sub.at(n).y;	s+=tmp*tmp;
		tmp=pt_sub.z-vec_ctlpts_sub.at(n).z;	s+=tmp*tmp;
		Ua[n]=s*log(s+1e-20);
	}

	Ua[n_marker]  =1;
	Ua[n_marker+1]=pt_sub.x;
	Ua[n_marker+2]=pt_sub.y;
	Ua[n_marker+3]=pt_sub.z;

	s=0;
	for(V3DLONG p=0;p<n_marker+4;p++)	s+=Ua[p]*wa_sub2tar(p+1,1);
	pt_sub2tar.x=s;

	s=0;
	for(V3DLONG p=0;p<n_marker+4;p++)	s+=Ua[p]*wa_sub2tar(p+1,2);
	pt_sub2tar.y=s;

	s=0;
	for(V3DLONG p=0;p<n_marker+4;p++)	s+=Ua[p]*wa_sub2tar(p+1,3);
	pt_sub2tar.z=s;

	//free memory
	if(Ua) 	{delete []Ua;	Ua=0;}

	return true;
}


bool q_align_dapicells(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const V3DLONG sz_img[4],const QList<CellAPO> &ql_atlasapo,const QList<QString> &ql_celloi,
		QList<CellAPO> &ql_musclecell_output,COutputInfo &outputinfo)
{
	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(1). Extract the interesting image channel. \n");
	unsigned char *p_img_1c=0;
	V3DLONG sz_img_1c[4]={sz_img[0],sz_img[1],sz_img[2],1};
	V3DLONG l_npixels_1c=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];

	{
	//allocate memeory
	p_img_1c=new unsigned char[l_npixels_1c]();
	if(!p_img_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img_1c!\n");
		return false;
	}
	//extract/combine referece channel(s)
	printf("\t>>extract channel:[%ld]\n",paras.l_refchannel);
	bool arr_validchannel[3]={0,0,0};
	int n_validchannel=0;
	QString qs_refchannel=QString("%1").arg(paras.l_refchannel);
	if(qs_refchannel.contains("1") && sz_img[3]>=1)	{arr_validchannel[0]=1; n_validchannel++;}
	if(qs_refchannel.contains("2") && sz_img[3]>=2)	{arr_validchannel[1]=1; n_validchannel++;}
	if(qs_refchannel.contains("3") && sz_img[3]>=3)	{arr_validchannel[2]=1; n_validchannel++;}

	V3DLONG pgsz_y=sz_img[0];
	V3DLONG pgsz_xy=sz_img[0]*sz_img[1];
	V3DLONG pgsz_xyz=sz_img[0]*sz_img[1]*sz_img[2];
	for(V3DLONG x=0;x<sz_img[0];x++)
		for(V3DLONG y=0;y<sz_img[1];y++)
			for(V3DLONG z=0;z<sz_img[2];z++)
			{
				V3DLONG ind_1c=pgsz_xy*z+pgsz_y*y+x;
				for(V3DLONG c=0;c<3;c++)
					if(arr_validchannel[c])
					{
						V3DLONG ind_ref=pgsz_xyz*c+ind_1c;
						p_img_1c[ind_1c]+=p_img8u[ind_ref]/n_validchannel+0.5;
					}
			}
//	saveImage("img_1c.raw",p_img_1c,sz_img_1c,1);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(2). Resize image and atlas point. \n");
	//resize image and compute actual resize ratio
	V3DLONG sz_img_s[4]={1,1,1,1};//w,h,z,c
	double arr_downratio_actual[3];
	for(V3DLONG i=0;i<3;i++)
	{
		sz_img_s[i]=sz_img_1c[i]/paras.d_downsampleratio+0.5;
		arr_downratio_actual[i]=(double)sz_img_1c[i]/(double)sz_img_s[i];
	}

	unsigned char *p_img_s=0;
	if(!q_imresize_3D(p_img_1c,sz_img_1c,0,sz_img_s,p_img_s))
	{
		printf("ERROR: q_imresize8u_3D() return false!\n");
		if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
		return false;
	}
	printf("\t>>sz_img_s:[%ld,%ld,%ld,%ld]\n",sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3]);
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/img_s.raw",p_img_s,sz_img_s,1);

	//resize atlas
	QList<CellAPO> ql_atlasapo_s(ql_atlasapo);
	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
		ql_atlasapo_s[i].x/=arr_downratio_actual[0];
		ql_atlasapo_s[i].y/=arr_downratio_actual[1];
		ql_atlasapo_s[i].z/=arr_downratio_actual[2];
		ql_atlasapo_s[i].volsize=5;
		RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
		ql_atlasapo_s[i].color=color;
	}
//	writeAPO_file("atlas_s.apo",ql_atlasapo);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(3). Extract interesting atlas points. \n");
	QList<CellAPO> ql_insterestcell,ql_interestcell_output;
	vector< vector <V3DLONG> > vec2d_muslcecell_ind;
	for(V3DLONG i=0;i<ql_atlasapo_s.size();i++)
	{
		QString qs_cellname=ql_atlasapo_s[i].name;

		ql_insterestcell.push_back(ql_atlasapo_s[i]);
		ql_interestcell_output.push_back(ql_atlasapo_s[i]);

		if(qs_cellname.contains("BWM",Qt::CaseInsensitive) || qs_cellname.contains("DEP",Qt::CaseInsensitive))
		{
			vector<V3DLONG> vec_muslcecell_ind(2,0);
			vec_muslcecell_ind[1]=ql_insterestcell.size()-1;
			vec_muslcecell_ind[1]=i;
			vec2d_muslcecell_ind.push_back(vec_muslcecell_ind);
		}
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/atlas_muscle.apo",ql_musclecell);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(5). Extract forground image region. \n");
	vector<V3DLONG> 		vec_fg_ind;
	vector<point3D64F> 	vec_fg_xyz;
	V3DLONG l_npixel_s=sz_img_s[0]*sz_img_s[1]*sz_img_s[2]*sz_img_s[3];

	{
	//compute the mean and std of image
	double d_img_mean=0.0,d_img_std=0.0;
	for(V3DLONG i=0;i<l_npixel_s;i++)
		d_img_mean+=p_img_s[i];
	d_img_mean/=l_npixel_s;
	for(V3DLONG i=0;i<l_npixel_s;i++)
	{
		double temp=p_img_s[i]-d_img_mean;
		d_img_std+=temp*temp;
	}
	d_img_std=sqrt(d_img_std/l_npixel_s);
	printf("\t>>img_mean=%.2f, img_std=%.2f\n",d_img_mean,d_img_std);
	//compute threshold
//	double d_thresh_fg=d_img_mean+paras.d_fgthresh_factor*d_img_std;
	double d_thresh_fg=30;
	//extract foreground region
	V3DLONG pgsz_y=sz_img_s[0];
	V3DLONG pgsz_xy=sz_img_s[0]*sz_img_s[1];
	for(V3DLONG x=0;x<sz_img_s[0];x++)
		for(V3DLONG y=0;y<sz_img_s[1];y++)
			for(V3DLONG z=0;z<sz_img_s[2];z++)
			{
				V3DLONG ind=pgsz_xy*z+pgsz_y*y+x;
				if(p_img_s[ind]>d_thresh_fg)
				{
					vec_fg_ind.push_back(ind);
					vec_fg_xyz.push_back(point3D64F(x,y,z));
				}
			}

//	{
//	unsigned char *p_img_tmp=new unsigned char[l_npixel_s]();
//	for(V3DLONG i=0;i<vec_fg_ind.size();i++)
//		p_img_tmp[vec_fg_ind[i]]=255;
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/img_fg.raw",p_img_tmp,sz_img_s,1);
//	delete []p_img_tmp;
//	}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//creat new image window in V3D and show the downsampled image
	if(paras.b_showatlas || paras.b_showsegmentation)
	{
		v3dhandle newwin=callback.newImageWindow();
		unsigned char* newdata1d=new unsigned char[l_npixel_s]();
		memcpy(newdata1d,p_img_s,l_npixel_s);
		Image4DSimple tmp;
		tmp.setData(newdata1d,sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3],V3D_UINT8);
		callback.setImage(newwin,&tmp);
		callback.open3DWindow(newwin);
		callback.updateImageWindow(newwin);
		callback.pushImageIn3DWindow(newwin);

		//allow user to adjust the property of window before enter into the iteration loop
		//actually within the loop, we have another event mechanism to allow the user interaction, but that is less efficient
		if(CControlPanel::panel)
		{
			CControlPanel::panel->show();
			CControlPanel::panel->setstop();
		}
		else
		{
			CControlPanel *q_controlpanel=new CControlPanel(callback,paras.qw_rootparent);
			q_controlpanel->show();
			q_controlpanel->setstop();
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(6). Initial align atlas with image (deterministic annealing). \n");
	//format convert
	vector<point3D64F> vec_interestcell,vec_interestcell_output;
	for(V3DLONG i=0;i<ql_insterestcell.size();i++)
		vec_interestcell.push_back(point3D64F(ql_insterestcell[i].x,ql_insterestcell[i].y,ql_insterestcell[i].z));

	//------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------
	vector<point3D64F> vec_musclecell_output,vec_musclecell(vec_interestcell);

	double d_T=0.2;
	double d_T_min=0.2;
	double d_annealingrate=0.95;
	V3DLONG n_maxiter_inner=1;
	double d_lamda=0;

	vector< vector<double> > vec2d_labelprob(vec_fg_ind.size(),vector<double>(vec_musclecell.size(),0));
	vector<point3D64F> vec_musclecell_affine(vec_musclecell);
	vector<point3D64F> vec_musclecell_noaffine(vec_musclecell);
	vector<point3D64F> vec_musclecell_last(vec_musclecell);
	double d_disnorm=max(sz_img[0],max(sz_img[1],sz_img[2]));

	Matrix x4x4_affine,xnx4_c,xnxn_K;
	for(V3DLONG iter=0;iter<1000;iter++)
	{
		for(V3DLONG iter_inner=0;iter_inner<n_maxiter_inner;iter_inner++)
		{
			//------------------------------------------------------------------
			//1). given cellpos update labeling possibility
			double d_dis,d_dif_x,d_dif_y,d_dif_z;
			vector<double> vec_probnorm(vec_fg_ind.size(),0);
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
				for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				{
					//compute the distance from foreground pixel to a certain atlas cell
					d_dif_x=vec_fg_xyz[j].x-vec_musclecell_affine[i].x;
					d_dif_y=vec_fg_xyz[j].y-vec_musclecell_affine[i].y;
					d_dif_z=vec_fg_xyz[j].z-vec_musclecell_affine[i].z;
					d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);

					if(d_dis<1e-10) d_dis=1e-10;	//avoid nan result from denominator=0


					//update the labeling possibility
					double d_cost=(255-p_img8u[vec_fg_ind[j]]) * (d_dis/d_disnorm);
					vec2d_labelprob[j][i]=exp(-d_cost/d_T);

					//sum of prob of all pixel to a certain cell (for normalize)
					vec_probnorm[j]+=vec2d_labelprob[j][i];
				}
			//normalize the labeling prob (critical!)
			//(increase the prob of unassigned pixel, decrease the prob of multiassigned pixel -> relocate the prob)
			for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
				{
					vec2d_labelprob[j][i]/=vec_probnorm[j];

					if(isnan(vec2d_labelprob[j][i]))
						vec2d_labelprob[j][i]=0;
				}

			//add cluster energy (give clustered pixels higher prob, for reduce noise affect)

			//------------------------------------------------------------------
			//2). update cellpos (probability weighted mass center)
			for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
			{
				double d_probsum=0,d_sum_x=0,d_sum_y=0,d_sum_z=0;
				for(unsigned V3DLONG j=0;j<vec_fg_ind.size();j++)
				{
					d_probsum+=vec2d_labelprob[j][i];
					d_sum_x+=vec2d_labelprob[j][i]*vec_fg_xyz[j].x;
					d_sum_y+=vec2d_labelprob[j][i]*vec_fg_xyz[j].y;
					d_sum_z+=vec2d_labelprob[j][i]*vec_fg_xyz[j].z;
				}
				vec_musclecell_affine[i].x=d_sum_x/d_probsum;
				vec_musclecell_affine[i].y=d_sum_y/d_probsum;
				vec_musclecell_affine[i].z=d_sum_z/d_probsum;
			}

			//------------------------------------------------------------------
			//3). fix the muscle cells
			for(V3DLONG i=0;i<vec2d_muslcecell_ind.size();i++)
			{
				vec_musclecell_affine[vec2d_muslcecell_ind[i][1]].x=vec_interestcell[vec2d_muslcecell_ind[i][2]].x;
				vec_musclecell_affine[vec2d_muslcecell_ind[i][1]].y=vec_interestcell[vec2d_muslcecell_ind[i][2]].y;
				vec_musclecell_affine[vec2d_muslcecell_ind[i][1]].z=vec_interestcell[vec2d_muslcecell_ind[i][2]].z;
			}

		}

		//------------------------------------------------------------------
		//5). compute the total pos change (affine)
		double d_totalposchange=0,d_dis,d_dif_x,d_dif_y,d_dif_z;
		for(unsigned V3DLONG i=0;i<vec_musclecell.size();i++)
		{
			d_dif_x=vec_musclecell_last[i].x-vec_musclecell_affine[i].x;
			d_dif_y=vec_musclecell_last[i].y-vec_musclecell_affine[i].y;
			d_dif_z=vec_musclecell_last[i].z-vec_musclecell_affine[i].z;
			d_dis=sqrt(d_dif_x*d_dif_x+d_dif_y*d_dif_y+d_dif_z*d_dif_z);
			d_totalposchange+=d_dis;
		}
		vec_musclecell_last=vec_musclecell_affine;
		printf("[%4ld]: T=%f, lamda=%f, poschange=%f\n",iter,d_T,d_lamda,d_totalposchange);

		//------------------------------------------------------------------
		//8). visualization
		if(paras.b_showatlas && !paras.b_showsegmentation)
		{
			//visualize the deformation of atlas
			V3DLONG l_makradius=10;
			q_push2V3D_pts(callback,vec_musclecell_affine,l_makradius);
		}

		//force process events during the iteration
		if(paras.b_showatlas || paras.b_showsegmentation)
			for(V3DLONG o=0;o<5;o++)
				QCoreApplication::processEvents();

		//take screen shot
		v3dhandle curwin = callback.currentImageWindow();
		QString BMPfilename = QString("/groups/peng/home/qul/Desktop/segmentation/bbb_%1").arg(iter);
		callback.screenShot3DWindow(curwin, BMPfilename);

		//------------------------------------------------------------------
		//6). jude whether stop iter
		if(d_totalposchange<0.1 && d_T<=d_T_min)
			break;

		//------------------------------------------------------------------
		//7). decrease temperature
		if(d_T>d_T_min) d_T*=d_annealingrate;
//		if(d_T<=d_T_min) d_lamda*=d_annealingrate;
		d_lamda*=d_annealingrate;

	}

	vec_musclecell_output=vec_musclecell_last;
	vec_interestcell_output=vec_musclecell_output;
	//------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(9). Prepare the final output. \n");
	//rescale the deformed atlas
	//+(arr_downratio_actual[0]+0.5) is for compensate the downsample error (image origin offset error)
	RGBA8 color;	color.r=255; 	color.g=0;	color.b=0;
	for(V3DLONG i=0;i<ql_interestcell_output.size();i++)
	{
		ql_interestcell_output[i].x=vec_interestcell_output[i].x*arr_downratio_actual[0]+(arr_downratio_actual[0]+0.5);
		ql_interestcell_output[i].y=vec_interestcell_output[i].y*arr_downratio_actual[1]+(arr_downratio_actual[1]+0.5);
		ql_interestcell_output[i].z=vec_interestcell_output[i].z*arr_downratio_actual[2]+(arr_downratio_actual[2]+0.5);
		ql_interestcell_output[i].volsize=50;
		ql_interestcell_output[i].color=color;
	}
//	writeAPO_file("/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/tmp_final.apo",ql_musclecell_output);

	//------------------------------------------------------------------------------------------------------------------------------------
	//destroy control panel dialog
	if(CControlPanel::panel)
	{
		delete CControlPanel::panel;
		CControlPanel::panel=0;
	}
	printf(">>Free memory\n");
	if(p_img_1c) 		{delete []p_img_1c;			p_img_1c=0;}
	if(p_img_s) 		{delete []p_img_s;			p_img_s=0;}
	return true;
}

