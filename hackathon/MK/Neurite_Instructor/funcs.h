#ifndef FUNCS_H
#define FUNCS_H

#include "Neurite_Instructor_plugin.h"
#include "neuriteinstructorui.h"
#include "v3d_message.h"
#include <vector>
#include <iostream>


using namespace std;

void cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
    int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);

void maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
    int xlb, int xhb, int ylb, int yhb, int zlb, int zhb);

//QStringList importSeriesFileList_addnumbersort(const QString & curFilePath);

void predictSWCstroke(neuriteInstructorUI*, V3DPluginCallback2*, v3dhandle);


#endif // FUNCS_H
