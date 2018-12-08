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

template<class T>
void copyData(T *&p, V3DLONG psx, V3DLONG pex, V3DLONG psy, V3DLONG pey, V3DLONG psz, V3DLONG pez,
              T *q, V3DLONG qsx, V3DLONG qex, V3DLONG qsy, V3DLONG qey, V3DLONG qsz, V3DLONG qez)
{
    V3DLONG rpsx, rpex, rpsy, rpey, rpsz, rpez;
    V3DLONG rqsx, rqex, rqsy, rqey, rqsz, rqez;

    // w/ overlap
    if(psx>qex || pex<qsx || psy>qey || pey<qsy || psz>qez || pez<qsz)
    {
        return;
    }

    V3DLONG pw = pex - psx;
    V3DLONG ph = pey - psy;
    V3DLONG pd = pez - psz;

    V3DLONG qw = qex - qsx;
    V3DLONG qh = qey - qsy;
    V3DLONG qd = qez - qsz;

    //cout<<"test ... ... size ... "<<pw<<" "<<ph<<" "<<pd<<endl;
    //cout<<"test ... ... size ... "<<qw<<" "<<qh<<" "<<qd<<endl;

    // x overlap
    if(psx<=qsx && pex>=qsx && pex<=qex)
    {
        // ... |---------|
        // +-----+

        rqsx = 0;
        rqex = pex - qsx;

        rpsx = qsx - psx;
        rpex = pw-1;

    }
    else if(psx>=qsx && pex<=qex)
    {
        // |-------------|
        // ... +-----+

        rqsx = psx - qsx;
        rqex = rqsx + pw - 1;

        rpsx = 0;
        rpex = pw-1;
    }
    else if(psx<=qsx && pex>=qex)
    {
        // ... |------|
        // +-------------+

        rqsx = 0;
        rqex = qw-1;

        rpsx = qsx - psx;
        rpex = rpsx + qw - 1;
    }
    else if(psx>=qsx && psx<=qex && pex>=qex)
    {
        // |----------|
        // ... ... +-----+

        rqsx = psx - qsx;
        rqex = qw-1;

        rpsx = 0;
        rpex = qex - psx;
    }
    else
    {
        cout<<"what x-conditions fall in here?"<<endl;
    }

    // y overlap
    if(psy<=qsy && pey>=qsy && pey<=qey)
    {
        rqsy = 0;
        rqey = pey - qsy;

        rpsy = qsy - psy;
        rpey = ph-1;

    }
    else if(psy>=qsy && pey<=qey)
    {
        rqsy = psy - qsy;
        rqey = rqsy + ph - 1;

        rpsy = 0;
        rpey = ph-1;
    }
    else if(psy<=qsy && pey>=qey)
    {
        rqsy = 0;
        rqey = qh-1;

        rpsy = qsy - psy;
        rpey = rpsy + qh - 1;
    }
    else if(psy>=qsy && psy<=qey && pey>=qey)
    {
        rqsy = psy - qsy;
        rqey = qh-1;

        rpsy = 0;
        rpey = qey - psy;
    }
    else
    {
        cout<<"what y-conditions fall in here?"<<endl;
    }

    // z overlap
    if(psz<=qsz && pez>=qsz && pez<=qez)
    {
        rqsz = 0;
        rqez = pez - qsz;

        rpsz = qsz - psz;
        rpez = pd-1;

    }
    else if(psz>=qsz && pez<=qez)
    {
        rqsz = psz - qsz;
        rqez = rqsz + pd - 1;

        rpsz = 0;
        rpez = pd-1;
    }
    else if(psz<=qsz && pez>=qez)
    {
        rqsz = 0;
        rqez = qd-1;

        rpsz = qsz - psz;
        rpez = rpsz + qd - 1;
    }
    else if(psz>=qsz && psz<=qez && pez>=qez)
    {
        rqsz = psz - qsz;
        rqez = qd-1;

        rpsz = 0;
        rpez = qez - psz;
    }
    else
    {
        cout<<"what z-conditions fall in here?"<<endl;
    }

    //cout <<"p ... "<<rpsx<<" "<<rpex<<" "<<rpsy<<" "<<rpey<<" "<<rpsz<<" "<<rpez<<endl;
    //cout <<"q ... "<<rqsx<<" "<<rqex<<" "<<rqsy<<" "<<rqey<<" "<<rqsz<<" "<<rqez<<endl;

    //
    V3DLONG pz,qz,py,qy,qx,px;
    for(qz=rqsz, pz=rpsz; qz<rqez; qz++, pz++)
    {
        V3DLONG ofqz = qz*qw*qh;
        V3DLONG ofpz = pz*pw*ph;
        for(qy=rqsy, py=rpsy; qy<rqey; qy++, py++)
        {
            V3DLONG ofqy = ofqz + qy*qw;
            V3DLONG ofpy = ofpz + py*pw;
            for(qx=rqsx, px=rpsx; qx<rqex; qx++, px++)
            {
                //cout<<"copy "<<ofqy+qx<<" -> "<<ofpy+px<<endl;

                p[ofpy + px] = q[ofqy + qx];
            }
        }
    }

    //cout<<"done copy"<<endl;
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
    //QString skipBlock = "NULL.tif";
    // QString blockPath = QString::fromAscii(filepath.c_str());

    // load non-empty image
    //if(blockPath.indexOf(skipBlock)==-1)
    if(filepath.find("NULL.tif")==std::string::npos)
    {
        uint32 sx, sy, sz;
        // char *errormessage = tiffread(const_cast<char*>(blockPath.toUtf8().constData()),p,sx,sy,sz,datatype);
        char *errormessage = tiffread(filepath.c_str(),p,sx,sy,sz,datatype);

        cout<<"pointer p: "<<(void*)p<<endl;

        // update sx, sy, sz again
        size_x = sx;
        size_y = sy;
        size_z = sz;

        if(errormessage)
            cout<<"load "<<filepath<<" "<<errormessage<<endl;
    }

    visited = true;
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
    if(visited==false)
    {
        load();
    }

    cout<<"2 pointer p: "<<(void*)p<<endl;

    //
    return p;
}

