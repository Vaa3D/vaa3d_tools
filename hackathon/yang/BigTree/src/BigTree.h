// BigTree.h


#include "Image.h"

#define MAX_IMAGES_STREAM 16

// meta
// Folder: Y/Y_X
// File: Y_X_Z.tif

// block
class Cube
{
public:
    Cube();
    ~Cube();

public:
    int offset_D;

    string fileName;
    uint32 depth;
};

// folder
class YXFolder
{
public:
    YXFolder();
    ~YXFolder();

public:
    int offset_V;
    int offset_H;

    uint16 lengthFileName; // 25 len("000000/000000_000000.tif") + 1
    uint16 lengthDirName; // 21 len("000000_000000_000000") + 1

    string dirName;

    uint32 height, width, color;
    uint32 bytesPerVoxel;
};

// layer
class Scale
{
public:
    Scale();
    ~Scale();

public:
    map<string, YXFolder> layer;

    uint16 rows, cols;
    float vs_x, vs_y, vs_z; // voxel sizes
    uint32 dim_V, dim_H, dim_D;
};

// folder
class BLOCK
{
public:
    BLOCK();
    ~BLOCK();

public:
    int findNonZeroBlocks();

public:
    int offset_V;
    int offset_H;
    vector<int> offsets_D;

    uint16 lengthFileName; // 25 len("000000/000000_000000.tif") + 1
    uint16 lengthDirName; // 21 len("000000_000000_000000") + 1

    string dirName;

    uint32 nBlocksPerDir;
    vector<string> fileNames;
    vector<uint32> depths;
    vector<bool> nonZeroBlocks;

    uint32 height, width, color;
    uint32 bytesPerVoxel; // assume bytesPerVoxel is same

    bool bWrite;
};

// resolution
class LAYER
{
public:
    LAYER();
    ~LAYER();

public:
    uint16 rows, cols;
    float vs_x, vs_y, vs_z; // voxel sizes
    uint32 dim_V, dim_H, dim_D;

    vector<BLOCK> blocks;
    uint32 n_scale;


};

// blocks in each resolution
class TMITREE
{
public:
    TMITREE();
    ~TMITREE();

public:
    vector<LAYER> layers;
    float org_V, org_H, org_D; // offsets (0, 0, 0)
    axis reference_V, reference_H, reference_D; // vertical, horizonal, depth
    float mdata_version; // 2

    map<int, Scale> scales;
};

//
class BigTree
{
public:
    BigTree(string inputdir, string outputdir, int scales, int genMetaInfo=0, bool genZeroData=false);
    ~BigTree();

public:
    int init();
    uint8* load(long zs, long ze);
    int reformat();

    // mdata.bin
    int index();

public:
    string srcdir, dstdir;
    int resolutions;
    uint32 width, height, depth; // 3D image stacks

    set<string> input2DTIFFs;
    uint32 *n_stacks_V, *n_stacks_H, *n_stacks_D;
    uint32 ****stacks_V, ****stacks_H, ****stacks_D;
    vector<string> filePaths;
    uint32 block_width, block_height, block_depth;
    uint16 datatype;
    uint32 color;
    int *halve_pow2;
    long z_ratio, z_max_res;
    uint8 *ubuffer;
    int nbits;

    TMITREE meta;
    int genMetaInfoOnly;
    bool genZeroDataOnly;
};
