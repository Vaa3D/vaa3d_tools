//smartscope_util.h
//some utility functions for the smartscope controller
// by Hanchuan Peng
// 2011-May-13

#ifndef __SMARTSCOPE_UTIL_H__
#define __SMARTSCOPE_UTIL_H__

#include <QtGui>

#include <v3d_interface.h>

void wait_milliseconds( V3DLONG milliseconds );
void wait_pixels( V3DLONG npixels, double scanning_rate );
QString get_title(); 

//generate a file name for an image, based on the prefix and suffix and current time
QString generate_img_name(QString prefix, QString suffix);

QString get_last_img_name(QString prefix, QString suffix);

//pass image data back to V3D. If b_convertTo8Bit is true, then scale image to 8-bit automatically
bool passImagingData2V3D(V3DPluginCallback &callback, float *pImFloat, QString imgname, bool b_convertTo8Bit, 
						 V3DLONG sx, V3DLONG sy, V3DLONG sz, V3DLONG sc, 
						 double fov_step_x, double fov_step_y, double fov_step_z, 
						 double min_fovx, double min_fovy, double min_fovz, 
						 double max_fovx, double max_fovy, double max_fovz,
						 double ratio_x, double ratio_y, double ratio_z,
						 int time_out, int scanning_rate, int ch,
						 int scanMethod, int b_useSimImg, int b_scan_z_reverse,
						 QString &filename_img_saved, bool b_open3dwin);



//sort from min to max ordering
template <class T1, class T2> void quickSort(T1 *a, T2 l, T2 r, T2 N, T2 *flag_markers, T2 flag_zyx) //AO X, Y, Z 
{
	if(l>=r) return;
	T2 i=l;
	T2 j=r+1;

	T2 offset_y = N;
	T2 offset_z = N+N;

	T1 pivot_z = a[l+offset_z], pivot_y = a[l+offset_y], pivot_x = a[l];
	
	T2 pivot_marker = flag_markers[l];

	while(true)
	{
		if(flag_zyx==0) // z 
		{
			do{ i = i+1; } while(a[i+offset_z]<pivot_z);
			do{ j = j-1; } while(a[j+offset_z]>pivot_z);
		}
		else if (flag_zyx==1) // y
		{
			do{ i = i+1; } while(a[i+offset_y]<pivot_y);
			do{ j = j-1; } while(a[j+offset_y]>pivot_y);
		}
		else if (flag_zyx==2) // x
		{
			do{ i = i+1; } while(a[i]<pivot_x);
			do{ j = j-1; } while(a[j]>pivot_x);
		}
		else
		{
			// error
			return;
		}

		if(i>=j) break;

		//swap
		T1 tmp;

		tmp = a[i]; a[i] = a[j]; a[j] = tmp; //x
		tmp = a[i+offset_y]; a[i+offset_y] = a[j+offset_y]; a[j+offset_y] = tmp; //y
		tmp = a[i+offset_z]; a[i+offset_z] = a[j+offset_z]; a[j+offset_z] = tmp; //z
		
		T2 tmp_n;
		
		tmp_n = flag_markers[i]; flag_markers[i] = flag_markers[j]; flag_markers[j] = tmp_n; 
	}
	
	//
	a[l] = a[j]; a[l+offset_y] = a[j+offset_y]; a[l+offset_z] = a[j+offset_z]; 
	a[j] = pivot_x; a[j+offset_y] = pivot_y; a[j+offset_z] = pivot_z; 
	
	//
	flag_markers[l] = flag_markers[j]; 
	flag_markers[j] = pivot_marker;

	//
	quickSort(a, l, j-1, N, flag_markers, flag_zyx);
	quickSort(a, j+1, r, N, flag_markers, flag_zyx);
}

// get mean and standard deviation of the image
template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev)
{
    if (!data || n<=0)
	  return false;

	V3DLONG j;
	double ep=0.0, s, p;

	if (n <= 1)
	{
	  //printf("len must be at least 2 in mean_and_std\n");
	  ave = data[0];
	  sdev = (T2)0;
	  return true; //do nothing
	}

	s=0.0;
	for (j=0; j<n; j++) 
	{
		//printf("data[%d]: %lf", j, data[j]);		
		s += (double)data[j];
	}

	double ave_double=(T2)(s/n); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++) {
		s=data[j]-(ave_double);
		var += (p=s*s);
	}
	var=(var-ep*ep/n)/(n-1);
	sdev=(T2)(sqrt(var));
	ave=(T2)ave_double; //use ave_double for the best accuracy

	return true;
}


#endif


