#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include "Dend_Section.h"
#include "myHeader.h"
#include "N3Dfix_plugin.h"

	
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

void find_nodes(std::vector<long> &ppid, std::vector<long> &nodes,std::vector<long> &endpoints,
                std::vector<long> &end_sec){

    std::vector<long> tmp_vector (ppid.size(), 0);
    long ind;

    for (unsigned it = 0;it<ppid.size(); it++){
        ind = ppid.at(it);
        if (ind >0){
            tmp_vector.at(ind-1)=tmp_vector.at(ind-1)+1;
        }
    }

    for (unsigned it = 0;it<tmp_vector.size(); it++){
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


void create_tree(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z, std::vector<float> &r,
                 std::vector<long> &ppid, std::vector<long> &end_sec, struct RawPoints &Point, std::vector<struct RawPoints > &n3d,
                 std::vector< std::vector<struct RawPoints > > &dend , std::vector<long> &pid, std::vector<long> &tree_id){

    if (end_sec.size() > 1){ //not sure if it's necessary

        for(int i=0;i<end_sec.at(0);i++){
            Point.pid = pid.at(i);
            Point.tid = tree_id.at(i);
            Point.x = x.at(i);
            Point.y = y.at(i);
            Point.z = z.at(i);
            Point.r = r.at(i);
            Point.ppid=ppid.at(i);
            n3d.push_back(Point);
        }

        dend.push_back(n3d);

        for (unsigned j=0;j<end_sec.size()-1;j++){
            n3d.clear();
            int i = ppid.at(end_sec.at(j));
            Point.pid = pid.at(i);
            Point.tid = tree_id.at(i);
            Point.x = x.at(i);
            Point.y = y.at(i);
            Point.z = z.at(i);
            Point.r = r.at(i);
            Point.ppid=ppid.at(i);
            n3d.push_back(Point);
            for (int i = end_sec.at(j);i<end_sec.at(j+1); i++ ){

                Point.pid = pid.at(i);
            Point.tid = tree_id.at(i);
                Point.x = x.at(i);
                Point.y = y.at(i);
                Point.z = z.at(i);
                Point.r = r.at(i);
                Point.ppid=ppid.at(i);
                n3d.push_back(Point);
            }
            n3d.at(0).r = n3d.at(1).r;
            dend.push_back(n3d);
        }





        for (unsigned ii =0; ii< dend.size(); ii++){
            dend[ii][0].arc = 0;
            if(dend[ii].size()>0){
                for (unsigned jj=1; jj<dend[ii].size(); jj++){
                    dend[ii][jj].arc = dend[ii][jj-1].arc  + p3Ddist( dend[ii][jj-1].x, dend[ii][jj-1].y, dend[ii][jj-1].z, dend[ii][jj].x, dend[ii][jj].y, dend[ii][jj].z);
                }
            }
        }

        //        for (unsigned ii =0; ii< dend.size(); ii++){
        //            for (unsigned i=0;i<dend[ii].size();i++){
        //                printf("dend[%d][%d] = [ %f, %f, %f, %f, %f ]\n",ii, i,dend[ii][i].x,dend[ii][i].y,dend[ii][i].z,dend[ii][i].r,dend[ii][i].arc);
        //            }
        //            printf("\n\n");
        //        }
        ////getchar()






        //std::vector<struct RawPoints > n3d[2];
        //n3d[0].push_back(Point[0]);
        //n3d[0].push_back(Point[1]);

        //n3d[1].push_back(Point[0]);
        //n3d[1].push_back(Point[1]);
        //n3d[1].push_back(Point[2]);
        //n3d[1].push_back(Point[3]);
        //n3d[1].push_back(Point[4]);
        //n3d[1].push_back(Point[5]);

        //	std::vector< std::vector<struct RawPoints > > dend;

        //dend.push_back(n3d[0]);
        //dend.push_back(n3d[1]);


        //for (unsigned ii =0; ii< dend.size(); ii++){
        //	dend[ii][0].arc = 0;
        //	if(dend[ii].size()>0){
        //		for (unsigned jj=1; jj<dend[ii].size(); jj++){
        //			dend[ii][jj].arc = dend[ii][jj-1].arc  + p3Ddist( dend[ii][jj-1].x, dend[ii][jj-1].y, dend[ii][jj-1].z, dend[ii][jj].x, dend[ii][jj].y, dend[ii][jj].z);
        //		}
        //	}
        //}
    }else{
        std::cerr << "impossible to find nodes" << std::endl;
    }
}
