#include "matsource.h"
#include "mathtouse.h"
MatSource::MatSource(QString fileName, int basicOverlap1, int basicOverlap2, int MIPsize)
{
     this->basicOverlap1 = basicOverlap1;
     this->basicOverlap2 = basicOverlap2;
     this->MIPsize = MIPsize;
     readMat(fileName);
}

void MatSource::readMat(QString fileName)
{
    QDir file(fileName);

}

vector<String> MatSource::readMats(QString fileName)
{

   vector<String> tile;
   QDir fileDir(fileName);
   QStringList filters;
   filters << "*.tif" << "*.tiff"<<"*.raw";
   QFileInfoList imageList=fileDir.entryInfoList(filters, QDir::Files);
   for(int i_image = 0; i_image < imageList.size(); i_image ++)
   {
       //cout<<"This is "<<i_image+1<<" image."<<endl;

       String image_i = imageList[i_image].filePath().toStdString();

       tile.push_back(image_i);
   }
   return tile;
}

x_y_shift MatSource::A_B_y_z_shift(const vector<String>& tileA,const vector<String>& tileB,int basicOverlap,QString orientation,int rows,int cols,int MIPsize)
{
//计算
    Mat MIPA, MIPB;
  if (orientation=="left" )
  {
      MIPA=getMIP(tileA,basicOverlap,"left",rows,cols,MIPsize);
      MIPB=getMIP(tileB,basicOverlap,"right",rows,cols,MIPsize);

  }
  if (orientation=="right" )
  {
      MIPA=getMIP(tileA,basicOverlap,"right",rows,cols,MIPsize);
      MIPB=getMIP(tileB,basicOverlap,"left",rows,cols,MIPsize);

  }
  if (orientation=="up" )
  {
      MIPA=getMIP(tileA,basicOverlap,"up",rows,cols,MIPsize);
      MIPB=getMIP(tileB,basicOverlap,"down",rows,cols,MIPsize);

  }
  if (orientation=="down" )
  {
      MIPA=getMIP(tileA,basicOverlap,"down",rows,cols,MIPsize);
      MIPB=getMIP(tileB,basicOverlap,"up",rows,cols,MIPsize);

  }

  //结果
  return mat_x_y_shift(MIPA, MIPB,"leftright",0);

}

vector<x_y_shift> MatSource::tile_x_y_shift(const vector<String>& tileA,const vector<String>& tileB,QString orientation,int basicOverlap)
{
    vector<x_y_shift> result;
    int n=tileA.size();
    if(n>tileB.size())n=tileB.size();
    for(int i=0;i<n;i++)
    {
        Mat tempA=imread(tileA[i],2);
        Mat tempB=imread(tileB[i],2);
        result.push_back(mat_x_y_shift(tempA,tempB,orientation,basicOverlap));
    }
    return result;
}

