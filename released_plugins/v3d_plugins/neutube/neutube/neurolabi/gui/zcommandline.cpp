#include "zcommandline.h"

#include <QFileInfoList>
#include <QDir>
#include <ostream>
#include <fstream>
#include <set>

#include "zargumentprocessor.h"
#include "ztest.h"
#include "zobject3dscan.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "tz_error.h"
#include "flyem/zflyemqualityanalyzer.h"

ZCommandLine::ZCommandLine() : m_ravelerHeight(2599), m_zStart(1490)
{
}

ZCommandLine::ECommand ZCommandLine::getCommand(const char *cmd)
{
  if (eqstr(cmd, "sobj_marker")) {
    return OBJECT_MARKER;
  }

  if (eqstr(cmd, "boundary_orphan")) {
    return BOUNDARY_ORPHAN;
  }

  return UNKNOWN_COMMAND;
}

int ZCommandLine::runObjectMarker()
{
  QDir dir(m_input[0].c_str());
  QStringList filters;
  filters << "*.sobj";

  std::cout << "Scanning object files: " << dir.absolutePath().toStdString()
            << "..." << std::endl;
  QFileInfoList fileList = dir.entryInfoList(filters);
  json_t *obj = json_object();
  json_t *dataObj = json_array();
  std::cout << "Computing marker locations ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    ZObject3dScan obj;
    std::cout << fileInfo.filePath().toStdString() << std::endl;
    obj.load(fileInfo.absoluteFilePath().toStdString());
    if (!obj.isEmpty()) {
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(m_ravelerHeight - 1 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(dataObj, arrayObj);
    }
  }
  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, m_output.c_str(), JSON_INDENT(2));

  return 0;
}

int ZCommandLine::runBoundaryOrphan()
{
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(ZArgumentProcessor::getStringArg("--boundary_orphan"));
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  int zStart = boundBox.cb[2] - 10;
  int ravelerHeight = boundBox.cb[1] - boundBox.ce[1] - 1 + 20;

  if (zStart != m_zStart || ravelerHeight != m_ravelerHeight) {
    std::cout << "Inconsistent values" << std::endl;
    std::cout << "z: " << zStart << "; H: " << ravelerHeight << std::endl;
    std::cout << "Abort" << std::endl;

    return 1;
  }

  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  //blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  QDir dir(ZArgumentProcessor::getStringArg("input"));
  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.getVoxelNumber() < 100000) {
      if (obj.isEmpty()) {
        std::cout << "Empty object: "
                  << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      }

      if (qc.isStitchedOrphanBody(obj)) {
        std::cout << "Orphan: "
                  << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                  << std::endl;
        ZVoxel voxel = obj.getMarker();
        std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        TZ_ASSERT(voxel.x() >= 0, "invalid point");

        json_array_append(arrayObj, json_integer(voxel.x()));
        json_array_append(arrayObj, json_integer(m_ravelerHeight - 1 - voxel.y()));
        json_array_append(arrayObj, json_integer(voxel.z() + m_zStart));
        json_array_append(dataObj, arrayObj);
      }
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, ZArgumentProcessor::getStringArg("-o"), JSON_INDENT(2));

  return 0;
}

