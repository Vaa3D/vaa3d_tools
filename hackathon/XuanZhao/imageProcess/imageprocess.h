#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include "v3d_interface.h"
//#include <QThread>

//static int m[10];

//class convertDataThread : public QThread
//{
//public:
//    convertDataThread() {}
//protected:
//    void run(int i, int j);
//};

void enhanceImage(unsigned char* data1d, V3DLONG* sz);

void get_2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

void getSwcL0Image(QString swcPath, QString brainPath, QString outPath, int times, V3DPluginCallback2 & callback);

void convertData(QString path, V3DPluginCallback2 &callback);

void _convertData(QString tifPath, V3DPluginCallback2 &callback);

void downSampleData(QString path, int times, V3DPluginCallback2 &callback);

void _downSampleData(QString tifPath, int times, V3DPluginCallback2 &callback);

void joinImage(QString tifDir, QString outPath, int times, V3DPluginCallback2 &callback);

void getSubImage(QString tifDir, QString outPath, size_t x0, size_t x1,
                 size_t y0, size_t y1, size_t z0, size_t z1,
                 int downSampleTimes, int resolution, V3DPluginCallback2 &callback);

void testThread();

#endif // IMAGEPROCESS_H
