/*
 *  sholl_func.cpp
 *  core functions for sholl neuron swc
 *  2018-06-26: by OuYang Qiang
 */

#ifndef __SHOLL_SWC_H_
#define __SHOLL_SWC_H_

#include <QtGlobal>
//#include <math.h>
//#include <unistd.h> //remove the unnecessary include file. //by PHC 20131228
#include "basic_surf_objs.h"
//#include <string.h>
#include <stdio.h>
//#include <algorithm>
//#include <vector>
#include <iostream>
#include <RInside.h>
#include <QSvgWidget>

#include <QtGui>

using namespace std;

#ifndef VOID
#define VOID 1000000000
#endif

bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined);
bool ShollSWC(QList<NeuronSWC> & neuron, double step);

class QtSholl : public QDialog
{

    Q_OBJECT

    public:
    QtSholl(RInside & R);
    static vector<double> radiusR;
    static vector<double> crossingsR;

//private slots:

private:
    void setupDisplay(void);    // standard GUI boilderplate of arranging things
    void plot(void);            // run a density plot in R and update the
    void filterFile(void);      // modify the richer SVG produced by R

    QSvgWidget *m_svg;          // the SVG device
    RInside & m_R;              // reference to the R instance passed to constructor
    QString m_tempfile;         // name of file used by R for plots
    QString m_svgfile;          // another temp file, this time from Qt
    //int m_bw, m_kernel;         // parameters used to estimate the density
    QString m_cmd;              // random draw command string
};

#endif
