#include "sigen/binarizer/binarizer.h"
#include <cstdint>
#include <glog/logging.h>
#include <opencv2/imgproc/imgproc.hpp>
namespace sigen {
BinaryCube Binarizer::Binarize(const ImageSequence &is) {
  CHECK(!is.empty());
  int width = is[0].cols;
  int height = is[0].rows;
  BinaryCube cube(width, height, is.size());
  for (int z = 0; z < (int)is.size(); ++z) {
    const cv::Mat &image = is[z];
    CHECK_EQ(2, image.dims);
    CHECK_EQ(1, image.channels());
    CHECK_EQ(width, image.cols);
    CHECK_EQ(height, image.rows);
    cv::Mat bin;
    // newval = maxval if val > thresh else 0
    cv::threshold(image, bin, /* thresh = */ 127, /* maxval = */ 255,
                  cv::THRESH_BINARY);
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        cube[x][y][z] = bin.at<uint8_t>(y, x);
      }
    }
  }
  return cube;
}
} // namespace sigen
