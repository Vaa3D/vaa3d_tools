#ifndef _SAMPLING_H
#define _SAMPLING_H


template<typename MatrixType, typename VectorType>
void getTrainSamplesFeaturesAndGt(const MatrixType &features_matrix,const VectorType &gt_vector,MatrixType &sampled_features_matrix, VectorType &sampled_gt_vector,unsigned int n_pos_samples,unsigned int n_neg_samples,float pos_thresh = 50.0);






#endif
