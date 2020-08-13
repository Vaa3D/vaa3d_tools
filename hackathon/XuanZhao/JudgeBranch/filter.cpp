#include "filter.h"



//using namespace vigra;

float* gaussianSmooth(unsigned char* pdata, V3DLONG* sz, double sigma){
    vigra::Shape3 shape(sz[0],sz[1],sz[2]);
    vigra::MultiArray<3,unsigned char> source = vigra::MultiArray<3,unsigned char>(shape,pdata);
    vigra::MultiArray<3,float> dest(shape);

    vigra::BlockwiseConvolutionOptions<3> opt;
    opt.numThreads(-1);
    opt.innerScale(sigma);
//    vigra::TinyVector<float,3> step_size(5,5,3);
//    opt.setStepSize(step_size);
    vigra::gaussianSmoothMultiArray(source,dest,sigma,opt);
    float* data = (float*)dest.unsafePtr();
    V3DLONG tolSZ = sz[0]*sz[1] *sz[2];
    float* outdata = new float[tolSZ];
    for(int i=0;i<tolSZ; i++){
        outdata[i] = data[i];
    }
    return outdata;
}

float* gaussianGradientMagnitude(unsigned char *pdata, long long *sz, double sigma){
    vigra::Shape3 shape(sz[0],sz[1],sz[2]);
    vigra::MultiArray<3,unsigned char> source = vigra::MultiArray<3,unsigned char>(shape,pdata);
    vigra::MultiArray<3,float> dest(shape);
    vigra::BlockwiseConvolutionOptions<3> opt;
    opt.numThreads(-1);
    opt.innerScale(sigma);
//    vigra::TinyVector<float,3> step_size(5,5,3);
//    opt.setStepSize(step_size);
    vigra::gaussianGradientMagnitude(source,dest,sigma,opt);
    float* data = (float*)dest.unsafePtr();
    V3DLONG tolSZ = sz[0]*sz[1] *sz[2];
    float* outdata = new float[tolSZ];
    for(int i=0;i<tolSZ; i++){
        outdata[i] = data[i];
    }
    return outdata;
}

float* laplacianOfGaussian(unsigned char* pdata, V3DLONG* sz, double sigma){
    vigra::Shape3 shape(sz[0],sz[1],sz[2]);
    vigra::MultiArray<3,unsigned char> source = vigra::MultiArray<3,unsigned char>(shape,pdata);
    vigra::MultiArray<3,float> dest(shape);
    vigra::BlockwiseConvolutionOptions<3> opt;
    opt.numThreads(-1);
    opt.innerScale(sigma);
//    vigra::TinyVector<float,3> step_size(5,5,3);
//    opt.setStepSize(step_size);
    vigra::laplacianOfGaussianMultiArray(source,dest,sigma,opt);
    float* data = (float*)dest.unsafePtr();
    V3DLONG tolSZ = sz[0]*sz[1] *sz[2];
    float* outdata = new float[tolSZ];
    for(int i=0;i<tolSZ; i++){
        outdata[i] = data[i];
    }
    return outdata;
}

vector<vector<float> > hessianOfGaussian(unsigned char *pdata, long long *sz, double sigma){
    V3DLONG tolSZ = sz[0]*sz[1] *sz[2];
    vigra::Shape3 shape(sz[0],sz[1],sz[2]);
    vigra::MultiArray<3,unsigned char> source = vigra::MultiArray<3,unsigned char>(shape,pdata);
    vigra::MultiArray<3,vigra::TinyVector<float,6> > dest(shape);

    vigra::BlockwiseConvolutionOptions<3> opt;
    opt.numThreads(-1);
    opt.innerScale(sigma);
//    vigra::TinyVector<float,3> step_size(5,5,3);
//    opt.setStepSize(step_size);

    qDebug()<<"start to hessian";
    vigra::hessianOfGaussianMultiArray(source,dest,sigma,opt);
    qDebug()<<"end to hessian";
    vigra::TinyVector<float,6>* data = (vigra::TinyVector<float,6>*)dest.unsafePtr();
    vector<vector<float> > outdata = vector<vector<float> >(tolSZ,vector<float>());
    for(int i=0; i<tolSZ; i++){
        for(int j=0; j<6; j++){
            outdata[i].push_back(data[i][j]);
        }
    }
    qDebug()<<"copy data";
    return outdata;
}






