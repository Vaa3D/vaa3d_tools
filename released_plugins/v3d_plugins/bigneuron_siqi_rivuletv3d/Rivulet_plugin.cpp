/* Rivulet_plugin.cpp
 * Rivulet algorithm for 3D neuron tracing.
 * 2015-8-25 : by Siqi Liu, Donghao Zhang
 */

#include "Rivulet_plugin.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"
#include "stackutil.h"
#include "v3d_message.h"
#include "vn_rivulet.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include "rivulet.h"

Q_EXPORT_PLUGIN2(Rivulet, RivuletPlugin);

using namespace rivulet;

struct input_PARA {
  QString inimg_file;
  V3DLONG channel;
  unsigned char threshold;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent,
                         input_PARA &PARA, bool bmenu);
// vector<swcnode> iterative_backtrack(V3DLONG *in_sz, unsigned char *data1d,
//                                     double *t, Soma soma, int threshold,
                                    // int length, float eraseratio);
float *get_boundary_distance(unsigned char *);
template <class T> unsigned char *charimg(T *speedimg, int NVOX);
template <class T> V3DLONG findmax(T *I, int NVOX, T *max);
double *float2double(float *f, V3DLONG NVOX);
double *distgradient(double *T, V3DLONG *in_sz);
double *c2mat(double *m, V3DLONG *in_sz);
// void binarysphere3d(vector<bool> tB, V3DLONG *in_sz, vector<Point> l,
                    // vector<int> radius);
// void add2swc(std::vector<swcnode> swc, std::vector<Point<>> branch, std::vector<int> rlist,
             // int connectid, bool random_color);
// V3DLONG findmindist(Point p, vector<swcnode> tree, double *m);
float exponential_moving_average(float p, float ema, int n);

QStringList RivuletPlugin::menulist() const {
  return QStringList() << tr("tracing") << tr("about");
}

QStringList RivuletPlugin::funclist() const {
  return QStringList() << tr("tracing_func") << tr("help");
}

void RivuletPlugin::domenu(const QString &menu_name,
                           V3DPluginCallback2 &callback, QWidget *parent) {
  if (menu_name == tr("tracing")) {
    bool bmenu = true;
    input_PARA PARA;

    if (callback.getImageWindowList().empty()) {
      v3d_msg("Oops... No image opened in V3D...");
      return;
    }
    PARA_RIVULET p;
    // fetch parameters from dialog
    if (!p.rivulet_dialog())
      return;
    PARA.threshold = p.threshold;

    reconstruction_func(callback, parent, PARA, bmenu);

  } else {
    v3d_msg(tr("Rivulet algorithm for 3D neuron tracing. . "
               "Developed by Siqi Liu, Donghao Zhang, 2015-8-25"));
  }
}

