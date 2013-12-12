#if defined(_USE_EIGEN_)
#include <Eigen/Dense>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <list>
#include "zeigenutils.h"

using namespace Eigen;
using namespace std;

ZMesh::ZMesh()
{
}

bool ZMesh::convertAllenAtlasMeshToWavefrontObj(const char* iFile, const char* oFile)
{
  Matrix<float, Dynamic, Dynamic, RowMajor> normals;
  Matrix<float, Dynamic, Dynamic, RowMajor> vertices;
  Matrix<int, Dynamic, Dynamic, RowMajor> triangles;
  if (!readAllenAtlasMesh(iFile, normals, vertices, triangles))
    return false;
  if (!writeWavefrontObj(oFile, normals, vertices, triangles))
    return false;
  return true;
}

bool ZMesh::convertAllenAtlasMeshToHxSurface(const char *iFile, const char *oFile)
{
  Matrix<float, Dynamic, Dynamic, RowMajor> normals;
  Matrix<float, Dynamic, Dynamic, RowMajor> vertices;
  Matrix<int, Dynamic, Dynamic, RowMajor> triangles;
  if (!readAllenAtlasMesh(iFile, normals, vertices, triangles))
    return false;
  if (!writeHxSurface(oFile, vertices, triangles))
    return false;
  return true;
}

bool ZMesh::convertAllenAtlasMeshToHxSurface(const QString &iFile, const char *oFile)
{
  return convertAllenAtlasMeshToHxSurface(iFile.toLocal8Bit().constData(), oFile);
}

bool ZMesh::convertWavefrontObjToHxSurface(const char *iFile, const char *oFile)
{
  Matrix<float, Dynamic, Dynamic, RowMajor> normals;
  Matrix<float, Dynamic, Dynamic, RowMajor> vertices;
  Matrix<int, Dynamic, Dynamic, RowMajor> triangles;
  if (!readWavefrontObj(iFile, normals, vertices, triangles))
    return false;
  if (!writeHxSurface(oFile, vertices, triangles))
    return false;
  return true;
}

bool ZMesh::convertWavefrontObjToHxSurface(const QString &iFile, const char *oFile)
{
  return convertWavefrontObjToHxSurface(iFile.toLocal8Bit().constData(), oFile);
}

bool ZMesh::readAllenAtlasMesh(const char *iFile, Matrix<float, Dynamic, Dynamic, RowMajor> &normals,
                               Matrix<float, Dynamic, Dynamic, RowMajor> &vertices,
                               Matrix<int, Dynamic, Dynamic, RowMajor> &triangles)
{
  // sizeof int and sizeof float should be 4, otherwise fail
  assert(sizeof(int)==4);
  assert(sizeof(float)==4);
  if (sizeof(int) != 4 || sizeof(float) != 4)
    return false;

  std::ifstream inputFileStream;
  inputFileStream.open(iFile, std::ios::in | std::ios::binary);
  if (!inputFileStream.is_open()) {
    std::perror((std::string("readAllenAtlasMesh: error while opening file ") + iFile).c_str());
    return false;
  }

  int numPoints;
  inputFileStream.read((char*)&numPoints, 4);

  normals.resize(numPoints, 3);
  vertices.resize(numPoints, 3);

  //read points
  // 3 floats for normals followed by 3 floats for coordinates per point
  float normal[3];
  float vertice[3];
  for (int i=0; i<numPoints; i++) {
    inputFileStream.read((char*)normal, 3*4);
    normals.row(i) << normal[0], normal[1], normal[2];
    inputFileStream.read((char*)vertice, 3*4);
    vertices.row(i) << vertice[0], vertice[1], vertice[2];
  }

  int numTriangleStrips = 0;
  inputFileStream.read((char*)&numTriangleStrips, 4);
  std::vector<std::vector<int> > allStrips;

  // read triangle strips
  for (int i=0; i<numTriangleStrips; i++) {
    //read number of points in the strip
    int numPointsInStrip = 0;
    inputFileStream.read((char*)&numPointsInStrip, 2);

    if (numPointsInStrip > 0) {
      std::vector<int> strip(numPointsInStrip);
      inputFileStream.read((char*)strip.data(), numPointsInStrip*4);
      allStrips.push_back(strip);
    }
  }
  inputFileStream.close();

  int numTriangles = 0;
  for (int i=0; i<numTriangleStrips; i++) {
    numTriangles += allStrips[i].size() - 2;
  }
  triangles.resize(numTriangles, 3);

  int triIdx = 0;
  for (int i=0; i<numTriangleStrips; i++) {
    for (size_t j = 0; j < allStrips[i].size()-2; j++) {
      //Indicies in the triStripIndices are like: ABCDEFG
      //We need to change them to be ABC CBD CDE EDF EFG(note swapping of ordering)
      if (j%2 == 0) {
        triangles.row(triIdx++) << allStrips[i][j]+1, allStrips[i][j+1]+1, allStrips[i][j+2]+1;
      } else {
        triangles.row(triIdx++) << allStrips[i][j+1]+1, allStrips[i][j]+1, allStrips[i][j+2]+1;
      }
    }
  }
  return true;
}

