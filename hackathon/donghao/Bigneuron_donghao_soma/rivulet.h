#ifndef _RIVULET_H
#define _RIVULET_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <numeric>
#include <typeinfo>
#include <vector>
#include "fastmarching/fastmarching_dt.h"
#include "fastmarching/msfm.h"
#include "utils/marker_radius.h"
#include "utils/rk4.h"

#define max2(a,b) \
 ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
   _a > _b ? _a : _b; })

#define min2(a,b) \
 ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
   _a < _b ? _a : _b; })

namespace rivulet {

class R2Tracer;
class Branch;

template <typename T>
vector<T> slice_vector(const vector<T> &v, int start = 0, int end = -1) {
  int oldlen = v.size();
  int newlen = (end == -1 || end >= oldlen) ? oldlen - start : end - start;
  vector<T> nv(newlen);
  for (int i = 0; i < newlen; i++) {
    nv[i] = v[start + i];
  }
  return nv;
}

template <typename T>
class Point {
 public:
  T x = 0;
  T y = 0;
  T z = 0;

  Point(T x, T y, T z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Point(long idx1d, long *dims) {  // Plays as ind to sub
    long chunk = dims[0] * dims[1];
    z = idx1d / chunk; 
    idx1d -= z * chunk; 

    y = idx1d / dims[0]; 
    idx1d -= y * dims[0]; 

    x = idx1d / 1;
  }

  Point() {
    this->x = -1;
    this->y = -1;
    this->z = -1;
  }

  long make_linear_idx(long *dims) {
    return (long) (this->z * dims[0] * dims[1] + this->y * dims[0] + this->x);
  }

  float dist(const Point<T> p) {
    Point d = *this - p;
    return pow(d.x * d.x + d.y * d.y + d.z * d.z, 0.5);
  }

  Point<long> tolong() {
    return Point<long>((long)this->x, (long)this->y, (long)this->z);
  }

  Point<int> toint() {
    return Point<int>((int) this->x, (int) this->y, (int) this->z);
  }

  Point<double> todouble() {
    return Point<double>((double)this->x, (double)this->y, (double)this->z);
  }

  vector<Point<T> > neighbours_3d(int radius) {
    vector<Point> neighbours;
    // Return the coordinates of neighbours within a radius
    for (float xgv = this->x - radius; xgv <= this->x + radius; xgv++)
      for (float ygv = this->y - radius; ygv <= this->y + radius; ygv++)
        for (float zgv = this->z - radius; zgv <= this->z + radius; zgv++) {
          Point<T> p(xgv, ygv, zgv);
          neighbours.push_back(p);
        }
    return neighbours;
  }

  Point<T> operator-(const Point &other) {
    Point pt;
    pt.x = this->x - other.x;
    pt.y = this->y - other.y;
    pt.z = this->z - other.z;
    return pt;
  }

  T *make_array() {
    T *a = new T[3];
    a[0] = this->x;
    a[1] = this->y;
    a[2] = this->z;
    return a;
  }
};

class SWCNode {
 public:
  int id = -2;
  int type = 2;
  Point<float> p;
  float radius = 1;
  int pid = -2;
  SWCNode(int id, int type, Point<float> p, int radius, int pid)
      : id(id), type(type), p(p), radius(radius), pid(pid) {}
  SWCNode() {}

};

// The == operator only cares about the node id
inline bool operator==(const SWCNode& lhs, const SWCNode& rhs)
{
  return lhs.id==rhs.id ? true:false;
}

struct CropRegion{
    long xmin=-1;
    long xmax=-1;
    long ymin=-1;
    long ymax=-1;
    long zmin=-1;
    long zmax=-1;
};

// Image3 is implemented here since templates cannot be compiled
template <typename T>
class Image3 {
 private:
  T *data1d = NULL;
  long *dims;
  int nvox;
  bool destroy = true;
  CropRegion crop_region;

