

#include <iostream>
#include <fstream>
#include <string>
#include "include/Eigen/Dense"
#include "superannotator/Matrix3D.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConvolutionImageFilter.h"

using namespace std;
using namespace Eigen;

#define MAXBUFSIZE  ((int) 1e6)

typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;


MatrixXd readMatrix(const char *filename);

template<typename ImageType, typename VectorType>
static void prepare1DKernel( typename ImageType::Pointer &kernel, const unsigned dir, const VectorType &origVec );


template<typename ImageType, typename VectorType>
static void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, Matrix3D<float> &out);


int main () {

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


    //load itk image

    const char *input_image_file ="/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2.nrrd";
    typedef float ImageScalarType;
    typedef itk::Image< ImageScalarType, 3 >         ITKImageType;
    typedef itk::ImageFileReader<ITKImageType> ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(input_image_file);

//convolve with constant filter
//typename ImageType::Pointer &kernel;
  //  typedef float ImageScalarType;
   // typedef itk::Image<ImageScalarType, 3>   ITKImageType;


    ITKImageType::Pointer kernelX = ITKImageType::New();
    ITKImageType::Pointer kernelY = ITKImageType::New();
ITKImageType::Pointer kernelZ = ITKImageType::New();

    const int kernel_size = (int)(sep_filters.rows()/3);

   // std::cout << kernel_size << std::endl;




   // for(unsigned int i_filter = 0; i_filter<sep_filters.cols() ;i_filter++)
  //  {
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
         convX->SetInput( reader->GetOutput() );
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


    //save image
    const char *output_image_file ="/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2_convolved.nrrd";

    typedef  itk::ImageFileWriter< ITKImageType  > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(output_image_file);
      writer->SetInput(convZ->GetOutput());
      writer->Update();
    for(unsigned int i_filter = 0; i_filter<sep_filters.cols() ;i_filter++)
    {
        const VectorTypeDouble kernel_eig = sep_filters.col(i_filter);


        Matrix3D<float> out_matrix;

        ITKImageType::Pointer in_img = reader->GetOutput();
        wholeConvolveSepFilter<ITKImageType, VectorTypeDouble >( in_img, kernel_eig, out_matrix);

    }//end for i_filter

  /*VectorType myvector;

  // set some values:
  for (int i=1; i<10; ++i) myvector.push_back(1.0);   // 1 1 1 1 1 1 1 1 1

  std::reverse(myvector.begin(),myvector.end());    // 1 1 1 1 1 1 1 1 1

  // print out content:
  std::cout << "myvector contains:";
  for (std::vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
    std::cout << ' ' << *it;
  std::cout << '\n';

*/
  return 0;
}

MatrixXd readMatrix(const char *filename)
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
    MatrixXd result(rows,cols);
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

    out.loadItkImage( convZ->GetOutput() );

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
