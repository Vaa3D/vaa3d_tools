#ifndef NEURON_MATCH_CLIQUE_H
#define NEURON_MATCH_CLIQUE_H

#include "neuron_stitch_func.h"
#include <v3d_interface.h>

#include <QDialog>
#include <vector>
#include <map>

using namespace std;

class neuron_match_clique
{
private:
    //1 always has larger coordinate than 0 in stacking direction
    //0 and 1 should have already been stacked in z direction and rescaled in z direction to make isotropic

    //neuron
    NeuronTree *nt0;
    NeuronTree *nt0_org;
    NeuronTree *nt1;
    NeuronTree *nt1_org;
    NeuronTree *nt1_a;
    NeuronTree *nt0_stitch;
    NeuronTree *nt1_stitch;
    QList<int> components0, components1;
    QList<int> parent0, parent1;
    HBNeuronGraph ng0, ng1;
    QList<int> neuronType0, neuronType1;

    //candidate point for matching
    QList<int> candID0, candID1; //neuron tree point ids of candidates
    QList<XYZ> candcoord0, candcoord1, candcoord1_adj; //cooridnate of candidates
    QList<XYZ> canddir0, canddir1, canddir1_adj; //direction of neuron tips
    QList<XYZ> canddircoord0, canddircoord1, canddircoord1_adj; //this is for transformation
    QList<int> pmatch0, pmatch1; //matched candidate, the index of neuron tree point
    QList<int> candmatch0, candmatch1; //matched candidate, the index of candidate in List
    QList<int> candcomponents0, candcomponents1; //the connected components each cand belongs to

    //2d array for matching score between candidates
    vector<int> candMS;
    int MS_x, MS_y;

    //cliques
    QVector<Clique3> cliqueList0, cliqueList1;
    multimap<int, QVector<int> , std::greater<int> > rankedCliqueMatch;

public:
    //parameters
    double spanCand;  //searching span from the stack plane for the candidate
    int direction;  //stack direction, 0x, 1y, 2z
    double dir_range;   //length of section used to calculate direction of dead end
    double midplane;    //coordinate of the plane in between
    double angThr_match; //angular threshold for clique match (-1 ~ 1)=cos(theta), when angle is larger than theta (<cos(theta)), the point will not be matched
    double angThr_stack; //angular threshold for candidate in stack direction (-1 ~ 1), -1 means nothing will be thresholded
    double cmatchThr;   //match threshold for length of cliques
    double pmatchThr;   //match threshold for points
    double zscale;  //resacle stack direction
    double segmentThr;  //threshold to filter out small segments when selecting candidates
    double gapThr; //threshold to filter out small gaps
    int spineLengthThr; //the length threshold of spine
    double spineAngThr; //angular threshold of spine
    double spineRadiusThr; //radius threshold of spine
    int maxClique3Num; //the maximum number of 3 cliques considered in each stack - for speed

    //transformation
    double shift_x, shift_y, shift_z, rotation_ang, rotation_cx, rotation_cy, rotation_cz;

public:
    neuron_match_clique(NeuronTree* botNeuron, NeuronTree* topNeuron);
    void init();
    void init(LandmarkList * mList); //init based on markers
    void initNeuronComponents(); //this is contained in init()
    void globalmatch();
    void update_matchedPoints_to_Markers(LandmarkList * mList);
    void output_matchedMarkers(QString fname, const NeuronTree& nt, QList<int> points);
    void output_matchedMarkers_orgspace(QString fname_0, QString fname_1);
    void output_markers_orgspace(QString fname);
    void output_markers_candspace(QString fname);
    void output_markers_affinespace(QString fname);
    void output_markers_stitchspace(QString fname);
    void output_candMatchScore(QString fname);
    void output_affine(QString fname_out, QString fname_nt0);
    void output_parameter(QString fname);
    void affine_nt1();
    void output_stitch(QString fname_out);
    void stitch(); //need to get match points and affine_nt1 first
    bool stitch(int point0, int point1); //input is the match points, will search both direction for stiching.
    bool checkloop(int point0, int point1); //check if connect point0 and point 1 will result in loop
    void highlight_nt1_seg(int point1, int type); //highlight the component of nt1 linked to point1
    void highlight_nt0_seg(int point0, int type); //highlight the component of nt0 linked to point0
    V3DLONG search_tip0(V3DLONG point0); //find the id of closest tips point, if there is none, return -1
    V3DLONG search_tip1(V3DLONG point1); //find the id of closest tips point, if there is none, return -1

private:
    //orientation should be 1/-1 for smaller/larger stack in direction
    void initNeuronAndCandidate(NeuronTree& nt, const HBNeuronGraph& ng, QList<int>& neuronType, QList<int>& cand, QList<XYZ>& candcoord, QList<XYZ>& canddir, QList<XYZ>& canddircoord, QList<int>& components, QList<int>& candcomponents, QList<int>& pList, int orientation); //this one will not consider small segments, small gaps
    void initNeuron(NeuronTree& nt, const HBNeuronGraph& ng, QList<int>& neuronType, QList<int>& components, QList<int>& pList);
    int initNeuronType(const NeuronTree& nt, const HBNeuronGraph& ng, QList<int>& neuronType);
    void initNeuronComponents(NeuronTree& nt, QList<int>& components, QList<int>& pList);
    void matchCliquesAndCands();
    void load_cand_from_Markers(LandmarkList * mList);
    void update_cand(const NeuronTree& nt, const HBNeuronGraph& ng, const QList<int>& neuronType, const QList<int>& cand, const QList<int>& pList, const QList<int>& components, QList<XYZ>& candcoord, QList<XYZ>& canddir, QList<XYZ>& canddircoord, QList<int>& candcomponents, const int orientation);
};

