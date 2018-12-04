/* flythrough_func.cpp
 * a plugin to create a fly-through virtual reality
 * 11/20/2018 : by Yang Yu
 */

//
//#include <fcntl.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <sys/sendfile.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <unistd.h>


//
#include "flythrough_func.h"

#if  defined(Q_OS_LINUX)
#include <omp.h>
#endif

// producer-consumer control nodes in neuron fragment
const int NNodes = 4; // buffer size

QSemaphore freeNodes(NNodes);
QSemaphore usedNodes;

//
const QString title = QObject::tr("get level 0 data");

//
Cube::Cube()
{
    toBeCopied = false;
}

Cube::~Cube()
{

}

//
YXFolder::YXFolder()
{
    lengthFileName = 25;
    lengthDirName = 21;
    toBeCopied = false;
}

YXFolder::~YXFolder()
{

}

//
Layer::Layer()
{
    vs_x = 1;
    vs_y = 1;
    vs_z = 1;
}

Layer::~Layer()
{

}

//
Point::Point()
{

}

Point::Point(float a, float b, float c)
{
    x = a;
    y = b;
    z = c;
}

void Point::release()
{
    if(p)
    {
        delete []p;
    }
}

void Point::setBoundingBox(V3DLONG x, V3DLONG y, V3DLONG z)
{
    sx = x;
    sy = y;
    sz = z;
}

unsigned char* Point::data()
{
    // crop data from hit blocks

    //
    return p;
}

Point::~Point()
{
    release();
}

//
Block::Block()
{
    p = NULL;
    visited = false;

    filepath = "";
    offset_x = 0;
    offset_y = 0;
    offset_z = 0;
    size_x = 1;
    size_y = 1;
    size_z = 1;
}

Block::Block(string fn, V3DLONG xoff, V3DLONG yoff, V3DLONG zoff, V3DLONG sx, V3DLONG sy, V3DLONG sz)
{
    p = NULL;
    visited = false;

    filepath = fn;
    offset_x = xoff;
    offset_y = yoff;
    offset_z = zoff;
    size_x = sx;
    size_y = sy;
    size_z = sz;
}

bool Block::compare(Block b)
{
    // if(offset_x == b.offset_x && offset_y == b.offset_y && offset_z == b.offset_z)

    //
    if(id == b.id)
        return true;
    else
        return false;
}

void Block::load()
{
    QString skipBlock = "NULL.tif";
    QString blockPath = QString::fromAscii(filepath.c_str());

    // load non-empty image
    if(blockPath.indexOf(skipBlock)==-1)
    {
        uint32 sx, sy, sz;
        char *errormessage = tiffread(const_cast<char*>(blockPath.toUtf8().constData()),p,sx,sy,sz,datatype);

        // update sx, sy, sz again
        size_x = sx;
        size_y = sy;
        size_z = sz;

        cout<<"load "<<filepath<<" "<<errormessage<<endl;
    }
}

void Block::release()
{
    if(p)
    {
        delete []p;
    }
}

unsigned char* Block::data()
{
    return p;
}

void Block::setID(V3DLONG key)
{
    id = key;
}

Block::~Block()
{
    release();
}

//
template<class T>
Node<T>::Node()
{
    prev = NULL;
    next = NULL;
}

template<class T>
Node<T>::~Node()
{
    prev = NULL;
    next = NULL;

    if(value.p)
    {
        value.release();
    }
}

//
template<class T>
LRUCache<T>::LRUCache(int _capacity)
{
    capacity = _capacity;
    sz = 0;
    lookup.clear();

    head = NULL;
    tail = NULL;
}

template<class T>
LRUCache<T>::~LRUCache()
{
    // release memory of lookup
}

template<class T>
void LRUCache<T>::add(V3DLONG key, T value)
{
    // pop head if necessary
    if(sz == capacity)
    {
        Node<T> *cur = lookup[head->key];

        if(head->next)
        {
            head->next->prev = NULL;
        }

        if(head == tail)
        {
            head = NULL;
            tail = NULL;
        }
        else
        {
            head = head->next;
        }

        lookup.erase(cur->key);
        delete cur;
        sz--;
    }

    // insert
    Node<T> *newNode = new Node<T>();
    newNode->key = key;
    newNode->value = value;
    newNode->prev = NULL;
    newNode->next = NULL;
    lookup[key] = newNode;

    if(head == NULL)
    {
        head = newNode;
        tail = newNode;
    }
    else
    {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }

    sz++;

    // load data ????
    lookup[key]->value.load();
}

