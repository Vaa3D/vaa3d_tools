//========================================================================
// This is a sample main function for using the implementation and header
// files LKM.cpp and LKM.h respectively
//========================================================================

#include <vector>
#include <string>
#include "LKM.h"
#include "mex.h"
#ifdef WINDOWS
#include "BMPhandler.h"
#else
#include <cv.h>
#include <highgui.h>
#include <fstream>
//#include "utils.h"
#endif

using namespace std;


string getNameFromPathWithoutExtension(string path){
  string nameWith =  path.substr(path.find_last_of("/\\")+1);
  string nameWithout = nameWith.substr(0,nameWith.find_last_of("."));
  return nameWithout;
}

//=================================================================================
/// DrawContoursAroundSegments
///
/// Internal contour drawing option exists. One only needs to comment the 'if'
/// statement inside the loop that looks at neighbourhood.
//=================================================================================
void DrawContoursAroundSegments(
                                UINT*			img,//contours will be drawn on this image
                                sidType*			labels,
                                const int&				width,
                                const int&				height,
                                const UINT&				color )
{
  const int dx8[8] = {-1, -1,  0,  1, 1, 1, 0, -1};
  const int dy8[8] = { 0, -1, -1, -1, 0, 1, 1,  1};

  int sz = width*height;

  vector<bool> istaken(sz, false);

  int mainindex(0);
  int cind(0);
  for( int j = 0; j < height; j++ )
    {
      for( int k = 0; k < width; k++ )
        {
          int np(0);
          for( int i = 0; i < 8; i++ )
            {
              int x = k + dx8[i];
              int y = j + dy8[i];

              if( (x >= 0 && x < width) && (y >= 0 && y < height) )
                {
                  int index = y*width + x;

                  if( false == istaken[index] )//comment this to obtain internal contours
                    {
                      if( labels[mainindex] != labels[index] ) np++;
                    }
                }
            }
          if( np > 1 )
            {
              istaken[mainindex] = true;
              img[mainindex] = color;
              cind++;
            }
          mainindex++;
        }
    }
}

#ifndef WINDOWS

//===========================================================================
///	SaveUINTBuffer
///
///	Provides UINT buffer after the necessary vertical flip and BRG to RGB ordering
//===========================================================================
void SaveImage(
               UINT*	ubuff,				// RGB buffer
               const int&			width,				// size
               const int&			height,
               const string&		fileName)			// filename to be given; even if whole path is given, it is still the filename that is used
{
  IplImage* img=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3); 
  uchar* pValue;
  int idx = 0;

  for(int j=0;j<img->height;j++)
    for(int i=0;i<img->width;i++)
      {
        pValue = &((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels];
        pValue[0] = ubuff[idx] & 0xff;
        pValue[1] = (ubuff[idx] >> 8) & 0xff;
        pValue[2] = (ubuff[idx] >>16) & 0xff;
        idx++;
      }

  cvSaveImage(fileName.c_str(),img);
}

//===========================================================================
///	SaveUINTBuffer
///
///	Provides UINT buffer after the necessary vertical flip and BRG to RGB ordering
//===========================================================================
void SaveUINTBuffer(
                    UINT*	ubuff,				// RGB buffer
                    const int&			width,				// size
                    const int&			height,
                    const string&		fileName,			// filename to be given; even if whole path is given, it is still the filename that is used
                    const string&		saveLocation,
                    const string&		stradd,
                    sidType*                labels,
                    const string&               labelFileName)
{
  IplImage* img=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3); 
  uchar* pValue;
  int idx = 0;

  ofstream ofs(labelFileName.c_str());

  for(int j=0;j<img->height;j++)
    for(int i=0;i<img->width;i++)
      {
        pValue = &((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels];
        pValue[0] = ubuff[idx] & 0xff;
        pValue[1] = (ubuff[idx] >> 8) & 0xff;
        pValue[2] = (ubuff[idx] >>16) & 0xff;
        idx++;

        ofs.write((char*)&labels[idx],sizeof(int));
      }

  ofs.close();
  cvSaveImage(saveLocation.c_str(),img);
}