x_y_shift MatSource::mat_x_y_shift(const Mat& matA,const Mat& matB,QString orientation,int basicOverlap)
{

    x_y_shift x_y=x_y_shift(0,0);
//计算
       int rows=(matA.rows<matB.rows)?matA.rows:matB.rows;
       int cols=(matA.cols<matB.cols)?matA.cols:matB.cols;
       int overlap_range = basicOverlap/10;
       int shift_range = (rows<cols)?rows/100:cols/100;
       int overlap=0,shift=0;
       double minError = INT_MAX;


       if(orientation == "left" )
       {

           for(int ss1 = basicOverlap-overlap_range; ss1 <= basicOverlap+overlap_range; ss1 ++)
           {
               for(int ss2 = -shift_range; ss2 <= shift_range; ss2 ++)
               {
                   double temp=0;
                   if(ss2<0)
                   {
                       for(int i=0;i<rows+ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matA.at<unsigned short>(-ss2+i,cols-ss1+j))-double(matB.at<unsigned short>(i,j)),2);
                           }
                       }
                   }
                   else
                   {
                       for(int i=0;i<rows-ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matA.at<unsigned short>(i,cols-ss1+j))-double(matB.at<unsigned short>(ss2+i,j)),2);
                           }
                       }
                   }
                   if(temp/(ss1*(rows-abs(ss2)))<minError)
                   {
                       minError=temp/(ss1*(rows-abs(ss2)));
                       overlap=ss1;
                       shift=ss2;
                   }
               }
           }
       }

       else if(orientation == "right" )
       {

           for(int ss1 = basicOverlap-overlap_range; ss1 <= basicOverlap+overlap_range; ss1 ++)
           {
               for(int ss2 = -shift_range; ss2 <= shift_range; ss2 ++)
               {
                   double temp=0;
                   if(ss2<0)
                   {
                       for(int i=0;i<rows+ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matB.at<unsigned short>(-ss2+i,cols-ss1+j))-double(matA.at<unsigned short>(i,j)),2);
                           }
                       }
                   }
                   else
                   {
                       for(int i=0;i<rows-ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matB.at<unsigned short>(i,cols-ss1+j))-double(matA.at<unsigned short>(ss2+i,j)),2);
                           }
                       }
                   }
                   if(temp/(ss1*(rows-abs(ss2)))<minError)
                   {
                       minError=temp/(ss1*(rows-abs(ss2)));
                       overlap=ss1;
                       shift=-ss2;
                   }
               }
           }
       }

       else if(orientation == "down" )
       {

           for(int ss1 = basicOverlap-overlap_range; ss1 <= basicOverlap+overlap_range; ss1 ++)
           {
               for(int ss2 = -shift_range; ss2 <= shift_range; ss2 ++)
               {
                   double temp=0;
                   if(ss2<0)
                   {
                       for(int i=0;i<ss1;i++)
                       {
                           for(int j=0;j<cols+ss2;j++)
                           {
                           temp=temp+pow(double(matA.at<unsigned short>(i+rows-ss1,-ss2+j))-double(matB.at<unsigned short>(i,j)),2);
                           }
                       }
                   }
                   else
                   {
                       for(int i=0;i<ss1;i++)
                       {
                           for(int j=0;j<cols-ss2;j++)
                           {
                           temp=temp+pow(double(matA.at<unsigned short>(i+rows-ss1,j))-double(matB.at<unsigned short>(i,j+ss2)),2);
                           }
                       }
                   }
                   if(temp/(ss1*(cols-abs(ss2)))<minError)
                   {
                       minError=temp/(ss1*(cols-abs(ss2)));
                       overlap=ss1;
                       shift=ss2;
                   }
               }
           }
       }
       else if(orientation == "up" )
       {

           for(int ss1 = basicOverlap-overlap_range; ss1 <= basicOverlap+overlap_range; ss1 ++)
           {
               for(int ss2 = -shift_range; ss2 <= shift_range; ss2 ++)
               {
                   double temp=0;
                   if(ss2<0)
                   {
                       for(int i=0;i<ss1;i++)
                       {
                           for(int j=0;j<cols+ss2;j++)
                           {
                           temp=temp+pow(double(matB.at<unsigned short>(i+rows-ss1,-ss2+j))-double(matA.at<unsigned short>(i,j)),2);
                           }
                       }
                   }
                   else
                   {
                       for(int i=0;i<ss1;i++)
                       {
                           for(int j=0;j<cols-ss2;j++)
                           {
                           temp=temp+pow(double(matB.at<unsigned short>(i+rows-ss1,j))-double(matA.at<unsigned short>(i,j+ss2)),2);
                           }
                       }
                   }
                   if(temp/(ss1*(cols-abs(ss2)))<minError)
                   {
                       minError=temp/(ss1*(cols-abs(ss2)));
                       overlap=ss1;
                       shift=-ss2;
                   }
               }
           }
       }
       else if(orientation == "leftright"&& basicOverlap==0)
       {

           shift_range=rows/100;
           for(int ss1 =cols- cols/100; ss1 <= cols; ss1 ++)
           {
               for(int ss2 = -shift_range; ss2 <= shift_range; ss2 ++)
               {
                   double temp=0;
                   if(ss2<0)
                   {
                       for(int i=0;i<rows+ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matA.at<unsigned short>(-ss2+i,cols-ss1+j))-double(matB.at<unsigned short>(i,j)),2);
                           }
                       }
                   }
                   else
                   {
                       for(int i=0;i<rows-ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matA.at<unsigned short>(i,cols-ss1+j))-double(matB.at<unsigned short>(ss2+i,j)),2);
                           }
                       }
                   }
                   if(temp/(ss1*(rows-abs(ss2)))<minError)
                   {
                       minError=temp/(ss1*(rows-abs(ss2)));
                       overlap=ss1;
                       shift=ss2;
                   }
               }
           }
           for(int ss1 = cols- cols/100; ss1 <= cols; ss1 ++)
           {
               for(int ss2 = -shift_range; ss2 <= shift_range; ss2 ++)
               {
                   double temp=0;
                   if(ss2<0)
                   {
                       for(int i=0;i<rows+ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matB.at<unsigned short>(-ss2+i,cols-ss1+j))-double(matA.at<unsigned short>(i,j)),2);
                           }
                       }
                   }
                   else
                   {
                       for(int i=0;i<rows-ss2;i++)
                       {
                           for(int j=0;j<ss1;j++)
                           {
                           temp=temp+pow(double(matB.at<unsigned short>(i,cols-ss1+j))-double(matA.at<unsigned short>(ss2+i,j)),2);
                           }
                       }
                   }
                   if(temp/(ss1*(rows-abs(ss2)))<minError)
                   {
                       minError=temp/(ss1*(rows-abs(ss2)));
                       overlap=ss1;
                       shift=-ss2;
                   }
               }
           }
       }

 x_y=x_y_shift(overlap,shift);

