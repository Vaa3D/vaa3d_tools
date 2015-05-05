#include "graphCut.h"
#include <climits>

GraphCut::GraphCut()
{
  m_node_ids = 0;
  m_graph = 0;
  running_maxflow = false;
  subCube = 0;
  subX = 0;
  subY = 0;
  subZ = 0;
}

GraphCut::~GraphCut() {
  // Free memory
  if(m_node_ids != 0)
    {
      delete[] m_node_ids;
    }
  if(m_graph!=0)
    delete m_graph;
  if(subCube != 0)
    delete subCube;
}

void GraphCut::applyCut(LabelImageType* ptrLabelInput, Cube* inputCube, unsigned char* output_data, int ccId, unsigned char* scoreImage)
{
    ulong wh = ni*nj;
    ulong n = wh*nk;
    int nodeType;
    ulong inputCube_sliceSize = inputCube->width*inputCube->height;
    ulong outputCubeIdx;
    ulong cubeIdx = 0;
    const int nh_size = 1;
    ulong sx,sy,sz,ex,ey,ez;
    LabelImageType::IndexType index;
    for(int z = 0;z < nk; z++) {
        for(int y = 0;y < nj; y++) {
            for(int x = 0;x < ni;x++) {
                outputCubeIdx = ((z+subZ)*inputCube_sliceSize) + ((y+subY)*inputCube->width) + (x+subX);
                //output_data[outputCubeIdx] = inputCube->at(i+subX,j+subY,k+subZ);

                if(scoreImage[outputCubeIdx] == 0)  // background
                    continue;

                cubeIdx = (z*wh) + (y*ni) + x;
                nodeType = m_graph->what_segment(m_node_ids[cubeIdx]);

                sx = max(0,x-nh_size);
                ex = min(x+nh_size,ni-1);
                sy = max(0,y-nh_size);
                ey = min(y+nh_size,nj-1);
                sz = max(0,z-nh_size);
                ez = min(z+nh_size,nk-1);
                for(int _x = sx; _x <= ex; _x++)
                for(int _y = sy; _y <= ey; _y++)
                for(int _z = sz; _z <= ez; _z++)
                {
                    //if(inputCube->at(_x+subX,_y+subY,_z+subZ) != 0) {
                    index[0] = _x + subX;
                    index[1] = _y + subY;
                    index[2] = _z + subZ;
                    if(ptrLabelInput->GetPixel(index) == ccId) {
                        cubeIdx = (_z*wh) + (_y*ni) + _x;
                        if(m_graph->what_segment(m_node_ids[cubeIdx]) != nodeType)
                        {
                            outputCubeIdx = ((_z+subZ)*inputCube_sliceSize) + ((_y+subY)*inputCube->width) + (_x+subX);
                            //printf("Cutting (%d,%d,%d)\n",_x+subX,_y+subY,_z+subZ);
                            output_data[outputCubeIdx] = 0;
                        }
                    }
                }
            }
        }
    }
}

// caller is responsible for freeing memory
void GraphCut::getOutputGivenSeeds(Cube* inputCube, uchar*& output_data)
{
  ulong inputCube_sliceSize = inputCube->width*inputCube->height;
  ulong cubeSize = inputCube_sliceSize*inputCube->depth;
  ulong cubeIdx = 0;
  for(int k=0;k<nk;k++)
    for(int j = 0;j<nj;j++)
      for(int i = 0;i<ni;i++,cubeIdx++) {
          ulong outputCubeIdx = ((k+subZ)*inputCube_sliceSize) + ((j+subY)*inputCube->width) + (i+subX);
          assert(outputCubeIdx < cubeSize);
          if(!inputCube->data || inputCube->at(i+subX,j+subY,k+subZ) != 0) {
            if(m_graph->what_segment(m_node_ids[cubeIdx]) == GraphType::SOURCE) {
                output_data[outputCubeIdx] = 128;
            } else {
                output_data[outputCubeIdx] = 255;
            }
          }
        }
}

void GraphCut::getOutput(Cube* inputCube, uchar*& output_data)
{
  ulong inputCube_sliceSize = inputCube->width*inputCube->height;
  ulong cubeSize = inputCube_sliceSize*inputCube->depth;
  ulong cubeIdx = 0;
  for(int k=0;k<nk;k++)
    for(int j = 0;j<nj;j++)
      for(int i = 0;i<ni;i++,cubeIdx++) {
          ulong outputCubeIdx = ((k+subZ)*inputCube_sliceSize) + ((j+subY)*inputCube->width) + (i+subX);
          assert(outputCubeIdx < cubeSize);
          if(m_graph->what_segment(m_node_ids[cubeIdx]) == GraphType::SOURCE) {
              output_data[outputCubeIdx] = 128;
          } else {
              output_data[outputCubeIdx] = 255;
          }
      }
}

