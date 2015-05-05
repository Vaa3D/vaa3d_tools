
#include "utils.h"

#include <float.h>
#include <sstream>



using namespace std;

void exportTIFCube(uchar* rawData,
                   const char* filename,
                   int cubeDepth,
                   int cubeHeight,
                   int cubeWidth)
{
  // import data to an itk image
  const int dimension = 3;
  typedef uchar TInputPixelType;
  typedef itk::Image< TInputPixelType, dimension > InputImageType;
  typedef itk::Image< TInputPixelType, dimension > OutputImageType;
  typedef itk::ImportImageFilter< TInputPixelType, dimension > ImportFilterType;
  ImportFilterType::Pointer importFilter = ImportFilterType::New();

  ImportFilterType::SizeType size;
  size[0] = cubeWidth;
  size[1] = cubeHeight;
  size[2] = cubeDepth;

  ImportFilterType::IndexType start;
  start.Fill(0);

  ImportFilterType::RegionType region;
  region.SetIndex(start);
  region.SetSize(  size  );

  importFilter->SetRegion( region );

  InputImageType::PointType origin;
  origin.Fill(0.0);

  importFilter->SetOrigin( origin );

  ImportFilterType::SpacingType spacing;
  spacing.Fill(1.0);

  importFilter->SetSpacing( spacing );
  importFilter->SetImportPointer(rawData, 0, false);

  stringstream sout;
  sout << filename;
  int n = strlen(filename);
  if(n < 4 || strcmp(filename+n-4,".tif")!=0)
     sout << ".tif";
  //printf("[Utils] Writing output cube %s\n", sout.str().c_str());
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(sout.str().c_str());
  writer->SetInput(importFilter->GetOutput());
  writer->Update();
}


void exportColorTIFCube(uchar* rawData,
                        const char* filename,
                        int cubeDepth,
                        int cubeHeight,
                        int cubeWidth)
{
  // import data to an itk image
  const int dimension = 3;
  typedef uchar TInputPixelType;
  typedef itk::RGBPixel<TInputPixelType> RGBPixelType;
  typedef itk::Image< RGBPixelType, dimension > InputImageType;
  typedef itk::Image< RGBPixelType, dimension > OutputImageType;
  typedef itk::ImportImageFilter< RGBPixelType, dimension > ImportFilterType;
  ImportFilterType::Pointer importFilter = ImportFilterType::New();

  ImportFilterType::SizeType size;
  size[0] = cubeWidth;
  size[1] = cubeHeight;
  size[2] = cubeDepth;

  ImportFilterType::IndexType start;
  start.Fill(0);

  ImportFilterType::RegionType region;
  region.SetIndex(start);
  region.SetSize(  size  );

  importFilter->SetRegion( region );

  InputImageType::PointType origin;
  origin.Fill(0.0);

  importFilter->SetOrigin( origin );

  ImportFilterType::SpacingType spacing;
  spacing.Fill(1.0);

  importFilter->SetSpacing( spacing );
  RGBPixelType* localBuffer = reinterpret_cast<RGBPixelType* >(rawData);
  importFilter->SetImportPointer(localBuffer, 0, false);

  stringstream sout;
  sout << filename << ".tif";
  //printf("[Utils] Writing output cube %s\n", sout.str().c_str());
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(sout.str().c_str());
  writer->SetInput(importFilter->GetOutput());
  writer->Update();
}

void cubeFloat2Uchar(float* inputData, uchar*& outputData,
                     int nx, int ny, int nz)
{
  float minValue = FLT_MAX; //9999999999;
  float maxValue = -1;
  int cubeIdx = 0;
  for(int z=0; z < nz; z++)
    for(int y=0; y < ny; y++)
      for(int x=0; x < nx; x++)
        {
          if(maxValue < inputData[cubeIdx])
              maxValue = inputData[cubeIdx];
          if(minValue > inputData[cubeIdx])
              minValue = inputData[cubeIdx];

          cubeIdx++;
        }

  printf("[util] cubeFloat2Uchar : min %f, max %f\n", minValue, maxValue);

  // allocate memory
  //MESSAGE("allocating memory\n");
  outputData = new uchar[nx*ny*nz];

  // copy to output cube
  float scale = 255.0f/(maxValue-minValue);
  cubeIdx = 0;
  for(int z=0; z < nz; z++)
    for(int y=0; y < ny; y++)
      for(int x=0; x < nx; x++)
        {
          outputData[cubeIdx] = (inputData[cubeIdx]-minValue)*scale;
          cubeIdx++;
        }
}

