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
               const string& mean_file,
               const string& label_file);
    std::vector<std::vector<Prediction> > Classify(const std::vector<cv::Mat>& imgs, int N = 2);
private:
    void SetMean(const string& mean_file);
    std::vector<std::vector<float> > Predict(const std::vector<cv::Mat>& imgs);
    void WrapInputLayer(std::vector<cv::Mat>* input_channels, int n);
    void Preprocess(const cv::Mat& img,
                    std::vector<cv::Mat>* input_channels);
private:
    shared_ptr<Net<float> > net_;
    cv::Size input_geometry_;
    int num_channels_;
    cv::Mat mean_;
    std::vector<string> labels_;
};

static bool PairCompare(const std::pair<float, int>& lhs,
                        const std::pair<float, int>& rhs);

QStringList importSeriesFileList_addnumbersort(const QString & curFilePath);

#endif

