#include "zflyemstackframe.h"

#include <QDir>

#include <fstream>

#include "zsegmentationbundle.h"
#include "zflyemstackdoc.h"
#include "zdoublevector.h"
#include "zstackpresenter.h"
#include "zstackview.h"
#include "zsegmentationanalyzer.h"
#include "zopencv_header.h"
#include "neutubeconfig.h"
#include "tz_u8array.h"
#include "zhdf5reader.h"
#include "zhdf5writer.h"
#include "zstring.h"
#include "flyem/zbcfset.h"
#include "zmatrix.h"

using namespace std;

ZFlyEmStackFrame::ZFlyEmStackFrame(QWidget *parent) :
  ZStackFrame(parent, false)
{
  constructFrame();
}


ZFlyEmStackFrame::~ZFlyEmStackFrame()
{

}

void ZFlyEmStackFrame::createDocument()
{
  setDocument(tr1::shared_ptr<ZStackDoc>(new ZFlyEmStackDoc));
}

bool ZFlyEmStackFrame::importSegmentationBundle(const std::string &filePath)
{
  /*
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(filePath);

  //bundle.update();

  ZStack *stack = bundle.deliverGreyScaleStack();
*/
  /*
  if (stack == NULL) {
    return false;
  } else {
    if (stack->channel() == 0) {
      delete stack;
      cout << "Empty stack" << endl;
      return false;
    }
  }
*/
  //m_doc->setStackMask(bundle.createBodyStack());
  //m_doc->loadStack(stack);
  ZFlyEmStackDoc *pDoc = completeDocument();

  pDoc->importSegmentation(filePath);
  //ZGraph *graph = bundle.deliverBodyGraph();
  //ZStack *bodyStack = bundle.deliverBodyStack();
  //pDoc->setBodyGraph(graph);
  //pDoc->setSegmentation(bodyStack);
  //pDoc->loadStack(stack);

  //pDoc->setBodyGraph(bundle.createBodyGraph());

  if (m_doc->hasStackData()) {
    m_presenter->optimizeStackBc();
    m_view->reset();
  }

  return true;
}

bool ZFlyEmStackFrame::importAxonExport(const string &filePath)
{
  ZFlyEmStackDoc *pDoc = completeDocument();

  return pDoc->importAxonExport(filePath);
}

void ZFlyEmStackFrame::importSegmentation(const string &filePath)
{
  completeDocument()->importSegmentation(filePath);
}

ZStack* ZFlyEmStackFrame::spinoffStackSelection(
    const std::vector<std::vector<double> > &selected,
    bool neighborIncluded)
{
  std::vector<std::vector<double> > newSelected = selected;

  if (neighborIncluded) {
    dynamic_cast<ZFlyEmStackDoc*>(m_doc.get())->appendBodyNeighbor(&newSelected);
  }

  return spinoffStackSelection(newSelected);
}

void ZFlyEmStackFrame::copyDocument(ZStackFrame *frame)
{
  document()->loadStack(frame->document()->stack()->clone());
  /*
  document()->setStackMask(
        frame->document()->stackMask()->clone());
        */
}

ZStackFrame* ZFlyEmStackFrame::spinoffSegmentationSelection(
    const std::vector<int> &selected)
{
  if (document()->hasStackMask()) {
    int channelNumber = document()->stackMask()->channelNumber();
    std::vector<std::vector<double> > selectedColor =
        ZDoubleVector::reshape(selected, channelNumber);

    return spinoffSegmentationSelection(selectedColor);
  }

  return NULL;
}

ZStackFrame* ZFlyEmStackFrame::spinoffSegmentationSelection(
    const std::vector<std::vector<double> > &selected)
{
  ZStackFrame *frame = NULL;

  ZFlyEmStackDoc *doc = completeDocument();

  if (doc->hasStackMask()) {
    frame = new ZStackFrame();
#ifdef _DEBUG_
    cout << "Color selected: ";
    ZDoubleVector::print(selected);
#endif
    ZStack *substack = doc->stackMask()->createSubstack(selected);

    frame->document()->loadStack(substack);
    frame->view()->reset();
  }

  return frame;
}

void ZFlyEmStackFrame::selectSegmentaion(const vector<int> &selected,
                                         bool viewUpdate)
{
  for (vector<int>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    ZObject3d *obj = completeDocument()->getBodyBoundaryObject(*iter);
    if (obj != NULL) {
      obj->setColor(255, 255, 0, 255);
      addDecoration(obj->clone());
    } else {
      cout << "No body found for id : " << *iter << endl;
    }
  }

  if (viewUpdate) {
    updateView();
  }
}

