#ifndef _RIVULET_H
#define _RIVULET_H

#include "fastmarching/msfm.h"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <typeinfo>
#include <vector>
#include "fastmarching/fastmarching_dt.h"

namespace rivulet {
class R2Tracer;

template <typename T> class Point {
public:
  T x = 0;
  T y = 0;
  T z = 0;

  Point(T x, T y, T z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Point(long idx1d, long *dims) { // Plays as ind to sub
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

  long make_linear_idx(long *dims){
    long xysz = (long)(dims[0] * dims[1]);
    return (long) this->z * xysz + this->y * dims[0] + this->x;
  }

  float dist(const Point<T> p) {
    Point d = *this - p;
    return pow(d.x * d.x + d.y * d.y + d.z * d.z, 0.5);
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
  SWCNode(){}
};

// Image3 is implemented here since templates cannot be compiled
template <typename T> class Image3 {
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

    this->dims = new long[3];
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
    cout << "deleting image:" << typeid(T).name() << endl;
    if (this->data1d && this->destroy) {
      delete[] this->data1d;
      this->data1d = NULL;
    }

    if (this->dims) {
      delete[] this->dims;
      this->dims = NULL;
    }

    cout << "finshed deleting image:" << typeid(T).name() << endl;
  }

  Image3<unsigned char> *binarize(float threshold) {
    unsigned char *vox = new unsigned char[this->nvox];
    for (int i = 0; i < this->nvox; i++) {
      vox[i] = (unsigned char)(((float)this->data1d[i] > threshold) ? 1 : 0);
    }

    Image3<unsigned char> *bimg = new Image3<unsigned char>(vox, this->dims);
    return bimg;
  }

  T max(){
    T m = 0;
    for (int i = 0; i < this->nvox; i++){
        m = this->data1d[i] > m ? this->data1d[i] : m;
    }
    return m;
  }

  T sum(){
    T s = 0;
    for (int i = 0; i < this->nvox; i++){
        s += this->data1d[i];
    }
    return s;
  }

  long *get_dims() { return this->dims; }
  T *get_data1d_ptr() { return this->data1d; }

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
      vox[i] = (double)this->data1d[i];
    }
    return new Image3<double>(vox, this->get_dims());
  }

  T get_1d(long idx) { return this->data1d[idx]; }
  void set_1d(long idx, T v) { this->data1d[idx] = v; }

  long size() { return this->nvox; }
};

class Soma {
private:
  Image3<unsigned char> *mask = NULL;

public:
  Point<float> centroid;
  float radius = -1;
  Image3<unsigned char>* get_mask();
  Soma(Point<float> centroid, float radius);
  ~Soma();
  void make_mask(Image3<unsigned char> *bimg);
};

class SWC {
private:
  vector<SWCNode> nodes;

public:
  SWC(){} // Initialise with an empty SWC
  void add_branch(vector<Point<float> > branch, vector<int> rlist, long pid);
  void add_node(SWCNode n);
  long size();
  long match(SWCNode n);
};

class R2Tracer {
private:
  Image3<unsigned char> *bimg = NULL;
  Image3<double> *t = NULL;  // Original timemap
  Image3<double> *tt = NULL; // The copy of the timemap
  Soma *soma = NULL;
  bool silent = false;
  float coverage = 0.;
  double* grad = NULL;

  void prep(); // Distance Transform and MSFM
  void makespeed(Image3<float> *dt);
  SWC *iterative_backtrack();
  double* make_dist_gradient();

public:
  R2Tracer();
  ~R2Tracer();
  void reset();
  SWC *trace(Image3<unsigned char> *img, float threshold);
};
}

#endif
