/* getlevel0data_func.cpp
 * a plugin to get level 0 data
 * 10/10/2018 : by Yang Yu
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

QueryAndCopy::QueryAndCopy(string swcfile, string inputdir, string outputdir, float ratio)
{
    // inputdir: "xxxx/RES(123x345x456)"
    // outputdir: "yyyy/RES(123x345x456)"

    // load input .swc file and mdata.bin


//    QString str = "a b c d e f g h i j k l m n o p q r s t u v w x y z a b c d e f g h i j k l m n o p q r s t u v w x y z a b c d e f g h i j k l m n o p q r s t u v w x y z";
//    // cout<<"str: "<<str.c_str()<<endl;

//    cout<<str.toUtf8().constData()<<endl;
    QString inputano=QString::fromStdString(swcfile);
    QFileInfo inputinfo(inputano);
    if(inputinfo.suffix()=="ano")
    {
         cout<<"get preimage from ano file"<<endl;
        P_ObjectFileType linker_object;

        if (!loadAnoFile(inputano,linker_object))
        {
            fprintf(stderr,"Error in reading the linker file.\n");
            return ;
        }
        QStringList nameList = linker_object.swc_file_list;
        int neuronNum = nameList.size();
        for (V3DLONG j=0;j<neuronNum;j++)
        {
            string tmpswcfile=nameList.at(j).toStdString();
            readSWC(tmpswcfile, ratio);
        }
    }
    else
    {
        cout<<"get preimage from swc file"<<endl;
        readSWC(swcfile, ratio);
    }

    //
    readMetaData(inputdir);


    // find hit & neighbor blocks

    //
    long n = pc.size();

    cout<<" ... ... consider "<<n<<" nodes in "<<tree.size()<<" blocks"<<endl;

    // test
//    map<long, Block>::iterator it = tree.begin();
//    while(it != tree.end())
//    {
//        cout<<(it++)->first<<", ";
//    }
//    cout<<endl;


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

    // based on hits, update the layer
//    unsigned short nrows = 0;
//    map<string, YXFolder>::iterator iter = layer.yxfolders.begin();
//    while(iter != layer.yxfolders.end())
//    {
//        //
//        YXFolder yxfolder = (iter++)->second;

//        unsigned int ncubes = 0;

//        //
//        if(yxfolder.toBeCopied==false)
//            continue;

//        //
//        nrows++;

//        //
//        map<int, Cube>::iterator it = yxfolder.cubes.begin();
//        while(it != yxfolder.cubes.end())
//        {
//            //
//            Cube cube = (it++)->second;

//            if(cube.toBeCopied==false)
//                continue;

//            ncubes++;
//        }

//        layer.yxfolders[yxfolder.dirName].ncubes = ncubes;
//    }
//    layer.cols = 1;
//    layer.rows = nrows;


    //
    map<string, YXFolder>::iterator iter = layer.yxfolders.begin();
    while(iter != layer.yxfolders.end())
    {
        YXFolder yxfolder = (iter++)->second;
        // cout<<"yxfolder.ncubes "<<yxfolder.ncubes<<endl;
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

        // cout<<"layer.yxfolders.size "<<layer.yxfolders.size()<<endl;

        //
        int count = 0;
        int nyxfolders = layer.yxfolders.size();
        map<string, YXFolder>::iterator iter = layer.yxfolders.begin();
        while(iter != layer.yxfolders.end())
        {
            cout<<"testing count "<<count<<" of "<<nyxfolders<<endl;

            //
            if(count++ >= nyxfolders)
            {
                iter++;
                continue;
            }

            // cout<<"count "<<count<<" >= "<<nyxfolders<<endl;

            //
            YXFolder yxfolder = (iter++)->second;

            // cout<<"check ncubes ... "<<yxfolder.ncubes<<" at "<<count<<endl;

            // cout<<"check "<<layer.yxfolders[yxfolder.dirName].toBeCopied<<endl;

            if(yxfolder.toBeCopied==false)
            {
                // continue;

                // trick: create a "zeroblocks" folder holds blocks with zeros
                // createDir(outputdir, dirName);

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

                    // cout<<"countCube "<<countCube<<" >= "<<ncubes<<endl;

                    //
                    Cube cube = (it++)->second;

//                    string cubeName = "zeroblock_" + to_string(yxfolder.height) + "_" + to_string(yxfolder.width) + "_" + to_string(cube.depth) + ".tif";
                    string cubeName = "NULL.tif";
                    unsigned short lengthCubeName = cubeName.length() + 1; // consider the end is '\0'

                    // cout<<"write/link ... "<<dirName<<" / "<<cubeName<<" "<<lengthCubeName<<endl;

//                    long cubeIndex = cube.depth*sx*sy + yxfolder.height*sx + yxfolder.width;
//                    string dstFilePath = outputdir + "/" + dirName + "/" + cubeName;

//                    if(zeroblocks.find(cubeIndex) == zeroblocks.end())
//                    {
//                        char *errorMsg = initTiff3DFile(const_cast<char*>(dstFilePath.c_str()), int(yxfolder.width), int(yxfolder.height), int(cube.depth), 1, bytesPerVoxel);
//                        // cout<<"create a zero block: "<<errorMsg<<endl;

//                        zeroblocks.insert(make_pair(cubeIndex, cubeName));
//                    }

                    //
                    fwrite(&(lengthCubeName), sizeof(unsigned short), 1, file);
                    fwrite(const_cast<char *>(cubeName.c_str()), lengthCubeName, 1, file);
                    fwrite(&(cube.depth), sizeof(unsigned int), 1, file);
                    fwrite(&(cube.offset_D), sizeof(int), 1, file);
                }
            }
            else
            {
                // cout<<"write ... "<<yxfolder.dirName<<endl;

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

                    // cout<<"countCube "<<countCube<<" >= "<<ncubes<<endl;

                    //
                    Cube cube = (it++)->second;

                    if(cube.toBeCopied==false)
                    {
                        //continue;

//                        string cubeName = "zeroblock_" + to_string(yxfolder.height) + "_" + to_string(yxfolder.width) + "_" + to_string(cube.depth) + ".tif";
//                        unsigned short lengthCubeName = cubeName.length();
//                        long cubeIndex = cube.depth*sx*sy + yxfolder.height*sx + yxfolder.width;
//                        string dstFilePath = outputdir + "/" + dirName + "/" + cubeName;

//                        if(zeroblocks.find(cubeIndex) == zeroblocks.end())
//                        {
//                            char *errorMsg = initTiff3DFile(const_cast<char*>(dstFilePath.c_str()), int(yxfolder.width), int(yxfolder.height), int(cube.depth), 1, bytesPerVoxel);
//                            // cout<<"create a zero block "<<errorMsg<<endl;
//                        }

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
                        //
//                        string srcFilePath = inputdir + "/" + yxfolder.dirName + "/" + cube.fileName;
//                        string dstFilePath = outputdir + "/" + yxfolder.dirName + "/" + cube.fileName;

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

int QueryAndCopy::copyblock(QString srcFile, QString dstFile)
{


//    qDebug()<<"srcFile "<<srcFile;
//    qDebug()<<"dstFile "<<dstFile;


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


////    cout<<"... ... test ... ..."<<endl;
//    // QString qstr = QString(QString("cp \'")).append(QString::fromStdString(srcFile)).append(QString("\' \'")).append(QString::fromStdString(dstFile)).append(QString("\'"));

////    cout<<str<<endl;

////    QString qstr = QString::fromAscii(str);

//    QString qstr = QString("cp \'").append(srcFile).append(QString("\' \'")).append(dstFile).append(QString("\'"));


//    qDebug()<<"qstr ... "<<qstr;


//    cout<<"str "<<qstr.toUtf8().constData()<<endl;


//    const char *cstr = qstr.toUtf8().constData();

//    cout<<"cstr "<<cstr<<endl;


//    cout<<" length "<<str.length()<<endl;

//    char * cstr = new char [str.length()+1];
//    std::strcpy (cstr, str.c_str());


//    cout<<"... cstr ... "<<cstr<<endl;


//    for(int i=0; i<str.length(); i++)
//        cout<<cstr[i];
//    cout<<endl;


//    char * p = std::strtok (cstr," ");
//    while (p!=0)
//    {
//        std::cout << p << endl;
//        p = std::strtok(NULL," ");
//    }
//    cout<<endl;

//    delete[] cstr;




//    char cpcmd[1024];

//    // string command = "cp \'" + srcFile + "\' \'" + dstFile +"\'\0";

//    // cout<<command.c_str()<<endl;

//    int i;

//    char const *src = &srcFile[0];
//    char const *dst = &dstFile[0];

//    strcpy(cpcmd, "cp \'");

//    cout<<"copy ... src name ... \n";
//    for(i=4; i-4<srcFile.length(); i++)
//    {
//        cpcmd[i] = src[i-4];
//        printf("%c", src[i-4]);
//    }
//    cout<<endl;

//    // strcat(command, src);
//    // strcat(command, "\' \'");

//    cpcmd[i] = '\'';
//    cpcmd[++i] = ' ';
//    cpcmd[++i] = '\'';

//    int j = ++i;

//    cout<<"copy ... dst name ... \n";
//    for(i=j; i-j<dstFile.length(); i++)
//    {
//        cpcmd[i] = dst[i-j];
//        printf("%c", dst[i-j]);
//    }
//    cout<<endl;

//    // strcat(command, dst);
//    // strcat(command, "\'\0");

//    cpcmd[i] = '\'';
//    cpcmd[++i] = '\0';


//    cout<<"command ... \n";
//    for(j=0; j<i; j++)
//        printf("%c", cpcmd[j]);
//    cout<<endl;



//    const char *a = cpcmd;

//    cout<<"a ... "<<a<<endl;

//    string str(a, j);

////    cout<<"... test ... "<<str<<" "<<str.length()<<" comparing to "<<j<<endl;
//    std::cout << "capacity: " << str.capacity() << "\n";
////    std::cout << "max_size: " << str.max_size() << "\n";

//    cout<<"str ... "<<str.c_str()<<endl;

//    // cout<<"command "<<command<<endl;

//    system(cpcmd);


//    int read_fd;
//    int write_fd;
//    struct stat stat_buf;
//    off_t offset = 0;

//    /* Open the input file. */
//    read_fd = open (srcFile.c_str(), O_RDONLY);
//    /* Stat the input file to obtain its size. */
//    fstat (read_fd, &stat_buf);
//    /* Open the output file for writing, with the same permissions as the
//      source file. */
//    write_fd = open (dstFile.c_str(), O_WRONLY | O_CREAT, stat_buf.st_mode);