bool RivuletPlugin::dofunc(const QString &func_name,
                           const V3DPluginArgList &input,
                           V3DPluginArgList &output,
                           V3DPluginCallback2 &callback, QWidget *parent) {
  if (func_name == tr("tracing_func")) {
    bool bmenu = false;
    input_PARA PARA;

    vector<char *> *pinfiles =
        (input.size() >= 1) ? (vector<char *> *)input[0].p : 0;
    vector<char *> *pparas =
        (input.size() >= 2) ? (vector<char *> *)input[1].p : 0;
    vector<char *> infiles = (pinfiles != 0) ? *pinfiles : vector<char *>();
    vector<char *> paras = (pparas != 0) ? *pparas : vector<char *>();

    if (infiles.empty()) {
      fprintf(stderr, "Need input image. \n");
      return false;
    } else
      PARA.inimg_file = infiles[0];
    int k = 0;
    PARA.channel = (paras.size() >= k + 1) ? atoi(paras[k]) : 1;
    k++;
    PARA.threshold = (paras.size() >= k + 1) ? atoi(paras[k]) : 0;
    k++;
    reconstruction_func(callback, parent, PARA, bmenu);
  } else if (func_name == tr("help")) {
    printf("**** Usage of Rivulet tracing **** \n");
    printf(
        "vaa3d -x Rivulet -f tracing_func -i <inimg_file> [-o outimg_file] -p "
        "<channel> <threshold>\n");
    printf("inimg_file\tThe input image\n");
    printf("channel\tData channel for tracing. Start from 1 (default 1).\n");
    printf(
        "threshold\tThe background threshold for segmentation (default 10).\n");
  } else {
    return false;
  }

  return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent,
                         input_PARA &PARA, bool bmenu) {
  unsigned char *data1d = 0;
  V3DLONG N, M, P, sc, c;
  V3DLONG in_sz[4];
  if (bmenu) {
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin) {
      QMessageBox::information(
          0, "", "You don't have any image open in the main window.");
      return;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);

    if (!p4DImage) {
      QMessageBox::information(0, "",
                               "The image pointer is invalid. Ensure your data "
                               "is valid and try again!");
      return;
    }

    data1d = p4DImage->getRawData();
    N = p4DImage->getXDim();
    M = p4DImage->getYDim();
    P = p4DImage->getZDim();
    sc = p4DImage->getCDim();

    bool ok1;

    if (sc == 1) {
      c = 1;
      ok1 = true;
    } else {
      c = QInputDialog::getInteger(parent, "Channel", "Enter channel NO:", 1, 1,
                                   sc, 1, &ok1);
    }

    if (!ok1)
      return;

    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = sc;

    PARA.inimg_file = p4DImage->getFileName();
  } else {
    int datatype = 0;
    if (!simple_loadimage_wrapper(callback,
                                  PARA.inimg_file.toStdString().c_str(), data1d,
                                  in_sz, datatype)) {
      fprintf(stderr,
              "Error happens in reading the subject file [%s]. Exit. \n",
              PARA.inimg_file.toStdString().c_str());
      return;
    }
    if (PARA.channel < 1 || PARA.channel > in_sz[3]) {
      fprintf(stderr, "Invalid channel number. \n");
      return;
    }
    N = in_sz[0];
    M = in_sz[1];
    P = in_sz[2];
    sc = in_sz[3];
    c = PARA.channel;
  }

  // Main neuron reconstruction code
  cout << "Start Rivulet" << endl;
  printf("Image Size: %d, %d, %d\n", in_sz[0], in_sz[1], in_sz[2]);
  long* in_sz3 = new V3DLONG[3];
  in_sz3[0] = in_sz[0]; in_sz3[1] = in_sz[1];  in_sz3[2] = in_sz[2]; 
  Image3<unsigned char>* img = new Image3<unsigned char>(data1d, in_sz3);// Make the Image
  img->set_destroy(false);

  // The meaty part
  R2Tracer* tracer = new R2Tracer();
  tracer->trace(img, PARA.threshold);

  if (in_sz3){
    delete [] in_sz3;
    in_sz3 = NULL;
  }

  if (img){
    delete img;
    img = NULL;
  }

  if (tracer){
    delete tracer;
    tracer = NULL;
  }

  return;
}

// vector<swcnode> iterative_backtrack(V3DLONG *in_sz, unsigned char *data1d,
//                                     double *t, Soma soma, int threshold,
//                                     int length = 5, float eraseratio = 1.2) {
//   srand (time(NULL));
//   V3DLONG NVOX = in_sz[0] * in_sz[1] * in_sz[2];
//   float TARGET_COVERAGE = 0.98;
//   float eps = 1e-5;

//   // Make a boolean image
//   bool *bimg = new bool[NVOX];
//   for (int i = 0; i < NVOX; i++) {
//     bimg[i] = data1d[i] > threshold;
//   }

//   // For making a large tube to contain the last traced branch
//   vector<bool> bb(NVOX);

//   // Get the gradient of the Time-crossing map
//   double *grad = distgradient(t, in_sz);

//   // Copy of T for labelling traced branches
//   double *tt = new double[NVOX];
//   copy(t, t + NVOX, tt);

