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
    float xysz = (float)dims[0] * (float)dims[1];
    this->z = (T)(floor((float)idx1d / (float)xysz));
    this->y = (T)(idx1d - this->z * xysz);
    this->y = (T)floor((float)this->y / (float)dims[0]);
    this->x = (T)floor(idx1d - this->z * xysz - this->y * (float)dims[0]);
  }

  Point() {
    this->x = -1;
    this->y = -1;
    this->z = -1;
  }

  long make_linear_idx(long *dims) {
    long xysz = (long)(dims[0] * dims[1]);
    return (long)this->z * xysz + this->y * dims[0] + this->x;
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
  int radius = 1;
  int pid = -2;
  SWCNode(int id, int type, Point<float> p, int radius, int pid)
      : id(id), type(type), p(p), radius(radius), pid(pid) {}
  SWCNode() {}
};

// Image3 is implemented here since templates cannot be compiled
template <typename T>
class Image3 {
 private:
  T *data1d = NULL;
  long *dims;
  int nvox;
  bool destroy = true;

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
      vox[i] = (unsigned char)(((float)this->data1d[i] > threshold) ? 1 : 0);
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
    long idx = p.make_linear_idx(this->dims);
    return this->get_1d(idx);
  }

  void set(Point<long> p, T val) {
    long idx = p.make_linear_idx(this->dims);
    this->set_1d(idx, val);
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

 public:
  SWC() {}  // Initialise with an empty SWC
  void add_branch(Branch &branch, long connect_id);
  void add_node(SWCNode n);
  void plus1();
  long size();
  long match(SWCNode n);
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
  float ma_short = -1;
  float ma_long = -1;
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
  void update(Point<float>, Image3<unsigned char> *);
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
  const static float target_coverage = 0.98;

  void prep();  // Distance Transform and MSFM
  Image3<double>* makespeed(Image3<float> *dt);
  SWC *iterative_backtrack();
  void make_gradient();
  void update_coverage();
  void step(Branch &);
  void erase(Branch &);
  void binary_sphere(Branch &, vector<int> &);

 public:
  R2Tracer();
  ~R2Tracer();
  void reset();
  SWC *trace(Image3<unsigned char> *img, float threshold);
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
