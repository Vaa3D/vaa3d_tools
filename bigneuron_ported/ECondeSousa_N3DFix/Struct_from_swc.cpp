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

#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include "Dend_Section.h"
#include "myHeader.h"
#include "N3DFix_plugin.h"

	
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// START
// auxiliar functions
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
float p3Ddist(float x1, float y1, float z1, float x2, float y2, float z2){
	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;
	float s = dx*dx + dy*dy + dz*dz;
	float sq = sqrt(s);
	return sq;
}




bool my_compare( float a, float b){
	return a < b;
}


void diff(std::vector<double> &my_vec){ //not tested

	printf("my_vec = [  ");
	for (unsigned it=0; it<my_vec.size(); it++){	
		printf("%d  ",my_vec.at(it));
	}
	printf("]\n\n\n");

	std::vector<double> tmp_vec;
	for(unsigned it=1; it<my_vec.size(); it++){
		tmp_vec.push_back(my_vec.at(it)-my_vec.at(it-1));
	}

	printf("tmp_vec = [  ");
	for (unsigned it=0; it<tmp_vec.size(); it++){	
		printf("%d  ",tmp_vec.at(it));
	}
	printf("]\n\n\n");
}
	
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// END
// auxiliar functions
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////







	
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// START
// Principal functions
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void find_nodes(std::vector<long> &tree_id, std::vector<long> &ppid, std::vector<long> &nodes, std::vector<long> &endpoints,
                std::vector<long> &end_sec,  std::vector<long> &soma){

    std::vector<long> tmp_vector (ppid.size(), 0);
    long ind;

    for (unsigned it = 0;it<ppid.size(); it++){
        ind = ppid.at(it);
        if (ind >0){
            tmp_vector.at(ind-1)=tmp_vector.at(ind-1)+1;
        }
    }

    for (unsigned it = 0;it<tmp_vector.size(); it++){
        if (tree_id.at(it) == 1){
            soma.push_back(it+1);
        }else{
            if (tmp_vector.at(it) > 1){
                nodes.push_back(it+1);
                end_sec.push_back(it+1);
            }else{
                if (tmp_vector.at(it) == 0){
                    endpoints.push_back(it+1);
                    end_sec.push_back(it+1);
                }
            }
        }
    }
//    // PRINT SOMA

//    printf("soma = \n");
//    for (unsigned it = 0;it<soma.size(); it++){
//            printf("%d\n",it+1 , soma.at(it));
//    }

//    // PRINT END_SEC

//    printf("end_sec = \n");
//    for (unsigned it = 0;it<end_sec.size(); it++){
//            printf("%d %d\n",it+1 , end_sec.at(it));
//    }

//    // PRINT TMP_VECTOR

//    printf("tmp_vec = \n");
//    for (unsigned it = 0;it<tmp_vector.size(); it++){
//            printf("%d %d\n",it+1 , tmp_vector.at(it));
//    }

}


