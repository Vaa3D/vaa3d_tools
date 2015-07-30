#ifndef _UTIL_H
#define _UTIL_H

#include "itkImageRegionIterator.h"
#include "itkResampleImageFilter.h"

//#include <itkExtractImageFilter.h>
#include "itkRegionOfInterestImageFilter.h"


#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryImageToLabelMapFilter.h"

#include "itkLabelStatisticsImageFilter.h"

#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryBallStructuringElement.h"

#include "FL_interpolateCoordLinear3D.h"

//#include "FL_downSample3D.h"

//#include "itkMultiThreader.h"


class SwcFileContent {

private:
  class SwcRow {
  public:
    int ind;
    int type;
    double x, y, z, r;
    int parentInd;

    SwcRow(int ind, int type, double x, double y, double z, double r, int parentInd): ind(ind), type(type), x(x), y(y), z(z), r(r), parentInd(parentInd) {

    }
  };

public:
  std::vector< SwcRow > rows;
  std::map< int, int > swcInd2VectorIndMap;

  SwcFileContent(std::vector< std::string > swcFilePaths) {
//    if(swcFilePaths.size() != 1) {
//      std::cerr << "For now swcFilePaths has to be of size 1! (And here it is of size " << swcFilePaths.size() << ")" << std::endl;
//    }

    int globalMaxInd = 0;
    for(unsigned int swci = 0; swci < swcFilePaths.size(); swci++) {
      std::ifstream swcFile(swcFilePaths[swci].c_str());
      std::string line;
      int currentMaxInd = globalMaxInd;
      if(swcFile.is_open()) {
        while( getline (swcFile,line) ) {
          size_t firstNonBlankIndex = line.find_first_not_of(" \t\r\n");
          if(firstNonBlankIndex != std::string::npos) {
            if(line[firstNonBlankIndex] != '#') {
             // std::cout << "line: " << line << '\n';
              std::stringstream lineStream(line);
              int ind;
              int type;
              double x, y, z, r;
              int parentInd;
              lineStream >> ind >> type >> x >> y >> z >> r >> parentInd;
              rows.push_back(SwcRow(ind + globalMaxInd, type, x, y, z, r, parentInd + globalMaxInd));
              swcInd2VectorIndMap[ind + globalMaxInd] = rows.size() - 1;
              if(ind + globalMaxInd > currentMaxInd) {
                currentMaxInd = ind + globalMaxInd ;
              }
            } else {
            //  std::cout << "comment: " << line << '\n';
            }
          } else {
           // std::cout << "blank line: " << line << '\n';
          }
        }
        swcFile.close();
        globalMaxInd = currentMaxInd;
       // std::cout << "globalMaxInd: " << globalMaxInd << std::endl;
      }

      else std::cout << "Unable to open file" << std::endl;
    }
  }

};



template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer v3d2ItkImage(Image4DSimple *inimg,const long int *in_sz, unsigned int c=1);

template<typename ImageType>
Image4DSimple itk2v3dImage(typename ImageType::Pointer itk_image);

template<typename ImageType,typename T>
typename ImageType::Pointer rawData2ItkImage(T *data1d,const long int *in_sz);

template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer swc2ItkImage(char * swc_file,const long int *in_sz,bool transpose_y);

template<typename ITKImageType>
bool check_range(typename ITKImageType::IndexType pixelIndex,typename ITKImageType::SizeType size);

template<typename ITKImageType>
bool check_range(typename ITKImageType::RegionType region,typename ITKImageType::SizeType size);

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg,const long int * out_sz);

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg,float *scale_factor);

template<typename ITKImageType,typename T>
typename ITKImageType::Pointer downsample_image_v3d(Image4DSimple *inimg,const double * dfactor, V3DLONG * szout);

template<typename ITKImageType,typename T>
typename ITKImageType::Pointer upsample_image_v3d(Image4DSimple *inimg,const double * scale_factor, V3DLONG * out_sz);

template<typename ITKImageType,typename T>
typename ITKImageType::Pointer upsample_image_v3d_size(Image4DSimple *inimg,V3DLONG * szout);

template<typename ITKImageType,typename T>
typename ITKImageType::Pointer resize_image_v3d(Image4DSimple * pred_img_v3d,const double * scale_factor, V3DLONG * out_sz);


template<typename ITKImageType>
typename ITKImageType::Pointer cropItkImage(typename ITKImageType::Pointer &origImg, long int * crop_start_idxs, long int * in_sz_cropped);

template<typename ITKImageType>
typename ITKImageType::Pointer cropItkImageUniformBackground(typename ITKImageType::Pointer origImg,const long int * crop_start_idxs,const long int * in_sz_cropped,double uniform_thresh);


//definition here to avoid explicit instantiation of template function
template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer v3d2ItkImage(Image4DSimple *inimg,const long int *in_sz, unsigned int c=1){

    typename ITKImageType::Pointer I  =  ITKImageType::New();

    I =rawData2ItkImage<ITKImageType,ImageScalarType>((ImageScalarType *)inimg->getRawDataAtChannel(c),in_sz);

    //    ////rescale in [0 1]
     typedef itk::RescaleIntensityImageFilter< ITKImageType, ITKImageType > RescaleFilterType;
    typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(I);
    rescaleFilter->SetOutputMinimum(0.0);
    rescaleFilter->SetOutputMaximum(1.0);

    rescaleFilter->Update();
    return rescaleFilter->GetOutput();
}


template<typename ImageType>
Image4DSimple itk2v3dImage(typename ImageType::Pointer itk_image){

//  std::cout << "Converting to V3D" << std::endl << std::flush;

        typename ImageType::RegionType region = itk_image->GetLargestPossibleRegion();
        itk::ImageRegionIterator<ImageType> imageIterator(itk_image,region);
        typename ImageType::SizeType size = region.GetSize();


        long int *in_sz = new long int[3];
        in_sz[0] = size[0];
        in_sz[1] = size[1];
        in_sz[2] = size[2];


        const long int n_pixels = in_sz[0]*in_sz[1]*in_sz[2];

            Image4DSimple outimg_v3d;

            float * out_data_copy = new float [n_pixels];


          long int i_pix = 0;
          while(!imageIterator.IsAtEnd())
            {

              out_data_copy[i_pix] = (float)imageIterator.Get();

            ++imageIterator;
            ++i_pix;
            }

        outimg_v3d.setData((unsigned char *)(out_data_copy), in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);



        return outimg_v3d;
}



