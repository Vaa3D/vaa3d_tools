#ifndef T_CLASS_H
#define T_CLASS_H
#include <v3d_interface.h>

struct T_Point{
    V3DLONG n;
    V3DLONG x,y,z;
    double x_angle,y_angle,z_angle;
    V3DLONG adjoin_d,turn_d,bifurcation_d;
    V3DLONG parent;
    int branch;
    bool is_used;
    bool bifurcation;
    bool is_turn;
    enum{ALIVE=-1,TRIAL=0,FAR=1};
    char state;
    V3DLONG bifparent;
    V3DLONG turnparent;
    double bif_angle;

    int mode;
};






































#endif // T_CLASS_H
