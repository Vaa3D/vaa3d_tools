/*=========================================================================
 Copyright 2009 Rensselaer Polytechnic Institute
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 =========================================================================*/

/*=========================================================================
 
 Program:   Farsight Biological Image Segmentation and Visualization Toolkit
 Language:  C++
 Date:      $Date:  $
 Version:   $Revision: 0.00 $
 
 =========================================================================*/

#include "ImageOperation.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkShapeDetectionLevelSetImageFilter.h"
#include "itkTimeProbe.h"

ImageOperation::ImageOperation()
{
    SM = 0;
    SN = 0;
    SZ = 0;
    mask_set = false;
    display_set = false;
    u1 = 100;
    u2 = 0;
    sigma1 = sigma2 = 100;
    num_soma = 0;
    coding_method = 1;
    IMask = NULL;
}

void ImageOperation::ImComputeInitBackgroundModel(double th)
{
    
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    typedef itk::ImageSliceIteratorWithIndex< ProbImageType > SliceIteratorType1;
    
    
    SliceIteratorType it(I, I->GetRequestedRegion() );
    it.SetFirstDirection( 0 );
    it.SetSecondDirection( 1 );
    it.GoToBegin();
    
    SliceIteratorType1 it1(IVessel, IVessel->GetRequestedRegion() );
    it1.SetFirstDirection( 0 );
    it1.SetSecondDirection( 1 );
    it1.GoToBegin();
    
    std::vector<float> intensity;
    I_cu.clear();
    
    while( !it.IsAtEnd() )
    {
        while ( !it.IsAtEndOfSlice() )
        {
            while ( !it.IsAtEndOfLine() )
            {
                //if( it1.Get() < th )
                //{
                intensity.push_back(it.Get());
                //}
                //else
                //{
                // I_cu.push_back(it.Get());
                //}
                ++it;
                ++it1;
            }
            it.NextLine();
            it1.NextLine();
        }
        it.NextSlice();
        it1.NextSlice();
    }
    
    u2 = 0;
    sigma2 = 0;
    for(std::vector<float>::iterator j=intensity.begin();j!=intensity.end();++j)
    {
        u2 += *j;
    }
    u2 /= intensity.size();
    
    for(std::vector<float>::iterator j=intensity.begin();j!=intensity.end();++j)
    {
        sigma2 += pow(*j - u2, 2);
    }
    
    sigma2 = sqrt(sigma2/intensity.size());
    std::cout<<"background model:"<<u2<<","<<sigma2<<std::endl;
    
    /*u1 = 0;
     sigma1 = 0;
     for(std::vector<float>::iterator j=I_cu.begin();j!=I_cu.end();++j)
     {
     u1 += *j;
     }
     u1 /= I_cu.size();
     for(std::vector<float>::iterator j=I_cu.begin();j!=I_cu.end();++j)
     {
     sigma1 += pow(*j - u1, 2);
     }
     sigma1 = sqrt(sigma1/I_cu.size());
     std::cout<<"foreground model:"<<u1<<","<<sigma1<<std::endl;*/
    
}

void ImageOperation::ImComputeInitForegroundModel()
{
    typedef itk::NearestNeighborInterpolateImageFunction<
    ImageType, float>  InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    interpolator->SetInputImage(I);
    I_cu.clear();
	//consider all the seed points as initial foreground
    for( int j = 0; j < SeedPt.NP; j++ )
    {
        ImageType::IndexType index;
        index[0] = SeedPt.Pt[j].x;
        index[1] = SeedPt.Pt[j].y;
        index[2] = SeedPt.Pt[j].z;
        float temp = interpolator->EvaluateAtIndex(index);
        I_cu.push_back(temp);
	}
    u1 = 0;
    sigma1 = 0;
    for(std::vector<float>::iterator j=I_cu.begin();j!=I_cu.end();++j)
    {
        u1 += *j;
    }
    u1 /= I_cu.size();
    for(std::vector<float>::iterator j=I_cu.begin();j!=I_cu.end();++j)
    {
        sigma1 += pow(*j - u1, 2);
    }
    
    sigma1 = sqrt(sigma1/I_cu.size());
    std::cout<<"foreground model:"<<u1<<","<<sigma1<<std::endl;
    //I_cu.clear();
    
}
void ImageOperation::ImComputeForegroundModel(PointList3D Cu, std::vector<float> Ru)
{
    typedef itk::NearestNeighborInterpolateImageFunction<
    ImageType, float>  InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    interpolator->SetInputImage(I);
    
    Vector3D v1,v2,v3, vtemp;
    float pi = 3.1415926;
    int m = 8;
    Point3D temp_r_pt;
    
    for( int j = 0; j < Cu.NP; j++ )
    {
        ImageType::IndexType index;
        index[0] = Cu.Pt[j].x;
        index[1] = Cu.Pt[j].y;
        index[2] = Cu.Pt[j].z;
        float temp = interpolator->EvaluateAtIndex(index);
        I_cu.push_back(temp);
        
        
        /*if( j == 0 )
         {
         v1.x = Cu.Pt[0].x - Cu.Pt[1].x;
         v1.y = Cu.Pt[0].y - Cu.Pt[1].y;
         v1.z = Cu.Pt[0].z - Cu.Pt[1].z;
         v1.ConvertUnit();
         }
         else
         {
         v1.x = Cu.Pt[j].x - Cu.Pt[j-1].x;
         v1.y = Cu.Pt[j].y - Cu.Pt[j-1].y;
         v1.z = Cu.Pt[j].z - Cu.Pt[j-1].z;
         v1.ConvertUnit();
         }
         
         v2.x = -v1.z;
         v2.y = 0;
         v2.z = v1.x;
         v2.ConvertUnit();
         v3.x = 1;
         v3.y = -(pow(v1.x,2) + pow(v1.z,2))/(v1.x*v1.y + std::numeric_limits<float>::epsilon());
         v3.z = v1.z/(v1.x + std::numeric_limits<float>::epsilon());
         v3.ConvertUnit();
         
         for( int k = 0; k < m; k++ )
         {
         float theta = (2 * pi * k)/m;
         vtemp.x = v2.x * cos(theta) + v3.x * sin(theta);
         vtemp.y = v2.y * cos(theta) + v3.y * sin(theta);
         vtemp.z = v2.z * cos(theta) + v3.z * sin(theta);
         vtemp.ConvertUnit();
         
         vnl_vector<float> oj(3);
         oj(0) = vtemp.x;
         oj(1) = vtemp.y;
         oj(2) = vtemp.z;
         
         temp_r_pt.x = Cu.Pt[j].x + 1/2 * Ru[j] * vtemp.x;
         temp_r_pt.y = Cu.Pt[j].y + 1/2 * Ru[j] * vtemp.y;
         temp_r_pt.z = Cu.Pt[j].z + 1/2 * Ru[j] * vtemp.z;
         
         if( temp_r_pt.check_out_of_range_3D(SM,SN,SZ) )
         continue;
         
         ProbImageType::IndexType temp_index;
         temp_index[0] = temp_r_pt.x;
         temp_index[1] = temp_r_pt.y;
         temp_index[2] = temp_r_pt.z;
         float temp = interpolator->EvaluateAtIndex(temp_index);
         I_cu.push_back(temp);
         }*/
    }
    
    u1 = 0;
    sigma1 = 0;
    for(std::vector<float>::iterator j=I_cu.begin();j!=I_cu.end();++j)
    {
        u1 += *j;
    }
    u1 /= I_cu.size();
    for(std::vector<float>::iterator j=I_cu.begin();j!=I_cu.end();++j)
    {
        sigma1 += pow(*j - u1, 2);
    }
    
    sigma1 = sqrt(sigma1/I_cu.size());
    
    //std::cout<<"foreground model:"<<u1<<","<<sigma1<<std::endl;
    
}

IOImagePointer ImageOperation::ImBkSub(IOImagePointer In)
{
	std::cout<<"subtracting background..."<<std::endl;
	int radius = 10;
    
    typedef itk::SubtractImageFilter<IOImageType, IOImageType, IOImageType>
    filterType;
	filterType::Pointer filter;
	filter = filterType::New();
    
    filter->SetInput1(In);
	filter->SetInput2(ImGaussian_XY(In, radius));
	filter->Update();
	return filter->GetOutput();
}

void ImageOperation::ImRemoveSlice(int in)
{
    if( in == 0 )
        return;
    
    SZ = I->GetLargestPossibleRegion().GetSize()[2];
    
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    SliceIteratorType It( I, I->GetRequestedRegion() );
    
    It.SetFirstDirection( 0 );
    It.SetSecondDirection( 1 );
    It.GoToBegin();
    
    while( !It.IsAtEnd() )
    {
        while ( !It.IsAtEndOfSlice() )
        {
            while ( !It.IsAtEndOfLine() )
            {
                if( It.GetIndex()[2] < in || It.GetIndex()[2] >= SZ - in )
                    It.Set(0);
                ++It;
            }
            It.NextLine();
        }
        It.NextSlice();
    }
}

void ImageOperation::ImDisplayRead(const char *filename, int shrink_factor)
{
	/* //IOImagePointer I;
     typedef itk::ImageFileReader<IORGBImageType> ReaderType;
     ReaderType::Pointer reader = ReaderType::New();
     
     reader->SetFileName(filename);
     IORGBImagePointer IIO = reader->GetOutput();
     reader->Update();
     
     typedef itk::CastImageFilter<IORGBImageType, RGBImageType> CasterType;
     
     CasterType::Pointer caster = CasterType::New();
     caster->SetInput(IIO);
     
     caster->Update();
     
     //RGBImagePointer IRGB3D = RGBImageType::New();
     //IRGB3D = caster->GetOutput();
     
     IRGB = caster->GetOutput();
     
     typedef itk::ShrinkImageFilter< RGBImageType,RGBImageType > ShrinkFilterType;
     ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
     
     shrinkFilter->SetInput( IRGB );
     unsigned int dfactors[3] = { shrink_factor, shrink_factor, 1};
     shrinkFilter->SetShrinkFactors(dfactors);
     shrinkFilter->UpdateLargestPossibleRegion();
     IRGB = shrinkFilter->GetOutput();
     shrinkFilter->Update(); */
    
	//IRGB = ImMinProjection(IRGB3D);
    
    
    typedef itk::ImageFileReader<IOImageType> ReaderType1;
	ReaderType1::Pointer reader1 = ReaderType1::New();
    
	reader1->SetFileName(filename);
	IOImagePointer IIO1 = reader1->GetOutput();
	reader1->Update();
    
    typedef itk::CastImageFilter<IOImageType, ImageType> CasterType1;
    
    CasterType1::Pointer caster1 = CasterType1::New();
    caster1->SetInput(IIO1);
    
	caster1->Update();
    
	IDisplay = caster1->GetOutput();
    
	typedef itk::ShrinkImageFilter< ImageType, ImageType > ShrinkFilterType1;
    ShrinkFilterType1::Pointer shrinkFilter1 = ShrinkFilterType1::New();
    
    shrinkFilter1->SetInput( IDisplay );
    unsigned int dfactors1[3] = { shrink_factor, shrink_factor, 1};
    shrinkFilter1->SetShrinkFactors(dfactors1);
    shrinkFilter1->UpdateLargestPossibleRegion();
    IDisplay = shrinkFilter1->GetOutput();
    shrinkFilter1->Update();
    
	display_set = true;
}

void ImageOperation::ImSeriesReadWrite(std::vector< std::string > filenames, const char *save_name, int shrink_factor, bool sixteen_bit)
{
    if( sixteen_bit )
    {
        typedef itk::ImageSeriesReader< IOImageType1 >  ReaderType;
        typedef itk::ImageFileWriter< IOImageType >  WriterType;
        
        ReaderType::Pointer reader = ReaderType::New();
        WriterType::Pointer writer = WriterType::New();
        
        //reader->SetImageIO( itk::TIFFImageIO::New() );
        
        reader->SetFileNames( filenames  );
        reader->Update();
        
        //typedef itk::CastImageFilter<IOImageType1, IOImageType> CasterType;
        //CasterType::Pointer caster = CasterType::New();
        //caster->SetInput(reader->GetOutput());
        //caster->Update();
        
        typedef itk::RescaleIntensityImageFilter< IOImageType1, IOImageType> RescaleFilterType;
        RescaleFilterType::Pointer rescale = RescaleFilterType::New();
        rescale->SetInput( reader->GetOutput() );
        rescale->SetOutputMinimum( 0 );
        rescale->SetOutputMaximum( 255 );
        rescale->Update();
        
        writer->SetFileName( save_name );
        writer->SetInput( ImShrink(rescale->GetOutput(), shrink_factor) );
        
        try
        {
            writer->Update();
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "ExceptionObject caught !" << std::endl;
            std::cerr << err << std::endl;
        }
    }
    else
    {
        typedef itk::ImageSeriesReader< IOImageType >  ReaderType;
        typedef itk::ImageFileWriter< IOImageType >  WriterType;
        
        ReaderType::Pointer reader = ReaderType::New();
        WriterType::Pointer writer = WriterType::New();
        
        //reader->SetImageIO( itk::TIFFImageIO::New() );
        
        reader->SetFileNames( filenames  );
        reader->Update();
        
        //typedef itk::CastImageFilter<IOImageType1, IOImageType> CasterType;
        //CasterType::Pointer caster = CasterType::New();
        //caster->SetInput(reader->GetOutput());
        //caster->Update();
        
        typedef itk::RescaleIntensityImageFilter< IOImageType, IOImageType> RescaleFilterType;
        RescaleFilterType::Pointer rescale = RescaleFilterType::New();
        rescale->SetInput( reader->GetOutput() );
        rescale->SetOutputMinimum( 0 );
        rescale->SetOutputMaximum( 255 );
        rescale->Update();
        
        writer->SetFileName( save_name );
        writer->SetInput( ImShrink(rescale->GetOutput(), shrink_factor) );
        
        try
        {
            writer->Update();
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "ExceptionObject caught !" << std::endl;
            std::cerr << err << std::endl;
        }
    }
}

void ImageOperation::ImRead(const char *filename)
{
	//IOImagePointer I;
	typedef itk::ImageFileReader<IOImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
    
	reader->SetFileName(filename);
	IOImagePointer IIO = reader->GetOutput();
	reader->Update();
    
    typedef itk::CastImageFilter<IOImageType, ImageType> CasterType;
    
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(IIO);
    
	caster->Update();
	I = caster->GetOutput();
    
	//ImSmoothing(1);
	//ImRemoveSlice(in);
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    rescale->SetInput( I );
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 255 );
	I = rescale->GetOutput();
    rescale->Update();
    
	SM = I->GetLargestPossibleRegion().GetSize()[0];
    SN = I->GetLargestPossibleRegion().GetSize()[1];
    SZ = I->GetLargestPossibleRegion().GetSize()[2];
}


void ImageOperation:: Imcreate(unsigned char *datald, int *in_sz)
{
    typedef itk::Image<signed int, 3> ImageType;
    SN = in_sz[0];
    SM = in_sz[1];
    SZ = in_sz[2];

    I  = ImageType::New();
    ImageType::SizeType size;
    size[0] = SN;
    size[1] = SM;
    size[2] = SZ;

    ImageType::IndexType idx;
    idx.Fill(0);
    ImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( idx );

    I->SetRegions(region);
    I->Allocate();
    I->FillBuffer(0);

    for(int iz = 0; iz < SZ; iz++)
      {
          int offsetk = iz*SM*SN;
          for(int iy = 0; iy < SM; iy++)
          {
              int offsetj = iy*SN;
              for(int ix = 0; ix < SN; ix++)
              {

                  int PixelVaule = (int) datald[offsetk + offsetj + ix];
                  itk::Index<3> indexX;
                  indexX[0] = ix;
                  indexX[1] = iy;
                  indexX[2] = iz;
                  I->SetPixel(indexX, PixelVaule);
              }
          }

      }


}
void ImageOperation::ImReadSoma(const char *filename)
{
	//IOImagePointer I;
	std::cout<< "Load soma image file"<<std::endl;
	typedef itk::ImageFileReader<SomaImageLabelType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
    
	reader->SetFileName(filename);
	
	std::cout<< "Creating Imask"<<std::endl;
    typedef itk::BinaryThresholdImageFilter< SomaImageLabelType, ImageType> SomaMaskBinaryFilterType;
    SomaMaskBinaryFilterType::Pointer thresholder = SomaMaskBinaryFilterType::New();
	thresholder->SetLowerThreshold( 1);
	thresholder->SetOutsideValue( 0);
	thresholder->SetInsideValue(255);
	thresholder->SetInput( reader->GetOutput());
	thresholder->Update();
	IMask = thresholder->GetOutput();

	//typedef itk::ConnectedComponentImageFilter< ImageType, LabelImageType> LabelFilterType;
	//typedef itk::RelabelComponentImageFilter< LabelImageType, LabelImageType> RelabelFilterType;

	//LabelFilterType::Pointer labelFilter = LabelFilterType::New();
	//labelFilter->SetInput(IMask);
	//RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
	//relabelFilter->SetInput(labelFilter->GetOutput());
	//relabelFilter->Update();
	//ISoma = relabelFilter->GetOutput();

	//std::cout<< "Soma features"<<std::endl;
	//typedef itk::LabelGeometryImageFilter< LabelImageType > LabelGeometryType;
	//LabelGeometryType::Pointer labelGeometryFilter = LabelGeometryType::New();
	//labelGeometryFilter->SetInput( ISoma);
	//labelGeometryFilter->Update();

	//LabelGeometryType::LabelPointType index_temp;
	//int labelValue = labelGeometryFilter->GetNumberOfLabels()-1;

	//num_soma = labelValue;
	//std::cout<< "Soma num:"<<num_soma<<std::endl;

	////compute the centroids of soma
	//Point3D temp;
	////PointList3D new_SeedPt(labelValue+1);
	//Centroid.RemoveAllPts();
	//for( int i = 1; i <= labelValue; i++)
	//{
	//    index_temp = labelGeometryFilter->GetCentroid(i);
	//    soma_size.push_back(labelGeometryFilter->GetVolume(i));
	//    float temp_radius = labelGeometryFilter->GetMinorAxisLength(i)/2;
	//    soma_radii.push_back(temp_radius);
	//    temp.x = index_temp[0];
	//    temp.y = index_temp[1];
	//    temp.z = index_temp[2];
	//    Centroid.AddPt(temp);
	//}

	//std::cout<< "voronoi map"<<std::endl;
	////compute the voronoi map
	//typedef itk::DanielssonDistanceMapImageFilter< LabelImageType, LabelImageType > DistanceMapFilterType;
	//DistanceMapFilterType::Pointer DMfilter = DistanceMapFilterType::New();
	//DMfilter->SetInput( ISoma);
	//DMfilter->Update();
	//IVoronoi = DMfilter->GetVoronoiMap();
	//std::cout<< "voronoi map done"<<std::endl;
}

void ImageOperation::ImRead_NoSmooth(const char *filename, int in)
{
	//IOImagePointer I;
	typedef itk::ImageFileReader<IOImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
    
	reader->SetFileName(filename);
	IOImagePointer IIO = reader->GetOutput();
	reader->Update();
    
    typedef itk::CastImageFilter<IOImageType, ImageType> CasterType;
    
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(IIO);
    
	caster->Update();
	I = caster->GetOutput();
    
	//ImSmoothing();
	ImRemoveSlice(in);
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    rescale->SetInput( I );
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 255 );
    
    //I = ImGaussian(rescale->GetOutput(),1);
	I = rescale->GetOutput();
    rescale->Update();
}

void ImageOperation::ImSmoothing(int smoothing_scale)
{
    
	if( smoothing_scale != 0 )
	{
        I = ImGaussian(I,smoothing_scale);
	}
    
    /*typedef itk::LaplacianRecursiveGaussianImageFilter<
     ImageType, ImageType >  FilterType;
     
     FilterType::Pointer laplacian = FilterType::New();
     laplacian->SetNormalizeAcrossScale( true );
     laplacian->SetInput( I );
     laplacian->SetSigma( 1 );
     laplacian->Update();
     I = laplacian->GetOutput();
     
     typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
     SliceIteratorType It( I, I->GetRequestedRegion() );
     It.SetFirstDirection( 0 );
     It.SetSecondDirection( 1 );
     It.GoToBegin();
     
     while( !It.IsAtEnd() )
     {
     while ( !It.IsAtEndOfSlice() )
     {
     while ( !It.IsAtEndOfLine() )
     {
     if( It.Get() > 0)
     It.Set(0);
     if( It.Get() < 0)
     It.Set(It.Get()*-1);
     
     ++It;
	 }
     It.NextLine();
     }
     It.NextSlice();
     }*/
    
    /*typedef itk::CastImageFilter<ImageType, ProbImageType> CasterType;
     CasterType::Pointer caster = CasterType::New();
     caster->SetInput(I);
     caster->Update();
     
     typedef itk::GradientAnisotropicDiffusionImageFilter<
     ProbImageType, ProbImageType >  FilterType;
     FilterType::Pointer filter = FilterType::New();
     
     filter->SetInput( ImGaussian(caster->GetOutput(), 1) );
     filter->SetNumberOfIterations( 5 );
     filter->SetTimeStep( 0.0625 );
     
     filter->Update();
     
     typedef itk::RescaleIntensityImageFilter< ProbImageType, ImageType> RescaleFilterType;
     RescaleFilterType::Pointer rescale = RescaleFilterType::New();
     rescale->SetInput( filter->GetOutput() );
     rescale->SetOutputMinimum( 0 );
     rescale->SetOutputMaximum( 255 );
     
     I = rescale->GetOutput();
     rescale->Update();  */
    
}


void ImageOperation::ConvertReadImage()
{
    /*//ImagePointer IOut;
     typedef itk::CastImageFilter<IOImageType, ImageType> CasterType;
     CasterType::Pointer caster = CasterType::New();
     caster->SetInput(IIO);
     
     typedef itk::RescaleIntensityImageFilter< ImageType, ImageType> RescaleFilterType;
     RescaleFilterType::Pointer rescale = RescaleFilterType::New();
     
     rescale->SetInput( caster->GetOutput() );
     rescale->SetOutputMinimum( 0 );
     rescale->SetOutputMaximum( 255 );
     
     I = rescale->GetOutput();
     rescale->Update();
     
     //return IOut; */
}

