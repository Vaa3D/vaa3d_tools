#include "zkmeansclustering.h"

#include <float.h>
#include <cmath>
#include <vector>
#include <set>

#include "zrandomgenerator.h"
#include "tz_geo3d_utils.h"
#include "tz_iarray.h"

using namespace std;

ZKmeansClustering::ZKmeansClustering()
{
  m_maxIter = 100;
  m_minDelta = 10.0;
}

vector<int>& ZKmeansClustering::runKmeans(const Object_3d *obj, int nclus,
                                  const ZDoubleVector &weightArray)
{
  ZRandomGenerator randGenerator;
  randGenerator.setSeed(1);

  double bestEnergy = Infinity;
  m_centerArray.resize(nclus);

  vector<ZDoubleVector> bestCC;

  for (int randIter = 1; randIter <= 5; randIter++) {
    vector<int> randIndexArray = randGenerator.randperm(obj->size);

    //For testing
    //for (size_t i = 0; i < randIndexArray.size(); i++) {
    //  randIndexArray[i] = i;
    //}
    ///////////////

    //Initialize the seed
    for (int k = 0; k < nclus; k++) {
      m_centerArray[k].resize(3);
      m_centerArray[k][0] = obj->voxels[randIndexArray[k]][0];
      m_centerArray[k][1] = obj->voxels[randIndexArray[k]][1];
      m_centerArray[k][2] = obj->voxels[randIndexArray[k]][2];
    }

    double sse0 = Infinity;
    for (int iter = 0; iter < m_maxIter; iter++) {
      vggKmiter(obj, weightArray);
      if (sse0 - m_sse < m_minDelta) {
        break;
      }
      sse0 = m_sse;
    }

    if (bestEnergy > m_sse) {
      bestCC = m_centerArray;
      bestEnergy = m_sse;
    }
  }

  set<ZDoubleVector> centerSet;

  //Remember to implement == in ZDoubleVector
  for (size_t i = 0; i < bestCC.size(); i++) {
    for (size_t j = 0; j < bestCC[i].size(); j++) {
      bestCC[i][j] = floor(bestCC[i][j]);
    }
    centerSet.insert(bestCC[i]);
  }

  m_centerArray.clear();
  for (set<ZDoubleVector>::const_iterator iter = centerSet.begin();
       iter != centerSet.end(); ++iter) {
    m_centerArray.push_back(*iter);
  }

  //Asign the labels
  m_labelArray.resize(obj->size, 0);
  for (size_t i = 0; i < obj->size; i++) {
    double dmin = Geo3d_Dist_Sqr(obj->voxels[i][0], obj->voxels[i][1],
                                 obj->voxels[i][2],
                                 m_centerArray[0][0], m_centerArray[0][1],
                                 m_centerArray[0][2]);
    for (size_t j = 1; j < m_centerArray.size(); j++) {
      double d = Geo3d_Dist_Sqr(obj->voxels[i][0], obj->voxels[i][1],
                                obj->voxels[i][2],
                                m_centerArray[j][0], m_centerArray[j][1],
                                m_centerArray[j][2]);
      if (d < dmin) {
        dmin = d;
        m_labelArray[i] = j;
      }
    }
  }

  /*
  iarray_write("/Users/zhaot/Work/neutube/neurolabi/data/test.dat", &(m_labelArray[0]),
               m_labelArray.size());
*/
  return m_labelArray;
}

vector<int>& ZKmeansClustering::runKmeans(const ZObject3d &obj, int nclus,
                                  const ZDoubleVector &weightArray)
{
  return runKmeans(obj.c_obj(), nclus, weightArray);
}

void ZKmeansClustering::vggKmiter(const Object_3d *obj,
                                  const ZDoubleVector &weightArray)
{
  int npts = obj->size;
  int dim = 3;
  int nclus = m_centerArray.size();
  double *px, *pcx;

  double *X = new double[obj->size * dim];
  px = X;
  for (size_t i = 0; i < obj->size; i++) {
    *(px++) = obj->voxels[i][0];
    *(px++) = obj->voxels[i][1];
    *(px++) = obj->voxels[i][2];
  }

  double *CX = new double[m_centerArray.size() * dim];
  pcx = CX;
  for (size_t i = 0; i < m_centerArray.size(); i++) {
    *(pcx++) = m_centerArray[i][0];
    *(pcx++) = m_centerArray[i][1];
    *(pcx++) = m_centerArray[i][2];
    m_centerArray[i][0] = 0.0;
    m_centerArray[i][1] = 0.0;
    m_centerArray[i][2] = 0.0;
  }

  m_sse = 0.0;

  double *CN = (double *) calloc(nclus, sizeof(double));

  int i, j, k;
  for (i = 0, px = X; i < npts; i++, px += dim)
  {
    double dmin = DBL_MAX;
    int c = 0;
    double weight = weightArray[i];
    for (j = 0, pcx = CX; j < nclus; j++, pcx += dim)
    {
      double d = 0.0;

      for (k = 0; k < dim; k++) {
        double dx = px[k] - pcx[k];
        d += dx * dx;
      }

      if (d < dmin)
      {
        dmin = d;
        c = j;
      }
    }

    m_sse += dmin * weight * weight;

    CN[c]+= weight;

    for (k = 0; k < dim; k++) {
      m_centerArray[c][k] += px[k] * weight;
    }
    /*
    pcxp = CXp + c * dim;
    for (k = 0; k < dim; k++)
      pcxp[k] += (px[k]*weightArray[i]);
      */
  }

  for (j = 0; j < nclus; j++)
  {
    if (CN[j])
    {
      for (k = 0; k < dim; k++)
        m_centerArray[j][k] /= CN[j];
    }
  }

  delete []CX;
  delete []X;
  free(CN);
}
