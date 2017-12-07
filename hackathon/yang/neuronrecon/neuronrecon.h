// "optimal constructing of neuron trees from voxels"
// -Yang
// 09/12/2017


#ifndef _NEURONRECON_H_
#define _NEURONRECON_H_

//
#include <QtGlobal>
#include <vector>
#include "v3d_interface.h"
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include <iostream>
#include "basic_4dimage.h"
#include "algorithm"
#include <string.h>
#include <cmath>
#include <climits>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <string>
#include <tuple>

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkTimeProbe.h"
#include "itkImageRegionIterator.h"

#include "itkOpenCLUtil.h"
#include "itkGPUImage.h"
#include "itkGPUKernelManager.h"
#include "itkGPUContextManager.h"
#include "itkGPUImageToImageFilter.h"
#include "itkGPUGradientAnisotropicDiffusionImageFilter.h"

#include "itkTIFFImageIOFactory.h"

#include <Eigen/Eigenvalues>
#include "nanoflann.hpp"

using namespace std;
using namespace nanoflann;

//
typedef tuple<float,float,long,long> PairCompareType;

//
class Plane
{
public:
    Plane();
    ~Plane();

public:
    float a,b,c,d;
};

//
class Vector
{
public:
    Vector();
    ~Vector();

public:
    Vector* vcross(Vector *a, Vector *b);
    float vdot(Vector *a, Vector *b);
    float vmag(Vector *a);
    float recip_vmag(Vector *a);
    Vector* vnorm(Vector *a);
    void info();

public:
    float dx,dy,dz;
};

//
class Point
{
public:
    Point();
    Point(float x, float y, float z);
    ~Point();

public:
    void setLocation(float x, float y, float z);
    void setRadius(float r);
    void setValue(float v);
    bool isSamePoint(Point p);
    void info();
    bool hasChildren();

public:
    float x, y, z; // location
    float radius; // radius
    float val; // intensity value
    vector<long> parents; // n (same definition in .swc)
    vector<long> children; // n (same definition in .swc) used for detect branch points
    V3DLONG n; // #
    int type; // -1 cell body; 0 tip point; 1 regular point; 3 branch point

    bool visited;
    int connected; // 0 not connected; 1 connected to one other point; 2 connected two other points; ...
    V3DLONG pre, next; // index
    vector<V3DLONG> nn; // k nearest neighbors (indices)

    bool neighborVisited; // sort points from soma/cell body

    bool treeConnected; // connected to a tree
    int lengthLine; // if the point is root, how many points connected to it
    bool isBranch; // is the point acandidates.begin(), candidates.end(), branch point
    bool isSoma; // is the point the soma
    int nLine; // # of the line
    bool isolated; // not connected to other point(s)
    bool isNoise;
};

typedef pair<Point, Point> PointPairType;

//
class PointPair
{
public:
    PointPair(Point a, Point b);
    ~PointPair();

public:
    bool samePair(Point a, Point b);

public:
    bool visited;
    pair<Point, Point> pointpair;
};

class Pairs
{
public:
    Pairs();
    ~Pairs();

public:
    void resetStatus();
    void appendPair(Point a, Point b);

public:
    vector<PointPair> pairs;
};

//
class NCPointCloud
{
    // breadth-first sort
    // depth-first line construction

public:
    NCPointCloud();
    ~NCPointCloud();

public:

    // load a single/multiple .swc file(s)
    int getPointCloud(QStringList files);

    // save as a .apo file
    int savePointCloud(QString filename, int format=0);

    // save as a .swc file
    int saveNeuronTree(NCPointCloud pc, QString filename);

    //
    int addPointFromNeuronTree(NeuronTree nt);

    int getBranchPoints(QString filename);
    int getNeurites(QString filename);

    bool findNextUnvisitPoint(unsigned long &index);

    int resample();

    // distance between point a and point b
    float distance(Point a, Point b);

    // compute angle between (a, b) and (b, c)
    float getAngle(Point a, Point b, Point c);

    // shortest distance between point p and line segment (a, b)
    float distPoint2LineSegment(Point a, Point b, Point p);

    // judge whether a point important
    bool isConsidered(unsigned long &index, float m);

    // k nearest neighbor search
    int knn(int k, float radius=0);

