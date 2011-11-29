/* minMaxfilterplugin.h
 * 2009-08-19: create this program by Yang Yu
 */


#ifndef __MINMAXFILTER_H__
#define __MINMAXFILTER_H__

//     ND min Max min-Max Max-min filter
//
//     pImage  - input & output
//     N       - width of the image
//     M       - height of the image
//     P       - depth of the image
//     Wx      - radius size along x not include itself
//     Wy      - radius size along y
//     Wz      - radius size along z
//
//int minMaxfilter(float *pImage, V3DLONG N, V3DLONG M, V3DLONG P, unsigned int Wx, unsigned int Wy, unsigned int Wz);
//e.g. if(!minMaxfilter(pImg, sz[0], sz[1], sz[2], sigmax, sigmay, sigmaz))
//			cout<<"err"<<endl;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class minMaxFilterPlugin : public QObject, public V3DSingleImageInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface2_1)

public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent);
    float getPluginVersion() const {return 1.1f;}
};

#endif



