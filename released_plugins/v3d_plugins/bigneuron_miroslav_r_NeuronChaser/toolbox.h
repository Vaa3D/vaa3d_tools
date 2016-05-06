#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

unsigned char quantile(unsigned char *a, int a_len, int ratioNum, int ratioDen);

unsigned char quantile(vector<unsigned char> a, int ratioNum, int ratioDen);

float zncc(float *v, int v_len, float v_avg, float *tmplt_hat, float tmplt_hat_sum_sqr);

void descending(float * a, int a_len, int * idx);

void probability_distribution(float * a, int a_len);

void conn3d(unsigned char * inimg, int width, int height, int depth, int * labimg, int maxNrRegions, bool diagonal, int valuesOverDouble, int minRegSize, vector<float> &xc, vector<float> &yc, vector<float> &zc, vector<float> &rc);

unsigned char intermodes_th(unsigned char * image1, long size);

unsigned char otsu_th(unsigned char * image1, long size);

unsigned char maxentropy_th(unsigned char * image1, long size);

/*
 * sorting and array together with corresponding indexes
 */
template<typename T>
void quicksort(T* array, int* idxs, int startIndex, int endIndex) {

    // pivot element - value and index
    T pivot = array[startIndex];
    int pivot_idx = idxs[startIndex];

    int splitPoint;
    if(endIndex > startIndex) {

        splitPoint = quicksort_partition(array, idxs, pivot, startIndex, endIndex);    // returns the pivot position

        array[splitPoint] = pivot;
        idxs[splitPoint] = pivot_idx;

        quicksort(array, idxs, startIndex, splitPoint-1);                     // Quick sort first half
        quicksort(array, idxs, splitPoint+1, endIndex);                       // Quick sort second half
    }

}



template<typename T>
void swp(T &a, T &b) {T temp; temp = a; a = b; b = temp;}


template<typename T>
int quicksort_partition(T* array, int* indxs,    T pivot,    int startIndex, int endIndex) {

    int lBdry = startIndex;
    int rBdry = endIndex;

    while(lBdry < rBdry) {

        while( pivot > array[rBdry] && rBdry > lBdry) rBdry--;

        swp(array[lBdry], array[rBdry]);
        swp(indxs[lBdry], indxs[rBdry]);

        while( pivot <= array[lBdry] && lBdry < rBdry) lBdry++;

        swp(array[rBdry], array[lBdry]);
        swp(indxs[rBdry], indxs[lBdry]);

    }

    return lBdry;

}





template<typename T>
int partition(T* G, int* Gidxs, int first, int last) {

    int pivot = first + rand()%(last - first + 1); // random number between 0 and (last - first + 1) excluded

    swp(G[last], G[pivot]);
    swp(Gidxs[last], Gidxs[pivot]);

    for (int i = first; i < last; i++) {
        if (G[i] > G[last]) {

            swp(G[i], G[first]);
            swp(Gidxs[i],Gidxs[first]);

            first++;
        }
    }

    swp(G[first], G[last]);
    swp(Gidxs[first], Gidxs[last]);

    return first;
}



//void quickselect(T* G, int* Gidxs, int first, int last, int k, T &Gout, int &Gidx);
template<typename T>
void quickselect(T* G, int* Gidxs, int first, int last, int k, T &Gout, int &Gidx) {

    if (first <= last) {

        int pivot = partition(G, Gidxs, first, last);

        if (pivot == k) {
            Gout=G[k];
            Gidx=Gidxs[k];
        }
        else if (pivot > k) {
            quickselect(G, Gidxs, first,    pivot-1,    k, Gout, Gidx);
        }
        else {
            quickselect(G, Gidxs, pivot+1,  last,       k, Gout, Gidx);
        }

    }
    else { // illegal case
        Gout = -1;
        Gidx = -1;
    }

}



template<typename T>
void getKhighest(T* G, int* Gidxs, int Glen, int K, T* Kvals, int* Kidxs) { // G, Gidxs will be modified!

    // quick select Kth highest -> pivot, pivot_idx
    T pivot;
    int pivot_idx;
    quickselect(G, Gidxs, 0, Glen-1, K, pivot, pivot_idx); // quick select K-th highest

    // use pivot, pivot_idx to partition the array around the pivot value
    // (this stage might not add anything - thet are partitioned already, just pick top K from G)
    // it will additionally swap values in G and Gidxs... probably redundant
    int splitP = quicksort_partition(G, Gidxs, pivot, 0, Glen-1);
    swp(G[splitP],  G[0]);  // 0 was the left index
    swp(Gidxs[splitP], Gidxs[0]);

    for (int kk = 0; kk < K; ++kk) {
        Kvals[kk] = G[kk];
        Kidxs[kk] = Gidxs[kk];
    }

}




//template<typename T>
//void getKhighest(T* G, int* Gidxs, int Glen, int K, T* Kvals, int* Kidxs);

template<typename T>
void getKhighestIdxs(T* G, int* Gidxs, int Glen, int K, int* Kidxs) { // G, Gidxs will be modified! Kidxs will stor the indexes
    //cout<<"getKhighest()"<<endl;
    // quick select Kth highest -> pivot, pivot_idx
    T pivot;
    int pivot_idx;
    quickselect(G, Gidxs, 0, Glen-1, K, pivot, pivot_idx); // quick select K-th highest

    // use pivot, pivot_idx to partition the array around the pivot value
    // (this stage might not add anything - thet are partitioned already, just pick top K from G)
    // it will additionally swap values in G and Gidxs... probably redundant
    int splitP = quicksort_partition(G, Gidxs, pivot, 0, Glen-1);
    swp(G[splitP],  G[0]);  // 0 was the left index
    swp(Gidxs[splitP], Gidxs[0]);

    for (int kk = 0; kk < K; ++kk) {
        Kidxs[kk] = Gidxs[kk];
    }

}





template<typename T>
void get1highest(T* G, int Glen, int &Gidx){ // G is not modified

    T max_post = G[0];
    Gidx = 0;
    for (int j = 1; j < Glen; ++j) {
            if (G[j]>max_post) {
                max_post = G[j];//postrtt[Nsteps-1][j];
                Gidx = j;
            }
    }

}





template<typename T>
void print_array(T* arr, int arr_len) {
    for (int var = 0; var < arr_len; ++var) {
        cout << arr[var] << "\t";
    }
    cout << endl;
}



#endif // TOOLBOX_H