template<class T>
T LRUCache<T>::get(V3DLONG key) // add one hit (use)
{
    if(lookup.find(key) != lookup.end())
    {
        // move node to tail
        Node<T> *node = lookup[key];

        if(node != tail && head != tail)
        {
            if(head == node)
            {
                head = head->next;
            }

            if(node->prev)
            {
                node->prev->next = node->next;
            }

            if(node->next)
            {
                node->next->prev = node->prev;
            }

            node->next = NULL;
            tail->next = node;
            node->prev = tail;
            tail = node;
        }

        //
        return lookup[key]->value;
    }

    return -1;
}

template<class T>
void LRUCache<T>::put(V3DLONG key, T value)
{
    if(lookup.find(key) == lookup.end())
    {
        add(key, value);
    }
    else
    {
        // move value to tail
        Node<T> *node = lookup[key];
        node->value = value;

        if(node == tail)
        {
            return;
        }

        if(head == tail)
        {
            return;
        }

        if(head == node)
        {
            head = head->next;
        }

        if(node->prev)
        {
            node->prev->next = node->next;
        }

        if(node->next)
        {
            node->next->prev = node->prev;
        }

        node->next = NULL;
        tail->next = node;
        node->prev = tail;
        tail = node;
    }
}

// display contents of cache
template<class T>
void LRUCache<T>::display()
{
    Node<T> * node = head;
    while(node != NULL)
    {
        cout << node->key << " ";
        node = node->next;
    }

    cout << endl;
}

//
void GetData::run()
{
    for(int i=0; i<size; ++i)
    {
        //
        freeNodes.acquire();

        // get data



        //
        usedNodes.release();
    }
}

void PutData::run()
{
//        for (int i = 0; i < DataSize; ++i) {
//            usedBytes.acquire();
//    #ifdef Q_WS_S60
//            QString text(buffer[i % BufferSize]);
//            freeBytes.release();
//            emit stringConsumed(text);
//    #else
//            fprintf(stderr, "%c", buffer[i % BufferSize]);
//            freeBytes.release();
//    #endif
//        }
//        fprintf(stderr, "\n");

    for(int i=0; i<size; ++i)
    {
        //
        usedNodes.acquire();

        //


        //
        freeNodes.release();
    }

}

//
DataFlow::DataFlow(PointCloud *&pc, string inputdir, V3DLONG sx, V3DLONG sy, V3DLONG sz)
{
    // neuron reconstruction (fragments/whole) with nodes/points coordinates (x, y, z)
    if(pc->size()<1)
    {
        cout<<"Need a valid neuron segment to work with"<<endl;
        return;
    }

    points = pc;

    // read a tree from "mdata.bin" in inputdir: "xxxx/RES(123x345x456)"
    readMetaData(inputdir);

    // update relations between points and tree blocks
    for(V3DLONG i=0; i<points->size(); i++)
    {
        points->at(i).setBoundingBox(sx, sy, sz);
        query(i);
    }

    //

}

DataFlow::~DataFlow()
{
    // release memory
}

int DataFlow::readSWC(string filename, float ratio)
{
    //
    NeuronTree nt = readSWC_file(QString(filename.c_str()));

    if(ratio>1)
    {
        for (V3DLONG i =0; i< nt.listNeuron.size(); i++)
        {
            Point p(nt.listNeuron[i].x/ratio, nt.listNeuron[i].y/ratio, nt.listNeuron[i].z/ratio);
            points->push_back(p);
        }
    }
    else
    {
        for (V3DLONG i =0; i< nt.listNeuron.size(); i++)
        {
            Point p(nt.listNeuron[i].x, nt.listNeuron[i].y, nt.listNeuron[i].z);
            points->push_back(p);
        }
    }

    //
    return 0;
}