LabelImagePointer2D ImageOperation::ImMaxProjection(LabelImagePointer IInput)
{
    int dim = 2;
    
    typedef itk::MaximumProjectionImageFilter< LabelImageType, LabelImageType > FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( IInput );
    filter->SetProjectionDimension( dim );
    // to be sure that the result is ok with several threads, even on a single
    // proc computer
    filter->SetNumberOfThreads( 2 );
    filter->Update();
    LabelImageType::SizeType inputSize = filter->GetOutput()->GetLargestPossibleRegion().GetSize();
    typedef itk::ExtractImageFilter< LabelImageType, LabelImageType2D > ExtractType;
    ExtractType::Pointer extract = ExtractType::New();
    extract->SetInput( filter->GetOutput() );
    LabelImageType::SizeType size;
    for(int i=0; i<=3; i++)
    {
        if(i == dim)
        {
            size[i] = 0;
        }
        else
        {
            size[i] = inputSize[i];
        }
    }
    LabelImageType::IndexType idx;
    idx.Fill(0);
    LabelImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( idx );
    extract->SetExtractionRegion( region );
    extract->Update();
    LabelImagePointer2D I2D = extract->GetOutput();
    
    return I2D;
    /* typedef itk::ImageLinearIteratorWithIndex< LabelImageType2D > LinearIteratorType;
     typedef itk::ImageSliceConstIteratorWithIndex< LabelImageType > SliceIteratorType;
     unsigned int projectionDirection = 2;
     unsigned int i, j;
     unsigned int direction[2];
     for (i = 0, j = 0; i < 3; ++i )
     {
     if (i != projectionDirection)
     {
     direction[j] = i;
     j++;
     }
     }
     
     LabelImageType2D::RegionType region;
     LabelImageType2D::RegionType::SizeType size;
     LabelImageType2D::RegionType::IndexType index;
     LabelImageType::RegionType requestedRegion = IInput->GetRequestedRegion();
     
     index[ direction[0] ] = requestedRegion.GetIndex()[ direction[0] ];
     index[ 1- direction[0] ] = requestedRegion.GetIndex()[ direction[1] ];
     size[ direction[0] ] = requestedRegion.GetSize()[ direction[0] ];
     size[ 1- direction[0] ] = requestedRegion.GetSize()[ direction[1] ];
     
     region.SetSize( size );
     region.SetIndex( index );
     LabelImagePointer2D I2D = LabelImageType2D::New();
     I2D->SetRegions( region );
     I2D->Allocate();
     
     SliceIteratorType inputIt( IInput, IInput->GetRequestedRegion() );
     LinearIteratorType outputIt( I2D, I2D->GetRequestedRegion() );
     inputIt.SetFirstDirection( direction[1] );
     inputIt.SetSecondDirection( direction[0] );
     outputIt.SetDirection( 1 - direction[0] );
     
     outputIt.GoToBegin();
     while ( ! outputIt.IsAtEnd() )
     {
     while ( ! outputIt.IsAtEndOfLine() )
     {
     outputIt.Set( itk::NumericTraits<unsigned short>::NonpositiveMin() );
     ++outputIt;
     }
     outputIt.NextLine();
     }
     
     inputIt.GoToBegin();
     outputIt.GoToBegin();
     while( !inputIt.IsAtEnd() )
     {
     while ( !inputIt.IsAtEndOfSlice() )
     {
     while ( !inputIt.IsAtEndOfLine() )
     {
     outputIt.Set( vnl_math_max( outputIt.Get(), inputIt.Get() ));
     ++inputIt;
     ++outputIt;
     }
     outputIt.NextLine();
     inputIt.NextLine();
     }
     
     outputIt.GoToBegin();
     inputIt.NextSlice();
     }
     
     return I2D; */
}


ImagePointer2D ImageOperation::ImMaxProjection(ImagePointer IInput)
{
	int dim = 2;
    
	typedef itk::MaximumProjectionImageFilter< ImageType, ImageType2D > FilterType;
	FilterType::Pointer filter = FilterType::New();
    
	filter->SetProjectionDimension(dim);
	filter->SetNumberOfThreads(16);
	filter->SetInput( IInput );
	
	filter->Update();
	ImageType2D::Pointer I2D = filter->GetOutput();
	return I2D;
    
    
    
    /*filter->SetInput( IInput );
     filter->SetProjectionDimension( dim );
     // to be sure that the result is ok with several threads, even on a single
     // proc computer
     filter->SetNumberOfThreads( 2 );
     filter->Update();
     ImageType::SizeType inputSize = filter->GetOutput()->GetLargestPossibleRegion().GetSize();
     typedef itk::ExtractImageFilter< ImageType, ImageType2D > ExtractType;
     ExtractType::Pointer extract = ExtractType::New();
     extract->SetInput( filter->GetOutput() );
     ImageType::SizeType size;
     for(int i=0; i<=3; i++)
     {
     if(i == dim)
     {
     size[i] = 0;
     }
     else
     {
     size[i] = inputSize[i];
     }
     }
     ImageType::IndexType idx;
     idx.Fill(0);
     ImageType::RegionType region;
     region.SetSize( size );
     region.SetIndex( idx );
     extract->SetExtractionRegion( region );
     extract->Update();
     ImagePointer2D I2D = extract->GetOutput();
     
     return I2D;*/
    
    /* typedef itk::ImageLinearIteratorWithIndex< ImageType2D > LinearIteratorType;
     typedef itk::ImageSliceConstIteratorWithIndex< ImageType > SliceIteratorType;
     unsigned int projectionDirection = 2;
     unsigned int i, j;
     unsigned int direction[2];
     for (i = 0, j = 0; i < 3; ++i )
     {
     if (i != projectionDirection)
     {
     direction[j] = i;
     j++;
     }
     }
     
     ImageType2D::RegionType region;
     ImageType2D::RegionType::SizeType size;
     ImageType2D::RegionType::IndexType index;
     ImageType::RegionType requestedRegion = IInput->GetRequestedRegion();
     
     index[ direction[0] ] = requestedRegion.GetIndex()[ direction[0] ];
     index[ 1- direction[0] ] = requestedRegion.GetIndex()[ direction[1] ];
     size[ direction[0] ] = requestedRegion.GetSize()[ direction[0] ];
     size[ 1- direction[0] ] = requestedRegion.GetSize()[ direction[1] ];
     
     region.SetSize( size );
     region.SetIndex( index );
     ImagePointer2D I2D = ImageType2D::New();
     I2D->SetRegions( region );
     I2D->Allocate();
     
     SliceIteratorType inputIt( IInput, IInput->GetRequestedRegion() );
     LinearIteratorType outputIt( I2D, I2D->GetRequestedRegion() );
     inputIt.SetFirstDirection( direction[1] );
     inputIt.SetSecondDirection( direction[0] );
     outputIt.SetDirection( 1 - direction[0] );
     
     outputIt.GoToBegin();
     while ( ! outputIt.IsAtEnd() )
     {
     while ( ! outputIt.IsAtEndOfLine() )
     {
     outputIt.Set( itk::NumericTraits<unsigned short>::NonpositiveMin() );
     ++outputIt;
     }
     outputIt.NextLine();
     }
     
     inputIt.GoToBegin();
     outputIt.GoToBegin();
     while( !inputIt.IsAtEnd() )
     {
     while ( !inputIt.IsAtEndOfSlice() )
     {
     while ( !inputIt.IsAtEndOfLine() )
     {
     outputIt.Set( vnl_math_max( outputIt.Get(), inputIt.Get() ));
     ++inputIt;
     ++outputIt;
     }
     outputIt.NextLine();
     inputIt.NextLine();
     }
     
     outputIt.GoToBegin();
     inputIt.NextSlice();
     }
     
     return I2D; */
}

RGBImagePointer2D ImageOperation::ImMaxProjection1(ImagePointer IInput)
{
    typedef itk::ImageLinearIteratorWithIndex< ImageType2D > LinearIteratorType;
    typedef itk::ImageSliceConstIteratorWithIndex< ImageType > SliceIteratorType;
    unsigned int projectionDirection = 2;
    unsigned int i, j;
    unsigned int direction[2];
    for (i = 0, j = 0; i < 3; ++i )
    {
        if (i != projectionDirection)
        {
            direction[j] = i;
            j++;
        }
    }
    
    ImageType2D::RegionType region;
    ImageType2D::RegionType::SizeType size;
    ImageType2D::RegionType::IndexType index;
    ImageType::RegionType requestedRegion = IInput->GetRequestedRegion();
    
    index[ direction[0] ] = requestedRegion.GetIndex()[ direction[0] ];
    index[ 1- direction[0] ] = requestedRegion.GetIndex()[ direction[1] ];
    size[ direction[0] ] = requestedRegion.GetSize()[ direction[0] ];
    size[ 1- direction[0] ] = requestedRegion.GetSize()[ direction[1] ];
    
    region.SetSize( size );
    region.SetIndex( index );
    ImagePointer2D I2D = ImageType2D::New();
    I2D->SetRegions( region );
    I2D->Allocate();
    
    SliceIteratorType inputIt( IInput, IInput->GetRequestedRegion() );
    LinearIteratorType outputIt( I2D, I2D->GetRequestedRegion() );
    inputIt.SetFirstDirection( direction[1] );
    inputIt.SetSecondDirection( direction[0] );
    outputIt.SetDirection( 1 - direction[0] );
    
    outputIt.GoToBegin();
    while ( ! outputIt.IsAtEnd() )
    {
        while ( ! outputIt.IsAtEndOfLine() )
        {
            outputIt.Set( itk::NumericTraits<unsigned short>::NonpositiveMin() );
            ++outputIt;
        }
        outputIt.NextLine();
    }
    
    inputIt.GoToBegin();
    outputIt.GoToBegin();
    while( !inputIt.IsAtEnd() )
    {
        while ( !inputIt.IsAtEndOfSlice() )
        {
            while ( !inputIt.IsAtEndOfLine() )
            {
                outputIt.Set( vnl_math_max( outputIt.Get(), inputIt.Get() ));
                ++inputIt;
                ++outputIt;
            }
            outputIt.NextLine();
            inputIt.NextLine();
        }
        
        outputIt.GoToBegin();
        inputIt.NextSlice();
    }
    
    typedef itk::CastImageFilter<ImageType2D, RGBImageType2D> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(I2D);
    caster->Update();
    
    return caster->GetOutput();
}


ProbImagePointer2D ImageOperation::ImMaxProjection(ProbImagePointer IInput)
{
    
    int dim = 2;
    
    typedef itk::MaximumProjectionImageFilter< ProbImageType, ProbImageType > FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( IInput );
    filter->SetProjectionDimension( dim );
    // to be sure that the result is ok with several threads, even on a single
    // proc computer
    filter->SetNumberOfThreads( 2 );
    filter->Update();
    ProbImageType::SizeType inputSize = filter->GetOutput()->GetLargestPossibleRegion().GetSize();
    typedef itk::ExtractImageFilter< ProbImageType, ProbImageType2D > ExtractType;
    ExtractType::Pointer extract = ExtractType::New();
    extract->SetInput( filter->GetOutput() );
    ProbImageType::SizeType size;
    for(int i=0; i<=3; i++)
    {
        if(i == dim)
        {
            size[i] = 0;
        }
        else
        {
            size[i] = inputSize[i];
        }
    }
    ProbImageType::IndexType idx;
    idx.Fill(0);
    ProbImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( idx );
    extract->SetExtractionRegion( region );
    extract->Update();
    ProbImagePointer2D I2D = extract->GetOutput();
    
    return I2D;
    
    /*typedef itk::ImageLinearIteratorWithIndex< ProbImageType2D > LinearIteratorType;
     typedef itk::ImageSliceConstIteratorWithIndex< ProbImageType > SliceIteratorType;
     unsigned int projectionDirection = 2;
     unsigned int i, j;
     unsigned int direction[2];
     for (i = 0, j = 0; i < 3; ++i )
     {
     if (i != projectionDirection)
     {
     direction[j] = i;
     j++;
     }
     }
     
     ImageType2D::RegionType region;
     ImageType2D::RegionType::SizeType size;
     ImageType2D::RegionType::IndexType index;
     ImageType::RegionType requestedRegion = IInput->GetRequestedRegion();
     
     index[ direction[0] ] = requestedRegion.GetIndex()[ direction[0] ];
     index[ 1- direction[0] ] = requestedRegion.GetIndex()[ direction[1] ];
     size[ direction[0] ] = requestedRegion.GetSize()[ direction[0] ];
     size[ 1- direction[0] ] = requestedRegion.GetSize()[ direction[1] ];
     
     region.SetSize( size );
     region.SetIndex( index );
     ProbImagePointer2D I2D = ProbImageType2D::New();
     I2D->SetRegions( region );
     I2D->Allocate();
     
     SliceIteratorType inputIt( IInput, IInput->GetRequestedRegion() );
     LinearIteratorType outputIt( I2D, I2D->GetRequestedRegion() );
     inputIt.SetFirstDirection( direction[1] );
     inputIt.SetSecondDirection( direction[0] );
     outputIt.SetDirection( 1 - direction[0] );
     
     outputIt.GoToBegin();
     while ( ! outputIt.IsAtEnd() )
     {
     while ( ! outputIt.IsAtEndOfLine() )
     {
     outputIt.Set( itk::NumericTraits<unsigned short>::NonpositiveMin() );
     ++outputIt;
     }
     outputIt.NextLine();
     }
     
     inputIt.GoToBegin();
     outputIt.GoToBegin();
     while( !inputIt.IsAtEnd() )
     {
     while ( !inputIt.IsAtEndOfSlice() )
     {
     while ( !inputIt.IsAtEndOfLine() )
     {
     outputIt.Set( vnl_math_max( outputIt.Get(), inputIt.Get() ));
     ++inputIt;
     ++outputIt;
     }
     outputIt.NextLine();
     inputIt.NextLine();
     }
     
     outputIt.GoToBegin();
     inputIt.NextSlice();
     }
     
     return I2D;*/
}
RGBImagePointer2D ImageOperation::ImMinProjection(RGBImagePointer IInput)
{
    typedef itk::ImageLinearIteratorWithIndex< RGBImageType2D > LinearIteratorType;
    typedef itk::ImageSliceConstIteratorWithIndex< RGBImageType > SliceIteratorType;
    unsigned int projectionDirection = 2;
    unsigned int i, j;
    unsigned int direction[2];
    for (i = 0, j = 0; i < 3; ++i )
    {
        if (i != projectionDirection)
        {
            direction[j] = i;
            j++;
        }
    }
    
    ImageType2D::RegionType region;
    ImageType2D::RegionType::SizeType size;
    ImageType2D::RegionType::IndexType index;
    ImageType::RegionType requestedRegion = IInput->GetRequestedRegion();
    
    index[ direction[0] ] = requestedRegion.GetIndex()[ direction[0] ];
    index[ 1- direction[0] ] = requestedRegion.GetIndex()[ direction[1] ];
    size[ direction[0] ] = requestedRegion.GetSize()[ direction[0] ];
    size[ 1- direction[0] ] = requestedRegion.GetSize()[ direction[1] ];
    
    region.SetSize( size );
    region.SetIndex( index );
    RGBImagePointer2D I2D = RGBImageType2D::New();
    I2D->SetRegions( region );
    I2D->Allocate();
    
    SliceIteratorType inputIt( IInput, IInput->GetRequestedRegion() );
    LinearIteratorType outputIt( I2D, I2D->GetRequestedRegion() );
    inputIt.SetFirstDirection( direction[1] );
    inputIt.SetSecondDirection( direction[0] );
    outputIt.SetDirection( 1 - direction[0] );
    
    outputIt.GoToBegin();
    while ( ! outputIt.IsAtEnd() )
    {
        while ( ! outputIt.IsAtEndOfLine() )
        {
            outputIt.Set( 10000 );
            ++outputIt;
        }
        outputIt.NextLine();
    }
    
    inputIt.GoToBegin();
    outputIt.GoToBegin();
    while( !inputIt.IsAtEnd() )
    {
        while ( !inputIt.IsAtEndOfSlice() )
        {
            while ( !inputIt.IsAtEndOfLine() )
            {
                RGBImageType2D::PixelType temp;
                temp[0] = vnl_math_min( outputIt.Get()[0], inputIt.Get()[0] );
                temp[1] = vnl_math_min( outputIt.Get()[1], inputIt.Get()[1] );
                temp[2] = vnl_math_min( outputIt.Get()[2], inputIt.Get()[2] );
                outputIt.Set( temp );
                ++inputIt;
                ++outputIt;
            }
            outputIt.NextLine();
            inputIt.NextLine();
        }
        
        outputIt.GoToBegin();
        inputIt.NextSlice();
    }
    
    return I2D;
    
}

RGBImagePointer2D ImageOperation::ImMaxProjection(RGBImagePointer IInput)
{
    typedef itk::ImageLinearIteratorWithIndex< RGBImageType2D > LinearIteratorType;
    typedef itk::ImageSliceConstIteratorWithIndex< RGBImageType > SliceIteratorType;
    unsigned int projectionDirection = 2;
    unsigned int i, j;
    unsigned int direction[2];
    for (i = 0, j = 0; i < 3; ++i )
    {
        if (i != projectionDirection)
        {
            direction[j] = i;
            j++;
        }
    }
    
    ImageType2D::RegionType region;
    ImageType2D::RegionType::SizeType size;
    ImageType2D::RegionType::IndexType index;
    ImageType::RegionType requestedRegion = IInput->GetRequestedRegion();
    
    index[ direction[0] ] = requestedRegion.GetIndex()[ direction[0] ];
    index[ 1- direction[0] ] = requestedRegion.GetIndex()[ direction[1] ];
    size[ direction[0] ] = requestedRegion.GetSize()[ direction[0] ];
    size[ 1- direction[0] ] = requestedRegion.GetSize()[ direction[1] ];
    
    region.SetSize( size );
    region.SetIndex( index );
    RGBImagePointer2D I2D = RGBImageType2D::New();
    I2D->SetRegions( region );
    I2D->Allocate();
    
    SliceIteratorType inputIt( IInput, IInput->GetRequestedRegion() );
    LinearIteratorType outputIt( I2D, I2D->GetRequestedRegion() );
    inputIt.SetFirstDirection( direction[1] );
    inputIt.SetSecondDirection( direction[0] );
    outputIt.SetDirection( 1 - direction[0] );
    
    outputIt.GoToBegin();
    while ( ! outputIt.IsAtEnd() )
    {
        while ( ! outputIt.IsAtEndOfLine() )
        {
            outputIt.Set( itk::NumericTraits<unsigned short>::NonpositiveMin() );
            ++outputIt;
        }
        outputIt.NextLine();
    }
    
    inputIt.GoToBegin();
    outputIt.GoToBegin();
    while( !inputIt.IsAtEnd() )
    {
        while ( !inputIt.IsAtEndOfSlice() )
        {
            while ( !inputIt.IsAtEndOfLine() )
            {
                RGBImageType2D::PixelType temp;
                temp[0] = vnl_math_max( outputIt.Get()[0], inputIt.Get()[0] );
                temp[1] = vnl_math_max( outputIt.Get()[1], inputIt.Get()[1] );
                temp[2] = vnl_math_max( outputIt.Get()[2], inputIt.Get()[2] );
                outputIt.Set( temp );
                ++inputIt;
                ++outputIt;
            }
            outputIt.NextLine();
            inputIt.NextLine();
        }
        
        outputIt.GoToBegin();
        inputIt.NextSlice();
    }
    
    return I2D;
    
}

ImagePointer ImageOperation::ImCopy()
{
    typedef itk::CastImageFilter<ImageType, ImageType> CasterType;
    
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(I);
	caster->Update();
	return caster->GetOutput();
}

void ImageOperation::ImWrite(const char *filename, int image_id)
{
	IOImagePointer1 IIO;
	typedef itk::ImageFileWriter<IOImageType1> WriterType;
    typedef itk::CastImageFilter<ProbImageType,IOImageType1> ProbCasterType;
	ProbCasterType::Pointer prob_caster = ProbCasterType::New();
	typedef itk::CastImageFilter<LabelImageType,IOImageType1> LabelCasterType;
	LabelCasterType::Pointer label_caster = LabelCasterType::New();
	typedef itk::CastImageFilter<ImageType, IOImageType1> CasterType;
	CasterType::Pointer caster = CasterType::New();
    
	if( image_id == 1 )
	{
        caster->SetInput(I);
        IIO = caster->GetOutput();
        caster->Update();
	}
	else if( image_id == 2 )
	{
        prob_caster->SetInput(IVessel);
        IIO = prob_caster->GetOutput();
        prob_caster->Update();
	}
	else if( image_id == 3 )
	{
        label_caster->SetInput(IL);
        IIO = label_caster->GetOutput();
        label_caster->Update();
	}
    
	WriterType::Pointer writer;
	writer = WriterType::New();
    
	writer->SetFileName(filename);
	writer->SetInput(IIO);
	writer->Update();
}

void ImageOperation::ImWrite(const char *filename, LabelImagePointer ROI)
{
	IOImagePointer IIO;
	typedef itk::ImageFileWriter<IOImageType> WriterType;
    
	typedef itk::CastImageFilter<LabelImageType, IOImageType> CasterType;
	CasterType::Pointer caster = CasterType::New();
    
    caster->SetInput(ROI);
	IIO = caster->GetOutput();
	caster->Update();
    
	WriterType::Pointer writer;
	writer = WriterType::New();
    
	writer->SetFileName(filename);
	writer->SetInput(IIO);
	writer->Update();
}


void ImageOperation::ImWrite(const char *filename, ImagePointer ROI)
{
	IOImagePointer IIO;
	typedef itk::ImageFileWriter<IOImageType> WriterType;
    
	typedef itk::CastImageFilter<ImageType, IOImageType> CasterType;
	CasterType::Pointer caster = CasterType::New();
    
    caster->SetInput(ROI);
	IIO = caster->GetOutput();
	caster->Update();
    
	WriterType::Pointer writer;
	writer = WriterType::New();
    
	writer->SetFileName(filename);
	writer->SetInput(IIO);
	writer->Update();
}

void ImageOperation::ImWrite_Soma(const char *filename)
{
	typedef itk::RescaleIntensityImageFilter< ImageType, ImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    rescale->SetInput( IMask );
    rescale->SetOutputMinimum( 0);
    rescale->SetOutputMaximum( 255);
    rescale->Update();
    
	IOImagePointer IIO;
	typedef itk::ImageFileWriter<IOImageType> WriterType;
	typedef itk::CastImageFilter<ImageType, IOImageType> CasterType;
	CasterType::Pointer caster = CasterType::New();
    
	caster->SetInput(rescale->GetOutput());
	IIO = caster->GetOutput();
	caster->Update();
    
	typedef itk::ImageFileWriter<IOImageType> WriterType;
    
	WriterType::Pointer writer;
	writer = WriterType::New();
    
	writer->SetFileName(filename);
	writer->SetInput(IIO);
	writer->Update();
}

void ImageOperation::ImWrite_Segmentation(const char *filename)
{
	IOImagePointer IIO;
	typedef itk::ImageFileWriter<IOImageType> WriterType;
	typedef itk::CastImageFilter<ImageType, IOImageType> CasterType;
	CasterType::Pointer caster = CasterType::New();
    
	caster->SetInput(ISeg);
	IIO = caster->GetOutput();
	caster->Update();
    
	typedef itk::ImageFileWriter<IOImageType> WriterType;
    
	WriterType::Pointer writer;
	writer = WriterType::New();
    
	writer->SetFileName(filename);
	writer->SetInput(IIO);
	writer->Update();
}


void ImageOperation::ConvertWriteImage()
{
	/*//IOImagePointer IIO;
     
     typedef itk::CastImageFilter<ImageType,IOImageType> CasterType;
     CasterType::Pointer caster = CasterType::New();
     
     caster->SetInput(I);
     IIO = caster->GetOutput();
     caster->Update();
     
     //return IIO;*/
}

