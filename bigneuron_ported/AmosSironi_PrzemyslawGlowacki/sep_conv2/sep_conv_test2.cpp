

#include <iostream>
#include <fstream>
#include <string>
#include "include/Eigen/Dense"
#include "superannotator/Matrix3D.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConvolutionImageFilter.h"
#include "itkImageRandomNonRepeatingConstIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"


#include <omp.h>

using namespace std;
using namespace Eigen;

#define MAXBUFSIZE  ((int) 1e6)

typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;

MatrixTypeDouble readMatrix(const char *filename);

template<typename ImageType, typename VectorType>
static void prepare1DKernel( typename ImageType::Pointer &kernel, const unsigned dir, const VectorType &origVec );


template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, Matrix3D<float> &out);

//template<typename ImageType, typename VectorType>
//static void itkImage2EigenVector( typename ImageType::Pointer &input_img, VectorType &out_vector,const unsigned int n_rand_samples,const unsigned int tot_n_pixels);

template<typename ImageType, typename VectorType>
VectorType itkImage2EigenVector( typename ImageType::Pointer &input_img,const unsigned int n_rand_samples,const unsigned int tot_n_pixels);

template<typename ImageType, typename VectorType>
typename ImageType::Pointer eigenVector2itkImage(const VectorType &input_vector, const typename ImageType::SizeType &size_image);




