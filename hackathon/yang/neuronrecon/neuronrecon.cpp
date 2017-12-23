// neuronrecon.cpp
// construct neuron tree(s) from detected signals

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

// class Plane
Plane::Plane()
{

}

Plane::~Plane()
{

}

// class Vector
Vector::Vector()
{

}

Vector::~Vector()
{

}

// axb
Vector* Vector::vcross(Vector *a, Vector *b)
{
    Vector *c = new Vector;
    c->dx = a->dy * b->dz - a->dz * b->dy;
    c->dy = a->dz * b->dx - a->dx * b->dz;
    c->dz = a->dx * b->dy - a->dy * b->dx;
    return (c);
}

// inner product
float Vector::vdot(Vector *a, Vector *b)
{
    return (a->dx * b->dx + a->dy * b->dy + a->dz * b->dz);
}

// magnitude
float Vector::vmag(Vector *a)
{
    return (sqrt(vdot(a, a)));
}

// reciprocal magnitude
float Vector::recip_vmag(Vector *a)
{
    return (1.0 / sqrt(vdot(a, a)));
}

// a /= ||a||
Vector* Vector::vnorm(Vector *a)
{
    float d;

    if ((d = recip_vmag(a)) > 1.0E6)
    {
        printf("\nvector at 0x%x: (%g, %g, %g) ?\n", a, a->dx, a->dy, a->dz);
        a->dx = a->dy = a->dz = 0.0;
        return (NULL);
    }
    else
    {
        a->dx *= d;
        a->dy *= d;
        a->dz *= d;
        return (a);
    }
}

void Vector::info()
{
    cout<<"vector: ("<<dx<<", "<<dy<<", "<<dz<<")"<<endl;
}

// class Point
Point::Point()
{
    x = 0;
    y = 0;
    z = 0;
    radius = 0;
    val = 0;
    parents.clear();
    nn.clear();
    n=0;
    visited = false;
    connects = 0;
    pre = -1;
    next = -1;

    neighborVisited = false;
    isolated = false;
    isNoise = false;

    weight = -1;
    connect1 = -1;
    connect2 = -1;
    interested = true;
}

Point::Point (float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;

    radius = 0;
    val = 0;
    parents.clear();
    nn.clear();
    n=0;
    visited = false;

    connects = 0;
    pre = -1;
    next = -1;

    neighborVisited = false;
    isolated = false;
    isNoise = false;

    weight = -1;
    connect1 = -1;
    connect2 = -1;
    interested = true;
}

