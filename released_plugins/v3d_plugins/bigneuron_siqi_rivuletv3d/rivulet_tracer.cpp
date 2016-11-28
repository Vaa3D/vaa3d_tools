#include "rivulet.h"
using namespace rivulet;

void Branch::add(Point<float> pt, float conf, float radius = 1.0) {
  this->pts.push_back(pt);
  this->conf.push_back(conf);
  this->radius.push_back(radius);
}

void Branch::set_touched(bool touched) { this->touched = true; }

Point<float> Branch::get_point(int i) { return this->pts[i]; }

float Branch::get_radius_at(int idx) { return this->radius[idx]; }

int Branch::get_steps_after_touch() { return this->steps_after_touch; }

vector<float> Branch::get_radius() { return this->radius; }

float Branch::get_step_size() { return this->stepsz; }

void Branch::set_low_conf(bool low) {this->low_online_conf = low;}

bool Branch::is_stucked() {
  if (this->pts.size() > 15) {
    if (this->get_head().dist(this->pts[this->get_length() - 15]) > 1) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool Branch::is_low_conf() { return this->low_online_conf; }

int Branch::get_touch_idx() { return this->touchidx; }

float Branch::get_curve_length() { return this->branch_len; }

void Branch::set_touch_idx(int idx) { this->touchidx = idx; }

void Branch::reach_soma(){this->reached_soma = true;}
bool Branch::is_reach_soma(){return this->reached_soma;}

void Branch::slice(int start, int end) {
  this->pts = slice_vector(this->pts, start, end);
  this->conf = slice_vector(this->conf, start, end);
  this->radius = slice_vector(this->radius, start, end);
}

int Branch::get_length() { return this->pts.size(); }

float Branch::exponential_moving_average(float p, float ema, int n) {
  /*

    The exponential moving average (EMA) traditionally
    used in analysing stock market.
    EMA_{i+1} = (p * \alpha) + (EMA_{i} * (1 - \alpha))
    where p is the new value; EMA_{i} is the last ema value;
    n is the time period; \alpha=2/(1+n) is the smoothing factor.

    ---------------------------------------------
    Parameters:
    p: The new value in the sequence
    ema: the last EMA value
    n: The period window size
*/
  float alpha = 2.0 / (float)(1.0 + n);
  return p * alpha + ema * (1 - alpha);
}

void Branch::update(Point<float> pt, Image3<unsigned char> *bimg) {
  float eps = 1e-5;
  Point<float> head = this->pts.back();
  this->velocity[0] = pt.x - head.x;
  this->velocity[1] = pt.y - head.y;
  this->velocity[2] = pt.z - head.z;
  this->stepsz = norm3(this->velocity);
  this->branch_len += this->stepsz;
  unsigned char b = bimg->get(pt.tolong());
  if (b > 0) {
    this->gap += this->stepsz;
  }

  // The online confidence (OC) starts from 0.5 due to the +1 term,
  // It is helpful to make the OC score from noise points decay fast
  this->online_voxsum += b;
  float oc = this->online_voxsum / this->pts.size() + 1; // Online confidence

  this->update_ma(oc);

  // We are stepping in a valley
  if (this->ma_short < ma_long - eps && oc < 0.5 && !this->in_valley) {
    this->in_valley = true;
  }

  // Cut at the valley
  if (this->in_valley && this->ma_short > this->ma_long) {
    int valleyidx =
        std::distance(this->conf.begin(),
                      std::min_element(this->conf.begin(), this->conf.end()));

    // Only cut if the valley oc is below 0.5
    if (this->conf[valleyidx] < 0.5) {
      // Slice the branch up to valleyidx
      this->slice(0, valleyidx);
      this->low_online_conf = true;
    } else {
      in_valley = false;
    }
  }

  // Count steps after touch
  if (this->touched) {
    this->steps_after_touch++;
  }

  // Estimate the radius
  int r = this->estimate_radius(pt, bimg);
  this->add(pt, oc, r);
}

/*Update the two MA curves of OC*/
void Branch::update_ma(float oc) {
  // Compute the two MA curves of OC
  if (this->pts.size() > ma_long_window) {
    // Short MA
    if (this->ma_short == -1) {
      this->ma_short = oc;
    } else {
      int N = (this->pts.size() >= ma_short_window) ? ma_short_window
                                                    : this->pts.size();
      this->ma_short = exponential_moving_average(oc, ma_short, N);
    }

    // Long MA
    if (this->ma_long == -1) {
      this->ma_long = oc;
    } else {
      int N = (this->pts.size() >= ma_long_window) ? ma_long_window
                                                   : this->pts.size();
      this->ma_long = exponential_moving_average(oc, ma_long, N);
    }
  }
}

float Branch::mean_radius() {
  return accumulate(this->radius.begin(), this->radius.end(), 0.0) /
         this->radius.size();
}

float Branch::get_gap() { return this->gap; }

void Branch::reset_gap() { this->gap = 0; }

Point<float> Branch::get_head() { return this->pts.back(); }
Point<float> Branch::get_tail() { return this->pts[0]; }

int Branch::estimate_radius(Point<float> pt, Image3<unsigned char> *bimg) {
  MyMarker marker;
  marker.x = pt.x;
  marker.y = pt.y;
  marker.z = pt.z;
  unsigned char *bimg_ptr = bimg->get_data1d_ptr();
  return markerRadius(bimg_ptr, bimg->get_dims(), marker, 0.0, 2);
}

/* Trace for one step
*/
void R2Tracer::step(Branch &branch) {
  // RK4 walk for one step
  double *p = branch.get_head().todouble().make_array();
  cout<<"In R2tracer::step:"<<"startpoint:"<<p[0]<<" "<<p[1]<<" "<<p[2]<<endl;
  p = rk4(p, this->grad, this->bimg->get_dims(), (double) 1.0);

  // if (p[0]!=p[0] ||p[1]!=p[1] || p[2] != p[2]){ // nan since gradient 0 
  //   branch.set_low_conf(true);
  //   return;
  // }

  // Update Branch stats
  Point<float> endpt((float)p[0], (float)p[1], (float)p[2]);

  // Momentum boost: If the velocity is too small, sprint a bit with momentum
  if (branch.get_step_size() < 0.5 && branch.get_length() >= 5) {
    endpt.x =
        branch.get_head().x +
        (branch.get_head().x - branch.get_point(branch.get_length() - 4).x);
    endpt.y =
        branch.get_head().y +
        (branch.get_head().y - branch.get_point(branch.get_length() - 4).y);
    endpt.z =
        branch.get_head().z +
        (branch.get_head().z - branch.get_point(branch.get_length() - 4).z);
  }

  // Update the branch stats
  branch.update(endpt, this->bimg);

  if (p) {
    delete [] p;
    p = NULL;
  }
}

R2Tracer::R2Tracer() {}

R2Tracer::~R2Tracer() {
  if (this->bimg) {
    delete bimg;
    bimg = NULL;
  }

  if (this->dilated_bimg) {
    delete dilated_bimg;
    dilated_bimg = NULL;
  }

  if (this->bb) {
    delete this->bb;
    this->bb = NULL;
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
    delete [] this->grad;
    this->grad = NULL;
  }
}

/* Make a contour volume for the branch on bb */
void R2Tracer::binary_sphere(Branch& branch, vector<int>& radius){
  Point<int> p;
  long* dims = this->bimg->get_dims();
  for (int i = 0; i < branch.get_length(); i++) {
    vector<Point<float> > neighbours = branch.get_point(i).neighbours_3d(radius[i]);
    for (int n = 0; n < neighbours.size(); n++) {
      p.x = constrain(neighbours[n].x, 0.0, dims[0] - 1);
      p.y = constrain(neighbours[n].y, 0.0, dims[1] - 1);
      p.z = constrain(neighbours[n].z, 0.0, dims[2] - 1);
      V3DLONG idx = (V3DLONG) p.make_linear_idx(dims);
      this->bb->set_1d(idx, 1);
    }
  }
}

/* Erase a branch from the time crossing map */
void R2Tracer::erase(Branch &branch) {
  float eraseratio = 1.1;
  vector<int> r_large;
  for (int i = 0; i < branch.get_length(); i++) {
    r_large.push_back(ceil(branch.get_radius_at(i) * eraseratio + 1));
  }

  this->binary_sphere(branch, r_large);
  double start_time = this->t->get(branch.get_tail().tolong());
  double end_time = this->t->get(branch.get_head().tolong());

  // The erase region is the region between end_t<=tt<=start_t
  if (branch.get_length() > 5 && end_time < start_time) {
    for (int i = 0; i<this->tt->size(); i++) {
      if (this->t->get_1d(i) >= end_time &&
          this->t->get_1d(i) <= start_time && this->bb->get_1d(i) == true) {
        this->tt->set_1d(i, (double) branch.is_low_conf() ? -2.0 : -1.0);
      }
    }
  } else{ // Simply erase with bb 
    for (int i = 0; i<this->tt->size(); i++) {
      if (this->bb->get_1d(i) == true) {
        this->tt->set_1d(i, (double) branch.is_low_conf() ? -2.0 : -1.0);
      }
    }
  }

  // Fill bb with 0
  this->bb->fill_zero();
}

SWC *R2Tracer::trace(Image3<unsigned char> *img, float threshold) {
  this->bimg = img->binarize(threshold);
  this->prep();
  this->t->save("t.v3draw", true);
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
  fastmarching_dt(this->bimg->get_data1d_ptr(), bdist1d, dims[0],
                  dims[1], dims[2], 2, 0);

  bimg->save("bimg.v3draw");
  Image3<float> *dt = new Image3<float>(bdist1d, this->bimg->get_dims());

  dt->save("dt.v3draw", true);

  // Find the source point
  long max_dt_idx = dt->max_idx_1d();
  long max_dt = dt->get_1d(max_dt_idx);
  Point<float> max_dt_point(max_dt_idx, this->bimg->get_dims());
  this->soma = new Soma(max_dt_point, max_dt * 2);
  this->soma->make_mask(this->bimg);

  this->soma->get_mask()->save("soma_mask.v3draw");

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
  this->tt = this->t->make_copy();

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
void R2Tracer::make_dist_gradient() {
  V3DLONG *dims = this->bimg->get_dims();
  V3DLONG nvox = this->bimg->size();
  V3DLONG j_in_sz[3]; // matrix for jacobian
  j_in_sz[0] = dims[0] + 2;
  j_in_sz[1] = dims[1] + 2;
  j_in_sz[2] = dims[2] + 2;
  V3DLONG j_nvox = j_in_sz[0] * j_in_sz[1] * j_in_sz[2];
  Image3<double>* Fx = new Image3<double>(dims);
  Image3<double>* Fy = new Image3<double>(dims);
  Image3<double>* Fz = new Image3<double>(dims);
  double *J = new double[j_nvox](); // Will be freed by jacobian image
  std::fill(J, J + j_nvox, this->t->max());
  Image3<double>* jacobian = new Image3<double>(J, j_in_sz);

  // Assign the center of J to T
  for (long v = 0; v < nvox; v++) {
    Point<V3DLONG> pt(v, dims);
    Point<V3DLONG> pj(pt.x + 1, pt.y + 1, pt.z + 1);
    jacobian->set(pj, this->t->get(pt));
  }

  // Make the neighbour position kernel
  double Ne[26][3] = {{-1.0, -1.0, -1.0}, {-1.0, -1.0, 0.0}, {-1.0, -1.0, 1.0},
                     {-1.0, 0.0, -1.0},  {-1.0, 0.0, 0.0},  {-1.0, 0.0, 1.0},
                     {-1.0, 1.0, -1.0},  {-1.0, 1.0, 0.0},  {-1.0, 1.0, 1.0},
                     {0.0, -1.0, -1.0},  {0.0, -1.0, 0.0},  {0.0, -1.0, 1.0},
                     {0.0, 0.0, -1.0},   {0.0, 0.0, 1.0},   {0.0, 1.0, -1.0},
                     {0.0, 1.0, 0.0},    {0.0, 1.0, 1.0},   {1.0, -1.0, -1.0},
                     {1.0, -1.0, 0},     {1.0, -1.0, 1.0},  {1.0, 0.0, -1.0},
                     {1.0, 0.0, 0.0},    {1.0, 0.0, 1.0},   {1.0, 1.0, -1.0},
                     {1.0, 1.0, 0.0},    {1.0, 1.0, 1.0}};

  for (int i = 0; i < 26; i++)
    for (long v = 0; v < nvox; v++) {
      Point<V3DLONG> pt(v, dims);
      Point<V3DLONG> pj(pt.x + 1 + Ne[i][0], pt.y + 1 + Ne[i][1],
                        pt.z + 1 + Ne[i][2]);
      double in = jacobian->get(pj);
      if (in < this->t->get(pt)) {
        double powsum = sqrt(Ne[i][0] * Ne[i][0] + Ne[i][1] * Ne[i][1] +
                             Ne[i][2] * Ne[i][2]);
        Fx->set(pt, -Ne[i][0] / powsum);
        Fy->set(pt, -Ne[i][1] / powsum);
        Fz->set(pt, -Ne[i][2] / powsum);
      }
    }

  // Change grad to 1d
  this->grad = new double[nvox * 3];
  std::copy(Fx->get_data1d_ptr(), Fx->get_data1d_ptr() + nvox, this->grad);
  std::copy(Fy->get_data1d_ptr(), Fy->get_data1d_ptr() + nvox,
            this->grad + nvox);
  std::copy(Fz->get_data1d_ptr(), Fz->get_data1d_ptr() + nvox,
            this->grad + nvox * 2);

  if (Fx) {
    delete Fx;
    Fx = NULL;
  }
  if (Fy) {
    delete Fy;
    Fy = NULL;
  }
  if (Fz) {
    delete Fz;
    Fz = NULL;
  }
  if (jacobian) {
    delete jacobian;
    jacobian = NULL;
  }
}

void R2Tracer::makespeed(Image3<float> *dt) {
  for (int i = 0; i < dt->size(); i++) {
    if (this->bimg->get_1d(i) > 0) {
      dt->set_1d(i, pow(dt->get_1d(i), 4));
    } else {
      dt->set_1d(i, 1e-10);
    }
  }
}

/* Update the coverage percentile according to the timecrossing map and the
 * bimg*/
void R2Tracer::update_coverage() {
  long ctr = 0;
  long nvox = this->bimg->size();
  for (int i = 0; i < nvox; i++) {
    if (this->tt->get_1d(i) < 0 && this->bimg->get_1d(i) > 0) {
      ctr++;
    }
  }

  cout << "ctr/bsum:" << ctr << "/" << bsum << endl;
  this->coverage = (float)ctr / (float)this->bsum;
}

SWC *R2Tracer::iterative_backtrack() {
  cout << "== Iterative backtrack..." << endl;
  SWC *swc = new SWC();
  srand(time(NULL));
  float eps = 1e-5;
  this->bb = new Image3<unsigned char>(this->bimg->get_dims());

  this->dilated_bimg = this->bimg->make_copy();// TODO: dilation

  // Get the gradient of the Time-crossing map
  this->make_dist_gradient();

  // Count the number of foreground voxels
  V3DLONG nforeground = (V3DLONG) this->bimg->sum();

  // Mark background tt to -2, soma region to -3
  for (int i = 0; i < this->bimg->size(); i++) {
    if (this->bimg->get_1d(i) == 0) {
      this->tt->set_1d(i, (double) -2.0);
    } else if (this->soma->get_mask()->get_1d(i) > 0) {
      this->tt->set_1d(i, (double) -3.0);
    }
  }

  Image3<unsigned char> *dilate_bimg = this->bimg;
  SWCNode soma_node(0, 1, soma->centroid, soma->radius, -1);
  swc->add_node(soma_node);
  this->bsum = this->bimg->sum(); // The total number of foreground voxels

  // Iteration for different branches
  while (this->coverage < this->target_coverage) {
    // Find the geodesic furthest point on foreground time-crossing-map
    long maxtidx = this->tt->max_idx_1d();

    Point<float> srcpt(maxtidx, this->bimg->get_dims());
    Branch branch;
    branch.add(srcpt, 1.0); // Add the initial point
    this->tt->set(srcpt.tolong(), (double)-1.0); // Just in case
    cout<<"Cov before:"<<this->coverage<<endl;
    this->update_coverage();
    cout<<"Cov after:"<<this->coverage<<endl;

    bool keep = true;
    // Iteration for one branch
    while (true) {
      this->step(branch);
      Point<float> head = branch.get_head();

      // 1. Check ouf of bound
      if (!this->bimg->is_in_bound(head.tolong())) {
        branch.slice(0, branch.get_length()-1); // Exclude last node
        break;
      }

      this->tt->set(head.tolong(), (double) -1.0); // Just in case

      // 2. Check for the large gap criterion
      unsigned char end_pt_b =
          this->dilated_bimg->get(head.tolong());
      if (branch.get_gap() > branch.mean_radius() * 8) {
        break;
      } else {
        branch.reset_gap();
      }

      // 3. Check if soma has been reached
      if (this->soma->get_mask()->get(head.tolong()) > 0) {
        keep = branch.get_curve_length() > 15 ? true : false;
        branch.reach_soma();
        break;
      }

      // 4. If it has not moved for 15 iterations, stop
      if (branch.is_stucked()) {
        break;
      }

      // 5. Check for low online confidence
      if (branch.is_low_conf()) {
        keep = false;
        break;
      }

      // 6. Check for branch merge
      // Consider reaches previous explored area traced with branch
      // Note: when the area was traced due to noise points
      // (erased with -2), not considered as 'reached'
      if (this->tt->get(head.tolong()) == -1.0) {
        branch.set_touched(true);

        // Stop exploring if there has not been any branch yet
        if (swc->size() == 1) {
          break;
        }

        SWCNode n(-1, 1, head,
                  branch.get_radius_at(branch.get_length() - 1), -1);
        long matched_idx = swc->match(n);
        if (matched_idx > 0) {
          branch.set_touch_idx(matched_idx);
          break;
        }

        if (branch.get_steps_after_touch() > 200) {
          break;
        }
      }
    }

    cout<<"== Got a branch with size:"<<branch.get_length()<<endl;
    // Erase it from the timemap
    this->erase(branch);

    // Find the node ID to connect to
    int pid = -2;
    if (branch.is_reach_soma()) {
      pid = 0;
    } else if (branch.get_touch_idx() > 0) {
      pid = swc->get_node(branch.get_touch_idx()).id;
    }

    // Add to the SWC if the OC score is high
    if (keep){
      swc->add_branch(branch, pid);
    }

  }

  return swc;
}
