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
    parents.clear();
    children.clear();
    n=0;
    visited = false;
    connected = 0;
}

Point::Point (float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;

    radius = 0;
    val = 0;
    parents.clear();
    children.clear();
    n=0;
    visited = false;

    connected = 0;
}

Point::~Point()
{
    parents.clear();
    children.clear();
}

void Point::setLocation(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

bool Point::isSamePoint(Point p)
{
    //
    if(abs(x - p.x)<1e-6 && abs(y - p.y)<1e-6 & abs(z - p.z)<1e-6)
    {
        return true;
    }

    //
    return false;
}

void Point::setRadius(float r)
{
    radius = r;
}

void Point::setValue(float v)
{
    val = v;
}

//
NCPointCloud::NCPointCloud()
{
    points.clear();
}

NCPointCloud::~NCPointCloud()
{
    points.clear();
}

int NCPointCloud::getPointCloud(QStringList files)
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

int NCPointCloud::addPointFromNeuronTree(NeuronTree nt)
{
    //
    if(nt.listNeuron.size()>0)
    {
        V3DLONG n = nt.listNeuron.size();

        for(V3DLONG i=0; i<n; i++)
        {
            Point p;

            p.n = nt.listNeuron[i].n;
            p.x = nt.listNeuron[i].x;
            p.y = nt.listNeuron[i].y;
            p.z = nt.listNeuron[i].z;
            p.radius = nt.listNeuron[i].r;
            p.parents.push_back(nt.listNeuron[i].parent);

            points.push_back(p);
        }

        // update children
        for(V3DLONG i=0; i<n; i++)
        {
            if(points[i].parents[0]>0)
            {
                for(V3DLONG j=0; j<n; j++)
                {
                    if(points[j].n == points[i].parents[0])
                    {
                        points[j].children.push_back(points[i].n);
                    }
                }
            }
        }

        // update type
        for(V3DLONG i=0; i<n; i++)
        {
            if(points[i].parents[0] == -1)
            {
                // cell body
                points[i].type = -1;
            }
            else
            {
                if(points[i].children.size() > 0)
                {
                    if(points[i].children.size() > 1)
                    {
                        // branch points
                        points[i].type = 3;
                    }
                    else
                    {
                        // regular points
                        points[i].type = 1;
                    }
                }
                else
                {
                    // tip points
                    points[i].type = 0;
                }
            }
        }

    }

    //
    cout << "after add a neuron tree, the size of point cloud become "<<points.size()<<endl;

    //
    return 0;
}

int NCPointCloud::savePointCloud(QString filename)
{
    //
    V3DLONG n = points.size();

    // .apo
    QList <CellAPO> pointcloud;

    for(V3DLONG i=0; i<n; i++)
    {
        Point p = points[i];
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
    //        Point p = points[i];
    //        ImageMarker marker(0, 1, p.x, p.y, p.z, p.radius);

    //        markers.push_back(marker);
    //    }

    //    //
    //    writeMarker_file(filename, markers);

    //
    return 0;
}

int NCPointCloud::savePC2SWC(NCPointCloud pc, QString filename)
{
    //
    NeuronTree nt;

    for(V3DLONG i=0; i<pc.points.size(); i++)
    {
        NeuronSWC S;
        S.n = pc.points[i].n;
        S.type = 3;
        S.x = pc.points[i].x;
        S.y= pc.points[i].y;
        S.z = pc.points[i].z;
        S.r = pc.points[i].radius;
        S.pn = pc.points[i].parents[0];

        nt.listNeuron.append(S);
        nt.hashNeuron.insert(S.n, nt.listNeuron.size()-1);
    }

    //
    return writeESWC_file(filename, nt);
}

int NCPointCloud::resample()
{
    //

    //
    return 0;
}

float NCPointCloud::distance(Point a, Point b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    float z = a.z - b.z;

    return sqrt(x*x + y*y + z*z);
}

int NCPointCloud::getBranchPoints(QString filename)
{
    //
    if(filename.toUpper().endsWith(".SWC"))
    {
        NeuronTree nt = readSWC_file(filename);
        addPointFromNeuronTree(nt);

        // find the branch point
        for(V3DLONG i=0; i<points.size(); i++)
        {
            Point current;

            current.n = points[i].n;
            current.x = points[i].x;
            current.y = points[i].y;
            current.z = points[i].z;
            current.radius = points[i].radius;

            current.parents.push_back(points[i].parents[0]);

            // saving branches
            if(points[i].type == 3)
            {
                cout<<i<<" branch point: "<<points[i].n<< " - parent - "<<points[i].parents[0]<<endl;

                for(V3DLONG j=0; j<points[i].children.size(); j++)
                {
                    cout<<" children "<<j<<" : "<<points[i].children[j]<<endl;
                }

                //
                NCPointCloud pc;

                Point parent, child1, child2;

                V3DLONG pn, c1n, c2n;

                pn = points[i].parents[0];
                c1n = points[i].children[0];
                c2n = points[i].children[1];

                //
                for(V3DLONG j=0; j<points.size(); j++)
                {
                    if(points[j].n == pn)
                    {
                        parent.n = points[j].n;
                        parent.x = points[j].x;
                        parent.y = points[j].y;
                        parent.z = points[j].z;
                        parent.radius = points[j].radius;

                        parent.parents.push_back(-1);

                        pn = points[i].parents[0];
                    }
                    else if(points[j].n == c1n)
                    {
                        child1.n = points[j].n;
                        child1.x = points[j].x;
                        child1.y = points[j].y;
                        child1.z = points[j].z;
                        child1.radius = points[j].radius;

                        child1.parents.push_back(points[i].n);
                    }
                    else if(points[j].n == c2n)
                    {
                        child2.n = points[j].n;
                        child2.x = points[j].x;
                        child2.y = points[j].y;
                        child2.z = points[j].z;
                        child2.radius = points[j].radius;

                        child2.parents.push_back(points[i].n);
                    }
                }

                pc.points.push_back(parent);
                pc.points.push_back(current);
                pc.points.push_back(child1);
                pc.points.push_back(child2);

                QString output = filename.left(filename.lastIndexOf(".")).append("_branch_%1.swc").arg(current.n);
                savePC2SWC(pc, output);

                // recover parent's parent after saving the branch
                parent.parents[0] = pn;

                // segment start from the parent of branch point
                NCPointCloud pcSeg;
                V3DLONG nseg = parent.n;
                bool keepFinding = true;

                if(parent.visited == false)
                {
                    //
                    pcSeg.points.push_back(parent);

                    //
                    while(keepFinding)
                    {
                        //
                        bool found = false;
                        for(V3DLONG j=0; j<points.size(); j++)
                        {
                            if(points[j].n == parent.parents[0])
                            {
                                found = true;

                                //
                                if(points[j].visited == false && points[j].type == 1)
                                {
                                    parent.n = points[j].n;
                                    parent.x = points[j].x;
                                    parent.y = points[j].y;
                                    parent.z = points[j].z;
                                    parent.radius = points[j].radius;

                                    parent.parents[0] = points[j].parents[0];

                                    //
                                    pcSeg.points.push_back(parent);

                                    //
                                    break;
                                }
                                else
                                {
                                    keepFinding = false;
                                }
                            }
                        }

                        if(!found)
                        {
                            break;
                        }

                    }

                    //
                    if(pcSeg.points.size()>1)
                    {
                        QString outSegs = filename.left(filename.lastIndexOf(".")).append("_segment_%1.swc").arg(nseg);
                        savePC2SWC(pcSeg, outSegs);
                    }
                }

            }
            else if(points[i].type == 0)
            {
                NCPointCloud pcSeg;
                V3DLONG nseg = current.n;
                bool keepFinding = true;

                if(current.visited == false)
                {
                    //
                    pcSeg.points.push_back(current);

                    //
                    while(keepFinding)
                    {
                        //
                        bool found = false;
                        for(V3DLONG j=0; j<points.size(); j++)
                        {
                            if(points[j].n == current.parents[0])
                            {
                                found = true;

                                //
                                if(points[j].visited == false && points[j].type == 1)
                                {
                                    current.n = points[j].n;
                                    current.x = points[j].x;
                                    current.y = points[j].y;
                                    current.z = points[j].z;
                                    current.radius = points[j].radius;

                                    current.parents[0] = points[j].parents[0];

                                    //
                                    pcSeg.points.push_back(current);

                                    //
                                    break;
                                }
                                else
                                {
                                    keepFinding = false;
                                }
                            }
                        }

                        if(!found)
                        {
                            break;
                        }

                    }

                    //
                    if(pcSeg.points.size()>1)
                    {
                        QString outSegs = filename.left(filename.lastIndexOf(".")).append("_segment_%1.swc").arg(nseg);
                        savePC2SWC(pcSeg, outSegs);
                    }
                }
            } // type
        }
    }
    else
    {
        cout<<"please input a .swc file\n";
    }

    //
    return 0;
}

int NCPointCloud::getNeurites(QString filename)
{
    //


    //
    return 0;
}

float NCPointCloud::getAngle(Point a, Point b, Point c)
{
    // get angle between a->b and b->c

    float x1 = b.x - a.x;
    float y1 = b.y - a.y;
    float z1 = b.z - a.z;

    float x2 = c.x - b.x;
    float y2 = c.y - b.y;
    float z2 = c.z - b.z;

    float dot = x1*x2 + y1*y2 + z1*z2;
    float lenSq1 = x1*x1 + y1*y1 + z1*z1;
    float lenSq2 = x2*x2 + y2*y2 + z2*z2;

    //
    return acos(dot/sqrt(lenSq1 * lenSq2));
}

bool NCPointCloud::findNextUnvisitPoint(unsigned long &index)
{
    V3DLONG n = points.size();

    if(n>0)
    {
        for(V3DLONG i=0; i<n; i++)
        {
            if(points[i].visited==false)
            {
                index = i;
                return true;
            }
        }
    }

    return false;
}

int NCPointCloud::knn(int k)
{
    //
    if(k<1 || cloud.points.size()<k)
    {
        cout<<"invalid k or cloud\n";
        return -1;
    }

    //
    vector<int> k_indices;
    k_indices.resize (k);
    vector<float> k_distances;
    k_distances.resize (k);

    KdTreeFLANN<PointXYZ> kdtree;
    kdtree.setInputCloud(cloud.makeShared());

    //
    for(long i=0; i<cloud.points.size(); i++)
    {
        PointXYZ p = cloud.points[i];

        //cout<<i<<" "<<p.x<<" "<<p.y<<" "<<p.z<<endl; // for debug

        kdtree.nearestKSearch (p, k, k_indices, k_distances);

        // 0 is self, so start recording nearest neighbor from 1
        for (size_t j = 1; j < k_indices.size (); ++j)
        {
            //const PointXYZ& point = cloud.points[k_indices[j]];
            //cout<<"test ... "<<point.x<<" "<<point.y<<" "<<point.z<<" ... dist: "<< euclideanDistance(point, cloud.points[i])<<endl;

            points[i].children.push_back(k_indices[j]);
        }
    }

    //
    return 0;
}

int NCPointCloud::connectPoints2Lines(QString infile, QString outfile)
{
    // load point cloud save as a .apo file
    QList <CellAPO> inputPoints = readAPO_file(infile);

    V3DLONG n = inputPoints.size();

    for(V3DLONG i=0; i<n; i++)
    {
        CellAPO cell = inputPoints[i];

        //
        Point p;

        p.n = i+1; // # assigned
        p.x = cell.x;
        p.y = cell.y;
        p.z = cell.z;
        p.radius = 0.5*cell.volsize;

        points.push_back(p);

        //
        PointXYZ point;

        point.x = cell.x;
        point.y = cell.y;
        point.z = cell.z;

        cloud.points.push_back(point);
    }

    // find k nearest neighbors and save indices into children
    int k=6;
    knn(k);

    // connect points into lines
    unsigned long loc;
    while(findNextUnvisitPoint(loc))
    {
        //
        cout<<"current point ..."<<loc<<endl;

        //
        points[loc].visited = true;
        points[loc].parents.push_back(-1);

        Point p = points[loc];

        cout<<" ... "<<p.x<<" "<<p.y<<" "<<p.z<<endl;

        //
        while(minAngle(loc)>=0)
        {
            cout<<"next point ... "<<loc<<endl;
        }
    }

    // save connected results into a .swc file
    savePC2SWC(*this, outfile);

    //
    return 0;
}

// cost func
int NCPointCloud::minAngle(unsigned long &loc)
{
    //
    Point p = points[loc];

    //
    unsigned int nneighbors = 5;
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    float minAngle = 3.14;

    long next = -1;
    long next_next = -1;

    //
    for(V3DLONG i=0; i<p.children.size(); i++)
    {
        Point p_next = points[p.children[i]];

        if(p_next.visited || p_next.isSamePoint(p))
        {
            continue;
        }

        cout<<"next ... "<<p_next.x<<" "<<p_next.y<<" "<<p_next.z<<endl;

        for(V3DLONG j=0; j<p_next.children.size(); j++)
        {
            Point p_next_next = points[p_next.children[j]];

            if(p_next_next.visited || p_next_next.isSamePoint(p_next) || p_next_next.isSamePoint(p))
            {
                continue;
            }

            cout<<"next next ... "<<p_next_next.x<<" "<<p_next_next.y<<" "<<p_next_next.z<<endl;

            float angle = getAngle(p, p_next, p_next_next);

            if(angle<maxAngle && angle<minAngle)
            {
                minAngle = angle;
                next = p.children[i];
                next_next = p_next.children[j];
            }
        }

    }

    //
    if(next>0 && next_next>0)
    {
        points[loc].connected++;
        points[next].visited = true;
        points[next].connected++;
        points[next].parents.push_back(p.n);
        points[next_next].visited = true;
        points[next_next].connected++;
        points[next_next].parents.push_back(points[next].n);

        loc = next_next;

        cout<<"update loc ... "<<loc<<endl;

        return 0;
    }

    //
    return -1;
}

// class Quadruple
Quadruple::Quadruple()
{

}

Quadruple::~Quadruple()
{

}

int Quadruple::find3nearestpoints(Point p, NCPointCloud pc)
{

}

// class LineSegment
LineSegment::LineSegment()
{
    meanval_adjangles = 0;
    stddev_adjangles = 0;
    points.clear();
}

LineSegment::~LineSegment()
{
    meanval_adjangles = 0;
    stddev_adjangles = 0;
    points.clear();
}

int LineSegment::getMeanDev()
{
    // mean and stdard deviation
    if(points.size()<3)
    {
        cout<<"Not enough points for statistics\n";
        return -1;
    }
    else
    {
        //
        Point firstPoint = points.back();
        points.pop_back();

        Point secondPoint = points.back();
        points.pop_back();

        long n = 0;
        float sum = 0;

        vector<float> angles;

        while(!points.empty())
        {
            Point thirdPoint = points.back();
            points.pop_back();

            if(thirdPoint.x == secondPoint.x && thirdPoint.y == secondPoint.y && thirdPoint.z == secondPoint.z)
            {
                // duplicated point
                continue;
            }

            float angle = 3.14159265f - getAngle(firstPoint, secondPoint, thirdPoint);

            sum += angle;
            angles.push_back(angle);

            n++;

            qDebug()<<"angle ... "<<angle<<" sum "<<sum;

            firstPoint = secondPoint;
            secondPoint = thirdPoint;

            qDebug()<<"first point ..."<<firstPoint.x<<firstPoint.y<<firstPoint.z;
        }

        qDebug()<<"sum of "<<n<<" angles ... "<<sum;

        //
        meanval_adjangles = sum / n;

        //
        sum = 0;
        for(int i=0; i<angles.size(); i++)
        {
            sum += (angles[i] - meanval_adjangles)*(angles[i] - meanval_adjangles);
        }

        stddev_adjangles = sqrt(sum/(n-1));

        qDebug()<<"mean ... "<<meanval_adjangles<<" std dev ..."<<stddev_adjangles;
    }

    //
    return 0;
}

int LineSegment::save(QString filename)
{
    //
    std::ofstream outfile;

    outfile.open(filename.toStdString().c_str(), std::ios_base::app);
    outfile << meanval_adjangles << ", " << stddev_adjangles << endl;

    //
    return 0;
}





