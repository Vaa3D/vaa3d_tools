#ifndef GRAPHCUT_H_
#define GRAPHCUT_H_

// Include graph.h first to avoid compilation errors
// due to the re-definition of symbols
#include "kgraph.h"
#include "kgraph.cpp"
#include "maxflow.cpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <float.h>
#include "utils.h"

// ITK Header Files
#include "itkConnectedComponentFunctorImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkLabelObject.h"
#include "itkLabelMap.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkShapeLabelObject.h"
#include "itkLabelMapToBinaryImageFilter.h"


//-----------------------------------------------------------------------------

#define flag_verbose false
#define PRINT_MESSAGE(format, ...) if(flag_verbose) printf (format, ## __VA_ARGS__)

using namespace std;

typedef maxflow::Graph<float,float,float> GraphType;
typedef unsigned char uchar;

const unsigned int Dimension = 3;
typedef unsigned int TLabelPixelType;
typedef itk::Image< TLabelPixelType, Dimension > LabelImageType;

struct Point
{
    int x;
    int y;
    int z;

    Point() { x = 0; y = 0; z = 0; }

    Point(int _x, int _y, int _z) { x = _x; y = _y; z = _z; }
};

struct Cube
{
  ulong width;
  ulong height;
  ulong depth;
  ulong wh; //width*height
  unsigned char* data;
  bool deleteData;

  //unsigned char at(int i, int j, int k)
  int at(int i, int j, int k)
  {
    return (int)data[k*wh+j*width+i];
  }

  Cube() { deleteData = false; }

  Cube(bool _deleteData) { deleteData = _deleteData; }

  ~Cube()
  {
    if(deleteData && data) {
        delete[] data;
    }
  }
};

enum eUnaryWeights {
    UNARY_NONE = 0,
    UNARY_HISTOGRAMS,
    UNARY_SCORE
};

//-----------------------------------------------------------------------------

class GraphCut
{
 private:
  GraphType::node_id* m_node_ids;
  int ni,nj,nk;
  ulong nij;
  ulong subX, subY, subZ;
  Cube* subCube;
  //uchar* subCube;
  //ulong subWidth, subHeight, subDepth;

 public:

  GraphCut();
  ~GraphCut();

  void applyCut(LabelImageType* ptrLabelInput, Cube* inputCube, unsigned char* output_data, int ccId, unsigned char* scoreImage);

  unsigned long at(int i, int j, int k);

  void displayCounts();

  template <typename TInputPixelType>
    void extractSubCube(TInputPixelType* inputData,
                        TInputPixelType* weights,
                        LabelImageType::Pointer labelInput,
                        vector<Point>& sourcePoints,
                        vector<Point>& sinkPoints,
                        long nx, long ny, long nz);

  template <typename TInputPixelType>
    void extractSubCube(TInputPixelType* weights,
                      vector<Point>& sourcePoints,
                      vector<Point>& sinkPoints,
                      long nx, long ny, long nz,
                      int max_width, int max_height, int max_depth);

  void getCubeSize(ulong& cubeWidth,
                   ulong& cubeHeight,
                   ulong& cubeDepth);

  void getOutput(Cube* inputCube,uchar*& output_data);

  void getOutputGivenSeeds(Cube* inputCube,uchar*& output_data);
 
  void run_maxflow(Cube* cube,
                   vector<Point>& sourcePoints, vector<Point>& sinkPoints,
                   float sigma = 100.0f, int minDist = 3,
                   eUnaryWeights unaryType = UNARY_NONE, Cube* scores = 0);
 
  GraphType *m_graph;

  // true if the run_maxflow method is running
  bool running_maxflow;
};


