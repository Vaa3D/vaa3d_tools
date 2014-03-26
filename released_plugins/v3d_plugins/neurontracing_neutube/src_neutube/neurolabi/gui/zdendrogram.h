#ifndef ZDENDROGRAM_H
#define ZDENDROGRAM_H

#include <vector>
#include <string>

struct DendrogramLink {
  int first;
  int second;
  double distance;

  DendrogramLink(int n1, int n2, double d)
     { first = n1; second = n2; distance = d; }
};

class ZDendrogram : public std::vector<DendrogramLink>
{
public:
  ZDendrogram();

public:
  void setLeafName(const std::vector<std::string> &nameArray);
  void loadLeafName(const std::string filePath);
  void loadLeafLink(const std::string filePath);

  void importCsv(std::string filePath);

  void addLink(int first, int second, double distance);

  std::string toSvgString(double interval);

  void updateTreeStructure();

  int leafNumber();
  int nodeNumber();
  double distance(int clusterIndex);

  inline int parent(int i) { return m_parentList[i]; }
  inline int leftChild(int i) { return m_leftChildList[i]; }
  inline int rightChild(int i) { return m_rightChildList[i]; }

  double maxDist();
  double minDist();
  double clusterDistance(int clusterIndex);

  std::vector<int> sortLeaf();
  bool isLeftChild(int node);
  bool isRightChild(int node);
  bool isRoot(int node);
  bool isLeaf(int node);

private:
  std::vector<int> m_parentList;
  std::vector<int> m_leftChildList;
  std::vector<int> m_rightChildList;
  std::vector<double> m_distList;
  std::vector<std::string> m_leafNameArray;
  std::vector<std::string> m_leafLinkArray;
};

#endif // ZDENDROGRAM_H