int DataFlow::readMetaData(string filename, bool mDataDebug)
{
    //
    string inputdir = filename;

    DIR *outdir = opendir(inputdir.c_str());
    if(outdir == NULL)
    {
        cout<<"Empty folder: "<<inputdir<<endl;
        return -1;
    }
    else
    {
        closedir(outdir);
    }

    //
    string blockNamePrefix = inputdir + "/";

    //
    filename = inputdir + "/mdata.bin";

    struct stat info;

    // mdata.bin does not exist
    if( stat( filename.c_str(), &info ) != 0 )
    {
        cout<<filename<<" does not exist"<<endl;
        return -1;
    }
    else
    {
        // read
        FILE *file;

        file = fopen(filename.c_str(), "rb");

        fread(&(mdata_version), sizeof(float), 1, file);
        fread(&(reference_V), sizeof(axis), 1, file); // int
        fread(&(reference_H), sizeof(axis), 1, file);
        fread(&(reference_D), sizeof(axis), 1, file);
        fread(&(layer.vs_x), sizeof(float), 1, file);
        fread(&(layer.vs_y), sizeof(float), 1, file);
        fread(&(layer.vs_z), sizeof(float), 1, file);
        fread(&(layer.vs_x), sizeof(float), 1, file);
        fread(&(layer.vs_y), sizeof(float), 1, file);
        fread(&(layer.vs_z), sizeof(float), 1, file);
        fread(&(org_V), sizeof(float), 1, file);
        fread(&(org_H), sizeof(float), 1, file);
        fread(&(org_D), sizeof(float), 1, file);
        fread(&(layer.dim_V), sizeof(unsigned int), 1, file);
        fread(&(layer.dim_H), sizeof(unsigned int), 1, file);
        fread(&(layer.dim_D), sizeof(unsigned int), 1, file);
        fread(&(layer.rows), sizeof(unsigned short), 1, file);
        fread(&(layer.cols), sizeof(unsigned short), 1, file);

        sx = layer.dim_H;
        sy = layer.dim_V;
        sz = layer.dim_D;

        int count=0; // get cube size

        //
        if(mDataDebug)
        {
            cout<<"filename "<<filename<<endl;

            cout<<"meta.mdata_version "<<mdata_version<<endl;
            cout<<"meta.reference_V "<<reference_V<<endl;
            cout<<"meta.reference_H "<<reference_H<<endl;
            cout<<"meta.reference_D "<<reference_D<<endl;
            cout<<"layer.vs_x "<<layer.vs_x<<endl;
            cout<<"layer.vs_y "<<layer.vs_y<<endl;
            cout<<"layer.vs_z "<<layer.vs_z<<endl;
            cout<<"meta.org_V "<<org_V<<endl;
            cout<<"meta.org_H "<<org_H<<endl;
            cout<<"meta.org_D "<<org_D<<endl;
            cout<<"layer.dim_V "<<layer.dim_V<<endl;
            cout<<"layer.dim_H "<<layer.dim_H<<endl;
            cout<<"layer.dim_D "<<layer.dim_D<<endl;
            cout<<"layer.rows "<<layer.rows<<endl;
            cout<<"layer.cols "<<layer.cols<<endl;
        }

        //
        int n = layer.rows*layer.cols;
        for(int i=0; i<n; i++)
        {
            //
            YXFolder yxfolder;

            // char dirName[100]; // 21

            //
            fread(&(yxfolder.height), sizeof(unsigned int), 1, file);
            fread(&(yxfolder.width), sizeof(unsigned int), 1, file);
            fread(&(layer.dim_D), sizeof(unsigned int), 1, file);
            fread(&(yxfolder.ncubes), sizeof(unsigned int), 1, file);
            fread(&(color), sizeof(unsigned int), 1, file);
            fread(&(yxfolder.offset_V), sizeof(int), 1, file);
            fread(&(yxfolder.offset_H), sizeof(int), 1, file);
            fread(&(yxfolder.lengthDirName), sizeof(unsigned short), 1, file);

            string dirName(yxfolder.lengthDirName, '\0');

            fread(&(dirName[0]), sizeof(char), yxfolder.lengthDirName, file);

            yxfolder.dirName = dirName;

            //
            if(mDataDebug)
            {
                cout<<"... "<<endl;
                cout<<"HEIGHT "<<yxfolder.height<<endl;
                cout<<"WIDTH "<<yxfolder.width<<endl;
                cout<<"DEPTH "<<layer.dim_D<<endl;
                cout<<"N_BLOCKS "<<yxfolder.ncubes<<endl;
                cout<<"N_CHANS "<<color<<endl;
                cout<<"ABS_V "<<yxfolder.offset_V<<endl;
                cout<<"ABS_H "<<yxfolder.offset_H<<endl;
                cout<<"str_size "<<yxfolder.lengthDirName<<endl;
                cout<<"DIR_NAME "<<yxfolder.dirName<<endl;
                // printf("DIR_NAME: %s\n",yxfolder.dirName.c_str());
            }

            //
            for(int j=0; j<yxfolder.ncubes; j++)
            {
                //
                Cube cube;

                // char fileName[100]; // 25

                //
                fread(&(yxfolder.lengthFileName), sizeof(unsigned short), 1, file);

                string fileName(yxfolder.lengthFileName, '\0');

                fread(&(fileName[0]), sizeof(char), yxfolder.lengthFileName, file);
                fread(&(cube.depth), sizeof(unsigned int), 1, file);
                fread(&(cube.offset_D), sizeof(int), 1, file);

                cube.fileName = fileName;

                yxfolder.cubes.insert(make_pair(cube.offset_D, cube));

                //
                Block block(blockNamePrefix + yxfolder.dirName + "/" + cube.fileName,
                            V3DLONG(yxfolder.offset_H), V3DLONG(yxfolder.offset_V), V3DLONG(cube.offset_D),
                            V3DLONG(yxfolder.width), V3DLONG(yxfolder.height), V3DLONG(cube.depth) );

                if(count==0)
                {
                    cubex = yxfolder.width;
                    cubey = yxfolder.height;
                    cubez = cube.depth;
                    count++;
                }

                if (std::find(xoff.begin(), xoff.end(), V3DLONG(block.offset_x)) == xoff.end())
                {
                    xoff.push_back(V3DLONG(block.offset_x));
                }

                if (std::find(yoff.begin(), yoff.end(), V3DLONG(block.offset_y)) == yoff.end())
                {
                    yoff.push_back(V3DLONG(block.offset_y));
                }

                if (std::find(zoff.begin(), zoff.end(), V3DLONG(block.offset_z)) == zoff.end())
                {
                    zoff.push_back(V3DLONG(block.offset_z));
                }

                block.setID(V3DLONG(block.offset_z)*sx*sy+V3DLONG(block.offset_y)*sx+V3DLONG(block.offset_x));

                tree.insert(make_pair(block.id, block));

                //
                if(mDataDebug)
                {
                    cout<<"... ..."<<endl;
                    cout<<"str_size "<<yxfolder.lengthFileName<<endl;
                    cout<<"FILENAMES["<<cube.offset_D<<"] "<<cube.fileName<<endl;
                    cout<<"BLOCK_SIZE+i "<<cube.depth<<endl;
                    cout<<"BLOCK_ABS_D+i "<<cube.offset_D<<endl;
                }
            }
            fread(&(bytesPerVoxel), sizeof(unsigned int), 1, file);

            if(mDataDebug)
            {
                cout<<"N_BYTESxCHAN "<<bytesPerVoxel<<endl;
            }

            layer.yxfolders.insert(make_pair(yxfolder.dirName, yxfolder));
        }
        fclose(file);
    }

    //
    return 0;
}