void ZFlyEmStackFrame::selectNeighborSegmentation(
    const vector<int> &selected)
{
  for (vector<int>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    vector<ZObject3d*> bodyArray =
        completeDocument()->getNeighborBodyObject(*iter);
    for (vector<ZObject3d*>::const_iterator bodyIter = bodyArray.begin();
         bodyIter != bodyArray.end(); ++bodyIter) {
      if (*bodyIter != NULL) {
        addDecoration((*bodyIter)->clone());
      }
    }
  }

  updateView();
}

void ZFlyEmStackFrame::selectSegmentationPair(int s1, int s2, bool viewUpdate)
{
  ZObject3d* obj1 = completeDocument()->getBodyBoundaryObject(s1);
  if (obj1 == NULL) {
    return;
  }

  obj1->setColor(255, 255, 0, 100);

  ZObject3d* obj2 = completeDocument()->getBodyBoundaryObject(s2);

  if (obj2 == NULL) {
    return;
  }

  obj2->setColor(255, 255, 0, 100);

  ZObject3d* border = completeDocument()->getBodyObjectBorder(s1, s2);

  if (border == NULL) {
    return;
  }

  border->setColor(255, 0, 0, 255);

  addDecoration(border->clone());
  addDecoration(obj1->clone());
  addDecoration(obj2->clone());

  if (viewUpdate) {
    updateView();
  }
}

void ZFlyEmStackFrame::predictSegmentationError()
{
#if defined(_USE_OPENCV_)
  //Calculate connection features
  FlyEm::ZSegmentationBundle *bundle =
      completeDocument()->getSegmentationBundle();

  ZGraph *bodyGraph = bundle->getBodyGraph();

  NeutubeConfig &config = NeutubeConfig::getInstance();
  const vector<string> &featureSet = config.getBodyConnectionFeature();

  for (vector<string>::const_iterator iter = featureSet.begin();
       iter != featureSet.end(); ++iter) {
    FlyEm::ZSegmentationAnalyzer::generateBcf(*bundle, *iter);
  }

  FlyEm::ZBcfSet bcfSet(bundle->getBcfPath());
  ZMatrix *matrix = bcfSet.load(featureSet);

  cv::Mat featureMatrix(
        matrix->getRowNumber(), matrix->getColumnNumber(), CV_32FC1);

  for (int i = 0; i < matrix->getRowNumber(); ++i) {
    for (int j = 0; j < matrix->getColumnNumber(); ++j) {
      featureMatrix.at<float>(i, j) = matrix->getValue(i, j);
    }
  }

  delete matrix;

  //Load classifier
  string classiferPath = config.getPath(NeutubeConfig::FLYEM_BODY_CONN_CLASSIFIER);

  CvRTrees rtree;
  rtree.load(classiferPath.c_str());


  //Predict
  for (int i = 0; i < featureMatrix.rows; ++i) {
    float result = rtree.predict(featureMatrix.row(i), cv::Mat());
    if (result > config.getSegmentationClassifThreshold()) {
      int s1 = bodyGraph->edgeStart(i);
      int s2 = bodyGraph->edgeEnd(i);
      /*
      ZObject3d* obj1 = completeDocument()->getBodyBoundaryObject(s1);
      obj1->setColor(255, 255, 0, 100);
      ZObject3d* obj2 = completeDocument()->getBodyBoundaryObject(s2);
      obj2->setColor(255, 255, 0, 100);
      */
      if (bundle->isImportantBorder(s1, s2)) {
        ZObject3d* border = completeDocument()->getBodyObjectBorder(s1, s2);
        border->setColor(255, 0, 0, 255);

        addDecoration(border->clone());
      }
      //addDecoration(obj1->clone());
      //addDecoration(obj2->clone());
    }
  }

  updateView();
#endif
}

