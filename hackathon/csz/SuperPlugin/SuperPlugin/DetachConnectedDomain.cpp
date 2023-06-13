#include "DetachConnectedDomain.h"

std::vector<std::vector<std::vector<int>>> reshape(unsigned char* data, long width, long height, long depth) {
    std::vector<std::vector<std::vector<int>>> result;

    for (int z = 0; z < depth; ++z) {
        std::vector<std::vector<int>> slice;
        for (int y = 0; y < height; ++y) {
            std::vector<int> row;
            for (int x = 0; x < width; ++x) {
                int index = z * (width * height) + y * width + x;
                int value = static_cast<int>(data[index]);
                row.push_back(value);
            }
            slice.push_back(row);
        }
        result.push_back(slice);
    }

    return result;
}

unsigned char* convertVectorToUnsignedChar(const std::vector<std::vector<std::vector<int>>>& data) {
    int width = data[0][0].size();
    int height = data[0].size();
    int depth = data.size();
    unsigned char* result = new unsigned char[width * height * depth];
    for(int i=0;i<width * height * depth;i++){
        result[i]=0;
    }
    for (int z = 0; z < depth; ++z) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = z * (width * height) + y * width + x;
                result[index] = static_cast<unsigned char>(data[z][y][x]);
            }
        }
    }

    return result;
}

// 逆时针旋转90度
std::vector<std::vector<std::vector<int>>> rotateMatrix(const std::vector<std::vector<std::vector<int>>>& matrix) {
    int x = matrix.size();
    int y = matrix[0].size();
    int z = matrix[0][0].size();

    std::vector<std::vector<std::vector<int>>> rotatedMatrix(z, std::vector<std::vector<int>>(y, std::vector<int>(x, 0)));

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            for (int k = 0; k < z; ++k) {
                rotatedMatrix[z - k - 1][j][i] = matrix[i][j][k];
            }
        }
    }

    return rotatedMatrix;
}

// 沿着 z 轴翻转
std::vector<std::vector<std::vector<int>>> flipMatrixZ(const std::vector<std::vector<std::vector<int>>>& matrix) {
    int x = matrix.size();
    int y = matrix[0].size();
    int z = matrix[0][0].size();

    std::vector<std::vector<std::vector<int>>> flippedMatrix(x, std::vector<std::vector<int>>(y, std::vector<int>(z, 0)));

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            for (int k = 0; k < z; ++k) {
                flippedMatrix[i][j][z - k - 1] = matrix[i][j][k];
            }
        }
    }

    return flippedMatrix;
}

std::vector<std::vector<std::vector<int>>> flipMatrixX(const std::vector<std::vector<std::vector<int>>>& matrix) {
    int x = matrix.size();
    int y = matrix[0].size();
    int z = matrix[0][0].size();

    std::vector<std::vector<std::vector<int>>> flippedMatrix(x, std::vector<std::vector<int>>(y, std::vector<int>(z, 0)));

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            for (int k = 0; k < z; ++k) {
                flippedMatrix[x - i - 1][j][k] = matrix[i][j][k];
            }
        }
    }

    return flippedMatrix;
}

std::vector<std::vector<std::vector<int>>> flipMatrixY(const std::vector<std::vector<std::vector<int>>>& matrix) {
    int x = matrix.size();
    int y = matrix[0].size();
    int z = matrix[0][0].size();

    std::vector<std::vector<std::vector<int>>> flippedMatrix(x, std::vector<std::vector<int>>(y, std::vector<int>(z, 0)));

    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            for (int k = 0; k < z; ++k) {
                flippedMatrix[i][y - j - 1][k] = matrix[i][j][k];
            }
        }
    }

    return flippedMatrix;
}


QStringList Solution::detach(QString name,V3DPluginCallback2 &callback)
{

    char *inimg_file=qstring2char(name);

    Image4DSimple *img= callback.loadImage(inimg_file);
    long x=img->getXDim();
    long y=img->getYDim();
    long z=img->getZDim();
    unsigned char * data=img->getRawData();
    qDebug()<<"load img succeed";
    std::vector<std::vector<std::vector<int>>> volume=reshape(data,x,y,z);
    qDebug()<<volume.size()<<volume[0].size()<<volume[0][0].size();
    qDebug()<<"reshape img succeed";

    vector<vector<Point>> res = bwlabel(volume);
    qDebug()<<"detach img succeed";
    qDebug()<<res.size();
    QStringList namelist;
    for(int i=0;i<res.size();i++){
        vector<vector<vector<int>>> imgn(x, vector<vector<int>>(y, vector<int>(z, 0)));
        if(res[i].size()<=125){
            continue;
        }

        qDebug()<<res[i].size();

        for(int j=0;j<res[i].size();j++){
            imgn[res[i][j].z][res[i][j].y][res[i][j].x]=1;
        }
        vector<vector<vector<int>>> imgrot90=rotateMatrix(imgn);
        vector<vector<vector<int>>> imgflip=flipMatrixX(imgrot90);
        unsigned char* imgdata=convertVectorToUnsignedChar(imgflip);
        for(long i=0;i<img->getTotalBytes();i++){
            imgdata[i]*=img->getRawData()[i];
        }
        Image4DSimple *newimg=new Image4DSimple;
        newimg->setData(imgdata,x,y,z,1,V3D_UINT8);
        QString outimg=name+"_"+QString::number(i)+".tiff";
        namelist.append(outimg);
        char* outname=qstring2char(outimg);
        callback.saveImage(newimg,outname);
        delete newimg;
    }
    return namelist;
}

QString Solution::seg(QString name, V3DPluginCallback2 &callback)
{
    qDebug()<<"start";
    char * inimg_file = qstring2char(name);

    Image4DSimple *img= callback.loadImage(inimg_file);

    double imgAve, imgStd;
    bool ok=mean_and_std(img->getRawData(),img->getTotalBytes(),imgAve, imgStd);
    if(imgStd<2)
        return "";
    double value=imgAve+0.5*imgStd;
    for(long i=0;i<img->getTotalBytes();i++){
        if(img->getRawData()[i]<value)
            img->getRawData()[i]=0;
    }


    long x=img->getXDim();
    long y=img->getYDim();
    long z=img->getZDim();
    unsigned char * data=img->getRawData();
    qDebug()<<"load img succeed";
    std::vector<std::vector<std::vector<int>>> volume=reshape(data,x,y,z);
    qDebug()<<volume.size()<<volume[0].size()<<volume[0][0].size();
    qDebug()<<"reshape img succeed";

    vector<vector<Point>> res = bwlabel(volume);
    qDebug()<<"detach img succeed";
    qDebug()<<res.size();
    vector<vector<vector<int>>> imgn(x, vector<vector<int>>(y, vector<int>(z, 0)));
    for(int i=0;i<res.size();i++){

        if(res[i].size()<125){
            continue;
        }

        qDebug()<<res[i].size();

        for(int j=0;j<res[i].size();j++){
            imgn[res[i][j].z][res[i][j].y][res[i][j].x]=1;
        }
    }

    vector<vector<vector<int>>> imgrot90=rotateMatrix(imgn);
    vector<vector<vector<int>>> imgflip=flipMatrixX(imgrot90);
    unsigned char* imgdata=convertVectorToUnsignedChar(imgflip);
    for(long i=0;i<img->getTotalBytes();i++){
        img->getRawData()[i]*=imgdata[i];
    }
    QString outimg=name+"_seg.tiff";
    char* outname=qstring2char(outimg);
    callback.saveImage(img,outname);
    delete img;
    return outimg;
}
