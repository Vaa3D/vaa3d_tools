

#include <iostream>
#include <fstream>
#include <string>
#include "include/Eigen/Dense"

//#include "itkImage.h"
//#include "itkImageFileReader.h"
//#include "itkImageFileWriter.h"


using namespace std;
using namespace Eigen;

#define MAXBUFSIZE  ((int) 1e6)

MatrixXd readMatrix(const char *filename);

int main () {

    const char *weight_file = "./filters/oof_fb_3d_scale_5_weigths_cpd_rank_25_rot_n1.txt";
    const char *sep_filters_file = "./filters/oof_fb_3d_scale_5_sep_cpd_rank_25_rot_n1.txt";

    MatrixXd weights = readMatrix(weight_file);
    MatrixXd sep_filters = readMatrix(sep_filters_file);


    //std::cout << weights << std::endl;
    std::cout << weights.rows() << std::endl; //25 ( rows of W = number of sep filters )
    std::cout << weights.cols() << std::endl;//30 ( cols of W = number of original filters )

    //std::cout << sep_filters << std::endl;
    std::cout << sep_filters.rows() << std::endl; //75 ( rows of S = actual sep filters stored as a column )
    std::cout << sep_filters.cols() << std::endl;//25 ( cols of S = number of separable filters )


 /*   //load itk image

    const char *input_image_file ="./filters/cropped_N2.nrrd";
    typedef float ImageScalarType;
    typedef itk::Image< ImageScalarType, 3 >         ImageType;
    typedef itk::ImageFileReader<ImageType> ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(input_image_file);

//convolve with constant filter
//typename ImageType::Pointer &kernel;
  //  typedef float ImageScalarType;
   // typedef itk::Image<ImageScalarType, 3>   ITKImageType;


    ImageType::Pointer kernelX = ImageType::New();


//    typedef itk::ConvolutionImageFilter< ImageType, ImageType, ImageType >  ConvolutionType;

*/
    //save image
    const char *output_image_file ="./filters/cropped_N2_convolved.nrrd";

    typedef  itk::ImageFileWriter< ImageType  > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(output_image_file);
      writer->SetInput(reader->GetOutput());
      writer->Update();


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





/*


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

#include "itkConvolutionImageFilter.h"

/// Convolves an entire image
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
