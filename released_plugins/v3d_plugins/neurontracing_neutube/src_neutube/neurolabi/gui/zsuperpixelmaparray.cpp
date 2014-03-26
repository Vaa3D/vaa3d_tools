#include "zsuperpixelmaparray.h"

#include <iostream>
#include "tz_string.h"
#include "tz_intpair_map.h"
#include "tz_error.h"

using namespace std;

ZSuperpixelMapArray::ZSuperpixelMapArray()
{
}

vector<int> ZSuperpixelMapArray::segmentToSuperpixel(int segmentId)
{
  vector<int> superpixelArray;

  for (ZSuperpixelMapArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    if ((*iter).segmentId() == segmentId) {
      superpixelArray.push_back((*iter).superpixelId());
    }
  }

  return superpixelArray;
}

vector<int> ZSuperpixelMapArray::segmentToSuperpixel(
    const vector<int> &segmentId)
{
  vector<int> superpixelArray;

  for (vector<int>::const_iterator iter = segmentId.begin();
       iter != segmentId.end(); ++iter) {
    vector<int> subArray = segmentToSuperpixel(*iter);
    superpixelArray.insert(superpixelArray.end(), subArray.begin(),
                           subArray.end());
  }

  return superpixelArray;
}

int ZSuperpixelMapArray::load(const string &filePath, int planeId)
{
  clear();

  int recordNumber = 0;

  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp != NULL) {
    String_Workspace *sw = New_String_Workspace();
    int superpixel_map[4];
    /* For each row of the file */
    char *line = NULL;

    while ((line = Read_Line(fp, sw)) != NULL) {
      int n = 0;
      String_To_Integer_Array(line, superpixel_map, &n);
      if (n == 3) {
        /* If the plane id is the target */
        if (superpixel_map[0] == planeId || planeId < 0) {
          /* Add the plane id to the map at the superpixel ID position */
          this->push_back(ZSuperpixelMap(superpixel_map[0], superpixel_map[1],
                                         superpixel_map[2]));
          recordNumber++;
        }
      } else if (n == 4) {
        /* If the plane id is the target */
        if (superpixel_map[0] == planeId || planeId < 0) {
          /* Add the plane id to the map at the superpixel ID position */
          this->push_back(ZSuperpixelMap(superpixel_map[0], superpixel_map[1],
                                         superpixel_map[2], superpixel_map[3]));
          recordNumber++;
        }
      }
    }

    Kill_String_Workspace(sw);

    fclose(fp);
  } else {
    cout << "Failed to open " << filePath << endl;
  }

  return recordNumber;
}

void ZSuperpixelMapArray::print() const
{
  for (ZSuperpixelMapArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    (*iter).print();
  }
}

void ZSuperpixelMapArray::setBodyId(const ZIntMap &segMapArray)
{
  for (ZSuperpixelMapArray::iterator iter = begin(); iter != end();
       ++iter) {
    iter->setBodyId(segMapArray.at(iter->segmentId()));
  }
}

ZStack* ZSuperpixelMapArray::mapStack(ZStack &stack)
{
  ZStack *out = NULL;

  int maxBodyId = 0;
  int minPlaneId = -1;

  //Intpair_Map *superpixelToBodyMap = New_Intpair_Map();
  vector<map<int, int> > superpixelToBodyMap(stack.depth());

  for (const_iterator iter = begin(); iter != end(); ++iter) {
    //superpixelToBodyMap[iter->superpixelId()] = iter->bodyId();
    if (maxBodyId < iter->bodyId()) {
      maxBodyId = iter->bodyId();
    }
    if (minPlaneId < 0 || minPlaneId > iter->planeId()) {
      minPlaneId = iter->planeId();
    }
  }

  for (const_iterator iter = begin(); iter != end(); ++iter) {
    superpixelToBodyMap[iter->planeId() - minPlaneId][iter->superpixelId()] =
        iter->bodyId();
  }

  int nchannel = 0;
  while (maxBodyId >0) {
    nchannel++;
    maxBodyId /= 256;
  }
  out = new ZStack(GREY, stack.width(), stack.height(), stack.depth(),
                   nchannel);

  //size_t volume = stack.voxelNumber();
  //ZSingleChannelStack *ch[3] = { NULL, NULL, NULL };
  uint8_t *ch_array8[3] = { NULL, NULL, NULL };
  uint16_t *ch_array16[3] = { NULL, NULL, NULL };

  size_t stride = 1;
  for (size_t i = 0; i < 3; i++) {
    if (stack.kind() == GREY) {
      ch_array8[i] = stack.array8(i);
    } else if (stack.kind() == GREY16) {
      ch_array16[i] = stack.array16(i);
    } else if (stack.kind() == COLOR){
      ch_array8[i] = stack.array8() + i;
      stride = 3;
    } else {
      TZ_ERROR(ERROR_DATA_TYPE);
    }
  }


  size_t voxelIndex = 0;
  int area = stack.width() * stack.height();
  for (int z = 0; z < stack.depth(); z++) {
    for (int p = 0; p < area; p++) {
      int superpixelId = 0;
      if (ch_array8[0] != NULL) {
        superpixelId += ch_array8[0][voxelIndex * stride];
      } else if (ch_array16[0] != NULL) {
        superpixelId += ch_array16[0][voxelIndex];
      }

      if (ch_array8[1] != NULL) {
        superpixelId += 256 * ch_array8[1][voxelIndex * stride];
      } else if (ch_array16[1] != NULL) {
        superpixelId += 256 * ch_array16[1][voxelIndex];
      }

      if (ch_array8[2] != NULL) {
        superpixelId += 65536 * ch_array8[2][voxelIndex * stride];
      } else if (ch_array16[2] != NULL) {
        superpixelId += 65536 * ch_array16[2][voxelIndex];
      }

      //int bodyId = superpixelToBodyMap[superpixelId];
      /*
      int bodyId = Intpair_Map_Value(superpixelToBodyMap, z + minPlaneId,
                                     superpixelId);
                                     */
#ifdef _DEBUG_2
      if (superpixelId > 0) {
        cout << "debug here" << endl;
      }
#endif

      int bodyId = superpixelToBodyMap[z][superpixelId];

      for (int c = 0; c < out->channelNumber(); c++) {
        out->array8(c)[voxelIndex] = bodyId % 256;
        bodyId /= 256;
      }
      voxelIndex++;
    }
  }

  //Kill_Intpair_Map(superpixelToBodyMap);

  return out;
}