//definition here to avoid explicit instantiation of template function
template<typename ImageType,typename T>
typename ImageType::Pointer rawData2ItkImage(T *data1d,const long int *in_sz){
  //typename ImageType::Pointer rawData2ItkImage(typename ImageType::PixelType *data1d,const long int *in_sz){

  //typedef itk::Image<signed int, 3> ImageType;
  unsigned int SN = in_sz[0];
  unsigned int  SM = in_sz[1];
  unsigned int  SZ = in_sz[2];

  // typename ImageType::Pointer I  = ImageType::New();
  typename ImageType::SizeType size;
  size[0] = SN;
  size[1] = SM;
  size[2] = SZ;


  typename ImageType::RegionType region;
  typename  ImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;


  region.SetSize(size);
  region.SetIndex(start);

  typename ImageType::Pointer I = ImageType::New();
  I->SetRegions(region);
  I->Allocate();


  for(int iz = 0; iz < SZ; iz++)
  {
    int offsetk = iz*SM*SN;
    for(int iy = 0; iy < SM; iy++)
    {
      int offsetj = iy*SN;
      for(int ix = 0; ix < SN; ix++)
      {

        T PixelVaule =  data1d[offsetk + offsetj + ix];
        itk::Index<3> indexX;
        indexX[0] = ix;
        indexX[1] = iy;
        indexX[2] = iz;
        I->SetPixel(indexX, PixelVaule);
      }
    }

  }

  cout<<"allocated image to itk format."<<size <<endl;

  return I;

}



template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer swc2ItkImage(char * swc_file,const long int *in_sz, bool transpose_y = false){

  std::vector< std::string > swcFilePathsVector;
  swcFilePathsVector.push_back(std::string(swc_file));
  SwcFileContent swcFileContent(swcFilePathsVector);

  itk::Index<3> start; start.Fill(0);
  itk::Size<3> size;
  size[0] = in_sz[0]; size[1] = in_sz[1]; size[2] = in_sz[2];
  typename ITKImageType::RegionType region(start, size);

  typename ITKImageType::Pointer I  =  ITKImageType::New();
  I->SetRegions(region);
  I->Allocate();
  I->FillBuffer(0);
  //I->Update();

  typename ITKImageType::IndexType pixelIndex;


  for(std::vector< SwcFileContent::SwcRow >::iterator rowi = swcFileContent.rows.begin(); rowi != swcFileContent.rows.end(); rowi++) {
    typename ITKImageType::IndexType pixelIndex;

    if(rowi->parentInd != -1) {
      std::vector< SwcFileContent::SwcRow >::iterator parentRowi = swcFileContent.rows.begin() + swcFileContent.swcInd2VectorIndMap[rowi->parentInd];
      double vecX = (parentRowi->x - rowi->x);
      double vecY = (parentRowi->y - rowi->y);
      double vecZ = (parentRowi->z - rowi->z);

      double segmentLength = sqrt(
                (rowi->x - parentRowi->x) * (rowi->x - parentRowi->x)
              + (rowi->y - parentRowi->y) * (rowi->y - parentRowi->y)
              + (rowi->z - parentRowi->z) * (rowi->z - parentRowi->z)
            );
      for(double i = 0.0; i < 1.0; i += 1.0 / segmentLength) {
        pixelIndex[0] = round(rowi->x + vecX * i);
        if(transpose_y){
            pixelIndex[1] = in_sz[1] -1 -round(rowi->y + vecY * i);//v3d transpose y axis
        }else{
                pixelIndex[1] =round(rowi->y + vecY * i);
        }
        pixelIndex[2] = round(rowi->z + vecZ * i);

        if(!check_range<ITKImageType>(pixelIndex,size)){
            std::cout << "pixel out of range (Interp). Index: " << pixelIndex << "Size: " <<  size <<std::endl << std::flush;
        }

        I->SetPixel(pixelIndex, (1.0 - i) * rowi->r + i * parentRowi->r);
      }

      pixelIndex[0] = round(parentRowi->x);
      if(transpose_y){
        pixelIndex[1] = in_sz[1] -1 -round(parentRowi->y);
      }else{
           pixelIndex[1] = round(parentRowi->y);
      }
      pixelIndex[2] = round(parentRowi->z);

      if(!check_range<ITKImageType>(pixelIndex,size)){
          std::cout << "pixel out of range (End). Index: " << pixelIndex << "Size: " << size<< std::endl << std::flush;
      }
      I->SetPixel(pixelIndex, parentRowi->r);
    }


    pixelIndex[0] = round(rowi->x);
    if(transpose_y){
        pixelIndex[1] = in_sz[1] -1 -round(rowi->y);
    }else{
        pixelIndex[1] = round(rowi->y);
    }
    pixelIndex[2] = round(rowi->z);

    if(!check_range<ITKImageType>(pixelIndex,size)){
        std::cout << "pixel out of range (Root). Index: " << pixelIndex << "Size: " << size<< std::endl << std::flush;
    }
    I->SetPixel(pixelIndex, rowi->r);
  }

  return I;

}



template<typename ITKImageType>
bool check_range(typename ITKImageType::IndexType pixelIndex,typename ITKImageType::SizeType size){

    if(pixelIndex[0]>=size[0] || pixelIndex[1]>=size[1] || pixelIndex[2]>=size[2] || pixelIndex[0]<0 || pixelIndex[1]<0 || pixelIndex[2]<0){
        return false;
    }else{
        return true;
    }

}

