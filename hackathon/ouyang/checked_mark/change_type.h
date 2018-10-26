#ifndef CHANGE_TYPE_H
#define CHANGE_TYPE_H

#include <QtGlobal>
#include <math.h>
#include "basic_surf_objs.h"
#include <string.h>
#include <vector>
#include <iostream>
#include <v3d_interface.h>
//#include <PMain.h>
using namespace std;
struct forbackup{
public:
    QList<NeuronSWC> backup;
    QList<NeuronSWC> backupelse;
    QList<NeuronSWC> backup_current_type0,current_block_all,current_block_all_else;
    vector<double> all_current_x,all_current_y,all_current_z;//all backup x,y,z
    vector<int> all_current_type;
};
void checked_mark_type(V3DPluginCallback2 &callback, QWidget *parent);
void return_checked(V3DPluginCallback2 &callback, QWidget *parent);
#endif // CHANGE_TYPE_H
