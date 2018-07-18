#ifndef SOMA_REMOVE_MAIN_H
#define SOMA_REMOVE_MAIN_H
#include "v3d_interface.h"
#include "v3d_message.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h"
#include "../../heyishan/blastneuron_bjut/overlap/overlap_gold.h"
#include  <vector>

//struct Linker
//{
//    Coordinate coord;
//    int ind;
//};
bool soma_remove_main(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
bool soma_remove_main_2(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);

#endif // SOMA_REMOVE_MAIN_H
