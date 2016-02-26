//consensus_skeleton.cpp
//generate consensus tree from multiple neurons

//(1) use a volume image to recode and accumulate the locations of nodes for all trees
//(2) keep one node for each small window, record its confidence value as a feature value (eswc) by the votes
//(3) final result is a minimum spanning tree from 2)


#include "consensus_skeleton.h"
//#include "kcentroid_cluster.h"
#include "mst_dij.h"
#include "mst_prim.h"
#include <QtGlobal>
#include <iostream>
#include <climits>
#include "basic_4dimage.h"
#include "algorithm"
#include <string.h>
#include <sort_eswc.h>
#include <cmath>
using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
template <class T> T pow2(T a)
{
	return a*a;

}


#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif


struct NeuronSize{
	float x;
	float y;
	float z;
};



unsigned int v_min(vector<unsigned int> x)
{
	sort(x.begin(), x.end());
	return  x[0];
}

unsigned int v_max(vector<unsigned int> x)
{
	sort(x.begin(), x.end());
	return  x[x.size()-1];
}

int max_image_value( unsigned char *  img1d, V3DLONG siz)
{
	int max_v = 0;
	for(int i = 0; i < siz; i++ )
	{
		if (int(img1d[i]) > max_v)
			max_v = img1d[i];
	}
	return max_v;
}

double  mean_image_value( unsigned char *  img1d, V3DLONG siz)
{
	double sum_v= 0;
	int count = 0 ;
	for(int i = 0; i < siz; i++ )
	{
		if (int(img1d[i]) > 1){
			sum_v += img1d[i];
			count++;
		}
	}
	return sum_v/count;
}

double median(vector<double> x)
{
	sort(x.begin(), x.end());
	return  x[x.size()/2];
}

#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

double  computeTotalLength(const NeuronTree & nt)
{
    double Length = 0;
    QList<NeuronSWC> list = nt.listNeuron;


    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);

        int parent = getParent(i,nt);
        if (parent==VOID) continue;
        double l = dist(curr,list.at(parent));
        Length += l;

    }
    return Length;

}

void remove_outliers(vector<NeuronTree> & nt_list ,double &median_root_x, double &median_root_y, double  &median_root_z)
{  //validate each tree, only keep the first tree
	/* 
	   for(int i = 0; i < nt_list.size(); i++){
	   NeuronTree tree = nt_list[i];
	   QList <NeuronSWC>  ln= tree.listNeuron;
	   for(int ii = 1;ii <ln;ii++){
	   if (ln[ii].pn == -1){
	   } 
	   }
	   }
	   */
    //remove statistically outliers
    // use total length

	cout<<"\nOutlier detection:"<<endl;
    vector<double> valid_nt_sizes;
    vector<double> nt_sizes;

	for(int i = 0; i < nt_list.size(); i++){
		NeuronTree tree = nt_list[i];

        double len = computeTotalLength(tree);
        nt_sizes.push_back(len);
        if (len > 10 && len <50000){
            valid_nt_sizes.push_back(len);
		}
	}

    V3DLONG median_size = median(valid_nt_sizes);

	vector<V3DLONG > rm_ids;
	cout <<"Median node size (exclude those num_nodes <10 or >50000 ) = " << median_size <<endl;
	cout <<"Detecting SWCs have nodes > 3*Median_size or nodes < Median_size/3:"<<endl;
	for(int i = 0; i < nt_list.size(); i++){
        double len = nt_sizes[i];
        if ( len > 3*median_size  ||  len < double(median_size)/3 || len > 50000 )
		{
            cout <<"Remove neuron "<< i<<":"<< nt_list[i].file.toStdString().c_str() << " with "<<  len<< " in total length"<<endl;
			rm_ids.push_back(i);

		}
        if (len == median_size)
		{  cout <<"Will use the median case ("<<i<<") soma location as the soma of the resulting consensus."<<endl;
			NeuronSWC root = nt_list[i].listNeuron.at(0);
			median_root_x = root.x;
			median_root_y = root.y;
			median_root_z = root.z;
		}
	}

	for (int i =0; i< rm_ids.size();i++){
		nt_list.erase(nt_list.begin()+rm_ids[i]);
	}

	cout<< nt_list.size()<< " neurons left are going to be included for consensus."<<endl;
}

struct MyBoundingBox{
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
};


