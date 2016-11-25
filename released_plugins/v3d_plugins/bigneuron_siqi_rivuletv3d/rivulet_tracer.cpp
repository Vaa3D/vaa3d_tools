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
  p = rk4(p, this->grad, this->bimg->get_dims(), 1);

  // Update Branch statas
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
    delete p;
    p = NULL;
  }
}

R2Tracer::R2Tracer() {}

R2Tracer::~R2Tracer() {
  if (this->bimg) {
    delete bimg;
    bimg = NULL;
  }

  if (this->bb) {
    delete this->bb;
    this->bb = NULL;
  }

  if (this->t) {
    delete this->t;
    this->t = NULL;
  }

  // cout<<"Try to print the first 10 of tt"<<endl;
  // for(int i=0;i<10;i++){
  //   cout<<this->tt->get_1d(i)<<endl;
  // }

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
  double start_time = this->tt->get(branch.get_tail().tolong());
  double end_time = this->tt->get(branch.get_head().tolong());

  // The erase region is the region between end_t<=tt<=start_t
  if (branch.get_length() > 5 && end_time < start_time) {
    for (int i = 0; i<this->tt->size(); i++) {
      if (this->tt->get_1d(i) >= end_time &&
          this->tt->get_1d(i) <= start_time && this->bb->get_1d(i) == true) {
        this->tt->set_1d(i, branch.is_low_conf() ? -2.0 : -1.0);
      }
    }
  } else{ // Simply erase with bb 
    for (int i = 0; i<this->tt->size(); i++) {
      if (this->bb->get_1d(i) == true) {
        this->tt->set_1d(i, branch.is_low_conf() ? -2.0 : -1.0);
      }
    }
  }

  // Fill bb with 0
  this->bb->fill_zero();
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
void R2Tracer::make_dist_gradient() {
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
          Point<V3DLONG> p(1 + x + Ne[i][0], 1 + y + Ne[i][1],
                           1 + z + Ne[i][2]);
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
  this->grad = new double[nvox * 3];
  std::copy(Fx, Fx + nvox, this->grad);
  std::copy(Fy, Fy + nvox, this->grad + nvox);
  std::copy(Fz, Fz + nvox, this->grad + nvox * 2);
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

  long nforeground = this->bimg->sum();
  this->coverage = (float) ctr / (float) nforeground;
  cout<<ctr<<"/"<<(long)nforeground<<endl;
}

SWC *R2Tracer::iterative_backtrack() {
  cout << "== Iterative backtrack..." << endl;
  SWC *swc = new SWC();
  srand(time(NULL));
  float eps = 1e-5;
  this->bb = new Image3<unsigned char>(this->bimg->get_dims());

  this->dilated_bimg = this->bimg;// TODO: dilation

  // Get the gradient of the Time-crossing map
  this->make_dist_gradient();

  // Count the number of foreground voxels
  V3DLONG nforeground = (V3DLONG) this->bimg->sum();

  // Mark background tt to -2, soma region to -3
  for (int i = 0; i < this->bimg->size(); i++) {
    if (this->bimg->get_1d(i) == false) {
      this->tt->set_1d(i, -2.0);
    } else if (this->soma->get_mask()->get_1d(i) == true) {
      this->tt->set_1d(i, -3.0);
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
    cout<<"iter"<<endl;

    // if (maxtidx < 0) {
    //   cout<<"No more postive t left"<<endl;
    //   break;
    // }

    Point<float> srcpt(maxtidx, this->bimg->get_dims());
    Branch branch;
    branch.add(srcpt, 1.0); // Add the initial point
    this->tt->set(srcpt.tolong(), -1.0); // Just in case
    this->update_coverage();

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

      this->tt->set(head.tolong(), -1.0); // Just in case

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
    cout<<"Keep is "<<keep<<" swc size: "<<swc->size()<<" Cov: "<<this->coverage<<endl;
    if (keep){
      swc->add_branch(branch, pid);
    }

  }

  return swc;
}
