#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include "myHeader.h"
#include "Dend_Section.h"
#include "v3d_message.h"

/////////////////////////////////////////////////////////////////////////////////////////
//	remove_points_with_same_coordinates
/////////////////////////////////////////////////////////////////////////////////////////
// Some points have the same 3d coordinates. This will cause problems during the rest of 
// the process of smoothing. The following proc will search for these bugs and fix them. 
void remove_points_with_same_coordinates(std::vector< std::vector<struct RawPoints > > &dend , bool &preprocessing_flag, unsigned dend_num){

	preprocessing_flag = 1;
	//printf("preprocessing_flag = %d\n", preprocessing_flag);

	double m1, m2;
	if(dend.at(dend_num).size()>2){
		unsigned long it = 0;
		while(it <= dend.at(dend_num).size()-2){
			if(dend.at(dend_num).at(it+1).x==dend.at(dend_num).at(it).x && 
				dend.at(dend_num).at(it+1).y==dend.at(dend_num).at(it).y && 
				dend.at(dend_num).at(it+1).z==dend.at(dend_num).at(it).z){
//                printf("########################################################################## \n");
//                printf("########################################################################## \n");
//                printf("It is necessary to remove one point at dend.at(%d)!\n",dend_num);
//                printf("########################################################################## \n");
//                printf("########################################################################## \n");


				if(it>0 && it<dend.at(dend_num).size()-2){
					if (dend.at(dend_num).at(it+1).arc-dend.at(dend_num).at(it-1).arc!=0){
						m1=(dend.at(dend_num).at(it+1).r-dend.at(dend_num).at(it-1).r)/(dend.at(dend_num).at(it+1).arc-dend.at(dend_num).at(it-1).arc); // assuming that we use pt3dremove(it)
					}else{
						m1=(dend.at(dend_num).at(it+1).r-dend.at(dend_num).at(it-1).r)/(1e-10); // assuming that we use pt3dremove(it)
					}
					if (dend.at(dend_num).at(it).arc-dend.at(dend_num).at(it+2).arc!=0){
						m2=(dend.at(dend_num).at(it).r-dend.at(dend_num).at(it+2).r)/(dend.at(dend_num).at(it).arc-dend.at(dend_num).at(it+2).arc); // assuming that we use pt3dremove(it+1)
					}else{
						m2=(dend.at(dend_num).at(it).r-dend.at(dend_num).at(it+2).r)/(1e-10);
					}
					if(m1>m2){
						dend[dend_num].erase(dend[dend_num].begin()+it-1);
					}else{						
						dend[dend_num].erase(dend[dend_num].begin()+it);					
					}
				}else{
					if(it==0){
						dend[dend_num].erase(dend[dend_num].begin()+it); // we cannot change the first point
					}else{
						dend[dend_num].erase(dend[dend_num].begin()+it-1); // we cannot change the last point
					}
				}
				it = -1;
			}
		it = it +1;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//	define ARC and DIAM to use in plots
/////////////////////////////////////////////////////////////////////////////////////////
void create_vector_data(std::vector< std::vector<struct RawPoints > > &dend , bool &preprocessing_flag, unsigned dend_num,std::vector<double> &ARC,std::vector<double> &DIAM){


	if(preprocessing_flag == 0){
		remove_points_with_same_coordinates(dend,preprocessing_flag,dend_num);
	}
 
	ARC.clear();
	DIAM.clear();
    for(unsigned it=0;it<dend.at(dend_num).size();it++){
		ARC.push_back(dend.at(dend_num).at(it).arc); 
		DIAM.push_back(dend.at(dend_num).at(it).r);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//	deriv_dend
/////////////////////////////////////////////////////////////////////////////////////////
void deriv_dend(std::vector<double> &dydx, std::vector< std::vector<struct RawPoints > > &dend ,
                bool &preprocessing_flag, unsigned dend_num,std::vector<double> &ARC,std::vector<double> &DIAM){

    create_vector_data(dend , preprocessing_flag,dend_num,ARC,DIAM);
    dydx.clear();

	if(preprocessing_flag == 0){
		remove_points_with_same_coordinates(dend,preprocessing_flag,dend_num);
	}
	
	

    for (unsigned it=0; it<dend.at(dend_num).size()-1; it++){
		if ((dend.at(dend_num).at(it+1).arc-dend.at(dend_num).at(it).arc)!=0){
            dydx.push_back(((dend.at(dend_num).at(it+1).r-dend.at(dend_num).at(it).r)/(dend.at(dend_num).at(it+1).arc-dend.at(dend_num).at(it).arc)));

        }else{ //it was not necessary in any of the tests, but better safe than sorry
			double signal = fabs(dend.at(dend_num).at(it+1).r-dend.at(dend_num).at(it).r)/(dend.at(dend_num).at(it+1).r-dend.at(dend_num).at(it).r);
			dydx.push_back(1000*(signal)); //prevent infinite derivatives
		}
	}
	dydx.push_back(0);	// Towards facilitating the code below it should be associate
			// with each 3D-point a slope. Since this is the last point of 
			// a section (which should be equal to the first point of the 
            // section son) we have decided to assign zero to its slope.
//    for (unsigned i=0;i<dend[dend_num].size();i++){
//            printf("dend[%d][%d] = [ %f, %f, %f, %f, %f ]\n",dend_num, i,dend[dend_num][i].x,dend[dend_num][i].y,dend[dend_num][i].z,dend[dend_num][i].r,dend[dend_num][i].arc);
//    }
//    printf("\n\n");
    ////getchar()
//    for(unsigned it =0;it<dydx.size();it++){
//        printf("dydx(%d) = %f\n",it,dydx.at(it));
//    }
}




/////////////////////////////////////////////////////////////////////////////////////////
//	define ppslope, pnslope, pzslope
/////////////////////////////////////////////////////////////////////////////////////////
void ups_and_downs(std::vector<double> &dydx, std::vector< std::vector<struct RawPoints > > &dend ,
                   bool &preprocessing_flag, unsigned dend_num,std::vector<long> &ppslope,
                   std::vector<long> & pnslope, std::vector<long> &pzslope,std::vector<double> &ARC,std::vector<double> &DIAM){
//    printf("\t*\tarrancou ups_and_downs\n\n");
//    printf("\t*\tcomecou deriv_dend\n\n");

    deriv_dend(dydx, dend ,preprocessing_flag, dend_num,ARC,DIAM);
//    printf("\t*\tacabou deriv_dend\n\n");

//    printf("paused inside ups_and_downs\n");
    //getchar()
//    printf("\t*\tdydx.size() = %d\n\n",dydx.size());

//    for(unsigned i=0;i<dend.at(dend_num).size();i++){
//        printf("%f %f %f %f\n",dend.at(dend_num).at(i).x,dend.at(dend_num).at(i).y,dend.at(dend_num).at(i).z,dend.at(dend_num).at(i).r);
//    }
    pnslope.clear();
    ppslope.clear();
    pzslope.clear();
    for(unsigned it =0;it<dydx.size();it++){
//        printf("*\tdydx[%d] = %f\n",it,dydx.at(it));
		if(dydx.at(it)<0){
			pnslope.push_back(it);		
		}else{
			if(dydx.at(it)>0){
				ppslope.push_back(it);
			}else{
				pzslope.push_back(it);
			}
		}
	}
}




/////////////////////////////////////////////////////////////////////////////////////////
// Locate and eliminate bumps 
/////////////////////////////////////////////////////////////////////////////////////////

// find all points (at right) within a radius of max_dist micrometers
long find_nearby_points(long point, double max_dist, std::vector< std::vector<struct RawPoints > > &dend ,
                        unsigned dend_num,std::vector<double> &ARC,std::vector<double> &DIAM){


	if(max_dist>=(ARC.at(ARC.size()-1)-ARC.at(point))){ //all points at right of point should be considered
        return  (long)ARC.size()-1;
	}else{
		long pt = point;
		while ((ARC.at(pt)-ARC.at(point))<max_dist){
            pt++;
		}
		pt = pt-1;
        return pt;
	}
}

void smooth(std::vector<double> dydx, std::vector< std::vector<struct RawPoints > > &dend ,
            bool &preprocessing_flag, unsigned dend_num,std::vector<long> &ppslope,
            std::vector<long> & pnslope, std::vector<long> &pzslope,
            std::vector<double> &ARC,std::vector<double> &DIAM, long x1, long x2){


//    printf("\n\t*\tSMOOTHING between %d and %d\n\n",x1,x2);
    double delta_x;
    delta_x = ARC.at(x2) - ARC.at(x1);
    if(abs(delta_x)<0.001){
        delta_x = 0.001;
    }
    double delta_y = DIAM.at(x2) - DIAM.at(x1);

    double m = delta_y / delta_x; // m = slope between x1 and x2

    for (unsigned it = x1;it<1+x2;it++){
        double y = m*(ARC.at(it)-ARC.at(x1))+DIAM.at(x1);
//        printf("DEND[%d][%d].r changed from %.2f to",dend_num,it,dend.at(dend_num).at(it).r);
        dend.at(dend_num).at(it).r= y;
//        printf(" %.2f\n",dend.at(dend_num).at(it).r);
    }
    //printf( "UNIU OS PONTOS DE ABCISSA %.3f E %.3f\n",ARC.x[x1],ARC.x[x2])
    ups_and_downs(dydx, dend, preprocessing_flag, dend_num, ppslope, pnslope, pzslope,ARC,DIAM);
    //printf( ".....................DADOS ACTUALIZADOS.....................\n")
}








void locate_and_smooth_bumps(std::vector<double> &dydx, std::vector< std::vector<struct RawPoints > > &dend ,
                             bool &preprocessing_flag, unsigned dend_num,std::vector<long> &ppslope,
                             std::vector<long> & pnslope, std::vector<long> &pzslope, std::vector<double> &ARC,
                             std::vector<double> &DIAM, double &thresh,double &max_dist,double &step_min){

//    printf("\t*\tlocate_and_smooth_bumps\t--->\tStarted\n\n");
    ups_and_downs(dydx, dend, preprocessing_flag, dend_num, ppslope, pnslope, pzslope,ARC,DIAM);
//    printf("\t*\tprimeiro ups_and_downs\n\n");

    bool scan_flag = 1;
    // This variable indicates whether the "while" loop  needs to perform, or not, a new search for artifacts.
    double dydxMAX = maximo(dydx);
    unsigned long x1;
    if(dydxMAX>=thresh){
                x1 = indexEqualOrGreaterThan(dydx,thresh);
                /*printf*/( "x1=%d detected at dend[%d]\n",x1,dend_num);
                //getchar()
    }else{
//        printf( "threshold not trespassed\n");
        scan_flag = 0;
    }

    //printf("x1 = %d\nx2 = %d";x1,x2);


    while(scan_flag == 1){

        long x2;
        x2 = find_nearby_points(x1, max_dist, dend ,dend_num,ARC,DIAM);

//        printf( "x2 = %d\n",x2);
        //getchar()
//        printf("test_ascend = %d\n",test_ascend(DIAM,step_min,x1,x2));

        if( test_ascend(DIAM,step_min,x1,x2) == 0 ){
            // Despite the fast rise the rising step is to short to be accounted

            for(int it = 0;it<x1+1;it++){
                    dydx.at(it)=0;
            }	//temporarilly deletes information about x1
                // in order to move forward

            dydxMAX = maximo(dydx);//printf("dydx.max() = %.2f\n",dydxMAX);
            if(dydxMAX>=thresh){
                x1 = indexEqualOrGreaterThan(dydx,thresh);
//                printf("test_ascend == 0.... new x1 needed\n");
//                printf("new x1 = %d\n",x1);
                //reset information deleted in the previous step
                ups_and_downs(dydx, dend, preprocessing_flag, dend_num, ppslope, pnslope, pzslope,ARC,DIAM);
            }else{
                scan_flag = 0; //the proc reaches it end
                //print "there is no new candidate to be x1"
                //print "proc should end here"
            }
        }else{
            long x3 = test_descend(DIAM,pnslope,dydx,thresh,step_min,x1,x2);
//            printf("test_descend = %d\n",x3);
            //getchar()
            //print "test_descend = ", test_descend(x1,x2,step_min,thresh,DIAM,dydx,pnslope)
            if(x3!=-1){
                //print "move_down_right"
                x3 = move_down_right(x2,x3,thresh,DIAM,ARC,pnslope);
                //print "move_down_left"
                x1 = move_down_left(x1,x3,thresh,max_dist,DIAM,ARC,ppslope);
                //print "smoothing"


                smooth(dydx,dend ,preprocessing_flag,dend_num,ppslope,pnslope, pzslope,ARC,DIAM, x1,x3);
                ups_and_downs(dydx, dend, preprocessing_flag, dend_num, ppslope, pnslope, pzslope,ARC,DIAM);
                //print "updating data"
                dydxMAX = maximo(dydx);
                if(dydxMAX>=thresh){
                    //print "look for new x1"
                    x1 = indexEqualOrGreaterThan(dydx,thresh);
//                    printf("x1 = %d\n",x1);
                    scan_flag = 1;
                }else{
                    scan_flag = 0; //the proc reaches it end
                    //print "there is no new candidate to be x1"
                    //print "proc should end here"
                }
            }else{
                for(long it = 0;it<x1+1;it++){
                    dydx.at(it)=0;
                }
                dydxMAX = maximo(dydx);
                if(dydxMAX>=thresh){
                    x1 = indexEqualOrGreaterThan(dydx,thresh);
                    //print "test_descend == -1.... new x1 needed"
//                    printf("new x1 = %d\n", x1);
                    ups_and_downs(dydx, dend, preprocessing_flag, dend_num, ppslope, pnslope, pzslope,ARC,DIAM);//reset information deleted in the previous step
                }else{
                    scan_flag = 0; //the proc reaches it end
                    //print "there is no new candidate to be x1"
                    //print "proc should end here"
                }
            }
        }
    }
}



