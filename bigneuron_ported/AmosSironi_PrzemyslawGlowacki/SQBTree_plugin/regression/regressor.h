#ifndef _REGRESSOR_H
#define _REGRESSOR_H


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



void trainRegressor(gFeatArrayType all_samples_features,gResponseArrayType all_samples_gt,char *regressor_output_file,char * loss_type = "squaredloss",const unsigned int max_boost_iters = 200,const unsigned int max_depth_wl_tree = 0,const double shrink_factor = 0.1,unsigned int m_try =0);


#endif
