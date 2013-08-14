//by Hanchuan Peng
//090518
//090609: change location of pointcloud_atlas_io.h

#ifndef __CONVERTER_PC_ATLASDATA_H__
#define __CONVERTER_PC_ATLASDATA_H__

#include "pointcloud_atlas_io.h"
#include "FL_atlasBuilder.h"

void convert_FLAtlasBuilderInfo_to_apoAtlasLinkerInfo(const MarkerGeneInfo & m, apoAtlasLinkerInfoAll & a);
void convert_FLAtlasBuilderInfo_from_apoAtlasLinkerInfo(MarkerGeneInfo & m, const apoAtlasLinkerInfoAll & a);

#endif

