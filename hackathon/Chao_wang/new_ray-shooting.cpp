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
    float max_indd=10;
    int branch_flag=1;
    vector<int> indd,ind1;//ind1 is the all piex of each ray

    for(int i = 0; i < m; i++)   //m is the numble of the ray
    {
       sum=0;
     for(int j = 0; j < n; j++)    // n is the numble of the points of the each ray
        {
            pixe = project_interp_2d(point_y+ray_y[i][j], point_x+ray_x[i][j], P, sz0,sz1 , point_x, point_y);
            pixe=exp(0.05*(j+1))*pixe;
//            v3d_msg(QString("pixe is %1").arg(pixe));
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

    int max_number=0;
    if((max_indd/n)>5)
    {
        for(int num=0;num<ind1.size();num++)
        {
            if (ind1[num] >= max_indd*0.5)
                {
//                     v3d_msg(QString("location is %1").arg(num));
                     indd.push_back(num);
                     if(num>=max_number)
                     {
                         max_number=num;
                     }
                     count++;
                }
        }
//        v3d_msg(QString("max_number is %1").arg(max_number));

        int ray_distance=0;
//        v3d_msg(QString("count is %1").arg(count));
        if (count > 1)
            {
                for (int i = 0;i < count - 2;i++)
                {
                   ray_distance=abs(indd[i]-indd[i+1]);
                   if(ray_distance>=2)
                   {
//                       v3d_msg(QString("indd[i] is %1, indd[i+1] is %2").arg(indd[i]).arg(indd[i+1]));
                       branch_flag=branch_flag+1;
                   }
                }
//                v3d_msg(QString("indd[0] is %1, indd[count] is %2").arg(indd[0]).arg(indd[count-1]));
                long dis=indd[0]-indd[count];

                if(abs(dis)==(m-1))
                {
                  branch_flag=branch_flag-1;
                }

            }

    }
//    v3d_msg(QString("flag is %1").arg(branch_flag));
    if(branch_flag>2)
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
void harrisResponse(float* & Gxx, float* & Gyy, float* & Gxy, float*  & Hresult, float k,V3DLONG x1,V3DLONG y1,float &max_hresult)
{

    for (int i = 0; i < y1; i++)
    {
        for (int j = 0; j < x1; j++)
        {

            float a = Gxx[i*x1+j];
            float b = Gyy[i*x1+j];
            float c = Gxy[i*x1+j];
            Hresult[i*x1+j] = a*b - c*c - k*(a + b)*(a + b);
            if(Hresult[i*x1+j]>max_hresult)
            {
                max_hresult=Hresult[i*x1+j];
            }
        }

    }
}

void mul(unsigned char * & G, float * &GG,V3DLONG x1,V3DLONG y1)
{
    for (int i = 0; i < y1; i++)
     {
          for (int j = 0; j < x1; j++)
          {
               GG[i*x1+j] = G[i*x1+j]*G[i*x1+j];
          }
     }
}

void mul_xy(unsigned char * & Gx, unsigned char * &Gy, float *& Gxy, V3DLONG x1,V3DLONG y1)
{
    for (int i = 0; i < y1; i++)
     {
          for (int j = 0; j < x1; j++)
          {
               Gxy[i*x1+j] = Gx[i*x1+j]*Gy[i*x1+j];
          }
     }
}


bool mip_z_slices(Image4DSimple * pp, Image4DSimple & outImage,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum)
{
    if (!pp|| !pp->valid())
        return false;

    V3DLONG sz0 = pp->getXDim();
    V3DLONG sz1 = pp->getYDim();
    V3DLONG sz2 = pp->getZDim();
    V3DLONG sz3 = pp->getCDim();

    //
    V3DLONG sz2_new = 1;

//    outImage.createBlankImage(sz0, sz1, sz2_new, sz3, subject->getDatatype());
//    if (!outImage.valid())
//        return false;

    V3DLONG c, pagesz;
    for (V3DLONG i=startnum; i<=endnum; i+=increment)
    {
            pagesz = sz0*sz1;
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
                unsigned char *src = pp->getRawDataAtChannel(c) + i*pagesz;
                if (i==startnum)
                {
                    memcpy(dst, src, pagesz);
                }
                else
                {
                    for (V3DLONG j=0; j<pagesz; j++)
                        if (dst[j]<src[j]) dst[j] = src[j];
                }
            }

    }

    return true;
}
bool parseFormatString(QString t, V3DLONG & startnum, V3DLONG & increment, V3DLONG & endnum, V3DLONG sz2)
{
    if (sz2<=0)
        return false;

    QStringList list = t.split(":");
    if (list.size()<2)
        return false;

    bool ok;

    startnum = list.at(0).toLong(&ok)-1;
    if (!ok)
        startnum = 0;

    if (list.size()==2)
    {
        increment = 1;
        endnum = list.at(1).toLong(&ok);
        if (!ok)
            endnum = sz2-1;
    }
    else
    {
        increment = list.at(1).toLong(&ok);
        if (!ok) increment = 1;
        endnum = list.at(2).toLong(&ok);
        if (!ok) endnum = sz2-1;
    }

    if (increment<0) //this will not reverse the order of all z slices in a stack. This can be enhanced later.
        increment = -increment;
    if (endnum>=sz2)
        endnum = sz2-1;
    if (startnum<0)
        startnum = 0;
    if (startnum>endnum)
    {
        V3DLONG tmp=endnum; endnum=startnum; startnum=tmp;
    }

    qDebug() << " start=" << startnum << " increment=" << increment << " end=" << endnum;
    return true;
}

void XY_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip)
{
    for(int iy = 0; iy < ny; iy++)
    {
        int offsetj = iy*nx;
        for(int ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(int iz = 0; iz < nz; iz++)
            {
                int offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
//            v3d_msg(QString("xy_mip is %1").arg(max_mip));
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
                    v3d_msg(QString("location is %1, pixe is %2").arg(offsetk + offsetj + ix).arg(datald[offsetk + offsetj + ix]));
                    max_mip = datald[offsetk + offsetj + ix];
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

void XZ_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip)
{
    v3d_msg(QString("come here"));

    for(int iz = 0; iz < nz; iz++)
    {
        int offsetk = iz*nx*ny;
        for(int ix = 0; ix < nx; ix++)
        {
            unsigned char max_mip = 0;
            for(int iy = 0; iy < ny; iy++)
            {
                int offsetj = iy*nx;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    v3d_msg(QString("come here"));
                    v3d_msg(QString("pixe of mip is %1").arg(image_mip[0]));
                    image_mip[iz*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
            v3d_msg(QString("xz_mip is %1").arg(max_mip));
        }
    }

}

void YZ_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip)
{
    v3d_msg(QString("come here"));
    for(int iz = 0; iz < nz; iz++)
    {
        v3d_msg(QString("come here"));
        int offsetk = iz*nx*ny;
        v3d_msg(QString("offsetk is %1").arg(offsetk));
        for(int iy = 0; iy < ny; iy++)
        {
            int offsetj = iy*nx;
            int max_mip = 0;
            for(int ix = 0; ix < nx; ix++)
            {
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iz*ny + iy] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                    v3d_msg(QString("location is %1, pixe is %2").arg(offsetk + offsetj + ix).arg(datald[offsetk + offsetj + ix]));
                }
            }
            v3d_msg(QString("yz_mip is %1").arg(max_mip));
        }
    }

}