ZStack* ZSuperpixelMapArray::mapBoundaryStack(ZStack &stack)
{
  ZStack *out = NULL;

  int minPlaneId = -1;

  //Intpair_Map *superpixelToBodyMap = New_Intpair_Map();
  vector<map<int, int> > superpixelToBodyMap(stack.depth());

  for (const_iterator iter = begin(); iter != end(); ++iter) {
    if (minPlaneId < 0 || minPlaneId > iter->planeId()) {
      minPlaneId = iter->planeId();
    }
  }

  for (const_iterator iter = begin(); iter != end(); ++iter) {
    superpixelToBodyMap[iter->planeId() - minPlaneId][iter->superpixelId()] =
        iter->bodyId();
  }

  out = new ZStack(GREY, stack.width(), stack.height(), stack.depth(),
                   1);

  //size_t volume = stack.voxelNumber();
  //ZSingleChannelStack *ch[3] = { NULL, NULL, NULL };
  uint8_t *ch_array8[3] = { NULL, NULL, NULL };
  uint16_t *ch_array16[3] = { NULL, NULL, NULL };

  for (size_t i = 0; i < 3; i++) {
    if (stack.kind() == GREY) {
      ch_array8[i] = stack.array8(i);
    } else if (stack.kind() == GREY16) {
      ch_array16[i] = stack.array16(i);
    }
    /*
    ch[i] = stack.singleChannel(i);
    if (ch[i] != NULL) {
      if (ch[i]->kind() == GREY) {
        ch_array8[i] = ch[i]->array8();
      }
      if (ch[i]->kind() == GREY16) {
        ch_array16[i] = ch[i]->array16();
      }
    }
    */
  }


  size_t voxelIndex = 0;
  int area = stack.width() * stack.height();
  for (int z = 0; z < stack.depth(); z++) {
    for (int p = 0; p < area; p++) {
      int superpixelId = 0;
      if (ch_array8[0] != NULL) {
        superpixelId += ch_array8[0][voxelIndex];
      } else if (ch_array16[0] != NULL) {
        superpixelId += ch_array16[0][voxelIndex];
      }

      if (ch_array8[1] != NULL) {
        superpixelId += 256 * ch_array8[1][voxelIndex];
      } else if (ch_array16[1] != NULL) {
        superpixelId += 256 * ch_array16[1][voxelIndex];
      }

      if (ch_array8[2] != NULL) {
        superpixelId += 65536 * ch_array8[2][voxelIndex];
      } else if (ch_array16[2] != NULL) {
        superpixelId += 65536 * ch_array16[2][voxelIndex];
      }

      //int bodyId = superpixelToBodyMap[superpixelId];
      /*
      int bodyId = Intpair_Map_Value(superpixelToBodyMap, z + minPlaneId,
                                     superpixelId);
                                     */
#ifdef _DEBUG_2
      if (z == 0 && superpixelId == 88) {
        cout << "debug here" << endl;
      }
#endif

      int bodyId = superpixelToBodyMap[z][superpixelId];

      for (int c = 0; c < out->channelNumber(); c++) {
        out->array8(c)[voxelIndex] = bodyId % 256;
        bodyId /= 256;
      }
      voxelIndex++;
    }
  }

  //Kill_Intpair_Map(superpixelToBodyMap);

  return out;
}

void ZSuperpixelMapArray::append(
    int planeId, int superpixelId, int segmentId, int bodyId)
{
  resize(size() + 1);
  back().set(planeId, superpixelId, segmentId, bodyId);
}

void ZSuperpixelMapArray::compressBodyId(int startId)
{
  TZ_ASSERT(startId >= 0, "Negative start id");

  map<int, int> bodyIdIndexMap;

  int lastIndex = 0;
  for (size_t i = 0; i < size(); ++i) {
    int bodyId = (*this)[i].bodyId();
    if (bodyIdIndexMap.count(bodyId) == 0) {
      bodyIdIndexMap[bodyId] = lastIndex++;
    }
  }

  for (ZSuperpixelMapArray::iterator iter = begin(); iter != end(); ++iter) {
    iter->setBodyId(bodyIdIndexMap[iter->bodyId()] + startId);
  }
}

int ZSuperpixelMapArray::minPlaneId()
{
  int minId = front().planeId();
  for (ZSuperpixelMapArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    if (minId > iter->planeId()) {
      minId = iter->planeId();
    }
  }

  return minId;
}