//results
    return  x_y;
}

double MatSource::getMatDistortionK1UseOverLapAndShift(const Mat& matA,const Mat& matB,QString orientation,int overlap,int shift,int num)
{

    int rows=(matA.rows<matB.rows)?matA.rows:matB.rows;
    int cols=(matA.cols<matB.cols)?matA.cols:matB.cols;
    int range;
    int start1;
    double center;
    double half;
    if(orientation=="left" || orientation=="right")
    {
        range=(rows-abs(shift)*2)/num;
        start1=(rows-range*num)/2;
        center=rows/2;
        half=cols/2;
    }
    else
    {
        range=(cols-abs(shift)*2)/num;
        start1=(cols-range*num)/2;
        center=cols/2;
        half=rows/2;
    }
    double k1;
    vector<int> distortLine = getMatDistortionOverLaps(matA,matB,orientation,overlap,shift,num);
    vector<double> x;
    vector<double> y;
    for(int i=0;i<num;i++)
    {
        x.push_back(half-distortLine[i]);
        y.push_back(fabs(start1+i*range+range/2.0-center));
    }
    x.push_back(half-distortLine[num]);//put into y=0
    y.push_back(0.0);//put into y=0
  //distortion

    double temp1=0.0,temp2=0.0,temp3=0.0;
    for(int i=0;i<num;i++)
    {
        temp1=temp1+x[i];
        temp2=temp2+y[i]*y[i]/(x[i]*x[i]);
    }

    temp3=(num*x[num]-temp1)/temp2+x[num];

    k1=(x[num]-temp3)/(temp3*temp3*temp3);

    return  k1/2;//left-right
}

double MatSource::tileZEnlargeRatioUseOverLapAndShift(const vector<String>& tileA,const vector<String>& tileB, int basicOverlap)
{
  double zEnlargeRatio = 0.0;






  return zEnlargeRatio;
}

Mat MatSource::getMIP(const vector<String>& tileA,int basicOverlap,QString orientation,int rows,int cols,int MIPsize)
{

    Mat** tile=new Mat*[tileA.size()];
    for(int i=0;i<tileA.size();i++)
    {

        Mat temp=imread(tileA[i],2);
        Mat* temp2=new Mat(temp.rows,temp.cols,CV_16UC1);

        for(int j=0;j<rows;j++)
        {
            for(int k=0;k<cols;k++)
            {
                temp2->at<unsigned short>(j,k)=temp.at<unsigned short>(j,k);
            }
        }
        tile[i]=temp2;

    }

    int MIPCols=cols;
    if (orientation=="left" ||orientation=="right")MIPCols=rows;
    Mat MIP = Mat::zeros(tileA.size(),MIPCols, CV_16UC1);
//calculation
    int centerOverlap;
    if(orientation == "left" || orientation == "right")
    {
        if(orientation == "left")
            centerOverlap = cols-basicOverlap/2;
        else
            centerOverlap = basicOverlap/2;

        for(int i = 0; i <tileA.size(); i ++)
        {
            for(int j = 0; j < rows; j ++)
            {

                unsigned short maxValue = 0;
                for(int k = centerOverlap-MIPsize; k < centerOverlap+MIPsize; k++)
                {

                    maxValue = ((maxValue >= tile[i]->at<unsigned short>(j, k)) ? maxValue : tile[i]->at<unsigned short>(j, k));
                }
                MIP.at<unsigned short>(i, j) = maxValue;
            }
        }
    }
    else if(orientation == "up" || orientation == "down")
    {
        if(orientation == "down")
            centerOverlap = rows - basicOverlap/2;
        else
            centerOverlap = basicOverlap/2;

        for(int i = 0; i < tileA.size(); i ++)
        {
            for(int j = 0; j < cols; j ++)
            {
                unsigned short maxValue = 0;
                for(int k = centerOverlap-MIPsize; k < centerOverlap+MIPsize; k ++)
                {

                    maxValue = ((maxValue >= tile[i]->at<unsigned short>(k, j)) ? maxValue : tile[i]->at<unsigned short>(k, j));
                }
                 MIP.at<unsigned short>(i, j) = maxValue;
            }
        }
    }
 //results
    for(int i=0;i< tileA.size();i++)
        {
        delete tile[i];
        }
    delete[] tile;

    return MIP;
}

