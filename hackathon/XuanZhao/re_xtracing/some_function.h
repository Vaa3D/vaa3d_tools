#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

#define INF 1E20
#include <iostream>
#include <map>
#include <math.h>
#include "heap.h"
#include "some_class.h"


#define ABS(x) ((x) > 0 ? (x) : (-(x)))


#define GI(ind) givals[(int)((inimg1d[ind] - min_int)/max_int*255)]

static double givals[256] = {22026.5,   20368, 18840.3, 17432.5, 16134.8, 14938.4, 13834.9, 12816.8,
11877.4, 11010.2, 10209.4,  9469.8, 8786.47, 8154.96, 7571.17, 7031.33,
6531.99, 6069.98, 5642.39, 5246.52, 4879.94, 4540.36, 4225.71, 3934.08,
 3663.7, 3412.95, 3180.34,  2964.5, 2764.16, 2578.14, 2405.39,  2244.9,
2095.77, 1957.14, 1828.24, 1708.36, 1596.83, 1493.05, 1396.43, 1306.47,
1222.68, 1144.62, 1071.87, 1004.06, 940.819, 881.837, 826.806, 775.448,
727.504, 682.734, 640.916, 601.845, 565.329, 531.193, 499.271, 469.412,
441.474, 415.327, 390.848, 367.926, 346.454, 326.336, 307.481, 289.804,
273.227, 257.678, 243.089, 229.396, 216.541, 204.469, 193.129, 182.475,
172.461, 163.047, 154.195, 145.868, 138.033, 130.659, 123.717, 117.179,
111.022,  105.22, 99.7524, 94.5979, 89.7372, 85.1526,  80.827, 76.7447,
 72.891, 69.2522, 65.8152, 62.5681, 59.4994, 56.5987,  53.856, 51.2619,
48.8078, 46.4854, 44.2872, 42.2059, 40.2348, 38.3676, 36.5982, 34.9212,
33.3313, 31.8236, 30.3934, 29.0364, 27.7485,  26.526,  25.365, 24.2624,
23.2148, 22.2193,  21.273, 20.3733, 19.5176, 18.7037, 17.9292,  17.192,
16.4902,  15.822, 15.1855,  14.579, 14.0011, 13.4503, 12.9251, 12.4242,
11.9464, 11.4905, 11.0554, 10.6401, 10.2435, 9.86473, 9.50289, 9.15713,
8.82667, 8.51075, 8.20867, 7.91974, 7.64333, 7.37884, 7.12569, 6.88334,
6.65128, 6.42902,  6.2161, 6.01209, 5.81655, 5.62911, 5.44938, 5.27701,
5.11167, 4.95303, 4.80079, 4.65467, 4.51437, 4.37966, 4.25027, 4.12597,
4.00654, 3.89176, 3.78144, 3.67537, 3.57337, 3.47528, 3.38092, 3.29013,
3.20276, 3.11868, 3.03773,  2.9598, 2.88475, 2.81247, 2.74285, 2.67577,
2.61113, 2.54884, 2.48881, 2.43093, 2.37513, 2.32132, 2.26944, 2.21939,
2.17111, 2.12454, 2.07961, 2.03625, 1.99441, 1.95403, 1.91506, 1.87744,
1.84113, 1.80608, 1.77223, 1.73956, 1.70802, 1.67756, 1.64815, 1.61976,
1.59234, 1.56587, 1.54032, 1.51564, 1.49182, 1.46883, 1.44664, 1.42522,
1.40455,  1.3846, 1.36536,  1.3468,  1.3289, 1.31164, 1.29501, 1.27898,
1.26353, 1.24866, 1.23434, 1.22056,  1.2073, 1.19456, 1.18231, 1.17055,
1.15927, 1.14844, 1.13807, 1.12814, 1.11864, 1.10956, 1.10089, 1.09262,
1.08475, 1.07727, 1.07017, 1.06345, 1.05709, 1.05109, 1.04545, 1.04015,
1.03521,  1.0306, 1.02633, 1.02239, 1.01878,  1.0155, 1.01253, 1.00989,
1.00756, 1.00555, 1.00385, 1.00246, 1.00139, 1.00062, 1.00015,       1};

