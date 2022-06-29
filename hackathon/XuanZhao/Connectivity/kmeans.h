#ifndef KMEANS_H
#define KMEANS_H

#include <cstdlib>	//for rand()
#include <vector>	//for vector<>
#include <time.h>	//for srand
#include <limits.h>	//for INT_MIN INT_MAX
#include <math.h>
#include <iostream>
#include "v3d_interface.h"

using namespace std;

static double givalss[256] = {22026.5,   20368, 18840.3, 17432.5, 16134.8, 14938.4, 13834.9, 12816.8,
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

template<typename T>
class KMEANS
{
protected:
    //colLen:the dimension of vector;rowLen:the number of vectors
    V3DLONG colLen, rowLen;
    //count to be clustered
    int k;
    //mark the min and max value of a array
    typedef struct MinMax
    {
        T Min;
        T Max;
        MinMax(T min, T max) :Min(min), Max(max) {}
    }tMinMax;
    //distance function
    //reload this function if necessary
    double (*distEclud)(vector<T> &v1, vector<T> &v2);

    //get the min and max value in idx-dimension of dataSet
    tMinMax getMinMax(int idx)
    {
        T min, max;
        dataSet[0].at(idx) > dataSet[1].at(idx) ? (max = dataSet[0].at(idx), min = dataSet[1].at(idx)) : (max = dataSet[1].at(idx), min = dataSet[0].at(idx));

        for (int i = 2; i < rowLen; i++)
        {
            if (dataSet[i].at(idx) < min)	min = dataSet[i].at(idx);
            else if (dataSet[i].at(idx) > max) max = dataSet[i].at(idx);
            else continue;
        }

        tMinMax tminmax(min, max);
        return tminmax;
    }
    //generate clusterCount centers randomly
    void randCent(int clusterCount)
    {
        this->k = clusterCount;
        //init centroids
        centroids.clear();
        vector<T> vec(colLen, 0);
        for (int i = 0; i < k; i++)
            centroids.push_back(vec);

        //set values by column
        srand(time(NULL));
        for (int j = 0; j < colLen; j++)
        {
            tMinMax tminmax = getMinMax(j);
            T rangeIdx = tminmax.Max - tminmax.Min;
            for (int i = 0; i < k; i++)
            {
                /* generate float data between 0 and 1 */
                centroids[i].at(j) = tminmax.Min + rangeIdx * (rand() / (double)RAND_MAX);
            }
        }
    }
    //default distance function ,defined as dis = (x-y)'*(x-y)
    static double defaultDistEclud(vector<T> &v1, vector<T> &v2)
    {
        double sum = 0;
        int size = v1.size();
        for (int i = 0; i < size; i++)
        {
            sum += (v1[i] - v2[i])*(v1[i] - v2[i]);
        }
        return sum;
    }

    static double defaultGrayEclud(vector<T> &v1, vector<T> &v2)
    {
        double sum = 0;
        for (int i = 0; i < 3; i++)
        {
            sum += (v1[i] - v2[i])*(v1[i] - v2[i]);
        }
        sum = sqrt(sum) * givalss[v1[3]] * givalss[v2[3]];
        return sum;
    }

public:
    typedef struct Node
    {
        int minIndex; //the index of each node
        double minDist;
        Node(int idx, double dist) :minIndex(idx), minDist(dist) {}
    }tNode;

    KMEANS(void)
    {
        k = 0;
        colLen = 0;
        rowLen = 0;
        distEclud = defaultGrayEclud; //defaultDistEclud;
    }
    ~KMEANS(void){}
    //data to be clustered
    vector< vector<T> > dataSet;
    //cluster centers
    vector< vector<T> > centroids;
    //mark which cluster the data belong to
    vector<tNode>  clusterAssment;

    //load data into dataSet
    void loadData(vector< vector<T> > data)
    {
        cout<<"load data..."<<endl;
        this->dataSet = data; //kmeans do not change the original data;
        cout<<"COPY data..."<<endl;
        this->rowLen = data.size();
        cout<<"COPY data 2..."<<endl;
        this->colLen = data.at(0).size();
        cout<<"rowLen: "<<rowLen<<" colLen: "<<colLen<<endl;
    }
    //running the kmeans algorithm
    void kmeans(int clusterCount)
    {
        this->k = clusterCount;

        //initial clusterAssment
        this->clusterAssment.clear();
        tNode node(-1, -1);
        for (int i = 0; i < rowLen; i++)
            clusterAssment.push_back(node);

        //initial cluster center
        this->randCent(clusterCount);

        bool clusterChanged = true;
        //the termination condition can also be the loops less than	some number such as 1000
        while (clusterChanged)
        {
            clusterChanged = false;
            for (int i = 0; i < rowLen; i++)
            {
                int minIndex = -1;
                double minDist = INT_MAX;
                for (int j = 0; j < k; j++)
                {
                    double distJI = distEclud(centroids[j], dataSet[i]);
                    if (distJI < minDist)
                    {
                        minDist = distJI;
                        minIndex = j;
                    }
                }
                if (clusterAssment[i].minIndex != minIndex)
                {
                    clusterChanged = true;
                    clusterAssment[i].minIndex = minIndex;
                    clusterAssment[i].minDist = minDist;
                }
            }

            //step two : update the centroids
            for (int cent = 0; cent < k; cent++)
            {
                vector<T> vec(colLen, 0);
                int cnt = 0;
                for (int i = 0; i < rowLen; i++)
                {
                    if (clusterAssment[i].minIndex == cent)
                    {
                        ++cnt;
                        //sum of two vectors
                        for (int j = 0; j < colLen; j++)
                        {
                            vec[j] += dataSet[i].at(j);
                        }
                    }
                }

                //mean of the vector and update the centroids[cent]
                for (int i = 0; i < colLen; i++)
                {
                    if (cnt != 0)	vec[i] /= cnt;
                    centroids[cent].at(i) = vec[i];
                }
            }
        }
    }
};

#endif // KMEANS_H