int ZCommandLine::runObjectOverlap()
{
  if (m_input.size() != 2) {
    std::cerr << "Invalid number of input files" << std::endl;
    return 1;
  }

  QDir dir1(m_input[0].c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList1 = dir1.entryInfoList(filters);
  std::cout << fileList1.size() << " objects found in " << dir1.absolutePath().toStdString()
            << std::endl;

  QDir dir2(m_input[1].c_str());
  QFileInfoList fileList2 = dir2.entryInfoList(filters);
  std::cout << fileList2.size() << " objects found in " << dir2.absolutePath().toStdString()
            << std::endl;

  std::set<int> excludedBodySet;

  if (m_fullOverlapScreen) {
    foreach (QFileInfo fileInfo, fileList1) {
      QString fileName = fileInfo.fileName();
      QFileInfo targetPath = dir2.absoluteFilePath(fileName);
      if (targetPath.exists()) {
        ZObject3dScan obj1;
        obj1.load(fileInfo.absoluteFilePath().toStdString());
        ZObject3dScan obj2;
        obj2.load(targetPath.absoluteFilePath().toStdString());
        if (obj1.equalsLiterally(obj2)) {
          std::cout << "Untouched object: " << fileInfo.baseName().toStdString()
                    << std::endl;
        }
        excludedBodySet.insert(
              ZString::lastInteger(fileInfo.baseName().toStdString()));
      }
    }
  }

  QVector<ZObject3dScan> objArray1(fileList1.size());
  QVector<ZObject3dScan> objArray2(fileList2.size());
  //objArray1.resize(100);
  //objArray2.resize(100);

  QVector<int> idArray1(fileList1.size());
  QVector<int> idArray2(fileList2.size());

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray1.size(); ++i) {
    int id = ZString::lastInteger(fileList1[i].baseName().toStdString());

    if (excludedBodySet.count(id) == 0) {
      objArray1[i].load(fileList1[i].absoluteFilePath().toStdString());
      std::cout << i << ": Object size: " << objArray1[i].getVoxelNumber() << std::endl;
      objArray1[i].downsample(m_intv[0], m_intv[1], m_intv[2]);
      objArray1[i].canonize();
      std::cout << "Object size: " << objArray1[i].getVoxelNumber() << std::endl;
      idArray1[i] = id;
    }
  }

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray2.size(); ++i) {
    int id = ZString::lastInteger(fileList2[i].baseName().toStdString());
    if (excludedBodySet.count(id) == 0) {
      objArray2[i].load(fileList2[i].absoluteFilePath().toStdString());
      objArray2[i].downsample(m_intv[0], m_intv[1], m_intv[2]);
      objArray2[i].canonize();
      idArray2[i] = id;
    }
  }

  QVector<Cuboid_I> boundBoxArray1(objArray1.size());
  QVector<Cuboid_I> boundBoxArray2(objArray2.size());

  std::cout << "Computing bound box ..." << std::endl;
  for (int i = 0; i < objArray1.size(); ++i) {
    objArray1[i].getBoundBox(&(boundBoxArray1[i]));
  }

  std::cout << "Computing bound box ..." << std::endl;
  for (int i = 0; i < objArray2.size(); ++i) {
    objArray2[i].getBoundBox(&(boundBoxArray2[i]));
  }

  int index1 = 0;

  std::ofstream stream(m_output.c_str());

  int offset[3];
  foreach(Cuboid_I boundBox1, boundBoxArray1) {
    if (!objArray1[index1].isEmpty()) {
      Stack *stack = objArray1[index1].toStack(offset);
      for (int i = 0; i < 3; ++i) {
        offset[i] = -offset[i];
      }
      int id1 = idArray1[index1];
      std::cout << "ID: " << id1 << std::endl;
      int index2 = 0;
      foreach (Cuboid_I boundBox2, boundBoxArray2) {
        if (Cuboid_I_Overlap_Volume(&boundBox1, &boundBox2) > 0) {
          int id2 = idArray2[index2];
          size_t overlap =
              objArray2[index2].countForegroundOverlap(stack, offset);
          if (overlap > 0) {
            std::cout << id1 << " " << id2 << " " << overlap << std::endl;
            stream << id1 << " " << id2 << " " << overlap << std::endl;
          }
        }
        ++index2;
      }
      C_Stack::kill(stack);
    }
    ++index1;
  }

  stream.close();

  return 0;
}

