

#include <SQB/Core/RegTree.h>
#include <SQB/Core/Utils.h>

#include <SQB/Core/Booster.h>

#include <SQB/Core/LineSearch.h>


//using namespace Eigen;
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


void trainRegressor(gFeatArrayType all_samples_features,gResponseArrayType all_samples_gt,char *regressor_output_file,char * loss_type = "squaredloss",const unsigned int max_boost_iters = 200,const unsigned int max_depth_wl_tree = 0,const double shrink_factor = 0.1,unsigned int m_try =0){

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


    TB.learn( TreeBoosterType::SampleListType(all_samples_features),
              TreeBoosterType::FeatureListType(all_samples_features),
              TreeBoosterType::FeatureValueObjectType(all_samples_features),
              TreeBoosterType::ClassifierResponseValueObjectType(all_samples_gt),
              max_boost_iters );
    TB.printOptionsSummary();



//    for (unsigned i=0; i < all_samples_features.rows(); i++) {
//      std::cout << all_samples_features.coeff(i) << std::endl;
//    }

//    for (unsigned i=0; i < all_samples_gt.rows(); i++) {
//      std::cout << all_samples_gt.coeff(i) << std::endl;
//    }

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

