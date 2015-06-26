#ifndef SUBTREE_DIALOG_H
#define SUBTREE_DIALOG_H

#include <QtGui>
#include "v3d_basicdatatype.h"
#include "basic_surf_objs.h"
#include "v3d_interface.h"
#include "math.h"
#include <vector>
#include "v3d_message.h"

using namespace std;
void backupNeuron(NeuronTree & source, NeuronTree & backup);

struct tree{int tree_id; //start from 1
            int sort_node;  //use which node to sort, if no marker set to -1, if -1 no sort
            int soma_marker; //which landmark is the soma, if no soma set to -1
            vector<int> belong_markers;}; //record landmark idx put on this tree

class subtree_dialog:public QDialog
{
    Q_OBJECT
public:
   explicit subtree_dialog(V3DPluginCallback2 *cb);
   bool get_swc_marker();
   void assign_marker_type();
   void check_window();
   void calc_distance_to_subtree_root();
   bool sort_swc_dialog();
   void build_connt_LUT();
   void connected_components();

public slots:
   void marker_change();
   void soma_clicked();
   void axon_clicked();
   void apical_dendrite_clicked();
   void basal_clicked();
   void oblique_clicked();
   void refresh_marker();
   bool maybe_save();
   bool save();
   void run();

private:
   int calc_nearest_node_around_marker();
   void sort_type_def(int type, float dendrite_id, int marker_id);
   void define_sort_id_for_trees();
   void sort_all_trees();
   void label_single_tree(int type, int tree_id, int marker_id);
   void build_new_parent_LUT_after_sort();
   void subtree_define();

private:
   V3DPluginCallback2 *callback;
   LandmarkList LList_in;
   NeuronTree *nt;
   V3dR_MainWindow *swc_win;
   vector<vector<int> > subtree;
   vector<vector<int> > parent_LUT;
   QComboBox *markers;
   QDialog *mydialog2;
   map<int,int> mask;
   vector<tree> all_trees;
};

#endif // SUBTREE_DIALOG_H