int ZCommandLine::runSynapseObjectList()
{
  std::set<int> bodySet;
  FlyEm::ZSynapseAnnotationArray synaseArray;
  if (!synaseArray.loadJson(m_input[0])) {
    return 1;
  }

  for (FlyEm::SynapseLocation *location = synaseArray.beginSynapseLocation();
       location != NULL; location = synaseArray.nextSynapseLocation()) {
    bodySet.insert(location->bodyId());
  }

  std::ofstream stream(m_output.c_str());

  for (std::set<int>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    stream << *iter << std::endl;
  }

  stream.close();

  return 0;
}

int ZCommandLine::run(int argc, char *argv[])
{
#if !defined(__MINGW32__)
  static const char *Spec[] = {
    "--command",  "[--unit_test]", "[<input:string> ...] [-o <string>]",
    "[--sobj_marker]", "[--boundary_orphan <string>]", "[--config <string>]",
    "[--sobj_overlap]", "[--intv <int> <int> <int>]", "[--fulloverlap_screen]",
    "[--synapse_object]", 0
  };

  ZArgumentProcessor::processArguments(
        argc, argv, Spec);

  if (ZArgumentProcessor::isArgMatched("--unit_test")) {
#if !defined(__MINGW32__)
    return ZTest::runUnitTest(argc, argv);
#else
    return 1;
#endif
  }

  ECommand command = UNKNOWN_COMMAND;
  m_input.clear();
  for (int i = 0; i < 3; ++i) {
    m_intv[i] = 0;
  }


  if (ZArgumentProcessor::isArgMatched("--config")) {
    ZJsonObject obj;
    obj.load(ZArgumentProcessor::getStringArg("--config"));

    command = getCommand(ZJsonParser::stringValue(obj["command"]));
    m_input.push_back(ZJsonParser::stringValue(obj["input"]));
    m_output = ZJsonParser::stringValue(obj["output"]);

    if (obj.hasKey("raveler_height")) {
      m_ravelerHeight = ZJsonParser::integerValue(obj["raveler_height"]);
    }

    if (obj.hasKey("z_start")) {
      m_zStart = ZJsonParser::integerValue(obj["z_start"]);
    }

    m_blockFile = ZJsonParser::integerValue(obj["block_file"]);
  } else if (ZArgumentProcessor::isArgMatched("--sobj_marker")) {
    command = OBJECT_MARKER;
    m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
    m_output = ZArgumentProcessor::getStringArg("-o");
  } else if (ZArgumentProcessor::isArgMatched("--boundary_orphan")) {
    command = BOUNDARY_ORPHAN;
    m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
    m_output = ZArgumentProcessor::getStringArg("-o");
    m_blockFile = ZArgumentProcessor::getStringArg("--boundary_orphan");
  } else if (ZArgumentProcessor::isArgMatched("--sobj_overlap")) {
    command = OBJECT_OVERLAP;
    m_fullOverlapScreen = false;
    int inputNumber = ZArgumentProcessor::getRepeatCount("input");
    m_input.resize(inputNumber);
    for (int i = 0; i < inputNumber; ++i) {
      m_input[i] = ZArgumentProcessor::getStringArg("input", i);
    }
    m_output = ZArgumentProcessor::getStringArg("-o");

    if (ZArgumentProcessor::isArgMatched("--intv")) {
      for (int i = 1; i < 3; ++i) {
        m_intv[i] = ZArgumentProcessor::getIntArg("--intv", i);
      }
    }

    m_fullOverlapScreen =
        ZArgumentProcessor::isArgMatched("--fulloverlap_screen");
  } else if (ZArgumentProcessor::isArgMatched("--synapse_object")) {
    command = SYNAPSE_OBJECT;
    m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
    m_output = ZArgumentProcessor::getStringArg("-o");
  }

  switch (command) {
  case OBJECT_MARKER:
    return runObjectMarker();
  case BOUNDARY_ORPHAN:
    return runBoundaryOrphan();
  case OBJECT_OVERLAP:
    return runObjectOverlap();
  case SYNAPSE_OBJECT:
    return runSynapseObjectList();
  default:
    std::cout << "Unknown command" << std::endl;
    return 1;
  }
#endif

  return 0;
}
