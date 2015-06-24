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

class subtree_dialog:public QDialog
{
    Q_OBJECT
public:
   explicit subtree_dialog(V3DPluginCallback2 *cb);
   bool get_swc_marker();
   bool subtree_extract();
   void assign_marker_type();
   void check_window();
   void calc_distance_to_subtree_root();
   bool sort_swc_dialog();
   void build_connt_LUT();

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
   bool sort_slot();
   void skip_slot();

private:
   int calc_nearest_node_around_marker();
   void sort_type_def(int type, float dendrite_id, int marker_id);
private:
   V3DPluginCallback2 *callback;
   LandmarkList LList_in;
   NeuronTree *nt;
   V3DLONG sz_img[4];
   V3dR_MainWindow *swc_win;
   vector<vector<int> > subtree;
   QComboBox *markers;
   QDialog *mydialog2;
};

#endif // SUBTREE_DIALOG_H