MyBoundingBox neuron_trees_bb(vector<NeuronTree> nt_list)
{

	MyBoundingBox bb ={0,0,0,0,0,0};

	bb.min_x = LONG_MAX;
	bb.min_y = LONG_MAX;
	bb.min_z = LONG_MAX;
	bb.max_x = 0;
	bb.max_y = 0;
	bb.max_z = 0;
	for (int j = 0; j < nt_list.size(); j++){
		NeuronTree nt = nt_list[j];
		for (int i = 0; i < nt.listNeuron.size(); i++)
		{
			NeuronSWC a = nt.listNeuron.at(i);
			bb.min_x = MIN(a.x - a.r,bb.min_x );
			bb.max_x = MAX(a.x + a.r,bb.max_x );
			bb.min_y = MIN(a.y - a.r,bb.min_y );
			bb.max_y = MAX(a.y + a.r,bb.max_y );
			bb.min_z = MIN(a.z - a.r,bb.min_z );
			bb.max_z = MAX(a.z + a.r,bb.max_z );
		}
	}
	return bb;
}


void non_max_suppresion( unsigned char * img1d, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z, double threshold_votes, Point3D offset,
		vector<Point3D> &node_list,  vector<unsigned int> &vote_list,unsigned int win_size)
{ // extract the local maximum voted skelenton node locations : node_list
	// the corresponding votes are collected in: vote_list
	V3DLONG num_nodes =0;
    for (V3DLONG id_x = 0 + win_size/2; id_x <  sz_x- win_size/2; id_x++)
        for (V3DLONG id_y = 0 + win_size/2; id_y <  sz_y- win_size/2; id_y++)
            for (V3DLONG id_z = 0 + win_size/2; id_z <  sz_z- win_size/2; id_z++)
			{
				//nn, find the local max value within window size
				unsigned char max_val = 0 ;
                //int pre_dis_sqr = 3 * (win_size/2) *(win_size/2)-1;
				V3DLONG max_idx = 0 ;
                for ( V3DLONG xx = id_x - win_size/2;xx< id_x + win_size/2;xx++)
                    for ( V3DLONG yy = id_y - win_size/2;yy< id_y + win_size/2;yy++)
                        for ( V3DLONG zz = id_z - win_size/2;zz< id_z + win_size/2;zz++)
						{
							V3DLONG idx = zz * (sz_x*sz_y) + yy * sz_x + xx;
                            //int dis_sqr = (xx-id_x)* (xx-id_x)+ (yy-id_y)* (yy-id_y)+ (yy-id_z)* (zz-id_z);
                            //if (dis_sqr <  double(win_size*win_size)/4.0)
                            //{
                                if (img1d[idx] < threshold_votes)
                                {
                                    img1d[idx] =0;
                                }
                                if (img1d[idx] > max_val){
                                    max_val = img1d[idx] ;
                                    max_idx = idx;
                                    //pre_dis_sqr = dis_sqr;
                                }
                            //}

						}


				if ( max_val >= threshold_votes)
				{// found non-zero max that passes the majority votes threshold
					for ( V3DLONG xx =id_x - win_size/2;xx< id_x + win_size/2;xx++)
						for ( V3DLONG yy = id_y - win_size/2;yy< id_y + win_size/2;yy++)
							for ( V3DLONG zz = id_z - win_size/2;zz< id_z + win_size/2;zz++)
							{
								V3DLONG idx = zz * (sz_x*sz_y) + yy * sz_x + xx;
                                //int dis_sqr = (xx-id_x)* (xx-id_x)+ (yy-id_y)* (yy-id_y)+ (yy-id_z)* (zz-id_z);
                                //if (dis_sqr <  double(win_size*win_size)/4.0)
                                //{
                                   img1d[idx] = 0;
                                //}
							}

					img1d[max_idx] = max_val;
				}
			}
	//collect SWC nodes from the local max points
	for (V3DLONG id_x = 0 + win_size/2; id_x <  sz_x- win_size/2; id_x++)
		for (V3DLONG id_y = 0 + win_size/2; id_y <  sz_y- win_size/2; id_y++)
			for (V3DLONG id_z = 0 + win_size/2; id_z <  sz_z- win_size/2; id_z++)
			{
				V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;

				if (img1d[idx] >=threshold_votes)
				{
					num_nodes++;
					Point3D p;
					p.x = id_x+offset.x;
					p.y = id_y+offset.y;
					p.z = id_z+offset.z;
					node_list.push_back(p);
					vote_list.push_back(img1d[idx]);
				}
			}
	return;
}




QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons)
{
	QHash<V3DLONG, V3DLONG> neuron_id_table;
	for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
		neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i);
	return neuron_id_table;
}



void AddToMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,int dialate_radius,
		int imageCount, V3DPluginCallback2 & callback)
{
	NeuronSWC *p_cur = 0;
	double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
	V3DLONG pagesz = sx*sy;
	V3DLONG tol_sz = pagesz*sz;
	for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	{
		V3DLONG i,j,k;
		p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
		xs = p_cur->x;
		ys = p_cur->y;
		zs = p_cur->z;

		//rs = p_cur->r;
		// when radii estimation are not taken into consideration for consensus,
		// ignore radii;
		// here register swc nodes to its nearby 3x3 neighborhood volume in the mask image
		// to be more robust/smooth
		rs = dialate_radius;

		double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

		ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
		ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
		if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

		bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
		bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
		if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

		ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
		ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
		if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

		//mark all voxels close to the swc node(s)
		for (k = ballz0; k <= ballz1; k++){
			for (j = bally0; j <= bally1; j++){
				for (i = ballx0; i <= ballx1; i++){
					V3DLONG ind = (k)*pagesz + (j)*sx + i;
					ind = MIN(ind,tol_sz);
					ind = MAX(ind, 0);
					if (pImMask[ind]<=imageCount) {
						pImMask[ind] +=1;}

				}
			}
		}
	}
}

bool generate_vote_map(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d, V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset)
{
        cout <<"\nGenerate vote map "<<endl;
        long long tol_sz = sz_x* sz_y*sz_z;
        cout << "vote_map image size(memory) = " << tol_sz<<": " <<sz_x<<"x "<<sz_y<<" x"<<sz_z<< endl;


        V3DLONG pagesz = sz_x*sz_y;

        for (int j =0; j < nt_list.size(); j++){
            NeuronTree nt = nt_list[j];

            for (int i =0; i < nt.listNeuron.size(); i++)
            {
                NeuronSWC node = nt.listNeuron.at(i);
                double xs=node.x-offset.x;
                double ys=node.y-offset.y;
                double zs=node.z-offset.z;


                double rs = dialate_radius; //expansion

                double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

                ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sz_x-1));
                ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sz_x-1));
                if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

                bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sz_y-1));
                bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sz_y-1));
                if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

                ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz_z-1));
                ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz_z-1));
                if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

                //mark all voxels close to the swc node(s)
                for (int kk = ballz0; kk <= ballz1; kk++){
                    for (int jj = bally0; jj <= bally1; jj++){
                        for (int ii = ballx0; ii <= ballx1; ii++){
                            V3DLONG ind = (kk)*pagesz + (jj)*sz_x + ii;
                            ind = MIN(ind,tol_sz);
                            ind = MAX(ind, 0);
                            if (img1d[ind]<=j) {
                                img1d[ind]++;
                            }

                        }
                    }
                }

                //            V3DLONG id_x = (node.x-offset.x) +0.5; //round up
                //            V3DLONG id_y = (node.y-offset.y) +0.5;
                //            V3DLONG id_z = (node.z-offset.z) +0.5;
                //            V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;

                //            if (idx <tol_sz &&  VOTED[idx] == 0 ){
                //                img1d[idx] ++ ;
                //                VOTED[idx] = 1;
                //            }

            }
        }

        //for debug only
        //    Image4DSimple *image = new Image4DSimple();
        //    image->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);
        //    callback.saveImage(image, "./vote_map.v3draw");

        return true;

    }









void label_image(unsigned char * img1d, V3DLONG xs, V3DLONG ys, V3DLONG zs, double rs,
                 unsigned char * VOTED, V3DLONG sx, V3DLONG sy,V3DLONG sz)
{
    double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;
    long long  pagesz = sx*sy;
    ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
    ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
    if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

    bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
    bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
    if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

    ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
    ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
    if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

    //mark all voxels close to the swc node(s)
    for (V3DLONG k = ballz0; k <= ballz1; k++){
        for (V3DLONG j = bally0; j <= bally1; j++){
            for (V3DLONG i = ballx0; i <= ballx1; i++){
                long long  ind = (k)*pagesz + (j)*sx + i;
                if (VOTED[ind] == 0 /* and distance < rs*/) {
                    img1d[ind]++;
                    VOTED[ind] = 1;}
            }
        }
    }


}




