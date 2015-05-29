#include "sqb_0.1/include/Eigen/Dense"

//using namespace Eigen;
typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;


template<typename MatrixType, typename VectorType>
void getTrainSamplesFeaturesAndGt(const MatrixType &features_matrix,const VectorType &gt_vector,MatrixType &sampled_features_matrix, VectorType &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh = 50.0){

//float thres = 50.0; //consider positive gt samples greater than this threshold


    unsigned int n_features_tot = features_matrix.cols();

//pos_samples = gt_vector.Array()>pos_thresh;
    sampled_features_matrix = MatrixType::Zero(n_neg_samples+n_pos_samples,n_features_tot);
    sampled_gt_vector = VectorType::Zero(n_neg_samples+n_pos_samples);


}




////to instantiate explicitely
template void getTrainSamplesFeaturesAndGt<MatrixTypeFloat,VectorTypeFloat>(const MatrixTypeFloat &features_matrix,const VectorTypeFloat &gt_vector,MatrixTypeFloat &sampled_features_matrix, VectorTypeFloat &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh);
