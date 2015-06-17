#ifndef _UTIL_H
#define _UTIL_H

#include "itkImageRegionIterator.h"
#include "itkResampleImageFilter.h"

#include "../../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_upSample3D.h"
#include "../../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_downSample3D.h"


template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer v3d2ItkImage(Image4DSimple *inimg,const long int *in_sz, unsigned int c=1);


template<typename ImageType>
Image4DSimple itk2v3dImage(typename ImageType::Pointer itk_image);

template<typename ImageType,typename T>
typename ImageType::Pointer rawData2ItkImage(T *data1d,const long int *in_sz);


////TODO !
template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer swc2ItkImage(char * swc_file,const long int *in_sz);
////TODO !
template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer swc2ItkImage(char * swc_file,const long int *in_sz){

    typename ITKImageType::Pointer I  =  ITKImageType::New();
    return I;

}

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg,const long int * out_sz);

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg,float *scale_factor);

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_v3d(Image4DSimple *inimg,const double * scale_factor, long int * out_sz);

//template<typename ITKImageType>
//typename ITKImageType::Pointer resize_image_v3d(Image4DSimple *inimg,const long int * out_sz);



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


        //typename ImageType::RegionType region;
        typename ImageType::RegionType region = itk_image->GetLargestPossibleRegion();
//        typename  ImageType::IndexType start;
//         typename ImageType::SizeType size;
//        size = itk_image.GetSize();
//        start.Fill(0);
//        region.SetSize(size);
//        region.SetIndex(start);

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


              std::cout << " elem 1,2,3" <<out_data_copy[0] << "," << out_data_copy[1] << ","<<out_data_copy[2]<<std::endl;




        outimg_v3d.setData((unsigned char *)(out_data_copy), in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);



        return outimg_v3d;
}



//definition here to avoid explicit instantiation of template function
template<typename ImageType,typename T>
//void rawData2ItkImage(ImageType::PixelType *data1d){
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

 // cout<<"size    "<<size <<endl;


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
  //I->FillBuffer(0);
  //I->Update();


  //    typename ImageType::IndexType idx;
  //    idx.Fill(0);
  //    typename ImageType::RegionType region;
  //    region.SetSize( size );
  //    region.SetIndex( idx );

  //    I->SetRegions(region);
  //    I->Allocate();
  //    I->FillBuffer(0);

  //    I->Update();

  //cout<<"allocating image  " <<endl;


  //  ImageType::RegionType region;
  // / region.SetSize(regionSize);
  //  region.SetIndex(regionIndex);


  //    itk::ImageRegionIterator<ImageType> imageIterator(I,region);
  //    unsigned int idx_lin= 0;
  //    while(!imageIterator.IsAtEnd())
  //        {
  //            // Get the value of the current pixel
  //            //unsigned char val = imageIterator.Get();
  //            //std::cout << (int)val << std::endl;

  //            // Set the current pixel to white
  //            unsigned char PixelVaule =  data1d[idx_lin];
  //            if(PixelVaule == NULL){
  //                cout<< "NULL POINTER !: " <<idx_lin  << endl;
  //            }
  //            // PixelVaule = 0.0;
  //            imageIterator.Set(PixelVaule);

  //            ++imageIterator;
  //            ++idx_lin;
  //        }


  for(int iz = 0; iz < SZ; iz++)
  {
    int offsetk = iz*SM*SN;
    for(int iy = 0; iy < SM; iy++)
    {
      int offsetj = iy*SN;
      for(int ix = 0; ix < SN; ix++)
      {

        //  //                cout<< offsetk + offsetj + ix ;

        T PixelVaule =  data1d[offsetk + offsetj + ix];
        itk::Index<3> indexX;
        indexX[0] = ix;
        indexX[1] = iy;
        indexX[2] = iz;
        //cout<< ": " << indexX <<endl;
        I->SetPixel(indexX, PixelVaule);
      }
    }

  }


  cout<<"allocated image  "<<size <<endl;

  return I;

}

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg,const long int * out_sz){




//    ImagePixelType pixel_type = origImg->getDatatype();
    //resampler type
    typedef itk::ResampleImageFilter<ITKImageType, ITKImageType> ResampleImageFilterType;
    //typedef itk::IdentityTransform<typename ITKImageType::PixelType, 3> TransformType;
    typedef itk::IdentityTransform<double, 3> TransformType;


    //get/set input/output size and spacing
    typename ITKImageType::SizeType inputSize = origImg->GetLargestPossibleRegion().GetSize();
    std::cout << "Input size: " << inputSize << std::endl;

    typename ITKImageType::SizeType outputSize;
    outputSize[0] = out_sz[0]; outputSize[1] = out_sz[1]; outputSize[2] = out_sz[2];

 //    std::cout << "out size 2: "<< out_sz[0] <<  " "<< out_sz[1] <<" " << out_sz[2] << std::endl;

    typename ITKImageType::SpacingType outputSpacing;
      outputSpacing[0] = origImg->GetSpacing()[0] * (static_cast<double>(inputSize[0]) / static_cast<double>(outputSize[0]));
      outputSpacing[1] = origImg->GetSpacing()[1] * (static_cast<double>(inputSize[1]) / static_cast<double>(outputSize[1]));
       outputSpacing[2] = origImg->GetSpacing()[2] * (static_cast<double>(inputSize[2]) / static_cast<double>(outputSize[2]));

//       outputSpacing[0] = origImg->GetSpacing()[0] * ((double)(inputSize[0]) / (double)(outputSize[0]));
//       outputSpacing[1] = origImg->GetSpacing()[1] * ((double)(inputSize[1]) / (double)(outputSize[1]));
//        outputSpacing[2] = origImg->GetSpacing()[2] * ((double)(inputSize[2]) / (double)(outputSize[2]));

   //     std::cout << "in spacing : "<< origImg->GetSpacing()<< std::endl;

 //std::cout << "out spacing : "<< outputSpacing<< std::endl;

      //resample (use defualt linear interpolation)
      typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
      resample->SetInput(origImg);
      resample->SetSize(outputSize);
      resample->SetOutputSpacing(outputSpacing);
      TransformType::Pointer transform = TransformType::New();
      transform->SetIdentity();
      resample->SetTransform(transform);
      typedef itk::LinearInterpolateImageFunction<
                               ITKImageType, double > InterpolatorType;
      typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
      resample->SetInterpolator( interpolator );

      resample->SetDefaultPixelValue( 0 ); // value for regions without source
      resample->SetOutputOrigin( origImg->GetOrigin() );
      resample->SetOutputDirection( origImg->GetDirection() );

      resample->UpdateLargestPossibleRegion();
//resample->Update();

      typename ITKImageType::Pointer output = resample->GetOutput();


      std::cout << "Output size: " << output->GetLargestPossibleRegion().GetSize() << std::endl;

      return output;

}

