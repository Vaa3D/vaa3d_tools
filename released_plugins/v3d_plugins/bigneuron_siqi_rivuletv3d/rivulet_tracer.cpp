#include "rivulet.h"
using namespace rivulet;

R2Tracer::R2Tracer() {}
R2Tracer::~R2Tracer() {
  cout << "Trying to destroy R2Tracer" << endl;
  if (this->bimg) {
    delete bimg;
    bimg = NULL;
  }

  if (this->t) {
    delete this->t;
    this->t = NULL;
  }

  if (this->tt) {
    delete this->tt;
    this->tt = NULL;
  }

  if (this->soma) {
    delete this->soma;
    this->soma = NULL;
  }

  if (this->grad) {
    delete this->grad;
    this->grad = NULL;
  }
}

SWC *R2Tracer::trace(Image3<unsigned char> *img, float threshold) {
  this->bimg = img->binarize(threshold);
  this->prep();
  SWC *swc = this->iterative_backtrack();
  return swc;
}

void R2Tracer::prep() {
  // // Fast marching distance transform proposed in APP2
  if (!this->silent) {
    cout << "--DT..." << endl;
  }
  long *dims = this->bimg->get_dims();
  float *bdist1d = NULL;
  fastmarching_dt(this->bimg->get_data1d_ptr(), bdist1d, (int)dims[0],
                  (int)dims[1], (int)dims[2], 2, 0);
  Image3<float> *dt = new Image3<float>(bdist1d, this->bimg->get_dims());
  cout << endl << "--DT finished" << endl;

  // Find the source point
  long max_dt_idx = dt->max_idx_1d();
  long max_dt = dt->get_1d(max_dt_idx);
  Point<float> max_dt_point(max_dt_idx, this->bimg->get_dims());
  this->soma = new Soma(max_dt_point, max_dt * 2);
  this->soma->make_mask(this->bimg);

  // Make Speed Image dt**4 if bimg>0
  this->makespeed(dt);

  // Marching on the Speed Image
  float *sp = this->soma->centroid.make_array();
  int sp_int[3];
  sp_int[0] = (int)sp[0];
  sp_int[1] = (int)sp[1];
  sp_int[2] = (int)sp[2];

  Image3<double> *dt_double = dt->to_double();

  if (!this->silent)
    cout << "== MSFM..." << endl;
  double *t_ptr = msfm(dt_double->get_data1d_ptr(), dims, sp_int, false, false,
                       false); // Original Timemap
  if (!this->silent)
    cout << "== MSFM finished..." << endl;

  this->t = new Image3<double>(t_ptr, this->bimg->get_dims());
  this->tt = new Image3<double>(this->t);

  if (dt) {
    delete dt;
    dt = NULL;
  }

  if (dt_double) {
    delete dt_double;
    dt_double = NULL;
  }

  if (sp) {
    delete[] sp;
    sp = NULL;
  }
}