void ImageOperation::ImMasking(int shrink_factor)
{
    typedef itk::ShrinkImageFilter< ImageType,ImageType > ShrinkFilterType;
    ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
    
    shrinkFilter->SetInput( IMask );
    unsigned int dfactors[3] = { shrink_factor, shrink_factor, 1};
    shrinkFilter->SetShrinkFactors(dfactors);
    shrinkFilter->UpdateLargestPossibleRegion();
    IMask = shrinkFilter->GetOutput();
    shrinkFilter->Update();
    
    
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    SliceIteratorType It( I, I->GetRequestedRegion() );
    SliceIteratorType It1( IMask, IMask->GetRequestedRegion() );
    It.SetFirstDirection( 0 );
    It.SetSecondDirection( 1 );
    It.GoToBegin();
    It1.SetFirstDirection( 0 );
    It1.SetSecondDirection( 1 );
    It1.GoToBegin();
    
    
    /* while( !It.IsAtEnd() )
     {
     while ( !It.IsAtEndOfSlice() )
     {
     while ( !It.IsAtEndOfLine() )
     {
     if( It1.Get() != 0 )
     It.Set(0);
     ++It;
     ++It1;
     }
     It.NextLine();
     It1.NextLine();
     }
     It.NextSlice();
     It1.NextSlice();
     } */
    
    mask_set = true;
    
    //extract the centroids
    typedef itk::ConnectedComponentImageFilter< ImageType, LabelImageType > ConnectedComponentType;
    ConnectedComponentType::Pointer connectedComponentFilter = ConnectedComponentType::New();
    connectedComponentFilter->SetInput( IMask );
    
    connectedComponentFilter->Update();
    
    
    // Relabel the components in order of size.
    typedef itk::RelabelComponentImageFilter< LabelImageType, LabelImageType > RelabelType;
    RelabelType::Pointer relabeler = RelabelType::New();
    relabeler->SetInput( connectedComponentFilter->GetOutput() );
    relabeler->SetMinimumObjectSize(300);
    
    relabeler->Update();
    
    
    typedef itk::LabelGeometryImageFilter< LabelImageType > LabelGeometryType;
    LabelGeometryType::Pointer labelGeometryFilter = LabelGeometryType::New();
    labelGeometryFilter->SetInput( relabeler->GetOutput() );
    
    labelGeometryFilter->Update();
    
    //  labelGeometryFilter->Update();
    
    ISoma = relabeler->GetOutput();
    
    LabelGeometryType::LabelPointType index_temp;
    int labelValue = labelGeometryFilter->GetNumberOfLabels()-1;
    
    num_soma = labelValue;
    
    //compute the centroids of soma
    Point3D temp;
    //PointList3D new_SeedPt(labelValue+1);
    Centroid.RemoveAllPts();
    for( int i = 1; i <= labelValue; i++)
    {
        index_temp = labelGeometryFilter->GetCentroid(i);
        soma_size.push_back(labelGeometryFilter->GetVolume(i));
        //float temp_radius = (labelGeometryFilter->GetMajorAxisLength(i) + labelGeometryFilter->GetMinorAxisLength(i))/4;
        float temp_radius = labelGeometryFilter->GetMinorAxisLength(i)/2;
        soma_radii.push_back(temp_radius);
        temp.x = index_temp[0];
        temp.y = index_temp[1];
        temp.z = index_temp[2];
        Centroid.AddPt(temp);
    }
    
    //compute the voronoi map
    typedef itk::DanielssonDistanceMapImageFilter< LabelImageType, LabelImageType > DistanceMapFilterType;
    DistanceMapFilterType::Pointer DMfilter = DistanceMapFilterType::New();
    DMfilter->SetInput( ISoma );
    DMfilter->Update();
    IVoronoi = DMfilter->GetVoronoiMap();
    
    /*//partition the space and convert to small images
     LabelGeometryType::Pointer labelGeometryFilter1 = LabelGeometryType::New();
     labelGeometryFilter1->SetInput( IVoronoi );
     labelGeometryFilter1->Update();
     
     int region_num = labelGeometryFilter1->GetNumberOfLabels();
     ImageType::RegionType region;
     ImageType::IndexType index;
     ImageType::SizeType size;
     for( int i = 1; i <= region_num; i++)
     {
     typedef  itk::FixedArray<float,6> FixedArrayType;
     FixedArrayType BBox = labelGeometryFilter1->GetBoundingBox(i);
     index[0] = BBox[0];
     index[1] = BBox[2];
     index[2] = BBox[4];
     size[0] = BBox[1]-BBox[0];
     size[1] = BBox[3]-BBox[2];
     size[2] = BBox[5]-BBox[4];
     
     region.SetSize(size);
     region.SetIndex(index);
     
     typedef itk::RegionOfInterestImageFilter< ImageType,
     ImageType > FilterType;
     
     FilterType::Pointer filter = FilterType::New();
     filter->SetInput(I);
     filter->SetRegionOfInterest(region);
     filter->Update();
     std::stringstream sstr;
     if( i < 10 )
     {
     sstr<< 00;
     sstr<< i;
     }
     else if( i < 100 )
     {
     sstr<< 0;
     sstr<< i;
     }
     else
     {
     sstr<<i;
     }
     
     sstr <<".tif";
     
     ImWrite(sstr.str().c_str(), filter->GetOutput());
     }*/
    
}

PointList3D ImageOperation::ImSomaCentroidExtraction()
{
    typedef itk::BinaryBallStructuringElement< int, 3 > StructuringElementType;
    StructuringElementType structuringElement;
    structuringElement.SetRadius( 3 ); // 5x5 structuring element
    structuringElement.CreateStructuringElement();
    
    typedef itk::GrayscaleErodeImageFilter<ImageType,ImageType,StructuringElementType > ErodeFilterType;
    ErodeFilterType::Pointer grayscaleErode = ErodeFilterType::New();
    grayscaleErode->SetKernel( structuringElement );
    grayscaleErode->SetInput( I );
    grayscaleErode->Update();
    
    //ImagePointer I_E_S = ImGaussian_XY(grayscaleErode->GetOutput(),5);
    
    //ImWrite("eroded_image.tif", grayscaleErode->GetOutput());
    
    typedef itk::OtsuThresholdImageFilter<
    ImageType, ImageType > FilterType;
    FilterType::Pointer thresholder = FilterType::New();
    thresholder->SetInput( grayscaleErode->GetOutput() );
    thresholder->SetOutsideValue( 1 );
    thresholder->SetInsideValue( 0 );
    thresholder->SetNumberOfHistogramBins( 256 );
    thresholder->Update();
    
    //ImWrite("binarized_image.tif", thresholder->GetOutput());
    
    //std::cout << "Threshold = " << thresholder->GetThreshold() << std::endl;
    
    /*typedef itk::BinaryThresholdImageFilter< ImageType,
     ImageType>    ThresholdingFilterType;
     
     ThresholdingFilterType::Pointer thresholder1 = ThresholdingFilterType::New();
     
     thresholder1->SetLowerThreshold( thresholder->GetThreshold() );
     thresholder1->SetOutsideValue(  0  );
     thresholder1->SetInsideValue(  1 );
     thresholder1->SetUpperThreshold( 255 );
     thresholder1->SetInput( grayscaleErode->GetOutput() );
     thresholder1->Update();*/
    
    
    //extract the centroids
    typedef itk::ConnectedComponentImageFilter< ImageType, LabelImageType > ConnectedComponentType;
    ConnectedComponentType::Pointer connectedComponentFilter = ConnectedComponentType::New();
    connectedComponentFilter->SetInput( thresholder->GetOutput() );
    
    // Relabel the components in order of size.
    typedef itk::RelabelComponentImageFilter< LabelImageType, LabelImageType > RelabelType;
    RelabelType::Pointer relabeler = RelabelType::New();
    relabeler->SetInput( connectedComponentFilter->GetOutput() );
    relabeler->SetMinimumObjectSize(50);
    
    typedef itk::LabelGeometryImageFilter< LabelImageType > LabelGeometryType;
    LabelGeometryType::Pointer labelGeometryFilter = LabelGeometryType::New();
    labelGeometryFilter->SetInput( relabeler->GetOutput() );
    
    labelGeometryFilter->Update();
    
    
    LabelGeometryType::LabelPointType index_temp;
    int labelValue = labelGeometryFilter->GetNumberOfLabels()-1;
    
    //compute the centroids of soma
    PointList3D soma_seeds;
    Point3D temp;
    //PointList3D new_SeedPt(labelValue+1);
    soma_seeds.RemoveAllPts();
    for( int i = 1; i <= labelValue; i++)
    {
        index_temp = labelGeometryFilter->GetCentroid(i);
        soma_size.push_back(labelGeometryFilter->GetVolume(i));
        temp.x = index_temp[0];
        temp.y = index_temp[1];
        temp.z = index_temp[2];
        soma_seeds.AddPt(temp);
    }
    return soma_seeds;
}

void ImageOperation::ImMasking(const char *filename, int shrink_factor)
{
    typedef itk::ImageFileReader<IOImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    
    reader->SetFileName(filename);
    IOImagePointer IIO = reader->GetOutput();
    reader->Update();
    
    typedef itk::CastImageFilter<IOImageType, ImageType> CasterType;
    
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(IIO);
    
    caster->Update();
    IMask = caster->GetOutput();
    
    
    typedef itk::ShrinkImageFilter< ImageType,ImageType > ShrinkFilterType;
    ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
    
    shrinkFilter->SetInput( IMask );
    unsigned int dfactors[3] = { shrink_factor, shrink_factor, 1};
    shrinkFilter->SetShrinkFactors(dfactors);
    shrinkFilter->UpdateLargestPossibleRegion();
    IMask = shrinkFilter->GetOutput();
    shrinkFilter->Update();
    
    
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    SliceIteratorType It( I, I->GetRequestedRegion() );
    SliceIteratorType It1( IMask, IMask->GetRequestedRegion() );
    It.SetFirstDirection( 0 );
    It.SetSecondDirection( 1 );
    It.GoToBegin();
    It1.SetFirstDirection( 0 );
    It1.SetSecondDirection( 1 );
    It1.GoToBegin();
    
    /* while( !It.IsAtEnd() )
     {
     while ( !It.IsAtEndOfSlice() )
     {
     while ( !It.IsAtEndOfLine() )
     {
     if( It1.Get() != 0 )
     It.Set(0);
     ++It;
     ++It1;
     }
     It.NextLine();
     It1.NextLine();
     }
     It.NextSlice();
     It1.NextSlice();
     } */
    
    mask_set = true;
    
    //extract the centroids
    typedef itk::ConnectedComponentImageFilter< ImageType, LabelImageType > ConnectedComponentType;
    ConnectedComponentType::Pointer connectedComponentFilter = ConnectedComponentType::New();
    connectedComponentFilter->SetInput( IMask );
    
    // Relabel the components in order of size.
    typedef itk::RelabelComponentImageFilter< LabelImageType, LabelImageType > RelabelType;
    RelabelType::Pointer relabeler = RelabelType::New();
    relabeler->SetInput( connectedComponentFilter->GetOutput() );
    
    
    typedef itk::LabelGeometryImageFilter< LabelImageType > LabelGeometryType;
    LabelGeometryType::Pointer labelGeometryFilter = LabelGeometryType::New();
    labelGeometryFilter->SetInput( relabeler->GetOutput() );
    
    labelGeometryFilter->Update();
    
    ISoma = relabeler->GetOutput();
    
    LabelGeometryType::LabelPointType index_temp;
    int labelValue = labelGeometryFilter->GetNumberOfLabels()-1;
    
    num_soma = labelValue;
    
    //compute the centroids of soma
    Point3D temp;
    //PointList3D new_SeedPt(labelValue+1);
    Centroid.RemoveAllPts();
    for( int i = 1; i <= labelValue; i++)
    {
        index_temp = labelGeometryFilter->GetCentroid(i);
        soma_size.push_back(labelGeometryFilter->GetVolume(i));
        temp.x = index_temp[0];
        temp.y = index_temp[1];
        temp.z = index_temp[2];
        Centroid.AddPt(temp);
    }
    
    //compute the voronoi map
    typedef itk::DanielssonDistanceMapImageFilter< LabelImageType, LabelImageType > DistanceMapFilterType;
    DistanceMapFilterType::Pointer DMfilter = DistanceMapFilterType::New();
    DMfilter->SetInput( ISoma );
    DMfilter->Update();
    IVoronoi = DMfilter->GetVoronoiMap();
    
}

IOImagePointer ImageOperation::ImInvert(IOImagePointer In)
{
    std::cout<<"inverting image..."<<std::endl;
    typedef itk::ImageSliceIteratorWithIndex< IOImageType > SliceIteratorType;
    SliceIteratorType It( In, In->GetRequestedRegion() );
    It.SetFirstDirection( 0 );
    It.SetSecondDirection( 1 );
    It.GoToBegin();
    
    while( !It.IsAtEnd() )
    {
        while ( !It.IsAtEndOfSlice() )
        {
            while ( !It.IsAtEndOfLine() )
            {
                It.Set( 255 - It.Get() );
                ++It;
            }
            It.NextLine();
        }
        It.NextSlice();
    }
    
    return In;
}


void ImageOperation::ImShrink(int shrink_factor)
{
    typedef itk::ShrinkImageFilter< ImageType,ImageType > ShrinkFilterType;
    ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
    
    shrinkFilter->SetInput( I );
    unsigned int dfactors[3] = { shrink_factor, shrink_factor, 1};
    shrinkFilter->SetShrinkFactors(dfactors);
    shrinkFilter->UpdateLargestPossibleRegion();
    I = shrinkFilter->GetOutput();
    shrinkFilter->Update();
    
    SM = I->GetLargestPossibleRegion().GetSize()[0];
    SN = I->GetLargestPossibleRegion().GetSize()[1];
    SZ = I->GetLargestPossibleRegion().GetSize()[2];
}

IOImagePointer ImageOperation::ImShrink(IOImagePointer In, int shrink_factor)
{
    typedef itk::ShrinkImageFilter< IOImageType,IOImageType > ShrinkFilterType;
    ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
    
    shrinkFilter->SetInput( In );
    unsigned int dfactors[3] = { shrink_factor, shrink_factor, 1};
    shrinkFilter->SetShrinkFactors(dfactors);
    shrinkFilter->UpdateLargestPossibleRegion();
    In = shrinkFilter->GetOutput();
    shrinkFilter->Update();
    
    return In;
}

void ImageOperation::ImRemove_RedSeeds(PointList3D Cu, std::vector<float> Ru)
{
    int expand_ratio = 1;
    typedef itk::VesselTubeSpatialObject<3> VesselTubeType;
    typedef itk::VesselTubeSpatialObjectPoint<3> VesselTubePointType;
    VesselTubeType::Pointer VesselTube = VesselTubeType::New();
    VesselTubeType::PointListType list;
    for( int i=0; i<Cu.NP; i++)
    {
        VesselTubePointType p;
        p.SetPosition(Cu.Pt[i].x, Cu.Pt[i].y, Cu.Pt[i].z);
        p.SetRadius(Ru[i] * expand_ratio);
        list.push_back(p);
    }
    VesselTube->GetProperty()->SetName("VesselTube");
    VesselTube->SetId(1);
    VesselTube->SetPoints(list);
    
    typedef itk::Point< float, 3 > PointType;
    
    for( int i = 0; i < SeedPt.NP; i++ )
    {
        if( visit_label(i) == 0 )
        {
            PointType p;
            p[0] = SeedPt.Pt[i].x;
            p[1] = SeedPt.Pt[i].y;
            p[2] = SeedPt.Pt[i].z;
            if( VesselTube->IsInside(p) )
            {
                visit_label(i) = 1;
            }
        }
    }
}

void ImageOperation::ImRefresh_TracingImage()
{
    typedef itk::ImageSliceIteratorWithIndex< LabelImageType > SliceIteratorType;
    SliceIteratorType It( IL_Tracing, IL_Tracing->GetRequestedRegion() );
    It.SetFirstDirection( 0 );
    It.SetSecondDirection( 1 );
    It.GoToBegin();
    
    while( !It.IsAtEnd() )
    {
        while ( !It.IsAtEndOfSlice() )
        {
            while ( !It.IsAtEndOfLine() )
            {
                It.Set( 0 );
                ++It;
            }
            It.NextLine();
        }
        It.NextSlice();
    }
}

void ImageOperation::ImRefresh_LabelImage()
{
    typedef itk::CastImageFilter<ImageType, LabelImageType> CasterType1;
    CasterType1::Pointer caster1 = CasterType1::New();
    CasterType1::Pointer caster2 = CasterType1::New();
    caster1->SetInput(I);
    caster2->SetInput(I);
    IL = caster1->GetOutput();
    caster1->Update();
    IL_Tracing = caster2->GetOutput();
    caster2->Update();
    
    typedef itk::ImageSliceIteratorWithIndex< LabelImageType > SliceIteratorType;
    SliceIteratorType It1( IL, IL->GetRequestedRegion() );
    SliceIteratorType It2( IL_Tracing, IL_Tracing->GetRequestedRegion() );
    It1.SetFirstDirection( 0 );
    It1.SetSecondDirection( 1 );
    It1.GoToBegin();
    It2.SetFirstDirection( 0 );
    It2.SetSecondDirection( 1 );
    It2.GoToBegin();
    while( !It1.IsAtEnd() )
    {
        while ( !It1.IsAtEndOfSlice() )
        {
            while ( !It1.IsAtEndOfLine() )
            {
                It1.Set( 0 );
                It2.Set( 0 );
                ++It1;
                ++It2;
            }
            It1.NextLine();
            It2.NextLine();
        }
        It1.NextSlice();
        It2.NextSlice();
    }
}

ImagePointer ImageOperation::ImRescale(ProbImagePointer IInput)
{
    typedef itk::RescaleIntensityImageFilter< ProbImageType, ImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    
    rescale->SetInput( IInput );
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 255 );
    rescale->Update();
    ImagePointer output_image = rescale->GetOutput();
    
    return output_image;
}

ImagePointer ImageOperation::ImRescale(LabelImagePointer IInput)
{
    typedef itk::RescaleIntensityImageFilter< LabelImageType, ImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    
    rescale->SetInput( IInput );
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 255 );
    rescale->Update();
    ImagePointer output_image = rescale->GetOutput();
    
    return output_image;
}

ImagePointer ImageOperation::ImRescale(ImagePointer IInput)
{
    typedef itk::RescaleIntensityImageFilter< ImageType, ImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    rescale->SetInput( IInput );
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 255 );
    rescale->Update();
    return rescale->GetOutput();
}

void ImageOperation::computeGVF(int noise_level, int num_iteration, int smoothing_scale)
{
    if( smoothing_scale == 0 )
    {
        typedef itk::GradientImageFilter<ProbImageType, float, float> GradientImageFilterType;
        GradientImageFilterType::Pointer gradientFilter = GradientImageFilterType::New();

		itk::TimeProbe clock;
		clock.Start();
        typedef itk::RescaleIntensityImageFilter< ImageType, ProbImageType> RescaleFilterType;
        RescaleFilterType::Pointer rescale = RescaleFilterType::New();
        rescale->SetInput( I );
        rescale->SetOutputMinimum( 0 );
        rescale->SetOutputMaximum( 1 );
        rescale->Update();
        
		clock.Stop();
        //std::cout << "Rescale Total: " << clock.GetTotal() << std::endl;

		clock.Start();
        gradientFilter->SetInput(rescale->GetOutput());
        //gradientFilter->SetInput(I);
        
        try
        {
            gradientFilter->Update();
			clock.Stop();
        //	std::cout << "Gradient Total: " << clock.GetTotal() << std::endl;
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "Exception caught: " << err << std::endl;
        }
        
        //IG = gradientFilter->GetOutput();
        if( num_iteration == 0 )
        {
            IGVF = gradientFilter->GetOutput();
        }
        else
        {
			clock.Start();
            typedef itk::GradientVectorFlowImageFilter<GradientImageType, GradientImageType> GradientVectorFlowFilterType;
            GradientVectorFlowFilterType::Pointer GVFFilter = GradientVectorFlowFilterType::New();
            
            GVFFilter->SetInput(gradientFilter->GetOutput());
            GVFFilter->SetNoiseLevel(noise_level);
            GVFFilter->SetIterationNum(num_iteration);
            
            try
            {
                GVFFilter->Update();
				clock.Stop();
            //	std::cout << "Gradient Vector Flow Total: " << clock.GetTotal() << std::endl;
            }
            catch( itk::ExceptionObject & err )
            {
                std::cerr << "Exception caught: " << err << std::endl;
            }
            IGVF = GVFFilter->GetOutput();
        }
        
    }
    else
    {
        typedef itk::GradientRecursiveGaussianImageFilter<ProbImageType, GradientImageType> GradientImageFilterType;
        GradientImageFilterType::Pointer gradientFilter = GradientImageFilterType::New();
        
        typedef itk::RescaleIntensityImageFilter< ImageType, ProbImageType> RescaleFilterType;
        RescaleFilterType::Pointer rescale = RescaleFilterType::New();
        rescale->SetInput( I );
        rescale->SetOutputMinimum( 0 );
        rescale->SetOutputMaximum( 1 );
        rescale->Update();
        
        gradientFilter->SetSigma(smoothing_scale);
        gradientFilter->SetInput(rescale->GetOutput());
        
        try
        {
            gradientFilter->Update();
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "Exception caught: " << err << std::endl;
        }
        
        //IG = gradientFilter->GetOutput();
        
        typedef itk::GradientVectorFlowImageFilter<GradientImageType, GradientImageType> GradientVectorFlowFilterType;
        GradientVectorFlowFilterType::Pointer GVFFilter = GradientVectorFlowFilterType::New();
        
        GVFFilter->SetInput(gradientFilter->GetOutput());
        GVFFilter->SetNoiseLevel(noise_level);
        GVFFilter->SetIterationNum(num_iteration);
        
        try
        {
            GVFFilter->Update();
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "Exception caught: " << err << std::endl;
        }
        IGVF = GVFFilter->GetOutput();
    }
    
}

void ImageOperation::normalizeGVF()
{
    typedef itk::ImageSliceIteratorWithIndex< GradientImageType > SliceIteratorType;
    SliceIteratorType inputIt( IGVF, IGVF->GetRequestedRegion() );
    inputIt.SetFirstDirection( 0 );
    inputIt.SetSecondDirection( 1 );
    inputIt.GoToBegin();
    while( !inputIt.IsAtEnd() )
    {
        while ( !inputIt.IsAtEndOfSlice() )
        {
            while ( !inputIt.IsAtEndOfLine() )
            {
                if(inputIt.Get().GetNorm() != 0)
                {
                    inputIt.Set( inputIt.Get()/( inputIt.Get().GetNorm() + std::numeric_limits<float>::epsilon() ) );
                }
                ++inputIt;
            }
            inputIt.NextLine();
        }
        inputIt.NextSlice();
    }
    
    /* typedef itk::ImageDuplicator< GradientImageType > DuplicatorType;
     DuplicatorType::Pointer Duplicator = DuplicatorType::New();
     Duplicator->SetInputImage(IGVF);
     Duplicator->Update();
     IGVF_Norm = Duplicator->GetOutput();
     
     
     typedef itk::CastImageFilter<ImageType,ProbImageType> CasterType;
     CasterType::Pointer caster = CasterType::New();
     
     caster->SetInput(I);
     caster->Update();
     GVF_Mag = caster->GetOutput();
     
     
     typedef itk::ImageSliceIteratorWithIndex< GradientImageType > SliceIteratorType;
     typedef itk::ImageSliceIteratorWithIndex< ProbImageType > SliceIteratorType1;
     
     SliceIteratorType inputIt( IGVF, IGVF->GetRequestedRegion() );
     SliceIteratorType outputIt( IGVF_Norm, IGVF_Norm->GetRequestedRegion() );
     SliceIteratorType1 outputIt1( GVF_Mag, GVF_Mag->GetRequestedRegion() );
     inputIt.SetFirstDirection( 0 );
     inputIt.SetSecondDirection( 1 );
     outputIt.SetFirstDirection( 0 );
     outputIt.SetSecondDirection( 1 );
     outputIt1.SetFirstDirection( 0 );
     outputIt1.SetSecondDirection( 1 );
     
     inputIt.GoToBegin();
     outputIt.GoToBegin();
     outputIt1.GoToBegin();
     
     while( !inputIt.IsAtEnd() )
     {
     while ( !inputIt.IsAtEndOfSlice() )
     {
     while ( !inputIt.IsAtEndOfLine() )
     {
	 if(inputIt.Get().GetNorm() == 0)
	 {
     outputIt.Set( inputIt.Get());
	 }
	 else
	 {
     outputIt.Set( inputIt.Get()/inputIt.Get().GetNorm() );
	 }
     
     
     outputIt1.Set( inputIt.Get().GetNorm() );
     ++inputIt;
     ++outputIt;
	 ++outputIt1;
     }
     inputIt.NextLine();
     outputIt.NextLine();
     outputIt1.NextLine();
     }
     inputIt.NextSlice();
     outputIt.NextSlice();
     outputIt1.NextSlice();
     } */
}

