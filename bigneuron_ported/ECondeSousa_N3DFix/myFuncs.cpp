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
* (last update: June 30, 2015)
*/

#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include "myHeader.h"
#include "Dend_Section.h"
#include "v3d_message.h"
#include <algorithm>


long maximizante(std::vector<double> &myvec){
    double up = myvec.at(0)-1;
    long m;
        for(unsigned it = 0; it<myvec.size();it++){
            if(up < myvec.at(it)){
                up = myvec.at(it);
                m = (long)it;
            }
        }
        return m;
}

double maximo(std::vector<double> &myvec){
    double up = myvec.at(0)-1;
    for(unsigned it = 0; it<myvec.size();it++){
        if(up < myvec.at(it)){
            up = myvec.at(it);
        }
    }
    return up;
}

double minimo(std::vector<double> &myvec){
    double up = myvec.at(0)+1;
    for(unsigned it = 0; it<myvec.size();it++){
        if(up > myvec.at(it)){
            up = myvec.at(it);
        }
    }
    return up;
}

unsigned indexGreaterThan(std::vector<double> myvec, const double Objective){
    double tmp = maximo(myvec);
    if(tmp <= Objective){
        return -1;
    }else{
        long it=0;
        while(myvec.at(it)<=Objective){
            it++;
        }
        return it;
    }
}


long indexEqualOrGreaterThan(std::vector<double> myvec, const double Objective){
    double tmp = maximo(myvec);
    if(tmp < Objective){
        return -1;
    }else{
        long it=0;
        while(myvec.at(it)<Objective){
            it++;
        }
        return it;
    }
}


long indexEqualOrLesserThan(std::vector<double> myvec, const double Objective){
    double tmp = minimo(myvec);
    if(tmp > Objective){
        return -1;
    }else{
        long it=0;
        while(myvec.at(it)>Objective){
            it++;
        }
        return it;
    }
}

bool test_ascend(std::vector<double> &DIAM,double &step_min,long x1,long x2){
    std::vector<double> Diam;
    Diam.assign(DIAM.begin()+x1,DIAM.begin()+x2+1);
    double MAX=maximo(Diam);
    bool answer = ( ( MAX / Diam.at(0)) > (1 + step_min) );
    return answer;
}

bool test_peak(std::vector<double> Diam, double &step_min){
    double MAX=maximo(Diam);
    bool answer1 = ( ( MAX / Diam.at(0) ) > ( 1 + step_min ) );
    bool answer2 = ( ( MAX / Diam.at(Diam.size()-1) ) > ( 1 + step_min ) );
    bool answer = answer1 & answer2;
    return answer;
}

long test_descend(std::vector<double> &DIAM,std::vector<long> & pnslope,std::vector<double> &dydx,
                  double &thresh,double &step_min,long x1,long x2){

    std::vector<long> pnslope_tmp;
    for(unsigned i=0;i<pnslope.size();i++){
        if( (pnslope.at(i)>x1-1) & (pnslope.at(i)<x2+1) ){
            pnslope_tmp.push_back(pnslope.at(i));
        }
    }

    std::vector<double> dydx_tmp;
    for(unsigned i=0;i<pnslope_tmp.size();i++){
        dydx_tmp.push_back(dydx.at(pnslope_tmp.at(i)));
    }



    //vectors were reduced to contain only the points of interest
    bool flag;
    if(dydx_tmp.size()>0){
        flag = 1;	//flag = 1: signal to continue
                    //flag = 0: signal to stop
    }else{
        flag = 0;
    }

    long x3 = -1;
    while(flag==1){
        if(dydx_tmp.size()>0){
            double dydxMin=minimo(dydx_tmp);
            if(dydxMin<=-thresh){
                x3=indexEqualOrLesserThan(dydx_tmp,-1*thresh);
                //printf("x3=%d\n",x3);
                std::vector<double> Diam;
                Diam.assign( DIAM.begin()+x1, DIAM.begin()+(pnslope_tmp.at(x3)+1)+1);
                if( test_peak( Diam,step_min )==1 ){
                    flag =0;
                }else{
                    dydx_tmp.erase(dydx_tmp.begin()+x3);
                    pnslope_tmp.erase(pnslope_tmp.begin()+x3);
                    x3 = -1;
                }
            }else{
                flag = 0;
            }
        }else{
            flag = 0;
        }
    }
//    printf("x2 = %d\nx3 = %d\n",x2,x3);
    if(x3!=-1 && x3 <= x2){
//        printf("if(x3!=-1 && x3 <= x2)\n");
        return pnslope_tmp.at(x3)+1;
    }else{
//        printf("else\n");
        x3=-1;
        return x3;
    }
}




long move_down_right(long x2,long x3,double &thresh,std::vector<double> &DIAM,std::vector<double> &ARC,std::vector<long> & pnslope){

    long tmp_point = x3-1;
    while ((std::find(pnslope.begin(),pnslope.end(),x3) != pnslope.end()) & x3<x2){ //pnslope.contains(x3) & x3<x2
        x3=x3+1;
        double m = (DIAM.at(tmp_point)-DIAM.at(x3))/(ARC.at(tmp_point)-ARC.at(x3));
        if(m>-1*thresh){
            break;
        }
    }
    return x3;

}

long move_down_left(long x1,long x3_tmp,double &thresh,double &max_dist,
                    std::vector<double> &DIAM,std::vector<double> &ARC,std::vector<long> & ppslope){

    std::vector<double> Diam;
    Diam.assign( DIAM.begin()+x1, DIAM.begin()+x3_tmp+1);
    long tmp_point = maximizante(Diam);



    while (x1-1>=0 && (std::find(ppslope.begin(),ppslope.end(),x1-1) != ppslope.end()) && ARC.at(x3_tmp)-ARC.at(x1-1)<=max_dist){
        double m = (DIAM.at(tmp_point)-DIAM.at(x1-1))/(ARC.at(tmp_point)-ARC.at(x1-1));
        if(m<thresh){
            break;
        }else{
            x1=x1-1;
        }
    }
    return x1;
}