bool generate_vote_map_resample(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d, V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset)
{
        cout <<"\nGenerate vote map with resampling "<<endl;
        long long tol_sz = sz_x* sz_y*sz_z;
        cout << "vote_map image size(memory) = " << tol_sz<<": " <<sz_x<<"x "<<sz_y<<" x"<<sz_z<< endl;
        // from leaf nodes to roots, resample between nodes if necessary



        unsigned char * VOTED = new unsigned char[tol_sz];

        for (V3DLONG n_id =0;n_id <nt_list.size();n_id++){
            NeuronTree neurons = nt_list[n_id];
            NeuronSWC *p_cur = 0;
            //create a LUT
            QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

            //compute mask
            double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;

            for(long long i = 0; i < tol_sz; i++) VOTED[i] = 0;

            for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
            {
                V3DLONG i,j,k;
                p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
                xs = p_cur->x - offset.x;
                ys = p_cur->y - offset.y;
                zs = p_cur->z - offset.z;
                rs = dialate_radius;

                label_image(img1d, xs,  ys,  zs,  rs, VOTED,  sz_x,  sz_y, sz_z);

                //find previous node
                if (p_cur->pn < 0)
                    continue;//root node skip the following

                //get the parent info
                const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
                xe = pp.x- offset.x;
                ye = pp.y- offset.y;
                ze = pp.z- offset.z;
                re = pp.r;

                //judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
                if (xe==xs && ye==ys && ze==zs)
                {
                    continue;
                }

                // interpolate along the line
                double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));
                double dx = (xe - xs);
                double dy = (ye - ys);
                double dz = (ze - zs);
                double x = xs;
                double y = ys;
                double z = zs;

                int steps = lroundf(l);
                steps = (steps < fabs(dx))? fabs(dx):steps;
                steps = (steps < fabs(dy))? fabs(dy):steps;
                steps = (steps < fabs(dz))? fabs(dz):steps;
                if (steps<1)
                    steps =1;

                double xIncrement = double(dx) / (steps*2);
                double yIncrement = double(dy) / (steps*2);
                double zIncrement = double(dz) / (steps*2);

                for (int i = 0; i <= steps; i++)
                {
                    x += xIncrement;
                    y += yIncrement;
                    z += zIncrement;
                    label_image(img1d, x,  y,  z,  rs, VOTED, sz_x,  sz_y, sz_z);
                }
            }

        }


        //for debug only
        //    Image4DSimple *image = new Image4DSimple();
        //    image->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);
        //    callback.saveImage(image, "./vote_map.v3draw");

        return true;

    }

void AddToMaskImage_old(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,
		int imageCount, V3DPluginCallback2 & callback)
{
	NeuronSWC *p_cur = 0;
	//create a LUT
	QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

	//compute mask
	double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
	V3DLONG pagesz = sx*sy;
	V3DLONG tol_sz = pagesz*sz;

	for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	{
		V3DLONG i,j,k;
		p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
		xs = p_cur->x;
		ys = p_cur->y;
		zs = p_cur->z;
		rs = p_cur->r;

		double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

		ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
		ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
		if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

		bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
		bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
		if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

		ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
		ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
		if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

		//mark all voxels close to the swc node(s)
		for (k = ballz0; k <= ballz1; k++){
			for (j = bally0; j <= bally1; j++){
				for (i = ballx0; i <= ballx1; i++){
					V3DLONG ind = (k)*pagesz + (j)*sx + i;
					ind = MIN(ind,tol_sz);
					ind = MAX(ind, 0);
					if (pImMask[ind]>imageCount) continue;
					double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k);
					double dt = sqrt(norms10);
					if(dt <=rs || dt<=1)
					{
						pImMask[ind] +=1;
					}
				}
			}
		}

		//find previous node
		if (p_cur->pn < 0) continue;//then it is root node already
		//get the parent info
		const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
		xe = pp.x;
		ye = pp.y;
		ze = pp.z;
		re = pp.r;

		//judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
		if (xe==xs && ye==ys && ze==zs)
		{
			v3d_msg(QString("Detect overlapping coordinates of node\n"), 0);
			continue;
		}

		double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));
		double dx = (xe - xs);
		double dy = (ye - ys);
		double dz = (ze - zs);
		double x = xs;
		double y = ys;
		double z = zs;

		int steps = lroundf(l);
		steps = (steps < fabs(dx))? fabs(dx):steps;
		steps = (steps < fabs(dy))? fabs(dy):steps;
		steps = (steps < fabs(dz))? fabs(dz):steps;
		if (steps<1) steps =1;

		double xIncrement = double(dx) / (steps*2);
		double yIncrement = double(dy) / (steps*2);
		double zIncrement = double(dz) / (steps*2);

		if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
		V3DLONG idex1=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
		idex1 = MIN(idex1,tol_sz);
		idex1 = MAX(idex1, 0);

		pImMask[idex1] += 1;

		for (int i = 0; i <= steps; i++)
		{
			x += xIncrement;
			y += yIncrement;
			z += zIncrement;

			x = ( x > sx )? sx : x;
			y = ( y > sy )? sy : y;
			z = ( z > sz )? sz : z;
			if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
			V3DLONG idex=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);

			if (pImMask[idex]> imageCount) continue;

			pImMask[idex] += 1;

		}

		//finding the envelope of the current line segment

		double rbox = (rs>re) ? rs : re;
		double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=sx-1) x_down = sx-1;
		double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=sx-1)  x_top  = sx-1;
		double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=sy-1) y_down = sy-1;
		double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=sy-1)  y_top = sy-1;
		double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=sz-1) z_down = sz-1;
		double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=sz-1)  z_top = sz-1;

		//compute cylinder and flag mask

		for (k=z_down; k<=z_top; k++)
		{
			for (j=y_down; j<=y_top; j++)
			{
				for (i=x_down; i<=x_top; i++)
				{
					double rr = 0;
					double countxsi = (xs-i);
					double countysj = (ys-j);
					double countzsk = (zs-k);
					double countxes = (xe-xs);
					double countyes = (ye-ys);
					double countzes = (ze-zs);
					double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
					double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
					double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
					double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
					double t1 = -dots1021/norms21;
					if(t1<0) dist = sqrt(norms10);
					else if(t1>1)
						dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
					//compute rr
					if (rs==re) rr =rs;
					else
					{
						// compute point of intersection
						double v1 = xe - xs;
						double v2 = ye - ys;
						double v3 = ze - zs;
						double vpt = v1*v1 + v2*v2 +v3*v3;
						double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
						double xc = xs + v1*t;
						double yc = ys + v2*t;
						double zc = zs + v3*t;
						double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
						double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
						rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
					}
					if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
					V3DLONG ind1 = (k)*sx*sy + (j)*sx + i;

					if (pImMask[ind1]>imageCount) continue;

					if (dist <= rr || dist<=1)
					{
						pImMask[ind1] += 1;
					}
				}
			}
		}

	}


}