int DataFlow::query(V3DLONG idx)
{
    //
    if(idx<0)
    {
        cout<<"Invalid index"<<endl;
        return -1;
    }

    float x = points->at(idx).x;
    float y = points->at(idx).y;
    float z = points->at(idx).z;

    // find hit block and 6 neighbors
    if(tree.size()>0)
    {
        // hit block
        V3DLONG lx = findOffset(xoff, V3DLONG(x));
        V3DLONG ly = findOffset(yoff, V3DLONG(y));
        V3DLONG lz = findOffset(zoff, V3DLONG(z));

        V3DLONG olx = lx;
        V3DLONG oly = ly;

        V3DLONG index = lz*sx*sy + ly*sx + lx;

        //cout<<"node's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        //
        // label(index);
        if(tree.find(index) != tree.end())
        {
            points->at(idx).blocks.push_back(index);
        }

        // 6 neighbors

        // x-
        lx = findOffset(xoff, V3DLONG(x-cubex));
        index = lz*sx*sy + ly*sx + lx;
        if(tree.find(index) != tree.end())
        {
            points->at(idx).blocks.push_back(index);
        }

        //cout<<"node's x- neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        // x+
        lx = findOffset(xoff, V3DLONG(x+cubex));
        index = lz*sx*sy + ly*sx + lx;
        if(tree.find(index) != tree.end())
        {
            points->at(idx).blocks.push_back(index);
        }

        //cout<<"node's x+ neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        lx = olx;

        // y-
        ly = findOffset(yoff, V3DLONG(y-cubey));
        index = lz*sx*sy + ly*sx + lx;
        if(tree.find(index) != tree.end())
        {
            points->at(idx).blocks.push_back(index);
        }

        //cout<<"node's y- neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        // y+
        ly = findOffset(yoff, V3DLONG(y+cubey));
        index = lz*sx*sy + ly*sx + lx;
        if(tree.find(index) != tree.end())
        {
            points->at(idx).blocks.push_back(index);
        }

        //cout<<"node's y+ neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        ly = oly;

        // z-
        lz = findOffset(yoff, V3DLONG(z-cubez));
        index = lz*sx*sy + ly*sx + lx;
        if(tree.find(index) != tree.end())
        {
            points->at(idx).blocks.push_back(index);
        }

        //cout<<"node's z- neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        // z+
        lz = findOffset(yoff, V3DLONG(z+cubez));
        index = lz*sx*sy + ly*sx + lx;
        if(tree.find(index) != tree.end())
        {
            points->at(idx).blocks.push_back(index);
        }

        //cout<<"node's z+ neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;
    }
    else
    {
        cout<<"Read mdata.bin then query."<<endl;
        return -1;
    }

    //
    return 0;
}

