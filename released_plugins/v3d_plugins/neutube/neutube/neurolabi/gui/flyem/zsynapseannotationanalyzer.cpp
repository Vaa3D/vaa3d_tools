#include "zsynapseannotationanalyzer.h"

#include <stdio.h>
#include <iostream>
#include "zstring.h"
#include "zfiletype.h"
#include "zjsonparser.h"

using namespace FlyEm;
using namespace std;

ZSynapseAnnotationAnalyzer::ZSynapseAnnotationAnalyzer() :
  m_minTbarDistance(50.0), m_minPsdDistance(50.0), m_minTbarPsdDistance(50.0),
  m_maxTbarPsdDistance(200.0), m_minPartnerNumber(2), m_maxPartnerNumber(10)
{
}

ZSynapseAnnotationAnalyzer::~ZSynapseAnnotationAnalyzer()
{
  for (size_t i = 0; i < m_swcTreeList.size(); i++) {
    Kill_Swc_Tree(m_swcTreeList[i]);
  }
}

void ZSynapseAnnotationAnalyzer::set(const vector<int> bodyIdList,
                                     const vector<string> swcFileList)
{
  m_swcFileList = swcFileList;
  for (size_t i = 0; i < m_swcTreeList.size(); i++) {
    Kill_Swc_Tree(m_swcTreeList[i]);
  }
  m_bodyIdMap.clear();

  m_swcTreeList.resize(m_swcFileList.size());
  for (size_t i = 0; i < m_swcTreeList.size(); i++) {
    m_bodyIdMap[bodyIdList[i]] = i;
    m_swcTreeList[i] = Read_Swc_Tree(m_swcFileList[i].c_str());
    Swc_Tree_Iterator_Start(m_swcTreeList[i], SWC_TREE_ITERATOR_DEPTH_FIRST,
                            FALSE);
  }

}

void ZSynapseAnnotationAnalyzer::setStackSize(int width, int height, int depth)
{
  m_config.width = width;
  m_config.height = height;
  m_config.depth = depth;
}

void ZSynapseAnnotationAnalyzer::setDownsampleRate(int ds1, int ds2)
{
  m_config.swcDownsample1 = ds1;
  m_config.swcDownsample2 = ds2;
}

void ZSynapseAnnotationAnalyzer::setResolution(
    double xRes, double yRes, double zRes)
{
  m_config.xResolution = xRes;
  m_config.yResolution = yRes;
  m_config.zResolution = zRes;
}

void ZSynapseAnnotationAnalyzer::setOffset(int dx, int dy, int dz)
{
  m_config.offset.set(dx, dy, dz);
}

void ZSynapseAnnotationAnalyzer::setUnit(string unit)
{
  m_config.unit = unit;
}

void ZSynapseAnnotationAnalyzer::setStartNumber(int number)
{
  m_config.startNumber = number;
}

void ZSynapseAnnotationAnalyzer::identifySynapseLocation(
    ZSynapseAnnotationArray *synapseArray)
{
  for (SynapseLocation *synapse = synapseArray->beginSynapseLocation();
       synapse != NULL; synapse = synapseArray->nextSynapseLocation()) {
    int bodyId = synapse->getBodyId();
    size_t index = m_bodyIdMap[bodyId];
    Swc_Tree *tree = m_swcTreeList[index];
    ZPoint pt =
        synapse->mapPosition(m_config, SynapseLocation::ADJUSTED_SWC_SPACE);
    double pos[3];
    pt.toArray(pos);

    Swc_Tree_Node *tn = NULL;
    double distance = Swc_Tree_Point_Dist_N(tree, pt.x(), pt.y(), pt.z(), &tn);

    if (distance < 0.0) {
      distance = 0.0;
    } else {
      distance *= m_config.xResolution *
          m_config.swcDownsample1 * m_config.swcDownsample2;
    }

    //Set synapse location
    synapse->setSwcLocation(m_swcFileList[index], bodyId, tn->node.id,
                            distance);
  }
}

