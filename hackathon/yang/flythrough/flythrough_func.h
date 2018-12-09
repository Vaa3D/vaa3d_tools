/* flythrough_func.h
 * a plugin to create a fly-through virtual reality
 * 11/20/2018 : by Yang Yu
 */
 
#ifndef __NEURONRECON_FUNC_H__
#define __NEURONRECON_FUNC_H__

//
#include <QtGlobal>
#include <vector>
#include "v3d_interface.h"
#include "v3d_message.h"
#include <iostream>
#include "basic_4dimage.h"
#include <map>
#include <cmath>
#include <climits>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <string>
#include <cstring>
#include <tuple>
#include <stack>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unordered_map>

#include "tiffio.h"

using namespace std;

#define COMPPRESSION_METHOD COMPRESSION_LZW

enum  axis { vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0 };

//
char *tiffread(const char* filename, unsigned char *&p, uint32 &sz0, uint32  &sz1, uint32  &sz2, uint16 &datatype);
int readASWC_file(const QString& filename, QString &datapath, NeuronTree &nt);

// cube
class Cube
{
public:
    Cube();
    ~Cube();

public:
    int offset_D; // z

    string fileName; // 000000_000000_000000.tif
    string filePath; // ourdir/RESXXxXXxXX/000000/000000_000000/000000_000000_000000.tif
    unsigned int depth; // 256
    bool toBeCopied;
};

// folder
class YXFolder
{
public:
    YXFolder();
    ~YXFolder();

public:
    int offset_V; // y
    int offset_H; // x

    unsigned short lengthFileName; // 25 len("000000_000000_000000.tif") + 1
    unsigned short lengthDirName; // 21 len("000000/000000_000000") + 1

    string dirName; // 000000/000000_000000
    string xDirPath; // ourdir/RESXXxXXxXX/000000
    string yDirPath; // ourdir/RESXXxXXxXX/000000/000000_000000

    unsigned int height, width; // 256x256
    bool toBeCopied;
    unsigned int ncubes; // adaptive for keep only a few cubes

    map<int,Cube> cubes;
};

// layer
class Layer
{
public:
    Layer();
    ~Layer();

public:
    unsigned short rows, cols; // floor(dim_V/height)+1, floor(dim_H/width)+1
    unsigned int ncubes;
    float vs_x, vs_y, vs_z; // voxel sizes
    unsigned int dim_V, dim_H, dim_D; // dimensions y, x, z

    string layerName; // outdir/RESXXxXXxXX

    map<string, YXFolder> yxfolders; // <dirName, YXFolder>
};

template<class T>
void copyData(T *&p, V3DLONG psx, V3DLONG pex, V3DLONG psy, V3DLONG pey, V3DLONG psz, V3DLONG pez,
              T *q, V3DLONG qsx, V3DLONG qex, V3DLONG qsy, V3DLONG qey, V3DLONG qsz, V3DLONG qez);

// nodes of tree
class Block
{
public:
    Block();
    Block(string fn, V3DLONG xoff, V3DLONG yoff, V3DLONG zoff, V3DLONG sx, V3DLONG sy, V3DLONG sz);
    ~Block();

public:
    bool compare(Block b); // check whether this block is in the list
    void load();
    void release();
    unsigned char* data();
    void setID(V3DLONG key);

public:
    string filepath;
    V3DLONG offset_x, offset_y, offset_z;
    V3DLONG size_x, size_y, size_z;
    V3DLONG id; // key

    bool visited; // used, in/out of memory

    unsigned char *p;
    uint16 datatype; // in byte(s)
};

// metadata
typedef map<V3DLONG, Block> OneScaleTree; // key: offset_z*dimx*dimy+offset_y*dimx+offset_x
typedef vector<V3DLONG> OffsetType;
typedef map<V3DLONG, string> ZeroBlock;

// swc
class Point
{
public:
    Point();
    Point(float a, float b, float c);
    ~Point();

public:
    void release();
    unsigned char* data(unsigned int datatype, OneScaleTree tree); // crop recentered block data
    V3DLONG getSize(); // size of buffer
    V3DLONG getVoxels();
    void setBoundingBox(V3DLONG x, V3DLONG y, V3DLONG z);
    void setDatatype(unsigned int datatype);

public:
    float x,y,z;

    vector<V3DLONG> blocks; // hit blocks' IDs in OneScaleTree
    unsigned char *p; // cropped data
    V3DLONG sx, sy, sz; // size: [x-sx/2-1, x+sx/2], ...
    V3DLONG size;
    V3DLONG voxels;
    V3DLONG bytesPerVoxel; // bytes per voxel
};

typedef vector<Point> PointCloud;

// data flow

//
template<class T>
class Node
{
public:
    Node();
    ~Node();

public:
    V3DLONG key; // block id
    T value; // block
    Node *prev, *next;
};

// caching blocks in the storage
template<class T>
class LRUCache
{
public:
    LRUCache(int _capacity);
    ~LRUCache();

public:
    void add(V3DLONG key, T value);
    T get(V3DLONG key); // add one hit (use)
    void put(V3DLONG key, T value);
    void display(); // display contents of cache

public:
    int capacity, sz;
    unordered_map<V3DLONG, Node<T> *> lookup;
    Node<T> *head, *tail;
};

// producer
class GetData : public QThread
{
   // Q_OBJECT
public:
    void run();

public:
    int size;
};

// consumer
class PutData : public QThread
{
    Q_OBJECT

public:
    explicit PutData(int dataSize, QObject *parent = 0);

public:
    void run();

signals:
    void nodeProcessed(const int &n);

public:
    int size;
};

//
class DataFlow
{
    //Q_OBJECT
public:
    DataFlow(PointCloud *&pc, string inputdir, V3DLONG sx, V3DLONG sy, V3DLONG sz);
    ~DataFlow();

public:
    int readSWC(string filename, float ratio);
    int readMetaData(string filename, bool mDataDebug=false);

    int query(V3DLONG idx);
    vector<string> splitFilePath(string filepath);
    string getDirName(string filepath);
    int label(V3DLONG index);
    V3DLONG findOffset(OffsetType offsets, V3DLONG idx);

public:
    OneScaleTree tree;
    PointCloud *points;

public:
    // mdata.bin
    float org_V, org_H, org_D; // offsets (0, 0, 0)
    axis reference_V, reference_H, reference_D; // vertical, horizonal, depth
    float mdata_version; // 2

    unsigned int color, bytesPerVoxel; //
    V3DLONG cubex, cubey, cubez;
    V3DLONG sx, sy, sz;

    OffsetType xoff, yoff, zoff;
    ZeroBlock zeroblocks;

    Layer layer;

    // fly through
    QCache<V3DLONG, Block> dataLoaded; // in memory, by default its capacity is 100 < 5*27 less than 5 GB for each chunk with 256x256x256
    int flydirection; // forward 1 / backward 0
};

// functions
bool flythrough_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);


#endif