template<typename ITKImageType>
bool check_range(typename ITKImageType::RegionType region,typename ITKImageType::SizeType size){


    typename ITKImageType::IndexType start_index, end_index;
    typename ITKImageType::RegionType::SizeType  size_region = region.GetSize();
    start_index = region.GetIndex();
    end_index[0] = start_index[0] + size_region[0]-1;
    end_index[1] = start_index[1] + size_region[1]-1;
    end_index[2] = start_index[2] + size_region[2]-1;

    return check_range<ITKImageType>(start_index,size) && check_range<ITKImageType>(end_index,size);


//    if(pixelIndex[0]>=size[0] || pixelIndex[1]>=size[1] || pixelIndex[2]>=size[2] || pixelIndex[0]<0 || pixelIndex[1]<0 || pixelIndex[2]<0){
//        return false;
//    }else{
//        return true;
//    }

}




template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg,const long int * out_sz){


    //itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1);

    typedef itk::Image< double, 3 > ImageTypeDouble;

    typedef itk::CastImageFilter< ITKImageType, ImageTypeDouble > CastFilterType;
    typename  CastFilterType::Pointer castFilter = CastFilterType::New();
      castFilter->SetInput(origImg);

      std::cout << "casting img "  << std::endl;


      castFilter->Update();

      std::cout << "getting out" << std::endl;


ImageTypeDouble::Pointer origImgDouble = castFilter->GetOutput();


   typename ImageTypeDouble::SizeType     inputSize     = origImgDouble->GetLargestPossibleRegion().GetSize();
      std::cout << "Input Size: " << inputSize << std::endl;

    typename  ImageTypeDouble::SpacingType  inputSpacing  = origImgDouble->GetSpacing();
      std::cout << "Input Spacing: " << inputSpacing << std::endl;

     typename ImageTypeDouble::SpacingType  outputSpacing;

      typename ImageTypeDouble::SizeType     outputSize;
      outputSize[0] = out_sz[0];    outputSize[1] = out_sz[1];    outputSize[2] = out_sz[2];

     // for( unsigned int dim = 0; dim < Dimension; dim++ )
     //   {
        outputSpacing[0] = static_cast< double >( inputSpacing[0] ) * static_cast< double >( inputSize[0] ) / static_cast< double >( outputSize[0] );
        outputSpacing[1] = static_cast< double >( inputSpacing[1] ) * static_cast< double >( inputSize[1] ) / static_cast< double >( outputSize[1] );
        outputSpacing[2] = static_cast< double >( inputSpacing[2] ) * static_cast< double >( inputSize[2] ) / static_cast< double >( outputSize[2] );

        //  }



      std::cout << "Output Size: " << outputSize << std::endl;
      std::cout << "Output Spacing: " << outputSpacing << std::endl;

      typedef double TransformPrecisionType;
      typedef itk::IdentityTransform< TransformPrecisionType, 3 > TransformType;
      typedef itk::ResampleImageFilter< ImageTypeDouble, ImageTypeDouble > FilterType;
      typename FilterType::Pointer filter = FilterType::New();
      filter->SetInput( origImgDouble );
      filter->SetSize( outputSize );
      filter->SetOutputSpacing( outputSpacing );
      filter->SetOutputOrigin( origImgDouble->GetOrigin() );
      filter->SetTransform( TransformType::New() );
//filter->SetNumberOfThreads(1);

      filter->Update();


      typedef itk::CastImageFilter<ImageTypeDouble, ITKImageType > CastFilterTypeBack;
      typename  CastFilterTypeBack::Pointer castFilterBack = CastFilterTypeBack::New();
        castFilterBack->SetInput(filter->GetOutput());

        std::cout << "casting img back"  << std::endl;


        castFilterBack->Update();

        std::cout << "getting out back" << std::endl;


      return castFilterBack->GetOutput() ;


//    //resampler type
//    typedef itk::ResampleImageFilter<ITKImageType, ITKImageType> ResampleImageFilterType;
//    typedef itk::IdentityTransform<double, 3> TransformType;


//    //get/set input/output size and spacing
//    typename ITKImageType::SizeType inputSize = origImg->GetLargestPossibleRegion().GetSize();
//    std::cout << "BB Input size: " << inputSize << std::endl;

//    typename ITKImageType::SizeType outputSize;
//    outputSize[0] = out_sz[0]; outputSize[1] = out_sz[1]; outputSize[2] = out_sz[2];


//    typename ITKImageType::SpacingType outputSpacing;
//      outputSpacing[0] = origImg->GetSpacing()[0] * (static_cast<double>(inputSize[0]) / static_cast<double>(outputSize[0]));
//      outputSpacing[1] = origImg->GetSpacing()[1] * (static_cast<double>(inputSize[1]) / static_cast<double>(outputSize[1]));
//       outputSpacing[2] = origImg->GetSpacing()[2] * (static_cast<double>(inputSize[2]) / static_cast<double>(outputSize[2]));

//       std::cout << "CC "  << std::endl << std::flush;


//      //resample (use defualt linear interpolation)
//      typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
//      resample->SetInput(origImg);
//      resample->SetSize(outputSize);
//      resample->SetOutputSpacing(outputSpacing);
//      TransformType::Pointer transform = TransformType::New();
//      transform->SetIdentity();
//      resample->SetTransform(transform);
//      typedef itk::LinearInterpolateImageFunction<
//                               ITKImageType, double > InterpolatorType;
//      typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
//      resample->SetInterpolator( interpolator );

//      std::cout << "CC1 "  << std::endl << std::flush;


//      resample->SetDefaultPixelValue( 0 ); // value for regions without source
//      resample->SetOutputOrigin( origImg->GetOrigin() );
//      resample->SetOutputDirection( origImg->GetDirection() );

//      std::cout << "CC2 "  << std::endl << std::flush;


//      resample->UpdateLargestPossibleRegion();
////resample->Update();

//      std::cout << "DD "  << std::endl << std::flush;


//      typename ITKImageType::Pointer output = resample->GetOutput();

//      std::cout << "EE Output size: " << output->GetLargestPossibleRegion().GetSize() << std::endl;

//      return output;

}

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg, float *scale_factor){

    //get size image
    typename ITKImageType::RegionType region = origImg->GetLargestPossibleRegion();
    typename  ITKImageType::SizeType size = region.GetSize();

     std::cout << "AA in size: " << size << std::endl;


      long int * out_sz = new long int[3];
      out_sz[0] = (long int)((float)size[0])*scale_factor[0];
      out_sz[1] = (long int)((float)size[1])*scale_factor[1];
      out_sz[2] = (long int)((float)size[2])*scale_factor[2];

     std::cout << "AA out size: " << out_sz[0] <<  " "<< out_sz[1] <<" " << out_sz[2] << std::endl;

      typename ITKImageType::Pointer output =  resize_image_itk<ITKImageType>(origImg,out_sz);
      return output;

}



