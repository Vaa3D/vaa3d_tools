// q_registration_common.cpp
// by Lei Qu
// 2011-04-08

#include "q_registration_common.h"

#include <math.h>
#include "../../basic_c_fun/stackutil.h"
#include "../../basic_c_fun/basic_memory.cpp"


double q_round(double r)
{
	return (r>0.0) ? floor(r+0.5) : ceil(r-0.5);
}

//bool q_save64f01_image(const double *p_img64f,const long sz_img[4],const char *filename)
//{
//	if(p_img64f==0)
//	{
//		printf("ERROR: Invalid input image pointer!\n");
//		return false;
//	}
//	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<=0)
//	{
//		printf("ERROR: Invalid input image size!\n");
//		return false;
//	}
//	if(!filename)
//	{
//		printf("ERROR: Invalid output image filename pointer!\n");
//		return false;
//	}

//	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

//	unsigned char *p_img4save=0;
//	p_img4save=new unsigned char[l_npixels]();
//	if(!p_img4save)
//	{
//		printf("ERROR: Fail to allocate memory for image.\n");
//		return false;
//	}

//	for(long i=0;i<l_npixels;i++)
//	{
//		double tmp=p_img64f[i]*255.0;
//		tmp=tmp<0?0:tmp;	tmp=tmp>255?255:tmp;
//		p_img4save[i]=tmp+0.5;
//	}

//	saveImage(filename,p_img4save,sz_img,1);

//	if(p_img4save) 	{delete []p_img4save;		p_img4save=0;}

//	return true;
//}

bool q_extractchannel(const unsigned char *p_img32u,const long sz_img[4],const long l_refchannel,unsigned char *&p_img32u_1c)
{
	if(p_img32u==0)
	{
		printf("ERROR: Invalid input image pointer (target or subject)!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(l_refchannel<0 || l_refchannel>=sz_img[3])
	{
		printf("ERROR: Invalid reference channel!\n");
		return false;
	}
	if(p_img32u_1c!=0)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img32u_1c;		p_img32u_1c=0;
	}

	long sz_img_1c[4]={sz_img[0],sz_img[1],sz_img[2],1};
	long l_npixels_1c=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];

	p_img32u_1c=new unsigned char[l_npixels_1c]();
	if(!p_img32u_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img_1c!\n");
		return false;
	}

	long pgsz_y=sz_img_1c[0];
	long pgsz_xy=sz_img_1c[0]*sz_img_1c[1];
	long pgsz_xyz=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];
        if(l_refchannel<3 && l_refchannel<sz_img_1c[3])
        {
                printf("\t>>extract channel:[%ld]\n",l_refchannel);
                for(long x=0;x<sz_img_1c[0];x++)
                        for(long y=0;y<sz_img_1c[1];y++)
                                for(long z=0;z<sz_img_1c[2];z++)
                                {
                                        long ind_1c=pgsz_xy*z+pgsz_y*y+x;
                                        long ind_ref=pgsz_xyz*l_refchannel+ind_1c;
                                        p_img32u_1c[ind_1c]=p_img32u[ind_ref];
                                }
        }

	return true;
}

bool q_align_masscenter(const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		double *&p_img64f_sub2tar,long l_masscenteroffset[4])
{
	if(p_img64f_tar==0 || p_img64f_sub==0)
	{
		printf("ERROR: Invalid input image pointer (target or subject)!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(p_img64f_sub2tar!=0)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img64f_sub2tar;		p_img64f_sub2tar=0;
	}

	p_img64f_sub2tar=new double[sz_img[0]*sz_img[1]*sz_img[2]]();
	if(!p_img64f_sub2tar)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_sub2tar!\n");
		return false;
	}

	double d_masscenter_tar[3]={0,0,0},d_masscenter_sub[3]={0,0,0};
	double d_intsum_tar=0,d_intsum_sub=0;

	long pgsz_y=sz_img[0];
	long pgsz_xy=sz_img[0]*sz_img[1];
	for(long x=0;x<sz_img[0];x++)
		for(long y=0;y<sz_img[1];y++)
			for(long z=0;z<sz_img[2];z++)
			{
				long ind=pgsz_xy*z+pgsz_y*y+x;

				if(p_img64f_tar[ind]<0 || p_img64f_sub[ind]<0) continue;

				d_masscenter_tar[0]+=p_img64f_tar[ind]*x;
				d_masscenter_tar[1]+=p_img64f_tar[ind]*y;
				d_masscenter_tar[2]+=p_img64f_tar[ind]*z;
				d_masscenter_sub[0]+=p_img64f_sub[ind]*x;
				d_masscenter_sub[1]+=p_img64f_sub[ind]*y;
				d_masscenter_sub[2]+=p_img64f_sub[ind]*z;
				d_intsum_tar+=p_img64f_tar[ind];
				d_intsum_sub+=p_img64f_sub[ind];
			}
	d_masscenter_tar[0]/=d_intsum_tar;
	d_masscenter_tar[1]/=d_intsum_tar;
	d_masscenter_tar[2]/=d_intsum_tar;
	d_masscenter_sub[0]/=d_intsum_sub;
	d_masscenter_sub[1]/=d_intsum_sub;
	d_masscenter_sub[2]/=d_intsum_sub;

	l_masscenteroffset[0]=q_round(d_masscenter_sub[0]-d_masscenter_tar[0]);
	l_masscenteroffset[1]=q_round(d_masscenter_sub[1]-d_masscenter_tar[1]);
	l_masscenteroffset[2]=q_round(d_masscenter_sub[2]-d_masscenter_tar[2]);

	printf("\t\t>>masscenter_tar   :[%.2f,%.2f,%.2f]\n",d_masscenter_tar[0],d_masscenter_tar[1],d_masscenter_tar[2]);
	printf("\t\t>>masscenter_sub   :[%.2f,%.2f,%.2f]\n",d_masscenter_sub[0],d_masscenter_sub[1],d_masscenter_sub[2]);
	printf("\t\t>>masscenter_offset:[%ld,%ld,%ld]\n",l_masscenteroffset[0],l_masscenteroffset[1],l_masscenteroffset[2]);

	for(long x=0;x<sz_img[0];x++)
		for(long y=0;y<sz_img[1];y++)
			for(long z=0;z<sz_img[2];z++)
			{

				long x_sub=x+l_masscenteroffset[0];
				long y_sub=y+l_masscenteroffset[1];
				long z_sub=z+l_masscenteroffset[2];

				long ind_shifted=pgsz_xy*z+pgsz_y*y+x;
				if(x_sub<0 || x_sub>=sz_img[0] ||
				   y_sub<0 || y_sub>=sz_img[1] ||
				   z_sub<0 || z_sub>=sz_img[2])
				{
					p_img64f_sub2tar[ind_shifted]=0;
				}
				else
				{
					long ind_sub=pgsz_xy*z_sub+pgsz_y*y_sub+x_sub;
					p_img64f_sub2tar[ind_shifted]=p_img64f_sub[ind_sub];
				}
			}

	return true;
}