bool ZFlyEmStackFrame::trainBodyConnection()
{
#if defined(_USE_OPENCV_)
  FlyEm::ZSegmentationBundle testSegBundle;
  FlyEm::ZSegmentationBundle trueSegBundle;

  NeutubeConfig &config = NeutubeConfig::getInstance();

  cout << "Loading data ... " << endl;
  testSegBundle.importJsonFile(
        config.getPath(NeutubeConfig::FLYEM_BODY_CONN_TRAIN_DATA));
  trueSegBundle.importJsonFile(
        config.getPath(NeutubeConfig::FLYEM_BODY_CONN_TRAIN_TRUTH));

  cout << "Retrieving features ..." << endl;
  const vector<string> &featureSet = config.getBodyConnectionFeature();

  for (vector<string>::const_iterator iter = featureSet.begin();
       iter != featureSet.end(); ++iter) {
    FlyEm::ZSegmentationAnalyzer::generateBcf(testSegBundle, *iter);
  }


  FlyEm::ZBcfSet bcfSet(testSegBundle.getBcfPath());
  ZMatrix *featureMatrix = bcfSet.load(featureSet);

  int featureNumber = featureMatrix->getColumnNumber();
  int sampleNumber = featureMatrix->getRowNumber();

  cv::Mat trainingData(sampleNumber, featureNumber, CV_32FC1);

  for (int i = 0; i < sampleNumber; i++) {
    for (int j = 0; j < featureNumber; j++) {
      trainingData.at<float>(i, j) = featureMatrix->getValue(i, j);
    }
  }

  delete featureMatrix;


  vector<int> &groundLabel = testSegBundle.getGroundLabel();

  if (groundLabel.empty()) {
    cout << "Computing labels ..." << endl;
    ZStack *testBodyStack = testSegBundle.getBodyStack();
    ZStack *trueBodyStack = trueSegBundle.getBodyStack();

    ZStack *b1 = testSegBundle.getBodyBoundaryStack();
    ZStack *b2 = trueSegBundle.getBodyBoundaryStack();

    uint8_t *array1 = b1->array8(0);
    uint8_t *array2 = b2->array8(0);

    size_t volume = b1->getVoxelNumber();

    u8array_max2(array1, array2, volume);

    ZIntMap testBodySize =
        FlyEm::ZSegmentationAnalyzer::computeBodySize(testBodyStack,
                                                      testSegBundle.getBodyBoundaryStack());

    ZIntPairMap overlap = FlyEm::ZSegmentationAnalyzer::computeOverlap(
          testBodyStack, trueBodyStack, b1);
    ZIntMap bodyCorrespondence =
        FlyEm::ZSegmentationAnalyzer::inferBodyCorrespondence(
          overlap, testBodySize);

    groundLabel.resize(sampleNumber, 0);

    ZGraph *bodyGraph = testSegBundle.getBodyGraph();

    for (int i = 0; i < sampleNumber; i++) {
      int id1 = bodyGraph->edgeStart(i);
      int id2 = bodyGraph->edgeEnd(i);

      if (bodyCorrespondence[id1] == bodyCorrespondence[id2]) {
        groundLabel[i] = 1;
      } else {
        groundLabel[i] = 0;
      }
    }

    testSegBundle.saveGroundLabel();
  }

  cv::Mat trainingLabel(sampleNumber, 1, CV_32FC1);
  for (int i = 0; i < sampleNumber; i++) {
    trainingLabel.at<float>(i) = groundLabel[i];
  }

  cout << "Training classifier ..." << endl;

  std::vector<float> priors(featureNumber, 1.0);

  CvRTParams params = CvRTParams(5, // max depth
                                 2, // min sample count
                                 0, // regression accuracy: N/A here
                                 false, // compute surrogate split, no missing data
                                 2, // max number of categories (use sub-optimal algorithm for larger numbers)
                                 &(priors[0]), // the array of priors
                                 true,  // calculate variable importance
                                 2,      // number of variables randomly selected at node and used to find the best split(s).
                                 5,	 // max number of trees in the forest
                                 0.01f,				// forrest accuracy
                                 CV_TERMCRIT_EPS // termination cirteria
                                 );

  cv::Mat var_type = cv::Mat(featureNumber + 1, 1, CV_8U);
  var_type.setTo(cv::Scalar(CV_VAR_NUMERICAL) );

  CvRTrees rtree;
  rtree.train(trainingData, CV_ROW_SAMPLE, trainingLabel,
              cv::Mat(), cv::Mat(), var_type, cv::Mat(), params);


  rtree.save(config.getPath(NeutubeConfig::FLYEM_BODY_CONN_CLASSIFIER).c_str());
  cout << "done" << endl;

  vector<double> threshold(3);
  threshold[0] = 0.1;
  threshold[1] = 0.3;
  threshold[2] = 0.5;

  evaluateBodyConnectionClassifier(threshold);
#endif

  return true;
}