template<class T>
bool fastmarching_dt(T* inimg1d,float* &phi,int sz0,int sz1,int sz2,int cnn_type=3,int bkg_thres=25)
{
    enum{ALIVE = -1, TRIAL = 0, FAR = 1};

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;
    //int cnn_type = 3;  // ?

    if(phi == 0) phi = new float[tol_sz];
    char * state = new char[tol_sz];
    int bkg_count = 0;                          // for process counting
    int bdr_count = 0;                          // for process counting
    for(long i = 0; i < tol_sz; i++)
    {
        if(inimg1d[i] <= bkg_thres)
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
    map<long, HeapElem*> elems;

    // init heap
    {
        long i = -1, j = -1, k = -1;
        for(long ind = 0; ind < tol_sz; ind++)
        {
            i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
            if(state[ind] == ALIVE)
            {
                for(int kk = -1; kk <= 1; kk++)
                {
                    long k2 = k+kk;
                    if(k2 < 0 || k2 >= sz2) continue;
                    for(int jj = -1; jj <= 1; jj++)
                    {
                        long j2 = j+jj;
                        if(j2 < 0 || j2 >= sz1) continue;
                        for(int ii = -1; ii <=1; ii++)
                        {
                            long i2 = i+ii;
                            if(i2 < 0 || i2 >= sz0) continue;
                            int offset = ABS(ii) + ABS(jj) + ABS(kk);
                            if(offset == 0 || offset > cnn_type) continue;
                            long ind2 = k2 * sz01 + j2 * sz0 + i2;
                            if(state[ind2] == FAR)
                            {
                                long min_ind = ind;
                                // get minimum Alive point around ind2
                                if(phi[min_ind] > 0.0)
                                {
                                    for(int kkk = -1; kkk <= 1; kkk++)
                                    {
                                        long k3 = k2 + kkk;
                                        if(k3 < 0 || k3 >= sz2) continue;
                                        for(int jjj = -1; jjj <= 1; jjj++)
                                        {
                                            long j3 = j2 + jjj;
                                            if(j3 < 0 || j3 >= sz1) continue;
                                            for(int iii = -1; iii <= 1; iii++)
                                            {
                                                long i3 = i2 + iii;
                                                if(i3 < 0 || i3 >= sz0) continue;
                                                int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
                                                if(offset2 == 0 || offset2 > cnn_type) continue;
                                                long ind3 = k3 * sz01 + j3 * sz0 + i3;
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
    int time_counter = bkg_count;
    double process1 = 0;
    while(!heap.empty())
    {
        double process2 = (time_counter++)*100000.0/tol_sz;
        if(process2 - process1 >= 100) {cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
        }

        HeapElem* min_elem = heap.delete_min();
        elems.erase(min_elem->img_ind);

        long min_ind = min_elem->img_ind;
        delete min_elem;

        state[min_ind] = ALIVE;
        int i = min_ind % sz0;
        int j = (min_ind/sz0) % sz1;
        int k = (min_ind/sz01) % sz2;

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
                    long index = d*sz01 + h*sz0 + w;

                    if(state[index] != ALIVE)
                    {
                        float new_dist = phi[min_ind] + inimg1d[index] * sqrt(double(offset));

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
    //END_CLOCK;
    assert(elems.empty());
    if(state) {delete [] state; state = 0;}
    return true;
}

template<class T>
bool fm_find_segs(T* inimg1d,long* &plist,int sz0,int sz1,int sz2,int cnn_type=3,int bkg_thres=25)
{
    enum{ALIVE=-1,TRIAL=0,FAR=1,INVALID=2};

    long alive_c=0,trial_c=0,far_c=0,invalid_c=0;

    long tol_sz=sz0*sz1*sz2;
    long sz01=sz0*sz1;

    long i;
    float* phi=0;
    char* state=0;
    plist=0;
    cout<<"0"<<endl;
    cout<<"bkg_thre:"<<bkg_thres<<endl;
    //bkg_thres=30;
    try
    {
        phi=new float[tol_sz];
        plist=new long[tol_sz];
        state=new char[tol_sz];
        for(i=0;i<tol_sz;++i)
        {
            phi[i]=INF;
            plist[i]=i;
            if(inimg1d[i]<bkg_thres*10)
            {
                state[i]=INVALID;
                invalid_c++;
            }
            else
            {
                state[i]=FAR;
                far_c++;
            }
        }
    }
    catch (...)
    {
        cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
        if (phi) {delete []phi; phi=0;}
        if (plist) {delete []plist; plist=0;}
        if (state) {delete []state; state=0;}
        return false;
    }

    cout<<endl;

    cout<<"invalid_c:"<<invalid_c<<endl<<"far_c:"<<far_c<<endl;

    cout<<"1"<<endl;

    double max_int=0;
    double min_int=INF;
    for(long i=0;i<tol_sz;++i)
    {
        if(state[i]!=INVALID)
        {
            if(inimg1d[i]>max_int) max_int=inimg1d[i];
            if(inimg1d[i]<min_int) min_int=inimg1d[i];
        }
    }
    max_int-=min_int;
    //double li=10;

    cout<<"max_int:"<<max_int<<endl<<"min_int"<<min_int<<endl;

    BasicHeap<HeapElemX> heap;
    map<long,HeapElemX*> elems;

    bool is_remain_FAR=true;

    cout<<"2"<<endl;

    int while_i=0;

    //init heap
    while(is_remain_FAR)
    {
        for(long i=0;i<tol_sz;++i)
        {
            if(state[i]==FAR)
            {
                long index=i;
                state[index]=ALIVE;
                phi[index]=0.0;
                HeapElemX* elem=new HeapElemX(index,phi[index]);
                elem->prev_ind=-1;
                heap.insert(elem);
                elems[index]=elem;

                far_c--;
                alive_c++;

                break;
            }
        }

        cout<<"in while"<<" turn:"<<while_i<<endl;

        int heap_i=0;

        cout<<heap.size()<<endl;



        if(heap.empty())
        {
            is_remain_FAR=false;
        }

        while(!heap.empty())
        {
            HeapElemX* min_elem=heap.delete_min();
            elems.erase(min_elem->img_ind);

            //cout<<"in heap"<<"  turn:"<<heap_i<<endl;

            long min_ind=min_elem->img_ind;
            long prev_ind=min_elem->prev_ind;
            delete min_elem;

            plist[min_ind]=prev_ind;

            state[min_ind]=ALIVE;
            int i=min_ind%sz0;
            int j=(min_ind/sz0)%sz1;
            int k=(min_ind/sz01)%sz2;

            int w,h,d;
            for(int kk=-1;kk<=1;++kk)
            {
                d=k+kk;
                if(d<0||d>=sz2) continue;
                for(int jj=-1;jj<=1;++jj)
                {
                    h=j+jj;
                    if(h<0||h>=sz1) continue;
                    for(int ii=-1;ii<=1;++ii)
                    {
                        w=i+ii;
                        if(w<0||w>=sz0) continue;
                        int offset=ABS(ii)+ABS(jj)+ABS(kk);
                        if(offset == 0 || offset > cnn_type) continue;
                        double factor=(offset==1)?1:((offset==2)?1.414214:((offset==3)?1.732051:0));
                        long index=d*sz01+h*sz0+w;

                        if(state[index]!=ALIVE&&state[index]!=INVALID)
                        {
                            double new_dist=phi[min_ind]+(GI(index)+GI(min_ind))*factor*0.5;
                            long prev_ind=min_ind;

                            if(state[index]==FAR)
                            {
                                phi[index]=new_dist;
                                HeapElemX* elem=new HeapElemX(index,phi[index]);
                                elem->prev_ind=prev_ind;
                                heap.insert(elem);
                                elems[index]=elem;
                                state[index]=TRIAL;
                            }
                            else if(state[index]==TRIAL)
                            {
                                if(phi[index]>new_dist)
                                {
                                    phi[index]=new_dist;
                                    HeapElemX* elem=elems[index];
                                    heap.adjust(elem->heap_id,phi[index]);
                                    elem->prev_ind=prev_ind;
                                }
                            }
                        }
                    }
                }
            }

            heap_i++;
        }

        while_i++;



    }

    map<long, HeapElemX*>::iterator mit = elems.begin(); while(mit != elems.end()){HeapElemX * elem = mit->second; delete elem; mit++;}

    if(phi){delete [] phi; phi = 0;}
    //if(plist){delete [] plist; plist = 0;}
    if(state) {delete [] state; state = 0;}
    return true;

}

template<class T>
void trans_coord(long i,T &marker,int sz0,int sz1,int sz2)
{
    marker.x=(i)%(sz0);marker.y=((i)/(sz0))%sz1;marker.z=((i)/(sz0*sz1))%sz2;
}

bool trans_segs(long* plist,vector<unit_seg> &unit_segs,int sz0,int sz1,int sz2);



template<class T, class TMarker> double markerRadius_accurate(T* &inimg1d, V3DLONG * sz, TMarker & marker, double thresh)
{
    int max_r = MAX(MAX(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
    int r;
    double tol_num, bak_num;
    int mx = marker.x + 0.5;
    int my = marker.y + 0.5;
    int mz = marker.z + 0.5;
    //cout<<"mx = "<<mx<<" my = "<<my<<" mz = "<<mz<<endl;
    V3DLONG x[2], y[2], z[2];

    tol_num = bak_num = 0.0;
    V3DLONG sz01 = sz[0] * sz[1];
    for(r = 1; r <= max_r; r++)
    {
        double r1 = r - 0.5;
        double r2 = r + 0.5;
        double r1_r1 = r1 * r1;
        double r2_r2 = r2 * r2;
        double z_min = 0, z_max = r2;
        for(int dz = z_min ; dz < z_max; dz++)
        {
            double dz_dz = dz * dz;
            double y_min = 0;
            double y_max = sqrt(r2_r2 - dz_dz);
            for(int dy = y_min; dy < y_max; dy++)
            {
                double dy_dy = dy * dy;
                double x_min = r1_r1 - dz_dz - dy_dy;
                x_min = x_min > 0 ? sqrt(x_min)+1 : 0;
                double x_max = sqrt(r2_r2 - dz_dz - dy_dy);
                for(int dx = x_min; dx < x_max; dx++)
                {
                    x[0] = mx - dx, x[1] = mx + dx;
                    y[0] = my - dy, y[1] = my + dy;
                    z[0] = mz - dz, z[1] = mz + dz;
                    for(char b = 0; b < 8; b++)
                    {
                        char ii = b & 0x01, jj = (b >> 1) & 0x01, kk = (b >> 2) & 0x01;
                        if(x[ii]<0 || x[ii] >= sz[0] || y[jj]<0 || y[jj] >= sz[1] || z[kk]<0 || z[kk] >= sz[2]) return r;
                        else
                        {
                            tol_num++;
                            long pos = z[kk]*sz01 + y[jj] * sz[0] + x[ii];
                            if(inimg1d[pos] < thresh){bak_num++;}
                            if((bak_num / tol_num) > 0.0001) return r;
                        }
                    }
                }
            }
        }
    }
    return r;
}

template<class T, class TMarker> double markerRadius_fast(T* &inimg1d, V3DLONG * sz, TMarker & marker, double thresh)
{
    double max_r = MAX(MAX(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
    double ir;
    double tol_num, bak_num;
    double mx = marker.x;
    double my = marker.y;
    double mz = marker.z;
    //cout<<"mx = "<<mx<<" my = "<<my<<" mz = "<<mz<<endl;
    V3DLONG x[2], y[2], z[2];
    V3DLONG sz01 = sz[0] * sz[1];

    double factor = 1.0/sqrt(3.0);
    tol_num = bak_num = 0.0;
    for(ir = 0; ir <= max_r; ir++)
    {
        double r1 = (ir -1)*factor;
        double r2 = ir;
        for(V3DLONG k = r1 ; k < r2; k++)
        {
            for(V3DLONG j = r1; j < r2; j++)
            {
                for(V3DLONG i = r1; i < r2; i++)
                {
                    double dist = i * i + j * j + k * k;
                    if(dist >= ir * ir || dist < (ir -1 )*(ir -1)) continue;
                    x[0] = mx - i, x[1] = mx + i;
                    y[0] = my - j, y[1] = my + i;
                    z[0] = mz - k, z[1] = mz + k;
                    for(char b = 0; b < 8; b++)
                    {
                        char ii = b & 0x01, jj = (b >> 1) & 0x01, kk = (b >> 2) & 0x01;
                        if(x[ii]<0 || x[ii] >= sz[0] || y[jj]<0 || y[jj] >= sz[1] || z[kk]<0 || z[kk] >= sz[2]) return ir;
                        else
                        {
                            tol_num++;
                            long pos = z[kk]*sz01 + y[jj] * sz[0] + x[ii];
                            if(inimg1d[pos] < thresh){bak_num++;}
                            if((bak_num / tol_num) > 0.0001) return ir;
                        }
                    }
                }
            }
        }
    }
    return ir;
}

template<class T>
bool calculate_segs_radius(T* &inimg1d,V3DLONG* sz,vector<unit_seg> &unit_segs,double thresh)
{
    for(long i=0;i<unit_segs.size();++i)
    {
        for(long j=0;j<unit_segs[i].seg_tree.size();++j)
        {
            markerRadius_fast(inimg1d,sz,unit_segs[i].seg_tree[j],thresh);
        }
    }
    return true;
}

bool re_xtrace(V3DPluginCallback2 &callback,PARA_T &p);















































#endif // SOME_FUNCTION_H