vector<string> DataFlow::splitFilePath(string filepath)
{
    vector<string> splits;
    char delimiter = '/';

    size_t i = 0;
    size_t pos = filepath.find(delimiter);

    if(pos==0)
    {
        // case "/usr/xxx"
        i = 1;
        pos = filepath.find(delimiter, i);
    }

    while(pos != string::npos)
    {
        splits.push_back(filepath.substr(i, pos-i));
        i = pos + 1;
        pos = filepath.find(delimiter, i);
    }

    splits.push_back(filepath.substr(i, min(pos, filepath.length()) - i + 1));

    return splits;
}

string DataFlow::getDirName(string filepath)
{
    // filepath: xxxx/RES(123x456x789)/000/000_000/000_000_000.tif
    // dirName: 000/000_000
    // -------- splits[n-3] + "/" + splits[n-2]

    vector<string> splits = splitFilePath(filepath);

    //
    size_t n = splits.size();

    if(n<3)
    {
        cout<<"Invalid filepath "<<filepath<<endl;
        return "";
    }

    string dirName = splits[n-3] + "/" + splits[n-2];

    //
    return dirName;
}

int DataFlow::label(V3DLONG index)
{
    //
    if(tree.find(index) != tree.end())
    {

//        Block block = tree[index];

//        if(block.visited == false)
//        {
//            cout<<"hits the block "<<block.filepath<<" "<<block.offset_x<<" "<<block.offset_y<<" "<<block.offset_z<<" "<<index<<endl;

//            string dirName = getDirName(block.filepath);

//            cout<<"check dirName: "<<dirName<<endl;

//            layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
//            layer.yxfolders[dirName].toBeCopied = true;
//            tree[index].visited = true;
//        }
    }

    return 0;
}

V3DLONG DataFlow::findOffset(OffsetType offsets, V3DLONG idx)
{
    V3DLONG n = offsets.size();

    //
    if(n<1)
    {
        cout<<"Invalid offsets/index"<<endl;
        return -1;
    }

    //
    if(idx<0)
    {
        idx = 0;
    }

    //
    V3DLONG mindist = abs(idx - offsets[0]);

    V3DLONG offset = offsets[0];
    size_t index = 0;

    //
    for(V3DLONG i=1; i<offsets.size(); i++)
    {
        V3DLONG dist = abs(idx - offsets[i]);

        if(dist<mindist)
        {
            mindist = dist;
            offset = offsets[i];
            index = i;
        }
    }

    if(idx<offsets[index])
        offset = offsets[index-1];

    return offset;
}

