#ifndef DATASET_H
#define DATASET_H

#include "op_neurontree.h"
#include "branchtree.h"

bool writeLinesFeatures(vector<Branch *> branches, const QString& csv_path);

bool writeLinesMatrix(vector<Branch *> branches, const QString& csv_path, bool symmetry);

vector<NeuronTree> getAllTreesInBlock(QString image_path, QString swc_brain_dir_path, QString out_dir_path, bool save_mid_result,
                                      const XYZ &center, int resolution, V3DPluginCallback2& callback);

void getLinesInBlock(vector<NeuronTree> trees, QString lines_features_csv_path, QString lines_matrix_csv_path,
                     unsigned char* pdata, V3DLONG* sz,
                     float inflection_d, float cosAngleThres,
                     float l_thres_max, float l_thres_min, float t_length, float soma_ratio, bool symmetry);


#endif // DATASET_H