//   // Mark background tt to -2, soma region to -3
//   // Count the number of foreground voxels
//   V3DLONG nforeground = 0;
//   for (int i = 0; i < NVOX; i++) {
//     nforeground += (bimg[i]) ? 1 : 0;
//     if (bimg[i] == false) {
//       tt[i] = -2;
//     } else if (soma.mask[i] == true) {
//       tt[i] = -3;
//     }
//   }

//   // TODO: dilate bimg to make dilated_bimg
//   bool *dilated_bimg = bimg;
//   float coverage = 0.0;

//   // Initialise swc with soma node
//   vector<swcnode> swc;
//   swcnode node;
//   node.id = 0;
//   node.type = 1;
//   node.p = soma.centroid;
//   node.radius = soma.radius;
//   node.parent = -1;
//   swc.push_back(node);

//   double velocity[3];
//   double branchlen = 0.0;
//   V3DLONG coveredctr_old = 0;
//   float tmax = 0.0;
//   V3DLONG tmaxidx = 0;
//   Point srcpt, endpt;

//   while (coverage < TARGET_COVERAGE) {
//     V3DLONG coveredctr_new = 0;
//     for (int i = 0; i < NVOX; i++) {
//       if (tt[i] < 0 && bimg[i] == true) {
//         coveredctr_new++;
//       }

//       // Find the geodesic furthest point on foreground time-crossing-map
//       if (tmax < tt[i]) {
//         tmax = tt[i];
//         tmaxidx = i;
//       }
//     }

//     // Get the new coverage percentile
//     coverage = coveredctr_new / nforeground;
//     coveredctr_old = coveredctr_new;

//     // Get the source point for a new tracing iteration
//     srcpt = endpt = ind2sub(tmaxidx, in_sz);

//     // Vars for controlling tracing iteration
//     bool reached = false, touched = false, reachedsoma = false,
//          low_online_conf = false;
//     V3DLONG touchidx = -2;
//     int steps_after_reach = 0, gapdist = 0;
//     vector<Point> branch;      // vector for branch points
//     vector<float> branch_conf; // vector for online confidence
//     vector<int> rlist;       // vector for radii
//     branch.push_back(srcpt);
//     branch_conf.push_back(1.0);
//     V3DLONG fgctr =
//         0; // Count how many steps are made on foreground in this branch

//     // Vars for online confidence computing
//     int online_voxsum = 0;
//     float ma_short = -1, ma_long = -1;
//     int ma_short_window = 4, ma_long_window = 10;
//     bool in_valley = false;

//     // Start 1 back-tracking iteration
//     while (true) {
//       Point endpoint = rk4(srcpt, grad, in_sz, 1);
//       MyMarker marker;
//       marker.x = endpt.x;
//       marker.y = endpt.y;
//       marker.z = endpt.z;
//       int endradius = markerRadius(data1d, in_sz, marker, threshold, 2);
//       velocity[0] = (double)(endpt.x - srcpt.x);
//       velocity[1] = (double)(endpt.y - srcpt.y);
//       velocity[2] = (double)(endpt.z - srcpt.z);
//       double stepsz = norm3(velocity);
//       branchlen += stepsz;

//       // Count the number of steps it travels on the foreground
//       V3DLONG endptind =
//           sub2ind(floor(endpt.x), floor(endpt.y), floor(endpt.z), in_sz);
//       bool endpt_b = dilated_bimg[endptind];
//       fgctr += endpt_b;

//       // Check for the large gap criterion
//       if (endpt_b == false) {
//         // Get the mean radius so far
//         double rmean =
//             accumulate(rlist.begin(), rlist.end(), 0.0) / rlist.size();
//         gapdist += stepsz;
//         if (gapdist > rmean * 8)
//           break;
//       } else {
//         gapdist = 0; // Reset gapdist
//       }

//       // Compute the online confidence
//       online_voxsum += endpt_b;
//       // The online confidence (OC) starts from 0.5 due to the +1 term,
//       // It is helpful to make the OC score from noise points decay fast
//       float online_confidence = online_voxsum / (branch.size() + 1);