bool ZMesh::readAllenAtlasMesh(const QStringList &iFiles, Matrix<float, Dynamic, Dynamic, RowMajor> &normals,
                               Matrix<float, Dynamic, Dynamic, RowMajor> &vertices,
                               Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &triangles, QList<int> &pointSizes, QList<int> &triangleSizes)
{
  if (iFiles.size() == 0)
    return false;
  pointSizes.clear();
  triangleSizes.clear();
  bool ok;
  for (int i=0; i<iFiles.size(); i++) {
    if (i==0) {
      ok = readAllenAtlasMesh(iFiles[i].toLocal8Bit().constData(), normals, vertices, triangles);
      if (!ok)
        return false;
      pointSizes.push_back(vertices.rows());
      triangleSizes.push_back(triangles.rows());
    } else {
      Matrix<float, Dynamic, Dynamic, RowMajor> currentNormals;
      Matrix<float, Dynamic, Dynamic, RowMajor> currentVertices;
      Matrix<int, Dynamic, Dynamic, RowMajor> currentTriangles;
      ok = readAllenAtlasMesh(iFiles[i].toLocal8Bit().constData(), currentNormals, currentVertices, currentTriangles);
      if (!ok)
        return false;
      pointSizes.push_back(vertices.rows());
      triangleSizes.push_back(currentTriangles.rows());
      int pointStartIdx = vertices.rows();
      currentTriangles.array() += pointStartIdx;
      normals.conservativeResize(normals.rows()+currentNormals.rows(), NoChange);
      vertices.conservativeResize(vertices.rows()+currentVertices.rows(), NoChange);
      triangles.conservativeResize(triangles.rows()+currentTriangles.rows(), NoChange);
      normals.bottomRows(currentNormals.rows()) = currentNormals;
      vertices.bottomRows(currentVertices.rows()) = currentVertices;
      triangles.bottomRows(currentTriangles.rows()) = currentTriangles;
    }
  }
  return true;
}

