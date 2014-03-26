#include "zdendrogram.h"

#include "tz_utilities.h"
#include "ztree.h"
#include "zstring.h"
#include "tz_math.h"
#include "zsvggenerator.h"

using namespace std;

ZDendrogram::ZDendrogram()
{
}

void ZDendrogram::addLink(int first, int second, double distance)
{
  push_back(DendrogramLink(first, second, distance));
}

void ZDendrogram::setLeafName(const vector<string> &nameArray)
{
  m_leafNameArray = nameArray;
}

void ZDendrogram::loadLeafName(const string filePath)
{
  m_leafNameArray.clear();

  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");

  while (str.readLine(fp)) {
    if (!str.empty()) {
      m_leafNameArray.push_back(str);
    }
  }

  fclose(fp);
}

void ZDendrogram::loadLeafLink(const string filePath)
{
  m_leafLinkArray.clear();

  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");

  while (str.readLine(fp)) {
    if (!str.empty()) {
      m_leafLinkArray.push_back(str);
    }
  }

  fclose(fp);
}

void ZDendrogram::importCsv(string filePath)
{
  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");

  while (str.readLine(fp)) {
    vector<double> entry = str.toDoubleArray();
    if (entry.size() == 3) {
      if (entry[0] > entry[1]) {
        double tmp;
        SWAP2(entry[0], entry[1], tmp);
      }
      addLink(iround(entry[0]), iround(entry[1]), entry[2]);
    }
  }

  fclose(fp);
}

double ZDendrogram::maxDist()
{
  double max_dist = (*this)[0].distance;
  for (size_t i = 1; i < size(); i++) {
    if (max_dist < (*this)[i].distance) {
      max_dist = (*this)[i].distance;
    }
  }

  return max_dist;
}

double ZDendrogram::minDist()
{
  double min_dist = (*this)[0].distance;
  for (size_t i = 1; i < size(); i++) {
    if (min_dist > (*this)[i].distance) {
      min_dist = (*this)[i].distance;
    }
  }

  return min_dist;
}


double ZDendrogram::clusterDistance(int clusterIndex)
{
  if (clusterIndex <= leafNumber()) {
    return 0.0;
  }

  return (*this)[clusterIndex - leafNumber() - 1].distance;
}

bool ZDendrogram::isLeftChild(int node)
{
  return (leftChild(parent(node)) == node);
}

bool ZDendrogram::isRightChild(int node)
{
  return (rightChild(parent(node)) == node);
}

bool ZDendrogram::isRoot(int node)
{
  return (node == nodeNumber());
}

bool ZDendrogram::isLeaf(int node)
{
  return (leftChild(node) == 0);
}

vector<int> ZDendrogram::sortLeaf()
{
  vector<int> leafArray;
  vector<bool> nodeChecked(nodeNumber() + 1, false);

  int root = nodeNumber();
  bool nodeAvailable = true;
  int currentNode = leftChild(root);

  while (nodeAvailable) {
    if (nodeChecked[currentNode]) {
      if (isLeftChild(currentNode)) {
        currentNode = rightChild(parent(currentNode));
      } else {
        currentNode = parent(currentNode);
        nodeChecked[currentNode] = true;
      }
    } else {
      if (isLeaf(currentNode)) {
        leafArray.push_back(currentNode);
        nodeChecked[currentNode] = true;
      } else {
        currentNode = leftChild(currentNode);
      }
    }

    if (nodeChecked[root]) {
      nodeAvailable = false;
    }
  }

  return leafArray;
}

string ZDendrogram::toSvgString(double interval)
{
  string str;

  updateTreeStructure();

  vector<int> leafArray = sortLeaf();

  double margin = 10.0;
  int width = 800;

  vector<double> y(nodeNumber() + 1, 0.0);
  vector<double> x(nodeNumber() + 1, width - margin);

  double xScale = (width - margin * 2.0) / maxDist();

  x[0] = 0;
  y[0] = 0;
  double currentY = margin;

  int nodeIndex;

  for (size_t i = 0; i < leafArray.size(); i++) {
    nodeIndex = leafArray[i];
    y[nodeIndex] = currentY + interval;
    currentY = y[nodeIndex];
  }


  for (size_t i = 0; i < size(); i++) {
    nodeIndex = (*this)[i].first;
    x[nodeIndex] -= clusterDistance(nodeIndex) * xScale;
    if (nodeIndex > leafNumber()) {
      y[nodeIndex] = (y[m_leftChildList[nodeIndex]] +
                      y[m_rightChildList[nodeIndex]]) / 2.0;
    }

    nodeIndex = (*this)[i].second;
    x[nodeIndex] -= clusterDistance(nodeIndex) * xScale;
    if (nodeIndex > leafNumber()) {
      y[nodeIndex] = (y[m_leftChildList[nodeIndex]] +
                      y[m_rightChildList[nodeIndex]]) / 2.0;
    }
  }

  nodeIndex = nodeNumber();
  x[nodeIndex] -= clusterDistance(nodeIndex) * xScale;
  y[nodeIndex] = (y[m_leftChildList[nodeIndex]] +
                  y[m_rightChildList[nodeIndex]]) / 2.0;

  /*
  for (int i = 1; i <= nodeNumber(); i++) {
    x[i] -= clusterDistance(i) * xScale;
    if (i <= leafNumber()) {
      y[i] = y[i - 1] + interval;
    } else {
      y[i] = (y[m_leftChildList[i]] + y[m_rightChildList[i]]) / 2.0;
    }
  }
  */

  for (int i = 1; i <= nodeNumber(); i++) {
    str += ZSvgGenerator::lineTag(x[i], y[i], x[parent(i)], y[i],
                                  "style=\"stroke:rgb(255, 0, 0)\"") + "\n";
    if (i > leafNumber()) {
      str += ZSvgGenerator::lineTag(x[i], y[leftChild(i)],
                                    x[i], y[rightChild(i)],
                                    "style=\"stroke:rgb(255, 0, 0)\"") + "\n";
    }
  }

  if (!m_leafNameArray.empty()) {
    for (int i = 0; i < leafNumber(); i++) {
      int nodeIndex = i + 1;
      if (m_leafLinkArray.size() > (size_t) i) {
        str += ZSvgGenerator::hyperLinkTag(
              m_leafLinkArray[i],
              "target=\"_blank\"");
      }

      str += ZSvgGenerator::textTag(x[nodeIndex], y[nodeIndex] + interval / 5.0,
                                    m_leafNameArray[i]) + "\n";

      if (m_leafLinkArray.size() > (size_t) i) {
        str += ZSvgGenerator::hyperLinkTag();
      }
    }
  }

  return str;
}

int ZDendrogram::leafNumber()
{
  return size() + 1;
}

int ZDendrogram::nodeNumber()
{
  return size() + leafNumber();
}

void ZDendrogram::updateTreeStructure()
{
  m_parentList.resize(nodeNumber() + 1, 0);
  m_leftChildList.resize(nodeNumber() + 1, 0);
  m_rightChildList.resize(nodeNumber() + 1, 0);

  int parent = leafNumber() + 1;
  for (size_t i = 0; i < size(); i++) {
    m_parentList[(*this)[i].first] = parent;
    m_parentList[(*this)[i].second] = parent;
    m_leftChildList[parent] = (*this)[i].first;
    m_rightChildList[parent] = (*this)[i].second;
    parent++;
  }
}
