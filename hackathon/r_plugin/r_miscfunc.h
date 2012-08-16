/*r_miscfunc.h
 * Stores assorted R functions for other vaa3d plugin use
 * Brad Busse, GPL
 */
 
#ifndef __R_MISCFUNC_H__
#define __R_MISCFUNC_H__

#include <string>
#include <RInside.h> 
#include <vector>
#include <QtGui>
#include <v3d_interface.h>

using namespace std;

string hello();
//Rcpp::NumericMatrix createMatrix(const int n, const int m); //make meaningless R matrix
//Rcpp::NumericVector createvector(const int n); //make meaningless R vector
Rcpp::NumericMatrix vectvect2rmat(vector<vector<double> > in);
vector<double> r_SVMclassify(vector<vector<double> > trainX,vector<double>trainY,vector<vector<double> > testX);
void r_demo_processImage(V3DPluginCallback2 &callback, QWidget *parent);


#endif