template<typename ITKImageType,typename T>
typename ITKImageType::Pointer downsample_image_v3d(Image4DSimple *inimg,const double * dfactor, V3DLONG * szout){


   // typename ITKImageType::Pointer output = ITKImageType::New();
        //Image4DSimple * outImg;

        V3DLONG *szin = new V3DLONG[3];
        szin[0] = inimg->getXDim();
        szin[1] = inimg->getYDim();
        szin[2] = inimg->getZDim();

        for (V3DLONG i=0; i<3; i++)
        {
            szout[i] = (V3DLONG)(floor(double(szin[i]) / double(dfactor[i])));
        }



        T *indata = (T *)inimg->getRawDataAtChannel(0);
        T *outdata = new T [szout[0]*szout[1]*szout[2]];

        long szout_01 = szout[0]*szout[1];
        long szin_01 = szin[0]*szin[1];

        unsigned char tag = 0; //average for downsampling
        if (tag == 0)
        {
            for (V3DLONG k=0;k<szout[2];k++)
            {
                long tt1 = k*szout_01;

                V3DLONG k2low=V3DLONG(floor(k*dfactor[2])), k2high=V3DLONG(floor((k+1)*dfactor[2]-1));
                if (k2high>szin[2]) k2high = szin[2];
                V3DLONG kw = k2high - k2low + 1;

                for (V3DLONG j=0;j<szout[1];j++)
                {
                    long tt2 = j*szout[0];

                    V3DLONG j2low=V3DLONG(floor(j*dfactor[1])), j2high=V3DLONG(floor((j+1)*dfactor[1]-1));
                    if (j2high>szin[1]) j2high = szin[1];
                    V3DLONG jw = j2high - j2low + 1;

                    for (V3DLONG i=0;i<szout[0];i++)
                    {
                        long idx_out = tt1 + tt2 + i;

                        V3DLONG i2low=V3DLONG(floor(i*dfactor[0])), i2high=V3DLONG(floor((i+1)*dfactor[0]-1));
                        if (i2high>szin[0]) i2high = szin[0];
                        V3DLONG iw = i2high - i2low + 1;

                        double cubevolume = double(kw) * jw * iw;
                    //    cout<<cubevolume <<" ";

    //printf("%ld,%ld, %ld\n", i,j,k);
                        double s=0.0;
                        for (V3DLONG k1=k2low;k1<=k2high;k1++)
                        {
                            long tmp1 = k1*szin_01;

                            for (V3DLONG j1=j2low;j1<=j2high;j1++)
                            {
                                long tmp2 = j1*szin[0];

                                for (V3DLONG i1=i2low;i1<=i2high;i1++)
                                {
                                    long idx_in = tmp1+ tmp2 + i1;

                                   // if(idx_in < n_pix)
                                        s += (T)indata[idx_in];
                                   // else
                                   //     printf("OUT OF RANGE: %d, %d, %d, %d, %d, %d,%d,%d\n", k, j, i, k1, j1, i1,idx_in,n_pix);
                               //     if (indata[idx_in]>0)
                               //         printf("%d, %d, %d, %d, %d, %d, %d\n", k, j, i, k1, j1, i1, indata[idx_in]);
                                }
                            }
                        }

                        outdata[idx_out] = (T)(s/cubevolume);
                      //  if (outdata[idx_out]>0)
                      //      printf("*******%f\n", s/cubevolume);
                    }
                }
            }
        }
//        else
//        {
//            for (V3DLONG k=0;k<szout[2]; k++)
//            {

//                long tt1 = k*szout_01;

//                V3DLONG k2=V3DLONG(floor(k*dfactor[2])); //20110128

//    //			V3DLONG k2low=V3DLONG(floor(k*dfactor[2])), k2high=V3DLONG(floor((k+1)*dfactor[2]-1));
//    //			if (k2high>szin[2]) k2high = szin[2];
//    //			V3DLONG kw = k2high - k2low + 1;

//                for (V3DLONG j=0;j<szout[1];j++)
//                {

//                    long tt2 = j*szout[0];

//                    V3DLONG j2=V3DLONG(floor(j*dfactor[1])); //20110128

//    //				V3DLONG j2low=V3DLONG(floor(j*dfactor[1])), j2high=V3DLONG(floor((j+1)*dfactor[1]-1));
//    //				if (j2high>szin[1]) j2high = szin[1];
//    //				V3DLONG jw = j2high - j2low + 1;

//                    for (V3DLONG i=0;i<szout[0];i++)
//                    {

//                        long idx_out = tt1 + tt2 + i;

//                        V3DLONG i2=V3DLONG(floor(i*dfactor[0]));

//                        long idx_in = k2*szin_01 + j2*szin[0] + i2;

//    //					V3DLONG i2low=V3DLONG(floor(i*dfactor[0])), i2high=V3DLONG(floor((i+1)*dfactor[0]-1));
//    //					if (i2high>szin[0]) i2high = szin[0];
//    //					V3DLONG iw = i2high - i2low + 1;

//    //					outdata[k][j][i] = indata[int((k2low+k2high)/2)][int((j2low+j2high)/2)][int((i2low+i2high)/2)];

//                        outdata[idx_out] = indata[idx_in];
//                    }
//                }
//            }
//        }

         typename ITKImageType::Pointer outputItkImage = rawData2ItkImage<ITKImageType,T>(outdata,szout);

        return outputItkImage;


}