template <typename TInputPixelType>
void GraphCut::extractSubCube(TInputPixelType* inputData,
                              TInputPixelType* weights,
                              LabelImageType::Pointer labelInput,
                              vector<Point>& sourcePoints,
                              vector<Point>& sinkPoints,
                              long nx, long ny, long nz)
{
  //LabelImageType::Pointer labelInput = getLabelImage<TInputPixelType,LabelImageType>(inputData,nx,ny,nz);

  typedef itk::ShapeLabelObject< long, Dimension >       LabelObjectType;
  //typedef itk::ShapeLabelObject< TInputPixelType, Dimension >       LabelObjectType;
  typedef itk::LabelMap< LabelObjectType >             LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter< LabelImageType, LabelMapType > LabelFilterType;

  typename LabelFilterType::Pointer labelFilterInput = LabelFilterType::New();
  labelFilterInput->SetBackgroundValue(0);
  labelFilterInput->SetInput(labelInput);
  labelFilterInput->Update();

  LabelMapType* labelMapInput = labelFilterInput->GetOutput();

  typename LabelImageType::IndexType p;
  p[0] = sourcePoints[0].x;
  p[1] = sourcePoints[0].y;
  p[2] = sourcePoints[0].z;

  for(int i=0; i < labelMapInput->GetNumberOfLabelObjects(); i++)
    {
      LabelObjectType * labelObject = labelMapInput->GetNthLabelObject(i);

      typename LabelObjectType::RegionType region = labelObject->GetRegion();
      typename LabelImageType::IndexType indexType = region.GetIndex();
      typename LabelImageType::SizeType sizeType = region.GetSize();

      subX = indexType[0];
      subY = indexType[1];
      subZ = indexType[2];
      printf("Object %d. start_position=(%ld,%ld,%ld) size=(%ld,%ld,%ld) end_position=(%ld,%ld,%ld)\n", i, subX, subY, subZ, sizeType[0], sizeType[1], sizeType[2], subX+sizeType[0], subY+sizeType[1], subZ+sizeType[2]);
    }

  int ccID = labelInput->GetPixel(p) - 1;
  printf("labelObject=%d\n", ccID);
  if(ccID < 0)
    return; // error : only accept unsigned 8 bits images
  LabelObjectType * labelObject = labelMapInput->GetNthLabelObject(ccID);

  typename LabelObjectType::RegionType region = labelObject->GetRegion();
  typename LabelImageType::IndexType indexType = region.GetIndex();
  typename LabelImageType::SizeType sizeType = region.GetSize();

  ulong sub_cubeSize = sizeType[0]*sizeType[1]*sizeType[2];
  subX = indexType[0];
  subY = indexType[1];
  subZ = indexType[2];
  printf("Setting sub-cube. start_position=(%ld,%ld,%ld) size=(%ld,%ld,%ld) end_position=(%ld,%ld,%ld)\n",
         subX, subY, subZ, sizeType[0], sizeType[1], sizeType[2], subX+sizeType[0], subY+sizeType[1], subZ+sizeType[2]);
  subCube = new Cube(true);
  subCube->data = new uchar[sub_cubeSize];
  subCube->width = sizeType[0];
  subCube->height = sizeType[1];
  subCube->depth = sizeType[2];
  subCube->wh = subCube->width*subCube->height;

  ulong sliceSize = nx*ny;
  ulong cubeSize = nx*ny*nz;
  ulong cubeIdx = 0;
  ulong inputIdx;
  for(int k = 0; k < subCube->depth; k++)
    for(int j = 0; j < subCube->height; j++)
      for(int i = 0; i < subCube->width; i++)
        {
          inputIdx = (k+subZ)*sliceSize + (j+subY)*nx + (i+subX);
          if(inputIdx>=cubeSize) {
              printf("inputIdx>=cubeSize (%d %d %d) (%d %d %d) (%d %d %d) (%d %d %d) %ld %ld\n",
                      i, j, k, subX, subY, subZ, nx, ny, nz, sizeType[0], sizeType[1], sizeType[2], inputIdx, cubeSize);
          }
          assert(inputIdx<cubeSize);
          subCube->data[cubeIdx] = weights[inputIdx];
          ++cubeIdx;
        }

  //exportTIFCube(subCube->data, "subCube", subCube->depth, subCube->height, subCube->width);
}

template <typename TInputPixelType>
void GraphCut::extractSubCube(TInputPixelType* weights,
                              vector<Point>& sourcePoints,
                              vector<Point>& sinkPoints,
                              long nx, long ny, long nz,
                              int max_width, int max_height, int max_depth)
{
  Point centerSource;
  centerSource.x = 0;
  centerSource.y = 0;
  centerSource.z = 0;
  for(vector<Point>::iterator itPoint=sourcePoints.begin();
      itPoint != sourcePoints.end();itPoint++) {
      centerSource.x += itPoint->x;
      centerSource.y += itPoint->y;
      centerSource.z += itPoint->z;
  }
  centerSource.x /= sourcePoints.size();
  centerSource.y /= sourcePoints.size();
  centerSource.z /= sourcePoints.size();

  Point centerSink;
  centerSink.x = 0;
  centerSink.y = 0;
  centerSink.z = 0;
  for(vector<Point>::iterator itPoint=sinkPoints.begin();
      itPoint != sinkPoints.end();itPoint++) {
      centerSink.x += itPoint->x;
      centerSink.y += itPoint->y;
      centerSink.z += itPoint->z;
  }
  centerSink.x /= sinkPoints.size();
  centerSink.y /= sinkPoints.size();
  centerSink.z /= sinkPoints.size();

  Point center;
  center.x = (centerSource.x + centerSink.x)/2;
  center.y = (centerSource.y + centerSink.y)/2;
  center.z = (centerSource.z + centerSink.z)/2;

  subX = max(0, center.x - max_width/2);
  subY = max(0, center.y - max_height/2);
  subZ = max(0, center.z - max_depth/2);

  printf("Setting sub-cube. position=(%ld,%ld,%ld)\n", subX, subY, subZ);
  if(subCube)
      delete[] subCube;
  subCube = new Cube(true);
  subCube->width = min((ulong)max_width, nx-subX);
  subCube->height = min((ulong)max_height, ny-subY);
  subCube->depth = min((ulong)max_depth, nz-subZ);
  subCube->wh = subCube->width*subCube->height;
  ulong sub_cubeSize = subCube->width*subCube->height*subCube->depth;
  subCube->data = new uchar[sub_cubeSize];

  ulong sliceSize = nx*ny;
  ulong cubeSize = nx*ny*nz;
  ulong cubeIdx = 0;
  ulong inputIdx;
  for(int k = 0; k < subCube->depth; k++)
    for(int j = 0; j < subCube->height; j++)
      for(int i = 0; i < subCube->width; i++)
        {
          inputIdx = (k+subZ)*sliceSize + (j+subY)*nx + (i+subX);
          if(inputIdx>=cubeSize) {
              printf("inputIdx>=cubeSize (%d %d %d) (%d %d %d) (%d %d %d) %ld %ld\n",
                      i, j, k, subX, subY, subZ, nx, ny, nz, inputIdx, cubeSize);
          }
          assert(inputIdx<cubeSize);
          subCube->data[cubeIdx] = weights[inputIdx];
          ++cubeIdx;
        }

  //exportTIFCube(subCube->data, "subCube", subCube->depth, subCube->height, subCube->width);
}

#endif //GRAPHCUT_H_
