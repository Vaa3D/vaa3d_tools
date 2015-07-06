/* Created by:
*
* 				Eduardo Conde-Sousa [econdesousa@gmail.com]
*							and
*				Paulo de Castro Aguiar [pauloaguiar@ineb.up.pt]
* 
* on Dez 10, 2014
* to visualize and remove artifacts resulting 
* from the 3D reconstruction of dendrites / axons
*
* (last update: July 06, 2015)
*/

#ifndef __MYFUNCS_H__
#define __MYFUNCS_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <QtGui>
#include <v3d_interface.h>

int _main(V3DPluginCallback2 &callback,QWidget *parent);

bool N3DFix_func(const V3DPluginArgList & input, V3DPluginArgList & output);

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);

void load_data(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z,std::vector<long>& tree_id,
               std::vector<float>& r, std::vector<long>& ppid,std::vector<long> &pid,QString &path);

void print_data(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z, std::vector<long> &tree_id,
                std::vector<float> &r, std::vector<long> &ppid,std::vector<long> &pid);

void find_nodes(std::vector<long> &tree_id, std::vector<long> &ppid, std::vector<long> &nodes, std::vector<long> &endpoints,
                std::vector<long> &end_sec, std::vector<long> &soma);

bool my_compare( float a, float b);

void diff(std::vector<long> &my_vec);

//float p3Ddist(float x1, float y1, float z1, float x2, float y2, float z2);

void create_tree(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z, std::vector<float> &r,
                 std::vector<long> &ppid, std::vector<long> &end_sec, struct RawPoints &Point,
                 std::vector<struct RawPoints > &n3d, std::vector< std::vector<struct RawPoints > > &dend, std::vector<long> &pid ,
                 std::vector<long> &tree_id, std::vector<long> &soma);

void remove_points_with_same_coordinates(std::vector< std::vector<struct RawPoints > > &dend,
                                         bool &preprocessing_flag, unsigned dend_num);

void deriv_dend(std::vector<double> &dydx, std::vector< std::vector<struct RawPoints > > &dend ,
                bool &preprocessing_flag, unsigned dend_num,std::vector<double> &ARC,std::vector<double> &DIAM);

void ups_and_downs(std::vector<double> &dydx, std::vector< std::vector<struct RawPoints > > &dend ,
                   bool &preprocessing_flag, unsigned dend_num,std::vector<long> &ppslope, std::vector<long> & pnslope,
                   std::vector<long> &pzslope,std::vector<double> &ARC,std::vector<double> &DIAM);

void smooth(std::vector<double> dydx, std::vector< std::vector<struct RawPoints > > &dend ,
            bool &preprocessing_flag, unsigned dend_num,std::vector<long> &ppslope,
            std::vector<long> & pnslope, std::vector<long> &pzslope,
            std::vector<double> &ARC,std::vector<double> &DIAM, long x1, long x2);
void locate_and_smooth_bumps(std::vector<double> &dydx, std::vector< std::vector<struct RawPoints > > &dend ,
                             bool &preprocessing_flag, unsigned dend_num,std::vector<long> &ppslope,
                             std::vector<long> & pnslope, std::vector<long> &pzslope, std::vector<double> &ARC,
                             std::vector<double> &DIAM, double &thresh,double &max_dist,double &step_min);

long maximizante(std::vector<double> &myvec);

double maximo(std::vector<double> &myvec);

double minimo(std::vector<double> &myvec);

unsigned indexGreaterThan(std::vector<double> myvec, const double Objective);

long indexEqualOrGreaterThan(std::vector<double> myvec, const double Objective);

long indexEqualOrLesserThan(std::vector<double> myvec, const double Objective);

bool test_ascend(std::vector<double> &DIAM,double &step_min,long x1,long x2);

bool test_peak(std::vector<double> Diam, double &step_min); //attention Diam != DIAM; Diam is a truncation of DIAM.

long test_descend(std::vector<double> &DIAM,std::vector<long> & pnslope,std::vector<double> &dydx,
                  double &thresh,double &step_min,long x1,long x2);

long move_down_right(long x2,long x3,double &thresh,std::vector<double> &DIAM,std::vector<double> &ARC,std::vector<long> & pnslope);

long move_down_left(long x1,long x3_tmp,double &thresh,double &max_dist,
                    std::vector<double> &DIAM,std::vector<double> &ARC,std::vector<long> & ppslope);
void write_data(std::vector< std::vector<struct RawPoints > > &dend,QString &path,QString &fileSaveName);


#endif

