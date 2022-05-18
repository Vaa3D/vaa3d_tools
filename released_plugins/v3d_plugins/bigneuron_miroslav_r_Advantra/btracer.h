#ifndef BTRACER_H
#define BTRACER_H
#include <v3d_interface.h>
#include <vector>
#include <node.h>

class BTracer
{
public:

    int Niterations;
    int Nstates;

    static int Ndirs;
    static int Nsteps; // 2,3,4 fix
    static float ang;
    static float K; // likelihood exponential

//    static float bgratio;
//    static float ovlpratio;

    float step; // scale/Nsteps

    int N1; // total posteriot samples from 1 state
    int NN; // total posterior samples from N states

    int node_cnt; // # nodes in current trace [0, Niterations*Nsteps)

    static float outerring; // defines outer ring in tagging
    static float gcsstd2rad; // scaling between gaussian cross section standard deviation and the neurite radius
    static float gcsstd_step;
    static float gcsstd_min;
    float        gcsstd_max;

    bool is2D;
    float zDist;

    int     U, W, V, U2, W2, V2; // V = 2*V2+1 ~ [vx, vy, vz]

    float * gcsstd;
    int     gcsstd_nr;

    // template size U*W*V, W is added for 3d (it is 1 in 2d)
    // z components of each are scaled down

    float *     img_vals;   // U*W*V

    float       start_px; // each trace starts from oriented 3d point
    float       start_py;
    float       start_pz;
    float       start_vx;
    float       start_vy;
    float       start_vz;

    float **    tt;         // templates, U*W*V
    float *     tta;        // template averages

    // transition from 1 source state
    float **  xtt1;      // Nsteps x (Ndirs, Ndirs^2, Ndirs^3... Ndirs^Nsteps)
    float **  ytt1;
    float **  ztt1;
    float **  vxtt1;
    float **  vytt1;
    float **  vztt1;
    float **  rtt1;
    float **  priortt1;
    float **  lhoodtt1;
    float **  postrtt1;
    float*    postrtt1_aux;
    int*      postrtt1_idx;

    // transition from Nstates source states
    float **    xttN;     // Nsteps x (Nstates*Ndirs, Nstates*Ndirs^2, Nstates*Ndirs^3... Nstates*Ndirs^Nsteps)
    float **    yttN;
    float **    zttN;
    float **    vxttN;
    float **    vyttN;
    float **    vzttN;
    float **    rttN;
    float **    priorttN;
    float **    lhoodttN;
    float **    postrttN;
    float *     postrttN_aux;
    int *       postrttN_idx;

    int*        topIdxs;   // selection of Nstates

    // collection of states
    float **  xt;  // (Niterations*Nsteps) x Nstates
    float **  yt;  //
    float **  zt;  //
    float **  vxt; //
    float **  vyt; //
    float **  vzt; //
    float **  rt;  //
    float **  wt;  //

    // estimation
    float *   xc;  // Niterations*Nsteps
    float *   yc;  //
    float *   zc;  //
    float *   rc;  //

    // the values will be rotated according to current direction
    float *         vx_template;  // Nstates of such at initialization
    float *         vy_template;  //
    float *         vz_template;  //

    float **    rot;        // rotation matrix
    float       ux, uy, uz; // for orthogonals
    float       wx, wy, wz; //

    // offsets for tagging certain node
    // will define the offsets for particular scale defined with gcsstd
    // where each offset has a label on whether it belongs to inner or the outer sphere
    // spheres are bounded with radiuses that correspond to gcsstds
    std::vector< std::vector<int> > offx;
    std::vector< std::vector<int> > offy;
    std::vector< std::vector<int> > offz;
//    std::vector< std::vector<int> > offtag; // storage for tags allocated in advance

    BTracer(int _Niterations, int _Nstates, int _scale, bool _is2D, float _zDist);

    ~BTracer();

    void calculate_dirs(float * _vx, float * _vy, float * _vz, bool is2D, int nr_dirs);

   void rotation_matrix(float a1, float a2, float a3, float b1, float b2, float b3, float** R);

   void rotation_apply(float** R, float v1, float v2, float v3, float &out1, float &out2, float &out3);

   float interp(float atX, float atY, float atZ, unsigned char * img, int width, int height, int length, bool dbg);

   int get_gcsstd_idx(float gcsstd_val);

   void extract(std::vector<int> ttags, std::vector<int> ntags, std::vector<int> & nlist);

   bool traceoverlap(
           float x,  float y,  float z,
           float             r,
           int               img_width,
           int               img_height,
           int               img_length,
           int *             trace_map,
           int *             node_map,
           std::vector<Node> &    node_list
           );

//   bool tracetagfound(
//           int               tt,
//           float x,  float y,  float z,
//           float             r,
//           int               img_width,
//           int               img_height,
//           int               img_length,
//           int *             trace_map
//           );

   void traceX(
           bool              first,
           float x,  float y,  float z,
           float vx, float vy, float vz,
           float             r,
           unsigned char *   img,
           int               img_width,
           int               img_height,
           int               img_length,
           float             angstd_deg,
           int *             tag_map,
           int *             trace_map,
           std::vector<Node> &    node_list, // indexes of the node_list will fill tag_map and trace_map
           float             bgratio
           );

   void iter_1( float                   xloc,
                float                   yloc,
                float                   zloc,
                float                   vxloc,
                float                   vyloc,
                float                   vzloc,
                float                   rloc,
                unsigned char *         img,
                int                     img_width,
                int                     img_height,
                int                     img_length,
                float                   angula_diff_std_deg,
//                float                   gcsstd_diff_std_pix,
                float vx_prior,
                float vy_prior,
                float vz_prior,
                bool                    dbg);

   void iter_N( float *                  xlocs,
                float *                  ylocs,
                float *                  zlocs,
                float *                  vxlocs,
                float *                  vylocs,
                float *                  vzlocs,
                float *                  rlocs,
                float *                  wlocs,
                unsigned char *         img,
                int                     img_width,
                int                     img_height,
                int                     img_length,
                float                   angula_diff_std_deg,
//                float                   gcsstd_diff_std_pix,
                float vx_prior,
                float vy_prior,
                float vz_prior,
                bool                    dbg);

   void predict(   float px, float py, float pz,
                   float vx, float vy, float vz,
                   float r_pv,
                   float vx_prior,
                   float vy_prior,
                   float vz_prior,
                   float angula_diff_std_deg, //float gcsstd_diff_std_pix,
                   float * px_out, float * py_out, float * pz_out,
                   float * vx_out, float * vy_out, float * vz_out,
                   float * r_out,
                   float * prior_out);

   // different kinds of lkelihood measurements
   float likelihood1(float x, float y, float z,
                     unsigned char * img,
                     int img_width,
                     int img_height,
                     int img_length,
                     float & r) {
       r = 1;
       return 1.0+interp(x, y, z, img, img_width, img_height, img_length, false); // boolean was for debug
   }


//   float zncc(float x, float y, float z,
//                     float vx, float vy, float vz,
//                     unsigned char * img, int img_w, int img_h, int img_l,
//                     float & r, bool dbg
//           );

   float znccX( float x, float y, float z,
                        float vx, float vy, float vz,
                        unsigned char * img, int img_w, int img_h, int img_l,
                        int & gcsstd_idx
                       );

   void save_templates(V3DPluginCallback2 &callback, QString inimg_path);

};

#endif // BTRACER_H