Point::~Point()
{
    parents.clear();
    children.clear();
    nn.clear();
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

void Point::info()
{
    if(parents.size()>0)
    {
        if(children.size()>0)
        {
            cout<<"Point: "<<n<<" ( "<<x<<" "<<y<<" "<<z<<" ) - parent: "<<parents[0]<<" - first child: "<<children[0]<<endl;
        }
        else
        {
            cout<<"Point: "<<n<<" ( "<<x<<" "<<y<<" "<<z<<" ) - parent: "<<parents[0]<<endl;
        }
    }
    else
    {
        cout<<"Point: "<<n<<" ( "<<x<<" "<<y<<" "<<z<<" )"<<endl;
    }
}

bool Point::hasChildren()
{
    return (children.size()>0);
}

// class PointPair
PointPair::PointPair(Point a, Point b)
{
    pointpair = pair<Point, Point>(a,b);
    visited = false;
}

PointPair::~PointPair()
{

}

bool PointPair::samePair(Point a, Point b)
{
    Point p = get<0>(pointpair);
    Point q = get<1>(pointpair);

    if((p.isSamePoint(a) && q.isSamePoint(b))
            || (p.isSamePoint(b) && q.isSamePoint(a)))
    {
        return true;
    }

    return false;
}

// class Pairs
Pairs::Pairs()
{

}

Pairs::~Pairs()
{

}

void Pairs::resetStatus()
{
    for(size_t i=0; i<pairs.size(); i++)
    {
        pairs[i].visited = false;
    }
}

void Pairs::appendPair(Point a, Point b)
{
    PointPair pp(a,b);
    pairs.push_back(pp);
}

//
NCPointCloud::NCPointCloud()
{
    points.clear();
    skipConnecting.pairs.clear();
    maxDistNN = -1;
    threshDistNN = -1;
}

NCPointCloud::~NCPointCloud()
{
    points.clear();
    skipConnecting.pairs.clear();
    maxDistNN = -1;
    threshDistNN = -1;
}

int NCPointCloud::getPointCloud(QStringList files)
{
    //
    if(files.size()>0)
    {
        long n = files.size();

        for(long i=0; i<n; i++)
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
        long n = nt.listNeuron.size();

        for(long i=0; i<n; i++)
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
        for(long i=0; i<n; i++)
        {
            if(points[i].parents[0]>0)
            {
                for(long j=0; j<n; j++)
                {
                    if(points[j].n == points[i].parents[0])
                    {
                        points[j].children.push_back(points[i].n);
                    }
                }
            }
        }

        // update type
        for(long i=0; i<n; i++)
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

int NCPointCloud::savePointCloud(QString filename, int format)
{
    //
    long n = points.size();

    //
    if(format==0)
    {
        // apo
        QList <CellAPO> pointcloud;

        for(long i=0; i<n; i++)
        {
            Point p = points[i];

            if(p.radius > 2.5)
            {
                CellAPO cell;

                cell.x = p.x;
                cell.y = p.y;
                cell.z = p.z;
                cell.volsize = 2*p.radius;

                pointcloud.push_back(cell);
            }
        }

        writeAPO_file(filename, pointcloud);
    }
    else if(format==1)
    {
        filename = filename.left(filename.lastIndexOf(".")).append(".marker");

        // marker
        QList<ImageMarker> markers;
        for(long i=0; i<n; i++)
        {
            Point p = points[i];
            ImageMarker marker(0, 1, p.x+1, p.y+1, p.z+1, p.radius);

            markers.push_back(marker);
        }

        //
        writeMarker_file(filename, markers);
    }
    else
    {
        cout<<"Invalid file format specified \n";
        return -1;
    }

    //
    return 0;
}

int NCPointCloud::saveNeuronTree(NCPointCloud pc, QString filename)
{
    // isolated points
    // pc.isolatedPoints();

    //
    NeuronTree nt;

    for(long i=0; i<pc.points.size(); i++)
    {
        if(pc.points[i].isolated)
        {
            continue;
        }

        //
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
    return writeSWC_file(filename, nt);
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
        for(long i=0; i<points.size(); i++)
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

                for(long j=0; j<points[i].children.size(); j++)
                {
                    cout<<" children "<<j<<" : "<<points[i].children[j]<<endl;
                }

                //
                NCPointCloud pc;

                Point parent, child1, child2;

                long pn, c1n, c2n;

                pn = points[i].parents[0];
                c1n = points[i].children[0];
                c2n = points[i].children[1];

                //
                for(long j=0; j<points.size(); j++)
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
                saveNeuronTree(pc, output);

                // recover parent's parent after saving the branch
                parent.parents[0] = pn;

                // segment start from the parent of branch point
                NCPointCloud pcSeg;
                long nseg = parent.n;
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
                        for(long j=0; j<points.size(); j++)
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
                        saveNeuronTree(pcSeg, outSegs);
                    }
                }

            }
            else if(points[i].type == 0)
            {
                NCPointCloud pcSeg;
                long nseg = current.n;
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
                        for(long j=0; j<points.size(); j++)
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
                        saveNeuronTree(pcSeg, outSegs);
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
    return acos(dot/(sqrt(lenSq1 * lenSq2) + 1e-6));
}

bool NCPointCloud::findNextUnvisitPoint(unsigned long &index)
{
    long n = points.size();

    if(n>0)
    {
        for(long i=0; i<n; i++)
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

bool NCPointCloud::findNextPoint(unsigned long &index)
{
    long n = points.size();

    if(index==0)
    {
        float minweight = points[0].weight;
        for(size_t i=1; i<n; i++)
        {
            if(points[i].interested && points[i].connects<1 && points[i].weight<minweight)
            {
                minweight = points[i].weight;
                index = i;
            }
        }
    }
    else
    {
        if(points[index].interested)
        {
            if(points[index].connects<2)
            {
                return true;
            }
            else
            {
                float minweight = 1e6;
                for(size_t i=0; i<n; i++)
                {
                    if(points[i].weight<minweight && points[i].connects<2 && points[i].interested)
                    {
                        minweight = points[i].weight;
                        index = i;
                    }
                }
            }
        }
    }

    if(index>0)
    {
        return true;
    }

    return false;
}

int NCPointCloud::knn(int k, float radius)
{
    //
    cout<<"knn searching radius: "<<radius<<endl;

    //
    if(k<1 || points.size()<k)
    {
        cout<<"invalid k "<<k<<" cloud size of "<<points.size()<<endl;
        return -1;
    }

    //
    long n = points.size();
    PointCloud<float> cloud;
    cloud.pts.resize(n);

    for(long i=0; i<n; i++)
    {
        cloud.pts[i].x = points[i].x;
        cloud.pts[i].y = points[i].y;
        cloud.pts[i].z = points[i].z;
    }

    // construct a kd-tree index
    KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<float, PointCloud<float> >, PointCloud<float>, 3> kdtree(3, cloud, KDTreeSingleIndexAdaptorParams(max(10, 2*k)));
    kdtree.buildIndex();

    //
    float *p = NULL;
    try
    {
        p = new float [3];
    }
    catch(...)
    {
        cout<<"fail to alloc memory for a point\n";
        return -1;
    }

    //
    for(long i=0; i<n; i++)
    {
        p[0] = cloud.pts[i].x;
        p[1] = cloud.pts[i].y;
        p[2] = cloud.pts[i].z;

        //cout<<"current point ... "<<i<<" n "<<points[i].n<<" "<<p[0]<<" "<<p[1]<<" "<<p[2]<<endl;

        // 0 is itself, so start recording nearest neighbor from 1
        if(radius)
        {
            //
            vector<pair<size_t,float> > ret_matches;
            nanoflann::SearchParams params;

            //
            const size_t nMatches = kdtree.radiusSearch(p, static_cast<float>(radius), ret_matches, params);

            //cout <<"#"<<points[i].n<< " radiusSearch(): radius=" << radius << " -> " << nMatches << " matches\n";
            points[i].nn.clear();
            for (size_t j = 1; j < nMatches; j++)
            {
                //cout << "idx["<< j << "]=" << ret_matches[j].first << " dist["<< j << "]=" << ret_matches[j].second << endl;
                points[i].nn.push_back(ret_matches[j].first);
            }
        }
        else
        {
            //
            vector<size_t> k_indices;
            k_indices.resize (k);
            vector<float> k_distsqr;
            k_distsqr.resize (k);

            //
            size_t num_results = kdtree.knnSearch(p, k, &k_indices[0], &k_distsqr[0]);

            //cout << "knnSearch(): num_results=" << num_results << "\n";
            points[i].nn.clear();
            for (size_t j = 1; j < num_results; ++j)
            {
                //cout <<"#"<<points[i].n<<" idx["<< j << "]=" << long(k_indices[j]) << " dist["<< j << "]=" << k_distsqr[j] << endl;

                if(maxDistNN<k_distsqr[j])
                {
                    maxDistNN = k_distsqr[j];
                }

                points[i].nn.push_back(k_indices[j]);
            }
        }
    }

    maxDistNN = sqrt(maxDistNN);

    //
    if(p)
    {
        delete []p;
        p = NULL;
    }

    //
    return 0;
}

//
int NCPointCloud::removeNoise()
{
    //
    knn(3);

    NCPointCloud pc;
    for(size_t i=0; i<points.size(); i++)
    {
        Point p = points[i];

        if(p.nn.size()>1)
        {
            Point q = points[p.nn[1]];

            if(distance(p,q)<10*p.radius)
            {
                pc.points.push_back(p);
            }
        }
        else
        {
            cout<<"odd knn computing for point #"<<i<<endl;
        }
    }

    copy(pc);

    //
    return 0;
}

//
int NCPointCloud::tracing(QString infile, QString outfile, int k, float angle, float m, double distthresh)
{
    // load point cloud save as a .apo file
    QList <CellAPO> inputPoints = readAPO_file(infile);

    long n = inputPoints.size();

    //
    for(long i=0; i<n; i++)
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
    }

    //
    removeNoise();

    // connect points into lines
    connectPoints(k,angle,m);

    // merge lines
    // mergeLines(angle);

    // assemble fragments into trees
    //assembleFragments(k);

    // save connected results into a .swc file
    if(!outfile.isEmpty())
    {
        saveNeuronTree(*this, outfile);
    }

    // assemble fragments into trees
    QList<NeuronSWC> neuron, result;
    NeuronTree nt = readSWC_file(outfile);
    neuron = nt.listNeuron;
    if (sortswc::SortSWC<long>(neuron, result, VOID, distthresh))
    {
        QString fileDefaultName = outfile+QString("_sorted.swc");
        //write new SWC to file
        if (!sortswc::export_list2file<long>(result,fileDefaultName,outfile))
        {
            cout<<"fail to write the output result"<<endl;
            return -1;
        }
    }

    //
    return 0;
}

//
int NCPointCloud::sample(QString infile, QString outfile, float srx, float sry, float srz)
{
    // load point cloud save as a .apo file
    QList <CellAPO> inputPoints = readAPO_file(infile);

    long n = inputPoints.size();

    //
    for(long i=0; i<n; i++)
    {
        CellAPO cell = inputPoints[i];

        //
        Point p;

        p.n = i+1; // # assigned
        p.x = srx*cell.x;
        p.y = sry*cell.y;
        p.z = srz*cell.z;
        p.radius = 0.5*cell.volsize;

        points.push_back(p);
    }

    //
    if(!outfile.isEmpty())
    {
        savePointCloud(outfile);
    }

    //
    return 0;
}

//
int NCPointCloud::endpoints()
{
    //
    int n=0;

    //
    for(size_t i=0; i<points.size(); i++)
    {
        Point p = points[i];

        if(p.parents[0]==-1 || !p.hasChildren())
        {
            if(points[i].visited)
            {
                continue;
            }
            else
            {
                n++;
            }
        }
    }

    return n;
}

//
bool NCPointCloud::allendpointvisited()
{
    //
    for(size_t i=0; i<points.size(); i++)
    {
        Point p = points[i];

        if(p.parents[0]==-1 || !p.hasChildren())
        {
            if(points[i].visited)
            {
                continue;
            }
            else
            {
                cout<<"unvisited endpoint "<<i<<endl;
                return false;
            }
        }
    }

    //
    return true;
}

//
bool NCPointCloud::checkloop(Point p, Point q)
{
    // check possible loop if connect p to q

    // parent
    Point pcheck = p;
    long iter = 0;
    while(pcheck.parents[0]!=-1)
    {
        if(pcheck.parents[0]==q.n || (++iter>1 && pcheck.isSamePoint(p)))
        {
            return true;
        }
        pcheck = points[indexofpoint(pcheck.parents[0])];
        cout<<"... "<<iter<<" p "<<pcheck.parents[0]<<" index "<<indexofpoint(pcheck.parents[0])<<" n "<<pcheck.n<<endl;
    }

    // children
    pcheck = q;
    iter = 0;
    while(pcheck.hasChildren())
    {
        if(pcheck.children[0]==p.n || (++iter>1 && pcheck.isSamePoint(q)))
        {
            return true;
        }
        pcheck = points[indexofpoint(pcheck.children[0])];
        cout<<"... "<<iter<<" c "<<pcheck.children[0]<<endl;
    }

    //
    return false;
}

int NCPointCloud::assembleFragments(int k)
{
    // assemble lines into trees
    // connect root/tip point into its shortest neighbor

    float adist = 1;

    //
    knn(k);

    //
    long maxiteration = 10;
    for(long iter=0; iter<maxiteration; iter++,adist++)
    {
        //
        resetVisitStatus();

        //
        int ncurendpoints, npreendpoints;

        npreendpoints = endpoints();

        //
        while(!allendpointvisited())
        {
            if(ncurendpoints==npreendpoints)
            {
                cout<<"done assembling \n";
                break;
            }
            npreendpoints = ncurendpoints;

            //
            for(size_t i=0; i<points.size(); i++)
            {
                Point p = points[i];

                if(p.parents[0]==-1)
                {
                    cout<<"test ... root "<<i<<endl;

                    // root
                    if(p.visited)
                    {
                        continue;
                    }
                    else
                    {
                        points[i].visited = true;
                    }

                    //
                    Point pchild, pn, pchildn;
                    long pnidx, pchildnidx, pchildidx;
                    pchild = p;
                    while(pchild.hasChildren())
                    {
                        pchildidx = indexofpoint(pchild.children[0]);
                        pchild = points[pchildidx];
                    }

                    //
                    float distP=-1, distC=-1;
                    for(size_t j=1; j<p.nn.size(); j++)
                    {
                        pnidx = p.nn[j];
                        pn = points[pnidx];

                        if(pn.isSamePoint(p))
                        {
                            continue;
                        }
                        else
                        {
                            distP = distance(p,pn);
                            break;
                        }
                    }

                    if(!pchild.isSamePoint(p))
                    {
                        for(size_t j=1; j<pchild.nn.size(); j++)
                        {
                            pchildnidx = pchild.nn[j];
                            pchildn = points[pchildnidx];

                            if(pchildn.isSamePoint(pchild))
                            {
                                continue;
                            }
                            else
                            {
                                distC = distance(pchild,pchildn);
                                break;
                            }
                        }

                        // connect
                        if(distC<distP && distC>0)
                        {
                            if(distC > adist*points[pchildnidx].radius)
                            {
                                continue;
                            }

                            //
                            if(points[pchildnidx].parents[0]==-1)
                            {
                                points[pchildnidx].visited = true;
                                points[i].visited = false;

                                //
                                if(!checkloop(pchild, points[pchildnidx]))
                                {
                                    cout<<"connect "<<points[pchildnidx].n<<" -> "<<pchild.n<<endl;

                                    points[pchildnidx].parents[0] = pchild.n;
                                    points[pchildidx].children.push_back(points[pchildnidx].n);
                                }
                            }
                            else
                            {
                                // reverse
                                cout<<"need reverse \n";
                                reverseLineSegment(i);

                                if(!checkloop(points[pchildnidx], pchild))
                                {
                                    cout<<"connect "<<pchild.n<<" -> "<<points[pchildnidx].n<<endl;

                                    points[pchildidx].parents[0] = points[pchildnidx].n;
                                    points[pchildnidx].children.push_back(pchild.n);
                                }
                            }
                        }
                        else
                        {
                            if(distP > adist*points[i].radius)
                            {
                                continue;
                            }

                            if(!checkloop(pn, p) && distP>0)
                            {
                                cout<<"connect "<<p.n<<" -> "<<pn.n<<endl;

                                points[i].parents[0] = pn.n;
                                points[pnidx].children.push_back(p.n);
                            }
                        }
                    }
                    else
                    {
                        if(distP > adist*points[i].radius)
                        {
                            continue;
                        }

                        if(!checkloop(pn, p))
                        {
                            cout<<"connect "<<p.n<<" -> "<<pn.n<<endl;

                            points[i].parents[0] = pn.n;
                            points[pnidx].children.push_back(p.n);
                        }
                    }

                }
                else if(!p.hasChildren())
                {
                    cout<<"test ... tip "<<i<<endl;

                    // tip
                    if(p.visited)
                    {
                        continue;
                    }
                    else
                    {
                        points[i].visited = true;
                    }

                    //
                    Point pparent, pn, pparentn;
                    long pnidx, pparentnidx, pparentidx;
                    pparent = p;
                    while(pparent.parents[0]!=-1)
                    {
                        pparentidx = indexofpoint(pparent.parents[0]);
                        pparent = points[pparentidx];
                    }

                    //
                    float distP=-1, distC=-1;
                    for(size_t j=1; j<p.nn.size(); j++)
                    {
                        pnidx = p.nn[j];
                        pn = points[pnidx];

                        if(pn.isSamePoint(p))
                        {
                            continue;
                        }
                        else
                        {
                            distC = distance(p, pn);
                        }
                    }

                    for(size_t j=1; j<pparent.nn.size(); j++)
                    {
                        pparentnidx = pparent.nn[j];
                        pparentn = points[pparentidx];

                        if(pparentn.isSamePoint(pparent))
                        {
                            continue;
                        }
                        else
                        {
                            distP = distance(pparent,pparentn);
                            break;
                        }
                    }

                    if(distC<distP)
                    {
                        if(distC > adist*points[pnidx].radius)
                        {
                            continue;
                        }

                        //
                        if(pn.parents[0]==-1)
                        {
                            points[pnidx].visited = true;

                            //
                            if(!checkloop(p,pn) && distC>0)
                            {
                                cout<<"connect "<<points[pnidx].n<<" -> "<<p.n<<endl;

                                points[pnidx].parents[0] = p.n;
                                points[i].children.push_back(pn.n);
                            }

                        }
                        else
                        {
                            // reverse
                            cout<<"need reverse \n";
                            reverseLineSegment(i);

                            if(!checkloop(pn, p))
                            {
                                cout<<"connect "<<p.n<<" -> "<<points[pnidx].n<<endl;

                                points[i].parents[0] = points[pnidx].n;
                                points[pnidx].children.push_back(p.n);
                            }
                        }
                    }
                    else
                    {
                        points[pparentidx].visited = true;

                        if(distP > adist*points[pparentidx].radius)
                        {
                            continue;
                        }

                        //
                        if(!checkloop(pparentn, pparent) && distP>0)
                        {
                            cout<<"connect "<<points[pparentidx].n<<" -> "<<pparentn.n<<endl;

                            points[pparentidx].parents[0] = pparentn.n;
                            points[pparentnidx].children.push_back(pparent.n);
                        }
                    }
                }
            }

            //
            ncurendpoints = endpoints();
        }// while

    }// iter

    //
    return 0;
}

//
int NCPointCloud::connectPoints(int k, float maxAngle, float m)
{
    //
    float distThresh1, distThresh2;

    //
    knn(k);

    // connect points into lines
    unsigned long loc;
    while(findNextUnvisitPoint(loc))
    {
        //
        if(points[loc].visited == false)
        {
            points[loc].visited = true;
            points[loc].parents.push_back(-1);
        }

        //
        if(isConsidered(loc, m))
        {
            //
            Point p = points[loc];

            //
            vector<tuple<float,float,long,long>> candidates;

            //
            long next = -1;
            long next_next = -1;

            //
            if(p.parents[0] == -1)
            {
                // connect points from -1 "soma" (starting point)
                for(long i=0; i<p.nn.size(); i++)
                {
                    Point p_next = points[p.nn[i]];

                    if(p_next.visited || p_next.isSamePoint(p))
                    {
                        continue;
                    }

                    //cout<<"next ... "<<p_next.x<<" "<<p_next.y<<" "<<p_next.z<<endl;

                    for(long j=0; j<p_next.nn.size(); j++)
                    {
                        Point p_next_next = points[p_next.nn[j]];

                        if(p_next_next.visited || p_next_next.isSamePoint(p_next) || p_next_next.isSamePoint(p))
                        {
                            continue;
                        }

                        //cout<<"next next ... "<<p_next_next.x<<" "<<p_next_next.y<<" "<<p_next_next.z<<endl;

                        float angle = getAngle(p, p_next, p_next_next);
                        float dist1 = distance(p,p_next);
                        float dist2 = distance(p_next, p_next_next);
                        float dist = dist1 + dist2;

                        distThresh1 = 2*m*(p.radius + p_next.radius);
                        distThresh2 = 2*m*(p_next.radius + p_next_next.radius);

                        if(dist1<distThresh1 && dist2<distThresh2 && angle<maxAngle)
                        {
                            candidates.push_back(make_tuple(dist*angle, 1/p.radius, i, j));
                        }
                    }
                }

                //
                if(candidates.size()>0)
                {
                    sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                    {
                        return get<0>(a)*get<1>(a) < get<0>(b)*get<1>(b); // area as likelihood func
                    });

                    //
                    next = p.nn[get<2>(*(candidates.begin()))];
                    next_next = points[next].nn[get<3>(*(candidates.begin()))];

                    points[loc].connects++;
                    points[next].visited = true;
                    points[next].connects++;
                    points[next].parents.push_back(p.n);
                    points[loc].children.push_back(points[next].n);
                    points[next_next].visited = true;
                    points[next_next].connects++;
                    points[next_next].parents.push_back(points[next].n);
                    points[next].children.push_back(points[next_next].n);

                    points[next].pre = loc;
                    points[next].next = next_next;

                    points[next_next].pre = next;

                    loc = next_next;
                }
            }
            else
            {
                // connect points from the intermediate point
                Point p_pre = points[p.pre];

                //
                vector<tuple<float,float,long>> candidates;

                //
                for(long i=0; i<p.nn.size(); i++)
                {
                    Point p_next = points[p.nn[i]];

                    if(p_next.visited || p_next.isSamePoint(p) || p_next.isSamePoint(p_pre))
                    {
                        continue;
                    }

                    //cout<<"next ... "<<p_next.x<<" "<<p_next.y<<" "<<p_next.z<<endl;

                    float angle = getAngle(p_pre, p, p_next);
                    float dist1 = distance(p_pre,p);
                    float dist2 = distance(p, p_next);
                    float dist = dist1 + dist2;

                    distThresh1 = 2*m*(p_pre.radius + p.radius);
                    distThresh2 = 2*m*(p_next.radius + p.radius);

                    if(dist1<distThresh1 && dist2<distThresh2 && angle<maxAngle)
                    {
                        candidates.push_back(make_tuple(dist, angle, i));
                    }
                }

                //
                if(candidates.size()>0)
                {
                    sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long>& a, const tuple<float,float,long>& b) -> bool
                    {
                        return get<0>(a)*get<1>(a) < get<0>(b)*get<1>(b); // area as likelihood func
                    });

                    next = p.nn[get<2>(*(candidates.begin()))];

                    points[loc].connects++;
                    points[next].visited = true;
                    points[next].connects++;
                    points[next].parents.push_back(p.n);
                    points[loc].children.push_back(points[next].n);

                    points[next].pre = loc;

                    loc = next;
                }
            }
        }
    }

    //
    return 0;
}

