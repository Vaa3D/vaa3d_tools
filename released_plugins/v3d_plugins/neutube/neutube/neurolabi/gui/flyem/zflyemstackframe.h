#ifndef ZFLYEMSTACKFRAME_H
#define ZFLYEMSTACKFRAME_H

#include <string>
#include <vector>
#include <set>

#include "zstackframe.h"

class ZFlyEmStackDoc;

class ZFlyEmStackFrame : public ZStackFrame
{
  Q_OBJECT
public:
  explicit ZFlyEmStackFrame(QWidget *parent = 0);
  virtual ~ZFlyEmStackFrame();
  
  inline virtual std::string name() { return "flyem"; }

public:
  virtual void createDocument();
  void copyDocument(ZStackFrame *frame);

  inline ZFlyEmStackDoc* completeDocument() {
    return (ZFlyEmStackDoc*) m_doc.get();
  }

  bool importSegmentationBundle(const std::string &filePath);
  void importSegmentation(const std::string &filePath);
  bool importAxonExport(const std::string &filePath);

  ZStack* spinoffStackSelection(
      const std::vector<std::vector<double> > &selected,
      bool neighborIncluded = false);

  ZStackFrame* spinoffSegmentationSelection(
      const std::vector<std::vector<double> > &selected);

  ZStackFrame* spinoffSegmentationSelection(
      const std::vector<int> &selected);

  void selectSegmentaion(const std::vector<int> &selected,
                         bool viewUpdate = true);
  void selectNeighborSegmentation(const std::vector<int> &selected);

  void selectSegmentationPair(int s1, int s2, bool viewUpdate = true);

  virtual inline std::string classType() const { return "flyem"; }

  void predictSegmentationError();
  static bool trainBodyConnection();
  static bool evaluateBodyConnectionClassifier(
      const std::vector<double> &threshold);
  void computeBodyConnFeature();

signals:
  
public slots:

};

#endif // ZFLYEMSTACKFRAME_H
