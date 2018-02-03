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
#include <stack>
#include <fstream>

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

#include "sort_swc.hpp"

using namespace std;
using namespace nanoflann;

//
typedef tuple<float,float,long,long> PairCompareType;

//
class LineSegment;

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
    int connects; // 0 not connected; 1 connected to one other point; 2 connected two other points; ...
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
    float weight;
    int connect1, connect2;
    bool interested;
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

    bool childrenVisited(long n);
    long nextUnvisitedChild();

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

    //
    float distP2L(Point p, LineSegment line);

    // judge whether a point important
    bool isConsidered(unsigned long &index, float m);

    // k nearest neighbor search
    int knn(int k, float radius=0);

    //
    int connectPoints2Lines(QString infile, QString outfile, int k, float angle, float m);

    //
    int connectPoints(int k, float maxAngle, float m, unsigned char *pImg=NULL, long sx=1, long sy=1, long sz=1);

    //
    int connectPoints2(int k, float maxAngle, float m);
    bool findNextPoint(unsigned long &index);
    int connect(Point p);

    //
    int removeNoise(float distfactor=20);

    //
    int removeRedundant();

    //
    int shift(float *p, long sx, long sy, long sz, long nstep, Point &pt);

    //
    int assembleFragments(int k);

    //
    int tracing(QString infile, QString outfile, int k, float angle, float m, double distthresh=15, float rmNoiseDistFac=20, unsigned char *pImg=NULL, long sx=1, long sy=1, long sz=1);

    //
    int tracing2(QString infile, QString outfile, int k, float angle, float m, double distthresh=15, bool rmNoise=true);

    //
    int trace(QString infile, QString outfile, int k, float maxAngle, float m, double distthresh=15, float rmNoiseDistFac=20, unsigned char *pImg=NULL, long sx=1, long sy=1, long sz=1);

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
    int reverseVisitStatus();

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
    int sample(QString infile, QString outfile, float srx=0.5, float sry=0.5, float srz=0.5);

    //
    float hausdorffdistance(NCPointCloud a, NCPointCloud b);

    //
    float meandistance(NeuronTree a, NeuronTree b);

    //
    Point parent(long n);

    //
    int sortbyradius();

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

    void dfs(long v);
    int reconstruct();

public:
    vector<Point> points; // data
    Pairs skipConnecting; // for connecting line segments
    float maxDistNN, threshDistNN;
    vector<size_t> somas;

    list<long> *adj;
    vector<long> filament;
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
    float angleLine2Line(LineSegment ls);
    bool sidebyside(LineSegment ls);
    void info();
    bool isSmooth();
    float maxradius();
    float length(float vx, float vy, float vz, NCPointCloud pc);

public:
    float meanval_adjangles, stddev_adjangles; //
    Point pbbs, pbbe; // bounding box
    Point *origin;
    Vector *axis;
    bool b_bbox, b_updated;
    bool visited;
    Point root, tip;
};

//
vector<LineSegment> separate(NCPointCloud pc);
NCPointCloud combinelines(vector<LineSegment> lines, long thresh=3);