  bool kernelcheck(Point<long> p){
    Point<long> t;
    // Make the neighbour position kernel
    long ne[27][3] = {{-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1}, {-1, 0, -1},
                      {-1, 0, 0},   {-1, 0, 1},  {-1, 1, -1}, {-1, 1, 0},
                      {-1, 1, 1},   {0, -1, -1}, {0, -1, 0},  {0, -1, 1},
                      {0, 0, -1},   {0, 0, 0},   {0, 0, 1},   {0, 1, -1},
                      {0, 1, 0},    {0, 1, 1},   {1, -1, -1}, {1, -1, 0},
                      {1, -1, 1},   {1, 0, -1},  {1, 0, 0},   {1, 0, 1},
                      {1, 1, -1},   {1, 1, 0},   {1, 1, 1}};
    for (int i = 0; i < 27; i++) {
      t.x = p.x + ne[i][0];
      t.y = p.y + ne[i][1];
      t.z = p.z + ne[i][2];
      if (this->get(t) > 0){
        return true;
      }
    }

    return false;
  }

 public:
  void set_destroy(bool destroy) { this->destroy = destroy; }

  Image3() {
    this->data1d = NULL;
    this->dims = NULL;
    nvox = 0;
  }

  Image3(long *dims) {
    /*
    Initializer with only long[3] dims
    Initialize an image with all zeros
    */

    this->dims = new long[3]();
    this->dims[0] = dims[0];
    this->dims[1] = dims[1];
    this->dims[2] = dims[2];
    long nvox = dims[0] * dims[1] * dims[2];
    this->nvox = nvox;
    this->data1d = new T[nvox]();
  }

  Image3(Image3<T> *img) {
    long *dims = img->get_dims();
    T *img_1d_ptr = img->get_data1d_ptr();
    this->data1d = new T[img->size()];
    this->dims = new long[3];
    this->dims[0] = dims[0];
    this->dims[1] = dims[1];
    this->dims[2] = dims[2];
    this->nvox = img->size();
    for (int i = 0; i < this->nvox; i++) {
      this->data1d[i] = img_1d_ptr[i];
    }
  }

  Image3(T *data1d, long *dims) {
    /*
    Initializer with both an 1d array and long* dims
    */

    this->dims = new long[3];
    this->dims[0] = dims[0];
    this->dims[1] = dims[1];
    this->dims[2] = dims[2];
    this->nvox = dims[0] * dims[1] * dims[2];
    this->data1d = data1d;
  }

  ~Image3() {
    if (this->data1d && this->destroy) {
      delete[] this->data1d;
      this->data1d = NULL;
    }

    if (this->dims) {
      delete[] this->dims;
      this->dims = NULL;
    }
  }

  // Simply dilate the image > 0 with 1 iteration
  // Skip the border
  Image3<unsigned char> *dilate() {
    Point<long> p;
    Image3<unsigned char> *dilated = new Image3<unsigned char>(this->dims);

    for (p.x = 1; p.x < this->dims[0] - 1; ++p.x)
      for (p.y = 1; p.y < this->dims[1] - 1; ++p.y)
        for (p.z = 1; p.z < this->dims[2] - 1; ++p.z) {
          if (this->kernelcheck(p)) {
            dilated->set(p, 1);
          }
        }
    return dilated;
  }

  // Crop this image according to the input region definition
  // The original image will be over-written
  // (To be done) : To get the original image back, call this->restore_crop()
  Image3<T>* manualcrop(CropRegion crop_region){
    std::vector<Point<long> > pts;
    Point<long> p;
    for (p.x = 0; p.x < this->dims[0]; ++p.x)
      for (p.y = 0; p.y < this->dims[1]; ++p.y)
        for (p.z = 0; p.z < this->dims[2]; ++p.z) {
          if (this->get(p) > 0){
            pts.push_back(p);
          }
        }

    // Make the cropped image data
    long crop_dims[3];
    crop_dims[0] = crop_region.xmax - crop_region.xmin + 1;
    crop_dims[1] = crop_region.ymax - crop_region.ymin + 1;
    crop_dims[2] = crop_region.zmax - crop_region.zmin + 1;
    long nvox = crop_dims[0] * crop_dims[1] * crop_dims[2];
    T* croped_data = new T[nvox];

    // Copy the data to the cropped image
    Point<long> p_crop;
    for (p.x = crop_region.xmin, p_crop.x = 0; p.x < crop_region.xmax;
         ++p.x, ++p_crop.x)
      for (p.y = crop_region.ymin, p_crop.y = 0; p.y < crop_region.ymax;
           ++p.y, ++p_crop.y)
        for (p.z = crop_region.zmin, p_crop.z = 0; p.z < crop_region.zmax;
             ++p.z, ++p_crop.z) {
          croped_data[p_crop.make_linear_idx(crop_dims)] = this->get(p);
        }

    Image3<T>* cropped_image = new Image3<T>(croped_data, crop_dims);
    cropped_image->set_crop_region(crop_region);
    return cropped_image;
  }