int main () {


    ////load filters
    const char *weight_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_weigths_cpd_rank_25_rot_n1.txt";
    const char *sep_filters_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_sep_cpd_rank_25_rot_n1.txt";

    MatrixXd weights = readMatrix(weight_file);
    MatrixXd sep_filters = readMatrix(sep_filters_file);


    //std::cout << weights << std::endl;
   // std::cout << weights.rows() << std::endl; //25 ( rows of W = number of sep filters )
   // std::cout << weights.cols() << std::endl;//30 ( cols of W = number of original filters )

    //std::cout << sep_filters << std::endl;
   // std::cout << sep_filters.rows() << std::endl; //75 ( rows of S = actual sep filters stored as a column )
   // std::cout << sep_filters.cols() << std::endl;//25 ( cols of S = number of separable filters )


    ////load itk image

    const char *input_image_file ="/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2.nrrd";
    typedef float ImageScalarType;
    typedef itk::Image< ImageScalarType, 3 >         ITKImageType;
    typedef itk::ImageFileReader<ITKImageType> ReaderType;

    typedef  itk::ImageFileWriter< ITKImageType  > WriterType;

  ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(input_image_file);

//convolve with constant filter
//typename ImageType::Pointer &kernel;
  //  typedef float ImageScalarType;
   // typedef itk::Image<ImageScalarType, 3>   ITKImageType;

     ITKImageType::Pointer input_image = reader->GetOutput();
     input_image->Update();
   //   ITKImageType::RegionType region = input_image->GetLargestPossibleRegion();
      ITKImageType::SizeType size_image = input_image->GetLargestPossibleRegion().GetSize();



      ////rescale in [0 1]
       typedef itk::RescaleIntensityImageFilter< ITKImageType, ITKImageType > RescaleFilterType;
      RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
      rescaleFilter->SetInput(reader->GetOutput());
      rescaleFilter->SetOutputMinimum(0.0);
      rescaleFilter->SetOutputMaximum(1.0);


    ////convolve image
    ITKImageType::Pointer kernelX = ITKImageType::New();
    ITKImageType::Pointer kernelY = ITKImageType::New();
    ITKImageType::Pointer kernelZ = ITKImageType::New();

    const int kernel_size = (int)(sep_filters.rows()/3);

   // std::cout << kernel_size << std::endl;
      //    VectorType kernel_x_eig = sep_filters.row(1).segment(1,(int)(sep_filters.rows()/3));
            VectorTypeDouble kernel_eig = sep_filters.col(1);
            VectorTypeDouble kernel_x_eig;
            VectorTypeDouble kernel_y_eig;
            VectorTypeDouble kernel_z_eig;

          //  std::cout << kernel_eig << std::endl;


            kernel_x_eig = kernel_eig.head(kernel_size);
            kernel_y_eig = kernel_eig.segment(kernel_size,kernel_size);
            kernel_z_eig = kernel_eig.tail(kernel_size);

         //std::cout << kernel_x_eig << std::endl;
         //std::cout << kernel_x_eig.size() << std::endl;
         //std::cout << kernel_y_eig.size() << std::endl;
         //std::cout << kernel_z_eig.size() << std::endl;

            typedef itk::ConvolutionImageFilter< ITKImageType, ITKImageType, ITKImageType >  ConvolutionType;
         prepare1DKernel<ITKImageType, VectorTypeDouble>( kernelX, 0, kernel_x_eig );
         prepare1DKernel<ITKImageType, VectorTypeDouble>( kernelY, 1, kernel_y_eig );
         prepare1DKernel<ITKImageType, VectorTypeDouble>( kernelZ, 2, kernel_z_eig );


         ConvolutionType::Pointer convX, convY, convZ;

         convX = ConvolutionType::New();
         convX->SetInput( rescaleFilter->GetOutput() );
         convX->SetKernelImage(kernelX);
         convX->SetOutputRegionModeToSame();

         convY = ConvolutionType::New();
         convY->SetInput( convX->GetOutput() );
         convY->SetKernelImage(kernelY);
         convY->SetOutputRegionModeToSame();

         convZ = ConvolutionType::New();
         convZ->SetInput( convY->GetOutput() );
         convZ->SetKernelImage(kernelZ);
         convZ->SetOutputRegionModeToSame();

        convZ->Update();

 //out->loadItkImage( convZ->GetOutput() );
bool save_image_0 =0;
if(save_image_0)
{
    ////save image
    const char *output_image_file ="/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2_convolved.nrrd";

   // typedef  itk::ImageFileWriter< ITKImageType  > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(output_image_file);
      writer->SetInput(convZ->GetOutput());
      writer->Update();
}

//const unsigned int n_sep_features =3;
      const unsigned int n_sep_features = sep_filters.cols();
      const unsigned int n_pixels = size_image[0]*size_image[1]*size_image[2];
      //number of random smaples used for training
     // const unsigned int n_samples_per_image = 10;
 const unsigned int n_samples_per_image = n_pixels;



      ////convolve with filter bank
     std::cout << "convolving with separable filters" << std::endl;
    MatrixTypeFloat sep_features_all(n_samples_per_image,n_sep_features);

//std::cout << "n filter "<<n_sep_features << std::endl;
//std::cout << "n pixels "<<n_pixels << std::endl;
//std::cout << "n rows "<< size_image[0] << std::endl;
//std::cout << "n cols "<< size_image[1] << std::endl;
//std::cout << "n sli "<< size_image[2] << std::endl;



bool convolve_image = 1;
if(convolve_image){

    //#pragma omp parallel for
    for(unsigned int i_filter = 0; i_filter<n_sep_features ;i_filter++)
    {
        std::cout << "filter"<<i_filter << std::endl;
        const VectorTypeDouble kernel_eig = sep_filters.col(i_filter);

     //   std::cout << "loaded filter" << std::endl;

        Matrix3D<float> out_matrix;

        ITKImageType::Pointer in_img = rescaleFilter->GetOutput();
        wholeConvolveSepFilter<ITKImageType, VectorTypeDouble >( in_img, kernel_eig, out_matrix);

    //    std::cout << "convolved image" << std::endl;

        char buffer_out_name [500];
        //char *output_image_file_i_filter;
        const int temp_int =sprintf(buffer_out_name,"/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2_convolved_%i.nrrd",i_filter);

    //    std::cout << buffer_out_name << std::endl;
        out_matrix.save(buffer_out_name);


        // store result in coumns of a matrix
        ITKImageType::Pointer convolved_img= out_matrix.asItkImage();
        VectorTypeFloat temp_vector = itkImage2EigenVector<ITKImageType,VectorTypeFloat>(convolved_img,n_samples_per_image,n_pixels);

     //    std::cout << "copied vector" << std::endl;

       // std::cout << sep_features_all.col(i_filter).size()<< std::endl;
       // std::cout << temp_vector.size()<< std::endl;

       //  std::cout << "temp v"<< temp_vector << std::endl;

        sep_features_all.col(i_filter) = temp_vector;

    }//end for i_filter


  //  std::cout << sep_features_all << std::endl;

    std::cout << "min sep: " << sep_features_all.minCoeff() << std::endl;
    std::cout << "max sep: "<<sep_features_all.maxCoeff()   << std::endl;


    //multiply with weight to get original filters convolution
    const unsigned int n_nonsep_features = weights.cols();
    const float scale_factor = 1.0; // used for multiscale appraoch if rescale filers to compute features
    MatrixTypeFloat nonsep_features_all(n_samples_per_image,n_nonsep_features);

    nonsep_features_all = sep_features_all*weights.cast<float>() ;
    nonsep_features_all = nonsep_features_all/(scale_factor*scale_factor*scale_factor);

   std::cout << "min all: " << nonsep_features_all.minCoeff() << std::endl;
   std::cout << "max all: "<<nonsep_features_all.maxCoeff()   << std::endl;

    //std::cout << nonsep_features_all << std::endl;
   VectorTypeFloat convolved_vector = nonsep_features_all.col(0);

   std::cout << "min first col: " << convolved_vector.minCoeff() << std::endl;
   std::cout << "max first col: "<<convolved_vector.maxCoeff()   << std::endl;

   ITKImageType::Pointer convolved_img_orig = eigenVector2itkImage<ITKImageType,VectorTypeFloat>(convolved_vector,size_image);


   WriterType::Pointer writer = WriterType::New();
   const char *output_conv_file ="/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2_convolved_orig.nrrd";

   writer->SetFileName(output_conv_file);
   writer->SetInput(convolved_img_orig);
   writer->Update();


}


  return 0;
}