int NCPointCloud::connectPoints2(int k, float maxAngle, float m)
{
    //
    float distThresh1, distThresh2;

    //
    knn(k);

    //
    vector<tuple<float,float,long,long>> candidates;

    //
    for(long k=0; k<points.size(); k++)
    {
        Point p = points[k];

        //
        long next = -1;
        long next_next = -1;

        //
        for(long i=0; i<p.nn.size(); i++)
        {
            Point p_next = points[p.nn[i]];

            if(p_next.visited || p_next.isSamePoint(p))
            {
                continue;
            }

            for(long j=0; j<p_next.nn.size(); j++)
            {
                Point p_next_next = points[p_next.nn[j]];

                if(p_next_next.visited || p_next_next.isSamePoint(p_next) || p_next_next.isSamePoint(p))
                {
                    continue;
                }

                //cout<<"next next ... "<<p_next_next.x<<" "<<p_next_next.y<<" "<<p_next_next.z<<endl;

                float angle = getAngle(p, p_next, p_next_next);
                float dist1 = distance(p,p_next);
                float dist2 = distance(p_next, p_next_next);
                float dist = dist1 + dist2;

                distThresh1 = 2*m*(p.radius + p_next.radius);
                distThresh2 = 2*m*(p_next.radius + p_next_next.radius);

                if(dist1<distThresh1 && dist2<distThresh2 && angle<maxAngle)
                {
                    candidates.push_back(make_tuple(dist, angle, i, j));
                }
            }
        }

        //
        if(candidates.size()>0)
        {
            sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
            {
                return get<0>(a)*get<1>(a) < get<0>(b)*get<1>(b); // area as likelihood func
            });

            //
            next = p.nn[get<2>(*(candidates.begin()))];
            next_next = points[next].nn[get<3>(*(candidates.begin()))];

            //
            points[k].weight = get<0>(*(candidates.begin()))*get<1>(*(candidates.begin()));
            points[k].connect1 = next;
            points[k].connect2 = next_next;
        }
    }

    cout<<"connecting ... ..."<<endl;

    // connect points into lines
    unsigned long loc=0;
    while(findNextPoint(loc))
    {
        cout<<"processing ... "<<loc<<endl;

        //
        if(isConsidered(loc, m))
        {
            cout<<"considering ... "<<loc<<endl;

            //
            Point p = points[loc];

            //
            if(p.connect1>0)
            {
                Point p_next = points[p.connect1];

                cout<<"comparing ... "<<p.weight<<" <> "<<p_next.weight<<endl;

                if(p.weight<=p_next.weight)
                {
                    // connect
                    cout<<"connect #"<<loc<<" -> #"<<p.connect1<<endl;
                    connect(p);
                    loc = p.connect1;
                }
                else
                {
                    bool searching = true;
                    Point p_pre = p;
                    while(p.weight>p_next.weight && searching)
                    {
                        cout<<"continue comparing ... "<<p.weight<<" <> "<<p_next.weight<<endl;

                        p_pre = p;
                        p = p_next;

                        if(p.connect1>0)
                        {
                            p_next = points[p.connect1];
                        }
                        else
                        {
                            searching = false;
                        }
                    }

                    if(p.weight<=p_next.weight)
                    {
                        // already connected, if so mark loc
                        if(p_next.parents[0]==p.n)
                        {
                            cout<<loc<<" will not be considered next time ... "<<endl;
                            points[loc].visited = true;
                            points[loc].parents.push_back(-1);
                            points[loc].interested = false;
                            loc = 0;
                        }
                        else
                        {
                            //
                            cout<<"connect "<<" #"<<indexofpoint(p.n)<<" -> #"<<indexofpoint(p_next.n)<<" "<<p.weight<<"<="<<p_next.weight<<endl;
                            connect(p);
                            loc = p.connect1;
                        }
                    }
                    else
                    {
                        cout<<"not considering ... "<<loc<<" anymore "<<endl;
                        points[loc].visited = true;
                        points[loc].parents.push_back(-1);
                        points[loc].interested = false;
                        loc = 0;
                    }
                }
            }
            else
            {
                cout<<"not considering ... "<<loc<<" anymore "<<endl;
                points[loc].visited = true;
                points[loc].parents.push_back(-1);
                points[loc].interested = false;
                loc = 0;
            }
        }
        else
        {
            loc = 0;
        }
    }


    //
    return 0;
}

