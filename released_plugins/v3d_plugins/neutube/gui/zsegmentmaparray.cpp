#include "zsegmentmaparray.h"
#include "tz_string.h"

using namespace std;

ZSegmentMapArray::ZSegmentMapArray()
{
}

int ZSegmentMapArray::segmentToBody(int segmentId) const
{
  return m_segmentToBodyMap.at(segmentId);
}

//Body id to a set of segment id
vector<int> ZSegmentMapArray::bodyToSegment(int bodyId)
{
  vector<int> segmentArray;

  for (ZSegmentMapArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    if ((*iter).bodyId() == bodyId) {
      segmentArray.push_back((*iter).segmentId());
    }
  }

  return segmentArray;
}

void ZSegmentMapArray::print() const
{
  for (ZSegmentMapArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    (*iter).print();
  }
}

int ZSegmentMapArray::load(string filePath)
{
  clear();

  int recordNumber = 0;

  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp != NULL) {
    String_Workspace *sw = New_String_Workspace();
    int segement_map[2];
    /* For each row of the file */
    char *line = NULL;

    while ((line = Read_Line(fp, sw)) != NULL) {
      int n = 0;
      String_To_Integer_Array(line, segement_map, &n);
      if (n == 2) {
        this->push_back(ZSegmentMap(segement_map[0],
                                    segement_map[1]));
        recordNumber++;
      }
    }

    Kill_String_Workspace(sw);

    fclose(fp);
  }

  return recordNumber;
}
