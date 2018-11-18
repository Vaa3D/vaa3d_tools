#include "new_ray-shooting.h"
#include "v3d_message.h"
#include "stackutil.h"
#include "Branch_detection_plugin.h"
#include <iostream>
#include <cmath>
#include <vector>
#include "v3d_basicdatatype.h"
#include <stdio.h>
#include "new_ray-shooting.h"

using namespace std;
#define PI 3.1415926

float square(float x){return x*x;}

void ray_shooting(int m, int n,vector<vector<float> > ray_x,vector<vector<float> > ray_y)
{

    float ang = 2*PI/m;
    float x_dis, y_dis;

    for(int i = 0; i < m; i++)
        {
            x_dis = cos(ang*(i+1));
            y_dis = sin(ang*(i+1));
        for(int j = 0; j<n; j++)
            {
                ray_x[i][j] = x_dis*(j+1);
                ray_y[i][j] = y_dis*(j+1);
            }
        }
    return;
}

int rayinten_2D(int point_y,int point_x,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG sz0,V3DLONG sz1 )
{
    int count=0;
    float pixe = 0.0;
    float sum=0;
    float max_indd=0.0;
    int branch_flag=1;
    vector<int> indd,ind1;//ind1 is the all piex of each ray

    for(int i = 0; i < m; i++)   //m is the numble of the ray
    {
       sum=0;
     for(int j = 0; j < n; j++)    // n is the numble of the points of the each ray
        {
            pixe = project_interp_2d(point_y+ray_y[i][j], point_x+ray_x[i][j], P, sz0,sz1 , point_x, point_y);
            //pixe=exp(0.05*(j+1))*pixe;
            sum=sum+pixe;
        }
        ind1.push_back(sum);
    }

    //find the max ray
    for(int s=0;s<ind1.size();s++)
    {
        if(ind1[s]>max_indd)
        {
            max_indd=ind1[s];
        }
    }

    for(int num=0;num<ind1.size();num++)
    {
        if (ind1[num] >= max_indd*0.5)
            {
                 //v3d_msg(QString("flag is %1").arg(branch_flag));
                 indd.push_back(num);
                 count++;
            }
    }
    int ray_distance=0;
    if (count > 1)
        {
            for (int i = 0;i < count - 1;i++)
            {
               ray_distance=abs(indd[i]-indd[i+1]);
               if(ray_distance>=2)
               {
                   branch_flag=branch_flag+1;
               }
            }

            long dis=indd[0]-indd[count-1];

            if(abs(dis)==(m-1))
            {
              branch_flag=branch_flag-1;
            }

        }


    //v3d_msg(QString("x is %1, y is %2, flag is %3").arg(point_x).arg(point_y).arg(branch_flag));

    if(branch_flag==3)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

float project_interp_2d(float point_y,float point_x,unsigned char * PP,V3DLONG sz0,V3DLONG sz1,int old_x,int old_y)
{
    float result;
    if(point_y < 0||point_x < 0||point_y > sz1-1||point_x > sz0-1)
    {
        return 0.0;

    }
    else if(point_x < 1||point_y < 1||point_x > sz0-2||point_y > sz1-2)
    {
        result = get_2D_ValueUINT8(point_y,point_x,PP, sz0, sz1);
        return result;
    }
    else if((abs(point_x-old_x)==1)||(abs(point_x-old_x)==2)||(abs(point_x-old_x)==3)||(abs(point_x-old_x)==4)||(abs(point_x-old_x)==5)||(abs(point_x-old_x)==6)||(abs(point_x-old_x)==7)||(abs(point_x-old_x)==8)||
            (abs(point_y-old_y)==1)||(abs(point_y-old_y)==2)||(abs(point_y-old_y)==3)||(abs(point_y-old_y)==4)||(abs(point_y-old_y)==5)||(abs(point_y-old_y)==6)||(abs(point_y-old_y)==7)||(abs(point_y-old_y)==8)||
            (abs(point_x-old_x)==9)||(abs(point_x-old_x)==10)||(abs(point_x-old_x)==11)||(abs(point_x-old_x)==12)||(abs(point_x-old_x)==13)||(abs(point_x-old_x)==14)||(abs(point_x-old_x)==15)||(abs(point_x-old_x)==16)||
            (abs(point_y-old_y)==9)||(abs(point_y-old_y)==10)||(abs(point_y-old_y)==11)||(abs(point_y-old_y)==12)||(abs(point_y-old_y)==13)||(abs(point_y-old_y)==14)||(abs(point_y-old_y)==15)||(abs(point_y-old_y)==16))
    {
        result = get_2D_ValueUINT8(point_y,point_x,PP, sz0, sz1);
        return result;
    }
    else
    {
        int d = get_2D_ValueUINT8(ceil(point_y), ceil(point_x),PP,sz0,sz1);  // for example ceil(1.2)=2;
        int b = get_2D_ValueUINT8(floor(point_y), ceil(point_x),PP, sz0,sz1); // for examaple floor(1.2)=1;
        int c = get_2D_ValueUINT8(ceil(point_y), floor(point_x),PP, sz0,sz1);
        int a = get_2D_ValueUINT8(floor(point_y), floor(point_x),PP, sz0,sz1);


//        result = a*(point_x-floor(point_x))*(point_y-floor(point_y))+x1*(ceil(point_x)-point_x)*(point_y-floor(point_y))
//            +y0*(point_x-floor(point_x))*(ceil(point_y)-point_y)+y1*(ceil(point_x)-point_x)*(ceil(point_y)-point_y);
//        return result;
        result=a*(ceil(point_x)-point_x)*(ceil(point_y)-point_y)+b*(ceil(point_y)-point_y)*(point_x-floor(point_x))+c*(ceil(point_x)-point_x)*(point_y-floor(point_y))+d*(point_x-floor(point_x))*(point_y-floor(point_y));
        return result;
    }
}

v3d_uint8  get_2D_ValueUINT8(V3DLONG  y,  V3DLONG  x, unsigned char * T, V3DLONG  sz0, V3DLONG  sz1)
{

    V3DLONG im_total_sz = sz0*sz1;
    V3DLONG idx = y*sz0 + x;
    if (idx < im_total_sz)
    {
        return T[idx];//是否要用this指针
    }
    else
    {
        printf("Image4DSimple::getIntensity() error: index exceeds the image size");
        return 0;
    }
}


void Z_mip(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip)
{
    for(V3DLONG iy = 0; iy < ny; iy++)
    {
        V3DLONG offsetj = iy*nx;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < nz; iz++)
            {
                V3DLONG offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                //    v3d_msg(QString("max_mip is %1").arg(max_mip));
                }
            }
        }
    }

}

void mip(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip,int layer)
{
    for(V3DLONG iy = 0; iy < ny; iy++)
    {
        V3DLONG offsetj = iy*nx;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = nz; iz < nz+layer; iz++)
            {
                V3DLONG offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                //    v3d_msg(QString("max_mip is %1").arg(max_mip));
                }
            }
        }
    }

}

void thres_segment(V3DLONG size_image, unsigned char * old_image, unsigned char * &binary_image,unsigned char thres)
{
    for(V3DLONG i = 0; i < size_image; i++)
    {
        if(old_image[i] > thres)
            binary_image[i] = 255;
        else
            binary_image[i] = 0;
    }
}