//       // Reached soma
//       if (tt[endptind] == -3) {
//         // Get the branch length
//         if (branchlen < 15) {
//           low_online_conf = true;
//         }
//       }

//       // Compute the two MA curves of OC
//       if (branch.size() > ma_long_window) {
//         if (ma_short == -1) {
//           ma_short = online_confidence;
//         } else {
//           int N = (branch.size() >= ma_short_window) ? ma_short_window
//                                                      : branch.size();
//           ma_short = exponential_moving_average(online_confidence, ma_short, N);
//         }

//         if (ma_long == -1) {
//           ma_long = online_confidence;
//         } else {
//           int N = (branch.size() >= ma_long_window) ? ma_long_window
//                                                     : branch.size();
//           ma_long = exponential_moving_average(online_confidence, ma_long, N);
//         }

//         // We are stepping in a valley
//         if (ma_short < ma_long - eps && online_confidence < 0.5 && !in_valley) {
//           in_valley = true;
//         }

//         // Cut at the valley
//         if (in_valley && ma_short > ma_long) {
//           int valleyidx = std::distance(
//               branch_conf.begin(),
//               std::min_element(branch_conf.begin(), branch_conf.end()));
//           // Only cut if the valley confidence is below 0.5
//           if (branch_conf[valleyidx] < 0.5) {
//             vector<Point> sliced_branch(branch.begin(),
//                                         branch.begin() + valleyidx);
//             branch = sliced_branch;
//             vector<float> sliced_conf(branch_conf.begin(),
//                                       branch_conf.begin() + valleyidx);
//             branch_conf = sliced_conf;
//             low_online_conf = true;
//             break;
//           } else {
//             in_valley = false;
//           }
//         }
//       }

//       // Consider reaches previous explored area traced with branch
//       // Note: when the area was traced due to noise points
//       // (erased with -2), not considered as 'reached'
//       if (tt[endptind] == -1) {
//         reached = true;
//         if (swc.size() == 1) {
//           break; // # There has not been any branch added yet
//         }

//         steps_after_reach++;
//         double dist = -1;
//         touchidx = findmindist(endpt, swc, &dist);
//         if (touchidx != -1) {
//           touched = false;
//         }

//         if (touched || steps_after_reach >= 200) {
//           break;
//         }
//       }

//       // If the velocity is too small, sprint a bit with the momentum
//       if (stepsz <= 0.5 && branch.size() >= 5) {
//         endpt.x = srcpt.x + (branch.back().x - branch[branch.size() - 4].x);
//         endpt.y = srcpt.y + (branch.back().y - branch[branch.size() - 4].y);
//         endpt.z = srcpt.z + (branch.back().z - branch[branch.size() - 4].z);
//       }

//       if (branch.size() > 15 && pointdist(branch[branch.size() - 15], endpt)) {
//         break;
//       }

//       if (online_confidence <= 0.2) {
//         low_online_conf = true;
//         break;
//       }

//       // All in vain finally if it traces out of bound
//       if (!inbound(endpt, in_sz)) {
//         break;
//       }

//       // Add newly traced node to current branch
//       branch.push_back(endpt);
//       rlist.push_back(endradius);
//       branch_conf.push_back(online_confidence);
//       srcpt = endpt; // Shift forward
//     }

//     // TODO: forward conf

//     // Erase it from the timemap
//     vector<int> r_large;
//     for (int i = 0; i < branch.size(); i++) {
//       r_large.push_back(ceil(rlist[i] * eraseratio + 1));
//     }

//     binarysphere3d(bb, in_sz, branch, r_large);
//     V3DLONG startidx = sub2ind(branch[0].x, branch[1].y, branch[2].z, in_sz);
//     V3DLONG endidx =
//         sub2ind(branch.back().x, branch.back().y, branch.back().z, in_sz);
//     double endt = tt[endidx];
//     double startt = tt[startidx];