bool vote_map(vector<NeuronTree> & nt_list, int dialate_radius, QString outfileName,V3DPluginCallback2 & callback){

	//initialize the image volume to record/accumulate the  location votes from neurons
	MyBoundingBox bbUnion = neuron_trees_bb(nt_list);

	V3DLONG sz_x,sz_y,sz_z;
	//for swcs generated from vaa3d, all meta offset are disgarded, so the swc coordinates are in model coordinates
	//this will matches with the original image ( on the upper left corner)
	sz_x = bbUnion.max_x;
	sz_y = bbUnion.max_y;
	sz_z = bbUnion.max_z;


	V3DLONG  tol_sz = sz_x * sz_y * sz_z;
	cout << "image size = " << tol_sz<<": " <<sz_x<<" x "<<sz_y<<" x "<<sz_z<< endl;

	unsigned char* pImMask = 0;
	pImMask = new unsigned char [tol_sz];
	memset(pImMask,0,tol_sz*sizeof(unsigned char));

	for (int j = 0; j < nt_list.size(); j++){
		NeuronTree nt = nt_list[j];
		AddToMaskImage(nt, pImMask, sz_x, sz_y, sz_z,dialate_radius,j,callback);
	}


	if ( outfileName.size() >0)
	{
		Image4DSimple *image = new Image4DSimple();
		for (int i = 0 ; i< tol_sz;i++)
		{
			pImMask[i]= pImMask[i]*1.0/nt_list.size() *255;
		}
		image->setData(pImMask, sz_x, sz_y, sz_z, 1, V3D_UINT8);
		callback.saveImage(image, &(outfileName.toStdString()[0]));
	}

	return true;
}


bool soma_sort(double search_distance_th, QList<NeuronSWC> consensus_nt_list, double soma_x, double soma_y, double soma_z, QList<NeuronSWC> &out_sorted_consensus_nt_list, double bridge_size)
{

	cout<<"\nSoma sorting: matching soma from the median-sized swc"<<endl;


	NeuronTree consensus_nt;
	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();
	//set node
	NeuronSWC S;
	for (V3DLONG i=0;i<consensus_nt_list.size();i++)
	{
		NeuronSWC curr = consensus_nt_list.at(i);
		S.n 	= curr.n;
		S.type 	= curr.type;
		S.x 	= curr.x;
		S.y 	= curr.y;
		S.z 	= curr.z;
		S.r 	= curr.r;
		S.pn 	= curr.pn;
		S.seg_id = curr.seg_id;
		S.level = curr.level;
		S.fea_val = curr.fea_val;

		listNeuron.append(S);
		hashNeuron.insert(S.n, listNeuron.size()-1);

	}
	//consensus_nt.n = -1;
	//consensus_nt.on = true;
	consensus_nt.listNeuron = listNeuron;
	consensus_nt.hashNeuron = hashNeuron;


	QVector<QVector<V3DLONG> > children;

	V3DLONG num_nodes = consensus_nt.listNeuron.size();
	children = QVector< QVector<V3DLONG> >(num_nodes, QVector<V3DLONG>() );

	for (V3DLONG i=0;i<num_nodes;i++)
	{
		V3DLONG par = consensus_nt.listNeuron[i].pn;
		if (par<0) continue;
		children[consensus_nt.hashNeuron.value(par)].push_back(i);
	}


	double Dist = 10000000000;
	double Dist_inrange = 10000000000;
	V3DLONG soma_ID = -1;
	V3DLONG dist_ID = -1;
	int children_num = 0;


	for (V3DLONG i=0;i<num_nodes;i++)
	{
		NeuronSWC curr = consensus_nt_list.at(i);
		double nodedist = sqrt(pow2(curr.x - soma_x) + pow2(curr.y - soma_y) + pow2(curr.z - soma_z));
		if(nodedist <= search_distance_th && curr.pn <0)
		{
			soma_ID = curr.n;
			children_num = 1;
			break;
		}

		if(nodedist <= search_distance_th  && children[i].size() > children_num)
		{
			soma_ID = curr.n;
			children_num = children[i].size();
			Dist_inrange = nodedist;
		}

		if(nodedist <= search_distance_th && children[i].size() == children_num && nodedist < Dist_inrange)
		{
			soma_ID = curr.n;
			Dist_inrange = nodedist;
		}

		if(nodedist < Dist)
		{
			dist_ID = curr.n;
			Dist = nodedist;
		}
	}

	if(children_num < 1 || soma_ID == -1) soma_ID = dist_ID;


	cout<<"root="<<soma_ID<<endl;


	// sort
	// TODO: pick out the top tree
	return SortESWC (consensus_nt_list, out_sorted_consensus_nt_list, soma_ID, bridge_size);

}