//
char *tiffread(char* filename, unsigned char *&p, uint32 &sz0, uint32  &sz1, uint32  &sz2, uint16 &datatype)
{
    //
    TIFF *input = TIFFOpen(filename,"r");
    if (!input)
    {
        return ((char *) "Cannot open the file.");
    }

    if (!TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &sz0))
    {
        TIFFClose(input);
        return ((char *) "Image width of undefined.");
    }

    if (!TIFFGetField(input, TIFFTAG_IMAGELENGTH, &sz1))
    {
        TIFFClose(input);
        return ((char *) "Image length of undefined.");
    }

    if (!TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &datatype))
    {
        TIFFClose(input);
        return ((char *) "Undefined bits per sample.");
    }

    uint16 Cpage;
    if (!TIFFGetField(input, TIFFTAG_PAGENUMBER, &Cpage, &sz2) || sz2==0)
    {
        sz2 = 1;
        while ( TIFFReadDirectory(input) )
        {
            sz2++;
        }
    }
    datatype /= 8;

    //cout<<"test "<<sz0<<" "<<sz1<<" "<<sz2<<" "<<datatype<<endl;

    V3DLONG imgsz = (V3DLONG)sz0*(V3DLONG)sz1*(V3DLONG)sz2*(V3DLONG)datatype;

    //
    try
    {
        p = new unsigned char [imgsz];
    }
    catch(...)
    {
        return ((char*) "fail to alloc memory for loading a tiff image.");
    }

    //
    uint32 rps;
    int StripsPerImage,LastStripSize;

    //
    if (!TIFFGetField(input, TIFFTAG_ROWSPERSTRIP, &rps))
    {
        TIFFClose(input);
        return ((char *) "Undefined rowsperstrip.");
    }

    uint16 comp;
    if (!TIFFGetField(input, TIFFTAG_COMPRESSION, &comp))
    {
        TIFFClose(input);
        return ((char *) "Undefined compression.");
    }

    StripsPerImage =  (sz1 + rps - 1) / rps;
    LastStripSize = sz1 % rps;
    if (LastStripSize==0)
        LastStripSize=rps;

    if (!TIFFSetDirectory(input, 0)) // init
    {
        TIFFClose(input);
        return ((char *) "fail to setdir.");
    }

    unsigned char *buf = p;

    do{
        for (int i=0; i < StripsPerImage-1; i++)
        {
            if (comp==1)
            {
                TIFFReadRawStrip(input, i, buf,  rps * sz0 * datatype);
                buf = buf + rps * sz0 * datatype;
            }
            else
            {
                TIFFReadEncodedStrip(input, i, buf, rps * sz0 * datatype);
                buf = buf + rps * sz0 * datatype;
            }
        }

        if (comp==1)
        {
            TIFFReadRawStrip(input, StripsPerImage-1, buf, LastStripSize * sz0 * datatype);
        }
        else
        {
            TIFFReadEncodedStrip(input, StripsPerImage-1, buf, LastStripSize * sz0 * datatype);
        }
        buf = buf + LastStripSize * sz0 * datatype;

    }
    while (TIFFReadDirectory(input)); // while (TIFFReadDirectory(input));

    //
    TIFFClose(input);

    //
    return ((char *) 0);
}

//
bool flythrough_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1 || output.size() != 1)
    {
        cout<<"vaa3d -x flythrough -f flythrough -i inputdir input.swc -o outputdir -p scale<0/1/2/3/4/5/...>"<<endl;
        return false;
    }

    // parsing input
    float scale=0; // 0 highest resolution
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            scale = atof(paras->at(0));
            cout<<"get level 0 data at the scale: "<<scale<<endl;
        }
        else
        {
            cout<<"Invalid input"<<endl;
            cout<<"vaa3d -x flythrough -f flythrough -i inputdir input.swc -p scale<0/1/2/3/4/5/...>"<<endl;
            return false;
        }
    }

    if(scale < 0 || scale > 6)
    {
        cout<<"Invalid scale!"<<endl;
        return false;
    }

    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify inputs"<<endl;
        return false;
    }

    //
    QString inputdir = QString(inlist->at(0));
    QString swcfile = QString(inlist->at(1));

    // QString outputdir = QString(outlist->at(0));

    float ratio = pow(2.0, scale);

    //
    // DataFlow qc(swcfile.toStdString(), inputdir.toStdString(), ratio);

    //
    return true;
}