void ImageOperation::SeedAdjustment(int iter_num)
{
    
    normalizeGVF();
    
    if( iter_num == 0 )
    {
        //sort seeds by their saliency
        PointList3D New_SeedPt;
        
        vnl_vector<double> saliency(SeedPt.NP);
        for( int i = 0; i < SeedPt.NP; i++)
        {
            GradientImageType::IndexType index;
            SeedPt.Pt[i].check_out_of_range_3D(SM,SN,SZ);
            index[0] = ceil(SeedPt.Pt[i].x);
            index[1] = ceil(SeedPt.Pt[i].y);
            index[2] = ceil(SeedPt.Pt[i].z);
            saliency(i) = IVessel->GetPixel(index);
        }
        
        for( unsigned int i = 0; i < saliency.size(); i++)
        {
            int index = saliency.arg_max();
            New_SeedPt.AddPt(SeedPt.Pt[index]);
            saliency(index) = -1;
        }
        
        SeedPt = New_SeedPt;
        
        visit_label.set_size(SeedPt.GetSize());
        visit_label.fill(0);
        return;
    }
    
    //int iter_num = 100;
    
    typedef itk::VectorLinearInterpolateImageFunction<
    GradientImageType, float >  GradientInterpolatorType;
    
    GradientInterpolatorType::Pointer interpolator = GradientInterpolatorType::New();
    interpolator->SetInputImage(IGVF);
    
    
    //move seeds along gradient vector flow
    int j = 0;
    
    Point3D temp_pt;
    
    while( j < iter_num )
    {
        for( int i = 0; i < SeedPt.NP; i++ )
        {
            GradientImageType::IndexType index;
            // std::cout<<SeedPt.Pt[i].x<<","<<SeedPt.Pt[i].y<<","<<SeedPt.Pt[i].z<<std::endl;
            SeedPt.Pt[i].check_out_of_range_3D(SM,SN,SZ);
            index[0] = (SeedPt.Pt[i].x);
            index[1] = (SeedPt.Pt[i].y);
            index[2] = (SeedPt.Pt[i].z);
		    //std::cout<<index[0]<<","<<index[1]<<","<<index[2]<<std::endl;
            GradientPixelType gradient = interpolator->EvaluateAtIndex(index);
            //std::cout<<"check point 20"<<std::endl;
            //GradientPixelType gradient = IGVF->GetPixel(index);
            //std::cout<<gradient[0]<<","<<gradient[1]<<","<<gradient[2]<<std::endl;
            //Point3D temp_pt(gradient[0],gradient[1],gradient[2]);
            temp_pt.x = gradient[0];
            temp_pt.y = gradient[1];
            temp_pt.z = gradient[2];
            SeedPt.Pt[i] = SeedPt.Pt[i] + temp_pt;
        }
        j++;
    }
    
    /*//filter out seeds in the background
     int i = 0;
     while( i < SeedPt.NP )
     {
     ImageType::IndexType index;
     
     SeedPt.Pt[i].check_out_of_range_3D(SM,SN,SZ);
     
     index[0] = ceil(SeedPt.Pt[i].x);
     index[1] = ceil(SeedPt.Pt[i].y);
     index[2] = ceil(SeedPt.Pt[i].z);
     
     if( VBW->GetPixel(index) == 0)
     {
     SeedPt.RemovePt(i);
     continue;
     }
     i++;
     }*/
    
    
    seed_centroid();
    
    //sort seeds by their saliency
    PointList3D New_SeedPt;
    
    vnl_vector<double> saliency(SeedPt.NP);
    unsigned long int number_of_out_of_range=0;///
    //std::cout<<"Number of seeds:"<<SeedPt.NP<<std::endl;///
    for( unsigned long int i = 0; i < SeedPt.NP; i++)///
    {
        GradientImageType::IndexType index;
        SeedPt.Pt[i].check_out_of_range_3D(SM,SN,SZ);
        if (SeedPt.Pt[i].check_out_of_range_3D(SM,SN,SZ))
            number_of_out_of_range++;
        //std::cout<<"Out of Range"<<std::endl;
        index[0] = ceil(SeedPt.Pt[i].x);
        index[1] = ceil(SeedPt.Pt[i].y);
        index[2] = ceil(SeedPt.Pt[i].z);
        saliency(i) = IVessel->GetPixel(index);
    }
    //std::cout<<"After for on SeedPt.NP"<<std::endl;
    //std::cout<<"Number of out of range:"<<number_of_out_of_range<<std::endl;
    
    for( unsigned long int i = 0; i < saliency.size(); i++)////
    {
        unsigned long int index = saliency.arg_max();
        New_SeedPt.AddPt(SeedPt.Pt[index]);
        saliency(index) = -1;
    }
    //std::cout<<"After for on saliency size"<<std::endl;///
    
    SeedPt = New_SeedPt;
    
	visit_label.set_size(SeedPt.GetSize());
    //std::cout<<"After set size"<<std::endl;///
    
	visit_label.fill(0);
    
	//SeedPt_mg = SeedPt;
    //std::cout<<"Detected Seed Points:"<<SeedPt.NP<<std::endl;
}

void ImageOperation::OutputSeeds()
{
	//Make a unsigned char image to print out the critical points image
	typedef itk::Image< unsigned char, 3 > CriticalPointsImageType;
	CriticalPointsImageType::Pointer critical_point_image = CriticalPointsImageType::New();
	critical_point_image->SetRegions(I->GetLargestPossibleRegion());
	critical_point_image->Allocate();
	critical_point_image->FillBuffer(0);

	for( unsigned long int i = 0; i < SeedPt.NP; i++)
	{
		itk::Index<3> index;
		index[0] = SeedPt.Pt[i].x;
		index[1] = SeedPt.Pt[i].y;
		index[2] = SeedPt.Pt[i].z;
		for(int k = -5; k <= 5; k++)
		{
			itk::Index<3> indexX;
			itk::Index<3> indexY;

			indexX[0] = index[0] + k;
			indexX[1] = index[1];
			indexX[2] = index[2];
			if( indexX [0] >= 0 && indexX [0] <= SM)
			{
				critical_point_image->SetPixel(indexX, 255);
			}

			indexY[0] = index[0];
			indexY[1] = index[1] + k;
			indexY[2] = index[2];
			if( indexY [1] >= 0 && indexY [1] <= SN)
			{
				critical_point_image->SetPixel(indexY, 255);
			}
		}
	}

	typedef itk::ImageFileWriter< CriticalPointsImageType > CriticalPointsWriterType;
	CriticalPointsWriterType::Pointer crit_pts_writer = CriticalPointsWriterType::New();
	crit_pts_writer->SetInput(critical_point_image);
	crit_pts_writer->SetFileName("critical_point_image.tif");
	crit_pts_writer->Update();
}

void ImageOperation::SeedDetection(float th, int detection_method, int seed_radius)
{
	std::cout<< "Entering seed detection function."<<std::endl;
    SM = I->GetLargestPossibleRegion().GetSize()[0];
    SN = I->GetLargestPossibleRegion().GetSize()[1];
    SZ = I->GetLargestPossibleRegion().GetSize()[2];
    
    SeedType seed;
    SamplePointer seeds_candidate = SampleType::New();
    seeds_candidate->SetMeasurementVectorSize( 3 );
    
    //int max_num_seed = 100000;
    //SeedPt.SetN(max_num_seed);
    
    //int remove_radius = 5;
    
    Point3D temp_pt;
    
    int rad = 1;
    
    typedef itk::CastImageFilter<ImageType,LabelImageType> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(I);
    caster->Update();
    LabelImagePointer I_Seed = caster->GetOutput();
    
    typedef itk::NeighborhoodIterator< ProbImageType > NeighborhoodIteratorType;
    typedef itk::NeighborhoodIterator< ImageType > NeighborhoodIteratorType1;
    typedef itk::NeighborhoodIterator< GradientImageType > NeighborhoodIteratorType2;
    
    NeighborhoodIteratorType::RadiusType radius;
    NeighborhoodIteratorType1::RadiusType radius1;
    NeighborhoodIteratorType2::RadiusType radius2;
    radius.Fill(rad);
    radius1.Fill(rad);
    radius2.Fill(rad);
    
    NeighborhoodIteratorType it( radius, IVessel, IVessel->GetRequestedRegion());
    //NeighborhoodIteratorType2 it21( radius2, IGVF, IGVF->GetRequestedRegion());
    //NeighborhoodIteratorType2 it22( radius2, V1, V1->GetRequestedRegion());
    //std::cout<<"SeedDetection-Threshold:"<<th<<std::endl;///
    //std::cout<<"SeedDetection-Seed Radius:"<<seed_radius<<std::endl;///
    
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
        /*if( detection_method == 0 ) //skeleton seeds
         {
         if(it1.GetCenterPixel() == 1)
         {
         //SeedPt.AddPt(it.GetIndex()[0],it.GetIndex()[1],it.GetIndex()[2]);
         seed[0] = it.GetIndex()[0];
         seed[1] = it.GetIndex()[1];
         seed[2] = it.GetIndex()[2];
         if( !SeedSparsify( seeds_candidate, seed, seed_radius ) )
         seeds_candidate->PushBack( seed );
         }
         }*/
        
        if(it.GetCenterPixel() >= th )
            //if( it.GetCenterPixel() >= th )
        {
            //SeedPt.AddPt(it.GetIndex()[0],it.GetIndex()[1],it.GetIndex()[2]);
            seed[0] = it.GetIndex()[0];
            seed[1] = it.GetIndex()[1];
            seed[2] = it.GetIndex()[2];
            if( seed_radius != 0 )
            {
                if( !SeedSparsify( seeds_candidate, seed, seed_radius ) )
                    seeds_candidate->PushBack( seed );
            }
            else
            {
                seeds_candidate->PushBack( seed );
            }
        }
        
    }
    
    //SeedPt.SetN( seeds_candidate->Size() );
    SeedPt.RemoveAllPts();
    for( unsigned int i = 0; i < seeds_candidate->Size(); i++ )
	{
		SeedPt.AddPt( seeds_candidate->GetMeasurementVector(i)[0], seeds_candidate->GetMeasurementVector(i)[1], seeds_candidate->GetMeasurementVector(i)[2] );
	}
    
	visit_label.set_size(SeedPt.GetSize());
	visit_label.fill(0);
    
	//SeedPt_mg = SeedPt;
    std::cout<<"Detected Seed Points:"<<SeedPt.NP<<std::endl;///
    
}

bool ImageOperation::SeedSparsify(SamplePointer seeds_candidate, SeedType query_point, int radius)
{
    bool removal = false;
    Point3D temp_pt1, temp_pt2;
	temp_pt1.x = query_point[0];
    temp_pt1.y = query_point[1];
	temp_pt1.z = query_point[2];
    
	for( unsigned int i = 0; i < seeds_candidate->Size(); i++ )
	{
        SeedType seed;
        seed = seeds_candidate->GetMeasurementVector(i);
        temp_pt2.x = seed[0];
        temp_pt2.y = seed[1];
        temp_pt2.z = seed[2];
        if( temp_pt1.GetDistTo(temp_pt2) < radius )
        {
            removal = true;
            break;
        }
	}
    
	/*typedef itk::Statistics::KdTreeGenerator< SampleType > TreeGeneratorType;
     TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();
     
     treeGenerator->SetSample( seeds_candidate );
     treeGenerator->SetBucketSize( 16 );
     treeGenerator->Update();
     
     typedef TreeGeneratorType::KdTreeType TreeType;
     TreeType::Pointer tree = treeGenerator->GetOutput();
     
     TreeType::InstanceIdentifierVectorType neighbors;
     tree->Search( query_point, (double)radius, neighbors );
     
     if( neighbors.size() > 0 )
     {
     removal = true;
     } */
	return removal;
}

void ImageOperation::seed_centroid()
{
    typedef itk::CastImageFilter<ImageType,LabelImageType> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(I);
    caster->Update();
    LabelImagePointer I_Seed = caster->GetOutput();
    
    typedef itk::ImageSliceIteratorWithIndex< LabelImageType > SliceIteratorType;
    SliceIteratorType inputIt( I_Seed, I_Seed->GetRequestedRegion() );
    inputIt.SetFirstDirection( 0 );
    inputIt.SetSecondDirection( 1 );
    inputIt.GoToBegin();
    while( !inputIt.IsAtEnd() )
    {
        while ( !inputIt.IsAtEndOfSlice() )
        {
            while ( !inputIt.IsAtEndOfLine() )
            {
                inputIt.Set(0);
                ++inputIt;
            }
            inputIt.NextLine();
        }
        inputIt.NextSlice();
    }
    
    for( int i = 0; i < SeedPt.GetSize(); i++ )
    {
        LabelImageType::IndexType index;
        SeedPt.Pt[i].check_out_of_range_3D(SM,SN,SZ);
        /*for( int j = -1; j <= 1; j++ )
         {
         for( int k = -1; k <= 1; k++ )
         {
         for( int z = -1; z <= 1; z++)
         {
         index[0] = SeedPt.Pt[i].x + j;
         index[1] = SeedPt.Pt[i].y + k;
         index[2] = SeedPt.Pt[i].z + z;
         
         if( index[0] < 0 || index[1] < 0 || index[2] < 0 ||
         index[0] >= SM || index[1] >= SN || index[2] >= SZ )
         continue;
         
         I_Seed->SetPixel(index, 1);
         }
         }
         }*/
        
        index[0] = SeedPt.Pt[i].x;
        index[1] = SeedPt.Pt[i].y;
        index[2] = SeedPt.Pt[i].z;
        I_Seed->SetPixel(index,1);
    }
    
    // Set up a connected components filter to label the binary objects.
    typedef itk::ConnectedComponentImageFilter< LabelImageType, LabelImageTypeCCIF > ConnectedComponentType;//////////////////
    ConnectedComponentType::Pointer connectedComponentFilter = ConnectedComponentType::New();
    
    
    try
    {
        //std::cout<<"Before connectedComponentFilter->SetInput()"<< std::endl;
        connectedComponentFilter->SetInput( I_Seed );
    }
    catch( itk::ExceptionObject & err )
    {
        std::cerr << "ExceptionObject caught at connectedComponentFilter GetOutput!" << std::endl;
        std::cerr << err << std::endl;
    }
    
	
    // Relabel the components in order of size.
    typedef itk::RelabelComponentImageFilter< LabelImageTypeCCIF, LabelImageTypeCCIF > RelabelType;///////////
    RelabelType::Pointer relabeler = RelabelType::New();
    
    try
    {
        //std::cout<<"Before relaberer->GetOutput()"<< std::endl;
        relabeler->SetInput( connectedComponentFilter->GetOutput() );
    }
    catch( itk::ExceptionObject & err )
    {
        std::cerr << "ExceptionObject caught at relaberer GetOutput!" << std::endl;
        std::cerr << err << std::endl;
    }
    
    
    
    typedef itk::LabelGeometryImageFilter< LabelImageTypeCCIF > LabelGeometryType;///////////////////
    LabelGeometryType::Pointer labelGeometryFilter = LabelGeometryType::New();
    
    try
    {
        //std::cout<<"Before labelGeometryFilter->GetOutput()"<< std::endl;
        labelGeometryFilter->SetInput( relabeler->GetOutput() );
    }
    catch( itk::ExceptionObject & err )
    {
        std::cerr << "ExceptionObject caught at labelGeometryFilter GetOutput!" << std::endl;
        std::cerr << err << std::endl;
    }
    
    try
    {
        //std::cout<<"Before labelGeometryFilter->Update()"<< std::endl;
        labelGeometryFilter->Update();
    }
    catch( itk::ExceptionObject & err )
    {
        std::cerr << "ExceptionObject caught at labelGeometryFilter GetOutput!" << std::endl;
        std::cerr << err << std::endl;
    }
    
    //std::cout<<"After labelGeometryFilter->Update()"<< std::endl;
    
    
    LabelGeometryType::LabelPointType index_temp;
    int labelValue = labelGeometryFilter->GetNumberOfLabels()-1;
    
    //std::cout<<"After labelGeometryFilter->GetNumberofLabels()"<< std::endl;
    
    Point3D temp;
    //PointList3D new_SeedPt(labelValue+1);
    SeedPt.RemoveAllPts();
    for( int i = 1; i <= labelValue; i++)
    {
        index_temp = labelGeometryFilter->GetCentroid(i);
        temp.x = index_temp[0];
        temp.y = index_temp[1];
        temp.z = index_temp[2];
        SeedPt.AddPt(temp);
    }
    std::cout<<"Seed Centroid End"<< std::endl;
    
}

void ImageOperation::SetCodingMethod(int in)
{
    coding_method = in + 1;
}

void ImageOperation::ImCoding( PointList3D Cu, std::vector<float> Ru, int snake_id, bool tracing )
{
    
    int coding_radius = 1;
    //int coding_method = 2;
    //std::cout<<"coding_method"<<coding_method<<std::endl;
    
    typedef itk::VesselTubeSpatialObject<3> VesselTubeType;
    typedef itk::VesselTubeSpatialObjectPoint<3> VesselTubePointType;
    VesselTubeType::Pointer VesselTube = VesselTubeType::New();
    VesselTubeType::PointListType list;
    
    
    int min_x, min_y, min_z, max_x, max_y, max_z;
    min_x = min_y = min_z = max_x = max_y = max_z = 0;
    
    /* if( coding_method == 2 )
     {
     if( tracing )
	 {
     for( int i=3; i<Cu.NP-3; i++)
     {
     VesselTubePointType p;
     p.SetPosition(Cu.Pt[i].x, Cu.Pt[i].y, Cu.Pt[i].z);
     p.SetRadius(Ru[i]);
     list.push_back(p);
     }
     VesselTube->GetProperty()->SetName("VesselTube");
     VesselTube->SetId(1);
     VesselTube->SetPoints(list);
	 }
	 else
	 {
     for( int i=0; i<Cu.NP; i++)
     {
     VesselTubePointType p;
     p.SetPosition(Cu.Pt[i].x, Cu.Pt[i].y, Cu.Pt[i].z);
     p.SetRadius(Ru[i]);
     list.push_back(p);
     
     if( min_x > Cu.Pt[i].x)
     min_x = Cu.Pt[i].x;
     if( min_y > Cu.Pt[i].y)
     min_y = Cu.Pt[i].y;
     if( min_z > Cu.Pt[i].z)
     min_z = Cu.Pt[i].z;
     if( max_x < Cu.Pt[i].x)
     max_x = Cu.Pt[i].x;
     if( max_y < Cu.Pt[i].y)
     max_y = Cu.Pt[i].y;
     if( max_z < Cu.Pt[i].z)
     max_z = Cu.Pt[i].z;
     }
     VesselTube->GetProperty()->SetName("VesselTube");
     VesselTube->SetId(1);
     VesselTube->SetPoints(list);
	 }
     } */
    
    
    if( tracing )
    {
        if( Cu.NP <= 5 )
        {
            return;
        }
        
        if( 1 )
        {
            for( int i = 3; i < Cu.GetSize() - 3; i++ )
            {
                LabelImageType::IndexType index;
                index[0] = Cu.Pt[i].x;
                index[1] = Cu.Pt[i].y;
                index[2] = Cu.Pt[i].z;
                //IL->SetPixel( index, 1 );
                for( int ix = -coding_radius; ix <= coding_radius; ix++ )
                {
                    for( int iy = -coding_radius; iy <= coding_radius; iy++ )
                    {
                        for( int iz = -coding_radius; iz <= coding_radius; iz++ )
                        {
                            LabelImageType::IndexType new_index;
                            Point3D temp_pt;
                            temp_pt.x = Cu.Pt[i].x + ix;
                            temp_pt.y = Cu.Pt[i].y + iy;
                            temp_pt.z = Cu.Pt[i].z + iz;
                            temp_pt.check_out_of_range_3D(SM,SN,SZ);
                            new_index[0] = temp_pt.x;
                            new_index[1] = temp_pt.y;
                            new_index[2] = temp_pt.z;
                            IL_Tracing->SetPixel( new_index, 1 );
                        }
                    }
                }
            }
        }
        /*else
         {
         typedef itk::Point< float, 3 > PointType;
         typedef itk::ImageSliceIteratorWithIndex< LabelImageType > SliceIteratorType;
         SliceIteratorType It( IL_Tracing, IL_Tracing->GetRequestedRegion() );
         It.SetFirstDirection( 0 );
         It.SetSecondDirection( 1 );
         It.GoToBegin();
         while( !It.IsAtEnd() )
         {
         while ( !It.IsAtEndOfSlice() )
         {
         while ( !It.IsAtEndOfLine() )
         {
         PointType p;
         
         p[0] = It.GetIndex()[0];
         p[1] = It.GetIndex()[1];
         p[2] = It.GetIndex()[2];
         if( VesselTube->IsInside(p) )
         It.Set(1);
         
         ++It;
         }
         It.NextLine();
         }
         It.NextSlice();
         }
         }*/
    }
    else
    {
        if( coding_method == 1 )
        {
            for( int i = 0; i < Cu.GetSize(); i++ )
            {
                LabelImageType::IndexType index;
                index[0] = Cu.Pt[i].x;
                index[1] = Cu.Pt[i].y;
                index[2] = Cu.Pt[i].z;
                //IL->SetPixel( index, 1 );
                for( int ix = -coding_radius; ix <=coding_radius; ix++ )
                {
                    for( int iy = -coding_radius; iy <=coding_radius; iy++ )
                    {
                        for( int iz = -coding_radius; iz <=coding_radius; iz++ )
                        {
                            LabelImageType::IndexType new_index;
                            Point3D temp_pt;
                            temp_pt.x = Cu.Pt[i].x + ix;
                            temp_pt.y = Cu.Pt[i].y + iy;
                            temp_pt.z = Cu.Pt[i].z + iz;
                            temp_pt.check_out_of_range_3D(SM,SN,SZ);
                            new_index[0] = temp_pt.x;
                            new_index[1] = temp_pt.y;
                            new_index[2] = temp_pt.z;
                            IL->SetPixel( new_index, snake_id );
                        }
                    }
                }
            }
        }
        else
        {
            for( int i = 0; i < Cu.GetSize(); i++ )
            {
                LabelImageType::IndexType index;
                index[0] = Cu.Pt[i].x;
                index[1] = Cu.Pt[i].y;
                index[2] = Cu.Pt[i].z;
                //IL->SetPixel( index, 1 );
                for( int ix = -Ru[i]; ix <=Ru[i]; ix++ )
                {
                    for( int iy = -Ru[i]; iy <=Ru[i]; iy++ )
                    {
                        for( int iz = -Ru[i]; iz <=Ru[i]; iz++ )
                        {
                            LabelImageType::IndexType new_index;
                            Point3D temp_pt;
                            temp_pt.x = Cu.Pt[i].x + ix;
                            temp_pt.y = Cu.Pt[i].y + iy;
                            temp_pt.z = Cu.Pt[i].z + iz;
                            temp_pt.check_out_of_range_3D(SM,SN,SZ);
                            new_index[0] = temp_pt.x;
                            new_index[1] = temp_pt.y;
                            new_index[2] = temp_pt.z;
                            IL->SetPixel( new_index, snake_id );
                        }
                    }
                }
            }
            
		    /*ImageType::IndexType start;
             ImageType::SizeType size;
             start[0] = min_x;
             start[1] = min_y;
             start[2] = min_z;
             size[0] = max_x - min_x;
             size[1] = max_y - min_y;
             size[2] = max_z - min_z;
             ImageType::RegionType region;
             region.SetSize(size);
             region.SetIndex(start);
             
             typedef itk::Point< float, 3 > PointType;
             typedef itk::ImageSliceIteratorWithIndex< LabelImageType > SliceIteratorType;
             typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType1;
             SliceIteratorType It( IL, region );
             SliceIteratorType1 It1( I, region);
             It.SetFirstDirection( 0 );
             It.SetSecondDirection( 1 );
             It.GoToBegin();
             It1.SetFirstDirection( 0 );
             It1.SetSecondDirection( 1 );
             It1.GoToBegin();
             std::vector<float> b_int;
             std::vector<float> f_int;
             while( !It.IsAtEnd() )
             {
             while ( !It.IsAtEndOfSlice() )
             {
             while ( !It.IsAtEndOfLine() )
             {
             PointType p;
             p[0] = It.GetIndex()[0];
             p[1] = It.GetIndex()[1];
             p[2] = It.GetIndex()[2];
             
             if( VesselTube->IsInside(p) )
             {
             if( It.Get() == 0 )
             It.Set( snake_id );
             if( snake_id == 0 )
             It.Set( snake_id );
             }
             int new_int = It.Get();
             if( new_int == 0 )
             b_int.push_back(It1.Get());
             else
             f_int.push_back(It1.Get());
             
             ++It;
             ++It1;
             }
             It.NextLine();
             It1.NextLine();
             }
             It.NextSlice();
             It1.NextSlice();
             } */
            
			/*//update the background and foreground models
             u1 = sigma1 = u2 = sigma2 = 0;
             for(std::vector<float>::iterator j=b_int.begin();j!=b_int.end();++j)
             {
             u2 += *j;
             }
             for(std::vector<float>::iterator k=f_int.begin();k!=f_int.end();++k)
             {
             u1 += *k;
             }
             u2 /= b_int.size();
             u1 /= f_int.size();
             
             for(std::vector<float>::iterator j=b_int.begin();j!=b_int.end();++j)
             {
             sigma2 += pow(*j - u2, 2);
             }
			 for(std::vector<float>::iterator k=f_int.begin();k!=f_int.end();++k)
             {
             sigma1 += pow(*k - u1, 2);
             }
             sigma2 = sqrt(sigma2/b_int.size());
			 sigma1 = sqrt(sigma1/f_int.size());
			 std::cout<<"models:"<<u1<<","<<sigma1<<","<<u2<<","<<sigma2<<std::endl;*/
        }
    }
}


