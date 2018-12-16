#include "ray_shooting.h"
#include "v3d_message.h"
#include "stackutil.h"
#include "Tip_Detection_plugin.h"
#include <iostream>
#include <cmath>
#include<algorithm>

using namespace std;

#define PI 3.1415926

float square(float x){return x*x;}
int  rayinten_2D(int point_x,int point_y,int m,int n, vector<vector<float>> ray_x,vector<vector<float>> ray_y,V3DLONG nx,  V3DLONG ny, unsigned char *PP, int intensity_threshold,float angle_threshold)
{
    n=n+6;
    int result=0;
    float pixe,sum1,sum2,sum3,sum4,max_ang1,max_ang2,max_ang3,max_ang4;
    int flag1,flag2,flag3,flag4;
    vector<float> p(m);
    vector<int> ind1(m),ind2(m),ind3(m),ind4(m);
    vector<float> find_angle;
    for(int i = 0; i < m; i++)
    {
        sum1=0.0;
        sum2=0.0;
        sum3=0.0;
        sum4=0.0;
        //p.clear();
        for(int j = 0; j<n; j++)
        {


                 pixe = project_interp_2d(point_y+ray_y[i][j], point_x+ray_x[i][j], PP, nx,ny , point_x, point_y);
                 //v3d_msg(QString("i is %1,y is %2, ray_x is %3, ray_y is %4,pixe is %5").arg(i).arg(j).arg(ray_x[i][j]).arg(ray_y[i][j]).arg(pixe));
                 pixe=exp(0.05*(j+1))*pixe;
                 //cout<<"pixe is " <<pixe<<endl;
                 //p.push_back(pixe);
                 p[j]=pixe;
         }
         for(int k1=0;k1<n;k1++)
         {
             sum1=sum1+p[k1];
         }
         //cout<<"sum1 is "<<sum1<<endl;
         for(int k2=0;k2<n-2;k2++)
         {

             sum2=sum2+p[k2];
         }
         //cout<<"sum2 is "<<sum2<<endl;
         for(int k3=0;k3<n-4;k3++)
         {
             sum3=sum3+p[k3];
         }
         //cout<<"sum3 is "<<sum3<<endl;
         for(int k4=0;k4<n-6;k4++)
         {
             sum4=sum4+p[k4];
         }
         //cout<<"sum4 is "<<sum4<<endl;
         ind1.push_back(sum1);
         ind2.push_back(sum2);
         ind3.push_back(sum3);
         ind4.push_back(sum4);
     }
    flag1=get_max_angle(ind1,n,m,intensity_threshold,max_ang1);
    //cout<<"flag1  is "<<flag1<<endl;
    find_angle.push_back(max_ang1);
    flag2=get_max_angle(ind2,n-2,m,intensity_threshold,max_ang2);
    //cout<<"flag2  is "<<flag2<<endl;
    find_angle.push_back(max_ang2);
    flag3=get_max_angle(ind3,n-4,m,intensity_threshold,max_ang3);
    //cout<<"flag3  is "<<flag3<<endl;
    find_angle.push_back(max_ang3);
    flag4=get_max_angle(ind4,n-6,m,intensity_threshold,max_ang4);
    //cout<<"flag4  is "<<flag4<<endl;
    find_angle.push_back(max_ang4);

    if(flag1&&flag2&&flag3&&flag4)
    {
        float min_angle;
        float max_angle;
        min_angle=*min_element(find_angle.begin(),find_angle.end());
        max_angle=*max_element(find_angle.begin(),find_angle.end());
       // cout<<"angle is "<<min_angle<<endl;

         if((0.04*2*PI<min_angle)&&(min_angle<angle_threshold*PI))
        {
            result= 1;
        }
        else {
            result= 0;
        }

    }
    else
    {
        result=0;
    }
    return result;
}
int get_max_angle(vector<int> ind,int n,int m,int intensity_threshold, float &max_ang)
{
    int result;
    vector<int> indd(m);
    float ang = 2*PI/m;
    int count_num=0;
    float dis=0;
    float max_dis=0;
    for(int i=0;i<ind.size();i++)
    {
        if((ind[i]/float(n))>(intensity_threshold)*exp(0.05*n))       // Determine if it is a foreground ray
        {
            indd[count_num]=i;
            count_num++;
        }
    }
    //cout<<"the number of the foreground is "<<count_num<<endl;

    if (count_num > 1)
        {
            for (int i = 0;i < count_num - 1;i++)
            {
                for (int j = i+1;j < count_num;j++)
                {
                        dis = min(indd[j] - indd[i],m-(indd[j] - indd[i]));
                    if (dis > max_dis)
                        max_dis=dis;
                }
            }

            max_ang = (max_dis / m)*2*PI;
            //cout<<"angle is "<<max_ang<<endl;
        }
    else if (count_num == 1)
    {
        max_ang = 1/m;
        //cout<<"ang is "<<max_ang<<endl;
    }
    else
        {
            count_num = 0;
            max_ang = 0;
        }

    if((((max_ang/ang)-count_num)>=-1)&&(((max_ang/ang)-count_num)<=3)&&(max_ang<0.45*2*PI))
    {
        result=1;
    }

    else {
        result=0;
    }
    return result;

}

void thres_segment(V3DLONG size_image, unsigned char * old_image, unsigned char * &binary_image,unsigned char thres)
{
    for(V3DLONG i = 0; i < size_image; i++)
    {
        if(old_image[i] <=thres)
        {
            binary_image[i] = 0;
        }
        else
        {
            binary_image[i] = 255;
        }
    }
}

void XY_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip)
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
                }
            }
        }
    }
}


void XZ_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip)
{

    for(V3DLONG iz = 0; iz < nz; iz++)
    {
        V3DLONG offsetk = iz*nx*ny;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iy = 0; iy < ny; iy++)
            {
                V3DLONG offsetj = iy*nx;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iz*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
        }
    }

}

void YZ_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip)
{
    for(V3DLONG iz = 0; iz < nz; iz++)
    {
        V3DLONG offsetk = iz*nx*ny;
        for(V3DLONG iy = 0; iy < ny; iy++)
        {
            V3DLONG offsetj = iy*nx;
            int max_mip = 0;
            for(V3DLONG ix = 0; ix <nx; ix++)
            {
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iz*ny + iy] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
        }
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
        // fix this bug buy chaowang 2018.11.11
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