//
int NCPointCloud::connect(Point p)
{
    long loc = indexofpoint(p.n);
    long next = p.connect1;
    long next_next = p.connect2;

    if(p.parents.size()<1)
    {
        points[loc].parents.push_back(-1);
    }

    if(points[loc].connects>2)
    {
        points[loc].interested = false;
        return -1;
    }

    points[loc].visited = true;
    points[loc].connects++;
    points[next].visited = true;
    points[next].connects++;
    points[next].parents.push_back(p.n);
    points[loc].children.push_back(points[next].n);

    points[loc].next = next;
    points[next].pre = loc;

    points[next_next].visited = true;
    points[next_next].connects++;
    points[next_next].parents.push_back(points[next].n);
    points[next].children.push_back(points[next_next].n);
    points[next].next = next_next;
    points[next_next].pre = next;

    //
    return 0;
}

//
int NCPointCloud::connectPoints2Lines(QString infile, QString outfile, int k, float angle, float m)
{
    // load point cloud save as a .apo file
    QList <CellAPO> inputPoints = readAPO_file(infile);

    long n = inputPoints.size();

    for(long i=0; i<n; i++)
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
    }

    //
    removeNoise();

    // find k nearest neighbors
    //    float mean, stddev;
    //    knnMeanStddevDist(mean, stddev, k);
    //    knn(k, mean);

//    float searchingRadius;
//    knnMaxDist(searchingRadius);
//    knn(k, searchingRadius);
    knn(k);

    // connect points into lines
    unsigned long loc;
    while(findNextUnvisitPoint(loc))
    {
        //
        //cout<<"current point ..."<<loc<<endl;

        //
        if(points[loc].visited == false)
        {
            points[loc].visited = true;
            points[loc].parents.push_back(-1);
        }

        //
        if(isConsidered(loc, m))
        {
            //
            Point p = points[loc];

            //cout<<" ... "<<p.x<<" "<<p.y<<" "<<p.z<<endl;

            //
            while(minAngle(loc, angle)>=0)
            {
                //cout<<"next point ... "<<loc<<endl;
            }
        }
    }

    cout<<"done connection ... \n";

    // save connected results into a .swc file

    if(!outfile.isEmpty())
    {
        saveNeuronTree(*this, outfile);
    }

    //
    return 0;
}

// cost func
int NCPointCloud::minAngle(unsigned long &loc, float maxAngle)
{
    //
    Point p = points[loc];

    //
    //unsigned int nneighbors = 5;
    //float maxAngle = 0.1; // 0.942; // threshold 60 degree (120 degree)
    float minAngle = maxAngle;

    long next = -1;
    long next_next = -1;

    //
    if(p.parents[0] == -1)
    {
        // connect points from -1 "soma" (starting point)
        for(long i=0; i<p.nn.size(); i++)
        {
            Point p_next = points[p.nn[i]];

            if(p_next.visited || p_next.isSamePoint(p))
            {
                continue;
            }

            //cout<<"next ... "<<p_next.x<<" "<<p_next.y<<" "<<p_next.z<<endl;

            for(long j=0; j<p_next.nn.size(); j++)
            {
                Point p_next_next = points[p_next.nn[j]];

                if(p_next_next.visited || p_next_next.isSamePoint(p_next) || p_next_next.isSamePoint(p))
                {
                    continue;
                }

                //cout<<"next next ... "<<p_next_next.x<<" "<<p_next_next.y<<" "<<p_next_next.z<<endl;

                float angle = getAngle(p, p_next, p_next_next);

                if(angle<minAngle)
                {
                    minAngle = angle;
                    next = p.nn[i];
                    next_next = p_next.nn[j];
                }
            }
        }

        //
        if(next>0 && next_next>0)
        {
            points[loc].connects++;
            points[next].visited = true;
            points[next].connects++;
            points[next].parents.push_back(p.n);
            points[loc].children.push_back(points[next].n);
            points[next_next].visited = true;
            points[next_next].connects++;
            points[next_next].parents.push_back(points[next].n);
            points[next].children.push_back(points[next_next].n);

            points[next].pre = loc;
            points[next].next = next_next;

            points[next_next].pre = next;

            loc = next_next;

            //cout<<"update loc ... "<<loc<<" angle "<<minAngle<<endl;

            return 0;
        }
    }
    else
    {
        // connect points from the intermediate point
        Point p_pre = points[p.pre];

        for(long i=0; i<p.nn.size(); i++)
        {
            Point p_next = points[p.nn[i]];

            if(p_next.visited || p_next.isSamePoint(p) || p_next.isSamePoint(p_pre))
            {
                continue;
            }

            //cout<<"next ... "<<p_next.x<<" "<<p_next.y<<" "<<p_next.z<<endl;

            float angle = getAngle(p_pre, p, p_next);

            if(angle<minAngle)
            {
                minAngle = angle;
                next = p.nn[i];
            }
        }

        if(next>0)
        {
            points[loc].connects++;
            points[next].visited = true;
            points[next].connects++;
            points[next].parents.push_back(p.n);
            points[loc].children.push_back(points[next].n);

            points[next].pre = loc;

            loc = next;

            //cout<<"update loc ... "<<loc<<" angle "<<minAngle<<endl;

            return 0;
        }
    }

    //
    return -1;
}

int NCPointCloud::isolatedPoints()
{
    //
    for(std::vector<Point>::iterator it = points.begin(); it != points.end(); ++it)
    {
        if(it->parents[0]==-1)
        {
            bool deleted = true;
            for(std::vector<Point>::iterator j = points.begin(); j != points.end(); ++j)
            {
                if(j->n == it->n)
                    continue;

                if(j->parents[0] == it->n)
                {
                    deleted = false;
                    break;
                }
            }

            if(deleted)
            {
                it->isolated = true;
            }
        }
    }

    //
    return 0;
}

int NCPointCloud::resetVisitStatus()
{
    //
    for(std::vector<Point>::iterator it = points.begin(); it != points.end(); ++it)
    {
        it->visited = false;
    }

    //
    return 0;
}