vector<int> MatSource::getMatDistortionOverLaps(const Mat& matA,const Mat& matB,QString orientation,int overlap,int shift,int num)
{
    int rows=(matA.rows<matB.rows)?matA.rows:matB.rows;
    int cols=(matA.cols<matB.cols)?matA.cols:matB.cols;
    //cout<<shift<<endl;cout<<matA.cols<<endl;
    vector<int> result;
    int range;
    if(orientation=="left" || orientation=="right")
    {
        range=(rows-abs(shift)*2)/num;
        int start1=(rows-range*num)/2;
        int start2=abs(shift)+(rows-range*num)/2;
        if(shift>0)
        {
            for(int i=0;i<num;i++)
            {
               Mat tempA=matA.rowRange(start1+i*range,start1+i*range+range);
               Mat tempB=matB.rowRange(start2+i*range,start2+i*range+range);
               result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);
            }
            Mat tempA=matA.rowRange(rows/2-range/2,rows/2+range/2);
            Mat tempB=matB.rowRange(abs(shift)+rows/2-range/2,abs(shift)+rows/2+range/2);
            result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);
        }
        else
        {
            for(int i=0;i<num;i++)
            {
               Mat tempB=matB.rowRange(start1+i*range,start1+i*range+range);
               Mat tempA=matA.rowRange(start2+i*range,start2+i*range+range);
               result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);
            }
            Mat tempB=matB.rowRange(rows/2-range/2,rows/2+range/2);
            Mat tempA=matA.rowRange(abs(shift)+rows/2-range/2,abs(shift)+rows/2+range/2);
            result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);
        }
    }
    if(orientation=="up" || orientation=="down")
    {
        range=(cols-abs(shift)*2)/num;
        int start1=(cols-range*num)/2;
        int start2=abs(shift)+(cols-range*num)/2;
        if(shift>0)
        {
            for(int i=0;i<num;i++)
            {
                Mat tempA=matA.colRange(start1+i*range,start1+i*range+range);
                Mat tempB=matB.colRange(start2+i*range,start2+i*range+range);
               result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);

            }
            Mat tempA=matA.colRange(cols/2-range/2,cols/2+range/2);
            Mat tempB=matB.colRange(abs(shift)+cols/2-range/2,abs(shift)+cols/2+range/2);
            result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);
        }
        else
        {
            for(int i=0;i<num;i++)
            {
                Mat tempB=matB.colRange(start1+i*range,start1+i*range+range);
                Mat tempA=matA.colRange(start2+i*range,start2+i*range+range);
               result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);
            }
            Mat tempB=matB.colRange(cols/2-range/2,cols/2+range/2);
            Mat tempA=matA.colRange(abs(shift)+cols/2-range/2,abs(shift)+cols/2+range/2);
            result.push_back(mat_x_y_shift(tempA,tempB,orientation,overlap).x);
        }
    }

    return result;
}

vector<vector<double>> MatSource::getDistortK1andZenlargeByOverlapandShift(const vector<String> &tileA, const vector<String> &tileB, QString orientation, vector<x_y_shift> tlieABoverlapsAndSshifts)
{
    vector<vector<double>> result;
    vector<double> result_k1;
    vector<double> Zenlarge = MathToUse::centerExpansionRatio(tlieABoverlapsAndSshifts);
    double sumk1=0;
    for(int i=0;i<tlieABoverlapsAndSshifts.size();i++)
    {
        Mat tempMatA = imread(tileA[i],2);
        Mat tempMatB = imread(tileB[i],2);
        sumk1=sumk1+getMatDistortionK1UseOverLapAndShift(tempMatA,tempMatB,orientation,tlieABoverlapsAndSshifts[i].x,tlieABoverlapsAndSshifts[i].y);
    }
    double k1=sumk1/tlieABoverlapsAndSshifts.size();
    result_k1.push_back(k1);
    result.push_back(result_k1);
    result.push_back(Zenlarge);
    return result;
}