template<typename ITKImageType,typename T>
typename ITKImageType::Pointer upsample_image_v3d(Image4DSimple *inimg,const double * dfactor, V3DLONG * szout){



     T *indata = (T *)inimg->getRawDataAtChannel(0);

    V3DLONG *szin = new V3DLONG[3];
    szin[0] = inimg->getXDim();
    szin[1] = inimg->getYDim();
    szin[2] = inimg->getZDim();


    szout[0] = (V3DLONG)(ceil(dfactor[0]*((double)szin[0])));
    szout[1] = (V3DLONG)(ceil(dfactor[1]*((double)szin[1])));
    szout[2] = (V3DLONG)(ceil(dfactor[2]*((double)szin[2])));


    V3DLONG totallen = szout[0] * szout[1] * szout[2];
    Coord3D * c = new Coord3D [totallen];
    T * v = new T [totallen];

    T *outdata = new T [totallen];


    if (!c || !v)
    {
        fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
        if (c) {delete []c; c=0;}
        if (v) {delete []v; v=0;}
        return false;
    }

    //computation
    V3DLONG i,j,k, ind;
    ind=0;

    for (k=0;k<szout[2];k++)
    {
        for (j=0;j<szout[1];j++)
        {
            for (i=0;i<szout[0];i++)
            {
                c[ind].x = double(i)/dfactor[0];
//#ifndef POSITIVE_Y_COORDINATE
//				c[ind].y = double(szout[1]-1-j)/dfactor[1];
//#else
                c[ind].y = double(j)/dfactor[1];
//#endif
                c[ind].z = double(k)/dfactor[2];
                v[ind] = -1; //set as a special value
                ind++;
            }
        }
    }

    interpolate_coord_linear(v, c, totallen, indata, szin[0], szin[1], szin[2], 0, szin[0]-1, 0, szin[1]-1, 0, szin[2]-1);

    ind=0;


    for (i=0; i<totallen; i++)
        outdata[i] = (T)(v[i]);

    // free temporary memory
    if (c) {delete []c; c=0;}
    if (v) {delete []v; v=0;}
   // return true;


    typename ITKImageType::Pointer outputItkImage = rawData2ItkImage<ITKImageType,T>(outdata,szout);

   return outputItkImage;


}



template<typename ITKImageType,typename T>
typename ITKImageType::Pointer upsample_image_v3d_size(Image4DSimple *inimg,V3DLONG * szout){


    T *indata = (T *)inimg->getRawDataAtChannel(0);


    V3DLONG *szin = new V3DLONG[3];
    szin[0] = inimg->getXDim();
    szin[1] = inimg->getYDim();
    szin[2] = inimg->getZDim();

    double *dfactor = new double[3];

    dfactor[0] = ((double)(szout[0])/(double)(szin[0]));
    dfactor[1] = ((double)(szout[1])/(double)(szin[1]));
    dfactor[2] = ((double)(szout[2])/(double)(szin[2]));



    //szout[0] = (V3DLONG)(ceil(dfactor[0]*szin[0]));
    //szout[1] = (V3DLONG)(ceil(dfactor[1]*szin[1]));
    //szout[2] = (V3DLONG)(ceil(dfactor[2]*szin[2]));


    V3DLONG totallen = szout[0] * szout[1] * szout[2];
    Coord3D * c = new Coord3D [totallen];
    T * v = new T [totallen];

    T *outdata = new T [totallen];


    if (!c || !v)
    {
        fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
        if (c) {delete []c; c=0;}
        if (v) {delete []v; v=0;}
        return false;
    }

    //computation
    V3DLONG i,j,k, ind;
    ind=0;

    for (k=0;k<szout[2];k++)
    {
        for (j=0;j<szout[1];j++)
        {
            for (i=0;i<szout[0];i++)
            {
                c[ind].x = double(i)/dfactor[0];
//#ifndef POSITIVE_Y_COORDINATE
//				c[ind].y = double(szout[1]-1-j)/dfactor[1];
//#else
                c[ind].y = double(j)/dfactor[1];
//#endif
                c[ind].z = double(k)/dfactor[2];
                v[ind] = -1; //set as a special value
                ind++;
            }
        }
    }

    std::cout << "interpolating..." << std::endl << std::flush;

    interpolate_coord_linear(v, c, totallen, indata, szin[0], szin[1], szin[2], 0, szin[0]-1, 0, szin[1]-1, 0, szin[2]-1);
    std::cout << "interpolating..Done.." << std::endl << std::flush;


    ind=0;


    for (i=0; i<totallen; i++)
        outdata[i] = (T)(v[i]);

    // free temporary memory
    if (c) {delete []c; c=0;}
    if (v) {delete []v; v=0;}
   // return true;


    typename ITKImageType::Pointer outputItkImage = rawData2ItkImage<ITKImageType,T>(outdata,szout);

   return outputItkImage;





//     V3DLONG *szout_1 = new V3DLONG[3];

//    typename ITKImageType::Pointer outputItkImage = upsample_image_v3d<ITKImageType,T>(inimg,dfactor,szout_1);

//    if( szout_1[0]!= szout[0] || szout_1[1]!= szout[1] || szout_1[2]!=szout[2]){

//        std::cout << "Could not resize image to desired size: " << std::endl;
//         std::cout << "desired size: " <<szout[0] <<", "<<szout[1] <<", "<<szout[2] << std::endl;
//          std::cout << "output size: " <<szout_1[0] <<", "<<szout_1[1] <<", "<<szout_1[2] << std::endl;

//    }

//    return outputItkImage;

}

//template<typename ITKImageType>
//typename ITKImageType::Pointer downsample_image_v3d(Image4DSimple *inimg,const double * scale_factor, V3DLONG * out_sz){


//    Image4DSimple * outImg;