ImagePointer2D ImageOperation::extract_one_slice_yz(ImagePointer I_input, int index)
{
    typedef itk::ImageDuplicator< ImageType > DuplicatorType;
    DuplicatorType::Pointer Duplicator = DuplicatorType::New();
    Duplicator->SetInputImage(I_input);
    Duplicator->Update();
    ImagePointer I_temp = Duplicator->GetOutput();
    
    typedef itk::ExtractImageFilter< ImageType, ImageType2D > FilterType;
    FilterType::Pointer filter = FilterType::New();
    ImageType::RegionType inputRegion = I_temp->GetLargestPossibleRegion();
    ImageType::SizeType size = inputRegion.GetSize();
    
    size[0] = 0;
    ImageType::IndexType start = inputRegion.GetIndex();
    ImageType::RegionType desiredRegion;
    
    start[0] = index;
    desiredRegion.SetSize( size );
    desiredRegion.SetIndex( start );
	
    //std::cout<<"index:"<<index<<std::endl;
    filter->SetExtractionRegion(desiredRegion);
    filter->SetInput(I_temp);
    
    filter->Update();
    ImagePointer2D I2D = filter->GetOutput();
    
    typedef itk::PermuteAxesImageFilter<ImageType2D> PermuterType;
    PermuterType::PermuteOrderArrayType order;
    order[0] = 1;
    order[1] = 0;
    PermuterType::Pointer Permuter = PermuterType::New();
    Permuter->SetInput(I2D);
    Permuter->SetOrder(order);
    Permuter->Update();
    I2D = Permuter->GetOutput();
    
    return I2D;
}

ProbImagePointer2D ImageOperation::extract_one_slice_yz(ProbImagePointer I_input, int index)
{
    typedef itk::ImageDuplicator< ProbImageType > DuplicatorType;
    DuplicatorType::Pointer Duplicator = DuplicatorType::New();
    Duplicator->SetInputImage(I_input);
    Duplicator->Update();
    ProbImagePointer I_temp = Duplicator->GetOutput();
    
    typedef itk::ExtractImageFilter< ProbImageType, ProbImageType2D > FilterType;
    FilterType::Pointer filter = FilterType::New();
    ProbImageType::RegionType inputRegion = I_temp->GetLargestPossibleRegion();
    ProbImageType::SizeType size = inputRegion.GetSize();
    
    size[0] = 0;
    ProbImageType::IndexType start = inputRegion.GetIndex();
    ProbImageType::RegionType desiredRegion;
    
    start[0] = index;
    desiredRegion.SetSize( size );
    desiredRegion.SetIndex( start );
	
    //std::cout<<"index:"<<index<<std::endl;
    filter->SetExtractionRegion(desiredRegion);
    filter->SetInput(I_temp);
    filter->SetDirectionCollapseToSubmatrix(); // This is required.
    
    filter->Update();
    ProbImagePointer2D I2D = filter->GetOutput();
    
    typedef itk::PermuteAxesImageFilter<ProbImageType2D> PermuterType;
    PermuterType::PermuteOrderArrayType order;
    order[0] = 1;
    order[1] = 0;
    PermuterType::Pointer Permuter = PermuterType::New();
    Permuter->SetInput(I2D);
    Permuter->SetOrder(order);
    Permuter->Update();
    I2D = Permuter->GetOutput();
    
    return I2D;
}


ImagePointer2D ImageOperation::extract_one_slice_xz(ImagePointer I_input, int index)
{
    typedef itk::ImageDuplicator< ImageType > DuplicatorType;
    DuplicatorType::Pointer Duplicator = DuplicatorType::New();
    Duplicator->SetInputImage(I_input);
    Duplicator->Update();
    ImagePointer I_temp = Duplicator->GetOutput();
    
    typedef itk::ExtractImageFilter< ImageType, ImageType2D > FilterType;
    FilterType::Pointer filter = FilterType::New();
    ImageType::RegionType inputRegion = I_temp->GetLargestPossibleRegion();
    ImageType::SizeType size = inputRegion.GetSize();
    size[1] = 0;
    ImageType::IndexType start = inputRegion.GetIndex();
    ImageType::RegionType desiredRegion;
    
    start[1] = index;
    desiredRegion.SetSize( size );
    desiredRegion.SetIndex( start );
	
    filter->SetExtractionRegion(desiredRegion);
    filter->SetInput(I_temp);
    filter->SetDirectionCollapseToSubmatrix(); // This is required.
    
    filter->Update();
    ImagePointer2D I2D = filter->GetOutput();
    return I2D;
}

ProbImagePointer2D ImageOperation::extract_one_slice_xz(ProbImagePointer I_input, int index)
{
    typedef itk::ImageDuplicator< ProbImageType > DuplicatorType;
    DuplicatorType::Pointer Duplicator = DuplicatorType::New();
    Duplicator->SetInputImage(I_input);
    Duplicator->Update();
    ProbImagePointer I_temp = Duplicator->GetOutput();
    
    typedef itk::ExtractImageFilter< ProbImageType, ProbImageType2D > FilterType;
    FilterType::Pointer filter = FilterType::New();
    ProbImageType::RegionType inputRegion = I_temp->GetLargestPossibleRegion();
    ProbImageType::SizeType size = inputRegion.GetSize();
    size[1] = 0;
    ProbImageType::IndexType start = inputRegion.GetIndex();
    ProbImageType::RegionType desiredRegion;
    
    start[1] = index;
    desiredRegion.SetSize( size );
    desiredRegion.SetIndex( start );
	
    //std::cout<<"index:"<<index<<std::endl;
    filter->SetExtractionRegion(desiredRegion);
    filter->SetInput(I_temp);
    filter->SetDirectionCollapseToSubmatrix(); // This is required.
    
    filter->Update();
    ProbImagePointer2D I2D = filter->GetOutput();
    return I2D;
}


ImagePointer2D ImageOperation::extract_one_slice(ImagePointer I_input, int index)
{
    typedef itk::ExtractImageFilter< ImageType, ImageType2D > FilterType;
    FilterType::Pointer filter = FilterType::New();
    ImageType::RegionType inputRegion = I_input->GetLargestPossibleRegion();
    ImageType::SizeType size = inputRegion.GetSize();
    size[2] = 0;
    ImageType::IndexType start = inputRegion.GetIndex();
    ImageType::RegionType desiredRegion;
    
    start[2] = index;
    desiredRegion.SetSize( size );
    desiredRegion.SetIndex( start );
	
    filter->SetExtractionRegion(desiredRegion);
    filter->SetInput(I_input);
    filter->SetDirectionCollapseToSubmatrix(); // This is required.
    
    filter->Update();
    ImagePointer2D I2D = filter->GetOutput();
    return I2D;
}

ProbImagePointer2D ImageOperation::extract_one_slice(ProbImagePointer I_input, int index)
{
    typedef itk::ImageDuplicator< ProbImageType > DuplicatorType;
    DuplicatorType::Pointer Duplicator = DuplicatorType::New();
    Duplicator->SetInputImage(I_input);
    Duplicator->Update();
    ProbImagePointer I_temp = Duplicator->GetOutput();
    
    typedef itk::ExtractImageFilter< ProbImageType, ProbImageType2D > FilterType;
    FilterType::Pointer filter = FilterType::New();
    ProbImageType::RegionType inputRegion = I_temp->GetLargestPossibleRegion();
    ProbImageType::SizeType size = inputRegion.GetSize();
    size[2] = 0;
    ProbImageType::IndexType start = inputRegion.GetIndex();
    ProbImageType::RegionType desiredRegion;
    
    start[2] = index;
    desiredRegion.SetSize( size );
    desiredRegion.SetIndex( start );
	
    //std::cout<<"index:"<<index<<std::endl;
    filter->SetExtractionRegion(desiredRegion);
    filter->SetInput(I_temp);
    filter->SetDirectionCollapseToSubmatrix(); // This is required.
    
    filter->Update();
    ProbImagePointer2D I2D = filter->GetOutput();
    return I2D;
}

void ImageOperation::ImThinning(bool clean_skeleton, int min_length)
{
    if( !ISeg )
		return;
    
    typedef itk::BinaryThinningImageFilter3D< ImageType, ImageType > ThinningFilterType;
    ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();
    thinningFilter->SetInput( ISeg );
    thinningFilter->Update();
    ISeg = thinningFilter->GetOutput();
    
	ImagePointer SBW = ISeg;
    if( clean_skeleton )
    {
        //remove barbs
        typedef itk::CastImageFilter<ImageType,ImageType> CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(SBW);
        caster->Update();
        ImagePointer BBW = caster->GetOutput();
        
        typedef itk::NeighborhoodIterator< ImageType, itk::ConstantBoundaryCondition<ImageType> > IteratorType;
        IteratorType::RadiusType radius;
        radius.Fill(1);
        
        IteratorType it( radius, SBW, SBW->GetRequestedRegion() );
        IteratorType it1( radius, BBW, BBW->GetRequestedRegion() );
        it.SetNeedToUseBoundaryCondition(true);
        it1.SetNeedToUseBoundaryCondition(true);
        for( it1.GoToBegin(); !it1.IsAtEnd(); ++it1 )
        {
            it1.SetCenterPixel(0);
        }
        
        for( it.GoToBegin(), it1.GoToBegin(); !it.IsAtEnd(), !it1.IsAtEnd(); ++it, ++it1 )
        {
            if( it.GetCenterPixel() == 0 )
                continue;
            
            int neighbor = 0;
            for(unsigned int i = 0; i < it.Size(); i++)
            {
                if( i == 13 )
                    continue;
                if( it.GetPixel(i) == 1 )
                {
                    neighbor++;
                }
            }
            
            if( neighbor >= 3 )
            {
                it.SetCenterPixel(0);
                it1.SetCenterPixel(1);
                /*for( unsigned int i = 0; i < it.Size(); i++ )
                 {
                 if( it.GetPixel(i) == 1 )
                 {
                 it.SetPixel(i,0);
                 it1.SetPixel(i,1);
                 }
                 }*/
            }
        }
        
        //filter out small size skeletons
        typedef itk::ConnectedComponentImageFilter< ImageType,ImageType > ConnectedFilterType;
        typedef itk::RelabelComponentImageFilter< ImageType, ImageType > RelabelFilterType;
        ConnectedFilterType::Pointer filter1 = ConnectedFilterType::New();
        RelabelFilterType::Pointer filter2 = RelabelFilterType::New();
        filter1->SetFullyConnected(true);
        filter1->SetInput(SBW);
        filter1->Update();
        filter2->SetInput(filter1->GetOutput());
        filter2->SetMinimumObjectSize(min_length);
        filter2->Update();
        ImagePointer LBW = filter2->GetOutput();
        
        IteratorType it2( radius, LBW, LBW->GetRequestedRegion() );
        for( it.GoToBegin(), it1.GoToBegin(), it2.GoToBegin(); !it.IsAtEnd(), !it1.IsAtEnd(), !it2.IsAtEnd(); ++it, ++it1, ++it2 )
        {
            if( it2.GetCenterPixel() == 0 )
            {
                it.SetCenterPixel(0);
            }
            if( it1.GetCenterPixel() == 1 )
            {
                it.SetCenterPixel(1);
            }
        }
    }
}


void ImageOperation::ImFastMarchingAnimation(int threshold)
{
    
    /*typedef itk::BinaryThresholdImageFilter< ProbImageType,
     ImageType>    ThresholdingFilterType;
     ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
     thresholder->SetLowerThreshold( 0.0 );
     thresholder->SetUpperThreshold( threshold  );
     thresholder->SetOutsideValue(  0  );
     thresholder->SetInsideValue(  1 );
     thresholder->SetInput( IDist );
     thresholder->Update();
     
     ISeg = thresholder->GetOutput();*/
    
    
    /*typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
     SliceIteratorType it1( BW_temp, BW_temp->GetRequestedRegion() );
	 SliceIteratorType it2( ISeg, ISeg->GetRequestedRegion() );
     
	 it1.SetFirstDirection( 0 );
     it1.SetSecondDirection( 1 );
	 it2.SetFirstDirection( 0 );
     it2.SetSecondDirection( 1 );
     
	 it1.GoToBegin();
     it2.GoToBegin();
     
     while( !it1.IsAtEnd() )
     {
     while ( !it1.IsAtEndOfSlice() )
     {
     while( !it1.IsAtEndOfLine())
     {
     if( it1.Get() == 1 && it2.Get() == 0 )
     it2.Set(1);
     
     ++it1;
     ++it2;
     }
     it1.NextLine();
     it2.NextLine();
     }
     it1.NextSlice();
     it2.NextSlice();
     }*/
}

void ImageOperation::clear_IMask()
{
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    SliceIteratorType it2( IMask, IMask->GetRequestedRegion() );
    
    it2.SetFirstDirection( 0 );
    it2.SetSecondDirection( 1 );
    it2.GoToBegin();
    
    while( !it2.IsAtEnd() )
    {
        while ( !it2.IsAtEndOfSlice() )
        {
            while( !it2.IsAtEndOfLine())
            {
                it2.Set(0);
                ++it2;
            }
            it2.NextLine();
        }
        it2.NextSlice();
    }
}

void ImageOperation::ImFastMarching_Spine(PointList3D seg_seeds)
{
    typedef itk::NearestNeighborInterpolateImageFunction<
    ImageType, float>  InterpolatorType;
    InterpolatorType::Pointer I_Interpolator = InterpolatorType::New();
    I_Interpolator->SetInputImage(I);
    
	//move the picked points along z axis
    for( int i = 0; i < seg_seeds.NP; i++ )
	{
        seg_seeds.Pt[i].check_out_of_range_3D(SM,SN,SZ);
        ImageType::IndexType index;
        ImageType::IndexType index1;
        index[0] = index1[0] = seg_seeds.Pt[i].x;
        index[1] = index1[1] = seg_seeds.Pt[i].y;
        index[2] = seg_seeds.Pt[i].z;
        for( int j = 0; j < SZ; j++ )
        {
            index1[2] = j;
            if( I_Interpolator->EvaluateAtIndex(index1) > I_Interpolator->EvaluateAtIndex(index) )
            {
                seg_seeds.Pt[i].z = j;
                index[2] = j;
            }
        }
	}
    
    bool auto_threshold = true;
    
    int  timeThreshold = 40;
    int intTh = 5;
    int maxTh = 100;
    int stepTh = 5;
    std::vector<float> score;
    
    if( !ISeg )
    {
        ISeg = ImageType::New();
        ISeg->SetRegions(I->GetRequestedRegion());
        ISeg->Allocate();
    }
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ProbImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    
	/*if( display_set )
     rescale->SetInput( IDisplay );
     else
	 rescale->SetInput( I ); */
    
    ImagePointer ITemp = ImageType::New();
    ITemp->SetRegions(I->GetRequestedRegion());
    ITemp->Allocate();
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    typedef itk::ImageSliceIteratorWithIndex< LabelImageType > SliceIteratorType1;
    SliceIteratorType iit1( I, I->GetRequestedRegion() );
    SliceIteratorType1 iit2( IL, IL->GetRequestedRegion() );
    SliceIteratorType iit3( ITemp, ITemp->GetRequestedRegion() );
    
    iit1.SetFirstDirection( 0 );
    iit1.SetSecondDirection( 1 );
    iit2.SetFirstDirection( 0 );
    iit2.SetSecondDirection( 1 );
    iit3.SetFirstDirection( 0 );
    iit3.SetSecondDirection( 1 );
    
    iit1.GoToBegin();
    iit2.GoToBegin();
    iit3.GoToBegin();
    
    std::cout<<"check point 1"<<std::endl;
    
    while( !iit1.IsAtEnd() )
    {
        while ( !iit1.IsAtEndOfSlice() )
        {
            while( !iit1.IsAtEndOfLine())
            {
                if( iit2.Get() != 1 )
                    iit3.Set(iit1.Get());
                
                ++iit1;
                ++iit2;
                ++iit3;
            }
            iit1.NextLine();
            iit2.NextLine();
            iit3.NextLine();
        }
        iit1.NextSlice();
        iit2.NextSlice();
        iit3.NextSlice();
    }
    
    rescale->SetInput( ITemp );
    
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 1 );
    rescale->Update();
    
    typedef itk::BinaryThresholdImageFilter< ProbImageType,
    ImageType>    ThresholdingFilterType;
    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    
    thresholder->SetLowerThreshold( 0.0 );
    //thresholder->SetUpperThreshold( timeThreshold  );
    thresholder->SetOutsideValue(  0  );
    thresholder->SetInsideValue(  1 );
    
    typedef  itk::FastMarchingImageFilter< ProbImageType,
    ProbImageType >    FastMarchingFilterType;
    FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();
    
    typedef FastMarchingFilterType::NodeContainer           NodeContainer;
    typedef FastMarchingFilterType::NodeType                NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    seeds->Initialize();
    
    for( int i = 0; i< seg_seeds.NP; i++ )
    {
        ProbImageType::IndexType  seedPosition;
        seedPosition[0] = seg_seeds.Pt[i].x;
        seedPosition[1] = seg_seeds.Pt[i].y;
        seedPosition[2] = seg_seeds.Pt[i].z;
        NodeType node;
        const double seedValue = 0.0;
        node.SetValue( seedValue );
        node.SetIndex( seedPosition );
        seeds->InsertElement( i, node );
    }
    
    
    fastMarching->SetTrialPoints(  seeds  );
    fastMarching->SetOutputSize(
                                rescale->GetOutput()->GetBufferedRegion().GetSize() );
    const double stoppingTime = timeThreshold * 1.1;
    fastMarching->SetStoppingValue(  stoppingTime  );
    
    fastMarching->SetInput( rescale->GetOutput() );
    thresholder->SetInput( fastMarching->GetOutput() );
    //thresholder->Update();
    
    std::cout<<"check point 2"<<std::endl;
    
    if( auto_threshold )
    {
        for( int i = intTh; i < maxTh; i = i + stepTh )
        {
            thresholder->SetUpperThreshold( i );
            thresholder->Update();
            
            float score_temp = 0;
            
            //estimate u1 and u2
            float u1, u2;
            int size_u1, size_u2;
            size_u1 = size_u2 = 0;
            u1 = u2 = 0;
            
            SliceIteratorType it1( I, I->GetRequestedRegion() );
            SliceIteratorType it2( thresholder->GetOutput(), thresholder->GetOutput()->GetRequestedRegion() );
            
            it1.SetFirstDirection( 0 );
            it1.SetSecondDirection( 1 );
            it2.SetFirstDirection( 0 );
            it2.SetSecondDirection( 1 );
            
            it1.GoToBegin();
            it2.GoToBegin();
            
            while( !it1.IsAtEnd() )
            {
                while ( !it1.IsAtEndOfSlice() )
                {
                    while( !it1.IsAtEndOfLine())
                    {
                        if( it2.Get() == 1)
                        {
                            u1 += it1.Get();
                            size_u1++;
                        }
                        else
                        {
                            u2 += (float)it1.Get();
                            size_u2++;
                        }
                        ++it1;
                        ++it2;
                    }
                    it1.NextLine();
                    it2.NextLine();
                }
                it1.NextSlice();
                it2.NextSlice();
            }
            
            u1 = (float)u1/(float)size_u1;
            u2 = (float)u2/(float)size_u2;
            //std::cout<<"u1,u2:"<<u1<<","<<u2<<std::endl;
            //compute the score
            it1.GoToBegin();
            it2.GoToBegin();
            
            while( !it1.IsAtEnd() )
            {
                while ( !it1.IsAtEndOfSlice() )
                {
                    while( !it1.IsAtEndOfLine())
                    {
                        if( it2.Get() == 1)
                        {
                            //score_temp += pow(it1.Get() - u1, 2);
                            score_temp -=  log(MAX_snake(norm_density(it1.Get(),u1,sigma1),std::numeric_limits<float>::epsilon()));
                        }
                        else
                        {
                            //score_temp += pow(it1.Get() - u2, 2);
                            score_temp -=  log(MAX_snake(norm_density(it1.Get(),u2,sigma2),std::numeric_limits<float>::epsilon()));
                        }
                        ++it1;
                        ++it2;
                    }
                    it1.NextLine();
                    it2.NextLine();
                }
                it1.NextSlice();
                it2.NextSlice();
            }
            score.push_back(score_temp);
        }
        
        vnl_vector<float> score_vnl(score.size());
        //pick the threshold with lowest score (cost)
        for( int j = 0; j < score.size(); j++ )
        {
            score_vnl(j) = score[j];
            //std::cout<<score_vnl(j)<<",";
        }
        //std::cout<<std::endl;
        
        int idx = score_vnl.arg_max();
        std::cout<<"Selected Threshold:"<<intTh + stepTh * idx<<std::endl;
        thresholder->SetUpperThreshold( intTh + stepTh * idx );
        thresholder->Update();
        
    }
    else
    {
        thresholder->SetUpperThreshold( timeThreshold  );
        thresholder->Update();
    }
    
    
    
    std::cout<<"check point 3"<<std::endl;
    
    ImagePointer BW_temp = thresholder->GetOutput();
    
    SliceIteratorType it1( BW_temp, BW_temp->GetRequestedRegion() );
    SliceIteratorType it2( ISeg, ISeg->GetRequestedRegion() );
    
    it1.SetFirstDirection( 0 );
    it1.SetSecondDirection( 1 );
    it2.SetFirstDirection( 0 );
    it2.SetSecondDirection( 1 );
    
    it1.GoToBegin();
    it2.GoToBegin();
    
    while( !it1.IsAtEnd() )
    {
        while ( !it1.IsAtEndOfSlice() )
        {
            while( !it1.IsAtEndOfLine())
            {
                if( it1.Get() == 1 && it2.Get() == 0 )
                    it2.Set(1);
                
                ++it1;
                ++it2;
            }
            it1.NextLine();
            it2.NextLine();
        }
        it1.NextSlice();
        it2.NextSlice();
    }
    
    std::cout<<"check point 4"<<std::endl;
}


