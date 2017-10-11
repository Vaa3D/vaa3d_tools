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
#include <string>

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

#include <map>
#include <pcl/common/time.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/distances.h>
#include <pcl/io/pcd_io.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <pcl/kdtree/impl/kdtree_flann.hpp>

using namespace std;
using namespace pcl;

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

public:
    float x, y, z; // location
    float radius; // radius
    float val; // intensity value
    vector<V3DLONG> parents; // n (same definition in .swc)
    vector<V3DLONG> children; // n (same definition in .swc) used for detect branch points
    V3DLONG n; // #
    int type; // -1 cell body; 0 tip point; 1 regular point; 3 branch point

    bool visited;
    int connected; // 0 not connected; 1 connected to one other point; 2 connected two other points; ...
    V3DLONG pre, next; // index
    vector<V3DLONG> nn; // k nearest neighbors (indices)
};

//
class NCPointCloud
{
public:
    NCPointCloud();
    ~NCPointCloud();

public:

    // load a single/multiple .swc file(s)
    int getPointCloud(QStringList files);

    // save as a .apo file
    int savePointCloud(QString filename);

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
    int knn(int k);

    //
    int connectPoints2Lines(QString infile, QString outfile, int k, float angle, float m);

    // cost func
    int minAngle(unsigned long &loc, float maxAngle);

    //

public:
    vector<Point> points;

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

public:
    float meanval_adjangles, stddev_adjangles; //
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
        typedef itk::ImageFileWriter< OutputImageType >  WriterType;

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
        GPUFilter->SetConductanceParameter( 3.0 );
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

#endif // _NEURONRECON_H_