  Image3<unsigned char> *region_threshold(CropRegion crop_region, float threshold) {
    cout<<"test : region threshold has been tested"<<endl;
    std::vector<Point<long> > pts;
    Point<long> p;
    for (p.x = 0; p.x < this->dims[0]; ++p.x)
        for (p.y = 0; p.y < this->dims[1]; ++p.y)
            for (p.z = 0; p.z < this->dims[2]; ++p.z) {
                pts.push_back(p);
      }
    long crop_dims[3];
    crop_dims[0] = this->dims[0];
    crop_dims[1] = this->dims[1];
    crop_dims[2] = this->dims[2];
    cout<<"test: crop_dims[0]"<<crop_dims[0]<<"crop_dims[1]"<<crop_dims[1]<<"crop_dims[2]"<<crop_dims[2]<<endl;
    long nvox = crop_dims[0] * crop_dims[1] * crop_dims[2];
    T* croped_data = new T[nvox];
    Point<long> p_crop;
    for (p.x = 0, p_crop.x = 0; p.x < crop_dims[0];
         ++p.x, ++p_crop.x)
      for (p.y = 0, p_crop.y = 0; p.y < crop_dims[1];
           ++p.y, ++p_crop.y)
        for (p.z = 0, p_crop.z = 0; p.z < crop_dims[2];
             ++p.z, ++p_crop.z) {
            if ((p.x > crop_region.xmin) & (p.x < crop_region.xmax) & (p.y > crop_region.ymin) & (p.y < crop_region.ymax) & (p.z > crop_region.zmin) & (p.z < crop_region.zmax))
            {
                if (this->get(p) > threshold)
                {
                    croped_data[p_crop.make_linear_idx(crop_dims)] = 200;
                }
                else
                {
                    croped_data[p_crop.make_linear_idx(crop_dims)] = 0;
                }
            }
            else
                croped_data[p_crop.make_linear_idx(crop_dims)] = 0;
        }
    Image3<T>* cropped_image = new Image3<T>(croped_data, crop_dims);
    cropped_image->set_crop_region(crop_region);
    return cropped_image;
  }

  // Crop this image to keep only the region > 0
  // The original image will be over-written
  // To get the original image back, call this->restore_crop()
  Image3<T>* autocrop(){
    std::vector<Point<long> > pts;
    Point<long> p;
    for (p.x = 0; p.x < this->dims[0]; ++p.x)
      for (p.y = 0; p.y < this->dims[1]; ++p.y)
        for (p.z = 0; p.z < this->dims[2]; ++p.z) {
          if (this->get(p) > 0){
            pts.push_back(p);
          }
        }

    CropRegion crop_region;
    crop_region.xmin = this->dims[0], crop_region.ymin = this->dims[1], crop_region.zmin = this->dims[2],
    crop_region.xmax = 0, crop_region.ymax = 0, crop_region.zmax = 0;

    for (std::vector<Point<long> >::iterator it = pts.begin(); it != pts.end();
         ++it) {
      if (it->x <= crop_region.xmin){
        crop_region.xmin = it->x;
      }

      if (it->x >= crop_region.xmax){
        crop_region.xmax = it->x + 1;
      }

      if (it->y <= crop_region.ymin){
        crop_region.ymin = it->y;
      }

      if (it->y >= crop_region.ymax){
        crop_region.ymax = it->y + 1;
      }

      if (it->z <= crop_region.zmin){
        crop_region.zmin = it->z;
      }

      if (it->z >= crop_region.zmax){
        crop_region.zmax = it->z + 1;
      }
    }

    // Make the cropped image data
    long crop_dims[3];
    crop_dims[0] = crop_region.xmax - crop_region.xmin + 1;
    crop_dims[1] = crop_region.ymax - crop_region.ymin + 1;
    crop_dims[2] = crop_region.zmax - crop_region.zmin + 1;
    long nvox = crop_dims[0] * crop_dims[1] * crop_dims[2];
    T* croped_data = new T[nvox];

    // Copy the data to the cropped image
    Point<long> p_crop;
    for (p.x = crop_region.xmin, p_crop.x = 0; p.x < crop_region.xmax;
         ++p.x, ++p_crop.x)
      for (p.y = crop_region.ymin, p_crop.y = 0; p.y < crop_region.ymax;
           ++p.y, ++p_crop.y)
        for (p.z = crop_region.zmin, p_crop.z = 0; p.z < crop_region.zmax;
             ++p.z, ++p_crop.z) {
          croped_data[p_crop.make_linear_idx(crop_dims)] = this->get(p);
        }

    Image3<T>* cropped_image = new Image3<T>(croped_data, crop_dims);
    cropped_image->set_crop_region(crop_region);
    return cropped_image;
  }