void ImageOperation::ImFastMarching_Soma(PointList3D seg_seeds, int timeThreshold, double curvatureScaling, double rmsError, const char *somaFileName)
{
    typedef itk::NearestNeighborInterpolateImageFunction< ImageType, float>  InterpolatorType;
    InterpolatorType::Pointer I_Interpolator = InterpolatorType::New();
    I_Interpolator->SetInputImage(I);
    
	//move the picked points along z axis
    for( int i = 0; i < seg_seeds.NP; i++ )
	{
        seg_seeds.Pt[i].check_out_of_range_3D(SM,SN,SZ);
        ImageType::IndexType index;
        ImageType::IndexType index1;
        index[0] = index1[0] = seg_seeds.Pt[i].x;
        index[1] = index1[1] = seg_seeds.Pt[i].y;
        index[2] = seg_seeds.Pt[i].z;
        for( int j = 0; j < SZ; j++ )
        {
            index1[2] = j;
            if( I_Interpolator->EvaluateAtIndex(index1) > I_Interpolator->EvaluateAtIndex(index) )
            {
                seg_seeds.Pt[i].z = j;
                index[2] = j;
            }
        }
	}
    
    if( !IMask )
    {
        IMask = ImageType::New();
        IMask->SetRegions(I->GetRequestedRegion());
        IMask->Allocate();
    }
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ProbImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    
	if( display_set )
	{
		std::cout<< "Display_Set"<<std::endl;
		rescale->SetInput( IDisplay );
	}
	else
	{
		std::cout<< "Image"<<std::endl;
		rescale->SetInput( I );
	}
    
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 1 );
    rescale->Update();
    
    std::cout<< "Generating Distance Map..." <<std::endl;
    clock_t SomaExtraction_start_time = clock();
    typedef  itk::FastMarchingImageFilter< ProbImageType, ProbImageType >    FastMarchingFilterType;
    FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();
    
    typedef FastMarchingFilterType::NodeContainer           NodeContainer;
    typedef FastMarchingFilterType::NodeType                NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    seeds->Initialize();
    
    for( int i = 0; i< seg_seeds.NP; i++ )
    {
        ProbImageType::IndexType  seedPosition;
        seedPosition[0] = seg_seeds.Pt[i].x;
        seedPosition[1] = seg_seeds.Pt[i].y;
        seedPosition[2] = seg_seeds.Pt[i].z;
        NodeType node;
        const double seedValue = -3;
        node.SetValue( seedValue );
        node.SetIndex( seedPosition );
        seeds->InsertElement( i, node );
    }
    
    fastMarching->SetTrialPoints(  seeds);
    fastMarching->SetOutputSize( I->GetBufferedRegion().GetSize() );
    const double stoppingTime = timeThreshold * 1.1;
    fastMarching->SetStoppingValue(  stoppingTime);
    fastMarching->SetSpeedConstant( 1.0 );
    fastMarching->Update();
    std::cout<< "Total time for Distance Map is: " << (clock() - SomaExtraction_start_time) / (float) CLOCKS_PER_SEC << std::endl;
    
    SomaExtraction_start_time = clock();
	std::cout<< "Shape Detection..." <<std::endl;
	/// Shape Detection
	typedef itk::ShapeDetectionLevelSetImageFilter< ProbImageType, ProbImageType> ShapeDetectionFilterType;
	ShapeDetectionFilterType::Pointer shapeDetection = ShapeDetectionFilterType::New();
	
	shapeDetection->SetPropagationScaling(1.0);
	shapeDetection->SetCurvatureScaling(curvatureScaling);
	shapeDetection->SetMaximumRMSError( rmsError);
	shapeDetection->SetNumberOfIterations( 800);
    
	shapeDetection->SetInput( fastMarching->GetOutput());
	shapeDetection->SetFeatureImage( rescale->GetOutput());
    
	typedef itk::BinaryThresholdImageFilter< ProbImageType, ImageType> ShapeThresholdingFilterType;
    ShapeThresholdingFilterType::Pointer thresholder2 = ShapeThresholdingFilterType::New();
	thresholder2->SetLowerThreshold( std::numeric_limits< ProbImageType::PixelType >::min());
	thresholder2->SetUpperThreshold(0.0);
	thresholder2->SetOutsideValue( 0);
	thresholder2->SetInsideValue(255);
	thresholder2->SetInput( shapeDetection->GetOutput());
	thresholder2->Update();
	std::cout << "No. elpased iterations: " << shapeDetection->GetElapsedIterations() << std::endl;
	std::cout << "RMS change: " << shapeDetection->GetRMSChange() << std::endl;
	std::cout<< "Total time for Shape Detection is: " << (clock() - SomaExtraction_start_time) / (float) CLOCKS_PER_SEC << std::endl;
    
	typedef itk::CastImageFilter<ImageType, IOImageType> CasterType;
	CasterType::Pointer caster = CasterType::New();
    
	caster->SetInput(thresholder2->GetOutput());
    
	typedef itk::ImageFileWriter<IOImageType> WriterType;
	WriterType::Pointer writer1 = WriterType::New();
	writer1->SetInput( caster->GetOutput());
	writer1->SetFileName(somaFileName);
	writer1->Update();
    
	/// Write the image to IMask
	//IMask = thresholder2->GetOutput();
}

std::vector<float> ImageOperation::ImFastMarchingI(PointList3D seg_seeds)
{
    
    bool auto_threshold = true;
    
    int  timeThreshold = 40;
    int intTh = 5;
    int maxTh = 100;
    int stepTh = 5;
    std::vector<float> score;
    
    ISeg = ImageType::New();
    ISeg->SetRegions(I->GetRequestedRegion());
    ISeg->Allocate();
    
    
    /*typedef   itk::GradientMagnitudeRecursiveGaussianImageFilter<
     ImageType,
     ProbImageType >  GradientFilterType;
	 GradientFilterType::Pointer  gradientMagnitude = GradientFilterType::New();
	 gradientMagnitude->SetInput( I );
     gradientMagnitude->SetSigma(1);
     gradientMagnitude->Update();
	 IGMag = gradientMagnitude->GetOutput();*/
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ProbImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    
	if( IDisplay )
        rescale->SetInput( IDisplay );
	else
        rescale->SetInput( I );
    
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 1 );
    rescale->Update();
    
    typedef itk::BinaryThresholdImageFilter< ProbImageType,
    ImageType>    ThresholdingFilterType;
    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    
    thresholder->SetLowerThreshold( 0.0 );
    //thresholder->SetUpperThreshold( timeThreshold  );
    thresholder->SetOutsideValue(  0  );
    thresholder->SetInsideValue(  1 );
    
    typedef  itk::FastMarchingImageFilter< ProbImageType,
    ProbImageType >    FastMarchingFilterType;
    FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();
    
    typedef FastMarchingFilterType::NodeContainer           NodeContainer;
    typedef FastMarchingFilterType::NodeType                NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    seeds->Initialize();
    
    for( int i = 0; i< seg_seeds.NP; i++ )
    {
        ProbImageType::IndexType  seedPosition;
        seedPosition[0] = seg_seeds.Pt[i].x;
        seedPosition[1] = seg_seeds.Pt[i].y;
        seedPosition[2] = seg_seeds.Pt[i].z;
        NodeType node;
        const double seedValue = 0.0;
        node.SetValue( seedValue );
        node.SetIndex( seedPosition );
        seeds->InsertElement( i, node );
    }
    
    
    fastMarching->SetTrialPoints(  seeds  );
    fastMarching->SetOutputSize(
                                rescale->GetOutput()->GetBufferedRegion().GetSize() );
    //const double stoppingTime = timeThreshold * 1.1;
    const double stoppingTime = maxTh;
    fastMarching->SetStoppingValue(  stoppingTime  );
    
    fastMarching->SetInput( rescale->GetOutput() );
    thresholder->SetInput( fastMarching->GetOutput() );
    //thresholder->Update();
    
    if( auto_threshold )
    {
        for( int i = intTh; i < maxTh; i = i + stepTh )
        {
            thresholder->SetUpperThreshold( i );
            thresholder->Update();
            
            float score_temp = 0;
            
            //estimate u1 and u2
            //float u1, u2;
            u1 = u2 = sigma1 = sigma2 = 0;
            std::vector<float> b_int, f_int;
            
            typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
            SliceIteratorType it1( I, I->GetRequestedRegion() );
            SliceIteratorType it2( thresholder->GetOutput(), thresholder->GetOutput()->GetRequestedRegion() );
            
            it1.SetFirstDirection( 0 );
            it1.SetSecondDirection( 1 );
            it2.SetFirstDirection( 0 );
            it2.SetSecondDirection( 1 );
            
            it1.GoToBegin();
            it2.GoToBegin();
            
            while( !it1.IsAtEnd() )
            {
                while ( !it1.IsAtEndOfSlice() )
                {
                    while( !it1.IsAtEndOfLine())
                    {
                        if( it2.Get() == 1)
                        {
                            //u1 += it1.Get();
                            f_int.push_back(it1.Get());
                        }
                        else
                        {
                            b_int.push_back(it1.Get());
                        }
                        ++it1;
                        ++it2;
                    }
                    it1.NextLine();
                    it2.NextLine();
                }
                it1.NextSlice();
                it2.NextSlice();
            }
            
            for(std::vector<float>::iterator j=b_int.begin();j!=b_int.end();++j)
            {
                u2 += *j;
            }
            for(std::vector<float>::iterator k=f_int.begin();k!=f_int.end();++k)
            {
                u1 += *k;
            }
            u2 /= b_int.size();
            u1 /= f_int.size();
            
            for(std::vector<float>::iterator j=b_int.begin();j!=b_int.end();++j)
            {
                sigma2 += pow(*j - u2, 2);
            }
            for(std::vector<float>::iterator k=f_int.begin();k!=f_int.end();++k)
            {
                sigma1 += pow(*k - u1, 2);
            }
            sigma2 = sqrt(sigma2/b_int.size());
            sigma1 = sqrt(sigma1/f_int.size());
            
            //std::cout<<"u1,u2:"<<u1<<","<<u2<<std::endl;
            //compute the score
            it1.GoToBegin();
            it2.GoToBegin();
            
            while( !it1.IsAtEnd() )
            {
                while ( !it1.IsAtEndOfSlice() )
                {
                    while( !it1.IsAtEndOfLine())
                    {
                        if( it2.Get() == 1)
                        {
                            score_temp += pow(it1.Get() - u1, 2);
                            //score_temp -=  log(MAX(norm_density(it1.Get(),u1,sigma1),std::numeric_limits<float>::epsilon()));
                        }
                        else
                        {
                            score_temp += pow(it1.Get() - u2, 2);
                            //score_temp -=  log(MAX(norm_density(it1.Get(),u2,sigma2),std::numeric_limits<float>::epsilon()));
                        }
                        ++it1;
                        ++it2;
                    }
                    it1.NextLine();
                    it2.NextLine();
                }
                it1.NextSlice();
                it2.NextSlice();
            }
            score.push_back(score_temp);
        }
        
        vnl_vector<float> score_vnl(score.size());
        //pick the threshold with lowest score (cost)
        for( int j = 0; j < score.size(); j++ )
        {
            score_vnl(j) = score[j];
            //std::cout<<score_vnl(j)<<",";
        }
        //std::cout<<std::endl;
        
        int idx = score_vnl.arg_min();
        std::cout<<"Selected Threshold:"<<intTh + stepTh * idx<<std::endl;
        thresholder->SetUpperThreshold( intTh + stepTh * idx );
        thresholder->Update();
    }
    else
    {
        thresholder->SetUpperThreshold( timeThreshold  );
        thresholder->Update();
    }
    
    //IDist = fastMarching->GetOutput();
    
    /*typedef itk::CastImageFilter<ProbImageType,IOImageType1> ProbCasterType;
     ProbCasterType::Pointer prob_caster = ProbCasterType::New();
     prob_caster->SetInput(IDist);
     IOImagePointer1 IIO = prob_caster->GetOutput();
     prob_caster->Update();
     
	 typedef itk::ImageFileWriter<IOImageType1> WriterType;
     
	 WriterType::Pointer writer;
	 writer = WriterType::New();
     
	 writer->SetFileName("DistanceMap.tif");
	 writer->SetInput(IIO);
	 writer->Update(); */
    
    
    ISeg = thresholder->GetOutput();
    
    //ImThinning(false, 2);
    
    return score;
    /*typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
     SliceIteratorType it1( BW_temp, BW_temp->GetRequestedRegion() );
	 SliceIteratorType it2( ISeg, ISeg->GetRequestedRegion() );
     
	 it1.SetFirstDirection( 0 );
     it1.SetSecondDirection( 1 );
	 it2.SetFirstDirection( 0 );
     it2.SetSecondDirection( 1 );
     
	 it1.GoToBegin();
     it2.GoToBegin();
     
     while( !it1.IsAtEnd() )
     {
     while ( !it1.IsAtEndOfSlice() )
     {
     while( !it1.IsAtEndOfLine())
     {
     if( it1.Get() == 1 && it2.Get() == 0 )
     it2.Set(1);
     
     ++it1;
     ++it2;
     }
     it1.NextLine();
     it2.NextLine();
     }
     it1.NextSlice();
     it2.NextSlice();
     } */
    
    //VBW = ISeg;
}

void ImageOperation::ImFastMarchingI_New(PointList3D seg_seeds)
{
    
    int time_min = 10;
    int time_step = 2;
    int time_max = 100;
    
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    
    ISeg = ImageType::New();
    ISeg->SetRegions(I->GetRequestedRegion());
    ISeg->Allocate();
    
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ProbImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    rescale->SetInput( I );
    rescale->SetOutputMinimum( 0.01 );
    rescale->SetOutputMaximum( 1 );
    rescale->Update();
    
    //2-LabelFast Marching segmentation
    typedef  itk::FastMarchingImageFilter< ProbImageType,
    ProbImageType >    FastMarchingFilterType;
    FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();
    
    typedef FastMarchingFilterType::NodeContainer           NodeContainer;
    typedef FastMarchingFilterType::NodeType                NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    seeds->Initialize();
    
    for( int i = 0; i< seg_seeds.NP; i++ )
    {
        ProbImageType::IndexType  seedPosition;
        seedPosition[0] = seg_seeds.Pt[i].x;
        seedPosition[1] = seg_seeds.Pt[i].y;
        seedPosition[2] = seg_seeds.Pt[i].z;
        NodeType node;
        const double seedValue = 0.0;
        node.SetValue( seedValue );
        node.SetIndex( seedPosition );
        seeds->InsertElement( i, node );
    }
    fastMarching->SetTrialPoints(  seeds  );
    fastMarching->SetOutputSize(
                                rescale->GetOutput()->GetBufferedRegion().GetSize() );
    //const double stoppingTime = time_max * 1.1;
    //fastMarching->SetStoppingValue(  stoppingTime  );
    fastMarching->SetInput( rescale->GetOutput() );
    fastMarching->Update();
    
    //selecting time threshold
    
    std::vector<double> energy_score;
    
    std::cout<<"Time Threshold:";
    for( int i = time_min; i < time_max; i+= time_step )
    {
        std::cout<<i<<":"<<std::endl;
        typedef itk::ImageSliceIteratorWithIndex< ProbImageType > IteratorType;
        IteratorType it( fastMarching->GetOutput(), fastMarching->GetOutput()->GetRequestedRegion());
        SliceIteratorType it_I( I, I->GetRequestedRegion() );
        
        //compute u and v
        double u = 0;
        double v = 0;
        it.SetFirstDirection(0);
        it.SetSecondDirection(1);
        it.GoToBegin();
        it_I.SetFirstDirection(0);
        it_I.SetSecondDirection(1);
        it_I.GoToBegin();
        int u_count = 0;
        while( !it.IsAtEnd() )
        {
		    while( !it.IsAtEndOfSlice() )
			{
				while( !it.IsAtEndOfLine())
				{
                    if( it.Get() < i )
                    {
                        u += it_I.Get();
                        u_count++;
                    }
                    else
                        v += it_I.Get();
                    ++it;
                    ++it_I;
				}
				it.NextLine();
				it_I.NextLine();
			}
			it.NextSlice();
			it_I.NextLine();
        }
        
        u = u/u_count;
        v = v/(SM*SN*SZ - u_count);
        
        std::cout<<"u:"<<u<<","<<"v:"<<v<<std::endl;
        
        //compute the energy score
        double temp_score = 0;
        it.SetFirstDirection(0);
        it.SetSecondDirection(1);
        it.GoToBegin();
        it_I.SetFirstDirection(0);
        it_I.SetSecondDirection(1);
        it_I.GoToBegin();
        while( !it.IsAtEnd() )
        {
		    while( !it.IsAtEndOfSlice() )
			{
				while( !it.IsAtEndOfLine())
				{
                    if( it.Get() < i )
                        temp_score += pow(it_I.Get() - u, 2);
                    else
                        temp_score += pow(it_I.Get() - v, 2);
                    ++it;
                    ++it_I;
				}
				it.NextLine();
				it_I.NextLine();
			}
			it.NextSlice();
			it_I.NextSlice();
        }
        
        temp_score = sqrt(temp_score);
        energy_score.push_back(temp_score);
        std::cout<<temp_score<<std::endl;
    }
    
    
    vnl_vector<double> score(energy_score.size());
    for( int k = 0; k < energy_score.size(); k++ )
    {
        score(k) = energy_score[k];
    }
    
    int idx = score.arg_min();
    
    int timeThreshold = time_min + idx * time_step;
    
    std::cout<<"optimal time threshold:"<<timeThreshold<<std::endl;
	
    typedef itk::BinaryThresholdImageFilter< ProbImageType,
    ImageType>    ThresholdingFilterType;
    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    thresholder->SetLowerThreshold( 0.0 );
    thresholder->SetUpperThreshold( timeThreshold  );
    thresholder->SetOutsideValue(  0  );
    thresholder->SetInsideValue(  1 );
    thresholder->SetInput( fastMarching->GetOutput() );
    thresholder->Update();
    
    ImagePointer BW_temp = thresholder->GetOutput();
    
    SliceIteratorType it1( BW_temp, BW_temp->GetRequestedRegion() );
    SliceIteratorType it2( ISeg, ISeg->GetRequestedRegion() );
    
    it1.SetFirstDirection( 0 );
    it1.SetSecondDirection( 1 );
    it2.SetFirstDirection( 0 );
    it2.SetSecondDirection( 1 );
    
    it1.GoToBegin();
    it2.GoToBegin();
    
    while( !it1.IsAtEnd() )
    {
        while ( !it1.IsAtEndOfSlice() )
        {
            while( !it1.IsAtEndOfLine())
            {
                if( it1.Get() == 1 && it2.Get() == 0 )
                    it2.Set(1);
                
                ++it1;
                ++it2;
            }
            it1.NextLine();
            it2.NextLine();
        }
        it1.NextSlice();
        it2.NextSlice();
    }
    
    //VBW = ISeg;
    
    
}

void ImageOperation::ImFastMarchingII(PointList3D seg_seeds, int idx)
{
    
    if( idx == 1 )
	{
        ISeg = ImageType::New();
        ISeg->SetRegions(I->GetRequestedRegion());
        ISeg->Allocate();
        
        /*typedef   itk::GradientMagnitudeRecursiveGaussianImageFilter<
         ImageType,
         ProbImageType >  GradientFilterType;
         GradientFilterType::Pointer  gradientMagnitude = GradientFilterType::New();
         gradientMagnitude->SetInput( I );
         gradientMagnitude->SetSigma(1);
         gradientMagnitude->Update();
         IGMag = gradientMagnitude->GetOutput();*/
        
        typedef itk::RescaleIntensityImageFilter< ImageType, ProbImageType> RescaleFilterType;
        RescaleFilterType::Pointer rescale = RescaleFilterType::New();
        
        if( display_set )
            rescale->SetInput( IDisplay );
        else
            rescale->SetInput( I );
        
        rescale->SetOutputMinimum( 0 );
        rescale->SetOutputMaximum( 1 );
        rescale->Update();
        IGMag = rescale->GetOutput();
	}
    
    
    /*typedef itk::BinaryThresholdImageFilter< ProbImageType,
     ImageType>    ThresholdingFilterType;
     ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
     int  timeThreshold = 2;
     thresholder->SetLowerThreshold( 0.0 );
     thresholder->SetUpperThreshold( timeThreshold  );
     thresholder->SetOutsideValue(  0  );
     thresholder->SetInsideValue(  1 );*/
    
    
    
    
    typedef itk::BinaryThresholdImageFilter< ProbImageType,
    ImageType>    ThresholdingFilterType;
    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    int  timeThreshold = 50;
    thresholder->SetLowerThreshold( 0.0 );
    thresholder->SetUpperThreshold( timeThreshold  );
    thresholder->SetOutsideValue(  0  );
    thresholder->SetInsideValue(  1 );
    
    typedef  itk::FastMarchingImageFilter< ProbImageType,
    ProbImageType >    FastMarchingFilterType;
    FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();
    
    typedef FastMarchingFilterType::NodeContainer           NodeContainer;
    typedef FastMarchingFilterType::NodeType                NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    seeds->Initialize();
    
    for( int i = 0; i< seg_seeds.NP; i++ )
    {
        ProbImageType::IndexType  seedPosition;
        seedPosition[0] = seg_seeds.Pt[i].x;
        seedPosition[1] = seg_seeds.Pt[i].y;
        seedPosition[2] = seg_seeds.Pt[i].z;
        NodeType node;
        const double seedValue = 0.0;
        node.SetValue( seedValue );
        node.SetIndex( seedPosition );
        seeds->InsertElement( i, node );
    }
    
    fastMarching->SetTrialPoints(  seeds  );
    fastMarching->SetOutputSize(
                                IGMag->GetBufferedRegion().GetSize() );
    const double stoppingTime = timeThreshold * 1.1;
    fastMarching->SetStoppingValue(  stoppingTime  );
    
    fastMarching->SetInput( IGMag );
    thresholder->SetInput( fastMarching->GetOutput() );
    thresholder->Update();
    
    ImagePointer BW_temp = thresholder->GetOutput();
    
    
    typedef itk::ImageSliceIteratorWithIndex< ImageType > SliceIteratorType;
    SliceIteratorType it1( BW_temp, BW_temp->GetRequestedRegion() );
    SliceIteratorType it2( ISeg, ISeg->GetRequestedRegion() );
    
    it1.SetFirstDirection( 0 );
    it1.SetSecondDirection( 1 );
    it2.SetFirstDirection( 0 );
    it2.SetSecondDirection( 1 );
    
    it1.GoToBegin();
    it2.GoToBegin();
    
    while( !it1.IsAtEnd() )
    {
        while ( !it1.IsAtEndOfSlice() )
        {
            while( !it1.IsAtEndOfLine())
            {
                if( it1.Get() == 1 && it2.Get() == 0 )
                    it2.Set(idx);
                
                ++it1;
                ++it2;
            }
            it1.NextLine();
            it2.NextLine();
        }
        it1.NextSlice();
        it2.NextSlice();
    }
    
    //VBW = ISeg;
}