//    V3DLONG *szin = new V3DLONG[3];
//    szin[0] = inimg->getXDim();
//    szin[1] = inimg->getYDim();
//    szin[2] = inimg->getZDim();


//    float *outdata;

//  //  template <class T> bool downsample3dvol(T ***outdata, T *** indata, V3DLONG *szin, double *dfactor, unsigned char tag)
//   // bool downsampled  = downsample3dvol<float>(outdata, inimg->getRawDataAtChannel(0),szin,scale_factor, 0);
//    unsigned char tag = 0;
//    //template <class T> bool downsample3dvol(T *&outdata, T *indata, V3DLONG *szout, V3DLONG *szin, double *dfactor, unsigned char tag)
//    bool downsampled  = downsample3dvol<float>(outdata, (float *)inimg->getRawDataAtChannel(1), out_sz, szin, scale_factor, tag);

//    if(downsampled){
//        outImg->setData((unsigned char *)(outdata), out_sz[0], out_sz[1], out_sz[2], 1, V3D_FLOAT32);
//    }else{
//        std::cout << "could not downsample image !" << std::endl;

//    }

//    return v3d2ItkImage<ITKImageType>(outImg,out_sz);

//}


template<typename ITKImageType>
typename ITKImageType::Pointer cropItkImageUniformBackground(typename ITKImageType::Pointer &origImg, long int * crop_start_idxs, long int * in_sz_cropped,double uniform_thresh){


    typename ITKImageType::SizeType orig_img_size = origImg->GetLargestPossibleRegion().GetSize();
    typename ITKImageType::IndexType orig_img_origin = origImg->GetLargestPossibleRegion().GetIndex();

    //threshold image
    typedef unsigned char Uint8PixelType;
    typedef itk::Image< Uint8PixelType, 3 > Uint8ImageType;
    typedef itk::BinaryThresholdImageFilter<
                 ITKImageType, Uint8ImageType >  ThresholdFilterType;
    typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
    thresholdFilter->SetInput( origImg );

    const Uint8PixelType outsideValue = 255;
    const Uint8PixelType insideValue  = 0;
    thresholdFilter->SetOutsideValue( outsideValue );
    thresholdFilter->SetInsideValue(  insideValue  );
    const float lowerThreshold = 0.0;
    const float upperThreshold = 0.09;
    thresholdFilter->SetLowerThreshold( lowerThreshold );
    thresholdFilter->SetUpperThreshold( upperThreshold );
    //thresholdFilter->Update();

    // close thresholded image
        typedef itk::BinaryBallStructuringElement<Uint8ImageType::PixelType, Uint8ImageType::ImageDimension>
                  StructuringElementType;
      StructuringElementType structuringElement;
      structuringElement.SetRadius(10.0);
      structuringElement.CreateStructuringElement();
      typedef itk::BinaryMorphologicalClosingImageFilter <Uint8ImageType, Uint8ImageType, StructuringElementType>
              BinaryMorphologicalClosingImageFilterType;
      typename BinaryMorphologicalClosingImageFilterType::Pointer closingFilter
              = BinaryMorphologicalClosingImageFilterType::New();
      closingFilter->SetInput(thresholdFilter->GetOutput());
      closingFilter->SetKernel(structuringElement);
    //  closingFilter->Update();


    //remove small connected components
    //BinaryImageToLabelMapFilter
//relabelcomponentImageFilter
    typedef itk::BinaryImageToLabelMapFilter<Uint8ImageType> BinaryImageToLabelMapFilterType;
    typename  BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = BinaryImageToLabelMapFilterType::New();
    //  binaryImageToLabelMapFilter->SetInput(thresholdFilter->GetOutput());
       binaryImageToLabelMapFilter->SetInput(closingFilter->GetOutput());

      binaryImageToLabelMapFilter->Update();

      // The output of this filter is an itk::LabelMap, which contains itk::LabelObject's
   //     std::cout << "There are " << binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << " objects." << std::endl;

        // Loop over each region
//        for(unsigned int i = 0; i < binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
//          {
//          // Get the ith region
//           BinaryImageToLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToLabelMapFilter->GetOutput()->GetNthLabelObject(i);

//          // Output the pixels composing the region
//          //for(unsigned int pixelId = 0; pixelId < labelObject->Size(); pixelId++)
//          //  {
//          //  std::cout << "Object " << i << " contains pixel " << labelObject->GetIndex(pixelId) << std::endl;
//           // }

//       //     std::cout << "Object " << i << " contains " <<  labelObject->Size() << "pixels "<< std::endl;
//          }

       // return binaryImageToLabelMapFilter->GetOutput();
       // binaryImageToLabelMapFilter->Update();



    //get bounding box of connected components
//
    //    std::cout << "updating  labelMapToLabelImageFilter"<< std::endl << std::flush;

        typedef itk::LabelMapToLabelImageFilter<BinaryImageToLabelMapFilterType::OutputImageType, Uint8ImageType> LabelMapToLabelImageFilterType;
        typename  LabelMapToLabelImageFilterType::Pointer labelMapToLabelImageFilter = LabelMapToLabelImageFilterType::New();
         labelMapToLabelImageFilter->SetInput(binaryImageToLabelMapFilter->GetOutput());
         labelMapToLabelImageFilter->Update();

   //      std::cout << "updating  LabelStatisticsImageFilter"<< std::endl<< std::flush;

         typedef itk::LabelStatisticsImageFilter< Uint8ImageType, Uint8ImageType > LabelStatisticsImageFilterType;
        typename LabelStatisticsImageFilterType::Pointer labelStatisticsImageFilter = LabelStatisticsImageFilterType::New();
         labelStatisticsImageFilter->SetLabelInput( labelMapToLabelImageFilter->GetOutput() );
         labelStatisticsImageFilter->SetInput(thresholdFilter->GetOutput());
         labelStatisticsImageFilter->Update();
       //  std::cout << "done"<< std::endl<< std::flush;


     //    std::cout << "Number of labels: " << labelStatisticsImageFilter->GetNumberOfLabels() << std::endl;
     //      std::cout << std::endl;

           typedef LabelStatisticsImageFilterType::ValidLabelValuesContainerType ValidLabelValuesType;
           typedef LabelStatisticsImageFilterType::LabelPixelType                LabelPixelType;

           Uint8ImageType::RegionType region;
           Uint8ImageType::SizeType in_sz_cropped_temp;
           in_sz_cropped_temp = orig_img_size;
           Uint8ImageType::IndexType crop_start_idxs_temp ;
            crop_start_idxs_temp.Fill(0);
            Uint8ImageType::IndexType crop_end_idxs_temp ;
            crop_end_idxs_temp[0] = in_sz_cropped_temp[0]-1;  crop_end_idxs_temp[1] = in_sz_cropped_temp[1]-1;  crop_end_idxs_temp[2] = in_sz_cropped_temp[2]-1;


      //      std::cout << "init region idx: " << crop_start_idxs_temp <<", region size: " << in_sz_cropped_temp<<"region end: "<< crop_end_idxs_temp <<std::endl;

           bool found_region = false;

           for(ValidLabelValuesType::const_iterator vIt=labelStatisticsImageFilter->GetValidLabelValues().begin()+1;//first label is always background?
               vIt != labelStatisticsImageFilter->GetValidLabelValues().end();
               ++vIt)
             {
             if ( labelStatisticsImageFilter->HasLabel(*vIt) )
               {

                 LabelPixelType labelValue = *vIt;


//                  std::cout << "count: " << labelStatisticsImageFilter->GetCount( labelValue ) << std::endl;
//                   std::cout << "max: " << labelStatisticsImageFilter->GetMaximum( labelValue ) << std::endl;
//                   std::cout << "--------------- "  << std::endl;


                 if(    ( labelStatisticsImageFilter->GetCount( labelValue )   > 300  ) // large enough connected component
                     && ( labelStatisticsImageFilter->GetMaximum( labelValue ) > 0      )    ){ // no background

//                     std::cout << "************** "  << std::endl;
//                     std::cout << "ok count: " << labelStatisticsImageFilter->GetCount( labelValue ) << std::endl;
//                      std::cout << "ok max: " << labelStatisticsImageFilter->GetMaximum( labelValue ) << std::endl;
//                      std::cout << "************** "  << std::endl;

                     region = labelStatisticsImageFilter->GetRegion( labelValue );
                     if(!found_region){ // first region found

                            in_sz_cropped_temp =region.GetSize();
                            crop_start_idxs_temp = region.GetIndex();
                            crop_end_idxs_temp[0]  = crop_start_idxs_temp[0] +in_sz_cropped_temp[0] -1;
                            crop_end_idxs_temp[1]  = crop_start_idxs_temp[1] +in_sz_cropped_temp[1] -1;
                            crop_end_idxs_temp[2]  = crop_start_idxs_temp[2] +in_sz_cropped_temp[2] -1;
                             found_region = true;

//                           std::cout << "found first region, index:  " << crop_start_idxs_temp <<", region size: " << in_sz_cropped_temp<<"region end: " << crop_end_idxs_temp<< std::endl;


                     }else{


                         Uint8ImageType::SizeType in_sz_cropped_temp_temp = region.GetSize();
                         Uint8ImageType::IndexType crop_start_idxs_temp_temp = region.GetIndex();
                        Uint8ImageType::IndexType crop_end_idxs_temp_temp;
                        crop_end_idxs_temp_temp[0]  = crop_start_idxs_temp_temp[0] +in_sz_cropped_temp_temp[0] -1;
                        crop_end_idxs_temp_temp[1]  = crop_start_idxs_temp_temp[1] +in_sz_cropped_temp_temp[1] -1;
                        crop_end_idxs_temp_temp[2]  = crop_start_idxs_temp_temp[2] +in_sz_cropped_temp_temp[2] -1;


                         crop_start_idxs_temp[0] = std::min(crop_start_idxs_temp[0],crop_start_idxs_temp_temp[0]);
                         crop_start_idxs_temp[1] = std::min(crop_start_idxs_temp[1],crop_start_idxs_temp_temp[1]);
                         crop_start_idxs_temp[2] = std::min(crop_start_idxs_temp[2],crop_start_idxs_temp_temp[2]);

                         crop_end_idxs_temp[0] = std::max(crop_end_idxs_temp[0],crop_end_idxs_temp_temp[0]);
                         crop_end_idxs_temp[1] = std::max(crop_end_idxs_temp[1],crop_end_idxs_temp_temp[1]);
                         crop_end_idxs_temp[2] = std::max(crop_end_idxs_temp[2],crop_end_idxs_temp_temp[2]);

                         in_sz_cropped_temp[0] = crop_end_idxs_temp[0] - crop_start_idxs_temp[0] +1;
                         in_sz_cropped_temp[1] = crop_end_idxs_temp[1] - crop_start_idxs_temp[1] +1;
                         in_sz_cropped_temp[2] = crop_end_idxs_temp[2] - crop_start_idxs_temp[2] +1;


//                        std::cout << "found other region, index:  " << crop_start_idxs_temp <<", region size: " << in_sz_cropped_temp<<"region end: " << crop_end_idxs_temp<< std::endl;


                     }

                 }


              // std::cout << "min: " << labelStatisticsImageFilter->GetMinimum( labelValue ) << std::endl;
              // std::cout << "max: " << labelStatisticsImageFilter->GetMaximum( labelValue ) << std::endl;
              // std::cout << "median: " << labelStatisticsImageFilter->GetMedian( labelValue ) << std::endl;
              // std::cout << "mean: " << labelStatisticsImageFilter->GetMean( labelValue ) << std::endl;
              // std::cout << "sigma: " << labelStatisticsImageFilter->GetSigma( labelValue ) << std::endl;
              // std::cout << "variance: " << labelStatisticsImageFilter->GetVariance( labelValue ) << std::endl;
              // std::cout << "sum: " << labelStatisticsImageFilter->GetSum( labelValue ) << std::endl;
              // std::cout << "count: " << labelStatisticsImageFilter->GetCount( labelValue ) << std::endl;
               //std::cout << "box: " << labelStatisticsImageFilter->GetBoundingBox( labelValue ) << std::endl; // can't output a box
              // std::cout << "region: " << labelStatisticsImageFilter->GetRegion( labelValue ) << std::endl;
              // std::cout << std::endl << std::endl;

               }
             }



        //   std::cout << "adding 10 voxels:  "<< std::endl << std::flush;


           //add 10 voxels 'safe zone'
           crop_start_idxs[0] = std::max(crop_start_idxs_temp[0]-10, orig_img_origin[0]);
           crop_start_idxs[1] = std::max(crop_start_idxs_temp[1]-10, orig_img_origin[1]);
           crop_start_idxs[2] = std::max(crop_start_idxs_temp[2]-10, orig_img_origin[2]);
           in_sz_cropped[0] = std::min(in_sz_cropped_temp[0]+2*10, orig_img_size[0] - crop_start_idxs[0]); // - start index
           in_sz_cropped[1] = std::min(in_sz_cropped_temp[1]+2*10, orig_img_size[1] - crop_start_idxs[1]);
           in_sz_cropped[2] = std::min(in_sz_cropped_temp[2]+2*10, orig_img_size[2] - crop_start_idxs[2]);
           in_sz_cropped[3] = 1;


           ////for debug
           typename ITKImageType::RegionType region_to_crop;
           region_to_crop.SetSize(0,in_sz_cropped[0]);
           region_to_crop.SetSize(1,in_sz_cropped[1]);
           region_to_crop.SetSize(2,in_sz_cropped[2]);
           region_to_crop.SetIndex(0,crop_start_idxs[0]);
           region_to_crop.SetIndex(1,crop_start_idxs[1]);
           region_to_crop.SetIndex(2,crop_start_idxs[2]);

         //  std::cout << "final region to crop" << region_to_crop<<std::endl;

           if(!check_range<ITKImageType>(region_to_crop,orig_img_size )  ) {
               std::cout << "region out of range : " << region_to_crop << "Size: " << origImg->GetLargestPossibleRegion().GetSize()<< std::endl << std::flush;
           }
           /////

//           std::cout << "regionto crop : " << region_to_crop << "Size img: " << origImg->GetLargestPossibleRegion().GetSize()<< std::endl << std::flush;
//           std::cout << "caliing crop fun:  "<< std::endl << std::flush;


           return cropItkImage<ITKImageType>(origImg,  crop_start_idxs,  in_sz_cropped);

//return origImg;


////    origImg->DisconnectPipeline();

//    //for debug use predifined region
//   // crop_start_idxs[0] = 10;  crop_start_idxs[1] = 0;  crop_start_idxs[2] = 2;
//  //  in_sz_cropped[0] = 100; in_sz_cropped[1] = 80;in_sz_cropped[2] = 50; in_sz_cropped[3] =1;


//     typename ITKImageType::IndexType desiredStart;
//   //desiredStart.Fill(3);
//    desiredStart[0] = crop_start_idxs[0];
//    desiredStart[1] =  crop_start_idxs[1];
//    desiredStart[2] =  crop_start_idxs[2];


//    typename ITKImageType::SizeType desiredSize;
//      desiredSize[0] = in_sz_cropped[0];
//      desiredSize[1] = in_sz_cropped[1];
//      desiredSize[2] = in_sz_cropped[2];

//////

//      typedef itk::RegionOfInterestImageFilter< ITKImageType, ITKImageType > CropFilterType;
//      typename  CropFilterType::Pointer crop_filter = CropFilterType::New();

//      typename ITKImageType::RegionType desiredRegion;
//       desiredRegion.SetSize(desiredSize);
//       desiredRegion.SetIndex(desiredStart);

//       crop_filter->SetRegionOfInterest(desiredRegion);
//       crop_filter->SetInput(origImg);


//////
//       crop_filter->Update();
//       std::cout << "cropped image up" << std::endl;


//      typename ITKImageType::Pointer output = ITKImageType::New();
//      output =  crop_filter->GetOutput();
//       std::cout << "cropped image got out" << std::endl;

////std::cout << "size out" << output->GetLargestPossibleRegion().GetSize()<< std::endl;

////       output->DisconnectPipeline();
//       output->Update();


//       return output;

}


