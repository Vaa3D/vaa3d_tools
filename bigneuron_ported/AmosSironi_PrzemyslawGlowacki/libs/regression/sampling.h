#ifndef _SAMPLING_H
#define _SAMPLING_H

typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;
typedef Eigen::Matrix<unsigned int,-1,1 > MatrixTypeUint;

template<typename MatrixType, typename VectorType>
bool getTrainSamplesFeaturesAndGt(const MatrixType &features_matrix,const VectorType &gt_vector,MatrixType &sampled_features_matrix, VectorType &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh = 100.0);


template<typename VectorType>
void getIndecesSmallerGreater(const VectorType &gt_vector, float thresh, MatrixTypeUint &found_smaller_indeces,MatrixTypeUint & found_greater_indeces );


#endif