void create_tree(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z, std::vector<float> &r,
                 std::vector<long> &ppid, std::vector<long> &end_sec, struct RawPoints &Point, std::vector<struct RawPoints > &n3d,
                 std::vector< std::vector<struct RawPoints > > &dend , std::vector<long> &pid, std::vector<long> &tree_id,
                 std::vector<long> &soma){

//    printf("\n\n###################################################################\n");
//    printf("########################## creating tree ##########################\n");
//    printf("###################################################################\n\n\n\n");


    std::vector<long> tmp_vector (ppid.size(), 0);
    // mark points that belong to soma or that are end of sections



    //std::cout << "soma.size() = " << soma.size() << std::endl<< std::endl<< std::endl;
    if(soma.size() > 0){
        for(unsigned ii = 0; ii < soma.size(); ii++){
            tmp_vector.at(soma.at(ii)-1) = 1;
            Point.pid = pid.at(soma.at(ii)-1);
            Point.tid = tree_id.at(soma.at(ii)-1);
            Point.x = x.at(soma.at(ii)-1);
            Point.y = y.at(soma.at(ii)-1);
            Point.z = z.at(soma.at(ii)-1);
            Point.r = r.at(soma.at(ii)-1);
            Point.ppid=ppid.at(soma.at(ii)-1);
            n3d.push_back(Point);
        }
        dend.push_back(n3d);
//        for (unsigned ii =dend.size()-1; ii< dend.size(); ii++){
//            for (unsigned i=0;i<dend[ii].size();i++){
//                printf("dend[%d][%d] = [ %d, %d, %f, %f, %f, %f, %d, %f ]\n",ii, i,dend[ii][i].pid,dend[ii][i].tid,dend[ii][i].x,dend[ii][i].y,dend[ii][i].z,dend[ii][i].r,dend[ii][i].ppid,dend[ii][i].arc);
//            }
//            printf("\n\n");
//        }
    }


    long pt;
    std::vector<long> tmp_vector_id;

    if (end_sec.size() > 1){ //not sure if it's necessary

        for(unsigned ii = 0; ii < end_sec.size(); ii++){
            tmp_vector.at(end_sec.at(ii)-1)=1;
        }
        pt=0;

        while(ppid.at(pt) != -1){
            pt = pt + 1;
        }
        tmp_vector.at(pt)=1;


        for (unsigned j=0;j<end_sec.size();j++){
            tmp_vector_id.clear();
            n3d.clear();
            pt = end_sec.at(j);
            if (ppid.at(pt-1) != -1){
//                std::cout << "last point of section = " << pt << std::endl<< std::endl<< std::endl;
                tmp_vector.at(pt-1) = 0;
                while (tmp_vector.at(pt-1) == 0) {
                    tmp_vector.at(pt-1) = 1;
                    tmp_vector_id.push_back(pt);
                    pt = ppid.at(pt-1);
//                    std::cout << "pt = " << pt << std::endl<< std::endl;
                    if (pt == -1){
                        pt = tmp_vector_id.at(tmp_vector_id.size()-1);
                    }
                }
//                std::cout << "exit" << std::endl;

//                printf("tmp_vec_id = \n");
//                for (unsigned it = 0;it<tmp_vector_id.size(); it++){
//                    printf("%d %d\n",it+1 , tmp_vector_id.at(it));
//                }


                //find and append the last point of parent dend
                pt = ppid.at(tmp_vector_id.at(tmp_vector_id.size()-1) - 1)-1;
                if(pt>=0){
                    Point.pid = pid.at(pt);
                    Point.tid = tree_id.at(pt);
                    Point.x = x.at(pt);
                    Point.y = y.at(pt);
                    Point.z = z.at(pt);
                    Point.r = r.at(pt);
                    Point.ppid=ppid.at(pt);
                    n3d.push_back(Point);
                }
                //append the remainig points
                for (unsigned ii = 0; ii < tmp_vector_id.size(); ii++){
                    pt = tmp_vector_id.at(tmp_vector_id.size()-1-ii)-1;
                    Point.pid = pid.at(pt);
                    Point.tid = tree_id.at(pt);
                    Point.x = x.at(pt);
                    Point.y = y.at(pt);
                    Point.z = z.at(pt);
                    Point.r = r.at(pt);
                    Point.ppid=ppid.at(pt);
                    n3d.push_back(Point);
                }
                n3d.at(0).r = n3d.at(1).r;
                dend.push_back(n3d);
//                for (unsigned ii = dend.size()-1; ii< dend.size(); ii++){
//                    for (unsigned i=0;i<dend[ii].size();i++){
//                        printf("dend[%d][%d] = [ %d, %d, %f, %f, %f, %f, %d, %f ]\n",ii, i,dend[ii][i].pid,dend[ii][i].tid,dend[ii][i].x,dend[ii][i].y,dend[ii][i].z,dend[ii][i].r,dend[ii][i].ppid,dend[ii][i].arc);
//                    }
//                    printf("\n\n");
//                }
            }
        }


        for (unsigned ii =0; ii< dend.size(); ii++){
            dend[ii][0].arc = 0;
            if(dend[ii].size()>0){
                for (unsigned jj=1; jj<dend[ii].size(); jj++){
                    dend[ii][jj].arc = dend[ii][jj-1].arc  + p3Ddist( dend[ii][jj-1].x, dend[ii][jj-1].y, dend[ii][jj-1].z, dend[ii][jj].x, dend[ii][jj].y, dend[ii][jj].z);
                }
            }
        }


//                for (unsigned ii =0; ii< dend.size(); ii++){
//                    for (unsigned i=0;i<dend[ii].size();i++){
//                        printf("dend[%d][%d] = [ %d, %d, %f, %f, %f, %f, %d, %f ]\n",ii, i,dend[ii][i].pid,dend[ii][i].tid,dend[ii][i].x,dend[ii][i].y,dend[ii][i].z,dend[ii][i].r,dend[ii][i].ppid,dend[ii][i].arc);
//                    }
//                    printf("\n\n");
//                }


    }else{
        std::cerr << "impossible to find nodes" << std::endl;
    }
}