void ZSynapseAnnotationAnalyzer::loadConfig(string filePath)
{
  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp == NULL) {
    cerr << "Cannot open " << filePath << endl;
  } else {
    if (ZFileType::fileType(filePath) == ZFileType::JSON_FILE) {
      ZJsonObject jsonObject;
      jsonObject.load(filePath);
      map<string, json_t*> entryMap = jsonObject.toEntryMap();

      for (map<string, json_t*>::const_iterator iter = entryMap.begin();
           iter != entryMap.end(); ++iter) {
        if (eqstr(iter->first.c_str(), "size")) {
          setStackSize(ZJsonParser::integerValue(iter->second, 0),
                       ZJsonParser::integerValue(iter->second, 1),
                       ZJsonParser::integerValue(iter->second, 2));
        } else if (eqstr(iter->first.c_str(), "downsample")) {
          setDownsampleRate(ZJsonParser::integerValue(iter->second, 0),
                            ZJsonParser::integerValue(iter->second, 1));
        } else if (eqstr(iter->first.c_str(), "offset")) {
          setOffset(ZJsonParser::integerValue(iter->second, 0),
                    ZJsonParser::integerValue(iter->second, 1),
                    ZJsonParser::integerValue(iter->second, 2));
        } else if (eqstr(iter->first.c_str(), "resolution")) {
          setResolution(ZJsonParser::numberValue(iter->second, 0),
                        ZJsonParser::numberValue(iter->second, 1),
                        ZJsonParser::numberValue(iter->second, 2));
        } else if (eqstr(iter->first.c_str(), "min_tbar_dist")) {
          m_minTbarDistance = ZJsonParser::numberValue(iter->second);
        } else if (eqstr(iter->first.c_str(), "min_psd_dist")) {
          m_minPsdDistance = ZJsonParser::numberValue(iter->second);
        } else if (eqstr(iter->first.c_str(), "max_tbar_psd_dist")) {
          m_maxTbarPsdDistance = ZJsonParser::numberValue(iter->second);
        } else if (eqstr(iter->first.c_str(), "min_tbar_psd_dist")) {
          m_minTbarPsdDistance = ZJsonParser::numberValue(iter->second);
        } else if (eqstr(iter->first.c_str(), "min_partner_number")) {
          m_minPartnerNumber = ZJsonParser::integerValue(iter->second);
        } else if (eqstr(iter->first.c_str(), "max_partner_number")) {
          m_maxPartnerNumber = ZJsonParser::integerValue(iter->second);
        }
      }
    } else { //obsolete. only json format will be supported in the future.
      ZString line;

      while (line.readLine(fp)) {
        if (line.startsWith("size:")) {
          vector<int> value = line.toIntegerArray();
          setStackSize(value[0], value[1], value[2]);
        } else if (line.startsWith("downsample:")) {
          vector<int> value = line.toIntegerArray();
          setDownsampleRate(value[0], value[1]);
        } else if (line.startsWith("offset:")) {
          vector<int> value = line.toIntegerArray();
          setOffset(value[0], value[1], value[2]);
        } else if (line.startsWith("resolution:")) {
          vector<double> value = line.toDoubleArray();
          setResolution(value[0], value[1], value[2]);
        } else if (line.startsWith("start:")) {
          setStartNumber(line.firstInteger());
        }
      }
    }

    fclose(fp);
  }
}

void FlyEm::ZSynapseAnnotationAnalyzer::print()
{
  m_config.print();
  cout << "Synapse geometric range:" << endl;
  cout << "  Minimal TBar distance: " << m_minTbarDistance << "nm" << endl;
  cout << "  Minimal PSD distance: " << m_minPsdDistance << "nm" << endl;
  cout << "  Minimal TBar-PSD distance: " << m_minTbarPsdDistance << "nm" << endl;
  cout << "  Maximal TBar-PSD distance: " << m_maxTbarPsdDistance << "nm" << endl;
  cout << "  Minimal PSD number of a TBar: " << m_minPartnerNumber << endl;
  cout << "  Maximal PSD number of a TBar: " << m_maxPartnerNumber << endl;
}
