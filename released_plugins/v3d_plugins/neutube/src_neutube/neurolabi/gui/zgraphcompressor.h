#ifndef ZGRAPHCOMPRESSOR_H
#define ZGRAPHCOMPRESSOR_H

#include <vector>

class ZGraph;

class ZGraphCompressor
{
public:
  ZGraphCompressor();

  void compress();
  void uncompress();
  int uncompress(int v);

  void setGraph(ZGraph *graph);

  inline const std::vector<int>& getCompressMap() {
    return m_compressMap;
  }

  inline const std::vector<int>& getUncompressMap() {
    return m_uncompressMap;
  }

private:
  ZGraph *m_graph;
  std::vector<int> m_compressMap;
  std::vector<int> m_uncompressMap;
};

#endif // ZGRAPHCOMPRESSOR_H
