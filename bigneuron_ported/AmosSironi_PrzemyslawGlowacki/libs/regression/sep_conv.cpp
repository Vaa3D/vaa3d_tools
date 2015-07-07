#include <iostream>
#include <fstream>
#include <string>
#include "../libs/sqb_0.1/include/Eigen/Dense"
#include "../libs/superannotator_sel/Matrix3D.h"
#include <SQB/Core/RegTree.h>
#include <SQB/Core/RegTree.h>
#include <SQB/Core/Utils.h>

#include <SQB/Core/Booster.h>

#include <SQB/Core/LineSearch.h>
#include "itkImage.h"
#include "itkConvolutionImageFilter.h"
#include "itkImageRandomNonRepeatingConstIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMultiThreader.h"

//#include <omp.h>

using namespace std;
using namespace Eigen;

#define MAXBUFSIZE  ((int) 1e6)

typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;


typedef SQB::TreeBoosterWeightsType  WeightsType;
typedef float   FeatsType;

typedef Eigen::Array<FeatsType, Eigen::Dynamic, Eigen::Dynamic>     gFeatArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gResponseArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gWeightsArrayType;

typedef SQB::MatrixFeatureIndexList<gFeatArrayType>          MatrixFeatureIndexListType;
typedef SQB::MatrixSampleIndexList<gFeatArrayType>           MatrixSampleIndexListType;
typedef SQB::MatrixFeatureValueObject<gFeatArrayType>        MatrixFeatureValueObjectType;
typedef SQB::MatrixSingleResponseValueObject<gResponseArrayType>   MatrixClassifResponseValueObjectType;

typedef SQB::TreeBooster<
            MatrixSampleIndexListType,
            MatrixFeatureIndexListType,
            MatrixFeatureValueObjectType,
            MatrixClassifResponseValueObjectType >      TreeBoosterType;


////functions definitions
///

template<typename MatrixType>
void writeMatrix(const char *filename,MatrixType matrix_to_write){

    ofstream fout(filename); //opening an output stream for file test.txt
        /*checking whether file could be opened or not. If file does not exist or you do not have write permissions, file
      stream could not be opened.*/
      if(fout.is_open())
        {
        //file opened successfully so we are here
        cout << "File Opened successfully. Writing data from matrix to file" << endl;

        //std::ofstream output("Power vector.txt");
        for (unsigned int k=0; k<matrix_to_write.rows(); k++)
        {
            for ( unsigned int l=0; l<matrix_to_write.cols(); l++)
            {
             //   cout << matrix_to_write(k,l) << endl;
                fout << matrix_to_write(k,l) << " "; // behaves like cout - cout is also a stream
            }
            fout << ";" << endl;
        }
        cout << "matrix data successfully saved into the file " <<filename<< endl;
        }
        else //file could not be opened
        {
            cout << "File "<< filename<<" could not be written." << endl;
        }
}

MatrixTypeDouble readMatrix(const char *filename)
    {
    int cols = 0, rows = 0;
    double buff[MAXBUFSIZE];

    // Read numbers from file into buffer.
    ifstream infile;
    infile.open(filename);
    if(infile){

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

    }else{

        std::cout << "File " << filename << " could not be open. " << std::endl;
    }

    rows--;

    // Populate matrix with numbers.
    MatrixTypeDouble result(rows,cols);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result(i,j) = buff[ cols*i+j ];

    return result;
    }


