
#ifndef UTILS_H
#define UTILS_H

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "itkImportImageFilter.h"
#include "itkConnectedComponentFunctorImageFilter.h"
#include "itkLabelObject.h"
#include "itkLabelMap.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkShapeLabelObject.h"
#include "itkLabelMapToBinaryImageFilter.h"

typedef unsigned char uchar;

void exportTIFCube(uchar* rawData,
                   const char* filename,
                   int cubeDepth,
                   int cubeHeight,
                   int cubeWidth);

void exportColorTIFCube(uchar* rawData,
                        const char* filename,
                        int cubeDepth,
                        int cubeHeight,
                        int cubeWidth);

void cubeFloat2Uchar(float* inputData, uchar*& outputData,
                     int nx, int ny, int nz);

template <typename TInputPixelType, typename LabelImageType>
typename LabelImageType::Pointer getLabelImage(TInputPixelType* inputData,
                                               long nx, long ny, long nz, ulong* nObjects = 0)
{
  const unsigned int Dimension = 3;
  typedef itk::Image< TInputPixelType, Dimension > InputImageType;
  //typedef unsigned int TLabelPixelType;
  //typedef itk::Image< TLabelPixelType, Dimension > LabelImageType;

  typedef itk::ConnectedComponentImageFilter< InputImageType, LabelImageType > CCFilterType;
  typename CCFilterType::Pointer ccFilter;

  ccFilter = CCFilterType::New();

  // set parameters
  ccFilter->SetFullyConnected(true);
  ccFilter->SetBackgroundValue(0);

  // import data to an itk image
  typedef itk::ImportImageFilter< TInputPixelType, Dimension > ImportFilterType;

  typename ImportFilterType::Pointer importFilter = ImportFilterType::New();

  typename ImportFilterType::SizeType size;
  size[0] = nx;
  size[1] = ny;
  size[2] = nz;

  typename ImportFilterType::IndexType start;
  start.Fill( 0 );

  typename ImportFilterType::RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );

  importFilter->SetRegion( region );
  //region.SetSize( size );

  typename InputImageType::PointType origin;
  origin.Fill( 0.0 );

  importFilter->SetOrigin( origin );

  typename ImportFilterType::SpacingType spacing;
  spacing.Fill( 1.0 );

  importFilter->SetSpacing( spacing );
  importFilter->SetImportPointer(inputData, 0, false);

  // run filter
  ccFilter->SetInput(importFilter->GetOutput());
  ccFilter->Update();

  if(nObjects) {
      *nObjects = ccFilter->GetObjectCount();
  }
  printf("Number of objects: %d\n", ccFilter->GetObjectCount());

  LabelImageType* labelImage = ccFilter->GetOutput();
  return labelImage;
}

#endif //UTILS_H