MatrixTypeDouble readMatrix(const char *filename)
    {
    int cols = 0, rows = 0;
    double buff[MAXBUFSIZE];

    // Read numbers from file into buffer.
    ifstream infile;
    infile.open(filename);
    while (! infile.eof())
        {
        string line;
        getline(infile, line);

        int temp_cols = 0;
        stringstream stream(line);
        while(! stream.eof())
            stream >> buff[cols*rows+temp_cols++];

        if (temp_cols == 0)
            continue;

        if (cols == 0)
            cols = temp_cols;

        rows++;
        }

    infile.close();

    rows--;

    // Populate matrix with numbers.
    MatrixTypeDouble result(rows,cols);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result(i,j) = buff[ cols*i+j ];

    return result;
    };





// helper for itk separable filter
// dir is [0,1,2]
template<typename ImageType, typename VectorType>
static void prepare1DKernel( typename ImageType::Pointer &kernel, const unsigned dir, const VectorType &origVec )
{
typename ImageType::IndexType start;
start.Fill(0);

VectorType vec = origVec.reverse();

typename ImageType::SizeType size;
size.Fill(1);
size[dir] = vec.size();

typename ImageType::RegionType region;
region.SetSize(size);
region.SetIndex(start);

kernel->SetRegions(region);
kernel->Allocate();

itk::ImageRegionIterator<ImageType> imageIterator(kernel, region);

unsigned i=0;
while(!imageIterator.IsAtEnd())
{
//imageIterator.Set(255);
imageIterator.Set( vec.coeff(i) );

++imageIterator;
++i;
}
}