//    cout<<"write fd "<<write_fd<<endl;

//    /* Blast the bytes from one file to the other. */
//    ssize_t result = sendfile (write_fd, read_fd, &offset, stat_buf.st_size);

//    cout<<"sendfile "<<result<<endl;

//    /* Close up. */
//    close (read_fd);
//    close (write_fd);



//    //
//    std::ifstream  src(srcFile.c_str(), std::ios_base::in | std::ios_base::binary);

////    if(src.is_open())
////    {
////        std::ofstream  dst(dstFile.c_str(), std::ios_base::out | std::ios_base::binary);

////        if(dst.is_open())
////        {
////            dst << src.rdbuf();

////            if(dst.bad())
////            {
////                cout<<"Error writing file "<<dstFile<<endl;
////            }
////        }
////        else
////        {
////            cout<<"Error opening file "<<dstFile<<endl;
////        }

////        dst.close();
////    }
////    else
////    {
////        cout<<"Error opening file "<<srcFile<<endl;
////    }

//    // std::ofstream  dst(dstFile.c_str(), std::ios_base::out | std::ios_base::binary);
//    std::ofstream dst;
//    dst.open(dstFile, std::ios::binary);

//    if(!dst)
//    {
//        cout<<"error in opening dst file"<<endl;
//    }