//
float NCPointCloud::distPoint2LineSegment(Point a, Point b, Point p)
{
    //
    Point w, v, pb;

    w.x = p.x - a.x;
    w.y = p.y - a.y;
    w.z = p.z - a.z;

    v.x = b.x - a.x;
    v.y = b.y - a.y;
    v.z = b.z - a.z;

    float dot = w.x*v.x + w.y*v.y + w.z*v.z;

    if(dot <=0 )
    {
        return distance(a, p);
    }

    float dot2 = v.x*v.x + v.y*v.y + v.z*v.z;

    if(dot2 <= dot)
    {
        return distance(b, p);
    }

    float k = dot / dot2;

    pb.x = a.x + k*v.x;
    pb.y = a.y + k*v.y;
    pb.z = a.z + k*v.z;

    //
    return distance(p, pb);
}

//
bool NCPointCloud::isConsidered(unsigned long &index, float m)
{
    // assume knn has been run
    Point p = points[index];

    float thresh = m*p.radius;

    if(p.nn.size()<1)
    {
        cout<<"point #"<<p.n<<" without nearest neighbor ... increase your searching radius to include this point\n";
        return false;
    }
    else
    {
        for(long i=0; i<p.nn.size(); i++)
        {
            Point p1 = points[p.nn[i]];

            if(p1.pre!=-1)
            {
                cout<<"test ... pre "<<p1.pre<<endl;

                Point p1_pre = points[p1.pre];

                if(!p.isSamePoint(p1_pre) && distPoint2LineSegment(p1_pre, p1, p) <= thresh)
                {
                    cout<<"not considering ... "<<index<<endl;
                    points[index].visited = true;
                    points[index].parents.push_back(-1);
                    points[index].interested = false;
                    return false;
                }
            }

            if(p1.next!=-1)
            {
                cout<<"test ... next "<<p1.next<<endl;

                Point p1_next = points[p1.next];

                if(!p.isSamePoint(p1_next) && distPoint2LineSegment(p1, p1_next, p) <= thresh)
                {
                    cout<<"not considering ... "<<index<<endl;
                    points[index].visited = true;
                    points[index].parents.push_back(-1);
                    points[index].interested = false;
                    return false;
                }
            }
        }
    }

    //
    return true;
}

int NCPointCloud::delDuplicatedPoints()
{
    // tmp points
    std::vector<Point> pc;
    pc.clear();

    //
    for(std::vector<Point>::iterator it = points.begin(); it != points.end(); ++it)
    {
        if(pc.size()<1)
        {
            pc.push_back(*it);
        }
        else
        {
            bool found = false;
            for(std::vector<Point>::iterator i = pc.begin(); i != pc.end(); ++i)
            {
                if(i->isSamePoint(*it))
                {
                    found = true;
                    continue;
                }
            }

            if(found==false)
            {
                pc.push_back(*it);
            }
        }
    }

    // copy back
    points.clear();

    for(std::vector<Point>::iterator it = pc.begin(); it != pc.end(); ++it)
    {
        points.push_back(*it);
    }

    //
    return 0;
}

int NCPointCloud::copy(NCPointCloud pc)
{
    //
    if(pc.points.size()<1)
    {
        cout<<"Empty input\n";
        return -1;
    }

    //
    points.clear();

    for(std::vector<Point>::iterator it = pc.points.begin(); it != pc.points.end(); ++it)
    {
        points.push_back(*it);
    }

    //
    return 0;
}

// breadth-first sort
int NCPointCloud::ksort(NCPointCloud pc, int k)
{
    //
    points.clear();

    //
    pc.knn(k);

    //
    float maxradius = 0;
    long soma = 0;
    // find the point with biggest radius
    for(long i=0; i<pc.points.size(); i++)
    {
        if(pc.points[i].radius > maxradius)
        {
            maxradius = pc.points[i].radius;
            soma = i;
        }
    }

    somas.push_back(soma); // multiple somas

    // sort
    while(points.size() < pc.points.size())
    {
        if(points.size()<1)
        {
            // soma
            points.push_back(pc.points[soma]);
            //points[0].visited = true;
            points[0].n = soma;
            pc.points[soma].visited = true;
        }
        else
        {
            // add neighbors
            bool allvisited = true;

            // soma's neighbors
            if(points.size()==1 && pc.points[soma].neighborVisited==false)
            {
                pc.points[soma].neighborVisited = true;

                for(int j=1; j<pc.points[soma].nn.size(); j++)
                {
                    int neighbor = pc.points[soma].nn[j];
                    if(pc.distance(pc.points[neighbor], pc.points[soma])<2*pc.points[soma].radius)
                    {
                        pc.points[neighbor].visited = true;
                        allvisited = false;
                    }
                    else if(!pc.points[neighbor].visited)
                    {
                        allvisited = false;

                        pc.points[neighbor].visited = true;
                        points.push_back(pc.points[neighbor]);
                        points[points.size()-1].n = neighbor;
                    }
                }
            }
            else
            {
                //
                long n = points.size();
                for(long i=1; i<n; i++)
                {
                    int cur = points[i].n;

                    if(pc.points[cur].neighborVisited)
                    {
                        continue;
                    }

                    //
                    pc.points[cur].neighborVisited = true;

                    //
                    for(int j=1; j<pc.points[cur].nn.size(); j++)
                    {
                        int neighbor = pc.points[cur].nn[j];
                        if(!pc.points[neighbor].visited)
                        {
                            allvisited = false;

                            pc.points[neighbor].visited = true;
                            points.push_back(pc.points[neighbor]);
                            points[points.size()-1].n = neighbor;
                        }
                    }
                }
            }

            if(allvisited)
            {
                // find unvisited one from input point cloud
                unsigned long loc;
                if(pc.findNextUnvisitPoint(loc))
                {
                    pc.points[loc].visited = true;
                    points.push_back(pc.points[loc]);
                    points[points.size()-1].n = loc;
                }
                else
                {
                    cout<<"all peaks are found"<<endl;
                    break;
                }
            }
        }
    }

    //
    resetVisitStatus();

    //
    return 0;
}

int NCPointCloud::knnMeanStddevDist(float &mean, float &stddev, int k)
{
    //
    NCPointCloud pc;
    pc.copy(*this);

    //
    pc.knn(k);

    //
    mean = 0;
    stddev = 0;

    // mean
    long n = 0;

    for(long i=0; i<pc.points.size(); i++)
    {
        Point p = pc.points[i];

        for(long j=1; j<pc.points[i].nn.size(); j++)
        {
            Point q = pc.points[pc.points[i].nn[j]];

            if(q.isSamePoint(p))
            {
                continue;
            }
            else
            {
                mean += distance(p,q);
                n++;
            }
        }
    }

    cout<<k<<" nn ... "<<mean<<" / "<<n<<endl;

    if(n>0)
    {
        mean /= n;
    }

    cout<<"mean distance: "<<mean<<endl;

    // stdard deviation


    //
    return 0;
}

int NCPointCloud::knnMaxDist(float &max, int k)
{
    //
    NCPointCloud pc;
    pc.copy(*this);

    // compare at least 2 nearest neighbors
    pc.knn(k);

    // adjust searching area 5x
    max = 5*pc.maxDistNN;

    //
    return 0;
}

Point NCPointCloud::pplusv(Point *p, Vector *v)
{
    Point a;
    a.x = p->x + v->dx;
    a.y = p->y + v->dy;
    a.z = p->z + v->dz;
    return (a);
}

float NCPointCloud::point_plane_dist(Point *a, Plane *P)
{
    return (a->x * P->a + a->y * P->b + a->z * P->c + P->d);
}

Point NCPointCloud::plerp(Point *a, Point *b, float t)
{
    Point p;
    p.x = t * b->x + (1-t) * a->x;
    p.y = t * b->y + (1-t) * a->y;
    p.z = t * b->z + (1-t) * a->z;
    return (p);
}

Point NCPointCloud::intersect_line_plane(Point *a, Point *b, Plane *M)
{
    Point p;
    float Mdota, Mdotb, denom, t;

    Mdota = point_plane_dist (a, M);
    Mdotb = point_plane_dist (b, M);

    denom = Mdota - Mdotb;
    if (fabsf(denom / (fabsf(Mdota) + fabsf(Mdotb))) < 1E-6) {
        fprintf (stderr, "int_line_plane(): no intersection?\n");
        p.x = p.y = p.z = 0.0;
    }
    else    {
        t = Mdota / denom;
        p = plerp (a, b, t);
    }

    return (p);
}

Point NCPointCloud::intersect_dline_plane(Point *a, Vector *adir, Plane *M)
{
    Point b = pplusv (a, adir);
    return (intersect_line_plane (a, &b, M));
}

Plane* NCPointCloud::plane_from_two_vectors_and_point(Vector *u, Vector *v, Point *p)
{
    Plane *M = new Plane;
    Vector *t = t->vcross (u, v);

    if(!t)
    {
        cout<<"NULL plane!\n";
    }

    t = t->vnorm(t);

    M->a = t->dx;
    M->b = t->dy;
    M->c = t->dz;

    // plane contains p
    M->d = -(M->a * p->x + M->b * p->y + M->c * p->z);
    return (M);
}

