

#include <iostream>
#include <fstream>
#include <string>
#include "regressor.h"
#include "sep_conv.h"
#include "../superannotator_sel/Matrix3D.h"
#include "../sqb_0.1/include/Eigen/Dense"

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


static void testConvolve(){


    ////load filters
    const char *weight_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_weigths_cpd_rank_25_rot_n1.txt";
    const char *sep_filters_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_sep_cpd_rank_25_rot_n1.txt";

    MatrixXd weights = readMatrix(weight_file);
    MatrixXd sep_filters = readMatrix(sep_filters_file);


    ////load itk image

    const char *input_image_file ="/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2.nrrd";
    typedef float ImageScalarType;
    typedef itk::Image< ImageScalarType, 3 >         ITKImageType;
    typedef itk::ImageFileReader<ITKImageType> ReaderType;

    typedef  itk::ImageFileWriter< ITKImageType  > WriterType;

  ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(input_image_file);

     ITKImageType::Pointer input_image = reader->GetOutput();
     input_image->Update();
      ITKImageType::SizeType size_image = input_image->GetLargestPossibleRegion().GetSize();


      ////rescale in [0 1]
       typedef itk::RescaleIntensityImageFilter< ITKImageType, ITKImageType > RescaleFilterType;
      RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
      rescaleFilter->SetInput(reader->GetOutput());
      rescaleFilter->SetOutputMinimum(0.0);
      rescaleFilter->SetOutputMaximum(1.0);

    const float scale_factor =1;
    MatrixXf sep_filters_float = sep_filters.cast<float>();
    MatrixXf weights_float = weights.cast<float>();
    ITKImageType::Pointer rescaled_img = rescaleFilter->GetOutput();
    rescaleFilter->Update();
      MatrixXf nonsep_features_all = convolveSepFilterBankComb<ITKImageType,MatrixXf,VectorTypeFloat>(rescaled_img,sep_filters_float,weights_float, scale_factor);



      for(unsigned int i_filter = 0; i_filter<1 ;i_filter++)
      {

          VectorTypeFloat convolved_vector = nonsep_features_all.col(i_filter);
          ITKImageType::Pointer convolved_img_orig = eigenVector2itkImage<ITKImageType,VectorTypeFloat>(convolved_vector,size_image);


          WriterType::Pointer writer = WriterType::New();
          //const char *output_conv_file =printf("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2_convolved_orig_%i.nrrd",i_filter);

          char output_conv_file [500];
          //char *output_image_file_i_filter;
          const int temp_int =sprintf(output_conv_file,"/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/cropped_N2_convolved_orig_%i.nrrd",i_filter);


          writer->SetFileName(output_conv_file);
          writer->SetInput(convolved_img_orig);
          writer->Update();
      }
}
