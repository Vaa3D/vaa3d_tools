#include <SQB/Core/RegTree.h>
#include <SQB/Core/Utils.h>

#include <SQB/Core/Booster.h>

#include <SQB/Core/LineSearch.h>

#include "itkImage.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <math.h>
#include <regressor.h>


//using namespace Eigen;
typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;

typedef itk::Image<float, 4>  ITK4DDistImageType;
typedef itk::Image<float, 3>  ITKFloatImageType;
typedef itk::Image<unsigned char, 3>  ITKBinaryImageType;


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


void trainRegressor(const gFeatArrayType &all_samples_features,const gResponseArrayType &all_samples_gt,char *regressor_output_file,char * loss_type,const unsigned int max_boost_iters ,const unsigned int max_depth_wl_tree,const double shrink_factor,unsigned int m_try){

    SQB::TreeBoosterNaiveResampler< TreeBoosterType::ResamplerBaseObjectType::WeightsArrayType,
                                    TreeBoosterType::ResamplerBaseObjectType::LabelsArrayType >  resampler;


    TreeBoosterType TB;
    TB.setResamplerObject( &resampler );

    //// set options
    TB.setShrinkageFactor( shrink_factor );
    TB.setMTry(m_try);
    TB.setMaxTreeDepth( max_depth_wl_tree );
    if (strcmp(loss_type, "exploss") == 0)
                TB.setLoss( SQB::ExpLoss );
    else if ( strcmp(loss_type, "logloss") == 0 )
                TB.setLoss( SQB::LogLoss );
    else if ( strcmp(loss_type, "squaredloss") == 0 )
                TB.setLoss( SQB::SquaredLoss );
    else
          std::cerr<<"Invalid Loss value: "<< loss_type<< std::endl;

    TB.printOptionsSummary();

    //// training

//    TB.setRandSeed(std::time(0));
    TB.learn( TreeBoosterType::SampleListType(all_samples_features),
              TreeBoosterType::FeatureListType(all_samples_features),
              TreeBoosterType::FeatureValueObjectType(all_samples_features),
              TreeBoosterType::ClassifierResponseValueObjectType(all_samples_gt),
              max_boost_iters );
    TB.printOptionsSummary();


    ////save trained regressor

    libconfig::Config cfg;
    libconfig::Setting &root = cfg.getRoot();

    libconfig::Setting &regressor = root.add("regressor", libconfig::Setting::TypeList);

    TB.saveToLibconfig(regressor);

    // Write the new configuration.
    try
    {
      cfg.writeFile(regressor_output_file);
      std::cerr << "New configuration successfully written to: " << regressor_output_file
           << std::endl;

    }
    catch(const libconfig::FileIOException &fioex)
    {
      std::cerr << "I/O error while writing file: " << regressor_output_file << std::endl;
      //return(EXIT_FAILURE);
    }

}

void predictRegressor(const char * regressor_output_file,const gFeatArrayType &all_samples_features,TreeBoosterType::ResponseArrayType &newScores){


    ////apply trained regressor
   std::cout << "Loading Regressor..." << std::endl<< std::flush;

    libconfig::Config cfg;

    // Read the file. If there is an error, report it and exit.
    try
    {
      cfg.readFile(regressor_output_file);
    }
    catch(const libconfig::FileIOException &fioex)
    {
      std::cerr << "I/O error while reading file." << std::endl;
    }
    catch(const libconfig::ParseException &pex)
    {
      std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                << " - " << pex.getError() << std::endl;
    }

    libconfig::Setting &root = cfg.getRoot();

    libconfig::Setting &regressor = root["regressor"];

    TreeBoosterType TB;
    TB.loadFromLibconfig(regressor);

    unsigned maxIters = TB.numWeakLearners();
    std::cout << "Loading Regressor...Done." << std::endl<< std::flush;

    std::cout << "Predicting..."<< std::endl<< std::flush;
    newScores = TreeBoosterType::ResponseArrayType::Zero(all_samples_features.rows());

    TB.predict( TreeBoosterType::SampleListType(all_samples_features),
                TreeBoosterType::FeatureValueObjectType(all_samples_features),
                newScores,
                maxIters );
    std::cout<< "Predicting...Done!"<< std::endl<< std::flush;

}



