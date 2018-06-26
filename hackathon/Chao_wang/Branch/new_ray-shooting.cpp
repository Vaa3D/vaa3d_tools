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
    vector<int> indd,ind1;//ind1 is the all piex of each ray
                           //indd
    for(int i = 0; i < m; i++)   //m is the numble of the ray
        {
         float sum=0;
         for(int j = 0; j<n; j++)   // n is the numble of the points of the each ray
            {
                 pixe = 0.0;
                {
                    pixe = interp_2d(point_y+ray_y[i][j], point_x+ray_x[i][j], P, sz0,sz1 , point_x, point_y);

                    pixe=exp(0.05*j)*pixe;
                    //v3d_msg(QString("pixe is %1").arg(pixe));

                }
                sum=sum+pixe;
            }
           ind1.push_back(sum);
          // v3d_msg(QString("indli is %1").arg(ind1[i]));
        }



    float max_indd=10;
    for(int s=0;s<ind1.size();s++)

    {
        if(ind1[s]>max_indd)
        {
            max_indd=ind1[s];
        }

    }
   // v3d_msg(QString("max_indd is %1").arg(max_indd));

    for (int i = 0;i < ind1.size();i++)
    {
        if (ind1[i] >= max_indd*0.4)
            {
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
    {
        return 1;
    }
    else
    {
        return 0;
    }



}

float interp_2d(float point_y,float point_x,unsigned char * PP,V3DLONG sz0,V3DLONG sz1,int old_x,int old_y)
{
    float result;

    if(point_y < 0||point_x < 0||point_y > sz1-1||point_x > sz0-1)
        return 0.0;
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

//set<int> GetAi(int a[], int length)//获取A0~A5
//{
//    set<int> vec;
//    int neighbour[] = { 1,2,4,8,16,32,64,128,1,2,4,8,16,32,64 };
//    for (int i = 0; i<length; i++)
//        for (int j = 0; j<8; j++)
//        {
//            int sum = 0;
//            for (int k = j; k <= j + a[i]; k++)
//                sum += neighbour[k];
//            vec.insert(sum);

//            std::cout << sum << " ";
//        }
//    std::cout << std::endl;
//    return vec;
//}
//bool erodephase(vector<border_point> &border, unsigned char * &Input, int neighbour[][3], const set<int>& A,V3DLONG ix,V3DLONG iy)
//{
//    auto pt = border.begin();
//    bool result = false;
//    while(pt!= border.end())
//    {

//        int weight = 0;
//        for (int j = -1; j <= 1; ++j)
//        for (int k = -1; k <= 1; k++)
//        //weight += neighbour[j + 1][k + 1] * Input.at<uchar>(pt->y + j, pt->x + k);
//        weight += neighbour[j + 1][k + 1] * Input[(pt->border_point.y_point+j)*ix+pt->border_point.x_point+k];
//        if (std::find(A.begin(), A.end(), weight) != A.end())
//        {
//            Input[(pt->border_point.y_point)*ix+pt->border_point.x_point]= 0;
//            pt=border.erase(pt);
//            result = true;
//        }
//        else
//            ++pt;
//    }
//    return result;

//}

//void findborder(vector<border_point>& border, const unsigned char * &Input,V3DLONG ix,V3DLONG iy)
//{
//    int cnt = 0;
//    int rows = iy;
//    int cols = ix;
//    unsigned char *bordermat=new unsigned char [];
//    bordermat=Input;
//    //cv::Mat bordermat = Input.clone();
//        for (int row = 1; row<rows - 1; ++row)
//        for (int col = 1; col<cols - 1; ++col)
//        {
//            int weight = 0;
//            for (int j = -1; j <= 1; ++j)
//            for (int k = -1; k <= 1; k++)
//                {
//                    //if (Input.at<uchar>(row + j, col + k) == 1)
//                    if (Input[(row+j)*ix+col+k] == 1)
//                        ++cnt;
//                }
//            if (cnt == 9)
//                bordermat[row*iy+col] = 0;
//            cnt = 0;
//        }

//    for (int row = 1; row<rows - 1; ++row)
//        for (int col = 1; col < cols - 1; ++col)
//        {
//            if (bordermat[row*iy+col] == 1)
//                //border.push_back(cv::Point2i(col, row));
//                border_point pp;
//                pp.x_point=col;
//                pp.y_point=row;
//                border.push_back(pp);
//        }

//}

//void finalerode( unsigned char * &Input ,int neighbour[][3], const set<int>& A,V3DLONG ix,V3DLONG iy)
//{
//    int rows = iy;
//    int cols = ix;
//    for (int m = 1; m<rows - 1; ++m)
//        for (int n = 1; n<cols - 1; ++n)
//        {
//            int weight = 0;
//            for (int j = -1; j <= 1; ++j)
//                for (int k = -1; k <= 1; k++)
//                {
//                    //weight += neighbour[j + 1][k + 1] * Input.at<uchar>(m + j, n + k);
//                    weight += neighbour[j + 1][k + 1] * Input[(m+j)*ix+n+k];
//                }

//            if (std::find(A.begin(), A.end(), weight) != A.end())
//                //Input.at<uchar>(m, n) = 0;
//                Input[m*ix+n] = 0;

//        }
//}

//void thin(unsigned char* &Input) //Input是二值图像
//{
//    int a0[] = { 1,2,3,4,5,6 };
//    int a1[] = { 2 };
//    int a2[] = { 2,3 };
//    int a3[] = { 2,3,4 };
//    int a4[] = { 2,3,4,5 };
//    int a5[] = { 2,3,4,5,6 };
//    set<int> A0 = GetAi(a0, 6);
//    set<int> A1 = GetAi(a1, 1);
//    set<int> A2 = GetAi(a2, 2);
//    set<int> A3 = GetAi(a3, 3);
//    set<int> A4 = GetAi(a4, 4);
//    set<int> A5 = GetAi(a5, 5);
//    //list<cv::Point2i> border;
//    struct border_point
//    {
//        V3DLONG x_point;
//        V3DLONG y_point;
//    };
//    vector<border_point> border;
//    bool continue_ = true;
//    int neighbour[3][3] = {
//        { 128,1,2 },
//        { 64,0,4 },
//        { 32,16,8 }
//    };
//    while (continue_)
//    {
//        //找边界，对原文方法做了小改变，但影响不大。
//        continue_ = false;

//        findborder(border, Input,nx,ny);//Phase0
//        //可以在下面每一步打印结果，看每一步对提取骨架的贡献
//        erodephase(border, Input, neighbour,A1);//Phase1
//        erodephase(border, Input, neighbour, A2);//Phase2
//        erodephase(border, Input, neighbour, A3);//Phase3
//        erodephase(border, Input, neighbour, A4);//Phase4
//        continue_ =erodephase(border, Input, neighbour, A5);//Phase5
//        border.clear();

//    }
//    finalerode(Input,  neighbour, A0);//最后一步

//}