bool ZMesh::readWavefrontObj(const char *filename, Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &normals,
                             Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &vertices,
                             Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &triangles)
{
  ifstream inputFileStream;
  inputFileStream.open(filename, ios::in);
  if (!inputFileStream.is_open()) {
    perror((string("readWavefrontObj: error while opening file ") + filename).c_str());
    return false;
  }

  string line;
  int numPoints = 0;
  int numNormals = 0;
  int numTriangles = 0;

  while (getline(inputFileStream, line)) {
    //skip empty lines
    if (line.find_first_not_of(" \t\r") == string::npos) {
      continue;
    }
    line  = line.substr(line.find_first_not_of(" \t\r"));
    if (line[0] == '#')
      continue;
    else if (line[0] == 'v' && line[1] == 'n') {
      numNormals++;
    } else if (line[0] == 'v') {
      numPoints++;
    } else if (line[0] == 'f') {
      numTriangles++;
    }
  }
  if (inputFileStream.bad()) {
    std::perror((string("readWavefrontObj: error while reading file ") + filename).c_str());
    inputFileStream.close();
    return false;
  }
  if (numNormals > 0 && numPoints != numNormals) {
    cerr << "readWavefrontObj: can not read this file (normal number not equal to vertices number)" << endl;
    inputFileStream.close();;
    return false;
  }

  inputFileStream.clear();
  inputFileStream.seekg(0, std::ios::beg);
  normals.resize(numNormals, 3);
  vertices.resize(numPoints, 3);
  triangles.resize(numTriangles, 3);

  int normalIdx = 0;
  int verticeIdx = 0;
  int triangleIdx = 0;

  while (getline(inputFileStream, line)) {
    //skip empty lines
    if (line.find_first_not_of(" \t\r") == string::npos) {
      continue;
    }
    line  = line.substr(line.find_first_not_of(" \t\r"));
    if (line[0] == '#')
      continue;
    else if (line[0] == 'v' && line[1] == 'n') {
      int nData;
      RowVectorXd rowVector = ZEigenUtils::readRowVector(line, "vn", &nData, 3);
      if (nData != 3) {
        cerr << "readWavefrontObj: wrong file content (vn)" << endl;
        inputFileStream.close();;
        return false;
      } else {
        normals.row(normalIdx++) = rowVector.cast<float>();
      }
    } else if (line[0] == 'v') {
      int nData;
      RowVectorXd rowVector = ZEigenUtils::readRowVector(line, "v", &nData, 3);
      if (nData != 3) {
        cerr << "readWavefrontObj: wrong file content (v)" << endl;
        inputFileStream.close();;
        return false;
      } else {
        vertices.row(verticeIdx++) = rowVector.cast<float>();
      }
    } else if (line[0] == 'f') {
      int nData;
      RowVectorXd rowVector = ZEigenUtils::readRowVector(line, "f/", &nData, 6);
      if (nData != 6) {
        cerr << "readWavefrontObj: wrong file content (f)" << endl;
        inputFileStream.close();;
        return false;
      } else {
        triangles.row(triangleIdx++) << (int)rowVector(0), (int)rowVector(2), (int)rowVector(4);
      }
    }
  }
  if (inputFileStream.bad()) {
    std::perror((std::string("readWavefrontObj: error while reading file ") + filename).c_str());
    inputFileStream.close();
    return false;
  }

  inputFileStream.close();
  return true;
}

bool ZMesh::readWavefrontObj(const QStringList &iFiles, Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &normals,
                             Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &vertices,
                             Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &triangles,
                             QList<int> &pointSizes, QList<int> &triangleSizes)
{
  if (iFiles.size() == 0)
    return false;
  pointSizes.clear();
  triangleSizes.clear();
  bool ok;
  for (int i=0; i<iFiles.size(); i++) {
    if (i==0) {
      ok = readWavefrontObj(iFiles[i].toLocal8Bit().constData(), normals, vertices, triangles);
      if (!ok)
        return false;
      pointSizes.push_back(vertices.rows());
      triangleSizes.push_back(triangles.rows());
    } else {
      Matrix<float, Dynamic, Dynamic, RowMajor> currentNormals;
      Matrix<float, Dynamic, Dynamic, RowMajor> currentVertices;
      Matrix<int, Dynamic, Dynamic, RowMajor> currentTriangles;
      ok = readWavefrontObj(iFiles[i].toLocal8Bit().constData(), currentNormals, currentVertices, currentTriangles);
      if (!ok)
        return false;
      pointSizes.push_back(vertices.rows());
      triangleSizes.push_back(currentTriangles.rows());
      int pointStartIdx = vertices.rows();
      currentTriangles.array() += pointStartIdx;
      normals.conservativeResize(normals.rows()+currentNormals.rows(), NoChange);
      vertices.conservativeResize(vertices.rows()+currentVertices.rows(), NoChange);
      triangles.conservativeResize(triangles.rows()+currentTriangles.rows(), NoChange);
      normals.bottomRows(currentNormals.rows()) = currentNormals;
      vertices.bottomRows(currentVertices.rows()) = currentVertices;
      triangles.bottomRows(currentTriangles.rows()) = currentTriangles;
    }
  }
  return true;
}