// helper for itk separable filter
// dir is [0,1,2]
template<typename ImageType, typename VectorType>
 void prepare1DKernel( typename ImageType::Pointer &kernel, const unsigned dir, const VectorType &origVec )
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
  void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, typename ImageType::Pointer kernelX,typename ImageType::Pointer kernelY,typename ImageType::Pointer kernelZ, Matrix3D<float> &out){



    //  std::cout << "convolving sep..." << std::endl<<std::flush;
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

//    std::cout << "updating convolving sep..." << std::endl<<std::flush;

   convZ->Update();

  // std::cout << "updated convolving sep..." << std::endl<<std::flush;
  //  std::cout << "size conv: " <<convZ->GetOutput()->GetLargestPossibleRegion().GetSize()<< std::endl<<std::flush;

   //out.loadItkImage( convZ->GetOutput() );
   out.copyFrom( convZ->GetOutput() );

//std::cout << "got out conv" << std::endl<<std::flush;

}


  template<typename ImageType, typename VectorType>
    void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, typename ImageType::Pointer kernelX,typename ImageType::Pointer kernelY,typename ImageType::Pointer kernelZ, typename ImageType::Pointer &convolved_img){



      //  std::cout << "convolving sep..." << std::endl<<std::flush;
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

  //    std::cout << "updating convolving sep..." << std::endl<<std::flush;

     convZ->Update();

    // std::cout << "updated convolving sep..." << std::endl<<std::flush;
    //  std::cout << "size conv: " <<convZ->GetOutput()->GetLargestPossibleRegion().GetSize()<< std::endl<<std::flush;

     //out.loadItkImage( convZ->GetOutput() );
     convolved_img=  convZ->GetOutput() ;

  //std::cout << "got out conv" << std::endl<<std::flush;

  }



template<typename ImageType, typename VectorType>
  void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig_x,const VectorType &kernel_eig_y,const VectorType &kernel_eig_z, Matrix3D<float> &out)
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
    void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig_x,const VectorType &kernel_eig_y,const VectorType &kernel_eig_z, typename ImageType::Pointer &convolved_img)
  {


        typename ImageType::Pointer kernelX = ImageType::New();
        typename ImageType::Pointer kernelY = ImageType::New();
        typename ImageType::Pointer kernelZ = ImageType::New();

       prepare1DKernel< ImageType,  VectorType>( kernelX, 0, kernel_eig_x );
       prepare1DKernel< ImageType, VectorType>( kernelY, 1, kernel_eig_y );
       prepare1DKernel< ImageType, VectorType>( kernelZ, 2, kernel_eig_z );

       wholeConvolveSepFilter<ImageType,VectorType>(input_img, kernelX,kernelY,kernelZ,convolved_img);

  }


template<typename ImageType, typename VectorType>
  void wholeConvolveSepFilterSplitVec( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, Matrix3D<float> &out){

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
    void wholeConvolveSepFilterSplitVec( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, typename ImageType::Pointer &convolved_img){

          VectorType kernel_eig_x;
            VectorType kernel_eig_y;
            VectorType kernel_eig_z;


           const int kernel_size = (int)(kernel_eig.rows()/3);


          kernel_eig_x = kernel_eig.head(kernel_size);
          kernel_eig_y = kernel_eig.segment(kernel_size,kernel_size);
          kernel_eig_z = kernel_eig.tail(kernel_size);

          wholeConvolveSepFilter<ImageType,VectorType>(input_img, kernel_eig_x,kernel_eig_y,kernel_eig_z,convolved_img);

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
            out_vector(i_pixel) = imageIterator.Get() ;

            ++imageIterator;
            ++i_pixel;
            }

          return out_vector;
    }
    else // take all pixels
    {

    //    std::cout << "taking all pixels" <<std::endl<<std::flush;
         itk::ImageRegionIterator<ImageType> imageIterator(input_img,input_img->GetLargestPossibleRegion());
         imageIterator.GoToBegin();
         unsigned int i_pixel =0;
     //    std::cout << "iterating..." <<std::endl<<std::flush;

           while(!imageIterator.IsAtEnd())
             {

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
       printf ("Error: Tot number of pixels in vector (%i) does not match size of image' (%i, %i, %i)\n",input_vector.rows(),size_image[0],size_image[1],size_image[2]);
       return image;
  }


    itk::ImageRegionIterator<ImageType> imageIterator(image,image->GetLargestPossibleRegion());

    unsigned int i_pixel = 0;
    while(!imageIterator.IsAtEnd())
        {

        // Set the current pixel
        imageIterator.Set(input_vector(i_pixel));

        ++imageIterator;
        ++i_pixel;
        }

    return image;

}

