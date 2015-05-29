#ifndef _UTIL_H
#define _UTIL_H



template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer v3d2ItkImage(Image4DSimple *inimg,const long int *in_sz, unsigned int c=1);


template<typename ImageType>
typename ImageType::Pointer rawData2ItkImage(unsigned char *data1d,const long int *in_sz);


////TODO !
template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer swc2ItkImage(char * swc_file,const long int *in_sz);
////TODO !
template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer swc2ItkImage(char * swc_file,const long int *in_sz){

    typename ITKImageType::Pointer I  =  ITKImageType::New();
    return I;

}


//definition here to avoid explicit instantiation of template function
template<typename ITKImageType,typename ImageScalarType>
typename ITKImageType::Pointer v3d2ItkImage(Image4DSimple *inimg,const long int *in_sz, unsigned int c=1){

    typename ITKImageType::Pointer I  =  ITKImageType::New();

    I =rawData2ItkImage<ITKImageType>((unsigned char *)inimg->getRawDataAtChannel(c),in_sz);

    //    ////rescale in [0 1]
     typedef itk::RescaleIntensityImageFilter< ITKImageType, ITKImageType > RescaleFilterType;
    typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(I);
    rescaleFilter->SetOutputMinimum(0.0);
    rescaleFilter->SetOutputMaximum(1.0);

    rescaleFilter->Update();
    return rescaleFilter->GetOutput();
}


//definition here to avoid explicit instantiation of template function
template<typename ImageType>
//void rawData2ItkImage(ImageType::PixelType *data1d){
typename ImageType::Pointer rawData2ItkImage(unsigned char *data1d,const long int *in_sz){
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

  cout<<"size    "<<size <<endl;


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



  //    typename ImageType::IndexType idx;
  //    idx.Fill(0);
  //    typename ImageType::RegionType region;
  //    region.SetSize( size );
  //    region.SetIndex( idx );

  //    I->SetRegions(region);
  //    I->Allocate();
  //    I->FillBuffer(0);

  //    I->Update();

  cout<<"allocating image  " <<endl;


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

        unsigned char PixelVaule =  data1d[offsetk + offsetj + ix];
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





#endif
