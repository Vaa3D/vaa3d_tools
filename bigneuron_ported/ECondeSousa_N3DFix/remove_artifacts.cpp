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

/////////////////////////////////////////////////////////////////////////////////////////
//	remove_points_with_same_coordinates
/////////////////////////////////////////////////////////////////////////////////////////
// Some points have the same 3d coordinates. This will cause problems during the rest of 
// the process of smoothing. The following proc will search for these bugs and fix them. 
void remove_points_with_same_coordinates(std::vector< std::vector<struct RawPoints > > &dend , bool &preprocessing_flag, unsigned dend_num){

	preprocessing_flag = 1;
	//printf("preprocessing_flag = %d\n", preprocessing_flag);


    std::vector<int> vec(dend.at(dend_num).size() , 0);
    for (unsigned it=0; it<dend.at(dend_num).size()-1; it++){// all points except for the last one
                if (dend.at(dend_num).at(it+1).x==dend.at(dend_num).at(it).x &&
                        dend.at(dend_num).at(it+1).y==dend.at(dend_num).at(it).y &&
                        dend.at(dend_num).at(it+1).z==dend.at(dend_num).at(it).z){
                    vec.at(it) = 1;
                }
            }
    // at this point we have a vector filled with ones in every point that has the same coordinates of its descendant

    double norm;
    double new_x;
    double new_y;
    double new_z;
    double dist;

    if ( vec.at( vec.size()-2 )==1 ){// (at least) last two points have equal coordinates
    // this scenario has to be treated separatly because the last point cannot be pushed forward
        int n_move=0; // number of points to move in this block
        int it=dend.at(dend_num).size()-2;
        while (vec.at(it)==1){
            n_move = n_move+1;
            it=it-1;
        }
        if (n_move >= dend.at(dend_num).size()-1){
            printf("ERROR: ALL POINTS IN SECTION dend[%d] HAVE THE SAME COORDINATES\n",dend_num);
        }else{
            it = dend.at(dend_num).size()-1-n_move; // first point with equal coordinates
            while (n_move > 0){
                    norm = sqrt( (dend.at(dend_num).at(it-1).x - dend.at(dend_num).at(it).x) * (dend.at(dend_num).at(it-1).x - dend.at(dend_num).at(it).x) +
                                 (dend.at(dend_num).at(it-1).y - dend.at(dend_num).at(it).y) * (dend.at(dend_num).at(it-1).y - dend.at(dend_num).at(it).y) +
                                 (dend.at(dend_num).at(it-1).z - dend.at(dend_num).at(it).z) * (dend.at(dend_num).at(it-1).z - dend.at(dend_num).at(it).z) );
                    // norm = distance bewteen pt3d(it) and pt3d(it-1)
                    if (norm == 0){//security proceeding
                        printf( "ERROR: n_move miscalculated in section dend[%d]\n",dend_num);
                    }

                    if( norm > 1 ){
                        dist = 0.1;
                    }else{
                        dist= 0.1*norm;
                    } //the idea is to move a point the minimum between 0.1 microns or 10% the distance between sucessive points

                    new_x = dend.at(dend_num).at(it).x + ( (dend.at(dend_num).at(it-1).x - dend.at(dend_num).at(it).x )/norm ) * dist;
                    new_y = dend.at(dend_num).at(it).y + ( (dend.at(dend_num).at(it-1).y - dend.at(dend_num).at(it).y )/norm ) * dist;
                    new_z = dend.at(dend_num).at(it).z + ( (dend.at(dend_num).at(it-1).z - dend.at(dend_num).at(it).z )/norm ) * dist;


                    dend.at(dend_num).at(it).x = new_x;
                    dend.at(dend_num).at(it).y = new_y;
                    dend.at(dend_num).at(it).z = new_z;
                    vec.at(it) = 0; // update information
                    it = it + 1;
                    n_move = n_move - 1;
            }
        }
    }
    // if the last and penultimate points have different coordinates
    int sum_of_elems =0;
    for(unsigned i=0;i<vec.size();i++){
        sum_of_elems = sum_of_elems + vec.at(i);
    }
    int it;
    while (sum_of_elems>0) { // while there are points to fix
        it=dend.at(dend_num).size()-1;
        while (vec.at(it)==0){
        it=it-1;
        }
        it = it + 1;
        //find the last point to fix
        // The last 1 means that that point and the following have the same coordinates
        norm = sqrt( (dend.at(dend_num).at(it+1).x - dend.at(dend_num).at(it).x) * (dend.at(dend_num).at(it+1).x - dend.at(dend_num).at(it).x) +
                     (dend.at(dend_num).at(it+1).y - dend.at(dend_num).at(it).y) * (dend.at(dend_num).at(it+1).y - dend.at(dend_num).at(it).y) +
                     (dend.at(dend_num).at(it+1).z - dend.at(dend_num).at(it).z) * (dend.at(dend_num).at(it+1).z - dend.at(dend_num).at(it).z) );
        if (norm == 0){//security proceeding
            printf( "ERROR: n_move miscalculated in section dend[%d]\n",dend_num);
        }
        if( norm > 1 ){
            dist = 0.1;
        }else{
            dist= 0.1*norm;
        } //the idea is to move a point the minimum between 0.1 microns or 10% the distance between sucessive points

        new_x = dend.at(dend_num).at(it).x + ( (dend.at(dend_num).at(it+1).x - dend.at(dend_num).at(it).x )/norm ) * dist;
        new_y = dend.at(dend_num).at(it).y + ( (dend.at(dend_num).at(it+1).y - dend.at(dend_num).at(it).y )/norm ) * dist;
        new_z = dend.at(dend_num).at(it).z + ( (dend.at(dend_num).at(it+1).z - dend.at(dend_num).at(it).z )/norm ) * dist;

        dend.at(dend_num).at(it).x = new_x;
        dend.at(dend_num).at(it).y = new_y;
        dend.at(dend_num).at(it).z = new_z;

        vec.at(it-1) = 0; // update information
        sum_of_elems = sum_of_elems - 1;
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
    float M_shape_th = 0.95;
    bool flag_M;
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


                // M_Shape
                // M_Shape
                // M_Shape
                // M_Shape
                // M_Shape

                //print secname()
                long x1_tmp = x1;
                long x3_tmp = x3;
                if(DIAM.at(x3_tmp)/DIAM.at(x1)<M_shape_th){
                    flag_M = 1;
                }else{
                    flag_M = 0;
                }
                //printf("flag_M = %d\n",flag_M);

                while(flag_M == 1){ //if the swelling ends at an lower Diam then it may have been a double swelling

                    // searching for a neighbor artifact
                    x2 = find_nearby_points( x3, max_dist, dend ,dend_num,ARC,DIAM);
                    for(int it = 0; it < x3 +1; it++){
                        dydx.at(it)=0;
                    }
                    for(int it = x2; it < dend.at(dend_num).size()+1-1; it++){
                        dydx.at(it)=0;
                    }
                    dydxMAX = maximo(dydx);
                    if(dydxMAX>=thresh){
                        x1 = indexEqualOrGreaterThan(dydx,thresh);
                        //print "new ARC(x1)=", ARC.x(x1)
                        x2 = find_nearby_points( x3, max_dist, dend ,dend_num,ARC,DIAM);
                        //print "(Update)find M shape between ARC(x)=",ARC.x(x1) , " and ARC(x)=", ARC.x(x2)
                        ups_and_downs(dydx, dend, preprocessing_flag, dend_num, ppslope, pnslope, pzslope,ARC,DIAM);
                        //reset information deleted in the previous step
                        if( test_ascend(DIAM,step_min,x1,x2) == 1 ){
                            x3 = test_descend(DIAM,pnslope,dydx,thresh,step_min,x1,x2);
                            //print "x3 = ", x3
                            if(x3!=-1){
                                x3 = move_down_right(x2,x3,thresh,DIAM,ARC,pnslope);
                                x1 = move_down_left(x1,x3,thresh,max_dist,DIAM,ARC,ppslope);

                                if(DIAM.at(x3_tmp)/DIAM.at(x3)<M_shape_th){
                                    x3_tmp = x3;
                                    flag_M = 1;
                                }else{
                                    flag_M = 0;
                                }
                            }else{
                                flag_M=0;
                            }
                        }else{
                            flag_M = 0;
                        }
                    }else{
                        ups_and_downs(dydx, dend, preprocessing_flag, dend_num, ppslope, pnslope, pzslope,ARC,DIAM);
                        //reset information deleted in the previous step
                        flag_M = 0;
                    }
                }
                // printf("flag_M = %d\n",flag_M);

                // M_Shape
                // M_Shape
                // M_Shape
                // M_Shape
                // M_Shape
                // M_Shape
                // M_Shape
                // M_Shape

                smooth(dydx,dend ,preprocessing_flag,dend_num,ppslope,pnslope, pzslope,ARC,DIAM, x1_tmp,x3_tmp);
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