    //
    int connectPoints2Lines(QString infile, QString outfile, int k, float angle, float m);

    //
    int connectPoints(int k, float maxAngle, float m);

    //
    int removeNoise();

    //
    int assembleFragments(int k);

    //
    int tracing(QString infile, QString outfile, int k, float angle, float m, float nn);

    //
    int trace();

    // cost func
    int minAngle(unsigned long &loc, float maxAngle);

    // knn sort from the point with the biggest radius
    int ksort(NCPointCloud pc, int k);

    // delete duplicated points
    int delDuplicatedPoints();

    // copy
    int copy(NCPointCloud pc);

    //
    int knnMeanStddevDist(float &mean, float &stddev, int k=2);

    //
    int knnMaxDist(float &max, int k=2);

    //
    int resetVisitStatus();

    //
    int reverseLineSegment(long idx, long size);
    int reverseLineSegment(long idx);

    //
    int endpoints();

    //
    int connectLineSegments(long rooti, long tipj, float angle);

    //
    int append(Point p);

    //
    bool checkloop(Point p, Point q);

    //
    bool allendpointvisited();

    //
    Point pplusv(Point *p, Vector *v);
    float point_plane_dist(Point *a, Plane *P);
    Point plerp(Point *a, Point *b, float t);
    Point intersect_line_plane(Point *a, Point *b, Plane *M);
    Point intersect_dline_plane(Point *a, Vector *adir, Plane *M);
    Plane* plane_from_two_vectors_and_point(Vector *u, Vector *v, Point *p);
    int line_line_closest_point(Point &pA, Point &pB, Point *a, Vector *adir, Point *b, Vector *bdir);
    int mergeLines(float maxAngle);
    long indexofpoint(long n);
    int isolatedPoints();

public:
    vector<Point> points; // data
    Pairs skipConnecting; // for connecting line segments
    float maxDistNN, threshDistNN;
};

class Quadruple
{
public:
    Quadruple();
    ~Quadruple();

public:
    int find3nearestpoints(Point p, NCPointCloud pc);

public:
    vector<Point> quad;
    float alpha, beta, theta;

};

class LineSegment : public NCPointCloud
{
public:
    LineSegment();
    ~LineSegment();

public:
    int getMeanDev();
    int save(QString filename);

    int boundingbox();
    bool insideLineSegments(Point p);
    bool onSegment(Point p);
    int lineFromPoints();
    int update();
    float angle(LineSegment ls);
    bool sidebyside(LineSegment ls);
    void info();
    bool isSmooth();

public:
    float meanval_adjangles, stddev_adjangles; //
    Point pbbs, pbbe; // bounding box
    Point *origin;
    Vector *axis;
    bool b_bbox, b_updated;
    bool visited;
    Point root, tip;
};

template <typename T>
struct PointCloud
{
    struct Point
    {
        T  x,y,z;
    };

    std::vector<Point>  pts;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const { return pts.size(); }

