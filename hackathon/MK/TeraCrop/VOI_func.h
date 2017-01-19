#ifndef VOI_FUNC_H
#define VOI_FUNC_H

#include <v3d_interface.h>

bool SpecDialog(V3DPluginCallback2 &callback, QWidget *parent);
void Piling2Dimages(unsigned char* InputImagePtr, unsigned char* OutputImagePtr, 
				    int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, 
					int x_Length, int y_Length, int z_Length);

NeuronTree cropSWCfile3D(NeuronTree nt, int xb, int xe, int yb, int ye, int zb, int ze, int type);
QStringList importSeriesFileList_addnumbersort(const QString & curFilePath);


#endif // VOI_FUNC_H