bool ZFlyEmStackFrame::evaluateBodyConnectionClassifier(
    const std::vector<double> &threshold)
{
#if defined(_USE_OPENCV_)
  FlyEm::ZSegmentationBundle testSegBundle;
  FlyEm::ZSegmentationBundle trueSegBundle;

  NeutubeConfig &config = NeutubeConfig::getInstance();

  cout << "Evaluating classifier ..." << endl;
  cout << "  Loading data ... " << endl;
  testSegBundle.importJsonFile(
        config.getPath(NeutubeConfig::FLYEM_BODY_CONN_EVAL_DATA));
  trueSegBundle.importJsonFile(
        config.getPath(NeutubeConfig::FLYEM_BODY_CONN_EVAL_TRUTH));

  cout << "  Retrieving features ..." << endl;
  const vector<string> &featureSet = config.getBodyConnectionFeature();

  for (vector<string>::const_iterator iter = featureSet.begin();
       iter != featureSet.end(); ++iter) {
    FlyEm::ZSegmentationAnalyzer::generateBcf(testSegBundle, *iter);
  }

  FlyEm::ZBcfSet bcfSet(testSegBundle.getBcfPath());
  ZMatrix *matrix = bcfSet.load(featureSet);

  cv::Mat featureMatrix(
        matrix->getRowNumber(), matrix->getColumnNumber(), CV_32FC1);

  for (int i = 0; i < matrix->getRowNumber(); ++i) {
    for (int j = 0; j < matrix->getColumnNumber(); ++j) {
      featureMatrix.at<float>(i, j) = matrix->getValue(i, j);
    }
  }

  int sampleNumber = matrix->getRowNumber();

  delete matrix;

  string classiferPath = config.getPath(NeutubeConfig::FLYEM_BODY_CONN_CLASSIFIER);
  CvRTrees rtree;
  rtree.load(classiferPath.c_str());

  vector<int> &groundLabel = testSegBundle.getGroundLabel();

  if (groundLabel.empty()) {
    cout << "  Computing labels ..." << endl;
    ZStack *testBodyStack = testSegBundle.getBodyStack();
    ZStack *trueBodyStack = trueSegBundle.getBodyStack();

    ZStack *b1 = testSegBundle.getBodyBoundaryStack();
    ZStack *b2 = trueSegBundle.getBodyBoundaryStack();

    uint8_t *array1 = b1->array8(0);
    uint8_t *array2 = b2->array8(0);

    size_t volume = b1->getVoxelNumber();

    u8array_max2(array1, array2, volume);

    ZIntMap testBodySize =
        FlyEm::ZSegmentationAnalyzer::computeBodySize(testBodyStack,
                                                      testSegBundle.getBodyBoundaryStack());

    ZIntPairMap overlap = FlyEm::ZSegmentationAnalyzer::computeOverlap(
          testBodyStack, trueBodyStack, b1);
    ZIntMap bodyCorrespondence =
        FlyEm::ZSegmentationAnalyzer::inferBodyCorrespondence(
          overlap, testBodySize);

    ZGraph *bodyGraph = testSegBundle.getBodyGraph();
    groundLabel.resize(bodyGraph->size(), 0);

    for (int i = 0; i < sampleNumber; ++i) {
      int id1 = bodyGraph->edgeStart(i);
      int id2 = bodyGraph->edgeEnd(i);

      if (bodyCorrespondence[id1] == bodyCorrespondence[id2]) { //real connection
        groundLabel[i] = 1;
      }
    }

    testSegBundle.saveGroundLabel();
  }

  for (vector<double>::const_iterator iter = threshold.begin();
       iter != threshold.end(); ++iter) {
    int tp = 0;
    int fp = 0;
    int fn = 0;
    int evalSampleNumber = 0;

    for (int i = 0; i < sampleNumber; ++i) {
      float result = rtree.predict(featureMatrix.row(i), cv::Mat());

      /*
      ZGraph *bodyGraph = testSegBundle.getBodyGraph();
      int id1 = bodyGraph->edgeStart(i);
      int id2 = bodyGraph->edgeEnd(i);

      if (testSegBundle.isImportantBorder(id1, id2)) {
      */
        ++evalSampleNumber;

        if (groundLabel[i] == 1) { //real connection
          if (result > *iter) {
            ++tp;
          } else {
            ++fn;
          }
        } else {
          if (result > *iter) {
            ++fp;
          }
        }
      //}
    }

    cout << "Number of samples: " << evalSampleNumber << endl;

    double precision = (double) tp / (tp + fn);
    double recall = (double) tp / (tp + fp);
    cout << "Recall: " << tp << " / " << tp + fn << " = "
         << precision << endl;
    cout << "Precision: " << tp << " / " << tp + fp << " = "
         << recall << endl;
    cout << "F-measure: " << 2.0 * precision * recall / (precision + recall) << endl;
  }

  cout << "done" << endl;
#endif

  return true;
}

void ZFlyEmStackFrame::computeBodyConnFeature()
{
  NeutubeConfig &config = NeutubeConfig::getInstance();
  const vector<string> &featureSet = config.getBodyConnectionFeature();

  for (size_t i = 0; i < featureSet.size(); ++i) {
    FlyEm::ZSegmentationBundle *bundle =
        completeDocument()->getSegmentationBundle();
    FlyEm::ZSegmentationAnalyzer::generateBcf(*bundle, featureSet[i]);
  }
}