//TODO: VectorType and MatrixType should be compatible !!!
template<typename ImageType, typename MatrixType, typename VectorType>
void convolveSepFilterBank(MatrixType &sep_features_all, typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix){


    const unsigned int n_sep_features = sep_filters_matrix.cols();

    typename ImageType::SizeType size_image = input_img->GetLargestPossibleRegion().GetSize();
    const unsigned int n_pixels = size_image[0]*size_image[1]*size_image[2];

    sep_features_all = MatrixType::Zero(n_pixels,n_sep_features);
    VectorType temp_vector;
    typename ImageType::Pointer convolved_img = ImageType::New();
    //#pragma omp parallel for
    for(unsigned int i_filter = 0; i_filter<n_sep_features ;i_filter++)
    {
     //   std::cout << "convolving sep filter "<<i_filter << std::endl<< std::flush;
        const VectorType kernel_eig = sep_filters_matrix.col(i_filter);
   //kernel_eig = sep_filters_matrix.col(i_filter);


//  std::cout << "coping result matrix" << std::endl<< std::flush;
       //// Matrix3D<float> out_matrix;
      //// wholeConvolveSepFilterSplitVec<ImageType, VectorType >( input_img, kernel_eig, out_matrix);
      ////  typename ImageType::Pointer convolved_img= out_matrix.asItkImage();
   //     convolved_img= out_matrix.asItkImage();
    //    std::cout << "coping result matrix done" << std::endl<< std::flush;

  //      std::cout << "coping temp vec" << std::endl<< std::flush;
//

        ////

        wholeConvolveSepFilterSplitVec<ImageType, VectorType >( input_img, kernel_eig, convolved_img);
        temp_vector = itkImage2EigenVector<ImageType,VectorType>(convolved_img,n_pixels,n_pixels);
    //    temp_vector = itkImage2EigenVector<ImageType,VectorType>(convolved_img,n_pixels,n_pixels);
    //    std::cout << "coping temp vec done." << std::endl<< std::flush;

        // store result in coumns of a matrix
        sep_features_all.col(i_filter) = temp_vector;
      //  std::cout << "stored temp vec in matrix feat" << std::endl<< std::flush;


    }//end for i_filter



 //   return sep_features_all;
}

//NB: VectorType and MatrixType should have compatible types !!!
template<typename ImageType, typename MatrixType, typename VectorType>
void convolveSepFilterBankComb(MatrixType &nonsep_features_all, typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix, const MatrixType &weight_matrix ,const float scale_factor){

        typename ImageType::SizeType size_image = input_img->GetLargestPossibleRegion().GetSize();
        const unsigned int n_pixels = size_image[0]*size_image[1]*size_image[2];
        const unsigned int n_nonsep_features = weight_matrix.cols();

        MatrixType sep_features_all;
        convolveSepFilterBank<ImageType,MatrixType,VectorType>(sep_features_all,input_img,sep_filters_matrix);



        //multiply with weight to get original filters convolution
        //const float scale_factor = 1.0; // used for multiscale appraoch if rescale filers to compute features
        //MatrixTypeFloat nonsep_features_all(n_pixels,n_nonsep_features);


        nonsep_features_all = sep_features_all*weight_matrix;
        nonsep_features_all = nonsep_features_all/(scale_factor*scale_factor*scale_factor);



      //  return nonsep_features_all;
}



//TODO
template<typename MatrixType>
void getContextFeatures(MatrixType &nonsep_features_all,const MatrixType &nonsep_features_all_no_context,const float sigma_pool){

const unsigned int n_pixels = nonsep_features_all_no_context.rows();
    const unsigned int n_features = nonsep_features_all_no_context.cols();
    nonsep_features_all   = MatrixType::Zero(n_pixels,n_features);


}