template<typename ITKImageType>
typename ITKImageType::Pointer resize_image_itk(typename ITKImageType::Pointer origImg, float *scale_factor){

    //get size image
    typename ITKImageType::RegionType region = origImg->GetLargestPossibleRegion();
    typename  ITKImageType::SizeType size = region.GetSize();

     std::cout << "in size: " << size << std::endl;

     std::cout << "s1: " << ((float)size[0])*scale_factor[0] << std::endl;
     std::cout << "s2: " << ((float)size[1])*scale_factor[1] << std::endl;
     std::cout << "s3e: " << ((float)size[2])*scale_factor[2] << std::endl;


      long int * out_sz = new long int[3];
      out_sz[0] = (long int)((float)size[0])*scale_factor[0];
      out_sz[1] = (long int)((float)size[1])*scale_factor[1];
      out_sz[2] = (long int)((float)size[2])*scale_factor[2];

      std::cout << "out size: " << out_sz[0] <<  " "<< out_sz[1] <<" " << out_sz[2] << std::endl;
      //call over. funcrtion

      typename ITKImageType::Pointer output =  resize_image_itk<ITKImageType>(origImg,out_sz);
      return output;

}


template<typename ITKImageType,typename T>
typename ITKImageType::Pointer resize_image_v3d(Image4DSimple *inimg, const double *dfactor, long int * out_sz){


   //  std::cout << "starting resampling " << std::endl;

    //get input size
    V3DLONG in_sz[4];
    in_sz[0] = inimg->getXDim();
    in_sz[1] = inimg->getYDim();
    in_sz[2] = inimg->getZDim();
    in_sz[3] = inimg->getCDim();


   // std::cout << "in size: " << in_sz[0] << " " << in_sz[1] <<" "<<in_sz[2] << endl;

   // double dfactor[3];
   // dfactor[0] = scale_factor; dfactor[1] = scale_factor;  dfactor[2] =scale_factor;
  //  std::cout << "dfactor: " << dfactor[0] << " " << dfactor[1] <<" "<<dfactor[2] << endl;

  //  V3DLONG out_sz[4];

  //  V3DLONG pagesz = inimg->getTotalUnitNumberPerChannel();

   // ImagePixelType pixel_type = inimg->getDatatype();
    unsigned int c = 1;
    T* data1d = (T*)inimg->getRawDataAtChannel(c);

//    switch (pixel_type) //upsample3dvol can only handle unsigned char input
//    {
//         case V3D_UINT8: data1d = inimg->getRawDataAtChannel(c); break;
//         case V3D_UINT16: gaussian_filter((unsigned short int*)(inimg->getRawData()), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
//         case V3D_FLOAT32: gaussian_filter((float *)(inimg->getRawData()), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
//         default:
//              v3d_msg("Invalid datatype in resampling.", 0);
//              if (inimg) {delete inimg; inimg=0;}
//              return 0;
//    }




    //for (unsigned i=0; i< len; i++)
     //   data1d[i]= static_cast<unsigned char>(float_buff[i]);

    T * image_resampled = 0;
    if(dfactor[0]>1){//upsample. NB assume that dfactor>1 along all dimensions !
        std::cout << "starting upsampling " << std::endl;


        out_sz[0] = (V3DLONG)(ceil(dfactor[0]*in_sz[0]));
        out_sz[1] = (V3DLONG)(ceil(dfactor[1]*in_sz[1]));
        out_sz[2] = (V3DLONG)(ceil(dfactor[2]*in_sz[2]));
        out_sz[3] = 1;
      //  std::cout << "in size: " << in_sz[0] << " " << in_sz[1] <<" "<<in_sz[2] << endl;
   //     std::cout << "out size: " << out_sz[0] << " " << out_sz[1] <<" "<<out_sz[2] << endl;

        V3DLONG pagesz_resample = (V3DLONG)(out_sz[0]*out_sz[1]*out_sz[2]);
        try {image_resampled = new T [pagesz_resample];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}

         double dfactor_d[3];
        dfactor_d[0] = dfactor[0]; dfactor_d[1] = dfactor[1];  dfactor_d[2] = dfactor[2];
        upsample3dvol<T>((T *)image_resampled,(T *)data1d,out_sz,in_sz,dfactor_d);
    }else if(dfactor[0]<1){ //downsample NB assume that dfactor<1 along all dimensions !
        std::cout << "starting downsampling " << std::endl;


        out_sz[0] = (V3DLONG)(floor(double(in_sz[0]) * double(dfactor[0])));
        out_sz[1] = (V3DLONG)(floor(double(in_sz[1]) * double(dfactor[1])));
        out_sz[2] = (V3DLONG)(floor(double(in_sz[2]) * double(dfactor[2])));
        out_sz[3] = 1;

        std::cout << "out size: " << out_sz[0] << " " << out_sz[1] <<" "<<out_sz[2] << endl;

        V3DLONG pagesz_resample = (V3DLONG)(out_sz[0]*out_sz[1]*out_sz[2]);

        try {image_resampled = new T [pagesz_resample];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}

        double dfactor_d[3];
        dfactor_d[0] = 1.0/dfactor[0]; dfactor_d[1] = 1.0/dfactor[1];  dfactor_d[2] = 1.0/dfactor[2];

         std::cout << "ok so far: "<< std::endl;
        downsample3dvol<T>((T *&)image_resampled,(T *)data1d,out_sz,in_sz,dfactor_d,0);
        std::cout << "down done: "<< std::endl;
    }else{//just copy data  NB assume that dfactor=1 along all dimensions !
        out_sz = in_sz;
        V3DLONG pagesz_resample = (V3DLONG)(out_sz[0]*out_sz[1]*out_sz[2]);
        try {image_resampled = new T [pagesz_resample];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}
        for(unsigned int i = 0; i< pagesz_resample; i++)
            image_resampled[i] = data1d[i];
    }
    std::cout << "Resampling Done."<<std::endl;
    //I_resized =v3d2ItkImage<ITKImageType,ImageScalarType>(inimg,in_sz,c);
    typename ITKImageType::Pointer I_resized  =  ITKImageType::New();

    std::cout <<"image resampled: " << (float)image_resampled[0] << ", "<< (float)image_resampled[1] << ", "<< (float)image_resampled[2] << std::endl;

    I_resized =rawData2ItkImage<ITKImageType,T>((T *)image_resampled,out_sz);

    return I_resized;

}

//template<typename ITKImageType>
//typename ITKImageType::Pointer resize_image_v3d(Image4DSimple *inimg,const long int * out_sz){






//}



#endif
