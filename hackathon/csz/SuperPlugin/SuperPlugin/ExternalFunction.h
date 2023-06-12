#ifndef EXTERNALFUNCTION_H
#define EXTERNALFUNCTION_H

#include <v3d_interface.h>
#include "OutSource.h"
#include "ImageQuality.h"


#ifndef VOID_VALUE
#define VOID_VALUE 1000000000
#endif


bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);
QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons, QHash<V3DLONG, NeuronSWC> & LUT_newid_to_node);
QVector< QVector<V3DLONG> > get_neighbors(QList<NeuronSWC> &neurons, const QHash<V3DLONG,V3DLONG> & LUT);
QList<V3DLONG> DFS(QVector< QVector<V3DLONG> > neighbors, V3DLONG newrootid, V3DLONG siz);
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
bool combine_list2file(QList<NeuronSWC> & lN, QString fileSaveName);
bool rescale_to_0_255_and_copy(unsigned char *inimg, V3DLONG datalen, double & minn, double & maxx, unsigned char *outimg);
char* qstring2char(QString text);

Delivery PackImage(Image4DSimple *img,QString IP="127.0.0.1",QString Port="6869",QString filepath="",QString name="",QString method="",bool sendimg=false,bool quality=true);
bool mean_and_std(unsigned char *data, V3DLONG n, double & ave, double & sdev);
#endif // EXTERNALFUNCTION_H
