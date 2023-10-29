#include "volume_connection.h"
#include <fstream>

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
    visited = false;
}

Block::~Block()
{

}
//
QueryAndCopy::QueryAndCopy(string inputdir)
{
    // test
    readMetaData(inputdir, true);
}
int QueryAndCopy::readMetaData(string filename,bool mDataDebug)
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
                cout<<"DIR_NAME "<<yxfolder.dirName<<"."<<endl;
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
                            long(yxfolder.offset_H), long(yxfolder.offset_V), long(cube.offset_D),
                            long(yxfolder.width), long(yxfolder.height), long(cube.depth) );

                if(count==0)
                {
                    cubex = yxfolder.width;
                    cubey = yxfolder.height;
                    cubez = cube.depth;
                    count++;
                }

                if (std::find(xoff.begin(), xoff.end(), long(block.offset_x)) == xoff.end())
                {
                    xoff.push_back(long(block.offset_x));
                }

                if (std::find(yoff.begin(), yoff.end(), long(block.offset_y)) == yoff.end())
                {
                    yoff.push_back(long(block.offset_y));
                }

                if (std::find(zoff.begin(), zoff.end(), long(block.offset_z)) == zoff.end())
                {
                    zoff.push_back(long(block.offset_z));
                }

                tree.insert(make_pair(long(block.offset_z)*sx*sy+long(block.offset_y)*sx+long(block.offset_x), block));

                //
                if(mDataDebug)
                {
                    cout<<"... ..."<<endl;
                    cout<<"str_size "<<yxfolder.lengthFileName<<endl;
                    cout<<"FILENAMES["<<cube.offset_D<<"] "<<cube.fileName<<"."<<endl;
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

QueryAndCopy::QueryAndCopy(string inputdir, string outputdir, QStringList invols)
{
    readMetaData(inputdir,false);
    label_vols(invols);
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
    map<string, YXFolder>::iterator iter = layer.yxfolders.begin();
    while(iter != layer.yxfolders.end())
    {
        YXFolder yxfolder = (iter++)->second;
        layer.yxfolders[yxfolder.dirName].ncubes = yxfolder.cubes.size();
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
        fwrite(&(layer.rows), sizeof(unsigned short), 1, file); // need to be updated by hits
        fwrite(&(layer.cols), sizeof(unsigned short), 1, file); // need to be updated by hits

        cout<<layer.yxfolders.size()<<endl;

        string dirName = "zeroblocks/zeroblock"; //

        int count = 0;
        int nyxfolders = layer.yxfolders.size();
        map<string, YXFolder>::iterator iter = layer.yxfolders.begin();
        while(iter != layer.yxfolders.end())
        {
//            cout<<"testing count "<<count<<" of "<<nyxfolders<<endl;
            if(count++ >= nyxfolders)
            {
                iter++;
                continue;
            }

            //
            YXFolder yxfolder = (iter++)->second;

            if(yxfolder.toBeCopied==false)
            {
                //
                fwrite(&(yxfolder.height), sizeof(unsigned int), 1, file);
                fwrite(&(yxfolder.width), sizeof(unsigned int), 1, file);
                fwrite(&(layer.dim_D), sizeof(unsigned int), 1, file); // depth of all blocks
                fwrite(&(yxfolder.ncubes), sizeof(unsigned int), 1, file);
                fwrite(&(color), sizeof(unsigned int), 1, file);
                fwrite(&(yxfolder.offset_V), sizeof(int), 1, file);
                fwrite(&(yxfolder.offset_H), sizeof(int), 1, file);
                fwrite(&(yxfolder.lengthDirName), sizeof(unsigned short), 1, file);
                fwrite(const_cast<char *>(dirName.c_str()), yxfolder.lengthDirName, 1, file);

                //
                int countCube = 0;
                int ncubes = yxfolder.cubes.size();
                map<int, Cube>::iterator it = yxfolder.cubes.begin();
                while(it != yxfolder.cubes.end())
                {
                    if(countCube++ >= ncubes)
                    {
                        iter++;
                        continue;
                    }
                    //
                    Cube cube = (it++)->second;
                    string cubeName = "NULL.tif";
                    unsigned short lengthCubeName = cubeName.length() + 1; // consider the end is '\0'
                    fwrite(&(lengthCubeName), sizeof(unsigned short), 1, file);
                    fwrite(const_cast<char *>(cubeName.c_str()), lengthCubeName, 1, file);
                    fwrite(&(cube.depth), sizeof(unsigned int), 1, file);
                    fwrite(&(cube.offset_D), sizeof(int), 1, file);
                }
            }
            else
            {
                createDir(outputdir, yxfolder.dirName);
                //
                fwrite(&(yxfolder.height), sizeof(unsigned int), 1, file);
                fwrite(&(yxfolder.width), sizeof(unsigned int), 1, file);
                fwrite(&(layer.dim_D), sizeof(unsigned int), 1, file); // depth of all blocks
                fwrite(&(yxfolder.ncubes), sizeof(unsigned int), 1, file);
                fwrite(&(color), sizeof(unsigned int), 1, file);
                fwrite(&(yxfolder.offset_V), sizeof(int), 1, file);
                fwrite(&(yxfolder.offset_H), sizeof(int), 1, file);
                fwrite(&(yxfolder.lengthDirName), sizeof(unsigned short), 1, file);
                fwrite(const_cast<char *>(yxfolder.dirName.c_str()), yxfolder.lengthDirName, 1, file);

                //
                int countCube = 0;
                int ncubes = yxfolder.cubes.size();
                map<int, Cube>::iterator it = yxfolder.cubes.begin();
                while(it != yxfolder.cubes.end())
                {
                    //
                    if(countCube++ >= ncubes)
                    {
                        iter++;
                        continue;
                    }

                    //
                    Cube cube = (it++)->second;

                    if(cube.toBeCopied==false)
                    {
                        string cubeName = "NULL.tif";
                        unsigned short lengthCubeName = cubeName.length();

                        //
                        fwrite(&(lengthCubeName), sizeof(unsigned short), 1, file);
                        fwrite(const_cast<char *>(cubeName.c_str()), lengthCubeName, 1, file);
                        fwrite(&(cube.depth), sizeof(unsigned int), 1, file);
                        fwrite(&(cube.offset_D), sizeof(int), 1, file);
                    }
                    else
                    {

                        QString filePath = QString::fromAscii(yxfolder.dirName.c_str()).append(QString("/")).append(QString::fromAscii(cube.fileName.c_str()));
                        QString srcFilePath = QString::fromStdString(inputdir) + "/" + filePath;
                        QString dstFilePath = QString::fromStdString(outputdir) + "/" + filePath;

                        copyblock(srcFilePath, dstFilePath);

                        //
                        fwrite(&(yxfolder.lengthFileName), sizeof(unsigned short), 1, file);
                        fwrite(const_cast<char *>(cube.fileName.c_str()), yxfolder.lengthFileName, 1, file);
                        fwrite(&(cube.depth), sizeof(unsigned int), 1, file);
                        fwrite(&(cube.offset_D), sizeof(int), 1, file);
                    }
                }
            }
            fwrite(&(bytesPerVoxel), sizeof(unsigned int), 1, file);
        }
        fclose(file);
    }
}

QueryAndCopy::~QueryAndCopy()
{

}

int QueryAndCopy::label(long index)
{
    //
    if(tree.find(index) != tree.end())
    {
        Block block = tree[index];

        if(block.visited == false)
        {
            cout<<"hits the block "<<block.filepath<<" "<<block.offset_x<<" "<<block.offset_y<<" "<<block.offset_z<<" "<<index<<endl;

            string dirName = getDirName(block.filepath);

            cout<<"check dirName: "<<dirName<<endl;

            layer.yxfolders[dirName].cubes[block.offset_z].toBeCopied = true;
            layer.yxfolders[dirName].toBeCopied = true;
            tree[index].visited = true;
        }
    }

    return 0;
}


long QueryAndCopy::findOffset(OffsetType offsets, long idx)
{
    long n = offsets.size();

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
    long mindist = abs(idx - offsets[0]);

    long offset = offsets[0];
    size_t index = 0;

    //
    for(long i=1; i<offsets.size(); i++)
    {
        long dist = abs(idx - offsets[i]);

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

vector<string> QueryAndCopy::splitFilePath(string filepath)
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

string QueryAndCopy::getDirName(string filepath)
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

int QueryAndCopy::copyblock(QString srcFile, QString dstFile)
{
    std::ifstream  src(srcFile.toUtf8().constData(), std::ios_base::in | std::ios_base::binary);

    if(src.is_open())
    {
        std::ofstream  dst(dstFile.toUtf8().constData(), std::ios_base::out | std::ios_base::binary);

        if(dst.is_open())
        {
            dst << src.rdbuf();

            if(dst.bad())
            {
                cout<<"Error writing file "<<dstFile.toUtf8().constData()<<endl;
            }
        }
        else
        {
            cout<<"Error opening file "<<dstFile.toUtf8().constData()<<endl;
        }

        dst.close();
    }
    else
    {
        cout<<"Error opening file "<<srcFile.toUtf8().constData()<<endl;
    }

    src.close();
    return 0;
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
int QueryAndCopy::label_vols(QStringList invols){
    // input vol-path-list, label yxdir and cubes
    if(!invols.size()){return 0;}
//    cout<<"YXfolders="<<layer.yxfolders.size()<<endl;
    for(long long i=0; i<invols.size();i++){
        QString thisvol=invols.at(i);
        string thisvol_dirname=getDirName(thisvol.toStdString())+'\0';
        string thisvol_name=QFileInfo(thisvol).fileName().toStdString()+'\0';
//        cout<<thisvol_dirname<<","<<thisvol_dirname.size()<<endl;
        YXFolder this_yxfolder=layer.yxfolders.at(thisvol_dirname);
//        cout<<this_yxfolder.dirName<<"."<<endl;
//        break;
        // traversal cubes map<int,Cube> cubes;
        map<int, Cube>::iterator iter = this_yxfolder.cubes.begin();
        while(iter != this_yxfolder.cubes.end())
        {
            Cube tmp_cube = iter->second;
//            cout<<tmp_cube.fileName<<endl;
            if(tmp_cube.fileName==thisvol_name){
//                cout<<tmp_cube.offset_D<<endl;
                layer.yxfolders[thisvol_dirname].cubes[tmp_cube.offset_D].toBeCopied=true;
                layer.yxfolders[thisvol_dirname].toBeCopied = true;
                break;
            }
            iter++;
        }
    }
    return 1;
}

QVector<V3DLONG> cal_img_histogram(unsigned char * & data1d, V3DLONG datalen, V3DLONG histscale,double max_value)
{
    QVector<V3DLONG> hist;
    hist = QVector<V3DLONG>(histscale, 0);
    if(max_value<=0)
        for (V3DLONG i=0;i<datalen;i++)
            if(max_value<data1d[i])
                max_value=data1d[i];
    for (V3DLONG i=0;i<datalen;i++)
    {
        V3DLONG ind = data1d[i]/max_value * histscale;
        hist[ind] ++;
    }
    return hist;
}

double cal_img_entropy_1d(unsigned char * & data1d,V3DLONG * in_sz,V3DLONG histscale,V3DLONG hist_range){
    V3DLONG tol_sz=in_sz[0]*in_sz[1]*in_sz[2];
    QVector<V3DLONG> hist=cal_img_histogram(data1d,tol_sz,histscale,hist_range);
    double ent1d=0;

    for(V3DLONG i=0;i<hist.size();i++){
        double ip=double(hist.at(i))/double(tol_sz);
        if(ip<=0)
            continue;
        ent1d+=(- ip* log (ip));
    }
//    cout<<ent1d<<endl;
    return ent1d;
}

double cal_img_contrast(unsigned char * & data1d,V3DLONG * in_sz,uint ir){
    V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    long long tol_d=0; long tol_p=0;
    for(V3DLONG i=0;i<tol_sz;i++){
        V3DLONG ival=data1d[i];
        V3DLONG zs=i/sz01-ir; if(zs<0) {zs=0;}
        V3DLONG ze=i/sz01+ir; if(ze>=in_sz[2]) {ze=in_sz[2]-1;}
        V3DLONG ipage=V3DLONG(i/sz01)*sz01;
        for (V3DLONG iz= zs; iz <= ze; ++iz){
            V3DLONG ys=(i-ipage)/sz0-ir; if(ys<0) {ys=0;}
            V3DLONG ye=(i-ipage)/sz0+ir; if(ye<0) {ye=in_sz[1]-1;}
            V3DLONG ipage_row=V3DLONG((i-ipage)/sz0)*sz0;
            for (V3DLONG iy= ys; iy <= ye; ++iy){
                V3DLONG xs=i-ipage-ipage_row-ir; if(xs<0) {xs=0;}
                V3DLONG xe=i-ipage-ipage_row+ir; if(xe<0) {xe=in_sz[0]-1;}
                for (V3DLONG ix= xs; ix <= xe; ++ix){
                    if(i==iz*sz01+iy*sz0+ix){continue;}
                    V3DLONG tval=data1d[iz*sz01+iy*sz0+ix];
                    tol_d+=(tval-ival)*(tval-ival);
                    tol_p++;
                }
            }
        }
    }
    return double(tol_d/tol_p);
}

bool volume_signals(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For processing a volume, get the foreground voxels
     * input a dir, for all the tifs in dir, get volume features
            * filesize
            * intensity mean and std
            * SNR
            * image contrast
            * entropy-1d
            * entropy
            * intensity histogram
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    int hist_bins = 32; int to_8bit = 1;
    if(inparas.size() >= 1) hist_bins = atoi(inparas.at(0));
    if(inparas.size() >= 2) to_8bit = atoi(inparas.at(1));
   if(hist_bins<1)
       hist_bins=10;
//    char * inimg_file
    string inimg_file=  infiles[0];

    unsigned char * inimg1d = 0, *inimg1d_8bit =0;
    V3DLONG  in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    cout<<"data type = "<<datatype<<endl;
    V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
    V3DLONG min_value=0,max_value=255;
    int hist_bin_voxel=int((max_value-min_value)/hist_bins);
    V3DLONG voxel_nums[hist_bins];
    for(int i=0;i<hist_bins;i++)
        voxel_nums[i]=0;

    double imgAve, imgStd,imgContrast,imgEntropy1d;
    if(datatype>1&&to_8bit>0){
        cout<<"image convert to 8bit"<<endl;
        img_2_8bit(inimg1d,inimg1d_8bit,in_sz,datatype);        
        // volume contrast
        imgContrast=cal_img_contrast(inimg1d_8bit,in_sz,1);
        //image entropy
        imgEntropy1d=cal_img_entropy_1d(inimg1d_8bit,in_sz,256,256);
        mean_and_std(inimg1d_8bit, tol_sz, imgAve, imgStd);
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            int voxel_val=inimg1d_8bit[i];
            int voxel_range_i=int(voxel_val/hist_bin_voxel);
            voxel_nums[voxel_range_i]++;
        }
    }
    else{
        if(datatype==2)
            max_value=65535;
        else if(datatype>2)
            return false;
        hist_bin_voxel=int((max_value-min_value)/hist_bins);
        mean_and_std(inimg1d, tol_sz, imgAve, imgStd);
        // volume contrast
        imgContrast=cal_img_contrast(inimg1d,in_sz,1);
        //image entropy
        imgEntropy1d=cal_img_entropy_1d(inimg1d,in_sz,max_value,max_value);
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            int voxel_val=inimg1d[i];
            int voxel_range_i=int(voxel_val/hist_bin_voxel);
            voxel_nums[voxel_range_i]++;
        }
    }

    QString outfile=(outfiles.size()>=1)?outfiles[0]:(QString::fromStdString(inimg_file)+"_intensity_hist_count.csv");
    QFile qf_anofile(outfile);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        cout<<"Cannot open file for writing!"<<endl;
        return false;
    }

    QTextStream out(&qf_anofile);
    QString img_basename=QFileInfo(QString::fromStdString(inimg_file)).baseName();
    qint64 img_size=QFileInfo(QString::fromStdString(inimg_file)).size();
    out<<"file,"<<img_basename<<endl;
    out<<"size,"<<img_size<<endl;
    out<<"mean_val,"<<QString::number(imgAve)<<endl;
    out<<"std_val,"<<QString::number(imgStd)<<endl;
    out<<"snr,"<<QString::number(float(imgAve/imgStd))<<endl;
    out<<"img_contrast,"<<QString::number(imgContrast)<<endl;
    out<<"img_entropy1d,"<<QString::number(imgEntropy1d)<<endl;
    for(int i=0;i<hist_bins;i++)
        out<<"hist_"<<QString::number(i)<<","<<QString::number(double(voxel_nums[i])/(double)tol_sz)<<endl;
    delete [] inimg1d_8bit; inimg1d_8bit = 0;
    return true;
}
bool volume_filter(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For processing a volume, get the foreground voxels
     * 16-bit 3D volume :
     *
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    cout<<"Welcome to Volume filter !"<<endl;
    double bkg_thresh = 0; int to_8bit = 0;
    if(inparas.size() >= 1) bkg_thresh = atof(inparas.at(0));
    if(inparas.size() >= 2) to_8bit = atoi(inparas.at(1));

    char * inimg_file =  infiles[0];
    char * outimg_file = outfiles[0];

    unsigned char * inimg1d = 0, *inimg1d_8bit =0;
    V3DLONG  in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback,inimg_file, inimg1d, in_sz, datatype)) {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    cout<<"data type = "<<datatype<<endl;
    img_2_8bit(inimg1d,inimg1d_8bit,in_sz,datatype);

    V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
    if(bkg_thresh<=0)
    {
        double imgAve, imgStd;
        mean_and_std(inimg1d_8bit, tol_sz, imgAve, imgStd);
        cout<<"imgAve = "<<imgAve<<" ; image std = "<<imgStd<<endl;
        bkg_thresh=MIN(MAX(imgAve+imgStd+15,30),100);
    }
    cout<<"bkg_thresh = "<<bkg_thresh<<endl;
//    outimg1d = new unsigned char[tol_sz];
    unsigned char  * outimg1d = 0;
    try {outimg1d = new unsigned char [tol_sz];}
    catch(...)  {cout<<"cannot allocate memory for thresholding image."<<endl; return false;}
    for(V3DLONG i = 0; i < tol_sz; i++)
    {
        outimg1d[i]=(inimg1d_8bit[i]>=bkg_thresh)?int(inimg1d_8bit[i]):0;
    }
    in_sz[3]=1;
    simple_saveimage_wrapper(callback,outimg_file, (unsigned char *)outimg1d, in_sz, 1);
    delete [] outimg1d; outimg1d = 0;
    return true;
}
bool terafly_vol_index(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For a terafly volume in one resolution, index the related volumes in other resolution.
     * Input:
            * 1. terafly image path
            * 2. volume path
      * output dir
      * input para:
            * 1. volume res-level
            * 2. request res-level
       * Note:
            * 1. filename of terafly-volume is the coordinates of top-left-corner
            * 2. terafly-volume represents an index of virtual space in next-higher resolution.
            * 3. the volume-data-size is not equal with index-space.
            * 4. only volume size in highest-resolution represents index range of volume.
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.size()<2){cerr<<"Input error: at least two input paras"<<endl; return false;}
    QString inimg_file=  infiles[0];
    string invol_file=  infiles[1];

    int vol_res = 3; int to_res = 0;
    if(inparas.size() >= 1) vol_res = atoi(inparas.at(0));
    if(inparas.size() >= 2) to_res = atoi(inparas.at(1));
    QString outfile_path=(outfiles.size()>=1)?outfiles[0]:(QString::fromStdString(invol_file)+".csv");
    QFile outfile(outfile_path);
    if(!outfile.open(QIODevice::Append))
    {
        cout<<"Cannot open file for writing!"<<endl;
        return false;
    }
    QStringList tovols=terafly_vol_map(callback,inimg_file,invol_file,vol_res,to_res);
    if(!tovols.size()){cout<<"error in mapping to another terafly-level"<<endl;return false;}
    QTextStream out(&outfile);
    for(V3DLONG i=0;i<tovols.size();i++)
        out<<tovols.at(i)<<endl;
    return true;
}
bool terafly_mdata(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For terafly volumes in one resolution, output its metadata
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.size()<1){cerr<<"Input error: at least one input paras"<<endl; return false;}
    string inimg_file=  infiles[0];
    QueryAndCopy qac(inimg_file);
    return true;
}
bool terafly_vols_index(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For terafly volumes in one resolution, get all the indexed volumes in other resolution.
     * Input:
            * 1. terafly image path
            * 2. volume path/dir
      * output dir
      * input para:
            * 1. volume res-level
            * 2. request res-level
       * Note:
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.size()<2){cerr<<"Input error: at least two input paras"<<endl; return false;}
    QString inimg_file=  infiles[0];
    QString invol_dir=  infiles[1];

    int vol_res = 3; int to_res = 0;
    if(inparas.size() >= 1) vol_res = atoi(inparas.at(0));
    if(inparas.size() >= 2) to_res = atoi(inparas.at(1));
    QString outdir_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(invol_dir).path()+"/out");
    QDir outdir(outdir_path);QDir curdir;
    if(!outdir.exists())
    {
        if(!curdir.mkdir(outdir_path))
        {
            cout<<"Cannot create outdir!"<<endl;
            return false;
        }
    }
    //scan input dir and get all the volumes' name
    QStringList filefilters;
    filefilters<<"tiff"<<"tif"<<"v3draw"<<"v3dpbd";
    QStringList involfiles; involfiles.clear();
    if(!get_files_in_dir(invol_dir,involfiles,filefilters)){return false;}
    cout<<"Input volumes = "<<involfiles.size()<<endl;
    QFileInfoList sorted_terafly_dirs= sort_terafly_res_dirs(inimg_file);
    QString outres_dir_path=outdir_path+"/"+sorted_terafly_dirs.at(to_res).fileName();

    QStringList tovols; tovols.clear();
    for(V3DLONG i=0;i<involfiles.size();i++){
        string invol_file=involfiles.at(i).toStdString();
        QStringList this_tovols=terafly_vol_map(callback,inimg_file,invol_file,vol_res,to_res);
        if(!this_tovols.size()) {cout<<"error in mapping "<<invol_file<<endl; continue;}
        for(V3DLONG t=0;t<this_tovols.size();t++)
            tovols.append(this_tovols.at(t));
    }
    cout<<"From:"<<sorted_terafly_dirs.at(to_res).absoluteFilePath().toStdString()<<endl;
    cout<<"To:"<<outres_dir_path.toStdString()<<endl;
    QueryAndCopy qac(sorted_terafly_dirs.at(to_res).absoluteFilePath().toStdString(),outres_dir_path.toStdString(),tovols);
