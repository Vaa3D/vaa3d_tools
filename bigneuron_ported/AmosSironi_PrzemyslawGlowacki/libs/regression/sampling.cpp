//#include <random> //only c++11
#include "../libs/sqb_0.1/include/Eigen/Dense"
#include "sampling.h"
#include <iostream>
#include <ctime>

#include <../libs/boost_1_58_0/boost/random/mersenne_twister.hpp>
#include <../libs/boost_1_58_0/boost/random/uniform_int_distribution.hpp>



//using namespace Eigen;
typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;
typedef Eigen::Matrix<unsigned int,-1,1 > MatrixTypeUint;

template<typename MatrixType, typename VectorType>
bool getTrainSamplesFeaturesAndGt(const MatrixType &features_matrix,const VectorType &gt_vector,MatrixType &sampled_features_matrix, VectorType &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh = 100.0){

    unsigned int n_features_tot = features_matrix.cols();
    sampled_features_matrix = MatrixType::Zero(n_neg_samples+n_pos_samples,n_features_tot);
    sampled_gt_vector = VectorType::Zero(n_neg_samples+n_pos_samples);


  //  std::cout << "finding samples " << std::endl;

    MatrixTypeUint pos_indeces;// = getIndecesGreater<VectorType>(gt_vector,pos_thresh); //find(gt_vector>=pos_thresh)
    MatrixTypeUint neg_indeces;// = getIndecesSmaller<VectorType>(gt_vector,pos_thresh); //find(gt_vector<pos_thresh)
    getIndecesSmallerGreater<VectorType>(gt_vector,pos_thresh,neg_indeces,pos_indeces);  //find(gt_vector>=pos_thresh) and find(gt_vector<pos_thresh)


    if(pos_indeces.rows() < ((float)n_pos_samples)/100.0){

        std::cout << "No enough Positive samples found for image "<< std::endl;
        return false;
    }

    if(neg_indeces.rows() < ((float)n_neg_samples)/100.0){

        std::cout << "No enough Negative samples found for image " << std::endl;
        return false;
    }

    boost::random::mt19937 generator_pos;
    generator_pos.seed(static_cast<unsigned int>(std::time(0)));
    boost::random::uniform_int_distribution<unsigned int> distribution_pos(0,pos_indeces.rows()-1);
    boost::random::mt19937 generator_neg;
    generator_neg.seed(static_cast<unsigned int>(std::time(0)));
    boost::random::uniform_int_distribution<unsigned int> distribution_neg(0,neg_indeces.rows()-1);

    //get pos samples
    for(unsigned int i_pos =0; i_pos<n_pos_samples;i_pos++ ){

        unsigned int rand_pos_index = distribution_pos(generator_pos);


        sampled_features_matrix.row(i_pos) = features_matrix.row(pos_indeces(rand_pos_index));
        sampled_gt_vector(i_pos) = gt_vector(pos_indeces(rand_pos_index));
    }
    // get neg samples
    for(unsigned int i_neg =0; i_neg<n_neg_samples;i_neg++ ){
        unsigned int rand_neg_index = distribution_neg(generator_neg);

        sampled_features_matrix.row(i_neg+n_pos_samples) = features_matrix.row(neg_indeces(rand_neg_index));
        sampled_gt_vector(i_neg+n_pos_samples) = gt_vector(neg_indeces(rand_neg_index));
    }


    return true;

}


template<typename VectorType>
void getIndecesSmallerGreater(const VectorType &gt_vector, float thresh, MatrixTypeUint &found_smaller_indeces,MatrixTypeUint & found_greater_indeces ){


    unsigned int n_samples = gt_vector.rows();
    MatrixTypeUint found_greater_indeces_temp = MatrixTypeUint::Zero(n_samples,1);
    MatrixTypeUint found_smaller_indeces_temp = MatrixTypeUint::Zero(n_samples,1);


    unsigned int i_found_great = 0;
    unsigned int i_found_small = 0;
    for(unsigned int i_sample = 0; i_sample<n_samples; i_sample++){
        if(gt_vector(i_sample)>=thresh){
            found_greater_indeces_temp(i_found_great) = i_sample;
            i_found_great++;
        }
        else{
            found_smaller_indeces_temp(i_found_small) = i_sample;
            i_found_small++;
        }
    }

    found_greater_indeces = found_greater_indeces_temp.head(i_found_great);
    found_smaller_indeces = found_smaller_indeces_temp.head(i_found_small);




}

////to instantiate explicitely
template bool getTrainSamplesFeaturesAndGt<MatrixTypeFloat,VectorTypeFloat>(const MatrixTypeFloat &features_matrix,const VectorTypeFloat &gt_vector,MatrixTypeFloat &sampled_features_matrix, VectorTypeFloat &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh);
template void getIndecesSmallerGreater<VectorTypeFloat>(const VectorTypeFloat &gt_vector ,float thresh,MatrixTypeUint &found_smaller_indeces,MatrixTypeUint & found_greater_indeces );
//template MatrixTypeUint getIndecesGreater<VectorTypeFloat>(const VectorTypeFloat &gt_vector ,float thresh);
//template MatrixTypeUint getIndecesSmaller<VectorTypeFloat>(const VectorTypeFloat &gt_vector ,float thresh);