//    if(dst.is_open())
//    {
//        dst << src.rdbuf();

//        if(dst.bad())
//        {
//            cout<<"Error writing file "<<dstFile<<endl;
//        }
//    }
//    else
//    {
//        cout<<"Error opening file "<<dstFile<<endl;
//    }

//    // file size
//    src.seekg(0, ios::end);
//    ifstream::pos_type size = src.tellg();
//    src.seekg(0);
//    // allocate memory for buffer
//    char* buffer = new char[size];

//    // copy file
//    src.read(buffer, size);
//    dst.write(buffer, size);

//    // clean up
//    delete[] buffer;
//    dst.close();

////    const static int BUF_SIZE = 4096;

////    char buf[BUF_SIZE];

////    do {
////        src.read(&buf[0], BUF_SIZE);      // Read at most n bytes into
////        dst.write(&buf[0], src.gcount()); // buf, then write the buf to
////    } while (src.gcount() > 0);          // the output.


//    src.close();

    //
    return 0;
}

int QueryAndCopy::readSWC(string filename, float ratio)
{
    //
    NeuronTree nt = readSWC_file(QString(filename.c_str()));

//    float minX = 1000, minY = 1000, minZ = 1000;

    if(ratio>1)
    {
        for (long i =0; i< nt.listNeuron.size(); i++)
        {
            Point p(nt.listNeuron[i].x/ratio, nt.listNeuron[i].y/ratio, nt.listNeuron[i].z/ratio);
            pc.push_back(p);

            // cout<<"node ... "<<nt.listNeuron[i].x<<" "<<nt.listNeuron[i].y<<" "<<nt.listNeuron[i].z<<" : "<<p.x<<" "<<p.y<<" "<<p.z<<endl;

//            if(minX>p.x)
//                minX = p.x;

//            if(minY>p.y)
//                minY = p.y;

//            if(minZ>p.z)
//                minZ = p.z;
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

    // cout<<"min ... ... "<<minX<<" "<<minY<<" "<<minZ<<endl;

    //
    return 0;
}

int QueryAndCopy::readMetaData(string filename, bool mDataDebug)
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
    //cout<<"query "<<x<<" "<<y<<" "<<z<<endl;
    //cout<<"size "<<sx<<" "<<sy<<" "<<sz<<endl;
    //cout<<"cube size "<<cubex<<" "<<cubey<<" "<<cubez<<endl;
    //cout<<"search in "<<tree.size()<<" blocks"<<endl;

    // find hit block and 6 neighbors
    if(tree.size()>0)
    {
//        long nx = long(x)/cubex;
//        long ny = long(y)/cubey;
//        long nz = long(z)/cubez;

        // hit block

//        long lx = nx*cubex;
//        long ly = ny*cubey;
//        long lz = nz*cubez;

        long lx = findOffset(xoff, long(x));
        long ly = findOffset(yoff, long(y));
        long lz = findOffset(zoff, long(z));

        long olx = lx;
        long oly = ly;
//        long olz = lz;

        long index = lz*sx*sy + ly*sx + lx;

        //cout<<"node's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;


        // test
//        map<long, Block>::iterator it = tree.begin();
//        while(it != tree.end())
//        {
//            cout<<(it++)->first<<", ";
//        }
//        cout<<endl;

        //
        label(index);

        // 6 neighbors

        // x-
//        if(nx-1>0)
//        {
//            lx = (nx - 1) * cubex;
//            index = lz*sx*sy + ly*sx + lx;

//            label(index);
//        }

        lx = findOffset(xoff, long(x-cubex));
        index = lz*sx*sy + ly*sx + lx;
        label(index);

        //cout<<"node's x- neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        // x+
//        lx = (nx + 1) * cubex;

        lx = findOffset(xoff, long(x+cubex));
        index = lz*sx*sy + ly*sx + lx;
        label(index);

        //cout<<"node's x+ neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        lx = olx;

        // y-
//        lx = nx*cubex;

//        if(ny-1>0)
//        {
//            ly = (ny - 1)*cubey;

//            index = lz*sx*sy + ly*sx + lx;

//            label(index);
//        }

        ly = findOffset(yoff, long(y-cubey));
        index = lz*sx*sy + ly*sx + lx;
        label(index);

        //cout<<"node's y- neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        // y+
//        ly = (ny + 1)*cubey;

        ly = findOffset(yoff, long(y+cubey));
        index = lz*sx*sy + ly*sx + lx;
        label(index);

        //cout<<"node's y+ neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        ly = oly;

        // z-
//        ly = ny*cubey;

//        if(nz-1>0)
//        {
//            lz = (nz - 1)*cubez;

//            index = lz*sx*sy + ly*sx + lx;

//            label(index);
//        }

        lz = findOffset(yoff, long(z-cubez));
        index = lz*sx*sy + ly*sx + lx;
        label(index);

        //cout<<"node's z- neighbor's index "<<lx<<" "<<ly<<" "<<lz<<" "<<index<<endl;

        // z+
//        lz = (nz + 1)*cubez;

        lz = findOffset(yoff, long(z+cubez));
        index = lz*sx*sy + ly*sx + lx;
        label(index);

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

long QueryAndCopy::findClosest(OffsetType offsets, long idx)
{
    long n = offsets.size();
    long thresh = 5;

    //
    if(n<1)
    {
        cout<<"Invalid offsets/index"<<endl;
        return -1;
    }
    else
    {
        // test
        cout<<"... offset ... ";
        for(int i=0; i<n; i++)
        {
            cout<<offsets[i]<<" ";
        }
        cout<<endl;
    }

    //
    if(idx<0)
    {
        idx = 0;
    }

    //
    long mindist = abs(idx - offsets[0]);

    long offset = offsets[0];

    if(mindist<thresh)
        return offset;

    //
    for(long i=1; i<offsets.size(); i++)
    {
        long dist = abs(idx - offsets[i]);

        if(dist<mindist)
        {
            mindist = dist;
            offset = offsets[i];

            if(mindist<thresh)
                return offset;
        }
    }

    return offset;
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

/*
// save a 3D chunk tif image with all zeros
char *initTiff3DFile(char *filename, int sz0, int sz1, int sz2, int sz3, int datatype)
{
    //
    uint32 XSIZE  = sz0;
    uint32 YSIZE  = sz1;
    uint16 Npages = sz2;
    uint16 spp    = sz3;

    uint16 bpp=8 * datatype;

    int rowsPerStrip = -1;

    int check;

    if ( sz3 == 1 )
        spp = sz3;
    else if ( sz3 < 4 )
        spp = 3;
    else
        return ((char *) "More than 3 channels in Tiff files.");

    //
    long szSlice = (long)XSIZE * (long)YSIZE * (long)spp * (long)datatype;
    unsigned char *fakeData=NULL;
    try
    {
        fakeData = new unsigned char[ szSlice ];
        memset(fakeData,0,szSlice);
    }
    catch(...)
    {
        return ((char *)"Fail to alloc memory\n");
    }

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    TIFF *output;

    long expectedSize = ((long) sz0) * ((long) sz1) * ((long) sz2) * ((long) sz3) * ((long) datatype);
    long fourGBSize = 4;
    fourGBSize *= 1024;
    fourGBSize *= 1024;
    fourGBSize *= 1024;

    if ( expectedSize > (fourGBSize) )
    {
        if ( (rowsPerStrip == -1 && (((long) sz0) * ((long) sz1)) > (fourGBSize)) || ((rowsPerStrip * ((long) sz0)) > (fourGBSize)) )
            return ((char *) "Too many rows per strip for this image width.");
        else
            output = TIFFOpen(filename,"w8");
    }
    else
    {
        output = TIFFOpen(filename,"w");
    }

    if (!output)
    {
        return ((char *) "Cannot open the file.");
    }

    //
    if ( rowsPerStrip == -1 )
    {
        for(long slice=0; slice<Npages; slice++)
        {
            //
            TIFFSetDirectory(output, slice);

            //
            check = TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
            if (!check) {
                return ((char *) "Cannot set the image width.");
            }

            check = TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
            if (!check) {
                return ((char *) "Cannot set the image height.");
            }

            check = TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp);
            if (!check) {
                return ((char *) "Cannot set the image bit per sample.");
            }

            check = TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
            if (!check) {
                return ((char *) "Cannot set the image sample per pixel.");
            }

            check = TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? YSIZE : (uint32)rowsPerStrip);
            if (!check) {
                return ((char *) "Cannot set the image rows per strip.");
            }

            check = TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            if (!check) {
                return ((char *) "Cannot set the image orientation.");
            }

            check = TIFFSetField(output, TIFFTAG_COMPRESSION, COMPPRESSION_METHOD);
            if (!check) {
                return ((char *) "Cannot set the compression tag.");
            }

            check = TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
            if (!check) {
                return ((char *) "Cannot set the planarconfig tag.");
            }

            if ( spp == 1 )
                check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
            else // spp == 3
                check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
            if (!check) {
                return ((char *) "Cannot set the photometric tag.");
            }

            // We are writing single page of the multipage file
            check = TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
            if (!check) {
                return ((char *) "Cannot set the subfiletype tag.");
            }

            check = TIFFSetField(output, TIFFTAG_PAGENUMBER, slice, Npages);
            if (!check) {
                return ((char *) "Cannot set the page number.");
            }

            if(!TIFFWriteEncodedStrip(output, 0, fakeData, szSlice))
            {
                return ((char *) "Cannot write encoded strip to file.");
            }

            //
            if (!TIFFWriteDirectory(output))
            {
                return ((char *) "Cannot write a new directory.");
            }
        }
    }
    else
    {
        // TODO: modify codes to save 3D image stack later

        //
        // save one slice
        //
        int check,StripsPerImage,LastStripSize;
        uint32 rps = (uint32)rowsPerStrip;
        unsigned char *buf = fakeData;

        StripsPerImage =  (YSIZE + rps - 1) / rps;
        LastStripSize = YSIZE % rps;
        if (LastStripSize==0)
            LastStripSize=rps;

        for (int i=0; i < StripsPerImage-1; i++){
            check = TIFFWriteEncodedStrip(output, i, buf, spp * rps * XSIZE * (bpp/8));
            if (!check) {
                return ((char *) "Cannot write encoded strip to file.");
            }
            buf = buf + spp * rps * XSIZE * (bpp/8);
        }

        check = TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * XSIZE * (bpp/8));
        if (!check) {
            return ((char *) "Cannot write encoded strip to file.");
        }
        buf = buf + spp * LastStripSize * XSIZE * (bpp/8);
    }

    //
    if(fakeData)
    {
        delete[] fakeData;
    }

    //
    TIFFClose(output);

    //
    return (char *) 0;
}
*/

//
bool getPreimage(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1 || output.size() != 1)
    {
        cout<<"vaa3d -x getlevel0data -f getPreimage_fromAno -i inputdir input.ano -o outputdir -p scale<0/1/2/3/4/5/...>"<<endl;
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
            cout<<"get preimage data at the scale: "<<scale<<endl;
        }
        else
        {
            cout<<"Invalid input"<<endl;
            cout<<"vaa3d -x getlevel0data -f getPreimage_fromAno -i inputdir input.ano -o outputdir -p scale<0/1/2/3/4/5/...>"<<endl;
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
    // QueryAndCopy qc(outputdir.toStdString()); // test mdata.bin writing
    QueryAndCopy qc(swcfile.toStdString(), inputdir.toStdString(), outputdir.toStdString(), ratio);

    //
    return true;
}
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
    // QueryAndCopy qc(outputdir.toStdString()); // test mdata.bin writing
    QueryAndCopy qc(swcfile.toStdString(), inputdir.toStdString(), outputdir.toStdString(), ratio);

    //
    return true;
}
