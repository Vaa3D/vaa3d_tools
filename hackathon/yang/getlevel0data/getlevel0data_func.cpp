/* getlevel0data_func.cpp
 * a plugin to get level 0 data
 * 10/10/2018 : by Yang Yu
 */

//
#include "getlevel0data_func.h"

#if  defined(Q_OS_LINUX)
#include <omp.h>
#endif

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
Point::Point(float a, float b, float c)
{
    x = a;
    y = b;
    z = c;
}

Point::~Point()
{

}

//
Block::Block()
{

}

Block::Block(string fn, long xoff, long yoff, long zoff, long sx, long sy, long sz)
{
    filepath = fn;
    offset_x = xoff;
    offset_y = yoff;
    offset_z = zoff;
    size_x = sx;
    size_y = sy;
    size_z = sz;
}

Block::~Block()
{

}

//
QueryAndCopy::QueryAndCopy(string swcfile, string inputdir, string outputdir, float ratio)
{
    // inputdir: "xxxx/RES(123x345x456)"
    // outputdir: "yyyy/RES(123x345x456)"

    // load input .swc file and mdata.bin

    //
    readSWC(swcfile, ratio);

    //
    readMetaData(inputdir);


    // find hit & neighbor blocks

    //
    long n = pc.size();

    cout<<" ... ... consider "<<n<<" nodes in "<<tree.size()<<" blocks"<<endl;

    for(long i=0; i<n; i++)
    {
        Point p = pc[i];

        query(p.x, p.y, p.z);
    }

    // copying blocks and saving mdata.bin

    //
    DIR *outdir = opendir(outputdir.c_str());
    if(outdir == NULL)
    {
        // mkdir outdir
        if(makeDir(outputdir))
        {
            cout<<"fail in mkdir "<<outputdir<<endl;
            return;
        }
    }
    else
    {
        closedir(outdir);
    }

    //
    string mdatabin = outputdir + "/mdata.bin";

    struct stat info;

    // mdata.bin does not exist
    if( stat( mdatabin.c_str(), &info ) != 0 )
    {
        // save mdata.bin
        FILE *file;

        file = fopen(mdatabin.c_str(), "wb");

        fwrite(&(mdata_version), sizeof(float), 1, file);
        fwrite(&(reference_V), sizeof(axis), 1, file);
        fwrite(&(reference_H), sizeof(axis), 1, file);
        fwrite(&(reference_D), sizeof(axis), 1, file);
        fwrite(&(layer.vs_x), sizeof(float), 1, file);
        fwrite(&(layer.vs_y), sizeof(float), 1, file);
        fwrite(&(layer.vs_z), sizeof(float), 1, file);
        fwrite(&(layer.vs_x), sizeof(float), 1, file);
        fwrite(&(layer.vs_y), sizeof(float), 1, file);
        fwrite(&(layer.vs_z), sizeof(float), 1, file);
        fwrite(&(org_V), sizeof(float), 1, file);
        fwrite(&(org_H), sizeof(float), 1, file);
        fwrite(&(org_D), sizeof(float), 1, file);
        fwrite(&(layer.dim_V), sizeof(unsigned int), 1, file);
        fwrite(&(layer.dim_H), sizeof(unsigned int), 1, file);
        fwrite(&(layer.dim_D), sizeof(unsigned int), 1, file);
        fwrite(&(layer.rows), sizeof(unsigned short), 1, file);
        fwrite(&(layer.cols), sizeof(unsigned short), 1, file);

        //
        map<string, YXFolder>::iterator iter = layer.yxfolders.begin();
        while(iter != layer.yxfolders.end())
        {
            //
            YXFolder yxfolder = (iter++)->second;

            if(yxfolder.toBeCopied==false)
                continue;

            createDir(outputdir, yxfolder.dirName);

            //
            fwrite(&(yxfolder.height), sizeof(unsigned int), 1, file);
            fwrite(&(yxfolder.width), sizeof(unsigned int), 1, file);
            fwrite(&(layer.dim_D), sizeof(unsigned int), 1, file); // depth of all blocks
            fwrite(&layer.ncubes, sizeof(unsigned int), 1, file);
            fwrite(&(color), sizeof(unsigned int), 1, file);
            fwrite(&(yxfolder.offset_V), sizeof(int), 1, file);
            fwrite(&(yxfolder.offset_H), sizeof(int), 1, file);
            fwrite(&(yxfolder.lengthDirName), sizeof(unsigned short), 1, file);
            fwrite(const_cast<char *>(yxfolder.dirName.c_str()), yxfolder.lengthDirName, 1, file);

            //
            map<int, Cube>::iterator it = yxfolder.cubes.begin();
            while(it != yxfolder.cubes.end())
            {
                //
                Cube cube = (it++)->second;

                if(cube.toBeCopied==false)
                    continue;

                //
                string srcFilePath = inputdir + "/" + yxfolder.dirName + "/" + cube.fileName;
                string dstFilePath = outputdir + "/" + yxfolder.dirName + "/" + cube.fileName;

                copyblock(srcFilePath, dstFilePath);

                //
                fwrite(&(yxfolder.lengthFileName), sizeof(unsigned short), 1, file);
                fwrite(const_cast<char *>(cube.fileName.c_str()), yxfolder.lengthFileName, 1, file);
                fwrite(&(cube.depth), sizeof(unsigned int), 1, file);
                fwrite(&(cube.offset_D), sizeof(int), 1, file);
            }
            fwrite(&(bytesPerVoxel), sizeof(unsigned int), 1, file);
        }
        fclose(file);
    }
}