template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, Matrix3D<float> &out){

    //    VectorType kernel_x_eig = sep_filters.row(1).segment(1,(int)(sep_filters.rows()/3));
        //VectorType kernel_eig = sep_filters.col(i_filter);
         VectorType kernel_x_eig;
         VectorType kernel_y_eig;
         VectorType kernel_z_eig;


        typename ImageType::Pointer kernelX = ImageType::New();
        typename ImageType::Pointer kernelY = ImageType::New();
        typename ImageType::Pointer kernelZ = ImageType::New();

      //  std::cout << kernel_eig << std::endl;


         const int kernel_size = (int)(kernel_eig.rows()/3);


        kernel_x_eig = kernel_eig.head(kernel_size);
        kernel_y_eig = kernel_eig.segment(kernel_size,kernel_size);
        kernel_z_eig = kernel_eig.tail(kernel_size);

     //std::cout << kernel_x_eig << std::endl;
     //std::cout << kernel_x_eig.size() << std::endl;
     //std::cout << kernel_y_eig.size() << std::endl;
     //std::cout << kernel_z_eig.size() << std::endl;

        typedef itk::ConvolutionImageFilter< ImageType, ImageType, ImageType >  ConvolutionType;
     prepare1DKernel< ImageType,  VectorType>( kernelX, 0, kernel_x_eig );
     prepare1DKernel< ImageType, VectorType>( kernelY, 1, kernel_y_eig );
     prepare1DKernel< ImageType, VectorType>( kernelZ, 2, kernel_z_eig );


     typename ConvolutionType::Pointer convX, convY, convZ;

     convX = ConvolutionType::New();
     convX->SetInput( input_img );
     convX->SetKernelImage(kernelX);
     convX->SetOutputRegionModeToSame();

     convY = ConvolutionType::New();
     convY->SetInput( convX->GetOutput() );
     convY->SetKernelImage(kernelY);
     convY->SetOutputRegionModeToSame();

     convZ = ConvolutionType::New();
     convZ->SetInput( convY->GetOutput() );
     convZ->SetKernelImage(kernelZ);
     convZ->SetOutputRegionModeToSame();

    convZ->Update();

   // std::cout << "convolving image" << std::endl;


    out.loadItkImage( convZ->GetOutput() );

}



template<typename ImageType, typename VectorType>
VectorType itkImage2EigenVector( typename ImageType::Pointer &input_img,const unsigned int n_rand_samples,const unsigned int  tot_n_pixels){


    VectorType out_vector;
    out_vector.setZero(n_rand_samples);

    if(tot_n_pixels < n_rand_samples){

        //error
        printf ("Error: Tot number of pixels in image (%i) must be smaller to number or samples (%i)\n",tot_n_pixels,n_rand_samples);
        return out_vector;
    }
    else if(tot_n_pixels > n_rand_samples)
    {


        itk::ImageRandomNonRepeatingConstIteratorWithIndex<ImageType> imageIterator(input_img, input_img->GetLargestPossibleRegion());
        imageIterator.SetNumberOfSamples(n_rand_samples);
        imageIterator.GoToBegin();
        unsigned int i_pixel =0;
          while(!imageIterator.IsAtEnd())
            {
          //  std::cout << imageIterator.Get() << std::endl;

            out_vector(i_pixel) = imageIterator.Get() ;

            ++imageIterator;
            ++i_pixel;
            }

          return out_vector;
    }
    else // take all pixels
    {
         itk::ImageRegionIterator<ImageType> imageIterator(input_img,input_img->GetLargestPossibleRegion());
         imageIterator.GoToBegin();
         unsigned int i_pixel =0;
           while(!imageIterator.IsAtEnd())
             {
           //  std::cout << imageIterator.Get() << std::endl;

             out_vector(i_pixel) = imageIterator.Get() ;

             ++imageIterator;
             ++i_pixel;
             }

           return out_vector;
    }





}



template<typename ImageType, typename VectorType>
typename ImageType::Pointer eigenVector2itkImage(const VectorType &input_vector, const typename ImageType::SizeType &size_image){



//ImageType::Pointer image = ImageType::New();

typename ImageType::RegionType region;
 typename ImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;

//  ImageType::SizeType size_image;
//  size[0] = 200;
//  size[1] = 300;
 // size[2] = 300;

  region.SetSize(size_image);
  region.SetIndex(start);

 typename ImageType::Pointer image = ImageType::New();
  image->SetRegions(region);
  image->Allocate();



  //check dimensions
  if (size_image[0]*size_image[1]*size_image[2] != input_vector.rows()){
       printf ("Error: Tot number of pixels in vector (%i) don't match size of image' (%i, %i, %i)\n",input_vector.rows(),size_image[0],size_image[1],size_image[2]);
       return image;
  }


    itk::ImageRegionIterator<ImageType> imageIterator(image,image->GetLargestPossibleRegion());

    unsigned int i_pixel = 0;
    while(!imageIterator.IsAtEnd())
        {
        // Get the value of the current pixel
        //unsigned char val = imageIterator.Get();
        //std::cout << (int)val << std::endl;

        // Set the current pixel to white
        imageIterator.Set(input_vector(i_pixel));

        ++imageIterator;
        ++i_pixel;
        }

    return image;

}