//
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
        GPUFilter->SetNumberOfIterations( 5 );
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
        CPUFilter->SetNumberOfIterations( 5 );
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
    float max_r = max(max(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
    float r;
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

template<class T>
int getMarkersBetween(vector<Point> &allmarkers, Point m1, Point m2)
{
    double A = m2.x - m1.x;
    double B = m2.y - m1.y;
    double C = m2.z - m1.z;
    double R = m2.radius - m1.radius;
    double D = sqrt(A*A + B*B + C*C);
    A = A/D; B = B/D; C = C/D; R = R/D;

    double ctz = A/sqrt(A*A + B*B);
    double stz = B/sqrt(A*A + B*B);

    double cty = C/sqrt(A*A + B*B + C*C);
    double sty = sqrt(A*A + B*B)/sqrt(A*A + B*B + C*C);

    double x0 = m1.x;
    double y0 = m1.y;
    double z0 = m1.z;
    double r0 = m1.radius;

    vector<Point> marker_set;

    NCPointCloud pc;
    double dist = pc.distance(m1, m2);
    for(double t = 0.0; t <= dist; t += 1.0)
    {
        T cx = x0 + A*t + 0.5;
        T cy = y0 + B*t + 0.5;
        T cz = z0 + C*t + 0.5;
        T radius = r0 + R*t + 0.5;
        T radius2 = radius * radius;

        for(T k = -radius; k <= radius; k++)
        {
            for(T j = -radius; j <= radius; j++)
            {
                for(T i = -radius; i <= radius; i++)
                {
                    if(i * i + j * j + k * k > radius2) continue;
                    double x = i, y = j, z = k;
                    double x1, y1, z1;

                    //rotate_coordinate_z_clockwise(ctz, stz, x, y, z);
                    //rotate_along_y_clockwise     (cty, sty, x, y, z);
                    //rotate_coordinate_x_anticlock(ctz, stz, x, y, z);
                    //translate_to(cx, cy, cz, x, y, z);
                    y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
                    x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
                    z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
                    x += cx; y += cy; z += cz;
                    x = (int)(x+0.5);
                    y = (int)(y+0.5);
                    z = (int)(z+0.5);
                    marker_set.push_back(Point(x, y, z));
                }
            }
        }
    }

    allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());

    //
    return 0;
}

template<class T>
vector<Point> getLeaf_markers(NCPointCloud pc)
{
    vector<Point> leaf_markers;
    for(T i = 0; i < pc.points.size(); i++)
    {
        if(!pc.points[i].hasChildren())
            leaf_markers.push_back(pc.points[i]);
    }

    return leaf_markers;
}

template<class T>
int points2maskimage(T * &img1d, NCPointCloud pc, long sz0, long sz1, long sz2, T val)
{
    long sz01 = sz0 * sz1;
    if(img1d == 0)
    {
        cout<<"Invalid input"<<endl;
        return -1;
    }
//    vector<Point> leaf_markers = getLeaf_markers<T>(pc);
//    set<Point*> visited_markers;
//    for(int i = 0; i < leaf_markers.size(); i++)
//    {
//        Point leaf = leaf_markers[i];
//        Point p = leaf;
//        while(visited_markers.find(&p) == visited_markers.end() && p.parents.size() > 0)
//        {
//            Point par = pc.parent(p.parents[0]);;
//            vector<Point> tmp_markers;
//            getMarkersBetween<long>(tmp_markers, p, par);
//            for(int j = 0; j < tmp_markers.size(); j++)
//            {
//                int x = tmp_markers[j].x;
//                int y = tmp_markers[j].y;
//                int z = tmp_markers[j].z;
//                if(x < 0 || x >= sz0 || y < 0 || y >= sz1 || z < 0 || z >= sz2) continue;
//                img1d[z*sz01 + y * sz0 + x] = val;
//            }
//            visited_markers.insert(&p);
//            p = par;
//        }
//    }

    for(long i=0; i<pc.points.size(); i++)
    {
        Point p = pc.points[i];
        long x = p.x;
        long y = p.y;
        long z = p.z;
        double radius = p.radius;

        long xb,xe, yb,ye, zb,ze;

        xb = x-radius;
        if(xb<0) xb = 0;
        xe = x+radius;
        if(xe>sz0-1) xe = sz0-1;

        yb = y-radius;
        if(yb<0) yb = 0;
        ye = y+radius;
        if(ye>sz1-1) ye = sz1-1;

        zb = z-radius;
        if(zb<0) zb = 0;
        ze = z + radius;
        if(ze>sz2-1) ze = sz2-1;

        for(z = zb; z<=ze; z++)
        {
            long ofz = z*sz0*sz1;
            for(y = yb; y<=ye; y++)
            {
                long ofy = ofz + y*sz0;
                for(x = xb; x<=xe; x++)
                {
                    img1d[ofy+x] = val;
                }
            }
        }
    }

    //
    return 0;
}

template<class T>
int sortByRadiusIntensity(T *data1d, long sx, long sy, long sz, NCPointCloud &pc, float adjintthresh=0.5)
{
    //
    float thresh;
    estimateIntensityThreshold(data1d, sx*sy*sz, thresh);
    thresh *= adjintthresh;

    //
    NCPointCloud pointcloud;

    //
    for(size_t i=0; i<pc.points.size(); i++)
    {
        Point p = pc.points[i];

        long z = long(p.z);

        if(z>sz-1) continue;

        long y = long(p.y);

        if(y>sy-1) continue;

        long x = long(p.x);

        if(x>sx-1) continue;


        long xpre = x-1, xnext = x+1;

        if(xpre<0) xpre=0;
        if(xnext>sx-1) xnext = sx-1;

        long ypre = y-1, ynext = y+1;

        if(ypre<0) ypre=0;
        if(ynext>sy-1) ynext = sy-1;

        long zpre = z-1, znext = z+1;

        if(zpre<0) zpre=0;
        if(znext>sz-1) znext = sz-1;

        float maxval = 0;

        for(long zz=zpre; zz<=znext; zz++)
        {
            long ofz = zz*sx*sy;
            for(long yy=ypre; yy<=ynext; yy++)
            {
                long ofy = ofz + yy*sx;
                for(long xx=xpre; xx<=xnext; xx++)
                {
                    if(float(data1d[ofy + xx]) > maxval)
                    {
                        maxval = float(data1d[ofy + xx]);
                        x = xx;
                        y = yy;
                        z = zz;
                    }
                }
            }
        }

        if(z>sz-1) continue;
        if(y>sy-1) continue;
        if(x>sx-1) continue;

        //
        if(maxval>thresh)
        {
            p.x = x;
            p.y = y;
            p.z = z;
            p.val = maxval;

            pointcloud.points.push_back(p);
        }
    }

    //
    pc.copy(pointcloud);

    //
    sort(pc.points.begin(), pc.points.end(), [](const Point& a, const Point& b) -> bool
    {
        return a.val*a.radius > b.val*b.radius;
    });

    //
    return 0;
}

template<class T>
float meanIntensityValueLineSegment(T *pImg, long sx, long sy, long sz, Point p, Point q)
{
    //
    float meanVal = 0;
    long n=0;

    //
    LineSegment ls;
    ls.points.push_back(p);
    ls.points.push_back(q);

    ls.origin = new Point;
    ls.origin->x = p.x;
    ls.origin->y = p.y;
    ls.origin->z = p.z;

    ls.axis = new Vector;
    ls.axis->dx = q.x - p.x;
    ls.axis->dy = q.y - p.y;
    ls.axis->dz = q.z - p.z;

    ls.root = p;
    ls.tip = q;

    ls.b_updated = true; // do not update

    //
    long xs, xe, ys, ye, zs, ze;

    if(p.x>q.x)
    {
        xs = q.x;
        xe = p.x;
    }
    else
    {
        xs = p.x;
        xe = q.x;
    }

    if(p.y>q.y)
    {
        ys = q.y;
        ye = p.y;
    }
    else
    {
        ys = p.y;
        ye = q.y;
    }

    if(p.z>q.z)
    {
        zs = q.z;
        ze = p.z;
    }
    else
    {
        zs = p.z;
        ze = q.z;
    }

    for(long z=zs; z<ze; z++)
    {
        long ofz = z*sx*sy;
        for(long y=ys; y<ye; y++)
        {
            long ofy = ofz + y*sx;
            for(long x=xs; x<xe; x++)
            {
                long idx = ofy + x;
                Point m;
                m.x = x;
                m.y = y;
                m.z = z;

                if(ls.onSegment(m))
                {
                    meanVal += pImg[idx];
                    n++;
                }
            }
        }
    }


    //
    if(meanVal)
    {
        return meanVal/n;
    }
    else
    {
        return 0;
    }
}


#endif // _NEURONRECON_H_