  void set_crop_region(CropRegion rg){
    this->crop_region = rg;
  }

  CropRegion get_crop_region(){
    return this->crop_region;
  }

  Image3<T> *make_copy() {
    T *img_p = new T[this->nvox];
    for (int i = 0; i < this->nvox; i++) img_p[i] = this->data1d[i];
    Image3<T> *img = new Image3<T>(img_p, this->dims);
    return img;
  }

  void save(char *fname, bool normalise=false) {
    // Normalise need to be set true if T is not unsigned char
    V3DLONG dims_4d[4];
    dims_4d[0] = this->dims[0];
    dims_4d[1] = this->dims[1];
    dims_4d[2] = this->dims[2];
    dims_4d[3] = 1;

    if (normalise){
      T max = this->max();
      unsigned char* td = new unsigned char[this->nvox];
      for(int i=0;i<this->nvox;i++){
        td[i] = (unsigned char)(ceil(this->data1d[i] / max * 255 ));
      }
      saveImage(fname, td, dims_4d, V3D_UINT8);
      if (td){delete td; td = NULL;}
    }
    else{
      unsigned char * td = (unsigned char*) this->data1d;
      saveImage(fname, td, dims_4d, V3D_UINT8);
    }
  }

  bool is_in_bound(Point<long> pt) {
    // Check for nan
    if (pt.x != pt.x || pt.y != pt.y || pt.z != pt.z ){
      return false;
    }

    // Check for out of bound
    if (pt.x >= 0 && pt.x <= this->dims[0] && pt.y >= 0 &&
        pt.y <= this->dims[1] && pt.z >= 0 && pt.z <= this->dims[2]) {
      return true;
    } else {
      return false;
    }
  }

  Image3<unsigned char> *binarize(float threshold) {
    unsigned char *vox = new unsigned char[this->nvox];
    for (int i = 0; i < this->nvox; i++) {
      vox[i] = (unsigned char) ((float) this->data1d[i]) > threshold ? 1 : 0;
    }

    Image3<unsigned char> *bimg = new Image3<unsigned char>(vox, this->dims);
    return bimg;
  }

  Image3<unsigned char> *bi_mask(float threshold) {
    unsigned char *vox = new unsigned char[this->nvox];
    for (int i = 0; i < this->nvox; i++) {
      vox[i] = (unsigned char) ((float) this->data1d[i]) > threshold ? 50 : 0;
    }

    Image3<unsigned char> *bimg = new Image3<unsigned char>(vox, this->dims);
    return bimg;
  }

  T max() {
    T m = -1;
    for (int i = 0; i < this->nvox; i++) {
      m = this->data1d[i] > m ? this->data1d[i] : m;
    }
    return m;
  }

  long sum() {
    long s = 0;
    for (int i = 0; i < this->nvox; i++) {
      s += (long)this->data1d[i];
    }

    return s;
  }

  long *get_dims() { return this->dims; }

  T *get_data1d_ptr() { return this->data1d; }

  T get(Point<long> p) {
    return this->get_1d(p.make_linear_idx(this->dims));
  }

  void set(Point<long> p, T val) {
    this->set_1d(p.make_linear_idx(this->dims), val);
  }

  /* Find the max linear idx of the 3D volume*/
  long max_idx_1d() {
    T max = -1;
    long maxidx = -1;
    for (int i = 0; i < this->nvox; i++) {
      if (max < this->data1d[i]) {
        max = data1d[i];
        maxidx = i;
      }
    }

    return maxidx;
  }

  Image3<double> *to_double() {
    double *vox = new double[this->size()];
    for (int i = 0; i < this->size(); i++) {
      vox[i] = (double) this->data1d[i];
    }
    return new Image3<double>(vox, this->get_dims());
  }