void Block::setID(V3DLONG key)
{
    id = key;
}

Block::~Block()
{
    //release();
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
Point::Point()
{

}

Point::Point(float a, float b, float c)
{
    x = a;
    y = b;
    z = c;

    bytesPerVoxel = 1;
    voxels = 1;
    size = 1;

    p = NULL;
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

void Point::setDatatype(unsigned int datatype)
{
    bytesPerVoxel = datatype;
}

V3DLONG Point::getVoxels()
{
    return voxels;
}

V3DLONG Point::getSize()
{
    return size;
}

unsigned char* Point::data(unsigned int datatype, OneScaleTree tree)
{
    // tree will be replaced by LRUCache later

    setDatatype(datatype);

    // crop data from hit blocks
    try
    {
        V3DLONG startx,endx,starty,endy,startz,endz;

        startx = x - sx/2;

        if(startx<0)
            startx = 0;

        starty = y - sy/2;

        if(starty<0)
            starty = 0;

        startz = z - sz/2;

        if(startz<0)
            startz = 0;


        voxels = sx*sy*sz;
        size = voxels*bytesPerVoxel;

        p = new unsigned char [size];
        memset(p, 0, size);
    }
    catch(...)
    {
        cout<<"fail to allocate memory for point: ("<<x<<", "<<y<<", "<<z<<")"<<endl;
        return NULL;
    }

    if(blocks.empty())
    {
        cout<<"No hit blocks"<<endl;

        //
        return p;
    }
    else
    {
        //        for(int i=0; i<blocks.size(); i++)
        //        {

        //        }

        cout<<"blocks are not empty ... "<<endl;

        // test one block
        if(tree.find(blocks[0]) != tree.end())
        {

            cout<<"... block ... "<<blocks[0]<<" ... datatype ... "<<bytesPerVoxel<<endl;

            Block block = tree[blocks[0]];
            unsigned char *pBlock = block.data();

            cout<<"3 pointer p: "<<(void*)pBlock<<endl;

            if(pBlock)
            {
                if(bytesPerVoxel == 1)
                {

                }
                else if(bytesPerVoxel == 2)
                {
                    cout<<"16-bit data"<<endl;

                    unsigned short *pImg = (unsigned short *)p;
                    unsigned short *qImg = (unsigned short *)pBlock;

                    copyData<unsigned short>(pImg, x-sx/2, x+sx/2, y-sy/2, y+sy/2, z-sz/2, z+sz/2,
                                             qImg, block.offset_x, block.offset_x+block.size_x, block.offset_y, block.offset_y+block.size_y, block.offset_z, block.offset_z+block.size_z);
                }
            }
        }


        //
        return p;
    }
}

Point::~Point()
{
    //release();
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
    cout<<"call DataFlow "<<pc->size()<<endl;

    // neuron reconstruction (fragments/whole) with nodes/points coordinates (x, y, z)
    if(pc->size()<1)
    {
        cout<<"Need a valid neuron segment to work with"<<endl;
        return;
    }

    cout<<"points = pc"<<endl;
    points = pc;
    cout<<"test ... points "<<points->size()<<endl;

    // read a tree from "mdata.bin" in inputdir: "xxxx/RES(123x345x456)"
    readMetaData(inputdir);
    cout<<"test ... tree "<<tree.size()<<endl;

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
    inputdir.erase(std::remove_if(inputdir.begin(), inputdir.end(), [](unsigned char x){return std::isspace(x);}));

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
                QString qstrPath = QString::fromAscii(blockNamePrefix.c_str()).append(QString::fromAscii(yxfolder.dirName.c_str())).append(QString("/")).append(QString::fromAscii(cube.fileName.c_str()));
                Block block(qstrPath.toStdString(), //blockNamePrefix + yxfolder.dirName + "/" + cube.fileName,
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
char *tiffread(const char* filename, unsigned char *&p, uint32 &sz0, uint32  &sz1, uint32  &sz2, uint16 &datatype)
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
            cout<<"i "<<i<<endl;

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

    } while(TIFFReadDirectory(input));

    //
    TIFFClose(input);

    //
    return ((char *) 0);
}

//
int readASWC_file(const QString& filename, QString &datapath, NeuronTree &nt)
{
    // aswc file
    nt.file = QFileInfo(filename).absoluteFilePath();
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        cout<<"Invalid aswc file"<<endl;
        return -1;
    }

    //
    int count = 0;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    QString name = "";
    QString comment = "";

    qDebug("-------------------------------------------------------");
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        //  add #name, #comment
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
            for (buf++; (*buf && *buf==' '); buf++); //skip space

            if (buf[0]=='n'&&buf[1]=='a'&&buf[2]=='m'&&buf[3]=='e'&&buf[4]==' ')
            {
                name = buf+5;
            }
            else if (buf[0]=='c'&&buf[1]=='o'&&buf[2]=='m'&&buf[3]=='m'&&buf[4]=='e'&&buf[5]=='n'&&buf[6]=='t'&&buf[7]==' ')
            {
                comment = buf+89;
            }
            else if(buf[0]=='!')
            {
                QString finestscaledir = buf+1;
                QDir filepath = QFileInfo(filename).absoluteDir();
                datapath = filepath.absolutePath().append("/").append(finestscaledir);

                qDebug()<<"... ... finest resolution folder: "<<datapath;
            }

            continue;
        }

        count++;
        NeuronSWC S;

        QStringList qsl = QString(buf).trimmed().split(" ",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;

        for (int i=0; i<qsl.size(); i++)
        {
            qsl[i].truncate(99);
            if (i==0) S.n = qsl[i].toInt();
            else if (i==1) S.type = qsl[i].toInt();
            else if (i==2) S.x = qsl[i].toFloat();
            else if (i==3) S.y = qsl[i].toFloat();
            else if (i==4) S.z = qsl[i].toFloat();
            else if (i==5) S.r = qsl[i].toFloat();
            else if (i==6) S.pn = qsl[i].toInt();
            //the ESWC extension, by PHC, 20120217
            else if (i==7) S.seg_id = qsl[i].toInt();
            else if (i==8) S.level = qsl[i].toInt();
            else if (i==9) S.creatmode = qsl[i].toInt();
            else if (i==10) S.timestamp = qsl[i].toInt();
            //change ESWC format to adapt to flexible feature number, by WYN, 20150602
            else
                S.fea_val.append(qsl[i].toFloat());
        }

        //if (! listNeuron.contains(S)) // 081024
        {
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    qDebug("---------------------read %d lines, %d remained lines", count, listNeuron.size());

    if (listNeuron.size()<1)
    {
        cout<<"Empty aswc file"<<endl;
        return -2;
    }

    //now update other NeuronTree members
    nt.n = 1; //only one neuron if read from a file
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    nt.name = name.remove('\n'); if (nt.name.isEmpty()) nt.name = QFileInfo(filename).baseName();
    nt.comment = comment.remove('\n');

    return 0;
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