//TODO: for now just convolve image, eventually add context features
template<typename ImageType, typename MatrixType, typename VectorType>
void computeFeaturesSepComb(MatrixType &nonsep_features_all, typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix, const MatrixType &weight_matrix ,const float scale_factor,bool add_context_features ){



 //   itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1);


        typename ImageType::SizeType size_image = input_img->GetLargestPossibleRegion().GetSize();
        const unsigned int n_pixels = size_image[0]*size_image[1]*size_image[2];
        const unsigned int n_nonsep_features = weight_matrix.cols();

     //   std::cout<< "size image comp features: " << size_image << std::endl << std::flush;

        MatrixType sep_features_all;
        convolveSepFilterBank<ImageType,MatrixType,VectorType>(sep_features_all,input_img,sep_filters_matrix);



        //multiply with weight to get original filters convolution
        //const float scale_factor = 1.0; // used for multiscale appraoch if rescale filers to compute features
        //nonsep_features_all(n_pixels,n_nonsep_features);

        if(~add_context_features){

             std::cout << "Computing non-sep features..."<<std::endl<< std::flush;
                    nonsep_features_all = sep_features_all*weight_matrix;
                    nonsep_features_all /=(scale_factor*scale_factor*scale_factor);
                    std::cout << "Computing non-sep features...Done."<<std::endl<< std::flush;
        }
         else{//TODO

            MatrixType nonsep_features_all_no_context;
            std::cout << "Computing non-sep features..." << std::endl;
            nonsep_features_all_no_context = sep_features_all*weight_matrix;
            nonsep_features_all_no_context = nonsep_features_all/(scale_factor*scale_factor*scale_factor);
            std::cout << "Computing non-sep features...Done!" << std::endl;

            float sigma_pool = 2.0;
            std::cout << "Getting context features..."<< std::endl;
            getContextFeatures<MatrixType>(nonsep_features_all,nonsep_features_all_no_context,sigma_pool);
            std::cout << "Getting context features...Done!" << std::endl;

         }

        //return nonsep_features_all;
}



//// instantiate explicitely
////template MatrixTypeFloat convolveSepFilterBankComb<itk::Image< float, 3>, MatrixTypeFloat,VectorTypeFloat>(itk::Image< float, 3>::Pointer &I, const MatrixTypeFloat &M, const MatrixTypeFloat &V, const float scale_factor);
template void convolveSepFilterBankComb<itk::Image< float, 3>, MatrixTypeFloat,VectorTypeFloat>(MatrixTypeFloat &feats_sep_all,itk::Image< float, 3>::Pointer &I, const MatrixTypeFloat &M, const MatrixTypeFloat &V, const float scale_factor);
template void computeFeaturesSepComb<itk::Image< float, 3>, MatrixTypeFloat,VectorTypeFloat>(MatrixTypeFloat &feats_all, itk::Image< float, 3>::Pointer &I, const MatrixTypeFloat &M, const MatrixTypeFloat &V, const float scale_factor,bool add_context_features );
template itk::Image< float, 3>::Pointer eigenVector2itkImage<itk::Image< float, 3>,VectorTypeDouble>(const VectorTypeDouble &newScores,const itk::Image< float, 3>::SizeType &size_img);
template itk::Image< float, 3>::Pointer eigenVector2itkImage<itk::Image< float, 3>,VectorTypeFloat>(const VectorTypeFloat &newScores,const itk::Image< float, 3>::SizeType &size_img);
template VectorTypeFloat itkImage2EigenVector<itk::Image< float, 3>,VectorTypeFloat>( itk::Image< float, 3>::Pointer &input_img,const unsigned int n_rand_samples,const unsigned int tot_n_pixels);
template VectorTypeDouble itkImage2EigenVector<itk::Image< float, 3>,VectorTypeDouble>( itk::Image< float, 3>::Pointer &input_img,const unsigned int n_rand_samples,const unsigned int tot_n_pixels);
template TreeBoosterType::ResponseArrayType itkImage2EigenVector<itk::Image< float, 3>,TreeBoosterType::ResponseArrayType>( itk::Image< float, 3>::Pointer &input_img,const unsigned int n_rand_samples,const unsigned int tot_n_pixels);
template itk::Image< float, 3>::Pointer eigenVector2itkImage<itk::Image< float, 3>,TreeBoosterType::ResponseArrayType>(const TreeBoosterType::ResponseArrayType &input_vector, const itk::Image< float, 3>::SizeType &size_img);

template void getContextFeatures<MatrixTypeFloat>(MatrixTypeFloat &nonsep_features_all,const MatrixTypeFloat &nonsep_features_all_no_context,const float sigma_pool);

template void writeMatrix<MatrixTypeDouble>(const char * filename, MatrixTypeDouble mat);
template void writeMatrix<MatrixTypeFloat>(const char * filename, MatrixTypeFloat mat);


//#endif
