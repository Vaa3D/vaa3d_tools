#ifndef GLOBAL_H
#define GLOBAL_H

#define GET_IND(x,y,z) (x) + (y) * sz_x + (z) * sz_xy
#define GET_X(ind) ind % sz_x
#define GET_Y(ind) (ind / sz_x) % sz_y
#define GET_Z(ind) (ind / sz_xy) % sz_z
#define DISTANCE(src,dst) \
        sqrt(pow((GET_X(src) - GET_X(dst)) * 1.0,2.0)\
        + pow((GET_Y(src) - GET_Y(dst)) * 1.0,2.0)\
        + pow((GET_Z(src) - GET_Z(dst)) * 1.0,2.0));\

int sz_x = -1;
int sz_y = -1;
int sz_z = -1;
int sz_total = -1;
int sz_xy = -1;
unsigned char bresh = -1;
double coverRate = 1;
string fileName;
qint64 timeCost = 0;
qint64 totalTimeCost = 0;
qint64 lastTimeCost = 0;
QElapsedTimer timer;

#endif // GLOBAL_H