template<typename ImageType>
ITKFloatImageType::Pointer binaryGt2ExpDistGt(typename ImageType::Pointer train_gt_radial_ITK,float thresh_distance){


    typedef itk::BinaryThresholdImageFilter <ImageType, ITKBinaryImageType> BinaryThresholdImageFilterType;
    typedef itk::SignedMaurerDistanceMapImageFilter<ITKBinaryImageType, ITKFloatImageType> DistanceTransformType;


    //bool compute_multiscale_gt = true;

    //if(scales(0) <0.0)
    //    compute_multiscale_gt = false;


   // if(!compute_multiscale_gt){

        typename BinaryThresholdImageFilterType::Pointer thresholdFilter
            = BinaryThresholdImageFilterType::New();
          thresholdFilter->SetInput(train_gt_radial_ITK);
         // float lowerThreshold = 0.5;
          //thresholdFilter->SetLowerThreshold(lowerThreshold);
          float upperThreshold = 0.0;
          thresholdFilter->SetUpperThreshold(upperThreshold);
          thresholdFilter->SetInsideValue(255);
          thresholdFilter->SetOutsideValue(0);


          DistanceTransformType::Pointer dt = DistanceTransformType::New();
          dt->SetInput( thresholdFilter->GetOutput() );

          dt->SetBackgroundValue( thresholdFilter->GetInsideValue() );
          dt->SetSquaredDistance(false);
          dt->SetUseImageSpacing(true);

          dt->Update();

          ITKFloatImageType::Pointer distImg = dt->GetOutput();


    ITKFloatImageType::Pointer distImgExp = transformDistGt<ITKFloatImageType>(distImg,thresh_distance);

    return distImgExp;

}


template<typename ImageType>
ITK4DDistImageType::Pointer radialGt2ExpDistGt(typename ImageType::Pointer train_gt_radial_ITK,float thresh_distance,VectorTypeFloat scales, float scale_toll){


   // typedef itk::Image<unsigned char, 3>  ITKBinaryImageType;
    typedef itk::BinaryThresholdImageFilter <ImageType, ITKBinaryImageType> BinaryThresholdImageFilterType;
    typedef itk::SignedMaurerDistanceMapImageFilter<ITKBinaryImageType, ITK4DDistImageType> DistanceTransformType;


   /////TODO: convert to binary multiscale and compute 4D distance
   ///
    //ITK4DDistImageType::Pointer distImgExp = transformDistGt<ITK4DDistImageType>(distImg,thresh_distance);
    ITK4DDistImageType::Pointer distImgExp = ITK4DDistImageType::New();


    return distImgExp;


}


template<typename DistImageType>
typename DistImageType::Pointer transformDistGt(typename DistImageType::Pointer dist_gt_image,float thresh_distance){

    float a = 6.0;

    typename DistImageType::Pointer transfDistImg = dist_gt_image;

    typename DistImageType::RegionType region = dist_gt_image->GetLargestPossibleRegion();

    itk::ImageRegionIterator<DistImageType> imageIterator(transfDistImg,region);

    float pixel_value_in;
    float pixel_value_out;

      while(!imageIterator.IsAtEnd())
        {
           pixel_value_in = imageIterator.Get();

           if(pixel_value_in<0.0) //assume distance transofrm is positive
               pixel_value_in = 0.0;

           if(pixel_value_in<thresh_distance){
                pixel_value_out =exp(a *( 1.0 - pixel_value_in/thresh_distance) ) - 1.0;
           }
           else{
                pixel_value_out = 0.0;

           }

        imageIterator.Set(pixel_value_out);

        ++imageIterator;
        }

return transfDistImg;
}


////to instantiate explicitely
template ITKFloatImageType::Pointer binaryGt2ExpDistGt<ITKFloatImageType>(ITKFloatImageType::Pointer train_gt_radial_ITK,float thresh_distance);
template ITK4DDistImageType::Pointer radialGt2ExpDistGt<ITKFloatImageType>(ITKFloatImageType::Pointer train_gt_radial_ITK,float thresh_distance,VectorTypeFloat scales, float scale_toll);