//    QueryAndCopy qac(sorted_terafly_dirs.at(to_res).absoluteFilePath().toStdString());
    return true;
}
bool get_files_in_dir(const QString& inpath,QStringList & outfiles,QStringList filefilters){
    QDir indir(inpath);
    if(!indir.exists()){cerr<<"Input error: not an existed input folder"<<endl; return false;}
    indir.setFilter(QDir::Dirs|QDir::NoSymLinks|QDir::Files);
    QFileInfoList rawdirs=indir.entryInfoList();
    for(int i=0;i<rawdirs.size();i++){
        QFileInfo thisdir=rawdirs.at(i);
        if(thisdir.fileName()=="."|thisdir.fileName()=="..")
            continue;
        if(thisdir.isDir())
            if(!get_files_in_dir(thisdir.filePath(),outfiles,filefilters))
                return false;
        if(thisdir.isFile()&&filefilters.contains(thisdir.suffix())){outfiles.append(thisdir.filePath());}
    }
    return true;
}

QFileInfoList sort_terafly_res_dirs(const QString& inimg_file){
    // highest resolution dir index=0
    QFileInfoList rawterafly_dirs, sorted_terafly_dirs;
    QDir inimg_path(inimg_file);
    if(!inimg_path.exists()){cerr<<"Input error: not an exist input folder"<<endl; return sorted_terafly_dirs;}
    inimg_path.setFilter(QDir::Dirs|QDir::NoSymLinks);
    inimg_path.setSorting(QDir::DirsFirst);
    QFileInfoList rawdirs=inimg_path.entryInfoList();

    for(int i=0;i<rawdirs.size();i++){
        QFileInfo thisdir=rawdirs.at(i);
        if(thisdir.fileName()=="."|thisdir.fileName()=="..")
            continue;
        if(thisdir.fileName().left(3)=="RES")
            rawterafly_dirs.append(thisdir);
    }
    // sort input terafly-res-dirs : from highest res to lowest res
    for(int i=1;i<rawterafly_dirs.size();i++){
        for(int j=1;j<=(rawterafly_dirs.size()-i);j++){
            QFileInfo thisdir=rawterafly_dirs.at(j-1);
            QFileInfo nextdir=rawterafly_dirs.at(j);
            QString this_res_cords=thisdir.fileName();
            QString next_res_cords=nextdir.fileName();
            long coordx=this_res_cords.split("x").at(1).toLong();
            long next_coordx=next_res_cords.split("x").at(1).toLong();
            if(coordx>next_coordx){
                QFileInfo tmp_resdir=rawterafly_dirs.at(j-1);
                rawterafly_dirs[j-1]=rawterafly_dirs[j];
                rawterafly_dirs[j]=tmp_resdir;
            }
        }
    }
    for(int i=0;i<rawterafly_dirs.size();i++)
        sorted_terafly_dirs.append(rawterafly_dirs.at(rawterafly_dirs.size()-i-1));
    return sorted_terafly_dirs;
}