//     if (branch.size() > length && endt < startt) {
//       // The erase region is the region between end_t<=tt<=start_t
//       for (int i = 0; i < NVOX; i++) {
//         if (tt[i] >= endt && tt[i] <= startt && bb[i] == true) {
//           tt[i] = low_online_conf ? -2 : - 1;
//         }
//       }
//     }

//     fill(bb.begin(), bb.end(), 0);

//     int connectid = -2;
//     if (touched) {
//       connectid = swc[touchidx].id;
//     } else if (reachedsoma) {
//       connectid = 0;
//     }

//     // Check the confidence of this branch
//     if (low_online_conf) {
//       continue;
//     }

//     add2swc(swc, branch, rlist, connectid, true);
//   }

//   return swc;
// }

// bool inbound(Point pt, V3DLONG *in_sz) {
//   if (pt.x >= 0 && pt.x <= in_sz[0] && pt.y >= 0 && pt.y <= in_sz[1] &&
//       pt.z >= 0 && pt.z <= in_sz[2]) {
//     return true;
//   } else {
//     return false;
//   }
// }

// float exponential_moving_average(float p, float ema, int n) {
//   /*

//     The exponential moving average (EMA) traditionally
//     used in analysing stock market.
//     EMA_{i+1} = (p * \alpha) + (EMA_{i} * (1 - \alpha))
//     where p is the new value; EMA_{i} is the last ema value;
//     n is the time period; \alpha=2/(1+n) is the smoothing factor.

//     ---------------------------------------------
//     Parameters:
//     p: The new value in the sequence
//     ema: the last EMA value
//     n: The period window size
// */
//   float alpha = 2.0 / (float)(1.0 + n);
//   return p * alpha + ema * (1 - alpha);
// }

// template <class T> unsigned char *charimg(T *I, int NVOX) {
//   unsigned char *data1d_uchar;
//   try {
//     data1d_uchar = new unsigned char[NVOX];
//   } catch (...) {
//     v3d_msg("cannot allocate memory for data1d_uchar.");
//     return NULL;
//   }

//   // Find max
//   T max = 0;
//   findmax(I, NVOX, &max);

//   // Rescale to 0-255
//   for (int i = 0; i < NVOX; i++) {
//     data1d_uchar[i] = (unsigned char)((I[i] / max) * 255);
//   }

//   return data1d_uchar;
// }

// template <class T> V3DLONG findmax(T *I, int NVOX, T *max) {
//   *max = 0;
//   V3DLONG maxidx = 0;
//   for (int i = 0; i < NVOX; i++) {
//     if (*max < I[i]) {
//       *max = I[i];
//       maxidx = i;
//     }
//   }
//   return maxidx;
// }

// Point ind2sub(V3DLONG idx, V3DLONG *in_sz) {
//   Point p;
//   float xysz = (float)in_sz[0] * (float)in_sz[1];
//   p.z = (float)(floor((float)idx / (float)xysz));
//   p.y = idx - p.z * xysz;
//   p.y = (float)floor((float)p.y / (float)in_sz[0]);
//   p.x = (float)(floor(idx - p.z * xysz - p.y * (float)in_sz[0]));
//   return p;
// }

// V3DLONG sub2ind(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG *in_sz) {
//   V3DLONG xysz = (V3DLONG)(in_sz[0] * in_sz[1]);
//   return (V3DLONG)z * xysz + y * in_sz[0] + x;
// }

// double *float2double(float *f, V3DLONG NVOX) {
//   double *d = 0;
//   try {
//     d = new double[NVOX];
//   } catch (...) {
//     v3d_msg("cannot allocate memory for double image.");
//     return NULL;
//   }

//   for (int i = 0; i < NVOX; i++) {
//     d[i] = (double)f[i];
//   }
//   return d;
// }