#endif

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs < 1 || nrhs > 3)
    mexErrMsgTxt("Incorrect input format.. find_superpixel(input_file_name,step_size[optional],verbosity[optional]) \n");

  char* input_file;
  int n = mxGetN(prhs[0]);
  input_file = (char *)mxCalloc(n+1,sizeof(char));
  mxGetString(prhs[0],input_file,n+1);
  bool verbosity = false;
  
  //const int STEP = 10;//STEP decides superpixel size (which will roughly be STEP^2 pixels)
  int STEP = 15;
  if(nrhs > 1)
    STEP = (int)(mxGetScalar(prhs[1]));

  double M = 10;
  if(nrhs > 2)
    M = (double)(mxGetScalar(prhs[2]));

  if(nrhs > 3)
    if ((int)(mxGetScalar(prhs[3]))>0)
      verbosity=true;


  int numlabels = 10;
  int width(0), height(0);

#ifdef WINDOWS
  // FIXME : DOES NOT WORK !!!
  vector<UINT> ubuff(0);
  BMPhandler bh;
  bh.GetUINTBuffer(string(intput_file), ubuff, width, height);

  vector<int> labels(0);//will contain unique labels for segments.
  const int K = 10;
  LKM lkm;
  lkm.DoSuperpixelSegmentation(ubuff, width, height, labels, numlabels, K, M);

  //DrawContoursAroundSegments(ubuff, labels, width, height, 0xff0000);//0xff0000 draws red contours
  DrawContoursAroundSegments(ubuff, labels, width, height, 0x0000ff);//0xff0000 draws red contours

  bh.SaveUINTBuffer(ubuff, width, height, string("myfile.bmp"), savepath /*, "_optional_suffiex.bmp"*/);

#else
  //IplImage* img = cvLoadImage(input_file,CV_LOAD_IMAGE_COLOR);
  IplImage* img = cvLoadImage(input_file);
  if(!img)
    {
      mexPrintf("Error while opening %s\n", input_file);
      return;
    }

  if(verbosity==true)
    mexPrintf("Image loaded %d\n",img->nChannels);

  width = img->width;
  height = img->height;
  int sz = height*width;


  double K = ((width*height)/(double)(STEP*STEP)+0.5);

  //printf("Image loaded %d\n",img->nChannels);


  UINT* ubuff = new UINT[sz];
  UINT pValue;
  char c;
  UINT r,g,b;
  int idx = 0;
  for(int j=0;j<img->height;j++)
    for(int i=0;i<img->width;i++)
      {
        if(img->nChannels == 3)
          {
            //pValue = ((UINT*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels];
            // image is assumed to have data in BGR order
            b = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels];
            g = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels+1];
            r = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels+2];
            pValue = b | (g << 8) | (r << 16);
          }
        else if(img->nChannels == 1)
          {
            c = ((uchar*)(img->imageData + img->widthStep*(j)))[(i)*img->nChannels];
            pValue = c | (c << 8) | (c << 16);
          }
        else
          {
            mexPrintf("Unknown number of channels %d\n", img->nChannels);
            return;
          }          
        ubuff[idx] = pValue;
        idx++;
      }

  sidType* labels = new sidType[sz]; //will contain unique labels for segments.
  LKM lkm;

  if(verbosity==true)
    mexPrintf("Generating superpixels. STEP=%d, M=%f\n", STEP, M);

  lkm.DoSuperpixelSegmentation(ubuff, width, height, labels, numlabels, STEP, M);

  // Prepare Output
  double *output_labels;
  plhs[0] = mxCreateDoubleMatrix(img->width,img->height,mxREAL);
  output_labels = mxGetPr(plhs[0]);

  for(int i=0;i<img->height;i++){
    for(int j=0;j<img->width;j++){
      output_labels[i*img->width+j]=labels[i*img->width+j];
    }
  }

  delete[] ubuff;
  delete[] labels;
  cvReleaseImage(&img);

#endif

  mxFree(input_file);
  //printf("Done!\n");
}