QueryAndCopy::~QueryAndCopy()
{

}

int QueryAndCopy::makeDir(string dirname)
{
    //
    struct stat info;

    // if dir does not exist
    if( stat(dirname.c_str(), &info ) != 0 )
    {
        if(mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
        {
            cout<<"Fail to create folder: "<< dirname <<endl;
            return -1;
        }
    }

    //
    return 0;
}

int QueryAndCopy::copyblock(string srcFile, string dstFile)
{
    //
    std::ifstream  src(srcFile.c_str(), std::ios::binary);
    std::ofstream  dst(dstFile.c_str(), std::ios::binary);

    dst << src.rdbuf();

    src.close();
    dst.close();

    //
    return 0;
}

int QueryAndCopy::readSWC(string filename, float ratio)
{
    //
    NeuronTree nt = readSWC_file(QString(filename.c_str()));

    if(ratio>1)
    {
        for (long i =0; i< nt.listNeuron.size(); i++)
        {
            Point p(nt.listNeuron[i].x/ratio, nt.listNeuron[i].y/ratio, nt.listNeuron[i].z/ratio);
            pc.push_back(p);
        }
    }
    else
    {
        for (long i =0; i< nt.listNeuron.size(); i++)
        {
            Point p(nt.listNeuron[i].x, nt.listNeuron[i].y, nt.listNeuron[i].z);
            pc.push_back(p);
        }
    }

    //
    return 0;
}

int QueryAndCopy::readMetaData(string filename)
{
    //
    bool mDataDebug = false;

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

            char dirName[21];
            char fileName[25];

            //
            fread(&(yxfolder.height), sizeof(unsigned int), 1, file);
            fread(&(yxfolder.width), sizeof(unsigned int), 1, file);
            fread(&(layer.dim_D), sizeof(unsigned int), 1, file);
            fread(&layer.ncubes, sizeof(unsigned int), 1, file);
            fread(&(color), sizeof(unsigned int), 1, file);
            fread(&(yxfolder.offset_V), sizeof(int), 1, file);
            fread(&(yxfolder.offset_H), sizeof(int), 1, file);
            fread(&(yxfolder.lengthDirName), sizeof(unsigned short), 1, file);
            fread(&(dirName), yxfolder.lengthDirName, 1, file);

            yxfolder.dirName = dirName;

            //
            if(mDataDebug)
            {
                cout<<"... "<<endl;
                cout<<"HEIGHT "<<yxfolder.height<<endl;
                cout<<"WIDTH "<<yxfolder.width<<endl;
                cout<<"DEPTH "<<layer.dim_D<<endl;
                cout<<"N_BLOCKS "<<layer.ncubes<<endl;
                cout<<"N_CHANS "<<color<<endl;
                cout<<"ABS_V "<<yxfolder.offset_V<<endl;
                cout<<"ABS_H "<<yxfolder.offset_H<<endl;
                cout<<"str_size "<<yxfolder.lengthDirName<<endl;
                cout<<"DIR_NAME "<<yxfolder.dirName<<endl;
                // printf("DIR_NAME: %s\n",yxfolder.dirName.c_str());
            }

            //
            for(int j=0; j<layer.ncubes; j++)
            {
                //
                Cube cube;

                //
                fread(&(yxfolder.lengthFileName), sizeof(unsigned short), 1, file);
                fread(&(fileName), yxfolder.lengthFileName, 1, file);
                fread(&(cube.depth), sizeof(unsigned int), 1, file);
                fread(&(cube.offset_D), sizeof(int), 1, file);

                cube.fileName = fileName;

                yxfolder.cubes.insert(make_pair(cube.offset_D, cube));

                //
                Block block(blockNamePrefix + yxfolder.dirName + "/" + cube.fileName,
                            long(yxfolder.offset_H), long(yxfolder.offset_V), long(j)*long(layer.dim_D),
                            long(yxfolder.width), long(yxfolder.height), long(layer.dim_D) );

                if(count==0)
                {
                    cubex = yxfolder.width;
                    cubey = yxfolder.height;
                    cubez = layer.dim_D;
                    count++;
                }

                tree.insert(make_pair(long(block.offset_z)*sx*sy+long(block.offset_y)*sx+long(block.offset_x), block));

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

int QueryAndCopy::query(float x, float y, float z)
{
    cout<<"query "<<x<<" "<<y<<" "<<z<<endl;

    // find hit block and 6 neighbors
    if(tree.size()>0)
    {
        long nx = long(x)/cubex;
        long ny = long(y)/cubey;
        long nz = long(z)/cubez;

        // hit block

        long lx = nx*cubex;
        long ly = ny*cubey;
        long lz = nz*cubez;

        long index = lz*sx*sy + ly*sx + lx;

        cout<<"index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        // test
        map<long, Block>::iterator it = tree.begin();
        while(it != tree.end())
        {
            cout<<(it++)->first<<endl;
        }

        //
        if(tree.find(index) != tree.end())
        {
            cout<<"found a block"<<endl;

            Block block = tree[index];

            cout<<"block "<<block.filepath<<endl;

            string dirName = getDirName(block.filepath);

            cout<<"check dirName: "<<dirName<<endl;

            layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
            layer.yxfolders[dirName].toBeCopied = true;
        }

        // 6 neighbors

        // x-
        if(nx-1>0)
        {
            lx = (nx - 1) * cubex;
            index = lz*sx*sy + ly*sx + lx;

            if(tree.find(index) != tree.end())
            {
                Block block = tree[index];

                string dirName = getDirName(block.filepath);

                cout<<"check dirName: "<<dirName<<endl;

                layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
                layer.yxfolders[dirName].toBeCopied = true;
            }
        }

        // x+
        lx = (nx + 1) * cubex;
        index = lz*sx*sy + ly*sx + lx;

        if(tree.find(index) != tree.end())
        {
            Block block = tree[index];

            string dirName = getDirName(block.filepath);

            cout<<"check dirName: "<<dirName<<endl;

            layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
            layer.yxfolders[dirName].toBeCopied = true;
        }

        // y-
        lx = nx*cubex;

        if(ny-1>0)
        {
            ly = (ny - 1)*cubey;

            index = lz*sx*sy + ly*sx + lx;

            if(tree.find(index) != tree.end())
            {
                Block block = tree[index];

                string dirName = getDirName(block.filepath);

                cout<<"check dirName: "<<dirName<<endl;

                layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
                layer.yxfolders[dirName].toBeCopied = true;
            }
        }

        // y+
        ly = (ny + 1)*cubey;

        index = lz*sx*sy + ly*sx + lx;

        if(tree.find(index) != tree.end())
        {
            Block block = tree[index];

            string dirName = getDirName(block.filepath);

            cout<<"check dirName: "<<dirName<<endl;

            layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
            layer.yxfolders[dirName].toBeCopied = true;
        }

        // z-
        ly = ny*cubey;

        if(nz-1>0)
        {
            lz = (nz - 1)*cubez;

            index = lz*sx*sy + ly*sx + lx;

            if(tree.find(index) != tree.end())
            {
                Block block = tree[index];

                string dirName = getDirName(block.filepath);

                cout<<"check dirName: "<<dirName<<endl;

                layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
                layer.yxfolders[dirName].toBeCopied = true;
            }
        }

        // z+
        lz = (nz + 1)*cubez;

        index = lz*sx*sy + ly*sx + lx;

        if(tree.find(index) != tree.end())
        {
            Block block = tree[index];

            string dirName = getDirName(block.filepath);

            cout<<"check dirName: "<<dirName<<endl;

            layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
            layer.yxfolders[dirName].toBeCopied = true;
        }

    }
    else
    {
        cout<<"Read mdata.bin then query."<<endl;
        return -1;
    }

    //
    return 0;
}

vector<string> QueryAndCopy::splitFilePath(string filepath)
{
    cout<<filepath<<endl;

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

        cout<<splits.size()<<" "<<i<<" "<<pos<<endl;

        i = pos + 1;
        pos = filepath.find(delimiter, i);
    }

    splits.push_back(filepath.substr(i, min(pos, filepath.length()) - i + 1));

    return splits;
}

string QueryAndCopy::getDirName(string filepath)
{
    // filepath: xxxx/RES(123x456x789)/000/000_000/000_000_000.tif
    // dirName: 000/000_000
    // -------- splits[n-3] + "/" + splits[n-2]

    cout<<"call splitFilePath"<<endl;

    vector<string> splits = splitFilePath(filepath);

    cout<<"get "<<splits.size()<<" splits"<<endl;

    for(int i=0; i<splits.size(); i++)
        cout<<splits[i]<<endl;

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

int QueryAndCopy::createDir(string prePath, string dirName)
{
    //
    vector<string> splits = splitFilePath(dirName);

    if( splits.size() != 2 )
    {
        cout<<"Invalid dirName"<<endl;
        return -1;
    }

    string folder = prePath + "/" + splits[0];
    layer.yxfolders[dirName].xDirPath = folder;

    DIR *outdir = opendir(folder.c_str());
    if(outdir == NULL)
    {
        //
        if(makeDir(folder))
        {
            cout<<"fail in makeDir "<<folder<<endl;
            return -1;
        }
    }
    else
    {
        closedir(outdir);
    }

    folder = folder + "/" + splits[1];
    layer.yxfolders[dirName].yDirPath = folder;

    outdir = opendir(folder.c_str());
    if(outdir == NULL)
    {
        //
        if(makeDir(folder))
        {
            cout<<"fail in makeDir "<<folder<<endl;
            return -1;
        }
    }
    else
    {
        closedir(outdir);
    }

    //
    return 0;
}

//
bool getlevel0data_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1 || output.size() != 1)
    {
        cout<<"vaa3d -x getlevel0data -f getlevel0data -i inputdir input.swc -o outputdir -p scale<0/1/2/3/4/5/...>"<<endl;
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
            cout<<"vaa3d -x getlevel0data -f getlevel0data -i inputdir input.swc -o outputdir -p scale<0/1/2/3/4/5/...>"<<endl;
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

    QString outputdir = QString(outlist->at(0));

    float ratio = pow(2.0, scale);

    //
    QueryAndCopy qc(swcfile.toStdString(), inputdir.toStdString(), outputdir.toStdString(), ratio);

    //
    return true;
}
