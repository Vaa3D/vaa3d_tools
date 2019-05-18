#ifndef NEURONTREEPRUNEORGRAFT_H
#define NEURONTREEPRUNEORGRAFT_H
#include"tipdetector.h"
#include"ray_shooting.h"
#include"my_surf_objs.h"
#include"neuron_tools.h"
#include "v3d_curvetracepara.h"
#include "scoreinput.h"
#define using_exist_TMI_data
class neurontreepruneorgraft:public QObject
{
    Q_OBJECT
public slots:
    void getDateFromGUI();

public:
    neurontreepruneorgraft();
    ~neurontreepruneorgraft();
    TipDetector tp;
    bool getImgData(V3DPluginCallback2 &callback, QWidget *parent);
    bool getNeuronTree(V3DPluginCallback2 &callback2, QWidget *parent);

    NeuronTree return_neurontree()
    {
        //init neurontree
        return init_neurontree;
    }
    bool getImgData_not_process_tip(V3DPluginCallback2 &callback2, QWidget *parent);
    void do_function_dialog(V3DPluginCallback2 &callback, QWidget *parent);
    QStringList set_function_list();
    void set_small_length(double small_length_pruning)
    {
        small_length=small_length_pruning;
    }
    void set_leaf_fix_length(double all_leaf_fix_length_pruning)
    {
        pruing_all_leaf_fix_length=all_leaf_fix_length_pruning;
    }
    LandmarkList return_marker_need_to_show()
    {
        return need_to_show;
    }

    NeuronTree tipBasedPruning(NeuronTree nt);
    NeuronTree tipBasedPruning_stop_before_tip(NeuronTree nt);
    NeuronTree sort_neuron( NeuronTree nt);
    NeuronTree mergeSwc_two_leafnode( NeuronTree nt);
    NeuronTree reestimate_radius(NeuronTree nt);
    NeuronTree graftToBorder(NeuronTree nt);
    NeuronTree pruningSwc_small_length(NeuronTree nt);
    NeuronTree pruningSwc_two_leafnode(NeuronTree nt);
    NeuronTree tipBasedPruning_stop_before_tip_no_z(NeuronTree nt);
    NeuronTree Pruning_leaf_node_in_background(NeuronTree nt);
    NeuronTree TreeSegment(NeuronTree nt);
    NeuronTree pruningSwc_back_ground(NeuronTree nt);


    LandmarkList find_leaf_need_prun(NeuronTree nt);
    LandmarkList find_leaf_need_prun(NeuronTree nt, LandmarkList tip_list, float threshold=11, float threshold2=15, bool ignore_z=true);
    LandmarkList FromNeuronTreeGetLeaf(NeuronTree nt);
    LandmarkList FromNeuronTreeGetLeaf_branch(NeuronTree nt);

    //no tp
    NeuronTree tipBasedPruning_stop_before_tip_no_arg_input(LandmarkList need_pruning_leaf, NeuronTree nt, LandmarkList realtip);
    NeuronTree pruingSwc_one_leafnode_stop_before_tip_no_arg_input(LocationSimple lead_need_prun,   //叶子节点
                                NeuronTree nt,LandmarkList true_tip_list);
    void setShowMatchResult(bool s)
    {
        TMI_show_match_result=s;
    }
    NeuronTree deal_not_match(NeuronTree nt,LandmarkList not_match);
    void setcallback(V3DPluginCallback2 &callback2)
    {
        cb=&callback2;
    }
    bool InputTreeCheck(NeuronTree nt);


    int pruning_branch_count;
    bool TMI_simple_argu_input();
    void Binarization(int threshold);
    void setSegmentImg(unsigned char * img );
    int binary_threshold;
    void Hackathon_process_tip(NeuronTree nt);
    bool setSwcImg(NeuronTree nt);
//    void Hackthon_process_onetip(NeuronTree nt,V3DLONG i);
    LandmarkList need_to_show;

private:
    V3DPluginCallback2 *cb;
    LandmarkList tip_list;
    bool TMI_show_match_result;


//    LandmarkList swc_tip_list;
//    LandmarkList swc_branch_list;

//    vector<MyMarker*> swc_tip_Marker;
//    vector<MyMarker*> swc_branch_Marker;
    NeuronTree init_neurontree;
//    NeuronTree temp_neurontree;

//    vector<MyMarker*> neurontree;
//    vector<MyMarker*>  swc;
    double dist_traced;
    unsigned int radius_tip_checked;
    double dist_need_trace;
    unsigned int radius_leaf_detect;
    double pruingSwc_one_leafnode_stop_length;
    bool isProcessTip;
    bool isInputSwc;
    char  need_tip[13];
    double small_length;
    double pruing_all_leaf_fix_length;

    Image4DSimple * p4DImage;
    Image4DSimple * p4DImage_bimg;
    Image4DSimple * p4DImage_swc_img;
    V3DLONG sz[4];
    unsigned char *datald;
    unsigned char *bimg_datald;
    unsigned char *swcimg_datald;
    vector<MyMarker*> find_shortest_path_graphimg(MyMarker *start_nodes, MyMarker *end_nodes);
    NeuronTree pruingSwc_one_leafnode(LocationSimple lead_need_prun,   //叶子节点
                                NeuronTree nt);
    NeuronTree pruingSwc_one_leafnode_stop_before_tip(LocationSimple lead_need_prun,   //叶子节点
                                NeuronTree nt);
    NeuronTree pruning_single_leaf_in_small_length(LocationSimple lead_need_prun,NeuronTree nt,int length);
    NeuronTree pruning_single_leaf_in_small_length(NeuronTree nt);


    void run_function_list(QString func_name, NeuronTree &nt, V3DPluginCallback2 &callback);
    QStringList func_list;

    bool trace_one_pt_to_one_points_shortestdist(MyMarker* start_nodes, MyMarker* end_nodes, NeuronTree &nt);
    bool trace_one_pt_to_one_points_shortestdist(LocationSimple start_nodes, LocationSimple end_nodes, NeuronTree &nt);



    NeuronTree deal_not_match_points(NeuronTree nt,LocationSimple not_match_p);
//    QString
    LandmarkList temp_for_debug;

    //TMI argu
    int based_distance;
    float online_confidece_min;
    int Max_number_other_swc;

    int background_stop_num;
    float angle_swc;

    //Hackathon
    void  Init_Hackathon_score(NeuronTree nt);
    QHash<int,int> Hash_leaf;
    float *condidence_score;
    float *parent_score;
    float *no_other_swc_score;
    float *match_score;
    float *online_confidece_score;
    float *ultimate_score_all;

    float  cal_swc_score(NeuronTree nt,int neu_i);
    float  cal_online_confidence_score(NeuronTree nt,int neu_i);//
    float  cal_tip_score(NeuronTree nt,int neu_i);
    void   cal_weighted_score(void);

    float weight_confidence_score;
    float weight_match_score;
    float weight_swc_score;
    vector<vector<float> > x_dis, y_dis,z_dis;

    ScoreInput* s;
    NeuronTree hacka_Tree;



};

#endif // NEURONTREEPRUNEORGRAFT_H