int NCPointCloud::line_line_closest_point(Point &pA, Point &pB, Point *a, Vector *adir, Point *b, Vector *bdir)
{
    Vector *cdir;
    Plane *ac, *bc;

    // connecting line is perpendicular to both
    cdir = cdir->vcross(adir, bdir);

    // lines are near-parallel -- all points are closest
    if (!cdir->vnorm(cdir))   {

        cout<<"cdir is NULL\n";

        pA = *a;
        pB = *b;
        return (0);
    }

    // form plane containing line A, parallel to cdir
    ac = plane_from_two_vectors_and_point(cdir, adir, a);

    // form plane containing line B, parallel to cdir
    bc = plane_from_two_vectors_and_point(cdir, bdir, b);

    // closest point on A is line A ^ bc
    pA = intersect_dline_plane(a, adir, bc);

    // closest point on B is line B ^ ac
    pB = intersect_dline_plane(b, bdir, ac);

    if (distance(pA, pB) < 1E-6)
        return (1);     // coincident
    else
        return (2);     // distinct
}

int NCPointCloud::mergeLines(float maxAngle)
{
    // connect a line to all other lines by evaluating
    // 1. distance
    // 2. intersections and angles
    // likelihood func (area) = distance * angle

    // merge lines
    bool merging = true;
    long iter = 0;
    long nLines = -1;
    long npoints = points.size();

    //
    while(merging)
    {
        //
        merging = false;

        // update lines after each merging
        vector<LineSegment> lines;
        resetVisitStatus();

        //
        for(long i=0; i<npoints; i++)
        {
            //cout<<"update lines ... point #"<<i<<endl;
            if(points[i].parents[0]==-1)
            {
                if(points[i].children.size()>0 && points[i].visited==false)
                {
                    LineSegment ls;

                    long j=i;
                    while(points[j].children.size()>0)
                    {
                        ls.points.push_back(points[j]);
                        points[j].visited = true;

                        j = indexofpoint(points[j].children[0]);

                        if(j<0)
                            break;
                    }
                    ls.points.push_back(points[j]); // tip point

                    //
                    if(ls.points.size()>2)
                    {
                        ls.update();
                        lines.push_back(ls);
                    }
                }
            }
        }

        cout<<"iter #"<<++iter<<" : "<<lines.size()<<" lines to be considered merging"<<endl;

        //        if(iter > 30)
        //            break;

        // test
        //        for(int i=0; i<lines.size(); i++)
        //        {
        //            LineSegment line = lines[i];
        //            cout<<"test ... line "<<i<<" "<<line.points.size()<<endl;
        //            for(int k=0; k<line.points.size(); k++)
        //            {
        //                long child = -1;

        //                if(line.points[k].hasChildren()>0)
        //                {
        //                    child = line.points[k].children[0];
        //                }

        //                cout<<"node "<<k<<" n "<<line.points[k].n<<" "<<line.points[k].parents[0]<<" child "<<child<<endl;
        //            }
        //        }

        //        for(int i=0; i<lines.size(); i++)
        //        {
        //            cout<<"test ... line #"<<i<<" "<<lines[i].points.size()<<endl;
        //            lines[i].isSmooth();
        //        }

        if(nLines > 0 )
        {
            if(lines.size() == nLines)
            {
                // converged
                break;
            }
        }
        nLines = lines.size();

        if(lines.size()>1)
        {
            // all pair comparison
            vector<tuple<float,float,long,long>> candidates;
            vector<tuple<Point,Point,long,long>> ijpoints;

            //
            Point pi, pj;
            LineSegment lsi, lsj;
            for(long i=0; i<lines.size(); i++)
            {
                //cout<<"line i:"<<i<<endl;

                lsi = lines[i];
                lines[i].visited = true;
                for(long j=1; j<lines.size(); j++)
                {
                    if(j==i)
                        continue;

                    //cout<<"line j:"<<j<<endl;

                    lsj = lines[j];

                    //                    if(!lsi.sidebyside(lsj))
                    //                    {
                    //                        cout<<"not considering ... \n";
                    //                        continue;
                    //                    }
                    //                    cout<<"further computing ... \n";

                    // comparing line #i and #j
                    float angle = lsi.angleLine2Line(lsj);
                    if(angle > 6)
                    {
                        // parallel
                        continue;
                    }
                    else
                    {
                        if(angle == 0)
                        {
                            // colinear
                        }
                        else
                        {
                            //                            Point pA, pB;
                            //                            line_line_closest_point(pA, pB, lsi.origin, lsi.axis, lsj.origin, lsj.axis);

                            //                            //
                            //                            if(lsi.onSegment(pA) || lsj.onSegment(pB))
                            //                            {
                            //                                cout<<"angle ... "<<angle<<endl;

                            //                                cout<<"pA is on #i segment? "<<lsi.onSegment(pA)<<endl;
                            //                                pA.info();
                            //                                cout<<"pB is on #j segment? "<<lsj.onSegment(pB)<<endl;
                            //                                pB.info();

                            //                                continue;
                            //                            }

                            if(angle > maxAngle)
                                continue;
                        }

                        //
                        Point t1 = lsi.root;
                        Point t2 = lsi.tip;

                        Point q1 = lsj.root;
                        Point q2 = lsj.tip;

                        //
                        if(distance(t1, *(lsj.origin)) < distance(t2, *(lsj.origin)))
                        {
                            pi = t1;
                        }
                        else
                        {
                            pi = t2;
                        }

                        if(distance(q1, *(lsi.origin)) < distance(q2, *(lsi.origin)))
                        {
                            pj = q1;
                        }
                        else
                        {
                            pj = q2;
                        }

                        //
                        candidates.push_back(make_tuple(distance(pi, pj), angle, i, j));
                        ijpoints.push_back(make_tuple(pi,pj,i,j));

                        //
                        if(angle<maxAngle)
                            merging = true;

                    }
                } // j
            } // i

            //
            if(merging)
            {
                //                cout<<"before sorting ...\n";
                //                for(size_t i=0; i<candidates.size(); i++)
                //                {
                //                    cout<<" #"<<i<<": "<<get<0>(candidates[i])<<" "<<get<1>(candidates[i])<<" "<<get<2>(candidates[i])<<" "<<get<3>(candidates[i])<<endl;
                //                }

                //                // sort by distance
                //                sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                //                {
                //                    return std::get<0>(a) < std::get<0>(b);
                //                });


                //                cout<<"sort by distance ... \n";
                //                for(size_t i=0; i<candidates.size(); i++)
                //                {
                //                    cout<<" #"<<i<<": "<<get<0>(candidates[i])<<" "<<get<1>(candidates[i])<<" "<<get<2>(candidates[i])<<" "<<get<3>(candidates[i])<<endl;
                //                }


                //                // sort by angle
                //                if(candidates.size()>swe)
                //                {
                //                    sort(candidates.begin() + sws, candidates.begin() + swe, [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                //                    {
                //                        return std::get<1>(a) < std::get<1>(b);
                //                    });
                //                }
                //                else
                //                {
                //                    sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                //                    {
                //                        return std::get<1>(a) < std::get<1>(b);
                //                    });
                //                }


                //                sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                //                {
                //                    if(std::get<0>(a) < std::get<0>(b)) return true;
                //                    if(std::get<0>(a) == std::get<0>(b)) return std::get<1>(a) < std://                bool alldone = false;
                //                while(get<1>(*(candidates.begin())) > maxAngle)
                //                {
                //                    cout<<"possible pair lines within the searching window are satisfied\n";

                //                    sws += swstep;
                //                    swe += swstep;

                //                    if(sws>lines.size())
                //                    {
                //                        cout<<"all possible pair lines are satisfied\n";
                //                        alldone = true;
                //                        break;
                //                    }

                //                    if(candidates.size()>swe)
                //                    {
                //                        sort(candidates.begin() + sws, candidates.begin() + swe, [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                //                        {
                //                            return std::get<1>(a) < std::get<1>(b);
                //                        });
                //                    }
                //                    else
                //                    {
                //                        sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                //                        {
                //                            return std::get<1>(a) < std::get<1>(b);
                //                        });
                //                    }

                //                }

                //                if(alldone)
                //                    break;:get<1>(b);
                //                    return false;
                //                });

                sort(candidates.begin(), candidates.end(), [](const tuple<float,float,long,long>& a, const tuple<float,float,long,long>& b) -> bool
                {
                    return get<0>(a)*get<1>(a) < get<0>(b)*get<1>(b); // area as likelihood func
                });

                //                cout<<"after sort ... \n";
                //                for(size_t i=0; i<candidates.size(); i++)
                //                {
                //                    //cout<<" #"<<i<<": "<<get<0>(candidates[i])<<" "<<get<1>(candidates[i])<<" "<<get<2>(candidates[i])<<" "<<get<3>(candidates[i])<<endl;
                //                    cout<<"["<<get<0>(candidates[i])<<", "<<get<1>(candidates[i])<<"], ";
                //                }

                // merge
                bool connected = false;
                long ntests = 15;
                long itest = 0;
                long skip = 0;

                skipConnecting.resetStatus();

                while(!connected && itest<ntests)
                {
                    long offset = skip + itest;

                    if(offset>=candidates.size())
                        break;

                    cout<<"merging ... w/ angle: "<<get<1>(*(candidates.begin()+offset))<<" ... offset "<<offset<<" ... "<<itest<<endl;

                    //
                    long tipi, tipj;
                    long iLine = get<2>(*(candidates.begin()+offset));
                    long jLine = get<3>(*(candidates.begin()+offset));
                    for(long k=0; k<ijpoints.size(); k++)
                    {
                        if( get<2>(ijpoints[k]) == iLine && get<3>(ijpoints[k]) == jLine)
                        {
                            tipi = indexofpoint(get<0>(ijpoints[k]).n);
                            tipj = indexofpoint(get<1>(ijpoints[k]).n);
                            break;
                        }
                    }

                    //
                    bool found = false;
                    for(size_t i=0; i<skipConnecting.pairs.size(); i++)
                    {
                        if(skipConnecting.pairs[i].visited)
                            continue;

                        if(skipConnecting.pairs[i].samePair(points[tipi], points[tipj]))
                        {
                            skipConnecting.pairs[i].visited = true;
                            skip++;
                            found = true;
                            break;
                        }
                    }

                    if(found)
                    {
                        continue;
                    }

                    itest++;

                    //
                    float dist = get<0>(*(candidates.begin()+offset));
                    float angle = get<1>(*(candidates.begin()+offset));

                    //
                    cout<<"connecting ... line #"<<iLine<<" to line #"<<jLine<<" angle "<<angle<<" distance "<<dist<<endl;
                    cout<<"connected tip point #"<<tipi<<" "<<points[tipi].n<<" tip point #"<<tipj<<" "<<points[tipj].n<<endl;

                    cout<<"checking ... "<<points[tipi].parents[0]<<" "<<points[tipj].parents[0]<<endl;

                    //
                    lsi = lines[iLine];
                    lsj = lines[jLine];

                    //
                    if(points[tipi].parents[0]==-1 && points[tipj].parents[0]!=-1)
                    {
                        if(connectLineSegments(tipi, tipj, maxAngle)<0)
                        {
                            continue;
                        }

                    }
                    else if(points[tipi].parents[0]!=-1 && points[tipj].parents[0]==-1)
                    {
                        if(connectLineSegments(tipj, tipi, maxAngle)<0)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        cout<<"test ... lsi size "<<lsi.points.size()<<" lsj size "<<lsj.points.size()<<endl;

                        // reverse the short line and connect it to the other
                        if(lsi.points.size() < lsj.points.size())
                        {
                            cout<<"reverse lsi \n";

                            // reverse lsi
                            reverseLineSegment(tipi, lsi.points.size());

                            //
                            if(points[tipj].parents[0]==-1)
                            {
                                // pj -> pi
                                if(connectLineSegments(tipj, tipi, maxAngle)<0)
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                if(connectLineSegments(tipi, tipj, maxAngle)<0)
                                {
                                    continue;
                                }
                            }

                            cout<<"test ... done"<<endl;
                        }
                        else
                        {
                            cout<<"reverse lsj "<<lsj.points.size()<<endl;

                            // reverse lsj
                            reverseLineSegment(tipj, lsj.points.size());

                            //
                            if(points[tipi].parents[0]==-1)
                            {
                                // pi -> pj
                                if(connectLineSegments(tipi, tipj, maxAngle)<0)
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                //
                                if(connectLineSegments(tipj, tipi, maxAngle)<0)
                                {
                                    continue;
                                }
                            }
                        }
                    }

                    //
                    connected = true;

                } // while

            } // if merging
        } // if # line > 1
    } // while

    //
    return 0;
}

