#include "sqb_0.1/include/Eigen/Dense"
//#include <random> //only c++11
#include "sampling.h"
#include <iostream>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>


//using namespace Eigen;
typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;
typedef Eigen::Matrix<unsigned int,-1,1 > MatrixTypeUint;

template<typename MatrixType, typename VectorType>
bool getTrainSamplesFeaturesAndGt(const MatrixType &features_matrix,const VectorType &gt_vector,MatrixType &sampled_features_matrix, VectorType &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh = 50.0){

//float thres = 50.0; //consider positive gt samples greater than this threshold


    unsigned int n_features_tot = features_matrix.cols();

//pos_samples = gt_vector.Array()>pos_thresh;
    sampled_features_matrix = MatrixType::Zero(n_neg_samples+n_pos_samples,n_features_tot);
    sampled_gt_vector = VectorType::Zero(n_neg_samples+n_pos_samples);


    std::cout << "finding samples " << std::endl;

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


    std::cout << "rows pos indeces: "<< pos_indeces.rows()<< "cols pos indeces: "<< pos_indeces.cols()<<std::endl;

//    for(unsigned int i_pos =0; i_pos<n_pos_samples;i_pos++ ){
//        std::cout << "pos indeces all: " << pos_indeces(i_pos)<< std::endl;
//    }


 //   std::cout << "rand sampling  " << std::endl;

    boost::random::mt19937 generator_pos;
    boost::random::uniform_int_distribution<unsigned int> distribution_pos(0,pos_indeces.rows());
    boost::random::mt19937 generator_neg;
    boost::random::uniform_int_distribution<unsigned int> distribution_neg(0,pos_indeces.rows());
//    std::default_random_engine generator_pos;
//    std::uniform_int_distribution<unsigned int> distribution_pos(0,pos_indeces.rows());
//    std::default_random_engine generator_neg;
//    std::uniform_int_distribution<unsigned int> distribution_neg(0,neg_indeces.rows());

    //get pos samples
    for(unsigned int i_pos =0; i_pos<n_pos_samples;i_pos++ ){

        unsigned int rand_pos_index = distribution_pos(generator_pos);

    //    std::cout << "rand pos index: " <<pos_indeces(rand_pos_index) << std::endl;

        sampled_features_matrix.row(i_pos) = features_matrix.row(pos_indeces(rand_pos_index));
        sampled_gt_vector(i_pos) = gt_vector(pos_indeces(rand_pos_index));
    }
    // get neg samples
    for(unsigned int i_neg =0; i_neg<n_neg_samples;i_neg++ ){
        unsigned int rand_neg_index = distribution_neg(generator_neg);

    //    std::cout << "rand neg index: " <<neg_indeces(rand_neg_index) << std::endl;

        sampled_features_matrix.row(i_neg+n_pos_samples) = features_matrix.row(neg_indeces(rand_neg_index));
        sampled_gt_vector(i_neg+n_pos_samples) = gt_vector(neg_indeces(rand_neg_index));
    }

    return true;

}


template<typename VectorType>
void getIndecesSmallerGreater(const VectorType &gt_vector, float thresh, MatrixTypeUint &found_smaller_indeces,MatrixTypeUint & found_greater_indeces ){


   std::cout << "initi matrix samples " << std::endl;
    unsigned int n_samples = gt_vector.rows();
    MatrixTypeUint found_greater_indeces_temp = MatrixTypeUint::Zero(n_samples,1);
    MatrixTypeUint found_smaller_indeces_temp = MatrixTypeUint::Zero(n_samples,1);


    std::cout << "entering loop samples samples " << std::endl;
    unsigned int i_found_great = 0;
    unsigned int i_found_small = 0;
    for(unsigned int i_sample = 0; i_sample<n_samples; i_sample++){
        if(gt_vector(i_sample)>=thresh){
            found_greater_indeces_temp(i_found_great) << i_sample;
            i_found_great++;
        }
        else{
            found_smaller_indeces_temp(i_found_small) << i_sample;
            i_found_small++;
        }
    }

  //  const unsigned int found_great_tot=i_found_great;
  //  const unsigned int found_small_tot=i_found_small;

    std::cout << "n samples: " << n_samples<< std::endl;
    //std::cout << "size great " << found_greater_indeces.rows() << std::endl;
   // std::cout << "size small " << found_smaller_indeces.rows() << std::endl;


    std::cout << "i_found_great " << i_found_great << std::endl;


     //found_greater_indeces.head(found_great_tot);
    found_greater_indeces = found_greater_indeces_temp.head(i_found_great);
    std::cout << "i_found_small " << i_found_small << std::endl;

    found_smaller_indeces = found_smaller_indeces_temp.head(i_found_small);


}


//template<typename VectorType>
//MatrixTypeUint getIndecesGreater(const VectorType &gt_vector, float thresh){

//    unsigned int n_samples = gt_vector.rows();
//    MatrixTypeUint found_greater_indeces = MatrixTypeUint::Zero(n_samples,1);

//    unsigned int i_found = 0;
//    for(unsigned int i_sample = 0; i_sample<n_samples; i_sample++){
//        if(gt_vector(i_sample)>=thresh){
//            found_greater_indeces(i_found) = i_sample;
//            i_found++;
//        }
//    }

//    found_greater_indeces = found_greater_indeces.head(i_found);
//    return found_greater_indeces;


//}

//template<typename VectorType>
//MatrixTypeUint getIndecesSmaller(const VectorType &gt_vector, float thresh){

//    unsigned int n_samples = gt_vector.rows();
//    MatrixTypeUint found_smaller_indeces = MatrixTypeUint::Zero(n_samples,1);

//    unsigned int i_found = 0;
//    for(unsigned int i_sample = 0; i_sample<n_samples; i_sample++){
//        if(gt_vector(i_sample)<thresh){
//            found_smaller_indeces(i_found) = i_sample;
//            i_found++;
//        }
//    }

//    found_smaller_indeces = found_smaller_indeces.head(i_found);
//    return found_smaller_indeces;


//}




////to instantiate explicitely
template bool getTrainSamplesFeaturesAndGt<MatrixTypeFloat,VectorTypeFloat>(const MatrixTypeFloat &features_matrix,const VectorTypeFloat &gt_vector,MatrixTypeFloat &sampled_features_matrix, VectorTypeFloat &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh);
template void getIndecesSmallerGreater<VectorTypeFloat>(const VectorTypeFloat &gt_vector ,float thresh,MatrixTypeUint &found_smaller_indeces,MatrixTypeUint & found_greater_indeces );
//template MatrixTypeUint getIndecesGreater<VectorTypeFloat>(const VectorTypeFloat &gt_vector ,float thresh);
//template MatrixTypeUint getIndecesSmaller<VectorTypeFloat>(const VectorTypeFloat &gt_vector ,float thresh);