//this is an in-place filter which applies an LoG filter over the image before the rest of the preprocessing kicks in
void ImageOperation::ImLaplacian_of_Gaussian()
{
	int sigma = 3;
    
	typedef itk::LaplacianRecursiveGaussianImageFilter< ImageType > LoGFilterType;
	LoGFilterType::Pointer LoG = LoGFilterType::New();
	LoG->SetInput(I);
	LoG->SetSigma(sigma);
    
	//typedef itk::InvertIntensityImageFilter< ImageType, ImageType > InvertType;
	//InvertType::Pointer invert = InvertType::New();
	//invert->SetInput(LoG->GetOutput());
    
	//invert->Update();
	//I = invert->GetOutput();
	LoG->Update();
	I = LoG->GetOutput();
    
}

ImagePointer ImageOperation::ImGaussian(ImagePointer I_In, int sigma)
{
	//int sigma = 1;
    
    typedef itk::RecursiveGaussianImageFilter <ImageType, ImageType> FilterType;
    
    FilterType::Pointer filterX = FilterType::New();
    FilterType::Pointer filterY = FilterType::New();
	FilterType::Pointer filterZ = FilterType::New();
    filterX->SetDirection(0); // x direction
    filterY->SetDirection(1); // y direction
	filterZ->SetDirection(2);
    filterX->SetOrder(FilterType::ZeroOrder);
    filterY->SetOrder(FilterType::ZeroOrder);
	filterZ->SetOrder(FilterType::ZeroOrder);
    filterX->SetNormalizeAcrossScale(false);
    filterY->SetNormalizeAcrossScale(false);
	filterZ->SetNormalizeAcrossScale(false);
    filterX->SetSigma(sigma);
    filterY->SetSigma(sigma);
	filterZ->SetSigma(sigma);
    
    filterX->SetInput( I_In );
    filterY->SetInput(filterX->GetOutput() );
	filterZ->SetInput(filterY->GetOutput() );
    filterZ->Update();
    ImagePointer Out = filterZ->GetOutput();
	Out->DisconnectPipeline();
    return Out;
}

ProbImagePointer ImageOperation::ImGaussian(ProbImagePointer I_In, int sigma)
{
    typedef itk::RecursiveGaussianImageFilter <ProbImageType, ProbImageType> FilterType;
    
    FilterType::Pointer filterX = FilterType::New();
    FilterType::Pointer filterY = FilterType::New();
	FilterType::Pointer filterZ = FilterType::New();
    filterX->SetDirection(0); // x direction
    filterY->SetDirection(1); // y direction
	filterZ->SetDirection(2);
    filterX->SetOrder(FilterType::ZeroOrder);
    filterY->SetOrder(FilterType::ZeroOrder);
	filterZ->SetOrder(FilterType::ZeroOrder);
    filterX->SetNormalizeAcrossScale(false);
    filterY->SetNormalizeAcrossScale(false);
	filterZ->SetNormalizeAcrossScale(false);
    filterX->SetSigma(sigma);
    filterY->SetSigma(sigma);
	filterZ->SetSigma(sigma);
    
    filterX->SetInput( I_In );
    filterY->SetInput(filterX->GetOutput() );
	filterZ->SetInput(filterY->GetOutput() );
    filterZ->Update();
    ProbImagePointer Out = filterZ->GetOutput();
	Out->DisconnectPipeline();
    return Out;
}

ImagePointer ImageOperation::ImGaussian_XY(ImagePointer I_In, int sigma)
{
    typedef itk::RecursiveGaussianImageFilter <ImageType, ImageType> FilterType;
    
    FilterType::Pointer filterX = FilterType::New();
    FilterType::Pointer filterY = FilterType::New();
    filterX->SetDirection(0); // x direction
    filterY->SetDirection(1); // y direction
    filterX->SetOrder(FilterType::ZeroOrder);
    filterY->SetOrder(FilterType::ZeroOrder);
    filterX->SetNormalizeAcrossScale(false);
    filterY->SetNormalizeAcrossScale(false);
    filterX->SetSigma(sigma);
    filterY->SetSigma(sigma);
    
    filterX->SetInput( I_In );
    filterY->SetInput(filterX->GetOutput() );
    filterY->Update();
    ImagePointer Out = filterY->GetOutput();
	Out->DisconnectPipeline();
    return Out;
}

IOImagePointer ImageOperation::ImGaussian_XY(IOImagePointer I_In, int sigma)
{
    typedef itk::RecursiveGaussianImageFilter <IOImageType, IOImageType> FilterType;
    
    FilterType::Pointer filterX = FilterType::New();
    FilterType::Pointer filterY = FilterType::New();
    filterX->SetDirection(0); // x direction
    filterY->SetDirection(1); // y direction
    filterX->SetOrder(FilterType::ZeroOrder);
    filterY->SetOrder(FilterType::ZeroOrder);
    filterX->SetNormalizeAcrossScale(false);
    filterY->SetNormalizeAcrossScale(false);
    filterX->SetSigma(sigma);
    filterY->SetSigma(sigma);
    
    filterX->SetInput( I_In );
    filterY->SetInput(filterX->GetOutput() );
    filterY->Update();
    IOImagePointer Out = filterY->GetOutput();
	Out->DisconnectPipeline();
    return Out;
}

void ImageOperation::ComputeMultiVesselness(double sigma_min, double sigma_max, int sigma_step)
{
    
    if( display_set == false )
    {
        typedef itk::CastImageFilter<ImageType,ImageType> CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(I);
        IDisplay = caster->GetOutput();
        caster->Update();
        
        display_set = true;
    }
    else
    {
        typedef itk::CastImageFilter<ImageType,ImageType> CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(IDisplay);
        I = caster->GetOutput();
        caster->Update();
    }
    
    bool SatoVesselness = false;
    
	if( SatoVesselness )
	{                
        typedef itk::SymmetricSecondRankTensor<double, 3> HessianTensorType;
        typedef itk::Image< HessianTensorType, 3 > HessianImageType;
                
        // Declare the type of multiscale enhancement filter
        typedef itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType > MultiScaleEnhancementFilterType;
        
        // Declare the type of enhancement filter - use ITK's 3D vesselness (Sato)
        typedef itk::Hessian3DToVesselnessMeasureImageFilter< int > VesselnessFilterType;
        
        // Instantiate the multiscale filter and set the input image
        MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter = MultiScaleEnhancementFilterType::New();
        multiScaleEnhancementFilter->SetInput( I );
        multiScaleEnhancementFilter->SetSigmaMinimum( sigma_min );
        multiScaleEnhancementFilter->SetSigmaMaximum( sigma_max );
        multiScaleEnhancementFilter->SetNumberOfSigmaSteps( sigma_step );
        
        // Get the vesselness filter and set the parameters
        VesselnessFilterType::Pointer vesselnessFilter = VesselnessFilterType::New();
        vesselnessFilter->SetAlpha1(0.5);
        vesselnessFilter->SetAlpha2(2);
        
        multiScaleEnhancementFilter->SetHessianToMeasureFilter(vesselnessFilter);
        
        try
        {
            multiScaleEnhancementFilter->Update();
        }
        catch (itk::ExceptionObject & err)
        {
            std::cerr << "Exception caught: "<< err << std::endl;
        }
        I = ImRescale(multiScaleEnhancementFilter->GetOutput());
	}
	else
	{
        typedef itk::MultiScaleHessianSmoothed3DToVesselnessMeasureImageFilter<
        ImageType,ProbImageType> MultiScaleVesselnessFilterType;
        MultiScaleVesselnessFilterType::Pointer MultiScaleVesselnessFilter =
        MultiScaleVesselnessFilterType::New();
        MultiScaleVesselnessFilter->SetInput( I );
        MultiScaleVesselnessFilter->SetSigmaMin( sigma_min );
        MultiScaleVesselnessFilter->SetSigmaMax( sigma_max );
        MultiScaleVesselnessFilter->SetNumberOfSigmaSteps( sigma_step );
        
        try
        {
            MultiScaleVesselnessFilter->Update();
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "Exception caught: "<< err << std::endl;
        }
        
        I = ImRescale(MultiScaleVesselnessFilter->GetOutput());
	}
}

void ImageOperation::ComputeGVFVesselness2D()
{
    double FrangiAlpha = 0.5;
    double FrangiBeta = 0.5;
    double FrangiC = 10;
    //double A = 2 * pow(FrangiAlpha,2);
    double B = 2 * pow(FrangiBeta,2);
    double C = 2 * pow(FrangiC,2);
    
    typedef itk::CastImageFilter<ImageType,ProbImageType> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(I);
    caster->Update();
    IVessel = caster->GetOutput();
    
    typedef itk::RecursiveGaussianImageFilter<
    ProbImageType, ProbImageType > FilterType;
    //typedef itk::GradientImageFilter<ProbImageType, float, float> FilterType;
    ProbImagePointer Dx = extract_one_component(0,IGVF);
    ProbImagePointer Dy = extract_one_component(1,IGVF);
    
    
    //Dxx
    FilterType::Pointer filter1 = FilterType::New();
    filter1->SetDirection(0);
    filter1->SetOrder(FilterType::FirstOrder);
    filter1->SetInput(Dx);
    filter1->SetSigma(1);
    filter1->Update();
    ProbImagePointer Dxx = filter1->GetOutput();
    
    //Dxy
    FilterType::Pointer filter2 = FilterType::New();
    filter2->SetDirection(1);
    filter2->SetOrder(FilterType::FirstOrder);
    filter2->SetInput(Dx);
    filter2->SetSigma(1);
    filter2->Update();
    ProbImagePointer Dxy = filter2->GetOutput();
    
    //Dyy
    FilterType::Pointer filter4 = FilterType::New();
    filter4->SetDirection(1);
    filter4->SetOrder(FilterType::FirstOrder);
    filter4->SetInput(Dy);
    filter4->SetSigma(1);
    filter4->Update();
    ProbImagePointer Dyy = filter4->GetOutput();
    
    
    typedef itk::ImageSliceIteratorWithIndex< ProbImageType > SliceIteratorType;
    typedef itk::ImageSliceIteratorWithIndex< GradientImageType > GradientSliceIteratorType;
    typedef itk::ImageSliceIteratorWithIndex< ImageType > ISliceIteratorType;
    
    SliceIteratorType inputIt1( Dxx, Dxx->GetRequestedRegion() );
    inputIt1.SetFirstDirection( 0 );
    inputIt1.SetSecondDirection( 1 );
    SliceIteratorType inputIt2( Dxy, Dxy->GetRequestedRegion() );
    inputIt2.SetFirstDirection( 0 );
    inputIt2.SetSecondDirection( 1 );
    SliceIteratorType inputIt4( Dyy, Dyy->GetRequestedRegion() );
    inputIt4.SetFirstDirection( 0 );
    inputIt4.SetSecondDirection( 1 );
    
    ISliceIteratorType inputIt7(I, I->GetRequestedRegion() );
    inputIt7.SetFirstDirection( 0 );
    inputIt7.SetSecondDirection( 1 );
    
    /*SliceIteratorType outputIt1(Vx1, Vx1->GetRequestedRegion() );
     outputIt1.SetFirstDirection( 2 );
     outputIt1.SetSecondDirection( 1 );
     SliceIteratorType outputIt2(Vy1, Vy1->GetRequestedRegion() );
     outputIt2.SetFirstDirection( 2 );
     outputIt2.SetSecondDirection( 1 );
     SliceIteratorType outputIt3(Vz1, Vz1->GetRequestedRegion() );
     outputIt3.SetFirstDirection( 2 );
     outputIt3.SetSecondDirection( 1 );*/
    
    GradientSliceIteratorType inputIt0( IGVF, IGVF->GetRequestedRegion() );
    inputIt0.SetFirstDirection( 0 );
    inputIt0.SetSecondDirection( 1 );
    
    SliceIteratorType outputIt4( IVessel, IVessel->GetRequestedRegion() );
    outputIt4.SetFirstDirection( 0 );
    outputIt4.SetSecondDirection( 1 );
    
    inputIt0.GoToBegin();
    inputIt1.GoToBegin();
    inputIt2.GoToBegin();
    inputIt4.GoToBegin();
    inputIt7.GoToBegin();
    
    outputIt4.GoToBegin();
    
    double Ma[2][2];
    double V[2][2];
    double d[2];
    
    while( !inputIt1.IsAtEnd() )
    {
        while ( !inputIt1.IsAtEndOfSlice() )
        {
            while ( !inputIt1.IsAtEndOfLine() )
            {
                if( inputIt7.Get() == 0 )
                    //if( 0 )
                {
                    outputIt4.Set(0);
                }
                else
                {
                    Ma[0][0] = inputIt1.Get();
                    Ma[0][1] = inputIt2.Get();
                    Ma[1][0] = inputIt2.Get();
                    Ma[1][1] = inputIt4.Get();
                    eigen_decomposition2D(Ma,V,d);
                    
                    double Lambda1 = d[0];
                    double Lambda2 = d[1];
                    if(Lambda2>=0.0)
                    {
                        outputIt4.Set(0);
                    }
                    else
                    {
                        double Rb  = Lambda1 / Lambda2;
                        double S  = vcl_sqrt( pow(Lambda1,2) + pow(Lambda2,2) );
                        double vesMeasure_2  =
                        vcl_exp ( -1.0 * ((vnl_math_sqr( Rb )) /  ( B )));
                        double vesMeasure_3  =
                        ( 1 - vcl_exp( -1.0 * (( vnl_math_sqr( S )) / ( C ))));
                        
                        float V_Saliency =  vesMeasure_2 * vesMeasure_3;
                        outputIt4.Set(V_Saliency);
                    }
                }
                ++inputIt0;
                ++inputIt1;
                ++inputIt2;
                ++inputIt4;
                ++inputIt7;
                ++outputIt4;
            }
            inputIt0.NextLine();
            inputIt1.NextLine();
            inputIt2.NextLine();
            inputIt4.NextLine();
            inputIt7.NextLine();
            outputIt4.NextLine();
        }
        inputIt0.NextSlice();
        inputIt1.NextSlice();
        inputIt2.NextSlice();
        inputIt4.NextSlice();
        inputIt7.NextSlice();
        outputIt4.NextSlice();
    }
    
    //rescale the vesselness to [0,255]
    typedef itk::RescaleIntensityImageFilter< ProbImageType, ProbImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    rescale->SetInput( IVessel );
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 255 );
    rescale->Update();
    IVessel = rescale->GetOutput();
    
    typedef itk::OtsuThresholdImageFilter<
    ProbImageType, ProbImageType > TH_FilterType;
    TH_FilterType::Pointer thresholder = TH_FilterType::New();
    thresholder->SetInput( IVessel );
    thresholder->SetOutsideValue( 1 );
    thresholder->SetInsideValue( 0 );
    thresholder->SetNumberOfHistogramBins( 256 );
    thresholder->Update();
    
    v_threshold = thresholder->GetThreshold();
    
    std::cout<<" - Selected Otsu Threshold:"<<v_threshold<<std::endl;
}

void ImageOperation::ComputeGVFVesselness()
{
    if( this->SZ == 1 )
    {
        this->ComputeGVFVesselness2D();
        return;
    }
    
    double FrangiAlpha = 0.5;
    double FrangiBeta = 0.5;
    double FrangiC = 10;
    double A = 2 * pow(FrangiAlpha,2);
    double B = 2 * pow(FrangiBeta,2);
    double C = 2 * pow(FrangiC,2);
    
    typedef itk::CastImageFilter<ImageType,ProbImageType> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(I);
    caster->Update();
    IVessel = caster->GetOutput();
    
    /*CasterType::Pointer caster1 = CasterType::New();
     caster1->SetInput(I);
     caster1->Update();
     Vx1 = caster1->GetOutput();
     
     
     CasterType::Pointer caster2 = CasterType::New();
     caster2->SetInput(I);
     caster2->Update();
     Vy1 = caster2->GetOutput();
     
     
     CasterType::Pointer caster3 = CasterType::New();
     caster3->SetInput(I);
     caster3->Update();
     Vz1 = caster3->GetOutput();*/
    
    
    typedef itk::RecursiveGaussianImageFilter<
    ProbImageType, ProbImageType > FilterType;
    //typedef itk::GradientImageFilter<ProbImageType, float, float> FilterType;
    ProbImagePointer Dx = extract_one_component(0,IGVF);
    ProbImagePointer Dy = extract_one_component(1,IGVF);
    ProbImagePointer Dz = extract_one_component(2,IGVF);
    
    
    /*FilterType::Pointer filter_x = FilterType::New();
     filter_x->SetInput(Dx);
     filter_x->Update();
     Dxx = extract_one_component(0,filter_x->GetOutput());
     Dxy = extract_one_component(1,filter_x->GetOutput());
     Dxz = extract_one_component(2,filter_x->GetOutput());
     
     FilterType::Pointer filter_y = FilterType::New();
     filter_y->SetInput(Dy);
     filter_y->Update();
     Dyy = extract_one_component(1,filter_y->GetOutput());
     Dyz = extract_one_component(2,filter_y->GetOutput());
     
     FilterType::Pointer filter_z = FilterType::New();
     filter_z->SetInput(Dz);
     filter_z->Update();
     Dzz = extract_one_component(2,filter_z->GetOutput());*/
    
    //Dxx
    FilterType::Pointer filter1 = FilterType::New();
    filter1->SetDirection(0);
    filter1->SetOrder(FilterType::FirstOrder);
    filter1->SetInput(Dx);
    filter1->SetSigma(1);
    filter1->Update();
    ProbImagePointer Dxx = filter1->GetOutput();
    
    //Dxy
    FilterType::Pointer filter2 = FilterType::New();
    filter2->SetDirection(1);
    filter2->SetOrder(FilterType::FirstOrder);
    filter2->SetInput(Dx);
    filter2->SetSigma(1);
    filter2->Update();
    ProbImagePointer Dxy = filter2->GetOutput();
    
    //Dxz
    FilterType::Pointer filter3 = FilterType::New();
    filter3->SetDirection(2);
    filter3->SetOrder(FilterType::FirstOrder);
    filter3->SetInput(Dx);
    filter3->SetSigma(1);
    filter3->Update();
    ProbImagePointer Dxz = filter3->GetOutput();
    
    //Dyy
    FilterType::Pointer filter4 = FilterType::New();
    filter4->SetDirection(1);
    filter4->SetOrder(FilterType::FirstOrder);
    filter4->SetInput(Dy);
    filter4->SetSigma(1);
    filter4->Update();
    ProbImagePointer Dyy = filter4->GetOutput();
    
    //Dyz
    FilterType::Pointer filter5 = FilterType::New();
    filter5->SetDirection(2);
    filter5->SetOrder(FilterType::FirstOrder);
    filter5->SetInput(Dy);
    filter5->SetSigma(1);
    filter5->Update();
    ProbImagePointer Dyz = filter5->GetOutput();
    
    //Dzz
    FilterType::Pointer filter6 = FilterType::New();
    filter6->SetDirection(2);
    filter6->SetOrder(FilterType::FirstOrder);
    filter6->SetInput(Dz);
    filter6->SetSigma(1);
    filter6->Update();
    ProbImagePointer Dzz = filter6->GetOutput();
    
    
    typedef itk::ImageSliceIteratorWithIndex< ProbImageType > SliceIteratorType;
    typedef itk::ImageSliceIteratorWithIndex< GradientImageType > GradientSliceIteratorType;
    typedef itk::ImageSliceIteratorWithIndex< ImageType > ISliceIteratorType;
    
    SliceIteratorType inputIt1( Dxx, Dxx->GetRequestedRegion() );
    inputIt1.SetFirstDirection( 0 );
    inputIt1.SetSecondDirection( 1 );
    SliceIteratorType inputIt2( Dxy, Dxy->GetRequestedRegion() );
    inputIt2.SetFirstDirection( 0 );
    inputIt2.SetSecondDirection( 1 );
    SliceIteratorType inputIt3( Dxz, Dxz->GetRequestedRegion() );
    inputIt3.SetFirstDirection( 0 );
    inputIt3.SetSecondDirection( 1 );
    SliceIteratorType inputIt4( Dyy, Dyy->GetRequestedRegion() );
    inputIt4.SetFirstDirection( 0 );
    inputIt4.SetSecondDirection( 1 );
    SliceIteratorType inputIt5( Dyz, Dyz->GetRequestedRegion() );
    inputIt5.SetFirstDirection( 0 );
    inputIt5.SetSecondDirection( 1 );
    SliceIteratorType inputIt6( Dzz, Dzz->GetRequestedRegion() );
    inputIt6.SetFirstDirection( 0 );
    inputIt6.SetSecondDirection( 1 );
    
    ISliceIteratorType inputIt7(I, I->GetRequestedRegion() );
    inputIt7.SetFirstDirection( 0 );
    inputIt7.SetSecondDirection( 1 );
    
    /*SliceIteratorType outputIt1(Vx1, Vx1->GetRequestedRegion() );
     outputIt1.SetFirstDirection( 2 );
     outputIt1.SetSecondDirection( 1 );
     SliceIteratorType outputIt2(Vy1, Vy1->GetRequestedRegion() );
     outputIt2.SetFirstDirection( 2 );
     outputIt2.SetSecondDirection( 1 );
     SliceIteratorType outputIt3(Vz1, Vz1->GetRequestedRegion() );
     outputIt3.SetFirstDirection( 2 );
     outputIt3.SetSecondDirection( 1 );*/
    
    GradientSliceIteratorType inputIt0( IGVF, IGVF->GetRequestedRegion() );
    inputIt0.SetFirstDirection( 0 );
    inputIt0.SetSecondDirection( 1 );
    
    SliceIteratorType outputIt4( IVessel, IVessel->GetRequestedRegion() );
    outputIt4.SetFirstDirection( 0 );
    outputIt4.SetSecondDirection( 1 );
    
    inputIt0.GoToBegin();
    inputIt1.GoToBegin();
    inputIt2.GoToBegin();
    inputIt3.GoToBegin();
    inputIt4.GoToBegin();
    inputIt5.GoToBegin();
    inputIt6.GoToBegin();
    inputIt7.GoToBegin();
    
    outputIt4.GoToBegin();
    
    double Ma[3][3];
    double V[3][3];
    double d[3];
    
    while( !inputIt1.IsAtEnd() )
    {
        while ( !inputIt1.IsAtEndOfSlice() )
        {
            while ( !inputIt1.IsAtEndOfLine() )
            {
                if( inputIt7.Get() == 0 )
                    //if( 0 )
                {
                    outputIt4.Set(0);
                }
                else
                {
                    Ma[0][0] = inputIt1.Get();
                    Ma[0][1] = inputIt2.Get();
                    Ma[0][2] = inputIt3.Get();
                    Ma[1][0] = inputIt2.Get();
                    Ma[1][1] = inputIt4.Get();
                    Ma[1][2] = inputIt5.Get();
                    Ma[2][0] = inputIt3.Get();
                    Ma[2][1] = inputIt5.Get();
                    Ma[2][2] = inputIt6.Get();
                    eigen_decomposition(Ma,V,d);
                    
                    double Lambda1 = d[0];
                    double Lambda2 = d[1];
                    double Lambda3 = d[2];
                    if(Lambda2>=0.0 || Lambda3>=0.0)
                    {
                        outputIt4.Set(0);
                    }
                    else
                    {
                        double Ra  = Lambda2 / Lambda3;
                        double Rb  = Lambda1 / vcl_sqrt ( vnl_math_abs( Lambda2 * Lambda3 ));
                        double S  = vcl_sqrt( pow(Lambda1,2) + pow(Lambda2,2) + pow(Lambda3,2) );
                        double vesMeasure_1  =
                        ( 1 - vcl_exp(-1.0*(( vnl_math_sqr( Ra ) ) / ( A ))));
                        double vesMeasure_2  =
                        vcl_exp ( -1.0 * ((vnl_math_sqr( Rb )) /  ( B )));
                        double vesMeasure_3  =
                        ( 1 - vcl_exp( -1.0 * (( vnl_math_sqr( S )) / ( C ))));
                        
                        float V_Saliency = vesMeasure_1 * vesMeasure_2 * vesMeasure_3;
                        //float V_Saliency = fabs(inputIt0.Get()[0] * (float)V[0][0] + inputIt0.Get()[1] * (float)V[0][1] + inputIt0.Get()[2] * (float)V[0][2]);
                        //std::cout<<Lambda1<<","<<Lambda2<<","<<Lambda3<<std::endl;
                        //std::cout<<V_Saliency<<std::endl;
                        outputIt4.Set(V_Saliency);
                    }
                }
                ++inputIt0;
                ++inputIt1;
                ++inputIt2;
                ++inputIt3;
                ++inputIt4;
                ++inputIt5;
                ++inputIt6;
                ++inputIt7;
                ++outputIt4;
            }
            inputIt0.NextLine();
            inputIt1.NextLine();
            inputIt2.NextLine();
            inputIt3.NextLine();
            inputIt4.NextLine();
            inputIt5.NextLine();
            inputIt6.NextLine();
            inputIt7.NextLine();
            outputIt4.NextLine();
        }
        inputIt0.NextSlice();
        inputIt1.NextSlice();
        inputIt2.NextSlice();
        inputIt3.NextSlice();
        inputIt4.NextSlice();
        inputIt5.NextSlice();
        inputIt6.NextSlice();
        inputIt7.NextSlice();
        outputIt4.NextSlice();
    }
    
    //rescale the vesselness to [0,255]
    typedef itk::RescaleIntensityImageFilter< ProbImageType, ProbImageType> RescaleFilterType;
    RescaleFilterType::Pointer rescale = RescaleFilterType::New();
    rescale->SetInput( IVessel );
    rescale->SetOutputMinimum( 0 );
    rescale->SetOutputMaximum( 255 );
    rescale->Update();
    IVessel = rescale->GetOutput();
    
    typedef itk::OtsuThresholdImageFilter<
    ProbImageType, ProbImageType > TH_FilterType;
    TH_FilterType::Pointer thresholder = TH_FilterType::New();
    thresholder->SetInput( IVessel );
    thresholder->SetOutsideValue( 1 );
    thresholder->SetInsideValue( 0 );
    thresholder->SetNumberOfHistogramBins( 256 );
    thresholder->Update();
    
    v_threshold = thresholder->GetThreshold();
    
    std::cout<<" - Selected Otsu Threshold:"<<v_threshold<<std::endl;
}