long NCPointCloud::indexofpoint(long n)
{
    for(long i=0; i<points.size(); i++)
    {
        if(points[i].n == n)
            return i;
    }

    return -1;
}

//
int NCPointCloud::reverseLineSegment(long idx, long size)
{
    //
    long idxcur;
    long ncur, ncurparent;
    long nparent;
    long n = size - 1;

    // tip point
    if(points[idx].parents[0]==-1)
    {
        idxcur = idx;
        while(points[idxcur].hasChildren())
        {
            idxcur = indexofpoint(points[idxcur].children[0]);
        }
    }
    else
    {
        idxcur = idx;
    }

    //
    ncur = points[idxcur].n;
    ncurparent = points[idxcur].parents[0];

    points[idxcur].parents[0] = -1;
    points[idxcur].children.push_back(ncurparent);

    //
    while(n>0)
    {
        n--;

        idxcur = indexofpoint(ncurparent);

        nparent = ncurparent;
        ncurparent = points[idxcur].parents[0];
        points[idxcur].parents[0] = ncur;
        points[idxcur].children[0] = ncurparent;
        ncur = nparent;
    }

    //
    if(points[idxcur].hasChildren())
    {
        if(points[idxcur].children[0]==-1)
            points[idxcur].children.clear();
    }

    //
    return 0;
}

int NCPointCloud::reverseLineSegment(long idx)
{
    //
    long idxcur;
    long ncur, ncurparent;
    long nparent;
    long n = 0;

    // tip point
    if(points[idx].parents[0]==-1)
    {
        idxcur = idx;
        while(points[idxcur].hasChildren())
        {
            idxcur = indexofpoint(points[idxcur].children[0]);
            ++n;
        }
    }
    else
    {
        long idxtest = idx;
        while(points[idxtest].parents[0]!=-1)
        {
            idxtest = indexofpoint(points[idxtest].parents[0]);
            ++n;
        }

        idxcur = idx;
    }

    //
    ncur = points[idxcur].n;
    ncurparent = points[idxcur].parents[0];

    points[idxcur].parents[0] = -1;
    points[idxcur].children.push_back(ncurparent);

    //
    while(n>0)
    {
        n--;

        idxcur = indexofpoint(ncurparent);

        nparent = ncurparent;
        ncurparent = points[idxcur].parents[0];
        points[idxcur].parents[0] = ncur;
        points[idxcur].children[0] = ncurparent;
        ncur = nparent;
    }

    //
    if(points[idxcur].hasChildren())
    {
        if(points[idxcur].children[0]==-1)
        {
            cout<<"test reverse func "<<idxcur<<" = "<<idx<<endl;
            points[idxcur].children.clear();
        }
    }

    //
    return 0;
}

//
int NCPointCloud::connectLineSegments(long rooti, long tipj, float angle)
{
    //
    if(threshDistNN<0)
    {
        knnMaxDist(threshDistNN);
        threshDistNN *= 2;
    }

    if(distance(points[rooti], points[tipj])>threshDistNN)
    {
        skipConnecting.appendPair(points[rooti], points[tipj]);
        return -1;
    }

    //
    if(points[rooti].hasChildren())
    {
        long idx = indexofpoint(points[rooti].children[0]);

        if(getAngle(points[idx], points[rooti], points[tipj]) > angle)
        {
            skipConnecting.appendPair(points[rooti], points[tipj]);
            return -1;
        }
    }
    else
    {

    }

    long idx = indexofpoint(points[tipj].parents[0]);
    if(getAngle(points[rooti], points[tipj], points[idx]) > angle)
    {
        skipConnecting.appendPair(points[rooti], points[tipj]);
        return -1;
    }

    //
    points[rooti].parents[0] = points[tipj].n;
    points[tipj].children.push_back(points[rooti].n);

    //
    return 0;
}

//
int NCPointCloud::append(Point p)
{
    // add a new point
    float distthresh;
    bool added = false;
    for(long i=0; i<points.size(); i++)
    {
        //
        Point q = points[i];

        distthresh = p.radius + q.radius;

        if(distance(p,q)<distthresh)
        {
            if(q.radius < p.radius)
            {
                added = true;

                points[i].x = p.x;
                points[i].y = p.y;
                points[i].z = p.z;
                points[i].radius = p.radius;
            }
            else
            {
                // not adding
            }

            return 0;
        }
    }

    if(!added)
    {
        points.push_back(p);
    }

    //
    return 0;
}

//
int NCPointCloud::sortbyradius()
{
    //
    sort(points.begin(), points.end(), [](const Point& a, const Point& b) -> bool
    {
        return a.radius < b.radius;
    });

    //
    return true;
}

//
float NCPointCloud::hausdorffdistance(NCPointCloud a, NCPointCloud b)
{
    float h = 0;

    for(size_t i=0; i<a.points.size(); i++)
    {
        Point p = a.points[i];
        float d = 1e6;
        for(size_t j=0; j<b.points.size(); j++)
        {
            Point q = b.points[j];

            float dist = distance(p,q);

            if(dist < d)
                d = dist;
        }

        if(d > h)
            h = d;
    }

    return h;
}

float NCPointCloud::meandistance(NeuronTree a, NeuronTree b)
{
    float m = 0;
    long n = a.listNeuron.size();

    if(n<=0)
    {
        cout<<"Invalid input\n";
        return -1;
    }

    for(size_t i=0; i<n; i++)
    {
        Point p;
        p.x = a.listNeuron[i].x;
        p.y = a.listNeuron[i].y;
        p.z = a.listNeuron[i].z;

        float d = 1e6;
        for(size_t j=0; j<b.listNeuron.size(); j++)
        {
            Point q;
            q.x = b.listNeuron[j].x;
            q.y = b.listNeuron[j].y;
            q.z = b.listNeuron[j].z;

            float dist = distance(p,q);

            if(dist < d)
                d = dist;
        }

        m += d;
    }

    return m/n;
}

// method to publish
int NCPointCloud::trace()
{
    // input: detected points w/ radius info
    // output: traced neuron trees

    // 1. connect points into lines

    // 2. merge lines

    // 3. connect lines into trees

    //
    return 0;
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
    b_bbox = false;
    visited = false;
    b_updated = false;
}