QStringList terafly_vol_map(V3DPluginCallback2 &callback, const QString& inimg_file, string invol_file, int vol_res, int to_res)
{
    cout<<invol_file<<endl;
    QStringList tovols; tovols.clear();
    //parse terafly-data folder
    QFileInfoList sorted_terafly_dirs= sort_terafly_res_dirs(inimg_file);
    if(vol_res>sorted_terafly_dirs.size() -1 || vol_res < 0) {cerr<<"Input para error of inputing volume res-index"<<endl; return tovols;}
    if(to_res>sorted_terafly_dirs.size() -1 || to_res < 0) {cerr<<"Input para error of requesting volumes' res-index"<<endl; return tovols;}

    QStringList wholeSize=sorted_terafly_dirs.at(0).fileName().mid(4).split("x");
    V3DLONG wRES[3];
    wRES[1]=wholeSize.at(0).toLong();
    wRES[0]=wholeSize.at(1).toLong();
    wRES[2]=wholeSize.at(2).split(")").at(0).toLong();
//    cerr<<"image size: "<<wRES[0]<<","<<wRES[1]<<","<<wRES[2]<<endl;

    //get volume virtual range
    /*
     * this step is for mapping filename with volume coordinates
     * The filename of TeraFly volume is volume's coordinates coded in 6 numbers.
    */
    int default_terafly_res_code=6;
    int res_code_num=MAX(QString::number(wRES[0]).length(),QString::number(wRES[1]).length());
    res_code_num=MAX(QString::number(wRES[2]).length(),res_code_num);
    QStringList invol_cords=QFileInfo(QString::fromStdString(invol_file)).baseName().split("_");
    V3DLONG volCs[3],volCe[3];
    volCs[0] =invol_cords.at(1).toLong();
    volCs[1]= invol_cords.at(0).toLong();
    volCs[2]= invol_cords.at(2).toLong();
    if(default_terafly_res_code>res_code_num)
        for(int s=0;s<3;s++)
            volCs[s]/=(10*(default_terafly_res_code-res_code_num));
    if(volCs[0]<0||volCs[0]>wRES[0]-1||
            volCs[1]<0||volCs[1]>wRES[1]-1||
            volCs[2]<0||volCs[2]>wRES[2]-1) {
        cerr<<"Error of input volumes' top-left coordinates"<<endl;
        return tovols;
    }
//    cerr<<"vol start: "<<volCs[0]<<","<<volCs[1]<<","<<volCs[2]<<endl;

    /* get volume image size    */
    unsigned char * inimg1d = 0;
    V3DLONG  in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)invol_file.c_str(), inimg1d, in_sz, datatype)) {
        cerr<<"load image "<<invol_file<<" error!"<<endl;
        return tovols;
    }