void GraphCut::getCubeSize(ulong& cubeWidth,
                           ulong& cubeHeight,
                           ulong& cubeDepth)
{
  cubeWidth = subCube->width;
  cubeHeight = subCube->height;
  cubeDepth = subCube->depth;
}


unsigned long GraphCut::at(int i, int j, int k)
{
  return ((unsigned long)k*nij)+j*ni+i;
}

void GraphCut::displayCounts()
{
    ulong n = ni*nj*nk;
    ulong nSources = 0;
    ulong nSinks = 0;
    for(int cubeIdx=0; cubeIdx<n; cubeIdx++) {
        if(m_graph->what_segment(m_node_ids[cubeIdx]) == GraphType::SOURCE)
            nSources++;
        else
            nSinks++;
    }

    printf("nSources=%ld, nSinks=%ld\n", nSources,nSinks);
}

/*
 * Run Min-Cut/Max-Flow algorithm for energy minimization
 * Energy function is defined as E = B + R
 * B = Boundary term, R = Regional term
 */
void GraphCut::run_maxflow(Cube* cube,
                           vector<Point>& sourcePoints, vector<Point>& sinkPoints,
                           float sigma, int minDist, eUnaryWeights unaryType, Cube* scores)
{
  float weightToSource;
  float weightToSink;
  float weight;
  // TODO : compute the weight k
  // (weight of edge between mark object to the source or mark background to the sink)
  //float K = FLT_MAX;
  float K = 1e8;
  float MAX_WEIGHT = 1e2;
  ulong scoreCube_sliceSize = scores->width*scores->height;

  running_maxflow = true;

  if(subCube) {
    printf("[GraphCuts] Performing maxflow on sub-cube\n");
    printf("[GraphCuts] Original size=(%ld,%ld,%ld)\n",cube->width,cube->height,cube->depth);
    cube = subCube;
    printf("[GraphCuts] New size=(%ld,%ld,%ld)\n",cube->width,cube->height,cube->depth);
  }

  ni = cube->width;
  nj = cube->height;
  nk = cube->depth;
  nij = ni*nj;

  // Free memory
  if(m_node_ids!=0) {
    delete[] m_node_ids;
  }
  if(m_graph!=0) {
    delete m_graph;
  }

  // compute max scores in sub cube (use dimension of cube as dimension of scores correspond to the whole volume)
  int maxScore = -INT_MAX;
  for(int k = 0;k < cube->depth;k++) {
    for(int j = 0;j < cube->height;j++) {
      for(int i = 0;i < cube->width;i++) {
        int sc = scores->at(i+subX,j+subY,k+subZ);
        if(maxScore < sc) {
           maxScore = sc;
        }
      }
    }
  }
  printf("[GraphCuts] maxScore = %d\n", maxScore);

  printf("[GraphCuts] graph size = (%d,%d,%d)\n", ni, nj, nk);

  // TODO : Compute correct parameters
  ulong nNodes = ni*nj*nk;
  ulong nEdges = nNodes*3;
  m_graph = new GraphType(nNodes, nEdges);

  m_node_ids = new GraphType::node_id[nNodes];
  for(ulong i = 0;i < nNodes; i++) {
    m_node_ids[i] = m_graph->add_node();
  }  

  float* histoSource = 0;
  float* histoSink = 0;
  const int nbItemsPerBin = 25;
  const int histoSize = 255/nbItemsPerBin;
  switch(unaryType) {
  case UNARY_SCORE:
      break;
  case UNARY_HISTOGRAMS:
    // Compute histogram for boundary term
    histoSource = new float[histoSize];
    int binId;
    memset(histoSource,0,histoSize*sizeof(float));
    for(vector<Point>::iterator itPoint=sourcePoints.begin();
        itPoint != sourcePoints.end();itPoint++) {
      binId = (int)cube->at(itPoint->x, itPoint->y, itPoint->z)/nbItemsPerBin - 1;
      if(binId < 0)
        binId = 0;
      if(binId >= histoSize)
        binId = histoSize - 1;
      histoSource[binId]++;
    }

    histoSink = new float[histoSize];
    memset(histoSink,0,histoSize*sizeof(float));
    for(vector<Point>::iterator itPoint=sinkPoints.begin();
        itPoint != sinkPoints.end();itPoint++) {
      binId = (int)cube->at(itPoint->x, itPoint->y, itPoint->z)/nbItemsPerBin - 1;
      if(binId < 0)
        binId = 0;
      if(binId >= histoSize)
        binId = histoSize - 1;
      histoSink[binId]++;
    }

    // Normalize histograms
    for(int i = 0;i<histoSize;i++) {
      histoSource[i] /= histoSize;
      histoSink[i] /= histoSize;
    }

    printf("[GraphCuts] Histograms\n");
    printf("Source:\n");
    for(int i=0;i<histoSize;i++)
      printf("%f ",histoSource[i]);
    printf("\nSink:\n");
    for(int i=0;i<histoSize;i++)
      printf("%f ",histoSink[i]);
    printf("\n\n");

    break;
  default:
      break;
  }

  nEdges = 0;
  nNodes = 0;
  ulong dist;
  int seedX, seedY, seedZ;
  minDist *= minDist; //squared distance

  // Compute weights to source and sink nodes
  ulong nij = ni*nj;
  weightToSource = K;
  weightToSink = 0;
  for(vector<Point>::iterator itPoint=sourcePoints.begin();
      itPoint != sourcePoints.end();itPoint++) {
    seedX = itPoint->x - subX;
    seedY = itPoint->y - subY;
    seedZ = itPoint->z - subZ;
    ulong nodeIdx = seedZ*nij + seedY*ni + seedX;
    printf("(%d,%d,%d) %ld : %f/%f\n",seedX,seedY,seedZ,nodeIdx,weightToSource,weightToSink);
    m_graph->add_tweights(m_node_ids[nodeIdx],weightToSource,weightToSink);
  }

  weightToSource = 0;
  weightToSink = K;
  for(vector<Point>::iterator itPoint=sinkPoints.begin();
      itPoint != sinkPoints.end();itPoint++) {
    seedX = itPoint->x - subX;
    seedY = itPoint->y - subY;
    seedZ = itPoint->z - subZ;
    ulong nodeIdx = seedZ*nij + seedY*ni + seedX;
    printf("(%d,%d,%d) %ld : %f/%f\n",seedX,seedY,seedZ,nodeIdx,weightToSource,weightToSink);
    m_graph->add_tweights(m_node_ids[nodeIdx],weightToSource,weightToSink);
  }

  ulong nodeIdx = 0;
  for(int k = 0;k<nk;k++) {
    for(int j = 0;j<nj;j++) {
      for(int i = 0;i<ni;i++,nodeIdx++) {

        if(scores->at(i+subX,j+subY,k+subZ) == 0) // background
            continue;

        ++nNodes;

        if(unaryType == UNARY_SCORE || unaryType == UNARY_HISTOGRAMS) {
            // Compute regional term
            weightToSink = 0;
            weightToSource = 0;

            // Compute weights to source and sink nodes
            for(vector<Point>::iterator itPoint=sourcePoints.begin();
                itPoint != sourcePoints.end();itPoint++) {
              seedX = itPoint->x - subX;
              seedY = itPoint->y - subY;
              seedZ = itPoint->z - subZ;
              dist = (seedX-i)*(seedX-i) + (seedY-j)*(seedY-j) + (seedZ-k)*(seedZ-k);
              if(dist <= minDist) {
                //printf("[GraphCuts] Source found %d %d %d\n", i, j, k);
                weightToSource = K;
                break;
              }
            }

            for(vector<Point>::iterator itPoint=sinkPoints.begin();
                itPoint != sinkPoints.end();itPoint++) {
              seedX = itPoint->x - subX;
              seedY = itPoint->y - subY;
              seedZ = itPoint->z - subZ;
              dist = (seedX-i)*(seedX-i) + (seedY-j)*(seedY-j) + (seedZ-k)*(seedZ-k);
              if(dist <= minDist) {
                //printf("[GraphCuts] Sink found %d %d %d\n", i, j, k);
                weightToSink = K;
                break;
              }
            }

            switch(unaryType) {
            case UNARY_SCORE:
            {
                ulong cubeIdx = ((k+subZ)*scoreCube_sliceSize) + ((j+subY)*scores->width) + (i+subX);
                if(weightToSource != K && weightToSink != K) {
                    weightToSource = scores->data[cubeIdx];
                    weightToSink = max(0,maxScore-scores->data[cubeIdx]);
                }
                break;
            }
            case UNARY_HISTOGRAMS:
            {
              if(weightToSource != K) {
                // Get value from histogram
                int binId = (int)(cube->at(i,j,k)/nbItemsPerBin) - 1;
                if(binId >= histoSize) {
                  //printf("binId %d >= histoSize\n", binId);
                  binId = histoSize - 1;
                }
                if(binId < 0) {
                  //printf("binId %d < 0\n", binId);
                  binId = 0;
                }
                weightToSource = histoSource[binId];
              }
              if(weightToSink != K) {
                // Get value from histogram
                int binId = (int)(cube->at(i,j,k)/nbItemsPerBin) - 1;
                if(binId >= histoSize) {
                  //printf("binId >= histoSize\n");
                  binId = histoSize - 1;
                }
                if(binId < 0) {
                  binId = 0;
                }
                weightToSink = histoSink[binId];
              }
              break;
            }
            default:
                break;
            }

            if(weightToSource != 0 || weightToSink != 0) {
                printf("(%d,%d,%d) %ld : %f/%f\n",i,j,k,nodeIdx,weightToSource,weightToSink);
                m_graph->add_tweights(m_node_ids[nodeIdx],weightToSource,weightToSink);
            }
        }

        // Compute boundary term
        // B(p,q) = exp(-(Ip - Iq)^2 / 2*sigma)/dist(p,q)
        if(i+1 < ni && (m_node_ids[nodeIdx] != m_node_ids[at(i+1,j,k)]) && (scores->at(i+1+subX,j+subY,k+subZ) != 0))
          {
            //weight = exp(-pow((cube->at(i,j,k)-cube->at(i+1,j,k))*sigma,2.f));
            weight = pow(cube->at(i,j,k)-cube->at(i+1,j,k),2.0f);
            weight = ((weight==0)?MAX_WEIGHT:1.0f/weight)*sigma;
            m_graph->add_edge(m_node_ids[nodeIdx], m_node_ids[at(i+1,j,k)], weight, weight);
            //if(i==4)
            //  printf("(%d,%d,%d)-(%d,%d,%d) : %d %d %d %f\n",i,j,k,i+1,j,k,cube->at(i,j,k),cube->at(i+1,j,k),cube->at(i,j,k)-cube->at(i+1,j,k),weight);
            nEdges++;
          }

        if(j+1 < nj && (m_node_ids[nodeIdx] != m_node_ids[at(i,j+1,k)]) && (scores->at(i+subX,j+1+subY,k+subZ) != 0))
          {
            //weight = exp(-pow((cube->at(i,j,k)-cube->at(i,j+1,k))*sigma,2.f));
            weight = pow(cube->at(i,j,k)-cube->at(i,j+1,k),2.0f);
            weight = ((weight==0)?MAX_WEIGHT:1.0f/weight)*sigma;
            m_graph->add_edge(m_node_ids[nodeIdx], m_node_ids[at(i,j+1,k)], weight, weight);
            //printf("(%d,%d,%d)-(%d,%d,%d) : %f\n",i,j,k,i,j+1,k,weight);
            nEdges++;
          }
        if(k+1 < nk && (m_node_ids[nodeIdx] != m_node_ids[at(i,j,k+1)]) && (scores->at(i+subX,j+subY,k+1+subZ) != 0))
          {
            //weight = exp(-pow((cube->at(i,j,k)-cube->at(i,j,k+1))*sigma,2.f));
            weight = pow(cube->at(i,j,k)-cube->at(i,j,k+1),2.0f);
            weight = ((weight==0)?MAX_WEIGHT:1.0f/weight)*sigma;
            m_graph->add_edge(m_node_ids[nodeIdx], m_node_ids[at(i,j,k+1)], weight, weight);
            //printf("(%d,%d,%d)-(%d,%d,%d) : %f\n",i,j,k,i,j,k+1,weight);
            nEdges++;
          }
      }
    }
  }

  printf("[GraphCuts] %d nodes added\n", nNodes);
  printf("[GraphCuts] %d edges added\n", nEdges);

  printf("[GraphCuts] Computing max flow\n");
  int flow = m_graph->maxflow();

  // Cleaning
  if(histoSource) {
    delete[] histoSource;
  }
  if(histoSink) {
    delete[] histoSink;
  }

  printf("[GraphCuts] Max flow=%d\n", flow);
  running_maxflow = false;
}

