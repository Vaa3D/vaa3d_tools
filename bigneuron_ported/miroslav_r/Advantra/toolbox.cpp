/*
Copyright (C) Erasmus MC. Permission to use this software and corresponding documentation for educational, research, and not-for-profit purposes, without a fee and without a signed licensing agreement, is granted, subject to the following terms and conditions.

IT IS NOT ALLOWED TO REDISTRIBUTE, SELL, OR LEASE THIS SOFTWARE, OR DERIVATIVE WORKS THEREOF, WITHOUT PERMISSION IN WRITING FROM THE COPYRIGHT HOLDER. THE COPYRIGHT HOLDER IS FREE TO MAKE VERSIONS OF THE SOFTWARE AVAILABLE FOR A FEE OR COMMERCIALLY ONLY.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OF ANY KIND WHATSOEVER, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND CORRESPONDING DOCUMENTATION IS PROVIDED "AS IS". THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "toolbox.h"
#include <math.h>
#include "v3d_message.h"
#include <fstream>
#include <float.h>
#include <iostream>

using namespace std;

int mode(vector<int> vals) {

    if (vals.size()>0) {

        int mn = vals[0];
        int mx = vals[0];

        for (int k = 1; k < vals.size(); ++k) {
            if (vals[k]>mx)
                mx = vals[k];
            if (vals[k]<mn)
                mn = vals[k];
        }

        if (mx==mn)
            return mn;

        // draw a histogram
        int hlen = mx-mn+1;
        int hist[hlen];
        for (int i = 0; i < hlen; ++i)
            hist[i] = 0;

        int peakval  = -1;
        int peakfreq = -1;

        for (int i = 0; i < vals.size(); ++i) {
            hist[vals[i]-mn]++;

            if (hist[vals[i]-mn]>peakfreq) {
                peakfreq = hist[vals[i]-mn];
                peakval  = vals[i];
            }
        }

        return peakval;

    }

    return -1;

}

void selec(vector<int> trctags, vector<int> ndetags, int cntth, int excludetag, vector<int> & rchtrctags, vector<int> & rchndetags) {
    // each trace value will have at least one node value in ndevals, assign each histogram componennt that was >0 with corresponding nodetag

    if (trctags.size()!=ndetags.size()) cout<<"WRONG!!"<<endl;

    rchtrctags.clear();
    rchndetags.clear();

    // histogram of trace tags
    if (trctags.size()>0) {

        int mn = trctags[0];
        int mx = trctags[0];

        for (int k = 1; k < trctags.size(); ++k) {
            if (trctags[k]>mx) mx = trctags[k];
            if (trctags[k]<mn) mn = trctags[k];
        }

        if (mx==mn) {
            // there is only one trace tag value, all are the same trace
            if (trctags.size()>=cntth && mn!=excludetag) {
                rchtrctags.push_back(mn); // add it if it differs from excludetag
                rchndetags.push_back(ndetags[ndetags.size()/2]);
            }
        }
        else {
            // there is range of values, draw a histogram of trace tags within the neighbourhood
            int hlen = mx-mn+1;

            int hist[hlen];
            int matchnde[hlen];

            for (int i = 0; i < hlen; ++i) {
                hist[i] = 0;
                matchnde[i] = -1;
            }

            for (int i = 0; i < trctags.size(); ++i) {
                hist[trctags[i]-mn]++;
                matchnde[trctags[i]-mn] = ndetags[i];
            }

            // take those that had count above some value
            for (int i = 0; i < hlen; ++i) {
                int ttag = mn+i;
                if (hist[i]>=cntth && ttag!=excludetag) {
                    rchtrctags.push_back(ttag);
                    rchndetags.push_back(matchnde[i]);
                }
            }

        }

    }

}

unsigned char quantile(unsigned char *a, int a_len, int ratioNum, int ratioDen) {

    int n = a_len; // a.length;
    int i, j, l, m, k;
    double x;

    if (ratioNum>=ratioDen) k = n-1;
    else k = floor(n * ((float)ratioNum/(float)ratioDen));
//    else if ((ratioNum*n) % ratioDen == 0) k = ((ratioNum*n)/ratioDen)-1;
//    else k = (ratioNum*n)/ratioDen;

//    v3d_msg(QString("k=%1\t").arg(k), 0);

    l=0 ; m=n-1;
    while (l < m) {
        x=a[k];
        i = l;
        j = m;
        do {
            while (a[i] < x) i++ ;
            while (x < a[j]) j-- ;
            if (i <= j) {
                float temp = a[i];
                a[i] = a[j];
                a[j] = temp;
                i++ ; j-- ;
            }
        } while (i <= j);
        if (j < k) l = i;
        if (k < i) m = j;
    }

    return a[k];

}

unsigned char quantile(vector<unsigned char> a, int ratioNum, int ratioDen) {

    int n = a.size();
    int i, j, l, m, k;
    double x;

    if (ratioNum>=ratioDen) k = n-1;
    else k = floor(n * ((float)ratioNum/(float)ratioDen));

    l=0 ; m=n-1;
    while (l < m) {
        x=a[k];
        i = l;
        j = m;
        do {
            while (a[i] < x) i++ ;
            while (x < a[j]) j-- ;
            if (i <= j) {
                float temp = a[i];
                a[i] = a[j];
                a[j] = temp;
                i++ ; j-- ;
            }
        } while (i <= j);
        if (j < k) l = i;
        if (k < i) m = j;
    }

    return a[k];

}

//float zncc(float *v, int v_len, float v_avg, float *tmplt_hat, float tmplt_hat_sum_sqr) {

//    float num = 0;
//    float den = 0;

//    for (int i = 0; i < v_len; i++) {
//        num += (v[i] - v_avg) * tmplt_hat[i];
//        den += pow(v[i] - v_avg, 2);
//    }

//    return (float) (num / sqrt(den * tmplt_hat_sum_sqr));

//}

void descending(float * a, int a_len, int * idx) {

//    int[] idx = new int[a.length];
    for (int i=0; i<a_len; i++) idx[i] = i;

    for (int i = 0; i < a_len-1; i++) {
        for (int j = i+1; j < a_len; j++) {
            if (a[j]>a[i]) { // desc.
                float temp 	= a[i];
                a[i]		= a[j];
                a[j] 		= temp;

                int temp_idx 	= idx[i];
                idx[i] 			= idx[j];
                idx[j]			= temp_idx;
            }
        }
    }

//    return idx;

}

void probability_distribution(float * a, int a_len) {

    float sum = 0;
    for (int i = 0; i < a_len; i++) {
        sum += a[i];
    }

    if (sum<=0.00001) {
        for (int i = 0; i < a_len; i++) {
            a[i] = 1.0 / a_len; // they are all equal weight input has all zeros
        }
    }
    else {
        for (int i = 0; i < a_len; i++) {
            a[i] /= sum;
        }
    }
}

void conn3d(unsigned char * inimg, int width, int height, int depth, int * labimg, int maxNrRegions, bool diagonal, int valuesOverDouble, int minRegSize, vector<float> &xc, vector<float> &yc, vector<float> &zc, vector<float> &rc) {

//    cout << "conn3d()..." << endl;

    // this code is ported from fiji's find connected regions class
    // adopted to work on unit8 image stacks provided in form of 1d array
    // https://github.com/fiji/VIB/blob/master/src/main/java/util/Find_Connected_Regions.java
    // connected components of regins with same values in 3d using region growing

    // possible states of the point in the stack (pointstate will get these values)
    static unsigned char IN_QUEUE = 1;
    static unsigned char ADDED_TO_CURRENT_REGION = 2;
    static unsigned char IN_PREVIOUS_REGION = 3;

    int regionNumber = 0;

    long numberOfPointsInStack = (long) width * height * depth;
    unsigned char * pointstate = new unsigned char[numberOfPointsInStack];
    for (long i = 0; i < numberOfPointsInStack; ++i) {
        pointstate[i] = 0;
    }

    unsigned char * sliceDataBytes = new unsigned char[numberOfPointsInStack]; // copy of the slice elements
    for (long i = 0; i < numberOfPointsInStack; ++i) {
        sliceDataBytes[i] = inimg[i];
    }

    int ignoreBeforeX = 0;
    int ignoreBeforeY = 0;
    int ignoreBeforeZ = 0;

//    vector<float*> reglist; // x y z r
    xc.clear();
    yc.clear();
    zc.clear();
    rc.clear();

    while (true) {

        // Find the next pixel that's should be in a region:
        int initial_x = -1;
        int initial_y = -1;
        int initial_z = -1;
        int foundValueInt = -1;

        // Find the next starting point
        bool foundPoint = false;
        for (int z = ignoreBeforeZ; z < depth && ! foundPoint; ++z) {
            int startY = (z == ignoreBeforeZ) ? ignoreBeforeY : 0;
            for (int y = startY; y < height && ! foundPoint; ++y) {
                int startX = (z == ignoreBeforeZ && y == ignoreBeforeY) ? ignoreBeforeX : 0;
                for (int x = startX; x < width; ++x) {
                    long index = (long) width * (z * height + y) + x;

                    if( IN_PREVIOUS_REGION == pointstate[index] )
                                    continue;

                    int value = sliceDataBytes[index];

                    if (value > valuesOverDouble) {
                                    initial_x = x;
                                    initial_y = y;
                                    initial_z = z;
                                    foundValueInt = value;
                                    foundPoint = true;
                                    break;
                                }
                }
            }
        }

        if( foundValueInt == -1 )
            break;

        // Knowing starting point-> start at the next part when we start searching again
        // If x >= width it immediately moves on to the next y as we'd like
        ignoreBeforeX = initial_x + 1;
        ignoreBeforeZ = initial_z;
        ignoreBeforeY = initial_y;

        int vint = foundValueInt;

        //cout << "vint=" << vint << " at " << initial_x<<" , "<<initial_y<<" , " << initial_z << endl;
        int pointsInQueue = 0;
        int queueArrayLength = 4*1024;
        long * queue = new long[queueArrayLength];

        long index = (long) width * (initial_z * height + initial_y) + initial_x;
        pointstate[index] = IN_QUEUE;
        queue[pointsInQueue++] = index;

        int pointsInThisRegion = 0;
        float xmean=0, xmin=FLT_MAX, xmax=-FLT_MAX; // estimation of region centroid and radius rx = (xmax-xmin)/2
        float ymean=0, ymin=FLT_MAX, ymax=-FLT_MAX; // min and max will serve to roughly estimate radius
        float zmean=0;

        //cout << "QUEUE:" << flush; for (int var = 0; var < pointsInQueue; ++var) cout << queue[var] << " | "; cout << endl;

        while (pointsInQueue > 0) {

            long nextIndex = queue[--pointsInQueue];

            long currentPointStateIndex = nextIndex;
            int pz = (int) (nextIndex / (width * height));
            int currentSliceIndex = (int) (nextIndex % (width * height));
            int py = currentSliceIndex / width;
            int px = currentSliceIndex % width;

            pointstate[currentPointStateIndex] = ADDED_TO_CURRENT_REGION;
            sliceDataBytes[currentPointStateIndex] = 0;
            ++pointsInThisRegion;

            // iteratively eatimate centroid coordinates
            float t1 = (float)(pointsInThisRegion-1)/(float)pointsInThisRegion;
            float t2 = 1.0/pointsInThisRegion;
            xmean = t1 * xmean + t2 * px;
            ymean = t1 * ymean + t2 * py;
            zmean = t1 * zmean + t2 * pz;

            xmin = (px<xmin)?px:xmin;
            xmax = (px>xmax)?px:xmax;

            ymin = (py<ymin)?py:ymin;
            ymax = (py>ymax)?py:ymax;

            int x_unchecked_min = px - 1;
            int y_unchecked_min = py - 1;
            int z_unchecked_min = pz - 1;

            int x_unchecked_max = px + 1;
            int y_unchecked_max = py + 1;
            int z_unchecked_max = pz + 1;

            int x_min = max(0,x_unchecked_min);
            int y_min = max(0,y_unchecked_min);
            int z_min = max(0,z_unchecked_min);

            int x_max = min(x_unchecked_max,width-1);
            int y_max = min(y_unchecked_max,height-1);
            int z_max = min(z_unchecked_max,depth-1);

            for (int z = z_min; z <= z_max; ++z) {
                for (int y = y_min; y <= y_max; ++y) {
                    for (int x = x_min; x <= x_max; ++x) {

                        int x_off_centre = (x == x_unchecked_min || x == x_unchecked_max) ? 1 : 0;
                        int y_off_centre = (y == y_unchecked_min || y == y_unchecked_max) ? 1 : 0;
                        int z_off_centre = (z == z_unchecked_min || z == z_unchecked_max) ? 1 : 0;

                        int off_centre_total = x_off_centre + y_off_centre + z_off_centre;

                        // Ignore the start point:
                        if( off_centre_total == 0 )
                            continue; // for loop will drop this case

                        // If we're not including diagonals,
                        if (!diagonal) {
                                    if( x_off_centre + y_off_centre + z_off_centre > 1 ) continue;
                        }

                        int newSliceIndex = y * width + x;
                        long newPointStateIndex = (long) width * (z * height + y) + x;

                        int neighbourValue = sliceDataBytes[newPointStateIndex];

                        if (neighbourValue != vint)
                            continue;

                        if (0 == pointstate[newPointStateIndex]) {

                            pointstate[newPointStateIndex] = IN_QUEUE;

                            if (pointsInQueue == queueArrayLength) { // increase the queue capacity

                                int newArrayLength = (int) (queueArrayLength * 1.2);
                                long * newqueue = new long[newArrayLength];
                                for (int k = 0; k < pointsInQueue; ++k) newqueue[k] = queue[k];
                                delete queue; queue = 0;
                                queue = newqueue;
                                newqueue = 0;
                                queueArrayLength = newArrayLength;
                            }

                            queue[pointsInQueue++] = newPointStateIndex;

                        }
                    }
                }
            }
        } // while queue not empty

        if (pointsInThisRegion < minRegSize) {
            // But we don't want to keep searching these, so set as IN_PREVIOUS_REGION:
            for( long p = 0; p < numberOfPointsInStack; ++p )
                if( pointstate[p] == ADDED_TO_CURRENT_REGION )
                    pointstate[p] = IN_PREVIOUS_REGION;

            continue; // don't add it, it was lower than minRegSize
        }

        ++regionNumber;

        // add the region with estimated centroid and radius
//        float regtoadd[4];
//        regtoadd[0] = xmean;
//        regtoadd[1] = ymean;
//        regtoadd[2] = zmean;
//        regtoadd[3] = ; // average of rx and ry

        float rmean = min((xmax-xmin)/2.0, (ymax-ymin)/2.0);
        //( (xmax-xmin)/2.0 + (ymax-ymin)/2.0 )/2.0;

//        cout << "add R " << regionNumber << " : " << pointsInThisRegion << " voxels, vint=" << vint << "\t" << flush;
        cout << xmean << " " << ymean << " " << zmean << " " << rmean << endl;


//        reglist.push_back(regtoadd);
        xc.push_back(xmean);
        yc.push_back(ymean);
        zc.push_back(zmean);
        rc.push_back(rmean);

        for (long i = 0; i < numberOfPointsInStack; ++i) {
            if (pointstate[i] == ADDED_TO_CURRENT_REGION) {
                labimg[i] = regionNumber;
            }
        }

        for( long p = 0; p < numberOfPointsInStack; ++p )
            if( pointstate[p] == ADDED_TO_CURRENT_REGION )
                pointstate[p] = IN_PREVIOUS_REGION;

        delete queue; queue = 0;

        if ( (maxNrRegions > 0) && (regionNumber >= maxNrRegions) ) {
            cout << "reached limit of " << regionNumber << " regions. stopping." << endl;
            break;
        }

    } // while(true)

    // release memory
    delete pointstate; pointstate = 0;
    delete sliceDataBytes; sliceDataBytes = 0;

//    cout << "DONE!" << endl;

    // return the list of region spheres
//    return reglist;

}

bool bimodalTest(float * y, int ylen) {
//    int len=y.length;
    bool b = false;
    int modes = 0;

    for (int k=1;k<ylen-1;k++){
        if (y[k-1] < y[k] && y[k+1] < y[k]) {
            modes++;
            if (modes>2)
                return false;
        }
    }
    if (modes == 2)
        b = true;
    return b;
}

unsigned char intermodes_th(unsigned char * image1, long size) {

    // J. M. S. Prewitt and M. L. Mendelsohn, "The analysis of cell images," in
    // Annals of the New York Academy of Sciences, vol. 128, pp. 1035-1053, 1966.
    // ported to c++ from Antti Niemisto's Matlab code (GPL)
    // based on the imagej source http://rsb.info.nih.gov/ij/developer/source/ij/process/AutoThresholder.java.html
    // Original Matlab code Copyright (C) 2004 Antti Niemisto
    // See http://www.cs.tut.fi/~ant/histthresh/
    // Assumes a bimodal histogram

    int GRAYLEVEL = 256;

    int hist[GRAYLEVEL];
    for (int i = 0; i < GRAYLEVEL; i++) hist[i] = 0;
    for (long i = 0; i < size; ++i) hist[image1[i]]++;

    int minbin=-1, maxbin=-1;
    for (int i=0; i<GRAYLEVEL; i++) if (hist[i]>0) maxbin = i;
    for (int i=GRAYLEVEL-1; i>=0; i--) if (hist[i]>0) minbin = i;

    int length = (maxbin-minbin)+1;
    float hist1[length];

//    cout << minbin << " -- " << maxbin << " -- " << length << endl;

    for (int i=minbin; i<=maxbin; i++) hist1[i-minbin] = hist[i];

    int iter = 0;
    int threshold=-1;

    while (!bimodalTest(hist1, length) ) {

        //smooth with a 3 point running mean filter
        float previous=0, current=0, next=hist1[0];

        for (int i=0; i<length-1; i++) {
                    previous = current;
                    current = next;
                    next = hist1[i + 1];
                    hist1[i] = (previous+current+next)/3;
        }

        hist1[length-1] = (current+next)/3;
        iter++;
        if (iter>10000) {
                    threshold = -1; // so that nothing is segmented
                    cout <<"Intermodes Threshold not found after 10000 iterations." << endl;
                    return threshold;
                }
    }

    // The threshold is the mean between the two peaks.
    int tt=0;

    for (int i=1; i<length - 1; i++) {
        if (hist1[i-1] < hist1[i] && hist1[i+1] < hist1[i]){
            tt += i;
            //IJ.log("mode:" +i);
        }
    }

    threshold = (int) floor(tt/2.0);
    return threshold+minbin;

}

unsigned char otsu_th(unsigned char * image1, long size) {

    int GRAYLEVEL = 256;

    // binarization by Otsu
    int hist[GRAYLEVEL];
    float prob[GRAYLEVEL], omega[GRAYLEVEL];   // prob of graylevels
    float myu[GRAYLEVEL];                      // mean value for separation
    float max_sigma, sigma[GRAYLEVEL];         // inter-class variance
    int i, x, y;                               // loop variable
    unsigned char threshold;                   // threshold for binarization

    // histogram
    for (i = 0; i < GRAYLEVEL; i++) hist[i] = 0;
    for (long i = 0; i < size; ++i) {
        hist[image1[i]]++;
    }

    // calculation of probability density
    for ( i = 0; i < GRAYLEVEL; i ++ ) {
        prob[i] = (float)hist[i] / size;
    }

    // omega & myu generation
    omega[0] = prob[0];
    myu[0] = 0.0;                               // 0.0 times prob[0] equals zero
    for (i = 1; i < GRAYLEVEL; i++) {
        omega[i]    = omega[i-1] + prob[i];
        myu[i]      = myu[i-1] + i*prob[i];
    }

    // sigma maximization
    // sigma stands for inter-class variance
    // and determines optimal threshold value
    threshold = 0;
    max_sigma = 0.0;
    for (i = 0; i < GRAYLEVEL-1; i++) {
        if (omega[i] != 0.0 && omega[i] != 1.0)
            sigma[i] = pow(myu[GRAYLEVEL-1]*omega[i] - myu[i], 2) /
            (omega[i]*(1.0 - omega[i]));
        else
            sigma[i] = 0.0;
        if (sigma[i] > max_sigma) {
            max_sigma = sigma[i];
            threshold = i;
        }
    }

    return threshold;

}

unsigned char maxentropy_th(unsigned char * image1, long size) {

    // binarization by MaxEntropy
    int GRAYLEVEL = 256;

    int hist[GRAYLEVEL];
    for (int i = 0; i < GRAYLEVEL; i++) hist[i] = 0;
    for (long i = 0; i < size; ++i) {
        hist[image1[i]]++;
    }

    // Normalize histogram, that is makes the sum of all bins equal to 1.
    float sum = 0;
    for (int i = 0; i < GRAYLEVEL; ++i) {
        sum += hist[i];
    }

    float normalizedHist[GRAYLEVEL];
    for (int i = 0; i < GRAYLEVEL; i++) {
        normalizedHist[i] = hist[i] / sum;
    }

    float pT[GRAYLEVEL];
    pT[0] = normalizedHist[0];
    for (int i = 1; i < GRAYLEVEL; i++) {
        pT[i] = pT[i - 1] + normalizedHist[i];
    }

    // Entropy for black and white parts of the histogram
    float epsilon = FLT_MIN;//Double.MIN_VALUE;
    float hB[GRAYLEVEL];// = new double[hist.length];
    float hW[GRAYLEVEL];// = new double[hist.length];

    for (int t = 0; t < GRAYLEVEL; t++) {
        // Black entropy
        if (pT[t] > epsilon) {
          float hhB = 0;
          for (int i = 0; i <= t; i++) {
            if (normalizedHist[i] > epsilon) {
              hhB -= normalizedHist[i] / pT[t] * log(normalizedHist[i] / pT[t]);
            }
          }
          hB[t] = hhB;
        } else {
          hB[t] = 0;
        }

        // White  entropy
        double pTW = 1 - pT[t];
        if (pTW > epsilon) {
          float hhW = 0;
          for (int i = t + 1; i < GRAYLEVEL; ++i) {
            if (normalizedHist[i] > epsilon) {
              hhW -= normalizedHist[i] / pTW * log(normalizedHist[i] / pTW);
            }
          }
          hW[t] = hhW;
        } else {
          hW[t] = 0;
        }
      }

      // Find histogram index with maximum entropy
      float jMax = hB[0] + hW[0];
      unsigned char tMax = 0;
      for (int t = 1; t < GRAYLEVEL; ++t) {
        double j = hB[t] + hW[t];
        if (j > jMax) {
          jMax = j;
          tMax = t;
        }
      }

      return tMax;

}

