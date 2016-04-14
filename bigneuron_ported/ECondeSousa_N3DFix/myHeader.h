/*
* N3DFix - automatic removal of swelling artifacts in neuronal 2D/3D reconstructions
* last update: Mar 2016
* VERSION 2.0
*
* Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>
*          Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>
* Date:    Mar 2016
*
* N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)
* Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016
*
*    Disclaimer
*    ----------
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You can view a copy of the GNU General Public License at
*    <http://www.gnu.org/licenses/>.
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
                std::vector<long> &end_sec, std::vector<long> &soma, std::vector<long> &pid);


void create_tree(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z, std::vector<float> &r,
                 std::vector<long> &ppid, std::vector<long> &end_sec, struct RawPoints &Point,
                 std::vector<struct RawPoints > &n3d, std::vector< std::vector<struct RawPoints > > &dend, std::vector<long> &pid ,
                 std::vector<long> &tree_id, std::vector<long> &soma);

void write_data(std::vector< std::vector<struct RawPoints > > &dend_original, std::vector< std::vector<struct RawPoints > > &dend,
                QString &path, QString &fileSaveName, std::vector<float> &x, std::vector<float> &y, std::vector<float> &z,
                std::vector<long> &tree_id, std::vector<float> &r, std::vector<long> &ppid, std::vector<long> &pid);

double MEDIAN(std::vector<double> DIAM);

double MEAN(std::vector<double> DIAM);

double STDEV(std::vector<double> DIAM);

double RandGen(double A, double B);

void Calculate_Baseline(std::vector<double> DIAM, std::vector<double> ARC, std::vector<double> &baseline , std::vector<double> &weights);

void N3DFix_v2(std::vector< std::vector<struct RawPoints > > &dend , unsigned dend_num, std::vector<double> &ARC,
               std::vector<double> &DIAM, std::vector<double> &baseline , std::vector<double> &weights,
               double &bump_rnorm, double &rmin, double &bump_slope, QString fileReport);

#endif

