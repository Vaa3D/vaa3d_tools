// neuronrecon.cpp
// neuron reconstruction from multiple traced neurons

#include "neuronrecon.h"

#ifdef _MSC_VER
#define  LONG_LONG_MAX _I64_MAX

inline float  roundf(float num)  
{
    return num > 0 ? std::floor(num + 0.5f) : std::ceil(num - 0.5f);
}

inline long   lroundf(float num) { return static_cast<long>(roundf(num)); }   
#endif

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

template <class T> T pow2(T a)
{
    return a*a;
}

#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif

// class Point
Point::Point()
{
    x = 0;
    y = 0;
    z = 0;
    radius = 0;
    val = 0;
}

Point::~Point()
{

}

void Point::setLocation(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

void Point::setRadius(float r)
{
    radius = r;
}

void Point::setValue(float v)
{
    val = v;
}

PointCloud::PointCloud()
{
    pc.clear();
}

PointCloud::~PointCloud()
{
    pc.clear();
}

int PointCloud::getPointCloud(QStringList files)
{
    //
    if(files.size()>0)
    {
        V3DLONG n = files.size();

        for(V3DLONG i=0; i<n; i++)
        {
            QString filename = files[i];

            if(filename.toUpper().endsWith(".SWC"))
            {
                NeuronTree nt = readSWC_file(filename);
                addPointFromNeuronTree(nt);
            }
        }
    }

    //
    return 0;
}

int PointCloud::addPointFromNeuronTree(NeuronTree nt)
{
    //
    if(nt.listNeuron.size()>0)
    {
        for(V3DLONG i=0; i<nt.listNeuron.size(); i++)
        {
            Point p;

            p.x = nt.listNeuron[i].x;
            p.y = nt.listNeuron[i].y;
            p.z = nt.listNeuron[i].z;
            p.radius = nt.listNeuron[i].r;

            pc.push_back(p);
        }
    }

    //
    cout << "after add a neuron tree, the size of point cloud become "<<pc.size()<<endl;

    //
    return 0;
}

int PointCloud::savePointCloud(QString filename)
{
    //
    V3DLONG n = pc.size();

    // .apo
    QList <CellAPO> pointcloud;

    for(V3DLONG i=0; i<n; i++)
    {
        Point p = pc[i];
        CellAPO cell;

        cell.x = p.x;
        cell.y = p.y;
        cell.z = p.z;
        cell.volsize = 2*p.radius;

        pointcloud.push_back(cell);
    }

    writeAPO_file(filename, pointcloud);

//    QList<ImageMarker> markers;
//    for(V3DLONG i=0; i<n; i++)
//    {
//        Point p = pc[i];
//        ImageMarker marker(0, 1, p.x, p.y, p.z, p.radius);

//        markers.push_back(marker);
//    }

//    //
//    writeMarker_file(filename, markers);

    //
    return 0;
}

int PointCloud::resample()
{
    //

    //
    return 0;
}

float PointCloud::distance(Point a, Point b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    float z = a.z - b.z;

    return sqrt(x*x + y*y + z*z);
}