// // Change 1d volume storage to column wise (originally row wise)
// double *c2mat(double *m, V3DLONG *in_sz) {
//   V3DLONG NVOX = (V3DLONG)((float)in_sz[0] * (float)in_sz[1] * (float)in_sz[2]);
//   V3DLONG newsz[3];
//   newsz[0] = in_sz[1];
//   newsz[1] = in_sz[0];
//   newsz[2] = in_sz[2];
//   double *newmat = new double[NVOX];
//   for (int x = 0; x < in_sz[0]; x++)
//     for (int y = 0; y < in_sz[1]; y++)
//       for (int z = 0; z < in_sz[2]; z++) {
//         int newloc = sub2ind(y, x, z, in_sz);
//         int oldloc = sub2ind(x, y, z, newsz);
//         newmat[newloc] = m[oldloc];
//       }
//   return newmat;
// }

// double *distgradient(double *T, V3DLONG *in_sz) {
//   V3DLONG NVOX = in_sz[0] * in_sz[1] * in_sz[2];
//   V3DLONG jin_sz[3]; // matrix for jacobian
//   jin_sz[0] = in_sz[0] + 2;
//   jin_sz[1] = in_sz[1] + 2;
//   jin_sz[2] = in_sz[2] + 2;
//   int jsz = jin_sz[0] * jin_sz[1] * jin_sz[2];
//   double *Fx = new double[NVOX];
//   std::fill(Fx, Fx + NVOX, 0.0);
//   double *Fy = new double[NVOX];
//   std::fill(Fy, Fy + NVOX, 0.0);
//   double *Fz = new double[NVOX];
//   std::fill(Fz, Fz + NVOX, 0.0);
//   double *J;
//   try {
//     J = new double[jsz];
//   } catch (...) {
//     v3d_msg("Cannot allocate memory for J.");
//     return NULL;
//   }

//   double maxt = 0;
//   findmax(T, NVOX, &maxt);
//   std::fill(J, J + jsz, maxt);

//   // Assign the center of J to T
//   for (int x = 0; x < in_sz[0]; x++)
//     for (int y = 0; y < in_sz[1]; y++)
//       for (int z = 0; z < in_sz[2]; z++) {
//         int jloc = sub2ind(x + 1, y + 1, z + 1, jin_sz);
//         int tloc = sub2ind(x, y, z, in_sz);
//         J[jloc] = T[tloc];
//       }

//   // Make the neighbour position kernel
//   short *Ne[27];
//   int ctr = 0;
//   for (int i = -1; i <= 1; i++)
//     for (int j = -1; j <= 1; j++)
//       for (int k = -1; k <= 1; k++) {
//         Ne[ctr] = new short[3];
//         Ne[ctr][0] = i;
//         Ne[ctr][1] = j;
//         Ne[ctr][2] = k;
//         // printf("Ne %d, %d, %d\n", i, j, k);
//         ctr++;
//       }

//   for (int i = 0; i < 27; i++)
//     for (int x = 0; x < in_sz[0]; x++)
//       for (int y = 0; y < in_sz[1]; y++)
//         for (int z = 0; z < in_sz[2]; z++) {
//           int jloc = sub2ind(1 + x + Ne[i][0], 1 + y + Ne[i][1],
//                              1 + z + Ne[i][2], jin_sz);
//           int tloc = sub2ind(x, y, z, in_sz);
//           double in = J[jloc];
//           if (in < T[tloc]) {
//             T[tloc] = in;
//             float powsum =
//                 Ne[i][0] * Ne[i][0] + Ne[i][1] * Ne[i][1] + Ne[i][2] * Ne[i][2];
//             Fx[tloc] = -Ne[i][0] / sqrt(powsum);
//             Fy[tloc] = -Ne[i][1] / sqrt(powsum);
//             Fz[tloc] = -Ne[i][2] / sqrt(powsum);
//           }
//         }

