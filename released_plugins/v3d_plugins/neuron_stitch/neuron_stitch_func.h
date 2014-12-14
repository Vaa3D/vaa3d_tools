/*
 * 2014.10.07 by: Hanbo Chen cojoc(at)hotmail.com
*/

#ifndef NEURON_STITCH_FUNC_H
#define NEURON_STITCH_FUNC_H

#include <basic_surf_objs.h>
#include <basic_landmark.h>

#define NTDIS(a,b) (((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define NTNORM(a) (sqrt((a).x*(a).x+(a).y*(a).y+(a).z*(a).z))

typedef QVector<QVector<V3DLONG> > HBNeuronGraph;

struct Clique3{
    int v[3]; //vertex, v[0]~v[1]=e[0], v[1]~v[2]=e[1]
    double e[3]; //e[0]<=e[1]<=e[2]
    int idx[3]; //the id of vertices in XYZList
    XYZ dir[3]; //take clique center as origin, the center-point axis as x axis, the direction of the neruon
};

void constructNeuronGraph(const NeuronTree& nt, HBNeuronGraph& ng);
V3DLONG nextPointNeuronGraph(const HBNeuronGraph& ng, V3DLONG current, V3DLONG previous=-1);

void stitchMatchedPoint(NeuronTree* nt0, NeuronTree* nt1, const QList<int>& parent0, const QList<int>& parent1, int pid0, int pid1);
void stitchMatchedPoint(NeuronTree* nt0, NeuronTree* nt1, const HBNeuronGraph & ng0, const HBNeuronGraph & ng1, const QList<int> & neuronType0, const QList<int> & neuronType1, int pid0, int pid1);

bool matchCandidates(QList<NeuronTree> * ntList, QList<int> * cand, double span, int direction, QList<int> MatchMarkers[2]); //from cand[1] to cand[0]
bool matchCandidates_speed(QList<NeuronTree> * ntList, QList<int> * cand, double span, int direction, QList<int> MatchMarkers[2]); //from cand[1] to cand[0]

void getTopCliques(const NeuronTree& nt, const QList<int>& list, const QList<XYZ>& coord, const QList<XYZ>& dir, QVector<Clique3> & cqlist, double minDis,int stackDir, int maxCliqueNum);
void getCliques(const NeuronTree& nt, QList<int> list,  QList<Clique3> & cqlist, double minDis);
void getCliques(const QList<int>& list, const QList<XYZ>& coord, const QList<XYZ>& dir,  QVector<Clique3> & cqlist, double minDis, int stackDir);
void getCliques(const NeuronTree& nt, const QList<int>& list, const QList<XYZ>& coord, const QList<XYZ>& dir,  QVector<Clique3> & cqlist, double minDis, int stackDir); //only candidate with type 6/7/26/27 will be chosen
void getMatchingCandidates(const NeuronTree& nt, QList<int>& cand, float min, float max, int direction);
void getMatchingCandidates(const NeuronTree& nt, QList<int>& cand, float min, float max, int direction, float segThr); //this one will not consider small segmentsbool matchCandidates(QList<NeuronTree> * ntList, QList<int> * cand, double span, int direction, QList<int> MatchMarkers[2]); //from cand[1] to cand[0]

void getMatchPairs_XYZList(const QList<XYZ>& c0, const QList<XYZ>& c1, const QList<XYZ>& c0_dir, const QList<XYZ>& c1_dir, const QList<int>& c0_conncomponent, const QList<int>& c1_conncomponent, QList<int> * MatchMarkers, double span, double cos_angle);
void getMatchPairs_XYZList(const QList<XYZ>& c0, const QList<XYZ>& c1, const QList<XYZ>& c0_dir, const QList<XYZ>& c1_dir, QList<int> * MatchMarkers, double span, double cos_angle);
void getMatchPairs_XYZList(const QList<XYZ>& c0, const QList<XYZ>& c1, QList<int> * MatchMarkers, double span);
double distance_XYZList(QList<XYZ> c0, QList<XYZ> c1);
//double distance_XYZList(QList<XYZ> c0, QList<XYZ> c1, QList<XYZ> c0, QList<XYZ> c1);
void rotate_XYZList(QList<XYZ> in, QList<XYZ>& out, double angle, int axis); //angle is 0-360  based
void affine_XYZList(const QList<XYZ>& in, QList<XYZ>& out, double shift_x, double shift_y, double shift_z, double angle, double cent_x, double cent_y, double cent_z, int axis); //angle is 0-360  based
bool minus_XYZList(QList<XYZ>& a, QList<XYZ>& b, QList<XYZ>& out);
double compute_affine_4dof(QList<XYZ> reference, QList<XYZ> tomove, double& shift_x, double& shift_y, double & shift_z, double & angle_r, double & cent_x,double & cent_y,double & cent_z,int dir);
//double compute_affine_4dof(QList<XYZ> reference, QList<XYZ> tomove, QList<XYZ> reference_dir, QList<XYZ> tomove_dircoord, double& shift_x, double& shift_y, double & shift_z, double & angle_r, double & cent_x,double & cent_y,double & cent_z,int dir);
bool compute_rotation(QList<XYZ> reference, QList<XYZ> tomove, double& ang,int dir); //reference and tomove should all be 0 centered.

void update_marker_info(const LocationSimple& mk, int* info); //info[0]=neuron id, info[1]=point id, info[2]=matching marker
void update_marker_info(const LocationSimple& mk, int* info, int* color);
bool get_marker_info(const LocationSimple& mk, int* info);

double getNeuronTreeMidplane(const NeuronTree& nt0, const NeuronTree& nt1, int direction);
void getNeuronTreeBound(const NeuronTree& nt, float * bound, int direction);
void getNeuronTreeBound(const NeuronTree& nt, double &minx, double &miny, double &minz,
                        double &maxx, double &maxy, double &maxz,
                        double &mmx, double &mmy, double &mmz);

void highlight_planepoint(const NeuronTree& nt0, const NeuronTree& nt1, float dis, int direction);
int highlight_edgepoint(const QList<NeuronTree> *ntList, float dis, int direction);

int highlight_adjpoint(const NeuronTree& nt1, const NeuronTree& nt2, float dis);

void change_neuron_type(const NeuronTree& nt, int type);

void copyType(QList<int> source, const NeuronTree & target);

void copyType(const NeuronTree & source, QList<int> & target);

void backupNeuron(const NeuronTree & source, const NeuronTree & backup);

void copyProperty(const NeuronTree & source, const NeuronTree & target);

void copyCoordinate(const NeuronTree & source, const NeuronTree & target);

float quickMoveNeuron(QList<NeuronTree> * ntTreeList, int ant, int stackdir, int first_nt);
float quickMoveNeuron(NeuronTree *nt0, NeuronTree *nt1, int stackdir);
float quickMoveNeuron(NeuronTree * nt0, const QList<int>& cand0, NeuronTree * nt1, const QList<int>& cand1, int stackdir);
void affineNeuron(NeuronTree &nt_in, NeuronTree &nt_out, double shift_x, double shift_y, double shift_z, double angle, double cent_x, double cent_y, double cent_z, int axis); //angle is 0-360  based

void multiplyAmat(double* front, double* back);
void multiplyAmat_centerRotate(double* rotate, double* tmat, double cx, double cy, double cz);
void getAffineAmat(double amat[16], double& shift_x, double& shift_y, double & shift_z, double & angle_r, double & cent_x,double & cent_y,double & cent_z,int dir);

bool writeAmat(const char* fname, double* amat);
bool readAmat(const char* fname, double* amat);

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
#endif // NEURON_STITCH_FUNC_H
