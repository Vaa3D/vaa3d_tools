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


#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include "myHeader.h"
#include "Dend_Section.h"
#include "v3d_message.h"



/////////////////////////////////////////////////////////////////////////////////////////
//	define ARC and DIAM
/////////////////////////////////////////////////////////////////////////////////////////
void create_vector_data(std::vector< std::vector<struct RawPoints > > &dend , unsigned dend_num,std::vector<double> &ARC,std::vector<double> &DIAM){


	ARC.clear();
	DIAM.clear();
    for(unsigned it=0;it<dend.at(dend_num).size();it++){
		ARC.push_back(dend.at(dend_num).at(it).arc); 
		DIAM.push_back(dend.at(dend_num).at(it).r);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
// Locate and eliminate bumps 
/////////////////////////////////////////////////////////////////////////////////////////

void N3DFix_v2(std::vector< std::vector<struct RawPoints > > &dend , unsigned dend_num, std::vector<double> &ARC,
               std::vector<double> &DIAM, std::vector<double> &baseline , std::vector<double> &weights,
               double &bump_rnorm, double &rmin, double &bump_slope,QString fileReport){


    bool DoNodes = true;
    double dmin = 2*rmin;


    long Npoints = dend[dend_num].size();
    double r0;
    double m;

    // Calculate fibre/arc distance of all points in dend[dend_num]
    ARC.clear();
    DIAM.clear();
    create_vector_data(dend , dend_num,ARC,DIAM);



    //Correct points with "zero" diamter (d<dmin)
    for (unsigned i=0;i<Npoints;i=i+1){
        if(DIAM[i] < dmin){
            DIAM[i] = dmin;
        }
    }


    // CALCULATE TRIGGERS
    // Trigger 1: r, baseline radius
    baseline.clear();
    weights.clear();
    Calculate_Baseline( DIAM , ARC, baseline, weights);


    // Trigger 2: delta_r / delta_x, slope
    std::vector<double> slope;
    // Trigger 3: r / baseline, normalized radius
    std::vector<double> rnorm;
    rnorm.push_back(0);
    slope.push_back(0);

    for(long p =1;p< Npoints;p++ ){
        // rnorm
        rnorm.push_back( DIAM[p] / baseline[p]);
        // slope
        if( abs( ARC[p] - ARC[p-1] ) < 1.0e-6){
            if( abs( DIAM[p] - DIAM[p-1] ) < 1.0e-6){
                slope.push_back(0.0); // ATENTION!!! equal points are not removed! slope is set to zero!
            }else{
                double sign = ( DIAM[p] - DIAM[p-1] ) / abs( DIAM[p] - DIAM[p-1] );
                slope.push_back( 2.0 * sign * bump_slope); // ATENTION!!! same (x,y,z) but abrupt change in r! give the slope trigger
            }
        }else{// normal case
            slope.push_back( (DIAM[p] - DIAM[p-1] ) / ( ARC[p] - ARC[p-1] ) );
        }
    }




    // Identify locations to correct
    std::vector<double> signal_cor;
    signal_cor.push_back(0);
    for(unsigned p =1; p<Npoints ;p++){
        if( abs( slope[p] ) < bump_slope && abs( rnorm[p] - 1 ) < bump_rnorm ){
            signal_cor.push_back(0);
        }else{
            signal_cor.push_back(1);
        }
    }



    // Correct points using linear interpolation, whenever necessary
    std::vector<double> DIAM_new;
    for (unsigned i =0;i<DIAM.size();i=i+1){
        DIAM_new.push_back(DIAM[i]);
    }
    long p = 1, p1, p2;
    double r_father;
    while( p < Npoints){
        // search for row of points to correct
        if( signal_cor[p] == 1){
            p1 = p - 1;	// start point
            p += 1;
            while(p < Npoints && signal_cor[p] == 1){
                p += 1;
            }
			
            
			//p2 = p - 1;
            if( p<=Npoints ){
                p2 = p- 1;
            }else{
                p2 = Npoints - 1;
            }
			
            double delta_x = ARC[p2]-ARC[p1];
            if (abs(delta_x)>1.0e-6){
                // this IS a bump; remove according with the locations of p1 and p2

                //choose r0 according to the location of p1
                if(p1!=0){
                    // p1 is not a node
                    r0 = DIAM[p1];
                }else{
                    // p1 is a node
                    if( dend[dend_num][p1].ppid != -1 && DoNodes == true ){
                        // p1 is a node and is not the 1st point in tree - go upstream and check diameter profiles in the father section
                        for(long ii=0;ii<dend.size();ii++){
                            if(dend[ii][dend[ii].size()-1].pid == dend[dend_num][p1].pid){
                                r_father = dend[ii][dend[ii].size()-1].r;
                                break;
                            }
                        }
                        r0=r_father;
                        if(r_father > baseline[p1]){// chose this option if it generates less steepness at edge
                            r0 = baseline[p1];
                        }
                    }else{// dend[dend_num][p1].ppid == -1 || DoNodes == false
                        r0 = DIAM[p1];
                    }
                }



                //now choose m according to the location of p2
                if(p2!=Npoints-1){
                    //p2 is not a node
                    m = (DIAM[p2]-r0) / delta_x;
                }else{
                    //p2 is a node
                    //force end diam to baseline level
                    m = (baseline[p2]-r0) / delta_x;
                    if(m > bump_slope || abs(DIAM[p2]/baseline[p2] - 1) > bump_rnorm){
                        m = (MEDIAN(DIAM)-r0) / delta_x;
                    }
                }

                //interpolate swelling artifact using linear interpolation: DIAM_new[i] = m*(ARC[i]-ARC[i]) + r0
                //printf("changed points in dend[%d]:\n",dend_num);
                for (long i=p1;i<=p2;i=i+1){
                    //printf("ARC[%d]=%g->%g->",i,ARC[i],DIAM_new[i]);
                    DIAM_new[i] = m*(ARC[i]-ARC[p1]) + r0;
                    //printf("%g\n",DIAM_new[i]);
                }

            }else{//abs(delta_x)<=1.0e-6
                for (long i=p1;i<=p2;i=i+1){
                    DIAM_new[i] = baseline[i];
                }
            }

        }else{
            p = p+1;
        }
    }


    for(long ii=0;ii<Npoints;ii++){
        dend[dend_num][ii].r=DIAM_new[ii];
    }


    FILE * fout;
    fout = fopen(fileReport.toStdString().c_str(), "a");

    if (fout == NULL) {//if (err != 0) {
        v3d_msg("The file was not opened\n",1);
        //return -1;
    }else{

        for(unsigned ii=0;ii<Npoints;ii=ii+1){
            if(abs(DIAM[ii]-DIAM_new[ii])<1e-6){
                signal_cor[ii]=0;
            }else{
                signal_cor[ii]=1;
            }
            //#col:	0	1		2		3			4 		5		6			7           8
            // 		ii  ARC(ii) DIAM    DIAM_new    slope   rnorm   baseline    outlier     signal_cor
            fprintf (fout ,"dend[%d]\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",dend_num,ii,ARC[ii],DIAM[ii]/2,DIAM_new[ii]/2,slope[ii],
                     rnorm[ii],baseline[ii],1-weights[ii],signal_cor[ii]);
        }
        fclose(fout);
    }
}