bool q_normalize_points_3D(const vector<Point3D64f> vec_input,vector<Point3D64f> &vec_output,Matrix &x4x4_normalize)
{
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

	Point3D64f cord_centroid;
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
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x-=cord_centroid.x;
		vec_output[i].y-=cord_centroid.y;
		vec_output[i].z-=cord_centroid.z;
	}

	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(vec_output[i].x*vec_output[i].x+vec_output[i].y*vec_output[i].y+vec_output[i].z*vec_output[i].z);
		d_point2o_avg+=d_point2o;
	}
	d_point2o_avg/=n_point;
	double d_scale_factor=1.0/d_point2o_avg;
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x*=d_scale_factor;
		vec_output[i].y*=d_scale_factor;
		vec_output[i].z*=d_scale_factor;
	}

	x4x4_normalize(1,1)=d_scale_factor;
	x4x4_normalize(1,2)=0;
	x4x4_normalize(1,3)=0;
	x4x4_normalize(1,4)=-d_scale_factor*cord_centroid.x;
	x4x4_normalize(2,1)=0;
	x4x4_normalize(2,2)=d_scale_factor;
	x4x4_normalize(2,3)=0;
	x4x4_normalize(2,4)=-d_scale_factor*cord_centroid.y;
	x4x4_normalize(3,1)=0;
	x4x4_normalize(3,2)=0;
	x4x4_normalize(3,3)=d_scale_factor;
	x4x4_normalize(3,4)=-d_scale_factor*cord_centroid.z;
	x4x4_normalize(4,1)=0;
	x4x4_normalize(4,2)=0;
	x4x4_normalize(4,3)=0;
	x4x4_normalize(4,4)=1;

	return true;
}

bool q_gradientnorm(const double *p_img64f,const long sz_img[4],const bool b_norm01,
		double *&p_img64f_gradnorm)
{
	if(p_img64f==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(p_img64f_gradnorm!=0)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img64f_gradnorm;		p_img64f_gradnorm=0;
	}

	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2];

	p_img64f_gradnorm=new double[l_npixels]();
	if(!p_img64f_gradnorm)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_tmp!\n");
		return false;
	}

	double ****p_img64f_4d=0,****p_img64f_tmp_4d=0;
	if(!new4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f) ||
	   !new4dpointer(p_img64f_tmp_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_gradnorm))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img64f_gradnorm) 		{delete []p_img64f_gradnorm;			p_img64f_gradnorm=0;}
		if(p_img64f_4d)			{delete4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_tmp_4d) 	{delete4dpointer(p_img64f_tmp_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	double d_max=0;
	for(long z=0;z<sz_img[2];z++)
		for(long y=0;y<sz_img[1];y++)
			for(long x=0;x<sz_img[0];x++)
			{
				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=x-1;	x_b=x+1;
				y_s=y-1;	y_b=y+1;
				z_s=z-1;	z_b=z+1;
				x_s = x_s<0 ? 0:x_s;		x_b = x_b>=sz_img[0] ? sz_img[0]-1:x_b;
				y_s = y_s<0 ? 0:y_s;		y_b = y_b>=sz_img[1] ? sz_img[1]-1:y_b;
				z_s = z_s<0 ? 0:z_s;		z_b = z_b>=sz_img[2] ? sz_img[2]-1:z_b;

				double Ix,Iy,Iz;
				Ix=p_img64f_4d[0][z][y][x_b]-p_img64f_4d[0][z][y][x_s];
				Iy=p_img64f_4d[0][z][y_b][x]-p_img64f_4d[0][z][y_s][x];
				Iz=p_img64f_4d[0][z_b][y][x]-p_img64f_4d[0][z_s][y][x];
				p_img64f_tmp_4d[0][z][y][x]=sqrt(Ix*Ix+Iy*Iy+Iz*Iz);

				if(p_img64f_tmp_4d[0][z][y][x]>d_max) d_max=p_img64f_tmp_4d[0][z][y][x];
			}
	if(p_img64f_4d)			{delete4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img64f_tmp_4d) 	{delete4dpointer(p_img64f_tmp_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	if(b_norm01)
		for(long i=0;i<l_npixels;i++)
			p_img64f_gradnorm[i]/=d_max;

	return true;
}