LineSegment::~LineSegment()
{
    meanval_adjangles = 0;
    stddev_adjangles = 0;
    points.clear();
    b_bbox = false;
    visited = false;
    b_updated = false;
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

int LineSegment::boundingbox()
{
    if(points.size()>0)
    {
        for(long i=0; i<points.size(); i++)
        {
            Point p = points[i];

            if(i==0)
            {
                pbbs.x = pbbe.x = p.x;
                pbbs.y = pbbe.y = p.y;
                pbbs.z = pbbe.z = p.z;
            }
            else
            {
                if(p.x<pbbs.x)
                    pbbs.x = p.x;
                if(p.x>pbbe.x)
                    pbbe.x = p.x;

                if(p.y<pbbs.y)
                    pbbs.y = p.y;
                if(p.y>pbbe.y)
                    pbbe.y = p.y;

                if(p.z<pbbs.z)
                    pbbs.z = p.z;
                if(p.z>pbbe.z)
                    pbbe.z = p.z;
            }
        }
    }
    else
    {
        return -1;
    }

    b_bbox = true;

    //
    return 0;
}

bool LineSegment::insideLineSegments(Point p)
{
    //    if(b_bbox == false)
    //        boundingbox();

    //    if(p.x >= pbbs.x && p.x <= pbbe.x && p.y >= pbbs.y && p.y <= pbbe.y && p.z >= pbbs.z && p.z <= pbbe.z)
    //        return true;

    if(b_updated == false)
        update();

    float rt = distance(root, tip);
    if(distance(root, p) < rt || distance(tip, p) < rt )
        return true;

    return false;
}

bool LineSegment::onSegment(Point p)
{
    if(b_updated == false)
        update();

    float rt = distance(root, tip);
    if(distance(root, p) < rt || distance(tip, p) < rt )
        return true;

    if(p.x <= max(root.x,tip.x) && p.x >= min(root.x,tip.x) && p.y <= max(root.y,tip.y) && p.y >= min(root.y, tip.y) && p.z <= max(root.z,tip.z) && p.z >= min(root.z, tip.z))
        return true;

    return false;
}

int LineSegment::lineFromPoints()
{
    auto start = std::chrono::high_resolution_clock::now();

    // copy coordinates to  matrix in Eigen format
    size_t num_atoms = points.size();
    Eigen::Matrix< Eigen::Vector3d::Scalar, Eigen::Dynamic, Eigen::Dynamic > centers(num_atoms, 3);
    for (size_t i = 0; i < num_atoms; ++i)
    {
        centers.row(i) = Eigen::Vector3d(points[i].x, points[i].y, points[i].z);
    }

    Eigen::Vector3d v_origin = centers.colwise().mean();

    origin = new Point;

    origin->x = v_origin(0);
    origin->y = v_origin(1);
    origin->z = v_origin(2);

    Eigen::MatrixXd centered = centers.rowwise() - v_origin.transpose();
    Eigen::MatrixXd cov = centered.adjoint() * centered;
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(cov);
    Eigen::Vector3d v_axis = eig.eigenvectors().col(2).normalized();

    axis = new Vector;

    axis->dx = v_axis(0);
    axis->dy = v_axis(1);
    axis->dz = v_axis(2);

    auto end = std::chrono::high_resolution_clock::now();

    //cout<<"estimate a line in "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<endl;

    //
    return 0;
}

int LineSegment::update()
{
    // update bounding box
    //boundingbox();

    // update line equation (origin, axis)
    lineFromPoints();

    // update start/root and end/tip points
    for(long i=0; i<points.size(); i++)
    {
        if(points[i].parents[0] == -1)
        {
            root = points[i];
        }

        if(!points[i].hasChildren())
        {
            tip = points[i];
        }
    }

    b_updated = true;

    //
    return 0;
}

float LineSegment::angleLine2Line(LineSegment ls)
{
    // 1. angle; 2. =0 near colinear; 3. =inf near parallel


    // tolerance
    float tolerance = 0.01;

    //
    Vector *v;
    v = v->vcross(axis, ls.axis);

    //
    Vector *diff = new Vector();
    diff->dx = origin->x - ls.origin->x;
    diff->dy = origin->y - ls.origin->y;
    diff->dz = origin->z - ls.origin->z;

    //
    if(v->vmag(v) < tolerance)
    {
        if(diff->vmag(diff->vcross(diff,axis)) < tolerance)
        {
            // colinear
            return 0;
        }
        else
        {
            // parallel
            return 6.28; // INF angle
        }
    }
    else
    {
        //return acos(v->vdot(axis, ls.axis) / (v->vmag(axis)*v->vmag(ls.axis)));
        float angle = acos(v->vdot(axis, diff) / (v->vmag(axis)*v->vmag(diff)));
        if(angle > 3.14 - angle)
            angle = 3.14 - angle;

        return angle;
    }
}

bool LineSegment::sidebyside(LineSegment ls)
{
    //
    Vector *diff = new Vector();
    diff->dx = ls.origin->x - origin->x;
    diff->dy = ls.origin->y - origin->y;
    diff->dz = ls.origin->z - origin->z;

    cout<<"test ... ... angle: "<<angleLine2Line(ls)<<endl;
    cout<<"test ... ... diff angle: "<<acos(diff->vdot(axis, diff) / (diff->vmag(axis)*diff->vmag(diff)))<<endl;


    return false;

    //

    Point maxPosi, minPosi, maxPosj, minPosj;

    if(points.size()<1 || ls.points.size()<1)
    {
        return false;
    }

    // i
    maxPosi = minPosi = points[0];
    for(size_t i=1; i<points.size(); i++)
    {
        Point p = points[i];

        if(maxPosi.x < p.x)
            maxPosi.x = p.x;
        if(minPosi.x > p.x)
            minPosi.x = p.x;


        if(maxPosi.y < p.y)
            maxPosi.y = p.y;
        if(minPosi.y > p.y)
            minPosi.y = p.y;

        if(maxPosi.z < p.z)
            maxPosi.z = p.z;
        if(minPosi.z > p.z)
            minPosi.z = p.z;
    }

    // j
    maxPosj = minPosj = ls.points[0];
    for(size_t i=1; i<ls.points.size(); i++)
    {
        Point p = ls.points[i];

        if(maxPosj.x < p.x)
            maxPosj.x = p.x;
        if(minPosj.x > p.x)
            minPosj.x = p.x;


        if(maxPosj.y < p.y)
            maxPosj.y = p.y;
        if(minPosj.y > p.y)
            minPosj.y = p.y;

        if(maxPosj.z < p.z)
            maxPosj.z = p.z;
        if(minPosj.z > p.z)
            minPosj.z = p.z;
    }

    cout<<"side by side testing ... \n";
    maxPosi.info();
    minPosi.info();

    maxPosj.info();
    minPosj.info();

    //
    if((maxPosi.x < minPosj.x && maxPosi.z < minPosj.z)
            || (maxPosi.x < minPosj.x && maxPosi.y < minPosj.y)
            || (maxPosi.y < minPosj.y && maxPosi.z < minPosj.z)
            || (maxPosj.x < minPosi.x && maxPosj.y < minPosi.y)
            || (maxPosj.x < minPosi.x && maxPosj.z < minPosi.z)
            || (maxPosj.y < minPosi.y && maxPosj.z < minPosi.z))
    {
        return true;
    }

    //
    return false;
}

void LineSegment::info()
{
    for(size_t i=1; i<points.size(); i++)
    {
        points[i].info();
    }
}

bool LineSegment::isSmooth()
{
    // evaluate the neighbor angle
    if(points.size()<3)
    {
        cout<<"Not enough points for statistics\n";
        return -1;
    }
    else
    {
        //
        vector<Point> pc;
        for(size_t i=0; i<points.size(); i++)
        {
            pc.push_back(points[i]);
        }

        //
        Point firstPoint = pc.back();
        pc.pop_back();

        Point secondPoint = pc.back();
        pc.pop_back();


        long n = 0;
        float sum = 0;

        vector<float> angles;

        while(!pc.empty())
        {
            Point thirdPoint = pc.back();
            pc.pop_back();

            if(thirdPoint.x == secondPoint.x && thirdPoint.y == secondPoint.y && thirdPoint.z == secondPoint.z)
            {
                // duplicated point
                continue;
            }

            float angle = getAngle(firstPoint, secondPoint, thirdPoint);

            cout<<"... angle ... "<<angle/3.14*180<<endl;

            if(angle > 1.57)
                cout<<"!!!!!!!!!!!!!!!!!!"<<endl;

            sum += angle;
            angles.push_back(angle);

            n++;

            firstPoint = secondPoint;
            secondPoint = thirdPoint;
        }

        //
        meanval_adjangles = sum / n;

        //
        if(n>1)
        {
            sum = 0;
            for(int i=0; i<angles.size(); i++)
            {
                sum += (angles[i] - meanval_adjangles)*(angles[i] - meanval_adjangles);
            }

            stddev_adjangles = sqrt(sum/(n-1));
        }
        else
        {
            stddev_adjangles = 0;
        }

        cout<<"mean angle ... "<<meanval_adjangles/3.14*180<<" std dev ..."<<stddev_adjangles<<endl;
    }

    return true;
}
