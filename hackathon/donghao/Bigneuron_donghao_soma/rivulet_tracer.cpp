#include "rivulet.h"
#include <ctime>
#include <iomanip>
// #include "tqdm/tqdm.h"
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

void Branch::set_low_conf(bool low) { this->low_online_conf = low; }

bool Branch::is_stucked() {
  if( this->velocity[0]==0 && this->velocity[1]==0 && this->velocity[2]==0)
    return true;

  if (this->pts.size() > 15) {
    if (this->get_head().dist(this->pts[this->get_length() - 15]) < 1) {
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

void Branch::reach_soma() { this->reached_soma = true; }
bool Branch::is_reach_soma() { return this->reached_soma; }

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
  float alpha = 2.0 / (float)(1 + n);
  return p * alpha + ema * (1 - alpha);
}

void Branch::update(Point<float> pt, Image3<unsigned char> *bimg, Image3<unsigned char> *dilated_bimg) {
  float eps = 1e-5;
  Point<float> head = this->pts.back();
  this->velocity[0] = pt.x - head.x;
  this->velocity[1] = pt.y - head.y;
  this->velocity[2] = pt.z - head.z;
  this->stepsz = norm3(this->velocity);
  this->branch_len += this->stepsz;
  unsigned char b = dilated_bimg->get(pt.tolong());
  if (b > 0) {
    this->gap += this->stepsz;
  }

  // The online confidence (OC) starts from 0.5 due to the +1 term,
  // It is helpful to make the OC score from noise points decay fast
  this->online_voxsum += (long) b;
  float oc = (float) this->online_voxsum / (float)(this->pts.size() + 1); // Online confidence

  this->update_ma(oc);

  // printf("oc:%.3f\tma_short:%.3f\tma_long:%.3f\tb:%d\n", oc, ma_short, ma_long, b);
  // We are stepping in a valley
  if (this->get_length() > this->ma_long_window && this->ma_short < this->ma_long - eps && oc < 0.5 && !this->in_valley) {
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

  if (oc <= 0.2) {low_online_conf = true;}

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
  if (this->get_length() > this->ma_long_window) {
    // Short MA
    if (this->ma_short == -1) {
      this->ma_short = oc;
    } else {
      int N = (this->pts.size() >= this->ma_short_window) ? this->ma_short_window
                                                    : this->get_length();
      this->ma_short = exponential_moving_average(oc, this->ma_short, N);
    }

    // Long MA
    if (this->ma_long == -1) {
      this->ma_long = oc;
    } else {
      int N = (this->pts.size() >= this->ma_long_window) ? this->ma_long_window
                                                   : this->get_length();
      this->ma_long = exponential_moving_average(oc, this->ma_long, N);
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
  int r = 0;
  long* dims = bimg->get_dims();
  Point<long> centre(floor(pt.x), floor(pt.y), floor(pt.z));
  Point<long> p;

  while(1){
    ++r;
    try{
      int s = 0;
      for(p.x = max2(centre.x - r, 0); p.x < min2(centre.x+r+1, dims[0]); p.x++)
        for(p.y = max2(centre.y - r, 0); p.y < min2(centre.y+r+1, dims[1]); p.y++)
          for(p.z = max2(centre.z - r, 0); p.z < min2(centre.z+r+1, dims[2]); p.z++){
            s += bimg->get(p);
          }
      if( ((double) s) / (double) pow(2*r+1, 3) < 0.6){break;}
    }
    catch(...){
      break;
    }
  }
  return r;
}

/* Trace for one step
*/
void R2Tracer::step(Branch &branch) {
  // RK4 walk for one step
  double *p = branch.get_head().todouble().make_array();
  p = rk4(p, this->grad, this->bimg->get_dims(), 1);

  // Update Branch stats
  Point<float> endpt((float)p[0], (float)p[1], (float)p[2]);

  // Update the branch stats
  branch.update(endpt, this->bimg, this->dilated_bimg);

  if (p) {
    delete[] p;
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
    delete[] this->grad;
    this->grad = NULL;
  }
}

/* Make a contour volume for the branch on bb */
void R2Tracer::binary_sphere(Branch &branch, vector<int> &radius) {
  Point<V3DLONG> p;
  V3DLONG *dims = this->bimg->get_dims();
  for (int i = 0; i < branch.get_length(); i++) {
    vector<Point<float> > neighbours =
        branch.get_point(i).neighbours_3d(radius[i]);
    for (int n = 0; n < neighbours.size(); n++) {
      p.x = constrain(neighbours[n].x, 0.0, dims[0] - 1);
      p.y = constrain(neighbours[n].y, 0.0, dims[1] - 1);
      p.z = constrain(neighbours[n].z, 0.0, dims[2] - 1);
      this->bb->set(p, 1);
    }
  }
}

/* Erase a branch from the time crossing map */
void R2Tracer::erase(Branch &branch) {
  float eraseratio = 1.5;
  vector<int> r_large;
  for (int i = 0; i < branch.get_length(); i++) {
    r_large.push_back(ceil(branch.get_radius_at(i) * eraseratio));
  }

  this->binary_sphere(branch, r_large);

  double start_time = this->t->get(branch.get_tail().tolong());
  double end_time = this->t->get(branch.get_head().tolong());

  // The erase region is the region between end_t<=tt<=start_t
  if (branch.get_curve_length() > 5 && end_time < start_time) {
    for (int i = 0; i < this->tt->size(); i++) {
      if (this->t->get_1d(i) >= end_time && this->t->get_1d(i) <= start_time &&
          this->bb->get_1d(i) == true) {
        this->tt->set_1d(i, branch.is_low_conf() ? -2.0 : -1.0);
      }
    }
  } else { // Simply erase with bb
    for (int i = 0; i < this->tt->size(); i++) {
      if (this->bb->get_1d(i) == true) {
        this->tt->set_1d(i, branch.is_low_conf() ? -2.0 : -1.0);
      }
    }
  }

  // Fill bb with 0
  this->bb->fill_zero();
}

SWC *R2Tracer::trace(Image3<unsigned char> *img, float threshold) {
  int start_time = clock();
  this->bimg = img->binarize(threshold);
//  this->bimg->save("/home/donghao/Desktop/vaa3d/vaa3d_tools/hackathon/donghao/Bigneuron_donghao_soma/test_data/testxx.tif", false);
  //cout<<"test : I printed the step after the image is binarized"<<endl;
  if(!this->silent) cout<<"Step One : Binarization took -- "<< (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;
//  Image3<unsigned char>* timg = this->bimg->autocrop();
  // if (this->bimg){
  //   delete this->bimg;
  //   this->bimg = 0;
  // }
  // this->bimg = timg;

  // this->prep();

  SWC *swc = this->scentre();
  // The initial somatic point : somapt
  SWCNode ini_soma_pt;
  ini_soma_pt = swc->get_node(0);
  CropRegion soma_bounding_box;
  float scale_box = 3;
  long *dims = this->bimg->get_dims();
  soma_bounding_box.xmin = ini_soma_pt.p.x - ini_soma_pt.radius * scale_box;
  soma_bounding_box.xmax = ini_soma_pt.p.x + ini_soma_pt.radius * scale_box;
  soma_bounding_box.ymin = ini_soma_pt.p.y - ini_soma_pt.radius * scale_box;
  soma_bounding_box.ymax = ini_soma_pt.p.y + ini_soma_pt.radius * scale_box;
  soma_bounding_box.zmin = ini_soma_pt.p.z - ini_soma_pt.radius * scale_box;
  soma_bounding_box.zmax = ini_soma_pt.p.z + ini_soma_pt.radius * scale_box;
  cout<<"test: ini spt x: "<<ini_soma_pt.p.x<<" ini spt y: "<<ini_soma_pt.p.y<<" ini spt z: "<<ini_soma_pt.p.z<<endl;
  cout<<"test: xmin "<<soma_bounding_box.xmin<<" "<<"xmax "<<soma_bounding_box.xmax<<endl;
  cout<<"test: ymin "<<soma_bounding_box.ymin<<" "<<"ymax "<<soma_bounding_box.ymax<<endl;
  cout<<"test: zmin "<<soma_bounding_box.zmin<<" "<<"zmax "<<soma_bounding_box.zmax<<endl;
  cout<<"test: the radius of the somapt is "<<ini_soma_pt.radius<<endl;
  soma_bounding_box.xmin = max2(0, soma_bounding_box.xmin);
  soma_bounding_box.xmax = min2(dims[0], soma_bounding_box.xmax);
  soma_bounding_box.ymin = max2(0, soma_bounding_box.ymin);
  soma_bounding_box.ymax = min2(dims[1], soma_bounding_box.ymax);
  soma_bounding_box.zmin = max2(0, soma_bounding_box.zmin);
  soma_bounding_box.zmax = min2(dims[2], soma_bounding_box.zmax);
  cout<<"test: xmin "<<soma_bounding_box.xmin<<" "<<"xmax "<<soma_bounding_box.xmax<<endl;
  cout<<"test: ymin "<<soma_bounding_box.ymin<<" "<<"ymax "<<soma_bounding_box.ymax<<endl;
  cout<<"test: zmin "<<soma_bounding_box.zmin<<" "<<"zmax "<<soma_bounding_box.zmax<<endl;
  cout<<"test: dims[0] "<<dims[0]<<", dims[1] "<<dims[1]<<", dims[2] "<<dims[2]<<endl;
  // SWC *swc = this->iterative_backtrack();
  Image3<unsigned char>* somaimg;
  somaimg = img->manualcrop(soma_bounding_box);
  somaimg->save("/home/donghao/Desktop/vaa3d/vaa3d_tools/hackathon/donghao/Bigneuron_donghao_soma/test_data/soma.tif", false);
  // if(!this->silent) cout <<endl<<endl<< "Totally Rivulet2 took -- " << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;
  // if(this->is_prune){
  //   swc->prune();
  // }

  // swc->pad(this->bimg->get_crop_region()); // Pad the swc back
  // return swc;
  this->soma_img = somaimg;
  return swc;
}

void show_progress(float perc, long completed, long total) {
  int bar_width = 40;
  std::cout << std::setprecision(5) << perc * 100.0 << "%\t|";
  int pos = bar_width * perc;
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos)
      std::cout << "█";
    else if (i == pos)
      std::cout << "";
    else
      std::cout << " ";
  }
  std::cout << "|\t" << completed << "/" << total << "\r";
  std::cout.flush();
}

SWC *R2Tracer::scentre() {
  // // Fast marching distance transform proposed in APP2
  cout<<"Function scentre is been called"<<endl;
  if (!this->silent) cout << "Step Two : Boundary Distance Transform Time Spent is ";
  long *dims = this->bimg->get_dims();
  float *bdist1d = NULL;
  int start_time = -1;
  start_time = clock();
  fastmarching_dt(this->bimg->get_data1d_ptr(), bdist1d, dims[0], dims[1],
                  dims[2], 2, 0);
  if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  Image3<float> *dt = new Image3<float>(bdist1d, this->bimg->get_dims());

  // Find the source point
  long max_dt_idx = dt->max_idx_1d();
  long max_dt = dt->get_1d(max_dt_idx);
  cout<<"max_dt_idx: "<<(int) max_dt_idx<<endl;
  Point<float> max_dt_point(max_dt_idx, this->bimg->get_dims());
  cout<<"test : Centroid x "<<max_dt_point.x<<"Centroid y "<<max_dt_point.y<<"Centroid z "<<max_dt_point.z<<endl;
  this->soma = new Soma(max_dt_point, max_dt * 2);

  if(!this->silent) cout<<"Step Three: Somatic Mask Making Time Spent is ";
  start_time = clock();
  this->soma->make_mask(this->bimg);
  if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  // Make Speed Image dt**4 if bimg>0
  // if (!this->silent) cout << "(3/6) == Making Speed Image...";
  start_time = clock();
  Image3<double> *speed = this->makespeed(dt);
  if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;


  // Marching on the Speed Image
  int *sp = this->soma->centroid.toint().make_array();
  //cout<<"test: The initial centroid, "<<"x: "<<sp[0]<<" y: "<<sp[1]<<" z: "<<sp[2]<<endl;

  SWC *swc = new SWC();
  cout<<"test: scentre is running"<<endl;
  SWCNode soma_node(0, 1, max_dt_point, (int) max_dt, -1);
  swc->add_node(soma_node);
  // SWCNode(int id, int type, Point<float> p, int radius, int pid)


  // if (!this->silent){
  //   cout << "(4/6) == Multi-Stencils Fastmarching ";
  //   if (this->quality){
  //     cout << "with high quality...";
  //   }
  //   else{
  //     cout << "with low quality...";
  //   }
  //   cout.flush();   
  // }
  // start_time = clock();
  // double *t_ptr = msfm(speed->get_data1d_ptr(), dims, sp, this->quality, this->quality,
  //                                      false); // Original Timemap
  // if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  // this->t = new Image3<double>(t_ptr, this->bimg->get_dims());
  // this->tt = this->t->make_copy();

  // // Get the gradient of the Time-crossing map
  // if(!this->silent) cout <<  "(5/6) == Making Gradients of Time Crossing Map...";
  // start_time = clock();
  // this->make_gradient();
  // if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  if (dt) {
    delete dt;
    dt = NULL;
  }

  if (speed) {
    delete speed;
    speed = NULL;
  }

  if (sp) {
    delete[] sp;
    sp = NULL;
  }
  return swc;
}


void R2Tracer::prep() {
  // // Fast marching distance transform proposed in APP2
  if (!this->silent) cout << "Step Two : Boundary Distance Transform Time Spent is ";
  long *dims = this->bimg->get_dims();
  float *bdist1d = NULL;
  int start_time = -1;
  start_time = clock();
  fastmarching_dt(this->bimg->get_data1d_ptr(), bdist1d, dims[0], dims[1],
                  dims[2], 2, 0);
  if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  Image3<float> *dt = new Image3<float>(bdist1d, this->bimg->get_dims());

  // Find the source point
  long max_dt_idx = dt->max_idx_1d();
  long max_dt = dt->get_1d(max_dt_idx);
  cout<<"max_dt_idx: "<<(int) max_dt_idx<<endl;
  Point<float> max_dt_point(max_dt_idx, this->bimg->get_dims());
  cout<<"test : Centroid x "<<max_dt_point.x<<"Centroid y "<<max_dt_point.y<<"Centroid z "<<max_dt_point.z<<endl;
  this->soma = new Soma(max_dt_point, max_dt * 2);

  if(!this->silent) cout<<"Step Three: Somatic Mask Making Time Spent is ";
  start_time = clock();
  this->soma->make_mask(this->bimg);
  if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  // Make Speed Image dt**4 if bimg>0
  // if (!this->silent) cout << "(3/6) == Making Speed Image...";
  start_time = clock();
  Image3<double> *speed = this->makespeed(dt);
  if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;


  // Marching on the Speed Image
  int *sp = this->soma->centroid.toint().make_array();
  cout<<"test: The initial centroid, "<<"x: "<<sp[0]<<"y: "<<sp[1]<<"z: "<<sp[2]<<endl;


  // if (!this->silent){
  //   cout << "(4/6) == Multi-Stencils Fastmarching ";
  //   if (this->quality){
  //     cout << "with high quality...";
  //   }
  //   else{
  //     cout << "with low quality...";
  //   }
  //   cout.flush();   
  // }
  // start_time = clock();
  // double *t_ptr = msfm(speed->get_data1d_ptr(), dims, sp, this->quality, this->quality,
  //                                      false); // Original Timemap
  // if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  // this->t = new Image3<double>(t_ptr, this->bimg->get_dims());
  // this->tt = this->t->make_copy();

  // // Get the gradient of the Time-crossing map
  // if(!this->silent) cout <<  "(5/6) == Making Gradients of Time Crossing Map...";
  // start_time = clock();
  // this->make_gradient();
  // if(!this->silent) cout << (clock()-start_time) / double(CLOCKS_PER_SEC) <<"s"<<endl;

  if (dt) {
    delete dt;
    dt = NULL;
  }

  if (speed) {
    delete speed;
    speed = NULL;
  }

  if (sp) {
    delete[] sp;
    sp = NULL;
  }
}

/* Make the gradients for back-tracking*/
void R2Tracer::make_gradient() {
  V3DLONG *dims = this->t->get_dims();
  V3DLONG nvox = this->t->size();
  V3DLONG jdims[3];
  jdims[0] = dims[0] + 2;
  jdims[1] = dims[1] + 2;
  jdims[2] = dims[2] + 2;
  Image3<double> *jacobian = new Image3<double>(jdims);
  Image3<double> *fx = new Image3<double>(dims);
  Image3<double> *fy = new Image3<double>(dims);
  Image3<double> *fz = new Image3<double>(dims);
  std::fill(jacobian->get_data1d_ptr(),
            jacobian->get_data1d_ptr() + jacobian->size(), this->t->max());

  // Assign the center of J to T
  Point<V3DLONG> pt;
  Point<V3DLONG> pj;
  for(pt.x=0, pj.x=1; pt.x < dims[0]; pt.x++, pj.x++)
    for(pt.y=0, pj.y=1; pt.y < dims[1]; pt.y++, pj.y++)
      for(pt.z=0, pj.z=1; pt.z < dims[2]; pt.z++, pj.z++){
        jacobian->set(pj, this->t->get(pt));
      }

  // Make the neighbour position kernel
  long ne[26][3] = {{-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1}, {-1, 0, -1},
                    {-1, 0, 0},   {-1, 0, 1},  {-1, 1, -1}, {-1, 1, 0},
                    {-1, 1, 1},   {0, -1, -1}, {0, -1, 0},  {0, -1, 1},
                    {0, 0, -1},   {0, 0, 1},   {0, 1, -1},  {0, 1, 0},
                    {0, 1, 1},    {1, -1, -1}, {1, -1, 0},  {1, -1, 1},
                    {1, 0, -1},   {1, 0, 0},   {1, 0, 1},   {1, 1, -1},
                    {1, 1, 0},    {1, 1, 1}};

  // Make the norms for displacement vectors
  double ne_norm[26][3];
  for (int i = 0; i < 26; i++) {
    for (int j = 0; j < 3; j++) {
      ne_norm[i][j] = sqrt((double)(ne[i][0] * ne[i][0] + ne[i][1] * ne[i][1] +
                                    ne[i][2] * ne[i][2]));
    }
  }

  double jac = -1;
  for(pt.x=0 ; pt.x < dims[0]; pt.x++)
    for(pt.y=0; pt.y < dims[1]; pt.y++)
      for(pt.z=0; pt.z < dims[2]; pt.z++)
        for (int i = 0; i < 26; i++) {
          pj.x = pt.x + 1 + ne[i][0];
          pj.y = pt.y + 1 + ne[i][1];
          pj.z = pt.z + 1 + ne[i][2];
          jac = jacobian->get(pj);
          long tidx = pt.make_linear_idx(dims);

          if (jac < this->t->get_1d(tidx)) {
            this->t->set_1d(tidx, jac);
            fx->set_1d(tidx, -ne[i][0] / ne_norm[i][0]);
            fy->set_1d(tidx, -ne[i][1] / ne_norm[i][1]);
            fz->set_1d(tidx, -ne[i][2] / ne_norm[i][2]);
          }
    }

  // Change grad to 1D
  this->grad = new double[nvox * 3];
  std::copy(fx->get_data1d_ptr(), fx->get_data1d_ptr() + nvox, this->grad);
  std::copy(fy->get_data1d_ptr(), fy->get_data1d_ptr() + nvox,
            this->grad + nvox);
  std::copy(fz->get_data1d_ptr(), fz->get_data1d_ptr() + nvox,
            this->grad + nvox * 2);

  if (fx) {
    delete fx;
    fx = NULL;
  }
  if (fy) {
    delete fy;
    fy = NULL;
  }
  if (fz) {
    delete fz;
    fz = NULL;
  }
  if (jacobian) {
    delete jacobian;
    jacobian = NULL;
  }
}

Image3<double> *R2Tracer::makespeed(Image3<float> *dt) {
  double dmax = (double) dt->max();
  Image3<double> *speed = dt->to_double();
  for (int i = 0; i < dt->size(); i++) {
    double v = (double) dt->get_1d(i);
    double s = v == 0 ? 1e-10 : v / dmax;
    speed->set_1d(i, pow(s, 4));
  }
  return speed;
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

  this->coverage = (float)ctr / (float)this->bsum;

  // Show progress bar
  if (!this->silent) {
    show_progress(this->coverage, ctr, this->bsum);
  }
}

SWC *R2Tracer::iterative_backtrack() {
  cout << "(6/6) == Iterative backtrack..." << endl;
  SWC *swc = new SWC();
  srand(time(NULL));
  float eps = 1e-5;
  this->bb = new Image3<unsigned char>(this->bimg->get_dims());
  this->dilated_bimg = this->bimg->dilate(); // TODO: dilation

  // Count the number of foreground voxels
  V3DLONG nforeground = (V3DLONG) this->bimg->sum();

  // Mark background tt to -2, soma region to -3
  for (int i = 0; i < this->bimg->size(); i++) {
    if (this->bimg->get_1d(i) <= 0) {
      this->tt->set_1d(i, -2.0);
    } else if (this->soma->get_mask()->get_1d(i) > 0) {
      this->tt->set_1d(i, -3.0);
    }
  }

  SWCNode soma_node(0, 1, soma->centroid, soma->radius, -1);
  swc->add_node(soma_node);
  this->bsum = this->bimg->sum(); // The total number of foreground voxels

  // Iteration for different branches
  while (this->coverage < this->target_coverage) {
    // Find the geodesic furthest point on foreground time-crossing-map
    long maxtidx = this->tt->max_idx_1d();
    Point<float> srcpt(maxtidx, this->tt->get_dims());

    Branch branch;
    // Erase the source point just in case
    this->tt->set_1d(maxtidx, -2);
    branch.add(srcpt, 1.0); // Add the initial point
    this->update_coverage();

    bool keep = true;
    // Iteration for one branch
    while (true) {
      this->step(branch);
      Point<float> head = branch.get_head();

      // 1. Check ouf of bound
      if (!this->bimg->is_in_bound(head.tolong())) {
        branch.slice(0, branch.get_length() - 1); // Exclude last node
        break;
      }

      // 2. Check for the large gap criterion
      if (branch.get_gap() > branch.mean_radius() * 8) {
        break;
      } else {
        branch.reset_gap();
      }

      // 3. Check if soma has been reached
      if (this->tt->get(branch.get_head().tolong()) == -3) {
        keep = branch.get_curve_length() > 10 ? true : false;
        branch.reach_soma();
        break;
      }

      // 4. If it has not moved for 15 iterations, stop
      if (branch.is_stucked()) {
        // cout<<"Stucked!!"<<endl;
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
      if (this->tt->get(branch.get_head().tolong()) == -1) {
        branch.set_touched(true);

        // Stop exploring if there has not been any branch yet
        if (swc->size() == 1) {
          break;
        }

        SWCNode n(-1, 1, head, branch.get_radius_at(branch.get_length() - 1),
                  -1);
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

    // Add to the SWC if it was decided to be kept
    if (keep) {
      // Find the node ID to connect to
      int pid = -2;
      if (branch.is_reach_soma()) {
        pid = 0;
      } else if (branch.get_touch_idx() > 0) {
        pid = swc->get_node(branch.get_touch_idx()).id;
      }
      swc->add_branch(branch, pid);
    }
  }

  if(this->silent) cout << endl;

  return swc;
}

void R2Tracer::set_quality(bool quality){
  this->quality = quality;
}