bool ZMesh::writeWavefrontObj(const char *oFile, const Matrix<float, Dynamic, Dynamic, RowMajor> &normals,
                              const Matrix<float, Dynamic, Dynamic, RowMajor> &vertices,
                              const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &triangles)
{
  std::ofstream outputFileStream(oFile);
  if (!outputFileStream.is_open()) {
    std::perror((std::string("writeWavefrontObj: error while opening file ") + oFile).c_str());
    return false;
  }

  // write vertices
  for (int i=0; i<vertices.rows(); i++) {
    outputFileStream << "v " << vertices.row(i) << std::endl;
  }
  // write normals
  for (int i=0; i<normals.rows(); i++) {
    outputFileStream << "vn " << normals.row(i) << std::endl;
  }
  // write triangles
  for (int i=0; i<triangles.rows(); i++) {
    outputFileStream << "f " << triangles(i,0) << "//" << triangles(i,0) << " "
                     << triangles(i,1) << "//" << triangles(i,1) << " "
                     << triangles(i,2) << "//" << triangles(i,2) << std::endl;
  }
  outputFileStream.close();
  return true;
}

bool ZMesh::writeHxSurface(const char *oFile,
                           const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &vertices,
                           const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &triangles,
                           const QList<int> &triangleSizes)
{
  std::ofstream outputFileStream(oFile);
  if (!outputFileStream.is_open()) {
    std::perror((std::string("writeHxSurface: error while opening file ") + oFile).c_str());
    return false;
  }
  outputFileStream << "# HyperSurface ASCII" << endl << endl;
  outputFileStream << "Parameters {\n    Info \"3 colors\"\n}" << endl << endl;
  outputFileStream << "Materials { {\n    color color 0.83562 0.78 0.06,\n    Name \"Yellow\" }\n{\n    color 0.21622 0.8 0.16,\n    name \"Green\" }\n{\n    color 0.8 0.16 0.596115,\n    name \"Magenta\" }\n}" << endl << endl;

  outputFileStream << "Vertices " << vertices.rows() << endl;
  for (int i=0; i<vertices.rows(); i++) {
    outputFileStream << "    " << vertices.row(i) << endl;
  }
  outputFileStream << endl;
  outputFileStream << "Patches " << triangleSizes.size() << endl;
  int idx = 0;
  for (int i=0; i<triangleSizes.size(); i++) {
    if (i%3 == 0) {
      outputFileStream << "{   InnerRegion Green\n    OuterRegion Yellow\n    Triangles " << triangleSizes[i] << endl;
    } else if (i%3 == 1) {
      outputFileStream << "{   InnerRegion Magenta\n    OuterRegion Yellow\n    Triangles " << triangleSizes[i] << endl;
    } else if (i%3 == 2) {
      outputFileStream << "{   InnerRegion Magenta\n    OuterRegion Green\n    Triangles " << triangleSizes[i] << endl;
    }

    for (int j=0; j<triangleSizes[i]; j++) {
      outputFileStream << "       " << triangles.row(idx++) << endl;
    }

    outputFileStream << "}" << endl;
  }
  return true;
}

bool ZMesh::writeHxSurface(const char *oFile, const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &vertices,
                           const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &triangles)
{
  QList<int> triangleSizes;
  triangleSizes.push_back(triangles.rows());
  return writeHxSurface(oFile, vertices, triangles, triangleSizes);
}

bool ZMesh::convertAllenAtlasMeshToWavefrontObj(const QStringList &iFiles, const QString &oFile)
{
  Matrix<float, Dynamic, Dynamic, RowMajor> normals;
  Matrix<float, Dynamic, Dynamic, RowMajor> vertices;
  Matrix<int, Dynamic, Dynamic, RowMajor> triangles;
  QList<int> pointSizes;
  QList<int> triangleSizes;
  if (!readAllenAtlasMesh(iFiles, normals, vertices, triangles, pointSizes, triangleSizes))
    return false;
  if (!writeWavefrontObj(oFile.toLocal8Bit().constData(), normals, vertices, triangles))
    return false;
  return true;
}

bool ZMesh::convertAllenAtlasMeshToHxSurface(const QStringList &iFiles, const QString &oFile)
{
  Matrix<float, Dynamic, Dynamic, RowMajor> normals;
  Matrix<float, Dynamic, Dynamic, RowMajor> vertices;
  Matrix<int, Dynamic, Dynamic, RowMajor> triangles;
  QList<int> pointSizes;
  QList<int> triangleSizes;
  if (!readAllenAtlasMesh(iFiles, normals, vertices, triangles, pointSizes, triangleSizes))
    return false;
  if (!writeHxSurface(oFile.toLocal8Bit().constData(), vertices, triangles, triangleSizes))
    return false;
  return true;
}
#endif