//    V3DLONG volX = in_sz[0]; V3DLONG volY = in_sz[1]; V3DLONG volZ = in_sz[2];
//    cout<<"volume size: "<<volX<<","<<volY<<","<<volZ<<endl;
    delete [] inimg1d; inimg1d = 0;
    for(int s=0;s<3;s++)
        volCe[s]=volCs[s]+in_sz[s]*pow(2,vol_res);
    if(volCe[0]<0||volCe[0]>wRES[0]-1||
            volCe[1]<0||volCe[1]>wRES[1]-1||
            volCe[2]<0||volCe[2]>wRES[2]-1) {
        cerr<<"Error of input volumes' bottom-right coordinates"<<endl;
        return tovols;
    }
//    cerr<<"vol end: "<<volCe[0]<<","<<volCe[1]<<","<<volCe[2]<<endl;
    //get related volumes of requesting resolution
    QFileInfo toRes_dir=sorted_terafly_dirs.at(to_res);
    QList < QList<V3DLONG> > to_vol_Lists;
    //for wRESi in [wRESX,wRESY,wRESZ]
    for(int s=0;s<3;s++)
    {
        // initialization
        V3DLONG wRESi=wRES[s];
        V3DLONG volis=volCs[s]; V3DLONG volie=volCe[s];
        QList<V3DLONG> to_vol_list; to_vol_list.clear();
        //get vol sizes
        V3DLONG volRes_dir_size = ceil(wRESi/(256*pow(2,to_res)));
//        cerr<<"RES:"<<wRESi<<",volRes_dir_size: "<<volRes_dir_size<<endl;
        V3DLONG vol2_range=256*pow(2,to_res);
        V3DLONG vol1_num=volRes_dir_size;
        for(V3DLONG i=pow(2,to_res)*256-pow(2,to_res);i>=pow(2,to_res-1)*256;i-=pow(2,to_res)){
            V3DLONG bro_nums=(wRESi-volRes_dir_size*i)/pow(2,to_res);
//            if(s==2)
//                cerr<<"vol1_num="<<bro_nums<<",i="<<i<<endl;
            if(bro_nums>0){
                vol2_range=i;
                vol1_num=bro_nums;
                if(bro_nums>volRes_dir_size)
                {
//                    cout<<"one size"<<endl;
                    vol1_num=volRes_dir_size;
                }
                break;
            }
        }
        V3DLONG vol1_range=vol2_range+pow(2,to_res);
        V3DLONG vol2_num=volRes_dir_size-vol1_num;
//        cerr<<"vol1: "<<vol1_range<<", number="<<vol1_num<<endl;
//        cerr<<"vol2: "<<vol2_range<<", number="<<vol2_num<<endl;
//        break;
        //get start image name
        if(volis/vol1_range<vol1_num){
            // all in vol1 range
            if(volie/vol1_range<=vol1_num)
                for(V3DLONG i=V3DLONG(volis/vol1_range);i<=V3DLONG(volie/vol1_range);i++)
                    to_vol_list.append(i*vol1_range);
            else{
                for(V3DLONG i=V3DLONG(volis/vol1_range);i<vol1_num;i++)
                    to_vol_list.append(i*vol1_range);
                for(V3DLONG i=0;(i<=V3DLONG((volie-vol1_num*vol1_range)/vol2_range))&&(i<vol2_num);i++)
                    to_vol_list.append(i*vol2_range+vol1_num*vol1_range);
            }
        }
        else{
            //all in vol2 range
            V3DLONG vol1_Len=vol1_num*vol1_range;
//            cerr<<"vol1_Len="<<vol1_Len<<", start ="<<V3DLONG((volis-vol1_Len)/vol2_range)<<endl;
            for(V3DLONG i=V3DLONG((volis-vol1_Len)/vol2_range);(i<=V3DLONG((volie-vol1_Len)/vol2_range))&&(i<vol2_num);i++)
                to_vol_list.append(i*vol2_range+vol1_Len);
        }
//        for(int l=0;l<to_vol_list.size();l++)
//            cout<<to_vol_list.at(l)<<",";
        to_vol_Lists.append(to_vol_list);
//        break;
    }

    /* map to real volume file
     * RES(YxXxZ)/Y0/X0/Y0_X0_Z0.tif
    */
