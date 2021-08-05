#include <opencv2/opencv.hpp>

using namespace cv;

void points_norm(Mat x, Mat& T, Mat& xn);

void pca(Mat x, Mat y, Mat& TPCA, Mat& xp, Mat& tar_vec);

Mat affine3d(Mat x, Mat y);

Mat calc_vy(Mat vx, Mat y, double t);

Mat am_acol(Mat x, int flag);