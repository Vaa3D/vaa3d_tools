#include "standardize_image_plugin.h"
#include <iostream>
using namespace std;

bool standardize(unsigned char* data, V3DLONG* sz) {
    V3DLONG num = sz[0] * sz[1] * sz[2];
    double median_of_mean = 5.312;
    double median_of_std = 2.267;
    double eps=1e-7;
    double mean = 0, std = 0;
    for (int i=0; i< num; i++){
        //mean += (double)data[i];
        mean += data[i];
    }
    mean /= num;
    cout<<"first_mean:"<<mean<<endl;
    for (int i=0; i< num; i++){
        //std += (double)((double)data[i] - mean) * ((double)data[i] - mean);
        std += (data[i] - mean) * (data[i] - mean);
    }
    std = sqrt((std /
                num));
    cout<<"first_std:"<<std<<endl;
    for (int i=0; i<num; i++){
        //double data_temp = (double)((data[i] - mean) / (std + eps) * median_of_std + median_of_mean);
        double data_temp = (double)((data[i] - mean) / (std + eps) * median_of_std + median_of_mean);
        if (data_temp < 0) data_temp = 0;
        else if (data_temp > 255) data_temp = 255;
        //data[i] = (unsigned char)round(data_temp);
        data[i] = data_temp;
    }
    return true;
}