//   // Change to grad to 1d
//   double *grad4d = new double[NVOX * 3];
//   std::copy(Fx, Fx + NVOX, grad4d);
//   std::copy(Fy, Fy + NVOX, grad4d + NVOX);
//   std::copy(Fz, Fz + NVOX, grad4d + NVOX + NVOX);
//   if (Fx) {
//     delete[] Fx;
//     Fx = 0;
//   }
//   if (Fy) {
//     delete[] Fy;
//     Fy = 0;
//   }
//   if (Fz) {
//     delete[] Fz;
//     Fz = 0;
//   }
//   if (J) {
//     delete[] J;
//     J = 0;
//   }

//   return grad4d;
// }

// float pointdist(Point p1, Point p2) {
//   Point diff = p1 - p2;
//   float d = sqrt(pow((diff.x), 2) + pow((diff.y), 2) + pow((diff.z), 2));
//   return d;
// }

// float constrain(float x, float low, float high) {
//   if (x < low)
//     return low;
//   if (x > high)
//     return high;
//   return x;
// }

// vector<Point> neighbours3d(float x, float y, float z, float radius) {
//   vector<Point> neighbours;
//   // Return the coordinates of neighbours within a radius
//   for (float xgv = x - radius; xgv <= x + radius; xgv++)
//     for (float ygv = y - radius; ygv <= y + radius; ygv++)
//       for (float zgv = z - radius; zgv <= z + radius; zgv++) {
//         Point p;
//         p.x = xgv;
//         p.y = ygv;
//         p.z = zgv;
//         neighbours.push_back(p);
//       }
//   return neighbours;
// }

// void binarysphere3d(vector<bool> tB, V3DLONG *in_sz, vector<Point> l,
//                     vector<int> radius) {
//   for (int i = 0; i < l.size(); i++) {
//     vector<Point> neighbours = neighbours3d(l[i].x, l[i].y, l[i].z, radius[i]);
//     for (int n = 0; n < neighbours.size(); n++) {
//       float x = constrain(neighbours[n].x, 0.0, in_sz[0] - 1);
//       float y = constrain(neighbours[n].y, 0.0, in_sz[1] - 1);
//       float z = constrain(neighbours[n].z, 0.0, in_sz[2] - 1);
//       // printf("neigh x:%f.2, y:%f.2, z:%f.2\n", x, y, z);
//       V3DLONG ind = sub2ind((V3DLONG)x, (V3DLONG)y, (V3DLONG)z, in_sz);
//       tB[ind] = true;
//     }
//   }
// }

// V3DLONG findmindist(Point p, vector<swcnode> tree, double *m) {
//   *m = 1e10;
//   V3DLONG idx = -1;
//   for (int i = 0; i < tree.size(); i++) {
//     double dist = sqrt(pow(p.x - tree[i].p.x, 2) + pow(p.y - tree[i].p.y, 2) +
//                        pow(p.z - tree[i].p.z, 2));
//     if (*m > dist) {
//       *m = dist;
//       idx = i;
//     }
//   }

//   return idx;
// }

/*
Add a branch to swc
*/ 
// void add2swc(vector<swcnode> &swc, vector<Point> branch, vector<int> rlist,
//              int connectid, bool random_color=true) {

  // if(random_color)
  // {
  //   /* generate secret number between 1 and 10: */
  //   int rand_node_type = rand() % 256 + 0;
  // }

  // vector<swcnode> newbranch(branch.size());
  // long idstart;
  // if(swc.size() == 1) // First branch to add
  // {
  //   idstart = 1;
  // }
  // else{
  //   idstart = swc.back().id + 1;
  // }

  // int pid = -2;
  // for(int i=0; i<branch.size();i++)
  // {
  //   if(i == swc.size() - 1) 
  //   {
  //     pid = connectid > -2 ? connectid : -2;
  //   }
  //   else{
  //     pid = idstart + i + 1;
  //   }

  //   newbranch[i].id = idstart + i;
  //   newbranch[i].nodetype = rand_node_type;
  //   newbranch[i].p.x = branch[i].x;
  //   newbranch[i].p.y = branch[i].y;
  //   newbranch[i].p.z = branch[i].z;
  //   newbranch[i].radius = rlist[i];
  //   newbranch[i].parent = pid;
  // }

  // // Check if any tail should be connected to its head
  // head = newbranch[0];
  // minidx = findmindist(head.p, swc, double *m);


