#include "dataset.h"

#include <map>

bool writeLinesFeatures(vector<Branch*> branches, const QString &csv_path) {
    ofstream csv_file;
    csv_file.open(csv_path.toStdString().c_str());
//    csv_file<<"p1_x"<<','<<"p1_y"<<','<<"p1_z"<<','<<"p1_intensity"<<','<<"p1_linearity_3"<<','
//           <<"p1_linearity_5"<<','<<"p1_linearity_8"<<','<<"p1_near_edge"<<','
//          <<"p1_x"<<','<<"p1_y"<<','<<"p1_z"<<','<<"p1_intensity"<<','<<"p1_linearity_3"<<','
//         <<"p1_linearity_5"<<','<<"p1_linearity_8"<<','<<"p1_near_edge"<<','
//        <<"p1_x"<<','<<"p1_y"<<','<<"p1_z"<<','<<"p1_intensity"<<','<<"p1_linearity_3"<<','
//       <<"p1_linearity_5"<<','<<"p1_linearity_8"<<','<<"p1_near_edge"<<','
//      <<"p1_x"<<','<<"p1_y"<<','<<"p1_z"<<','<<"p1_intensity"<<','<<"p1_linearity_3"<<','
//     <<"p1_linearity_5"<<','<<"p1_linearity_8"<<','<<"p1_near_edge"<<',';
    for (int i = 1; i <= 5; ++i) {
        string stri = QString::number(i).toStdString();
        csv_file<<"px"<<stri<<','<<"py"<<stri<<','<<"pz"<<stri<<','<<"pintensity"<<stri<<','<<"plinearity_3_"<<stri<<','
               <<"plinearity_5_"<<stri<<','<<"plinearity_8_"<<stri<<','<<"pnear_edge"<<stri<<',';
    }
    for (int i = 0; i < 5; ++i) {
        string stri = QString::number(i).toStdString();
        csv_file<<"vx"<<stri<<','<<"vy"<<stri<<','<<"vz"<<stri<<',';
    }
    csv_file<<"intensity_mean"<<','<<"intensity_std"<<','
           <<"intensity_mean_r5"<<','<<"intensity_std_r5"<<','
          <<"length"<<','<<"distance"<<','<<"linearity_3_mean"<<','
         <<"linearity_5_mean"<<','<<"linearity_8_mean"<<endl;


    bool p = true;
    for (Branch* branch : branches) {
        for (PointFeature point : branch->line_feature.pointsFeature) {
            csv_file<<point.x<<','<<point.y<<','<<point.z<<','
                   <<point.intensity<<','<<point.linearity_3<<','
                  <<point.linearity_5<<','<<point.linearity_8<<','
                 <<point.nearEdge<<',';
            if (p) {
                cout<<point.x<<','<<point.y<<','<<point.z<<','
                   <<point.intensity<<','<<point.linearity_3<<','
                  <<point.linearity_5<<','<<point.linearity_8<<','
                 <<point.nearEdge<<',';
            }
        }
        for (XYZ v : branch->line_feature.directions) {
            csv_file<<v.x<<','<<v.y<<','<<v.z<<',';
            if (p) {
                cout<<v.x<<','<<v.y<<','<<v.z<<',';
            }
        }
        csv_file<<branch->line_feature.intensity_mean<<','<<branch->line_feature.intensity_std<<','
               <<branch->line_feature.intensity_mean_r5<<','<<branch->line_feature.intensity_std_r5<<','
              <<branch->length<<','<<branch->distance<<','<<branch->line_feature.linearity_3_mean<<','
             <<branch->line_feature.linearity_5_mean<<','<<branch->line_feature.linearity_8_mean<<endl;
        if (p) {
            cout<<branch->line_feature.intensity_mean<<','<<branch->line_feature.intensity_std<<','
               <<branch->line_feature.intensity_mean_r5<<','<<branch->line_feature.intensity_std_r5<<','
              <<branch->length<<','<<branch->distance<<','<<branch->line_feature.linearity_3_mean<<','
             <<branch->line_feature.linearity_5_mean<<','<<branch->line_feature.linearity_8_mean<<endl;
        }
        p = false;
    }
    csv_file.close();
    return true;
}