//    cout<<"mapping"<<endl;
    QList<V3DLONG> to_vol_Xlist=to_vol_Lists.at(0);
    QList<V3DLONG> to_vol_Ylist=to_vol_Lists.at(1);
    QList<V3DLONG> to_vol_Zlist=to_vol_Lists.at(2);
    for(V3DLONG iy=0;iy<to_vol_Ylist.size();iy++)
    {
        QString YD=QString::number(10*to_vol_Ylist.at(iy));
        YD="000000"+YD; YD=YD.right(6);
        for(V3DLONG ix=0;ix<to_vol_Xlist.size();ix++)
        {
            QString XD=QString::number(10*to_vol_Xlist.at(ix));
            XD="000000"+XD; XD=XD.right(6);
            for(V3DLONG iz=0;iz<to_vol_Zlist.size();iz++)
            {
                QString ZD=QString::number(10*to_vol_Zlist.at(iz));
                ZD="000000"+ZD; ZD=ZD.right(6);
                //file path
                QString tovol = inimg_file + "/" + toRes_dir.fileName() +"/" +YD +"/" +YD+"_"+XD +"/" +YD +"_" +XD +"_" +ZD + ".tif";
                QFileInfo tovol_file(tovol);
                if(tovol_file.isFile())
                    tovols.append(tovol);
                else
                    cerr<<"indexed file="<<tovol.toStdString()<<endl;
            }
        }
    }