    // Returns the dim'th component of the idx'th point in the class:
    // Since this is inlined and the "dim" argument is typically an immediate value, the
    //  "if/else's" are actually solved at compile time.
    inline T kdtree_get_pt(const size_t idx, int dim) const
    {
        if (dim == 0) return pts[idx].x;
        else if (dim == 1) return pts[idx].y;
        else return pts[idx].z;
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
};

template<class InputPixelType, class OutputPixelType, unsigned int VImageDimension >
int runGPUGradientAnisotropicDiffusionImageFilter(const std::string& inFile, const std::string& outFile)
{
    if(!itk::IsGPUAvailable())
    {
        std::cerr << "OpenCL-enabled GPU is not present." << std::endl;
        return EXIT_FAILURE;
    }

    itk::TIFFImageIOFactory::RegisterOneFactory();

    //
    bool useGPU = true;

    //
    if(useGPU)
    {
        typedef itk::GPUImage< InputPixelType,  VImageDimension >   InputImageType;
        typedef itk::GPUImage< OutputPixelType, VImageDimension >   OutputImageType;


        typedef itk::ImageFileReader< InputImageType  >  ReaderType;
        typedef itk::ImageFileWriter< OutputImageType >  WriterType;http://www.geeksforgeeks.org/construct-a-binary-tree-from-parent-array-representation/

        typename ReaderType::Pointer reader = ReaderType::New();
        typename WriterType::Pointer writer = WriterType::New();

        reader->SetFileName( inFile );
        writer->SetFileName( outFile );

        // Create GPU anistropic diffusion filter
        typedef itk::GPUGradientAnisotropicDiffusionImageFilter< InputImageType, OutputImageType > GPUAnisoDiffFilterType;
        typename GPUAnisoDiffFilterType::Pointer GPUFilter = GPUAnisoDiffFilterType::New();

        //
        reader->Update();

        // GPU anistropic diffusion
        itk::TimeProbe gputimer;
        gputimer.Start();

        GPUFilter->SetInput( reader->GetOutput() );
        GPUFilter->SetNumberOfIterations( 10 );
        GPUFilter->SetTimeStep( 0.0625 );
        GPUFilter->SetConductanceParameter( 1.0 ); // 3.0
        GPUFilter->UseImageSpacingOn();

        try
        {
            GPUFilter->Update();
        }
        catch (itk::ExceptionObject& excp)
        {
            std::cout << "Caught exception during GPUFilter->Update() " << excp << std::endl;
            return EXIT_FAILURE;
        }

        try
        {
            GPUFilter->GetOutput()->UpdateBuffers(); // synchronization point
        }
        catch (itk::ExceptionObject& excp)
        {
            std::cout << "Caught exception during GPUFilter->GetOutput()->UpdateBuffers() " << excp << std::endl;
            return EXIT_FAILURE;
        }

        //
        gputimer.Stop();
        std::cout << "GPU Anisotropic diffusion took " << gputimer.GetMean() << " seconds.\n" << std::endl;

        writer->SetInput( GPUFilter->GetOutput() ); // copy GPU->CPU implicilty

        // execute pipeline filter and write output
        writer->Update();

        GPUFilter = ITK_NULLPTR; // explicit GPU object destruction test
        itk::GPUContextManager::GetInstance()->DestroyInstance(); // GPUContextManager singleton destruction test

    }
    else
    {
        typedef itk::Image< InputPixelType,  VImageDimension >   InputImageType;
        typedef itk::Image< OutputPixelType, VImageDimension >   OutputImageType;


        typedef itk::ImageFileReader< InputImageType  >  ReaderType;
        typedef itk::ImageFileWriter< OutputImageType >  WriterType;

        typename ReaderType::Pointer reader = ReaderType::New();
        typename WriterType::Pointer writer = WriterType::New();

        reader->SetFileName( inFile );
        writer->SetFileName( outFile );

        // Create CPU anistropic diffusion filter
        typedef itk::GradientAnisotropicDiffusionImageFilter< InputImageType, OutputImageType > CPUAnisoDiffFilterType;
        typename CPUAnisoDiffFilterType::Pointer CPUFilter = CPUAnisoDiffFilterType::New();

        //
        reader->Update();

        // CPU anistropic diffusion
        itk::TimeProbe cputimer;
        cputimer.Start();

        CPUFilter->SetNumberOfThreads( 8 );

        CPUFilter->SetInput( reader->GetOutput() );
        CPUFilter->SetNumberOfIterations( 10 );
        CPUFilter->SetTimeStep( 0.0625 );
        CPUFilter->SetConductanceParameter( 3.0 );
        CPUFilter->UseImageSpacingOn();
        CPUFilter->Update();

        cputimer.Stop();

        std::cout << "CPU Anisotropic diffusion took " << cputimer.GetMean() << " seconds with "
                  << CPUFilter->GetNumberOfThreads() << " threads.\n" << std::endl;

        writer->SetInput( CPUFilter->GetOutput() );
        writer->Update();
    }

    //
    return EXIT_SUCCESS;
}

// modify markerRadius
template<class T>
float estimateRadius(T* &inimg1d, V3DLONG *sz, float mx, float my, float mz, float thresh)
{
    //
    double max_r = max(max(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
    double r;
    double tol_num, bak_num;

    //
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

                            //cout<<bak_num<<" / "<<tol_num<<" = "<<bak_num / tol_num<<" r "<<r<<endl;

                            if((bak_num / tol_num) > 0.1) return r;
                        }
                    }
                }
            }
        }
    }

    //
    return r;
}

#endif // _NEURONRECON_H_
