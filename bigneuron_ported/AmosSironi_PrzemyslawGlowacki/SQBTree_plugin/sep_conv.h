#include <iostream>
#include <fstream>
#include <string>
#include "sqb_0.1/include/Eigen/Dense"
#include "include/superannotator_sel/Matrix3D.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConvolutionImageFilter.h"
#include "itkImageRandomNonRepeatingConstIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"

//#include <omp.h>

using namespace std;
using namespace Eigen;

#define MAXBUFSIZE  ((int) 1e6)

typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;



////functions declarations
///
// read text file and store it in Eigen matrix
MatrixTypeDouble readMatrix(const char *filename);

//prepare 1D kernel for itk separable convolution
template<typename ImageType, typename VectorType>
static void prepare1DKernel( typename ImageType::Pointer &kernel, const unsigned dir, const VectorType &origVec );

//convolve 3D image with separable filters (itk arguments)
template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, typename ImageType::Pointer kernelX,typename ImageType::Pointer kernelY,typename ImageType::Pointer kernelZ, Matrix3D<float> &out);

//convolve 3D image with separable filters (eigen arguments)
template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig_x,const VectorType &kernel_eig_y,const VectorType &kernel_eig_z, Matrix3D<float> &out);

//convolve 3D image with separable filters (split eigen vector in 3 equal parts)
template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilterSplitVec( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, Matrix3D<float> &out);

// store itk image into eigen colmun vector
template<typename ImageType, typename VectorType>
VectorType itkImage2EigenVector( typename ImageType::Pointer &input_img,const unsigned int n_rand_samples,const unsigned int tot_n_pixels);

//create itk image reshaping eigne vector
template<typename ImageType, typename VectorType>
typename ImageType::Pointer eigenVector2itkImage(const VectorType &input_vector, const typename ImageType::SizeType &size_image);

template<typename ImageType, typename MatrixType>
MatrixType convolveSepFilterBank( typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix, const MatrixType &weight_matrix );


////functions definitions
///

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
        imageIterator.Set( vec.coeff(i) );

        ++imageIterator;
        ++i;
    }
}


template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, typename ImageType::Pointer kernelX,typename ImageType::Pointer kernelY,typename ImageType::Pointer kernelZ, Matrix3D<float> &out){


    typedef itk::ConvolutionImageFilter< ImageType, ImageType, ImageType >  ConvolutionType;
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

   out.loadItkImage( convZ->GetOutput() );



}


template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig_x,const VectorType &kernel_eig_y,const VectorType &kernel_eig_z, Matrix3D<float> &out)
{


      typename ImageType::Pointer kernelX = ImageType::New();
      typename ImageType::Pointer kernelY = ImageType::New();
      typename ImageType::Pointer kernelZ = ImageType::New();

     prepare1DKernel< ImageType,  VectorType>( kernelX, 0, kernel_eig_x );
     prepare1DKernel< ImageType, VectorType>( kernelY, 1, kernel_eig_y );
     prepare1DKernel< ImageType, VectorType>( kernelZ, 2, kernel_eig_z );

     wholeConvolveSepFilter<ImageType,VectorType>(input_img, kernelX,kernelY,kernelZ,out);

}


template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilterSplitVec( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, Matrix3D<float> &out){

        VectorType kernel_eig_x;
          VectorType kernel_eig_y;
          VectorType kernel_eig_z;


         const int kernel_size = (int)(kernel_eig.rows()/3);


        kernel_eig_x = kernel_eig.head(kernel_size);
        kernel_eig_y = kernel_eig.segment(kernel_size,kernel_size);
        kernel_eig_z = kernel_eig.tail(kernel_size);

        wholeConvolveSepFilter<ImageType,VectorType>(input_img, kernel_eig_x,kernel_eig_y,kernel_eig_z,out);

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


typename ImageType::RegionType region;
 typename ImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;


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