//    cout<<"Related volumes="<<tovols.size()<<endl;
    return tovols;
}

// data type conversion
template <class Tpre, class Tpost>
void converting(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt)
{
     if (!pre1d ||!pPost || imsz<=0 )
     {
          v3d_msg("Invalid parameters to converting().", 0);
          return;
     }

    Tpre *pPre = (Tpre *)pre1d;

    if(v3d_dt == V3D_UINT8)
    {
        Tpre max_v=0, min_v = 255;

        for(V3DLONG i=0; i<imsz; i++)
        {
            if(max_v<pPre[i]) max_v = pPre[i];
            if(min_v>pPre[i]) min_v = pPre[i];
        }
        max_v -= min_v;

        if(max_v>0)
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) 255*(double)(pPre[i] - min_v)/max_v;
        }
        else
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) pPre[i];
        }
    }
    else if(v3d_dt == V3D_UINT16)
    {
        Tpre max_v=0, min_v = 65535;

        for(V3DLONG i=0; i<imsz; i++)
        {
            if(max_v<pPre[i]) max_v = pPre[i];
            if(min_v>pPre[i]) min_v = pPre[i];
        }
        max_v -= min_v;

        if(max_v>0)
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) 65535*(double)(pPre[i] - min_v)/max_v;
        }
        else
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) pPre[i];
        }

    }
    else if(v3d_dt == V3D_FLOAT32)
    {
        for(V3DLONG i=0; i<imsz; i++)
            pPost[i] = (Tpost) pPre[i];
    }

}
bool img_2_8bit(unsigned char * & src, unsigned char * & data1d, V3DLONG * in_sz, int src_dt)
{
    V3DLONG	sz_sub = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
    try
    {
        data1d = new unsigned char [sz_sub];
    }
    catch(...)
    {
        printf("Error allocating memory. \n");
        return false;
    }

    if(src_dt == 1)
    {
        converting<unsigned char, unsigned char>((unsigned char *)src, data1d, sz_sub, V3D_UINT8);
    }
    else if(src_dt == 2)
    {
        converting<unsigned short, unsigned char>((unsigned short *)src, data1d, sz_sub, V3D_UINT8);
    }
    else if(src_dt == 4)
    {
        converting<float, unsigned char>((float *)src, data1d, sz_sub, V3D_UINT8);
    }
    return true;
}