// }

// float addbranch2tree(vector<swcnode> *tree, Path l, unsigned char connectrate,
//                      vector<float> radius, unsigned char *data1d,
//                      V3DLONG *in_sz, unsigned char length) {
//   float confidence = 0;
//   int pathlen = l.l.size();
//   if (pathlen < length) {
//     // cout<<"Dump a branch short length:"<<pathlen<<endl;
//     return confidence;
//   }
//   int NVOX = (int)((float)in_sz[0] * (float)in_sz[1] * (float)in_sz[2]);
//   int vsum = 0;

//   // Compute the confidence
//   for (int i = 0; i < pathlen; i++) {
//     Point p = l.l[i];
//     V3DLONG ind = sub2ind((V3DLONG)floor(p.x), (V3DLONG)floor(p.y),
//                           (V3DLONG)floor(p.z), in_sz);
//     if (data1d[ind] > 0) {
//       vsum++;
//     }
//   }

//   confidence = (float)vsum / (float)pathlen;
//   if (confidence < 0.5) {
//     // cout<<"Dump with confidence: "<<confidence<<" length:
//     // "<<l.l.size()<<endl;
//     return confidence;
//   }

//   assert(pathlen == radius.size());

//   if (tree->size() == 0) {
//     for (int i = 0; i < pathlen; i++) {
//       swcnode node;
//       node.id = i + 1;
//       node.type = 2;
//       node.p.x = l.l[i].x;
//       node.p.y = l.l[i].y;
//       node.p.z = l.l[i].z;
//       node.radius = (int)radius[i];
//       node.parent = i + 2;
//       (*tree).push_back(node);
//     }
//     (*tree).back().parent = -1; // Root node
//   } else {
//     vector<swcnode> newtree(pathlen);
//     Point termini1 = l.l[pathlen - 1];
//     Point termini2 = l.l[0];
//     // Get pairwise distance between the termini and tree nodes
//     double mind1, mind2;
//     V3DLONG idx1 = findmindist(termini1, *tree, &mind1);
//     V3DLONG idx2 = findmindist(termini2, *tree, &mind2);

//     // Sort internal relationship
//     for (int i = 0; i < pathlen; i++) {
//       newtree[i].id = tree->back().id + i + 1;
//       newtree[i].type = 2;
//       newtree[i].p.x = l.l[i].x;
//       newtree[i].p.y = l.l[i].y;
//       newtree[i].p.z = l.l[i].z;
//       newtree[i].radius = radius[i];
//       if (i != pathlen - 1)
//         newtree[i].parent = tree->back().id + i + 2;
//       else
//         newtree[i].parent = -2;
//     }

//     if (mind1 < ((*tree)[idx1].radius + 3) * connectrate ||
//         mind1 < (newtree.back().radius + 3) * connectrate && l.merged) {
//       newtree.back().parent = (*tree)[idx1].id; // Connect to the tree parent
//     } else {
//       newtree.back().parent = -2; // Remain unconnected!!
//     }

//     if (mind2 < ((*tree)[idx2].radius + 3) * connectrate ||
//         mind2 < (newtree.back().radius + 3) * connectrate && l.merged) {
//       // newtree[0].parent = (*tree)[idx2].id; // Connect to the tree parent
//       (*tree)[idx2].parent = newtree[0].id; // Connect to the tree parent
//     } else {
//       newtree[0].parent = -2; // Remain unconnected!!
//     }

//     vector<swcnode> backuptree = *tree;
//     tree->clear();
//     tree->reserve(
//         backuptree.size() +
//         newtree.size()); // Enlarge the tree vector to hold the new branch
//     tree->insert(tree->end(), backuptree.begin(), backuptree.end());
//     tree->insert(tree->end(), newtree.begin(), newtree.end());
//   }

//   return confidence;
// }