ProbImagePointer extract_one_component(int index, GradientImagePointer G)
{
    typedef itk::ImageAdaptor<  GradientImageType, 
    VectorPixelAccessor > ImageAdaptorType;
	ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
	VectorPixelAccessor  accessor;
    accessor.SetIndex( index );
    adaptor->SetPixelAccessor( accessor );
    adaptor->SetImage( G );
    typedef itk::CastImageFilter<ImageAdaptorType,ProbImageType> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(adaptor);
    caster->Update();
	ProbImagePointer output = caster->GetOutput();
    
    return output;
}

void eigen_decomposition2D(double A[2][2], double V[2][2], double d[2])
{
    double tmp = sqrt(pow(A[0][0]-A[1][1],2) + 4 * pow(A[0][1],2));
    double v1x = 2 * A[1][0];
    double v1y = A[1][1] - A[0][0] + tmp;
    double mag = sqrt(pow(v1x,2)+pow(v1y,2));
    
    v1x /= (mag + std::numeric_limits<float>::epsilon());
    v1y /= (mag + std::numeric_limits<float>::epsilon());
    
    double v2x = -1 * v1y;
    double v2y = v1x;
    
    double mu1 = 0.5 * (A[0][0] + A[1][1] + tmp);
    double mu2 = 0.5 * (A[0][0] + A[1][1] - tmp);
    
    if( fabs(mu1) > fabs(mu2) )
    {
        V[0][0] = v2x;
        V[1][0] = v2y;
        V[0][1] = v1x;
        V[1][1] = v1y;
        d[0] = mu2;
        d[1] = mu1;
    }
    else
    {
        V[0][0] = v1x;
        V[1][0] = v1y;
        V[0][1] = v2x;
        V[1][1] = v2y;
        d[0] = mu1;
        d[1] = mu2;
    }
    
}


void eigen_decomposition(double A[3][3], double V[3][3], double d[3]) {
	int n = 3;
    double e[3];
    double da[3];
    double dt, dat;
    double vet[3];
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            V[i][j] = A[i][j];
        }
    }
    tred2(V, d, e);
    tql2(V, d, e);
    
    /* Sort the eigen values and vectors by abs eigen value */
    da[0]=absd(d[0]); da[1]=absd(d[1]); da[2]=absd(d[2]);
    if((da[0]>=da[1])&&(da[0]>da[2]))
    {
        dt=d[2];   dat=da[2];    vet[0]=V[0][2];    vet[1]=V[1][2];    vet[2]=V[2][2];
        d[2]=d[0]; da[2]=da[0];  V[0][2] = V[0][0]; V[1][2] = V[1][0]; V[2][2] = V[2][0];
        d[0]=dt;   da[0]=dat;    V[0][0] = vet[0];  V[1][0] = vet[1];  V[2][0] = vet[2]; 
    }
    else if((da[1]>=da[0])&&(da[1]>da[2]))  
    {
        dt=d[2];   dat=da[2];    vet[0]=V[0][2];    vet[1]=V[1][2];    vet[2]=V[2][2];
        d[2]=d[1]; da[2]=da[1];  V[0][2] = V[0][1]; V[1][2] = V[1][1]; V[2][2] = V[2][1];
        d[1]=dt;   da[1]=dat;    V[0][1] = vet[0];  V[1][1] = vet[1];  V[2][1] = vet[2]; 
    }
    if(da[0]>da[1])
    {
        dt=d[1];   dat=da[1];    vet[0]=V[0][1];    vet[1]=V[1][1];    vet[2]=V[2][1];
        d[1]=d[0]; da[1]=da[0];  V[0][1] = V[0][0]; V[1][1] = V[1][0]; V[2][1] = V[2][0];
        d[0]=dt;   da[0]=dat;    V[0][0] = vet[0];  V[1][0] = vet[1];  V[2][0] = vet[2]; 
    }
}

static void tred2(double V[3][3], double d[3], double e[3]) {
    
	int n = 3;
    /*  This is derived from the Algol procedures tred2 by */
    /*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
    /*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
    /*  Fortran subroutine in EISPACK. */
    int i, j, k;
    double scale;
    double f, g, h;
    double hh;
    for (j = 0; j < n; j++) {d[j] = V[n-1][j]; }
    
    /* Householder reduction to tridiagonal form. */
    
    for (i = n-1; i > 0; i--) {
        /* Scale to avoid under/overflow. */
        scale = 0.0;
        h = 0.0;
        for (k = 0; k < i; k++) { scale = scale + fabs(d[k]); }
        if (scale == 0.0) {
            e[i] = d[i-1];
            for (j = 0; j < i; j++) { d[j] = V[i-1][j]; V[i][j] = 0.0;  V[j][i] = 0.0; }
        } else {
            
            /* Generate Householder vector. */
            
            for (k = 0; k < i; k++) { d[k] /= scale; h += d[k] * d[k]; }
            f = d[i-1];
            g = sqrt(h);
            if (f > 0) { g = -g; }
            e[i] = scale * g;
            h = h - f * g;
            d[i-1] = f - g;
            for (j = 0; j < i; j++) { e[j] = 0.0; }
            
            /* Apply similarity transformation to remaining columns. */
            
            for (j = 0; j < i; j++) {
                f = d[j];
                V[j][i] = f;
                g = e[j] + V[j][j] * f;
                for (k = j+1; k <= i-1; k++) { g += V[k][j] * d[k]; e[k] += V[k][j] * f; }
                e[j] = g;
            }
            f = 0.0;
            for (j = 0; j < i; j++) { e[j] /= h; f += e[j] * d[j]; }
            hh = f / (h + h);
            for (j = 0; j < i; j++) { e[j] -= hh * d[j]; }
            for (j = 0; j < i; j++) {
                f = d[j]; g = e[j];
                for (k = j; k <= i-1; k++) { V[k][j] -= (f * e[k] + g * d[k]); }
                d[j] = V[i-1][j];
                V[i][j] = 0.0;
            }
        }
        d[i] = h;
    }
    
    /* Accumulate transformations. */
    
    for (i = 0; i < n-1; i++) {
        V[n-1][i] = V[i][i];
        V[i][i] = 1.0;
        h = d[i+1];
        if (h != 0.0) {
            for (k = 0; k <= i; k++) { d[k] = V[k][i+1] / h;}
            for (j = 0; j <= i; j++) {
                g = 0.0;
                for (k = 0; k <= i; k++) { g += V[k][i+1] * V[k][j]; }
                for (k = 0; k <= i; k++) { V[k][j] -= g * d[k]; }
            }
        }
        for (k = 0; k <= i; k++) { V[k][i+1] = 0.0;}
    }
    for (j = 0; j < n; j++) { d[j] = V[n-1][j]; V[n-1][j] = 0.0; }
    V[n-1][n-1] = 1.0;
    e[0] = 0.0;
}

/* Symmetric tridiagonal QL algorithm. */
static void tql2(double V[3][3], double d[3], double e[3]) {
    
	int n = 3;
    /*  This is derived from the Algol procedures tql2, by */
    /*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
    /*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
    /*  Fortran subroutine in EISPACK. */
    
    int i, j, k, l, m;
    double f;
    double tst1;
    double eps;
    int iter;
    double g, p, r;
    double dl1, h, c, c2, c3, el1, s, s2;
    
    for (i = 1; i < n; i++) { e[i-1] = e[i]; }
    e[n-1] = 0.0;
    
    f = 0.0;
    tst1 = 0.0;
    eps = pow(2.0, -52.0);
    for (l = 0; l < n; l++) {
        
        /* Find small subdiagonal element */
        
        tst1 = MAX_snake(tst1, fabs(d[l]) + fabs(e[l]));
        m = l;
        while (m < n) {
            if (fabs(e[m]) <= eps*tst1) { break; }
            m++;
        }
        
        /* If m == l, d[l] is an eigenvalue, */
        /* otherwise, iterate. */
        
        if (m > l) {
            iter = 0;
            do {
                iter = iter + 1;  /* (Could check iteration count here.) */
                /* Compute implicit shift */
                g = d[l];
                p = (d[l+1] - g) / (2.0 * e[l]);
                r = hypot2(p, 1.0);
                if (p < 0) { r = -r; }
                d[l] = e[l] / (p + r);
                d[l+1] = e[l] * (p + r);
                dl1 = d[l+1];
                h = g - d[l];
                for (i = l+2; i < n; i++) { d[i] -= h; }
                f = f + h;
                /* Implicit QL transformation. */
                p = d[m]; c = 1.0; c2 = c; c3 = c;
                el1 = e[l+1]; s = 0.0; s2 = 0.0;
                for (i = m-1; i >= l; i--) {
                    c3 = c2;
                    c2 = c;
                    s2 = s;
                    g = c * e[i];
                    h = c * p;
                    r = hypot2(p, e[i]);
                    e[i+1] = s * r;
                    s = e[i] / r;
                    c = p / r;
                    p = c * d[i] - s * g;
                    d[i+1] = h + s * (c * g + s * d[i]);
                    /* Accumulate transformation. */
                    for (k = 0; k < n; k++) {
                        h = V[k][i+1];
                        V[k][i+1] = s * V[k][i] + c * h;
                        V[k][i] = c * V[k][i] - s * h;
                    }
                }
                p = -s * s2 * c3 * el1 * e[l] / dl1;
                e[l] = s * p;
                d[l] = c * p;
                
                /* Check for convergence. */
            } while (fabs(e[l]) > eps*tst1);
        }
        d[l] = d[l] + f;
        e[l] = 0.0;
    }
    
    /* Sort eigenvalues and corresponding vectors. */
    for (i = 0; i < n-1; i++) {
        k = i;
        p = d[i];
        for (j = i+1; j < n; j++) {
            if (d[j] < p) {
                k = j;
                p = d[j];
            }
        }
        if (k != i) {
            d[k] = d[i];
            d[i] = p;
            for (j = 0; j < n; j++) {
                p = V[j][i];
                V[j][i] = V[j][k];
                V[j][k] = p;
            }
        }
    }
}

double MAX_snake(double a, double b)
{
    return ((a)>(b)?(a):(b));
}

static double hypot2(double x, double y) { return sqrt(x*x+y*y); }

double absd(double val){ if(val>0){ return val;} else { return -val;} };

PGfloat ***pgDmatrix(int M, int N, int K)
{
    PGfloat ***Image = NULL;
    int i,j;
    
    Image = (PGfloat ***)malloc(M*sizeof(PGfloat **)); /* may need another * in sizeof */
    for(i=0; i<M; i++)
    {
        Image[i] = (PGfloat **)malloc(N*sizeof(PGfloat*));
        for(j=0; j<N; j++)
        {
            Image[i][j] = (PGfloat *)malloc(K*sizeof(PGfloat));
        }
    }
    
    return Image;
}

/* Frees a matrix allocated by dmatrix */
PGvoid pgFreeDmatrix(PGfloat ***Image, int M, int N, int K)
{
    int i,j;
    for(i=0; i<M; i++)
    {
        for(j=0; j<N; j++)
        {
            free(Image[i][j]);
        }
        free(Image[i]);
    }
    free(Image);
}

void GVFC(int XN, int YN, int ZN, float *f, float *ou, float *ov, float *oo, 
          float mu, int ITER)
{
    double mag2, temp, tempx, tempy, tempz, fmax, fmin;
    int count, x, y, z, XN_1, XN_2, YN_1, YN_2, ZN_1, ZN_2;
    
    PGfloat ***fx, ***fy, ***fz, ***u, ***v, ***o, ***Lu, ***Lv, ***Lo, ***g, ***c1, ***c2, ***c3, ***b;
    
    /* define constants and create row-major double arrays */
    XN_1 = XN - 1;
    XN_2 = XN - 2;
    YN_1 = YN - 1;
    YN_2 = YN - 2;
    ZN_1 = ZN - 1;
    ZN_2 = ZN - 2;
    
    fx = pgDmatrix(YN,XN,ZN);
    fy = pgDmatrix(YN,XN,ZN);
    fz = pgDmatrix(YN,XN,ZN);
    u = pgDmatrix(YN,XN,ZN);
    v = pgDmatrix(YN,XN,ZN);
    o = pgDmatrix(YN,XN,ZN);
    Lu = pgDmatrix(YN,XN,ZN);
    Lv = pgDmatrix(YN,XN,ZN);
    Lo = pgDmatrix(YN,XN,ZN);
    g = pgDmatrix(YN,XN,ZN);
    c1 = pgDmatrix(YN,XN,ZN);
    c2 = pgDmatrix(YN,XN,ZN);
    c3 = pgDmatrix(YN,XN,ZN);
    b = pgDmatrix(YN,XN,ZN);
    
    
    /************** I: Normalize the edge map to [0,1] **************/
    fmax = -1e10;
    fmin = 1e10;
    for (x=0; x<=YN*XN*ZN-1; x++) {
        fmax = PG_MAX(fmax,f[x]);
        fmin = PG_MIN(fmin,f[x]);
    } 
    
    for (x=0; x<=YN*XN*ZN-1; x++) 
        f[x] = (f[x]-fmin)/(fmax-fmin);
    
    
    /**************** II: Compute edge map gradient *****************/
    /* I.1: Neumann boundary condition: 
     *      zero normal derivative at boundary 
     */
    /* Deal with corners */
    for(z=0; z<= ZN_1; z++)
        for (y=0; y<=YN_1; y++){
            fx[y][0][z] = fy[y][0][z] = fz[y][0][z] = 0;
            fx[y][XN_1][z] = fy[y][XN_1][z] = fz[y][XN_1][z] = 0;
        }
    for(x=0; x<= XN_1; x++)
        for (y=0; y<=YN_1; y++){
            fx[y][x][0] = fy[y][x][0] = fz[y][x][0] = 0;
            fx[y][x][ZN_1] = fy[y][x][ZN_1] = fz[y][x][ZN_1] = 0;
        }
    for(x=0; x<= XN_1; x++)
        for (z=0; z<=ZN_1; z++){
            fx[0][x][z] = fy[0][x][z] = fz[0][x][z] = 0;
            fx[YN_1][x][z] = fy[YN_1][x][z] = fz[YN_1][x][z] = 0;
        }
    
    /* I.2: Compute interior derivative using central difference */
    for(z=1; z<= ZN_2; z++)
        for (y=1; y<=YN_2; y++)
            for (x=1; x<=XN_2; x++) {
                /* NOTE: f is stored in column major */
                fx[y][x][z] = 0.5 * (f[y + (x+1)*YN + z*YN*XN] - f[y + (x-1)*YN + z*YN*XN]); 	 
                fy[y][x][z] = 0.5 * (f[y+1 + x*YN + z*YN*XN] - f[y-1 + x*YN + z*YN*XN]);
                fz[y][x][z] = 0.5 * (f[y + x*YN + (z+1)*YN*XN] - f[y + x*YN + (z-1)*YN*XN]);
            }
    
    /******* III: Compute parameters and initializing arrays **********/
    temp = -1.0/(mu*mu);
    for(z=0; z<=ZN_1; z++)
        for (y=0; y<=YN_1; y++)
            for (x=0; x<=XN_1; x++) {
                tempx = fx[y][x][z];
                tempy = fy[y][x][z];
                tempz = fz[y][x][z];
                /* initial GVF vector */
                u[y][x][z] = tempx;
                v[y][x][z] = tempy;
                o[y][x][z] = tempz;
                /* gradient magnitude square */
                mag2 = tempx*tempx + tempy*tempy + tempz*tempz; 
                
                g[y][x][z] = mu;
                b[y][x][z] = mag2;
                
                c1[y][x][z] = b[y][x][z] * tempx;
                c2[y][x][z] = b[y][x][z] * tempy;
                c3[y][x][z] = b[y][x][z] * tempz;
            }
    
    /* free memory of fx, fy and fz */
    pgFreeDmatrix(fx,YN,XN,ZN);
    pgFreeDmatrix(fy,YN,XN,ZN);
    pgFreeDmatrix(fz,YN,XN,ZN);
    
    for(z=0; z<= ZN_1; z++)
        for (y=0; y<=YN_1; y++){
            Lu[y][0][z] = Lv[y][0][z] = Lo[y][0][z] = 0;
            Lu[y][XN_1][z] = Lv[y][XN_1][z] = Lo[y][XN_1][z] = 0;
        }
    for(x=0; x<= XN_1; x++)
        for (y=0; y<=YN_1; y++){
            Lu[y][x][0] = Lv[y][x][0] = Lo[y][x][0] = 0;
            Lu[y][x][ZN_1] = Lv[y][x][ZN_1] = Lo[y][x][ZN_1] = 0;
        }
    for(x=0; x<= XN_1; x++)
        for (z=0; z<=ZN_1; z++){
            Lu[0][x][z] = Lv[0][x][z] = Lo[0][x][z] = 0;
            Lu[YN_1][x][z] = Lv[YN_1][x][z] = Lo[YN_1][x][z] = 0;
        }
    
    /************* Solve GVF = (u,v) iteratively ***************/
    for (count=1; count<=ITER; count++) {
        /* IV: Compute Laplace operator using Neuman condition */
        
        /* IV.4: Compute interior */
        for(z=1; z<=ZN_2; z++)
            for (y=1; y<=YN_2; y++)
                for (x=1; x<=XN_2; x++) {
                    Lu[y][x][z] = (u[y][x-1][z] + u[y][x+1][z] 
                                   + u[y-1][x][z] + u[y+1][x][z] + u[y][x][z-1] + u[y][x][z+1])*1/6 - u[y][x][z];
                    Lv[y][x][z] = (v[y][x-1][z] + v[y][x+1][z] 
                                   + v[y-1][x][z] + v[y+1][x][z] + v[y][x][z-1] + v[y][x][z+1])*1/6- v[y][x][z];
                    Lo[y][x][z] = (o[y][x-1][z] + o[y][x+1][z] 
                                   + o[y-1][x][z] + o[y+1][x][z] + o[y][x][z-1] + o[y][x][z+1])*1/6 - o[y][x][z];
                }
        
        /******** V: Update GVF ************/
        for(z=0; z<=ZN_1; z++)
            for (y=0; y<=YN_1; y++)
                for (x=0; x<=XN_1; x++) {
                    u[y][x][z] = (1- b[y][x][z])*u[y][x][z] + g[y][x][z]*Lu[y][x][z]*4 + c1[y][x][z];
                    v[y][x][z] = (1- b[y][x][z])*v[y][x][z] + g[y][x][z]*Lv[y][x][z]*4 + c2[y][x][z];
                    o[y][x][z] = (1- b[y][x][z])*o[y][x][z] + g[y][x][z]*Lo[y][x][z]*4 + c3[y][x][z];
                }
        
        /* print iteration number */
        printf("%5d",count);
        if (count%15 == 0)
            printf("\n");
    }
    printf("\n");
    
    /* copy u,v to the output in column major order */
    for(z=0; z<= ZN_1; z++)
        for (y=0; y<=YN_1; y++)
            for (x=0; x<=XN_1; x++) {
                ou[z*XN*YN+x*YN+y] = u[y][x][z];
                ov[z*XN*YN+x*YN+y] = v[y][x][z];
                oo[z*XN*YN+x*YN+y] = o[y][x][z];
            }
    
    /* free all the array memory */
    
    pgFreeDmatrix(u,YN,XN,ZN);
    pgFreeDmatrix(v,YN,XN,ZN);
    pgFreeDmatrix(o,YN,XN,ZN);
    pgFreeDmatrix(Lu,YN,XN,ZN);
    pgFreeDmatrix(Lv,YN,XN,ZN);
    pgFreeDmatrix(Lo,YN,XN,ZN);
    pgFreeDmatrix(g,YN,XN,ZN);
    pgFreeDmatrix(c1,YN,XN,ZN);
    pgFreeDmatrix(c2,YN,XN,ZN);
    pgFreeDmatrix(c3,YN,XN,ZN);
    pgFreeDmatrix(b,YN,XN,ZN);
    
    return;
}

float norm_density(float x, float mu, float sigma)
{
    float pi = 3.1415926;
    float p;
    p = 1/MAX_snake(sigma * sqrt(2*pi),std::numeric_limits<float>::epsilon()) * exp( -pow(x-mu,2)/MAX_snake(2*pow(sigma,2),std::numeric_limits<float>::epsilon()));
    return p;
}