class NeuronMatchOnlyDialog : public QDialog
{
    Q_OBJECT

public:
    NeuronMatchOnlyDialog(NeuronTree *nt0, NeuronTree *nt1, LandmarkList *mList, double affineParam[7]);

private:
    void creat();

public slots:
    void match();
    void spineCheck(int c);

private:
    NeuronTree *nt0, *nt1;
    LandmarkList * mList;
    double *affine; //shiftx, shifty, shiftz, rotation_ang, rotation_cx, rotation_cy, rotation_cz

public:
    QGridLayout *gridLayout;
    QComboBox *cb_dir,*cb_pair;
    QDoubleSpinBox *spin_zscale, *spin_ang, *spin_matchdis, *spin_searchspan, *spin_cmatchdis, *spin_segthr;
    QSpinBox *spin_maxcnum;
    QPushButton *btn_quit, *btn_match;
    QSpinBox *spin_spineLen;
    QDoubleSpinBox *spin_spineAng, *spin_spineRadius;
    QCheckBox *check_spine;
};

class NeuronLiveMatchDialog : public QDialog
{
    Q_OBJECT

public:
    NeuronLiveMatchDialog(V3DPluginCallback2 * callback, V3dR_MainWindow* v3dwin);
    void enterEvent(QEvent *e);

private:
    void creat();
    void checkwindow();
    void updateview();
    void updatematchlist();
    void highlight_pair();
    void link_new_marker_neuron();
    void update_marker_to_neuron();

public slots:
    void match();
    void manualadd();
    void skip();
    void stitch();
    void stitchall();
    void output();
    void change_pair(int idx);
    void spineCheck(int c);

private:
    V3DPluginCallback2 * callback;
    V3dR_MainWindow* v3dwin;
    QList<NeuronTree> *ntList;
    QList<NeuronTree> ntList_bk;
    neuron_match_clique * matchfunc;
    LandmarkList * mList;
    View3DControl * v3dcontrol;

    QList<int> pmatch0, pmatch1, mmatch0, mmatch1;
    QList<int> stitchmask; //if loopmask[i] true, then connecct pmatch0[i] and pmatch1[i] will result in loop

    int cur_pair;
//    double bound_min[3],bound_max[3];

public:
    QGridLayout *gridLayout;
    QComboBox *cb_dir,*cb_pair;
    QDoubleSpinBox *spin_zscale, *spin_ang, *spin_matchdis, *spin_searchspan, *spin_cmatchdis, *spin_segthr, *spin_gapthr;
    QSpinBox *spin_maxcnum;
    QPushButton *btn_quit, *btn_match, *btn_manualmatch, *btn_skip, *btn_stitch, *btn_stitchall, *btn_output;
    QSpinBox *spin_spineLen;
    QDoubleSpinBox *spin_spineAng, *spin_spineRadius;
    QCheckBox *check_spine;
    QGroupBox *group_marker;
};

class NeuronMatchDialog : public QDialog
{
    Q_OBJECT

public:
    //NeuronMatchDialog(V3DPluginCallback2 * callback, V3dR_MainWindow* v3dwin);
    NeuronMatchDialog();
    //~NeuronMatchDialog();

public:
    void creat();

public slots:
    void run();
    bool load0();
    bool load1();
    bool output();
    void outputchange(QString text);
    void spineCheck(int c);

private:
//    V3DPluginCallback2 * callback;
//    V3dR_MainWindow* v3dwin;
//    QList<NeuronTree> *ntList;
    NeuronTree* nt0;
    NeuronTree* nt1;
    QString folder_output;
    QString fname_output;
    QString name_nt0;
    QString name_nt1;
    QString fname_nt0;
    QString fname_nt1;

public:
    QGridLayout *gridLayout;
    QPushButton *btn_load0, *btn_load1, *btn_output;
    QLabel *label_load0, *label_load1, *label_output;
    QLineEdit *edit_load0, *edit_load1, *edit_output;
    QComboBox *cb_dir;
    QDoubleSpinBox *spin_zscale, *spin_ang, *spin_matchdis, *spin_searchspan, *spin_cmatchdis, *spin_segthr;
    QSpinBox *spin_maxcnum;
    QPushButton *btn_quit, *btn_run;
    QCheckBox *check_stitch;
    QSpinBox *spin_spineLen;
    QDoubleSpinBox *spin_spineAng, *spin_spineRadius;
    QCheckBox *check_spine;
};

#endif // NEURON_MATCH_CLIQUE_H
