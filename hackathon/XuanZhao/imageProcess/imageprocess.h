#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H


class imageProcess
{
public:
    imageProcess(){
        pdata = 0;
        sx = sy = 0;
        lmin = 0;lmax = 4095;
    }
//    imageProcess(QString imageFile);
    unsigned short** pdata;
    int sx, sy;
    unsigned short lmin, lmax;
    double F(unsigned short intensity, double t);
    unsigned short  ENT(int i, int j, int t, int mode);
    double H(unsigned short** data);
    unsigned short** process(int mode);
    unsigned short** newImageM();
    bool deleteImageM(unsigned short** data);
};

#endif // IMAGEPROCESS_H