  T get_1d(long idx) {
    if (idx < this->nvox && idx >= 0)
      return this->data1d[idx];
    else
      return 0;
  }
  void set_1d(long idx, T v) {
    if (idx < this->nvox && idx >= 0) this->data1d[idx] = v;
  }

  long size() { return this->nvox; }

  void fill_zero() {
    memset(this->data1d, 0, sizeof(*this->data1d) * this->nvox);
  }
};

class Soma {
 private:
  Image3<unsigned char> *mask = NULL;

 public:
  Point<float> centroid;
  float radius = -1;
  Image3<unsigned char> *get_mask();
  Soma(Point<float> centroid, float radius);
  ~Soma();
  void make_mask(Image3<unsigned char> *bimg);
};

class SWC {
 private:
  vector<SWCNode> nodes;
  void prune_leaves();
  void prune_unreached();
  SWCNode get_parent(SWCNode);

 public:
  SWC() {}  // Initialise with an empty SWC
  void add_branch(Branch &branch, long connect_id);
  void add_node(SWCNode n);
  long size();
  long match(SWCNode n);
  void pad(CropRegion);
  void prune();
  SWCNode get_node(int i);
};

class Branch {
 private:
  // Data
  vector<Point<float> > pts;
  vector<float> conf;
  vector<float> radius;
  float stepsz = 0;

  double velocity[3];
  // For controlling the tracking
  float gap = 0;
  double branch_len = 0.0;  // The length of branch measured by voxels
  bool reached_soma = false;
  bool touched = false;  // Touched previously traced branch
  bool low_online_conf = false;
  int touchidx = -2;  // The swc index it touched at
  int steps_after_touch = 0;

  // For online confidence computing
  int online_voxsum = 0;
  float ma_short = -1.;
  float ma_long = -1.;
  const static int ma_short_window = 4, ma_long_window = 10;
  bool in_valley = false;

  // Private functions
  static float exponential_moving_average(float, float, int);
  void update_ma(float oc);
  int estimate_radius(Point<float>, Image3<unsigned char> *);

 public:
  Branch(){};
  void add(Point<float>, float, float);
  void slice(int startidx, int endidx);  // Left inclusive, right exclusive
  void update(Point<float>, Image3<unsigned char> *, Image3<unsigned char> *);
  float mean_radius();
  float get_gap();
  int get_length();
  void reset_gap();
  void set_touched(bool reached);
  Point<float> get_head();
  Point<float> get_tail();
  vector<float> get_radius();
  float get_radius_at(int);
  float get_step_size();
  bool is_stucked();
  void set_low_conf(bool);
  bool is_low_conf();
  void set_touch_idx(int idx);
  int get_touch_idx();
  int get_steps_after_touch();
  Point<float> get_point(int);
  void reach_soma();
  bool is_reach_soma();
  float get_curve_length();
};

class R2Tracer {
 private:
  Image3<unsigned char> *bimg = NULL;
  Image3<unsigned char> *dilated_bimg = NULL;
  long bsum = 0;
  Image3<unsigned char> *bb = NULL;  // For making the erasing contour
  Image3<double> *t = NULL;          // Original timemap
  Image3<double> *tt = NULL;         // The copy of the timemap
  Soma *soma = NULL;
  bool silent = false;
  float coverage = 0.;
  double *grad = NULL;
  float target_coverage = 0.98;
  bool quality = false;

  void prep();  // Distance Transform and MSFM
  Image3<double>* makespeed(Image3<float> *dt);
  SWC *iterative_backtrack();
  void make_gradient();
  void update_coverage();
  void step(Branch &);
  void erase(Branch &);
  void binary_sphere(Branch &, vector<int> &);
  bool is_prune = false;

 public:
  R2Tracer();
  void set_quality(bool);
  ~R2Tracer();
  void reset();
  SWC *trace(Image3<unsigned char> *img, float threshold);
  SWC *scentre();
  Image3<unsigned char> *soma_img = NULL;
  Image3<unsigned char> *soma_img_whole= NULL;
  void set_prune(bool p){
    this->is_prune = p;
  };
};
}

static float constrain(float x, float low, float high) {
  if (x < low) return low;
  if (x > high) return high;
  return x;
}

__inline double norm3(double *a) {
  return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

#endif
