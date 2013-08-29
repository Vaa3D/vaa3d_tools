// fastmarching_dt.h       2012-02-23 by Hang Xiao

#ifndef __FASTMARCHING_DT_H__
#define __FASTMARCHING_DT_H__

#include <iostream>
#include <map>
#include "heap.h"

#define INF 3.4e+38

using namespace std;

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif

/******************************************************************************
 * Fast marching based distance transformation
 * 
 * Input : inimg1d     original input grayscale image
 *         cnn_type    the connection type
 *         bkg_thresh  the background threshold, less or equal then bkg_thresh will considered as background
 *
 * Output : phi       distance tranformed output image
 *
 * Notice : 
 * 1. the input pixel number should not be larger than 2G if sizeof(long) == 4 //this should have been solved by changing "long" to "V3DLONG". see changes introduced in version 634
 * 2. The background point is of intensity 0
 * *****************************************************************************/

template<class T> bool fastmarching_dt(T * inimg1d, float * &phi, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, int cnn_type = 3, int bkg_thresh = 0, double z_thickness = 1.0)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

    V3DLONG tol_sz = sz0 * sz1 * sz2;
    V3DLONG sz01 = sz0 * sz1;
	//int cnn_type = 3;  // ?

    if(phi == 0) phi = new float[tol_sz];
    char * state = new char[tol_sz];
    V3DLONG bkg_count = 0;                          // for process counting
    V3DLONG bdr_count = 0;                          // for process counting
    for(V3DLONG i = 0; i < tol_sz; i++)
	{
		if(inimg1d[i] <= bkg_thresh)
		{
			phi[i] = inimg1d[i];
			state[i] = ALIVE;
			//cout<<"+";cout.flush();
			bkg_count++;
        }
		else
		{
			phi[i] = INF;
			state[i] = FAR;
		}
	}



	cout<<endl;
	BasicHeap<HeapElem> heap;
    map<V3DLONG, HeapElem*> elems;

	// init heap
	{
        V3DLONG i = -1, j = -1, k = -1;
        for(V3DLONG ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] == ALIVE)
			{
				for(int kk = -1; kk <= 1; kk++)
				{
                    V3DLONG k2 = k+kk;
					if(k2 < 0 || k2 >= sz2) continue;
					for(int jj = -1; jj <= 1; jj++)
					{
                        V3DLONG j2 = j+jj;
						if(j2 < 0 || j2 >= sz1) continue;
						for(int ii = -1; ii <=1; ii++)
						{
                            V3DLONG i2 = i+ii;
							if(i2 < 0 || i2 >= sz0) continue;
							int offset = ABS(ii) + ABS(jj) + ABS(kk);
							if(offset == 0 || offset > cnn_type) continue;
                            V3DLONG ind2 = k2 * sz01 + j2 * sz0 + i2;
							if(state[ind2] == FAR)
							{
                                V3DLONG min_ind = ind;
								// get minimum Alive point around ind2
								if(phi[min_ind] > 0.0)
								{
									for(int kkk = -1; kkk <= 1; kkk++)
									{
                                        V3DLONG k3 = k2 + kkk;
										if(k3 < 0 || k3 >= sz2) continue;
										for(int jjj = -1; jjj <= 1; jjj++)
										{
                                            V3DLONG j3 = j2 + jjj;
											if(j3 < 0 || j3 >= sz1) continue;
											for(int iii = -1; iii <= 1; iii++)
											{
                                                V3DLONG i3 = i2 + iii;
												if(i3 < 0 || i3 >= sz0) continue;
												int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
												if(offset2 == 0 || offset2 > cnn_type) continue;
                                                V3DLONG ind3 = k3 * sz01 + j3 * sz0 + i3;
												if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]) min_ind = ind3;
											}
										}
									}
								}
								// over
								phi[ind2] = phi[min_ind] + inimg1d[ind2];
								state[ind2] = TRIAL;
								HeapElem * elem = new HeapElem(ind2, phi[ind2]);
								heap.insert(elem);
								elems[ind2] = elem;
								bdr_count++;
							}
						}
					}
				}
			}
		}
	}

	cout<<"bkg_count = "<<bkg_count<<" ("<<bkg_count/(double)tol_sz<<")"<<endl;
	cout<<"bdr_count = "<<bdr_count<<" ("<<bdr_count/(double)tol_sz<<")"<<endl;
	cout<<"elems.size() = "<<elems.size()<<endl;
	// loop
    V3DLONG time_counter = bkg_count;
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*100000.0/tol_sz;
        if(process2 - process1 >= 10) {cout<<"\r"<<((V3DLONG)process2)/1000.0<<"%";cout.flush(); process1 = process2;
			//SAVE_PHI_IMAGE(phi, sz0, sz1, sz2, string("phi") + num2str((int)process1) + ".tif");
		}

		HeapElem* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

        V3DLONG min_ind = min_elem->img_ind;
		delete min_elem;

		state[min_ind] = ALIVE;
        V3DLONG i = min_ind % sz0;
        V3DLONG j = (min_ind/sz0) % sz1;
        V3DLONG k = (min_ind/sz01) % sz2;

		int w, h, d;
		for(int kk = -1; kk <= 1; kk++)
		{
			d = k+kk;
			if(d < 0 || d >= sz2) continue;
			for(int jj = -1; jj <= 1; jj++)
			{
				h = j+jj;
				if(h < 0 || h >= sz1) continue;
				for(int ii = -1; ii <= 1; ii++)
				{
					w = i+ii;
					if(w < 0 || w >= sz0) continue;
					int offset = ABS(ii) + ABS(jj) + ABS(kk);
					if(offset == 0 || offset > cnn_type) continue;
					double factor = 0.0;
                    V3DLONG index = d*sz01 + h*sz0 + w;

					if(state[index] != ALIVE)
					{
                        float new_dist = phi[min_ind] + inimg1d[index] * sqrt(ii*ii + jj*jj + kk * kk * z_thickness* z_thickness);

						if(state[index] == FAR)
						{
							phi[index] = new_dist;
							HeapElem * elem = new HeapElem(index, phi[index]);
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > new_dist)
							{
								cout<<"refrsh TRIAL"<<endl;
								phi[index] = new_dist;
								HeapElem * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
							}
						}
					}
				}
			}
		}
	}

	assert(elems.empty());
	if(state) {delete [] state; state = 0;}
	return true;
}

#endif
