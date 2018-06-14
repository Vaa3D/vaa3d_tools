
#include "v3d_message.h"
//#include "stackutil.h"
//#include "Branch_c_plugin.h"
#include <iostream>
#include <math.h>
#include <vector>
#include "v3d_basicdatatype.h"
#include <stdio.h>
#include "rayshooting_func.h"




using namespace std;
#define PI 3.1415926
#define EXP 2.71828


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

bool rayinten_2D(int point_y,int point_x,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG sz0,V3DLONG sz1 )
{
    int count=0;
    int point[2] = {point_y,point_x};
    //int  point_coordinate[3];
    float pixe = 0.0;
    //vector<int> indd[m],ind1[m];
     vector<int> indd,ind1;
    for(int i = 0; i < m; i++)   //m is the numble of the ray
        {
         float sum=0;
         for(int j = 0; j<n; j++)   // n is the numble of the points of the each ray
            {
                 pixe = 0.0;

                {
                    pixe = interp_2d(point[1]+ray_y[i][j], point[0]+ray_x[i][j], P, sz0,sz1 );
                    //v3d_msg(QString("pixe is %1").arg(pixe));
                    if (j<=n/4||j>=0)
                    {
                      pixe=2*pixe;
                    }
                    else if (j>n/4||j<=n/2)
                    {
                       pixe=1*pixe;
                    }
                    else if ((j>n/2)||j<=(3*n/4))
                    {
                       pixe=1*pixe;
                    }
                    else
                    {
                       pixe=2*pixe;
                    }

                }
                sum=sum+pixe;

            }
           ind1.push_back(sum);
          //ind1[i]=sum; //i is the numble of the ray
          // v3d_msg(QString("indli is %1").arg(ind1[i]));
        }
    float max_indd=10;
    for(int s=0;s<m;s++)
    {

        if(ind1[s]>max_indd)
        {
            max_indd=ind1[s];
        }

    }
    //v3d_msg(QString("max_indd is %1").arg(max_indd));

    for (int i = 0;i < m;i++)
    {
        if (ind1[i] >= max_indd*0.4)
            {
                //indd[count] = i;
                 indd.push_back(i);
                  count++;
            }
    }
    //v3d_msg(QString("count is %1").arg(count));
    //float dis=0,max_dis=0;
    int ray_distance=0;
    int branch_flag=0;
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
            long dis=indd[1]-indd[count];
            if(abs(dis)==(m-1))
                branch_flag=branch_flag-1;
        }
    //v3d_msg(QString("branch flag is %1").arg(branch_flag));

    if(branch_flag>2)
        return true;

}

float interp_2d(float point_y,float point_x,unsigned char * PP,V3DLONG sz0,V3DLONG sz1)
{
    float result;

    if(point_y < 0||point_x < 0||point_y > sz1-1||point_x > sz0-1)
        return 0.0;
    else if(point_x < 1||point_y < 1||point_x > sz0-2||point_y > sz1-2)
    {
        result = get_2D_ValueUINT8(point_y,point_x,PP, sz0, sz1);
        return result;
    }
    else
    {
        int y1 = get_2D_ValueUINT8(ceil(point_y), ceil(point_x),PP,sz0,sz1);
        int y0 = get_2D_ValueUINT8(floor(point_y), ceil(point_x),PP, sz0,sz1);
        int x1 = get_2D_ValueUINT8(ceil(point_y), floor(point_x),PP, sz0,sz1);
        int x0 = get_2D_ValueUINT8(floor(point_y), floor(point_x),PP, sz0,sz1);

        result = x0*(point_x-floor(point_x))*(point_y-floor(point_y))+x1*(ceil(point_x)-point_x)*(point_y-floor(point_y))
            +y0*(point_x-floor(point_x))*(ceil(point_y)-point_y)+y1*(ceil(point_x)-point_x)*(ceil(point_y)-point_y);
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

double *getOneGuassionArray(int size, double sigma)
{
    double sum = 0.0;
    //定义高斯核半径
    int R = size / 2;

    //建立一个size大小的动态一维数组
    double *arr = new double[size];
    for (int i = 0; i < size; i++)
    {

        // 高斯函数前的常数可以不用计算，会在归一化的过程中给消去
        arr[i] = exp(-((i - R)*(i - R)) / (2 * sigma*sigma));
        sum += arr[i];//将所有的值进行相加

    }
    //进行归一化
    for (int i = 0; i < size; i++)
    {
        arr[i] /= sum;
        cout << arr[i] << endl;
    }
    return arr;
}

void MyGaussianBlur(   float * & srcimgae , float* & dst, int size,V3DLONG x1,V3DLONG y1)
{
       int R = size / 2;
       dst=srcimgae;

       double* arr=0;
       arr = getOneGuassionArray(size, 1);//先求出高斯数组

       //遍历图像 水平方向的卷积
       for (int i = R; i < y1 - R; i++)
       {
           for (int j = R; j < x1 - R; j++)
           {
               float GuassionSum[3] = { 0 };


               //滑窗搜索完成高斯核平滑
               for (int k = -R; k <= R; k++)
               {
                       GuassionSum[0] += arr[R + k] *dst[(i+k)*x1+j];
                       //行不变，列变换，先做水平方向的卷积
               }
                   if (GuassionSum[0] < 0)
                       GuassionSum[0] = 0;
                   else if (GuassionSum[0] > 255)
                       GuassionSum[0] = 255;
                   dst[i*x1+j]=GuassionSum[0];

           }
       }


       //竖直方向
       for (int i = R; i < y1-R; i++)
       {
           for (int j = R; j < x1-R; j++)
           {
               float GuassionSum[3] = { 0 };
               //滑窗搜索完成高斯核平滑

               for (int k = -R; k <= R; k++)
               {
                       GuassionSum[0] += arr[R + k] * dst[i*x1+j+k];//行变，列不换，再做竖直方向的卷积
               }
                   if (GuassionSum[0] < 0)
                       GuassionSum[0] = 0;
                   else if (GuassionSum[0] > 255)
                       GuassionSum[0] = 255;
                   dst[i*x1+j]=GuassionSum[0];

           }
       }
       //v3d_msg(QString("new dst is %1").arg(dst[200]));
       delete[] arr;
}
void harrisResponse(float* & Gxx, float* & Gyy, float* & Gxy, float*  & Hresult, float k,V3DLONG x1,V3DLONG y1)
{

    for (int i = 0; i < y1; i++)
    {
        for (int j = 0; j < x1; j++)
        {

            float a = Gxx[i*x1+j];
            float b = Gyy[i*x1+j];
            float c = Gxy[i*x1+j];
            Hresult[i*x1+j] = a*b - c*c - k*(a + b)*(a + b);
        }

    }
}