bool consensus_skeleton(vector<NeuronTree> & nt_list, QList<NeuronSWC> & final_consensus, int max_vote_threshold, int cluster_distance_threshold,V3DPluginCallback2 &callback)
{
    int method_code = 2; // MST
	//int cluster_distance_threshold = 10;
	//threshold to ignore mapping  (too far away) for generting nodeMap below
	//potentially, there are invalid neuron trees (massive node points, no node points, looping)
	double root_x = 0;
	double root_y = 0;
	double root_z = 0;

	remove_outliers(nt_list, root_x, root_y, root_z);
	int neuronNum = nt_list.size();
	if (neuronNum<2){
		cout<<"not enough neurons left for consensuing...." <<endl;
		return false;
	}

	//initialize the image volume to record/accumulate the  location votes from neurons
	MyBoundingBox bbUnion = neuron_trees_bb(nt_list);

	Point3D offset = {bbUnion.min_x ,bbUnion.min_y ,bbUnion.min_z };
	float closeness = 1.0;
	V3DLONG  sz_x = ceil((bbUnion.max_x - bbUnion.min_x ) / closeness) +1; //+0.5 to round up from float to V3DLONG
	V3DLONG  sz_y = ceil((bbUnion.max_y - bbUnion.min_y ) / closeness) +1;
	V3DLONG  sz_z = ceil((bbUnion.max_z - bbUnion.min_z ) / closeness) +1;
    long long  tol_sz = sz_x * sz_y * sz_z;
    if (tol_sz > LONG_LONG_MAX){
		cout << sz_x<<"  "<<sz_y<<"  "<<sz_z<<endl;
        cout <<"when allocating for votemap volume: requires too much memory to allocate for the vote map:"<<tol_sz <<endl;
		return false;
	}



	//****************************************************
	// step 1: VOTE MAP
    unsigned char * img1d = new unsigned char[tol_sz];
    for(long long i = 0; i < tol_sz; i++) img1d[i] = 0;

    //if (! generate_vote_map(nt_list,0,img1d, sz_x,sz_y,sz_z, offset))
    if (! generate_vote_map_resample(nt_list,0,img1d, sz_x,sz_y,sz_z, offset))

    {
        cout <<"error in generating vote map"<<endl;
        return false;
    }







	//****************************************************
	// Step 2:  Consensus Nodes
	cout <<"\nCompute consensus nodes (non-max_supression)"<<endl;
	//non-maximum suppresion
	vector<Point3D>  node_list;
	vector<unsigned int>  vote_list;

	double mean_vote = mean_image_value(img1d, tol_sz);
	cout << "mean votes in the vote map:" << mean_vote << endl;
	double vote_threshold = mean_vote-1;
	if (vote_threshold < 1) { vote_threshold = 1.0;}
    if (vote_threshold > max_vote_threshold) {vote_threshold = max_vote_threshold;}
	cout << "threshold vote:" << vote_threshold << endl;

    int windows_siz = 5;
    non_max_suppresion (img1d,sz_x,sz_y,sz_z,vote_threshold, offset,node_list,vote_list,windows_siz);
	cout << "after non_max supression:"<< endl;
	cout << "number of nodes:"<< node_list.size() << endl;
	cout << "maximum votes:" << v_max(vote_list) << endl;



	Image4DSimple *image2 = new Image4DSimple();
	image2->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);
	callback.saveImage(image2, "./nms_image.v3draw");


	// for debug: save node_list to check locations
   /*
	QList<NeuronSWC> locationTree;
	for (int i=0;i<node_list.size();i++)
	{
		NeuronSWC tmp;
		tmp.x = node_list[i].x;
		tmp.y = node_list[i].y;
		tmp.z = node_list[i].z;

		tmp.type = 2; //edge votes
		tmp.pn = -1;  //parent id, form the edge
		tmp.r = double(vote_list[i]);
		tmp.n = i+1;

		locationTree.append(tmp);
	}
	export_listNeuron_2swc(locationTree, "./test_nms_location.swc");
   */




	//****************************************************
	// Step 3: adjacency matrix
	cout <<"\nCompute adjacency matrix (vote for edges)"<<endl;


	double * adjMatrix;
	V3DLONG * plist;
	V3DLONG num_nodes = node_list.size();
	try{
		adjMatrix = new double[num_nodes*num_nodes];
		plist = new V3DLONG[num_nodes];
		for (V3DLONG i=0;i<num_nodes*num_nodes;i++) adjMatrix[i] = 0;
	}
	catch (...)
	{
		fprintf(stderr,"fail to allocate memory.\n");
		if (adjMatrix) {delete[] adjMatrix; adjMatrix=0;}
		if (plist) {delete[] plist; plist=0;}
		return false;
	}


	int * EDGE_VOTED = new int[num_nodes*num_nodes];

	//cluster nodes from each neurontree to consensus nodes
	for (int i=0;i<neuronNum;i++)
	{
		QHash<V3DLONG, V3DLONG > nodeMap; //maps j node of tree i to consensus node(node_id)
		for (V3DLONG j=0;j<nt_list[i].listNeuron.size();j++)
		{
			NeuronSWC s = nt_list[i].listNeuron.at(j);
			Point3D cur;
			cur.x = s.x;
			cur.y = s.y;
			cur.z = s.z;

			//find its nearest node
			V3DLONG node_id = -1;// this node does not exist
			double min_dis = double(cluster_distance_threshold); //threshold to ignore mapping  (too far away)
			for (V3DLONG ni = 0; ni <node_list.size(); ni++)
			{
				Point3D p = node_list[ni];
				double dis = PointDistance(p,cur);

				if (dis < min_dis){
					min_dis = dis;
					node_id = ni;
				}
			}
			if (node_id > -1){
				nodeMap.insert( j, node_id);
			}
		}
		//maps.push_back(nodeMap);

		for (V3DLONG ii=0;ii<num_nodes*num_nodes;ii++) EDGE_VOTED[ii] = 0;
		for (V3DLONG j=0;j<nt_list[i].listNeuron.size();j++)
		{
			NeuronSWC cur = nt_list[i].listNeuron[j];
			// if (cur.pn<0) continue;
			V3DLONG n_id,pn_id;
			if (nodeMap.contains(j))
			{
				n_id = nodeMap[j];
                //V3DLONG pidx = cur.pn-1;//nt_list[i].hashNeuron.value(cur.pn);  // find the index in nueon_list
                V3DLONG pidx = nt_list[i].hashNeuron.value(cur.pn);

				pn_id = nodeMap[pidx];
                //if (pn_id > -1  && EDGE_VOTED[n_id*num_nodes + pn_id] ==0 ){
                if (EDGE_VOTED[n_id*num_nodes + pn_id] ==0 ){
					adjMatrix[n_id*num_nodes + pn_id] += 1;
					adjMatrix[pn_id*num_nodes + n_id] += 1;
					//cout<<adjMatrix[n_id*num_nodes + pn_id] <<endl;
					EDGE_VOTED[n_id*num_nodes + pn_id] = 1;
					EDGE_VOTED[pn_id*num_nodes + n_id] = 1;

				}
			}
		}

	}

	//***********************************************************
	//connecting nodes
	//filter out low edge votes use the same threshold, set it to low value 1
	for ( int i = 0; i <num_nodes; i++){
		for (int j = 0; j < num_nodes ; j++){
			if (double(adjMatrix[i+j*num_nodes]) < vote_threshold){
				adjMatrix[i+j*num_nodes] = 1 ;
				adjMatrix[j+i*num_nodes] = 1 ;
			}
		}

	}

	QList <NeuronSWC> merge_result;
	// MST method
	if (method_code == 2 ){
        long rootnode =0;
		cout <<"\nComputing max-spanning tree" <<endl;
		//if (!mst_dij(adjMatrix, num_nodes, plist, rootnode))
		if (!mst_prim(adjMatrix, num_nodes, plist, rootnode))

		{
			fprintf(stderr,"Error in minimum spanning tree!\n");
			return false;
		}

		// code the edge votes into type for visualization
		//         graph: duplicate swc nodes are allowed to accomandate mutiple parents for the child node, no root id,
		merge_result.clear();
		for (V3DLONG i = 0;i <num_nodes;i ++)
		{
			V3DLONG p = plist[i];
			//cout <<p<<endl;
			unsigned int edgeVote = adjMatrix[i*num_nodes + p];
			if (edgeVote > 1)
			{
				NeuronSWC tmp;
				tmp.x = node_list[i].x;
				tmp.y = node_list[i].y;
				tmp.z = node_list[i].z;

				tmp.type = edgeVote; //edge votes
				tmp.pn = p + 1;  //parent id, form the edge
				tmp.r = double(vote_list[i])/double(neuronNum);
				tmp.n = i+1;

				merge_result.append(tmp);
			}

		}
	}



	// alternative
	if  (method_code == 1){
		merge_result.clear();
		V3DLONG count = 0;
		for (V3DLONG i=0;i<num_nodes;i++)
		{
			NeuronSWC tmp;
			tmp.x = node_list[i].x;
			tmp.y = node_list[i].y;
			tmp.z = node_list[i].z;
			// tmp.fea_val.push_back(vote_list[i]);  //location votes are coded into radius

			tmp.type = 0; //vertices (soma)
			tmp.pn = -1;  //parent id, no edge
			tmp.r = double(vote_list[i])/double(neuronNum); //location votes are coded into radius
			tmp.n = count +1; //id start from 1
			merge_result.append(tmp);
			count++;
		}

		//output edges, go through half of the symmetric matrix, not directed graph
		for (V3DLONG row = 0;row <num_nodes;row ++)
		{
			for (V3DLONG col = row+1;col < num_nodes;col++){
				unsigned int edgeVote = adjMatrix[row*num_nodes + col];
				if (edgeVote >= vote_threshold)
				{
					if (merge_result[row].pn == -1)
					{//exsiting isolated vertex, modify parent id
						merge_result[row].type = edgeVote; //edge votes
						merge_result[row].pn = col + 1;  //parent id  , form the edge
						merge_result[row].r = double(vote_list[row])/double(neuronNum);
					}
					else{
						//add new edge  , via duplication nodes with different parent id and edge votes
						NeuronSWC tmp;
						tmp.x = node_list[row].x;
						tmp.y = node_list[row].y;
						tmp.z = node_list[row].z;

						tmp.type = edgeVote; //edge votes
						tmp.pn = col + 1;  //parent id  , form the edge
						tmp.r = double(vote_list[row])/double(neuronNum);
						tmp.n = count+1;

						merge_result.append(tmp);
						count++;
					}

				}
			}
		}


	}
	if (adjMatrix) {delete[] adjMatrix; adjMatrix = 0;}
	if (plist) {delete[] plist; plist=0;}


	cout <<"\nSort with soma root from median case." <<endl;
	double search_distance_th = cluster_distance_threshold;
	double soma_x = root_x;
	double soma_y = root_y;
	double soma_z = root_z;
	if (   soma_sort(search_distance_th, merge_result, soma_x, soma_y, soma_z, final_consensus, 3) )
	{
		cout <<"merged swc #nodes = "<< merge_result.size()<<endl<<endl;

		//only output the first connected tree
		//for (int ii =1 ; ii <final_consensus.size();ii++){
		//    if  (final_consensus[ii].pn == -1){
		//        final_consensus.erase(final_consensus.begin()+ii, final_consensus.end());
		//    }
		//}

		//cout <<"final swc #nodes = "<< final_consensus.size()<<endl;
		return true;
	}


	return false;
}




bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename)
{
	FILE * fp;
	fp = fopen(filename,"w");
	QString fn = QString(filename);
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",filename);
		return false;
	}

	if (fn.endsWith(".eswc"))
	{ // eswc
		fprintf(fp,"#n,type,x,y,z,radius,parent,seg_id,level,edge_vote\n");   //,vote\n");
		for (int i=0;i<list.size();i++)
		{
			NeuronSWC curr = list.at(i);
			fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d %d %d %d\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn,0,0, curr.type);
		}
	}
	else
	{
		fprintf(fp,"#n,type,x,y,z,radius,parent\n");   //,vote\n");
		for (int i=0;i<list.size();i++)
		{
			NeuronSWC curr = list.at(i);
			fprintf(fp,"%d %d %.2f %.2f %.2f %.3f  %d\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn);
		}
	}

	fclose(fp);
	return true;
}