/*
template<typename ImageType, typename VectorType>
static void itkImage2EigenVector( typename ImageType::Pointer &input_img, VectorType &out_vector,const unsigned int n_rand_samples,const unsigned int  tot_n_pixels){

    if(out_vector.rows() != n_rand_samples){

        //error
        printf ("Error: Lenght of Vector (%i) must be equal to number or samples (%i)\n",out_vector.rows(),n_rand_samples);
        return;
    }
    if(tot_n_pixels < n_rand_samples){

        //error
        printf ("Error: Tot number of pixels in image (%i) must be smaller to number or samples (%i)\n",tot_n_pixels,n_rand_samples);
        return;
    }

    itk::ImageRandomNonRepeatingConstIteratorWithIndex<ImageType> imageIterator(input_img, input_img->GetLargestPossibleRegion());
      imageIterator.SetNumberOfSamples(n_rand_samples);

    imageIterator.GoToBegin();
    unsigned int i_pixel =0;
      while(!imageIterator.IsAtEnd())
        {
        std::cout << imageIterator.Get() << std::endl;

        out_vector(i_pixel) = imageIterator.Get() ;

        ++imageIterator;
        ++i_pixel;
        }



}


/*

#include "itkConvolutionImageFilter.h"

/// Convolves an entire image
///
///
///
//static void wholeConvolveSepFilter( const SVCombo &combo, const SeparableFilter &filter, Matrix3D<float> *out )
static void wholeConvolveSepFilter( const SVCombo &combo, const SeparableFilter &filter, Matrix3D<float> *out )
{
typedef float ImageScalarType;
typedef itk::Image<ImageScalarType, 3>   ITKImageType;

ITKImageType::Pointer kernelX = ITKImageType::New();
ITKImageType::Pointer kernelY = ITKImageType::New();
ITKImageType::Pointer kernelZ = ITKImageType::New();

/*prepare1DKernel<ITKImageType, Eigen::VectorXf>( kernelX, 0, filter.filters[0] );
prepare1DKernel<ITKImageType, Eigen::VectorXf>( kernelY, 1, filter.filters[1] );
prepare1DKernel<ITKImageType, Eigen::VectorXf>( kernelZ, 2, filter.filters[2] );
/
prepare1DKernel<ITKImageType, Eigen::VectorXf>( kernelX, 0, filters[0] );
prepare1DKernel<ITKImageType, Eigen::VectorXf>( kernelY, 1, filters[1] );
prepare1DKernel<ITKImageType, Eigen::VectorXf>( kernelZ, 2, filters[2] );


typedef itk::ConvolutionImageFilter< ITKImageType, ITKImageType, ITKImageType >  ConvolutionType;

ITKImageType::Pointer rawImage = combo.channels[filter.channelIdx]->asItkImage();

ConvolutionType::Pointer convX, convY, convZ;

convX = ConvolutionType::New();
convX->SetInput( rawImage );
convX->SetKernelImage(kernelX);
convX->SetOutputRegionModeToSame();

convY = ConvolutionType::New();
convY->SetInput( convX->GetOutput() );
convY->SetKernelImage(kernelY);
convY->SetOutputRegionModeToSame();

convZ = ConvolutionType::New();
convZ->SetInput( convY->GetOutput() );
convZ->SetKernelImage(kernelZ);
convZ->SetOutputRegionModeToSame();

convZ->Update();

out->loadItkImage( convZ->GetOutput() );
}

*/
