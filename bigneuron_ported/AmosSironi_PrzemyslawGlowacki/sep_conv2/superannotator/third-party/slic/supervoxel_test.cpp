#include <algorithm>
#include <cv.h>
#include <dirent.h>
#include <errno.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "LKM.h"
#include "utils.h"

using namespace std;

//------------------------------------------------------------------------------

int width = -1;
int height = -1;
int depth;
int bytes_per_pixel;

#define VOXEL_STEP 10

//------------------------------------------------------------------------------

static string getNameFromPath(string path){
  return path.substr(path.find_last_of("/\\")+1);
}

static string getExtension(string path){
  return path.substr(path.find_last_of(".")+1);
}

static int getFilesInDir(const char* dir, vector<string> &files,
                         const char* ext = 0, bool includePath = false)
{
  DIR *dp;
  struct dirent *dirp;
  if((dp  = opendir(dir)) == NULL) {
    cout << "Error(" << errno << ") opening " << dir << endl;
    return errno;
  }

  string sext;
  if(ext!=0)
     sext = (string)ext;
  while ((dirp = readdir(dp)) != NULL)
    {
      string s(dirp->d_name);
      if(includePath)
        s = string(dir) + s;
      if(ext==0 || getExtension(s)==sext)
        files.push_back(s);
    }

  sort(files.begin(),files.end());

  closedir(dp);
  return 0;
}

/*
 * Load volume data using images in a specified directory
 */
void load_from_dir(char* dir, unsigned int**& data)
{
  int d = 0;
  IplImage* img;
  IplImage* gray_img;

  // load files
  vector<string> files;
  getFilesInDir(dir, files);

  int nImgs = 0;
  for(vector<string>::iterator itFile = files.begin();
      itFile != files.end(); itFile++)
    {
      if((itFile->c_str()[0] != '.') && (getExtension(*itFile) == "png"))
        {
          nImgs++;
          if(width == -1 && height == -1)
            {
              string fullpath(dir);
              fullpath += *itFile;

              printf("fullpath %s\n",fullpath.c_str());
              IplImage* img_slice = cvLoadImage(fullpath.c_str());
              width = img_slice->width;
              height = img_slice->height;
            }
        }
    }

  printf("Allocating memory\n");
  // ask for enough memory for the texels and make sure we got it before proceeding
  depth = nImgs;
  bytes_per_pixel = 1;
  int n = width*height*bytes_per_pixel;
  data = new unsigned int* [depth];
  if (data == 0)
    {
      printf("Error while allocating memory for 3d volume\n");
      exit(-1);
    }

  printf("Loading images\n");
  for(vector<string>::iterator itFile = files.begin();
      itFile != files.end(); itFile++)
    {
      if((itFile->c_str()[0] == '.') || (getExtension(*itFile) != "png"))
        continue;

      string fullpath(dir);
      fullpath += *itFile;

      printf("Loading image number %d : %s\n",d,fullpath.c_str());

      IplImage* img_slice = cvLoadImage(fullpath.c_str());

      if(!img_slice)
        continue;

      data[d] = new unsigned int[n];

      if(img_slice->width != width || img_slice->height != height)
        {
          if(img_slice->nChannels != bytes_per_pixel)
            {
              gray_img = cvCreateImage(cvSize(img_slice->width,img_slice->height),IPL_DEPTH_8U,bytes_per_pixel);
              //printf("%d\n",img_slice->nChannels);
              //printf("%d\n",gray_img->nChannels);
              cvCvtColor(img_slice,gray_img,CV_RGB2GRAY);
              img = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,bytes_per_pixel);
              cvResize(gray_img,img);
              cvReleaseImage(&gray_img);
            }
          else
            {
              img = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,bytes_per_pixel);
              cvResize(img_slice,img);
            }

          memcpy(data[d],img->imageData,n);
          cvReleaseImage(&img);
          cvReleaseImage(&img_slice);
        }
      else
        {
          img = img_slice;

          memcpy(data[d],img->imageData,n);
          cvReleaseImage(&img);
        }
      d++;
    }
}

int main(int argc, char* argv[])
{
  char* input_dir;
  char* output_name;
  if(argc > 2)
    {
      input_dir = argv[1];
      output_name = argv[2];
    }
  else
    {
      printf("Usage: %s input_directory output_filename\n",argv[0]);
      return -1;
    }

  int numlabels;
  unsigned int** vol_data;

  load_from_dir(input_dir, vol_data);

  sidType** klabels;
  int vstep = VOXEL_STEP;
  if(vstep > depth)
    vstep = depth;

  LKM* lkm = new LKM;
  printf("DoSupervoxelSegmentation %d %d %d\n",width,height,depth);
  lkm->DoSupervoxelSegmentation(vol_data,
                                width,height,depth,
                                klabels,numlabels,
                                vstep);

  // TODO : Change function to return a 1d array directly. Ask RK ?
  printf("Exporting supervoxel cube containing %d labels\n",numlabels);

  unsigned int n = width*height;
  ofstream ofs(output_name);
  for(int d=0;d<depth;d++)
    {
      //memcpy(klabels_1d+(d*n),klabels[d],n*sizeof(int));
      ofs.write((const char*)klabels[d],n*sizeof(int));
      printf("%d: %d %d %d %d\n", d,klabels[d][0],klabels[d][100],klabels[d][200],klabels[d][300]);
      delete[] klabels[d];
    }
  delete[] klabels;
  delete lkm;
  ofs.close();

  printf("------------\n");
  int* labels = new int[n];
  ifstream ifs(output_name);
  for(int d=0;d<depth;d++)
    {
      ifs.read((char*)labels,n*sizeof(int));
      printf("%d %d %d %d\n", labels[0],labels[100],labels[200],labels[300]);
    }
  ifs.close();

  // NFO file used by VIVA
  stringstream snfo;
  snfo << output_name << ".nfo";
  ofstream nfo(snfo.str().c_str());
  nfo << "voxelDepth 0.1" << endl;
  nfo << "voxelHeight 0.1" << endl;
  nfo << "voxelWidth 0.1" << endl;
  nfo << "cubeDepth " << depth << endl;
  nfo << "cubeHeight " << height << endl;
  nfo << "cubeWidth " << width << endl;
  nfo << "x_offset 0" << endl;
  nfo << "y_offset 0" << endl;
  nfo << "z_offset 0" << endl;
  nfo << "cubeFile " << getNameFromPath(output_name) << endl;
  nfo << "type int" << endl;
    /*
  if(bytes_per_pixel == 1)
    nfo << "type uchar" << endl;
  else
    if(bytes_per_pixel == 2)
      nfo << "type short" << endl;
    else
      nfo << "type int" << endl;
    */

  nfo.close();

  printf("Done\n");
}
