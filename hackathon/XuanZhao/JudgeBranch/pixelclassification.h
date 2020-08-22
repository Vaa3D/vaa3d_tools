#ifndef PIXELCLASSIFICATION_H
#define PIXELCLASSIFICATION_H

#include "filter.h"
#include "randomforest.h"

#include "basic_surf_objs.h"


static double sigmas[7] = {0.3,0.7,1.0,1.6,3.5,5.0,10.0};

QVector<QVector<float> > getPixelFeature(unsigned char *pdata, long long *sz);

RandomForest* trainPixelClassification(RandomForest* rf, V3DPluginCallback2* callback);

void getPixelClassificationResult(RandomForest* rf, V3DPluginCallback2* callback);


#endif // PIXELCLASSIFICATION_H