/* Make the gradients for back-tracking*/
double *R2Tracer::make_dist_gradient() {
  V3DLONG j_in_sz[3]; // matrix for jacobian
  V3DLONG *dims = this->bimg->get_dims();
  V3DLONG nvox = this->bimg->size();
  j_in_sz[0] = dims[0] + 2;
  j_in_sz[1] = dims[1] + 2;
  j_in_sz[2] = dims[2] + 2;
  int j_nvox = j_in_sz[0] * j_in_sz[1] * j_in_sz[2];
  double *Fx = new double[nvox];
  std::fill(Fx, Fx + nvox, 0.0);
  double *Fy = new double[nvox];
  std::fill(Fy, Fy + nvox, 0.0);
  double *Fz = new double[nvox];
  std::fill(Fz, Fz + nvox, 0.0);
  double *J = new double[j_nvox];
  double maxt = (double)this->t->max();
  std::fill(J, J + j_nvox, maxt);

  // Assign the center of J to T
  for (int x = 0; x < dims[0]; x++)
    for (int y = 0; y < dims[1]; y++)
      for (int z = 0; z < dims[2]; z++) {
        Point<long> p(x + 1, y + 1, z + 1);
        long jloc = p.make_linear_idx(j_in_sz);
        p.x = x;
        p.y = y;
        p.z = z;
        long tloc = p.make_linear_idx(dims);
        J[jloc] = this->t->get_1d(tloc);
      }

  // Make the neighbour position kernel
  short *Ne[27];
  int ctr = 0;
  for (int i = -1; i <= 1; i++)
    for (int j = -1; j <= 1; j++)
      for (int k = -1; k <= 1; k++) {
        Ne[ctr] = new short[3];
        Ne[ctr][0] = i;
        Ne[ctr][1] = j;
        Ne[ctr][2] = k;
        ctr++;
      }

  for (int i = 0; i < 27; i++)
    for (int x = 0; x < dims[0]; x++)
      for (int y = 0; y < dims[1]; y++)
        for (int z = 0; z < dims[2]; z++) {
          Point<V3DLONG> p(1 + x + Ne[i][0], 1 + y + Ne[i][1], 1 + z + Ne[i][2]);
          V3DLONG jloc = p.make_linear_idx(j_in_sz);
          p.x = x;
          p.y = y;
          p.z = z;
          V3DLONG tloc = p.make_linear_idx(dims);
          double in = J[jloc];
          if (in < this->t->get_1d(tloc)) {
            this->t->set_1d(tloc, in);
            float powsum =
                Ne[i][0] * Ne[i][0] + Ne[i][1] * Ne[i][1] + Ne[i][2] * Ne[i][2];
            Fx[tloc] = -Ne[i][0] / sqrt(powsum);
            Fy[tloc] = -Ne[i][1] / sqrt(powsum);
            Fz[tloc] = -Ne[i][2] / sqrt(powsum);
          }
        }

  // Change to grad to 1d
  double *grad4d = new double[nvox * 3];
  std::copy(Fx, Fx + nvox, grad4d);
  std::copy(Fy, Fy + nvox, grad4d + nvox);
  std::copy(Fz, Fz + nvox, grad4d + nvox * 2);
  if (Fx) {
    delete[] Fx;
    Fx = 0;
  }
  if (Fy) {
    delete[] Fy;
    Fy = 0;
  }
  if (Fz) {
    delete[] Fz;
    Fz = 0;
  }
  if (J) {
    delete[] J;
    J = 0;
  }

  return grad4d;
}

SWC *R2Tracer::iterative_backtrack() {
  cout << "== Iterative backtrack..." << endl;
  SWC *swc = new SWC();
  srand(time(NULL));
  float TARGET_COVERAGE = 0.98;
  float eps = 1e-5;

  // Make a boolean image
  Image3<unsigned char>* bb = new Image3<unsigned char>(this->bimg->get_dims());

  // Get the gradient of the Time-crossing map
  this->make_dist_gradient();

  // Count the number of foreground voxels
  V3DLONG nforeground = (V3DLONG) this->bimg->sum();

  // Mark background tt to -2, soma region to -3
  for (int i = 0; i < this->bimg->size(); i++) {
    if (this->bimg->get_1d(i) == false) {
      this->tt->set_1d(i, -2);
    } else if (this->soma->get_mask()->get_1d(i) == true) {
      this->tt->set_1d(i, -3);
    }
  }

  // TODO: bimg->dilate();
  Image3<unsigned char> *dilate_bimg = this->bimg;
  SWCNode soma_node(0, 1, soma->centroid, soma->radius, -1);
  swc->add_node(soma_node);

  
  if (bb){delete bb; bb = NULL;}
  return swc;
}

void R2Tracer::makespeed(Image3<float> *dt) {
  float *dtptr = dt->get_data1d_ptr();
  for (int i = 0; i < dt->size(); i++) {
    if (this->bimg->get_1d(i) > 0) {
      dtptr[i] = pow(dtptr[i], 4);
    } else {
      dtptr[i] = 1e-10;
    }
  }
}