bool writeLinesMatrix(vector<Branch*> branches, const QString &csv_path, bool symmetry) {
    ofstream csv_file;
    csv_file.open(csv_path.toStdString().c_str());
    map<Branch*, int> branch_map;
    int size = branches.size();
    for (int i = 0; i < size; ++i) {
        branch_map[branches[i]] = i;
    }

    vector<vector<float> > line_matrix = vector<vector<float> >(size, vector<float>(size, 0));
    for (int i = 0; i < size; ++i) {
        int cIndex = i;
        if (branches[cIndex]->parent) {
            int pIndex = branch_map[branches[cIndex]->parent];
            if (symmetry) {
                line_matrix[cIndex][pIndex] = 1;
            }
            line_matrix[pIndex][cIndex] = 1;
        }
    }
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (j != size - 1) {
                csv_file<<line_matrix[i][j]<<',';
            } else {
                csv_file<<line_matrix[i][j]<<endl;

            }
        }
    }
    csv_file.close();
    return true;
}

vector<NeuronTree> getAllTreesInBlock(QString image_path, QString swc_brain_dir_path, QString out_dir_path, bool save_mid_result,
                                      const XYZ &center, int resolution, V3DPluginCallback2& callback) {
    vector<NeuronTree> results;

    unsigned char* pdata = 0;
    int data_type = 1;
    V3DLONG sz[4] = {0, 0, 0, 0};
    simple_loadimage_wrapper(callback, image_path.toStdString().c_str(), pdata, sz, data_type);

    XYZ box = XYZ(sz[0], sz[1], sz[2]);

    QDir swc_brain_dir(swc_brain_dir_path);
    if (!swc_brain_dir.exists()) {
        qDebug()<<"the swc brain dir is not exists";
        return results;
    }
    swc_brain_dir.setFilter(QDir::Files|QDir::NoSymLinks);
    QStringList filters;
    filters<<QString("*.swc")<<QString("*.eswc");
    swc_brain_dir.setNameFilters(filters);
    QFileInfoList swcPathInfo = swc_brain_dir.entryInfoList();

    for (auto swc : swcPathInfo) {
        QString swc_path = swc.absoluteFilePath();
        NeuronTree nt = readSWC_file(swc_path);
        qDebug()<<"nt size"<<nt.listNeuron.size();
        NeuronTree nt_shift_scale = shiftAndScaleNeuronTree(nt, center, box, resolution);
        qDebug()<<"nt_shift_scale size"<<nt_shift_scale.listNeuron.size();
        qDebug()<<"box size: "<<box.x<<" "<<box.y<<" "<<box.z;
        NeuronTree nt_crop = cropNeuronTree(nt_shift_scale, XYZ(0, 0, 0), box);
        qDebug()<<"nt_crop size"<<nt_crop.listNeuron.size();
        NeuronTree nt_refine = refineNeuronTree(nt_crop, pdata, sz);
        qDebug()<<"nt_refine size"<<nt_refine.listNeuron.size();
        vector<NeuronTree> nts_split = splitNeuronTree(nt_refine);
        qDebug()<<"nts_split size"<<nts_split.size();
        if (save_mid_result) {
            QString swc_name = swc.fileName();
            writeSWC_file(out_dir_path + "//" + swc_name + "_shift_scale.swc", nt_shift_scale);
            writeSWC_file(out_dir_path + "//" + swc_name + "_crop.swc", nt_crop);
            writeSWC_file(out_dir_path + "//" + swc_name + "_refine.swc", nt_refine);
            for (int i = 0; i < nts_split.size(); ++i) {
                writeSWC_file(out_dir_path + "//" + swc_name + "_split_" + QString::number(i) + ".swc", nts_split[i]);
            }
        }
        results.insert(results.end(), nts_split.begin(), nts_split.end());
    }
    qDebug()<<"tree size: "<<results.size();
    return results;
}

void getLinesInBlock(vector<NeuronTree> trees, QString lines_features_csv_path, QString lines_matrix_csv_path,
                     unsigned char *pdata, long long *sz,
                     float inflection_d, float cosAngleThres,
                     float l_thres_max, float l_thres_min, float t_length, float soma_ratio, bool symmetry) {
    vector<Branch*> results;
    for (NeuronTree nt : trees) {
        BranchTree bt;
        bt.initialize(nt);
        bt.preProcess(inflection_d, cosAngleThres, l_thres_max, l_thres_min, t_length, soma_ratio);
        bt.getFeature(pdata, sz);
        qDebug()<<"branch size: "<<bt.branches.size();
        results.insert(results.end(), bt.branches.begin(), bt.branches.end());
    }
    qDebug()<<"results size: "<<results.size();
    qDebug()<<"write lines features to "<<lines_features_csv_path;
    writeLinesFeatures(results, lines_features_csv_path);
    qDebug()<<"write lines matrix to "<<lines_matrix_csv_path;
    writeLinesMatrix(results, lines_matrix_csv_path, symmetry);
}


