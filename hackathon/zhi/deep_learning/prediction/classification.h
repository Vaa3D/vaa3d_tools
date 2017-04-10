#ifndef __CLASSIFICATION_H__
#define __CLASSIFICATION_H__


#include <caffe/caffe.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "prediction_caffe_plugin.h"

using namespace caffe; // NOLINT(build/namespaces)
using std::string;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;
class Classifier {
public:
    Classifier(const string& model_file,
               const string& trained_file,
               const string& mean_file);
    std::vector<std::vector<float> > Predict(const std::vector<cv::Mat>& imgs);
    std::vector< float > PredictBatch(const vector< cv::Mat > imgs);

    std::vector<std::vector<float> > extractFeature_siamese(const std::vector<cv::Mat>& imgs);


private:
    void SetMean(const string& mean_file);
    void WrapInputLayer(std::vector<cv::Mat>* input_channels, int n);
    void Preprocess(const cv::Mat& img,
                    std::vector<cv::Mat>* input_channels);
    void WrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch);
    void PreprocessBatch(const vector<cv::Mat> imgs,
                                          std::vector< std::vector<cv::Mat> >* input_batch);

private:
    shared_ptr<Net<float> > net_;
    cv::Size input_geometry_;
    int num_channels_;
    cv::Mat mean_;
    int batch_size_ ;
};

QStringList importSeriesFileList_addnumbersort(const QString & curFilePath);
NeuronTree DL_eliminate_swc(NeuronTree nt,QList <ImageMarker> marklist);
NeuronTree remove_swc(NeuronTree nt,double length);
std::vector<std::vector<float> > batch_detection(unsigned char * & data1d,Classifier classifier, int N, int M, int P, int Sxy);
QList <ImageMarker> batch_deletion(unsigned char * & data1d,Classifier classifier, QList <ImageMarker> input_marker, int N, int M, int P);
//void connect_swc(NeuronTree nt,QList<NeuronSWC>& newNeuron, double disThr,double angThr);

#endif