template<typename ITKImageType>
typename ITKImageType::Pointer cropItkImage(typename ITKImageType::Pointer &origImg, long int * crop_start_idxs, long int * in_sz_cropped){



    typename ITKImageType::IndexType desiredStart;
  //desiredStart.Fill(3);
   desiredStart[0] = crop_start_idxs[0];
   desiredStart[1] =  crop_start_idxs[1];
   desiredStart[2] =  crop_start_idxs[2];


   typename ITKImageType::SizeType desiredSize;
     desiredSize[0] = in_sz_cropped[0];
     desiredSize[1] = in_sz_cropped[1];
     desiredSize[2] = in_sz_cropped[2];

////

     typedef itk::RegionOfInterestImageFilter< ITKImageType, ITKImageType > CropFilterType;
     typename  CropFilterType::Pointer crop_filter = CropFilterType::New();

     typename ITKImageType::RegionType desiredRegion;
      desiredRegion.SetSize(desiredSize);
      desiredRegion.SetIndex(desiredStart);

      crop_filter->SetRegionOfInterest(desiredRegion);
      crop_filter->SetInput(origImg);


////
  //    std::cout << "cropping image up" << std::endl;

      crop_filter->Update();
   //   std::cout << "cropped image up" << std::endl;


     typename ITKImageType::Pointer output = ITKImageType::New();
     output =  crop_filter->GetOutput();
   //   std::cout << "cropped image got out" << std::endl;

//std::cout << "size out" << output->GetLargestPossibleRegion().GetSize()<< std::endl;

//       output->DisconnectPipeline();
      output->Update();


      return output;




}



#endif
