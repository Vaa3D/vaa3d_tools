#include "zflyemdataframe.h"

#include <QMessageBox>
#include <QApplication>
#include <QMap>
#include <QMainWindow>
#include <QStatusBar>
#include <iostream>
#include <sstream>
#include <map>
#include "flyemdataform.h"
#include "informationdialog.h"
#include "parameterdialog.h"
#include "zstring.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "z3dwindow.h"
#include "z3dswcfilter.h"
#include "tz_darray.h"
#include "zswctree.h"
#include "zswcdisttrunkanalyzer.h"
#include "zswclayerfeatureanalyzer.h"
#include "zswcshollfeatureanalyzer.h"
#include "tz_error.h"
#include "zswclayershollfeatureanalyzer.h"
#include "zswclayertrunkanalyzer.h"
#include "neutubeconfig.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zswcobjsmodel.h"
#include "zpunctaobjsmodel.h"
#include "zflyemneuronpresenter.h"
#include "zswcnodebufferfeatureanalyzer.h"
#include "mainwindow.h"
#include <fstream>
#include "swc/zswcterminalsurfacemetric.h"
#include "flyemgeosearchdialog.h"
#include "flyemgeofilterdialog.h"
#include "flyem/zflyemneuronfilter.h"

using namespace std;

ZFlyEmDataFrame::ZFlyEmDataFrame(QWidget *parent) :
  QMdiSubWindow(parent)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);


  FlyEmDataForm *form = new FlyEmDataForm;
  m_centralWidget = form;
  form->setSizePolicy(QSizePolicy::Expanding,
                      QSizePolicy::Expanding);
  QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(parent);

  if (mainWindow != NULL) {
    form->setStatusBar(mainWindow->statusBar());
  }

  setWidget(form);
  connect(form, SIGNAL(showSummaryTriggered()), this, SLOT(showSummary()));
  connect(form, SIGNAL(saveBundleTriggered(int,QString)),
          this, SLOT(saveBundle(int, QString)));
  connect(form, SIGNAL(queryTriggered()), this, SLOT(query()));
  connect(form, SIGNAL(processTriggered()), this, SLOT(process()));
  connect(form, SIGNAL(testTriggered()), this, SLOT(test()));
  //connect(form, SIGNAL(generalTriggered()), this, SLOT(generalProcess()));
  connect(form, SIGNAL(optionTriggered()), this, SLOT(setParameter()));
  connect(form, SIGNAL(volumeTriggered(const QString&)),
          this, SLOT(openVolume(const QString&)));
  connect(form, SIGNAL(showNearbyNeuronTriggered(const ZFlyEmNeuron*)), this,
          SLOT(showNearbyNeuron(const ZFlyEmNeuron*)));

#ifdef _DEBUG_2
  QSize size = form->sizeHint();
  std::cout << size.width() << " x " << size.height() << std::endl;
#endif

  //adjustSize();

  //Setup configuration
  /*
  m_trunkAnalyzer = new ZSwcDistTrunkAnalyzer;
  ZSwcLayerFeatureAnalyzer *analyzer = new ZSwcLayerFeatureAnalyzer;
  analyzer->setLayerScale(4000.0);
  m_featureAnalyzer = dynamic_cast<ZSwcFeatureAnalyzer*>(analyzer);
*/

  ZSwcLayerTrunkAnalyzer *trunkAnalyzer = new ZSwcLayerTrunkAnalyzer;
  trunkAnalyzer->setStep(200.0);
  m_trunkAnalyzer = dynamic_cast<ZSwcTrunkAnalyzer*>(trunkAnalyzer);
  ZSwcLayerShollFeatureAnalyzer *helperAnalyzer = new ZSwcLayerShollFeatureAnalyzer;
  helperAnalyzer->setLayerScale(4000.0);
  helperAnalyzer->setLayerMargin(100.0);
  m_helperFeatureAnalyzer = helperAnalyzer;

  ZSwcNodeBufferFeatureAnalyzer *analyzer = new ZSwcNodeBufferFeatureAnalyzer;
  analyzer->setHelper(m_helperFeatureAnalyzer);

  m_featureAnalyzer = dynamic_cast<ZSwcFeatureAnalyzer*>(analyzer);

  /*
  ZSwcShollFeatureAnalyzer analyzer;
  analyzer.setShollRadius(200.0);
  analyzer.setShollEnd(2000.0);
  analyzer.setShollStart(200.0);
*/
  m_matcher.setTrunkAnalyzer(m_trunkAnalyzer);
  m_matcher.setFeatureAnalyzer(m_featureAnalyzer);
  //m_resampleStep = 400.0;
  m_resampleStep = 200.0;
  m_matchingLevel = 2;
  m_checkOrientation = true;

  m_infoPresenter = new ZFlyEmNeuronInfoPresenter(this);
  m_featurePresenter = new ZFlyEmNeuronFeaturePresenter(this);
  m_connectionPresenter = new ZFlyEmNeuronConnectionPresenter(this);
  m_volumePresenter = new ZFlyEmNeuronVolumePresenter(this);
  m_topMatchPresenter = new ZFlyEmNeuronTopMatchPresenter(this);

  m_specialProgressReporter.setProgressBar(getProgressBar());
  setProgressReporter(&m_specialProgressReporter);

  m_geoSearchDlg = new FlyEmGeoFilterDialog(this);
}

ZFlyEmDataFrame::~ZFlyEmDataFrame()
{
  delete m_trunkAnalyzer;
  delete m_featureAnalyzer;

  clearData();
}

void ZFlyEmDataFrame::clearData()
{
  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    delete data;
  }
}

void ZFlyEmDataFrame::addData(ZFlyEmDataBundle *data)
{
  if (data != NULL) {
    //Set source ID of the data
    int sourceId = m_dataArray.size();
    std::vector<ZFlyEmNeuron> &neuronArray = data->getNeuronArray();
    for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
         iter != neuronArray.end(); ++iter) {
      iter->setSourceId(sourceId);
    }

    m_dataArray.append(data);
  }
}

bool ZFlyEmDataFrame::load(const std::string &filePath, bool appending)
{
  ZFlyEmDataBundle *data = new ZFlyEmDataBundle;

  if (data->loadJsonFile(filePath)) {
    if (!appending) {
      clearData();
    }

    addData(data);
    return true;
  }

  return false;
}

string ZFlyEmDataFrame::getInformationText() const
{
  ostringstream stream;
  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    stream << data->toSummaryString() << endl;
    stream << data->toDetailString() << endl;
  }

  return stream.str();
}
/*
static size_t get_index(map<int, size_t> &bodyMap, int bodyId)
{
  if (bodyMap.count(bodyId) == 0) {
    size_t lastIndex = bodyMap.size();
    bodyMap[bodyId] = lastIndex;

    return lastIndex;
  }

  return bodyMap[bodyId];
}
*/

FlyEm::ZSynapseAnnotationArray *ZFlyEmDataFrame::getSynapseAnnotation()
{
  FlyEm::ZSynapseAnnotationArray *annotation = NULL;
  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    annotation = data->getSynapseAnnotation();
    if (annotation != NULL) {
      break;
    }
  }

  return annotation;
}

std::string ZFlyEmDataFrame::getName(int bodyId) const
{
  std::string name;
  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    name = data->getName(bodyId);
    if (!name.empty()) {
      break;
    }
  }

  return name;
}

std::string ZFlyEmDataFrame::getName(int bodyId, int bundleIndex) const
{
  if (bundleIndex < 0) {
    return getName(bodyId);
  }

  return m_dataArray[bundleIndex]->getName(bodyId);
}

std::string ZFlyEmDataFrame::getName(const std::pair<int, int> &bodyId) const
{
  return getName(bodyId.first, bodyId.second);
}

void ZFlyEmDataFrame::updatePresenter(EDataForm target)
{
  switch (target) {
  case SUMMARY:
    getMainWidget()->setPresenter(m_infoPresenter);
    break;
  case FEATURE:
    getMainWidget()->setPresenter(m_featurePresenter);
    break;
  case CONNECTION:
    getMainWidget()->setPresenter(m_connectionPresenter);
    break;
  case VOLUME:
    getMainWidget()->setPresenter(m_volumePresenter);
    break;
  case TOP_MATCH:
    getMainWidget()->setPresenter(m_topMatchPresenter);
    break;
  default:
    break;
  }
}

int ZFlyEmDataFrame::updateQuery(
    int id, int bundleIndex, EDataForm target, bool appending)
{
  ostringstream stream;

  const ZFlyEmNeuron *neuron = NULL;

  int minBundleIndex = 0;
  int maxBundleIndex = m_dataArray.size() - 1;

  if (bundleIndex >= 0) {
    minBundleIndex = bundleIndex;
    maxBundleIndex = bundleIndex;
  }

  int count = 0;

  for (int bundleIndex = minBundleIndex; bundleIndex <= maxBundleIndex;
       ++bundleIndex) {
    neuron = getNeuron(id, bundleIndex);

    if (neuron == NULL) {
      stream << id << ": no record" << endl;
    } else {
      updatePresenter(target);

      switch (target) {
      case NAME:
      case ID:
        //stream << id << ": \"" << neuron->getName() << "\"" << endl;
        //break;
      case CLASS:
      case SUMMARY:
      case FEATURE:
      case CONNECTION:
      case VOLUME:
      case TOP_MATCH:
        //neuron->printJson(&stream, 0);
        displayQueryOutput(neuron, appending);
        ++count;
        break;
      default:
        break;
      }
    }
  }

  return count;
}

void ZFlyEmDataFrame::parseCommand(
    const string &sourceType, const string &sourceValue, const string &action)
{
  updateSource(sourceType, sourceValue, false);

  std::string actionStr = action;
  std::transform(actionStr.begin(), actionStr.end(), actionStr.begin(),
                 ::tolower);

  if (actionStr == "match") {
    m_action = MATCH;
  } else if (actionStr == "sort_shape" || actionStr == "sort shape") {
    m_action = SORT_SHAPE;
    m_target = TOP_MATCH;
  } else if (actionStr == "predict_class" || actionStr == "predict class") {
    m_action = PREDICT_CLASS;
    m_target = TOP_MATCH;
  } else if (actionStr == "predict_error" || actionStr == "predict erro") {
    m_action = PREDICT_ERROR;
  } else {
    m_action = UNKNOWN_ACTION;
  }
}

void ZFlyEmDataFrame::updateSource(
    const string &sourceType, const string &sourceValue, bool usingRegexp)
{
  m_sourceIdArray.clear();
  std::string source = sourceType;
  std::transform(source.begin(), source.end(), source.begin(), ::tolower);
  if (usingRegexp) {
    QRegExp regexp(sourceValue.c_str());
    if (source == "class") {
      int bundleIndex = 0;
      foreach (ZFlyEmDataBundle *data, m_dataArray) {
        for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
             data->getNeuronArray().begin();
             neuronIter != data->getNeuronArray().end();
             ++neuronIter) {
          if (!neuronIter->getClass().empty()) {
            if (regexp.exactMatch(neuronIter->getClass().c_str())) {
              m_sourceIdArray.push_back(pair<int, int>(neuronIter->getId(),
                                                       bundleIndex));
            }
          }
        }
      }
    }
  } else {
    ZString queryString = sourceValue;
    if (source == "id") {
      vector<int> idArray = queryString.toIntegerArray();
      m_sourceIdArray.clear();

      for (size_t i = 0; i < idArray.size(); ++i) {
        if (idArray[i] >= 0) {
          m_sourceIdArray.push_back(pair<int, int>(idArray[i], -1));
        } else {
          if (i > 0) {
            int bundleIndex = -idArray[i] - 1;
            if (idArray[i - 1] >= 0) {
              m_sourceIdArray.back().second = bundleIndex;
            }
          }
        }
#if 0
        dump(QString("%1 %2").arg(m_sourceIdArray.back().first).
             arg(m_sourceIdArray.back().second));
#endif
      }

      //m_sourceIdArray = queryString.toIntegerArray();
    } else if (source == "name") {
      vector<string> queryWordArray = queryString.toWordArray(", =>;:\n");
      vector<string>::const_iterator nameIter = queryWordArray.begin();
      for (; nameIter != queryWordArray.end(); ++nameIter) {
        int bundleIndex = 0;
        foreach (ZFlyEmDataBundle *data, m_dataArray) {
          for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
               data->getNeuronArray().begin();
               neuronIter != data->getNeuronArray().end();
               ++neuronIter) {
            if (neuronIter->hasSimilarName(*nameIter)) {
              m_sourceIdArray.push_back(pair<int, int>(neuronIter->getId(),
                                                       bundleIndex));
            }
          }
          ++bundleIndex;
        }
      }
    } else if (source == "class") {
      vector<string> queryWordArray = queryString.toWordArray(", =>;:\n");
      vector<string>::const_iterator nameIter = queryWordArray.begin();
      for (; nameIter != queryWordArray.end(); ++nameIter) {
        int bundleIndex = 0;
        foreach (ZFlyEmDataBundle *data, m_dataArray) {
          for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
               data->getNeuronArray().begin();
               neuronIter != data->getNeuronArray().end();
               ++neuronIter) {
            if (*nameIter == "?") {
              if (neuronIter->getClass().empty()) {
                m_sourceIdArray.push_back(pair<int, int>(neuronIter->getId(),
                                                         bundleIndex));
              }
            } else {
              ZString className = *nameIter;
              if (className.startsWith("unknown")) {
                if (className.size() > 7) {
                  if (className[7] == '^') {
                    className[7] = ' ';
                  }
                }
              };

              if (neuronIter->getClass() == className) {
                m_sourceIdArray.push_back(pair<int, int>(neuronIter->getId(),
                                                         bundleIndex));
              }
            }
          }
          ++bundleIndex;
        }
      }
    } else if (source == "all") {
      int bundleIndex = 0;
      foreach (ZFlyEmDataBundle *data, m_dataArray) {
        for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
             data->getNeuronArray().begin();
             neuronIter != data->getNeuronArray().end();
             ++neuronIter) {
          m_sourceIdArray.push_back(pair<int, int>(neuronIter->getId(),
                                                   bundleIndex));
        }

        ++bundleIndex;
      }
    }
  }

  if (m_sourceIdArray.empty()) {
    dump(QString("No neuron found").arg(m_sourceIdArray.size()));
  }
}

void ZFlyEmDataFrame::parseQuery(
    const string &sourceType, const string &sourceValue, const string &targetType,
    bool usingRegexp)
{
  updateSource(sourceType, sourceValue, usingRegexp);

  std::string target = targetType;
  std::transform(target.begin(), target.end(), target.begin(), ::tolower);

  if (target == "model") {
    m_target = MODEL;
  } else if (target == "id") {
    m_target = ID;
  } else if (target == "name") {
    m_target = NAME;
  } else if (target == "model") {
    m_target = MODEL;
  } else if (target == "summary") {
    m_target = SUMMARY;
  } else if (target == "connection") {
    m_target = CONNECTION;
  } else if (target == "class") {
    m_target = CLASS;
  } else if (target == "feature") {
    m_target = FEATURE;
  } else if (target == "volume") {
    m_target = VOLUME;
  } else if (target == "top matches") {
    m_target = TOP_MATCH;
  } else if (target == "connection model")  {
    m_target = CONNECTION_MODEL;
  } else {
    m_target = UNKNOWN_DATA_FORM;
/*
    if (target == "match") {
      m_action = MATCH;
    } else if (target == "sort_shape") {
      m_action = SORT_SHAPE;
    } else if (target == "predict_class") {
      m_action = PREDICT_CLASS;
      m_target = TOP_MATCH;
    } else {
      m_action = UNKNOWN_ACTION;
    }
    */
  }
}

void ZFlyEmDataFrame::query()
{
  //ParameterDialog dlg;
  //dlg.setWindowTitle("query");

  if (m_queryDialog.exec()) {
    //parseCommand(m_queryDialog.getQueryString().toStdString());

    parseQuery(m_queryDialog.getSourceType().toStdString(),
               m_queryDialog.getSourceValue().toStdString(),
               m_queryDialog.getTarget().toStdString(),
               m_queryDialog.usingRegularExpression());

    //parseCommand(dlg.parameter().toStdString());

    int count = 0;

    switch (m_target) {
    case MODEL:
      showModel();
      break;
    case CONNECTION_MODEL:
      showConnection();
      break;
    case NAME:
    case ID:
    case SUMMARY:
    case CONNECTION:
    case CLASS:
    case FEATURE:
    case VOLUME:
    case TOP_MATCH:
      clearQueryOutput();
      for (vector<pair<int, int> >::const_iterator iter = m_sourceIdArray.begin();
           iter != m_sourceIdArray.end(); ++iter) {
        count += updateQuery((*iter).first, (*iter).second, m_target, true);
      }
      break;
    default:
      break;
    }
    if (count == 1) {
      dump("1 neuron found");
    } else if (count > 1) {
      dump(QString("%1 neurons found").arg(count));
    }
  }
}

const ZFlyEmNeuron* ZFlyEmDataFrame::getNeuron(int id, int bundleIndex) const
{
  const ZFlyEmNeuron *neuron = NULL;
  if (bundleIndex >= 0) {
    neuron = m_dataArray[bundleIndex]->getNeuron(id);
  } else {
    foreach (ZFlyEmDataBundle *data, m_dataArray) {
      neuron = data->getNeuron(id);
      if (neuron != NULL) {
        break;
      }
    }
  }

  return neuron;
}

const ZFlyEmNeuron* ZFlyEmDataFrame::getNeuron(
    const std::pair<int, int> &bodyId) const
{
  return getNeuron(bodyId.first, bodyId.second);
}

size_t ZFlyEmDataFrame::getNeuronNumber() const
{
  size_t n = 0;

  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    n += data->getNeuronArray().size();
  }
  return n;
}

std::vector<double> ZFlyEmDataFrame::getMatchingScore(
    const ZFlyEmNeuron *sourceNeuron, EMatchingPool pool)
{
  vector<double> score;

  if (sourceNeuron == NULL) {
    dump("Invalid id");
    return score;
  }


  ZSwcTree *tree1 = sourceNeuron->getModel();
  if (tree1 == NULL) {
    dump("No model found");
    return score;
  }

    /*
  ZSwcTree *tree1ForMatch = tree1->clone();
  tree1ForMatch->resample(m_resampleStep);
  */

  ZSwcTree *tree1ForMatch = sourceNeuron->getResampleBuddyModel(m_resampleStep);

  score.resize(getNeuronNumber(), 0.0);

  int index = 0;

  getProgressBar()->show();
  getProgressBar()->setRange(0, getNeuronNumber());

  int matchingLevel = 1;

  //For each neuron
  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
         data->getNeuronArray().begin();
         neuronIter != data->getNeuronArray().end();
         ++neuronIter, ++index) {
      //Compare the neuron with the source
      bool comparing = true;
      switch (pool) {
      case MATCH_WITHOUT_SELF:
        if (neuronIter->getId() == sourceNeuron->getId()) {
          comparing = false;
        }
        break;
      case MATCH_KNOWN_CLASS:
        if (neuronIter->getClass().empty() ||
            neuronIter->getId() == sourceNeuron->getId()) {
          comparing = false;
        }
        break;
      default:
        break;
      }

      if (comparing) {
        ZSwcTree *tree2 = neuronIter->getModel();

        if (tree2 != NULL) {
          getProgressBar()->setValue(index + 1);
          QApplication::processEvents();
          double ratio1 =
              ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*tree1);
          double ratio2 =
              ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*tree2);
          bool needFurtherMatch = true;
          if (m_checkOrientation) {
            if (min(ratio1, ratio2) < 1.0) {
              if (max(ratio1, ratio2) > 3.0) {
                score[index] = 0;
                needFurtherMatch = false;
              }
            } else {
              if (max(ratio1, ratio2) / min(ratio1, ratio2) > 3.0) {
                score[index] = 0;
                needFurtherMatch = false;
              }
            }
          }

          if (needFurtherMatch) {
            /*
            ZSwcTree *tree2ForMatch = tree2->clone();

            tree2ForMatch->resample(m_resampleStep);
            */

            ZSwcTree *tree2ForMatch =
                neuronIter->getResampleBuddyModel(m_resampleStep);

            m_matcher.matchAllG(*tree1ForMatch, *tree2ForMatch, matchingLevel);
            /*
        ostringstream stream;
        stream << neuronIter->getName() << " Matching score: " << m_matcher.matchingScore();
        dump(stream.str());
        */
            getProgressBar()->setValue(index + 1);
            QApplication::processEvents();

            score[index] = m_matcher.matchingScore();

            //delete tree2ForMatch;
          }
        } else {
          dump("No model found");
        }
      }
    }
  }

  getProgressBar()->reset();
  getProgressBar()->hide();

  //delete tree1ForMatch;

  return score;
}

std::vector<double> ZFlyEmDataFrame::getMatchingScore(
    int id, int bundleIndex, EMatchingPool pool)
{
  vector<double> score;

  const ZFlyEmNeuron *sourceNeuron = getNeuron(id, bundleIndex);
  if (sourceNeuron == NULL) {
    dump("Invalid id");
    return score;
  }

  ZSwcTree *tree1 = sourceNeuron->getModel();
  if (tree1 == NULL) {
    dump("No model found");
    return score;
  }

  /*
  ZSwcTree *tree1ForMatch = tree1->clone();
  tree1ForMatch->resample(m_resampleStep);
  */

  ZSwcTree *tree1ForMatch = sourceNeuron->getResampleBuddyModel(m_resampleStep);

  score.resize(getNeuronNumber(), 0.0);

  int index = 0;

  //getProgressBar()->show();
  //getProgressBar()->setRange(0, getNeuronNumber());

  int matchingLevel = 1;

  size_t neuronNumber = getNeuronNumber();

  startProgress();
  //For each neuron
  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
         data->getNeuronArray().begin();
         neuronIter != data->getNeuronArray().end();
         ++neuronIter, ++index) {
      advanceProgress(1.0 / neuronNumber);
      //Compare the neuron with the source
      bool comparing = true;
      switch (pool) {
      case MATCH_WITHOUT_SELF:
        if (neuronIter->getId() == sourceNeuron->getId()) {
          comparing = false;
        }
        break;
      case MATCH_KNOWN_CLASS:
        if (neuronIter->getClass().empty() ||
            neuronIter->getId() == sourceNeuron->getId()) {
          comparing = false;
        }
        break;
      default:
        break;
      }

      if (comparing) {
        ZSwcTree *tree2 = neuronIter->getModel();

        if (tree2 != NULL) {
          //getProgressBar()->setValue(index + 1);
          QApplication::processEvents();
          double ratio1 =
              ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*tree1);
          double ratio2 =
              ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*tree2);
          bool needFurtherMatch = true;
          if (m_checkOrientation) {
            if (min(ratio1, ratio2) < 1.0) {
              if (max(ratio1, ratio2) > 3.0) {
                score[index] = 0;
                needFurtherMatch = false;
              }
            } else {
              if (max(ratio1, ratio2) / min(ratio1, ratio2) > 3.0) {
                score[index] = 0;
                needFurtherMatch = false;
              }
            }
          }

          if (needFurtherMatch) {
            /*
            ZSwcTree *tree2ForMatch = tree2->clone();

            tree2ForMatch->resample(m_resampleStep);
            */

            ZSwcTree *tree2ForMatch =
                neuronIter->getResampleBuddyModel(m_resampleStep);

            m_matcher.matchAllG(*tree1ForMatch, *tree2ForMatch, matchingLevel);
            /*
        ostringstream stream;
        stream << neuronIter->getName() << " Matching score: " << m_matcher.matchingScore();
        dump(stream.str());
        */
            //getProgressBar()->setValue(index + 1);
            QApplication::processEvents();

            score[index] = m_matcher.matchingScore();

            //delete tree2ForMatch;
          }
        } else {
          dump("No model found");
        }
      }
    }
  }

  endProgress();
  //getProgressBar()->reset();
  //getProgressBar()->hide();

  //delete tree1ForMatch;

  return score;
}

QVector<const ZFlyEmNeuron*> ZFlyEmDataFrame::getTopMatch(
    const ZFlyEmNeuron *neuron, EMatchingPool pool)
{
  QVector<const ZFlyEmNeuron*> topMatch;

  vector<double> score = getMatchingScore(neuron, pool);

  if (!score.empty()) {
    vector<int> indexArray(score.size());
    darray_qsort(&(score[0]), &(indexArray[0]), score.size());

    size_t size = min(score.size(), (size_t) ZFlyEmNeuron::TopMatchCapacity);

    for (size_t i = 1; i <= size; ++i) {
      topMatch.append(getNeuronFromIndex(indexArray[indexArray.size() - i],
                      NULL));
    }
  }

  return topMatch;
}

void ZFlyEmDataFrame::clearQueryOutput()
{
  updatePresenter(m_target);
  displayQueryOutput(NULL);
  //displayQueryOutput("");
}

ZSwcTree* ZFlyEmDataFrame::getModel(int id, int bundleIndex)
{
  ZSwcTree *model = NULL;
  if (bundleIndex >= 0) {
    model = m_dataArray[bundleIndex]->getModel(id);
  } else {
    foreach (ZFlyEmDataBundle *data, m_dataArray) {
      model = data->getModel(id);
      if (model != NULL) {
        break;
      }
    }
  }

  return model;
}

const ZFlyEmNeuron* ZFlyEmDataFrame::getNeuronFromIndex(
    size_t idx, int *bundleIndex) const
{
  const ZFlyEmNeuron *neuron = NULL;
  if (bundleIndex != NULL) {
    *bundleIndex = 0;
  }
  foreach(ZFlyEmDataBundle *data, m_dataArray) {
    if (idx < data->getNeuronArray().size()) {
      neuron = &(data->getNeuronArray()[idx]);
      break;
    } else {
      idx -= data->getNeuronArray().size();
    }
    if (bundleIndex != NULL) {
      ++(*bundleIndex);
    }
  }

  return neuron;
}

void ZFlyEmDataFrame::predictClass(ZFlyEmNeuron *neuron)
{
  QVector<const ZFlyEmNeuron*> topMatch = getTopMatch(neuron);
  neuron->setMatched(topMatch.begin(), topMatch.end());
  displayQueryOutput(neuron, true);
}

void ZFlyEmDataFrame::process()
{
  //ParameterDialog dlg;
  //dlg.setWindowTitle("process");

  if (m_processDialog.exec()) {
    //parseCommand(dlg.parameter().toStdString());
    parseCommand(m_processDialog.getSourceType().toStdString(),
                 m_processDialog.getSourceString().toStdString(),
                 m_processDialog.getAction().toStdString());

    int matchingLevel = 1;

    switch (m_action) {
    case MATCH:
    {
      ZSwcTree *tree1 = getModel(
            m_sourceIdArray[0].first, m_sourceIdArray[0].second);
      ZSwcTree *tree2 = getModel(
            m_sourceIdArray[1].first, m_sourceIdArray[1].second);

      if (tree1 == NULL || tree2 == NULL) {
        break;
      }

      ZSwcTree *originalTree1 = tree1->clone();
      ZSwcTree *originalTree2 = tree2->clone();

      originalTree1->setColor(255, 0, 0);
      originalTree2->setColor(0, 0, 255);

      ZSwcTree *resampledTree1 = tree1->clone();
      ZSwcTree *resampledTree2 = tree2->clone();
      resampledTree1->resample(m_resampleStep);
      resampledTree2->resample(m_resampleStep);

      m_matcher.matchAllG(*resampledTree1, *resampledTree2, matchingLevel);

      dump(QString("Matching score: %1").arg(m_matcher.matchingScore()));

      ZSwcTree *matchingSwc = m_matcher.exportResultAsSwc();

      matchingSwc->resortId();
      matchingSwc->setColor(0, 255, 0);

      ZStackFrame *swcFrame = new ZStackFrame;
      swcFrame->createDocument();
      swcFrame->document()->addSwcTree(originalTree1);
      swcFrame->document()->addSwcTree(originalTree2);
      swcFrame->document()->addSwcTree(matchingSwc);
      Z3DWindow *window = swcFrame->open3DWindow(NULL);
      if (window != NULL) {
        window->getSwcFilter()->setRenderingPrimitive("Line");
        window->getSwcFilter()->setColorMode("Intrinsic");
      }

      delete resampledTree1;
      resampledTree1 = NULL;
      delete resampledTree2;
      resampledTree2 = NULL;

      delete swcFrame;
    }
      break;
    case SORT_SHAPE:
    {
      clearQueryOutput();

      for (size_t i = 0; i < m_sourceIdArray.size(); ++i) {
        const ZFlyEmNeuron *neuron = getNeuron(m_sourceIdArray[i]);
        QVector<const ZFlyEmNeuron*> topMatch =
            getTopMatch(neuron, MATCH_WITHOUT_SELF);
        neuron->setMatched(topMatch.begin(), topMatch.end());
        displayQueryOutput(neuron, true);
      }
    }
      break;
    case PREDICT_CLASS:
    {
      clearQueryOutput();
      int correctCount = 0;
      int count = 0;

      for (size_t i = 0; i < m_sourceIdArray.size(); ++i) {
        const ZFlyEmNeuron *neuron = getNeuron(m_sourceIdArray[i]);
        QVector<const ZFlyEmNeuron*> topMatch = getTopMatch(neuron);
        neuron->setMatched(topMatch.begin(), topMatch.end());
        displayQueryOutput(neuron, true);
        ++count;
        if (!topMatch.isEmpty()) {
          if (topMatch[0]->getClass() == neuron->getClass()) {
            ++correctCount;
          }
        }
        dump(QString("Accuracy: %1 / %2 = %3").arg(correctCount).arg(count).
             arg((double) correctCount / count));
      }
    }
      break;
    default:
      break;
    }
  }
}

const QColor* ZFlyEmDataFrame::getColor(int id, int bundleIndex) const
{
  const QColor *color = NULL;

  if (bundleIndex >= 0) {
    map<int, QColor> *colorMap = m_dataArray[bundleIndex]->getColorMap();
    if (colorMap->count(id) > 0) {
      color = &((*colorMap)[id]);
    }
  } else {
    foreach (ZFlyEmDataBundle *data, m_dataArray) {
      map<int, QColor> *colorMap = data->getColorMap();

      if (colorMap->count(id) > 0) {
        color = &((*colorMap)[id]);
        break;
      }
    }
  }
  return color;
}

const QColor* ZFlyEmDataFrame::getColor(const std::pair<int, int> &bodyId) const
{
  return getColor(bodyId.first, bodyId.second);
}

void ZFlyEmDataFrame::showModel() const
{
  ZStackFrame *swcFrame = new ZStackFrame;
  swcFrame->createDocument();
  for (size_t i = 0; i < m_sourceIdArray.size(); ++i) {
    const ZFlyEmNeuron *neuron = getNeuron(m_sourceIdArray[i]);
    if (neuron == NULL) {
      dump("Invalid id");
    } else {
      ZSwcTree *model = neuron->getModel();
      swcFrame->document()->blockSignals(true);
      if (model != NULL) {
        const QColor *color = getColor(m_sourceIdArray[i]);
        if (color != NULL) {
          model->setColor(*color);
        }

        swcFrame->document()->addSwcTree(model->clone());
      }
      vector<ZPunctum*> puncta = neuron->getSynapse();
      for (vector<ZPunctum*>::iterator iter = puncta.begin();
           iter != puncta.end(); ++iter) {
        swcFrame->document()->addPunctum(*iter);
      }
      swcFrame->document()->blockSignals(false);
      swcFrame->document()->swcObjsModel()->updateModelData();
      swcFrame->document()->punctaObjsModel()->updateModelData();
    }
#if 0
    string modelPath = m_data.getModelPath(m_sourceIdArray[i]);
    if (!modelPath.empty()) {
      swcFrame->document()->loadSwc(modelPath.c_str());
    } else {
     dump(QString("<font size=4 face=Times>No model path found : %1</font>")
          .arg(m_sourceIdArray[i]).toStdString());
    }
#endif

  }
  swcFrame->open3DWindow(NULL);
  delete swcFrame;
}

void ZFlyEmDataFrame::showConnection() const
{
  ZStackFrame *swcFrame = new ZStackFrame;
  swcFrame->createDocument();
  for (size_t i = 0; i < m_sourceIdArray.size(); ++i) {
    const ZFlyEmNeuron *neuron = getNeuron(m_sourceIdArray[i]);
    if (neuron == NULL) {
      dump("Invalid id");
    } else {
      ZSwcTree *model = neuron->getModel();
      swcFrame->document()->blockSignals(true);
      if (model != NULL) {
        const QColor *color = getColor(m_sourceIdArray[i]);
        if (color != NULL) {
          model->setColor(*color);
        }

        swcFrame->document()->addSwcTree(model->clone());
      }

      for (size_t j = 0; j < m_sourceIdArray.size(); ++j) {
        if (i != j) {
          if (m_sourceIdArray[i].second == m_sourceIdArray[j].second) {
            vector<ZPunctum*> puncta = neuron->getSynapse(m_sourceIdArray[j].first);
            for (vector<ZPunctum*>::iterator iter = puncta.begin();
                 iter != puncta.end(); ++iter) {
              swcFrame->document()->addPunctum(*iter);
            }
          }
        }
      }

      swcFrame->document()->blockSignals(false);
      swcFrame->document()->swcObjsModel()->updateModelData();
      swcFrame->document()->punctaObjsModel()->updateModelData();
    }
#if 0
    string modelPath = m_data.getModelPath(m_sourceIdArray[i]);
    if (!modelPath.empty()) {
      swcFrame->document()->loadSwc(modelPath.c_str());
    } else {
     dump(QString("<font size=4 face=Times>No model path found : %1</font>")
          .arg(m_sourceIdArray[i]).toStdString());
    }
#endif

  }
  swcFrame->open3DWindow(NULL);
  delete swcFrame;
}

#if 0
void ZFlyEmDataFrame::query() const
{
  ParameterDialog dlg;
  dlg.setWindowTitle("query");

  if (dlg.exec()) {
    ZString queryString = dlg.parameter();

    vector<string> queryWordArray = queryString.toWordArray(", ->;:\n");

    if (queryWordArray.size() < 2) {
      return;
    }

    std::string source = queryWordArray[0];
    std::string target = queryWordArray[1];

    /*
    std::string condition = "(equal)";
    if (queryWordArray.size() >= 3) {
      if (queryWordArray[2].startsWith("(") && queryWordArray[2].endsWith(")")) {
        condition = queryWordArray[2];
      }
    }
    */

    vector<int> bodyIdArray;
    if (source == "id") {
      bodyIdArray = queryString.toIntegerArray();
    } else if (source == "name") {
      vector<string>::const_iterator nameIter = queryWordArray.begin();
      ++nameIter;
      ++nameIter;
      for (; nameIter != queryWordArray.end(); ++nameIter) {
        for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
             m_data.getNeuronArray().begin();
             neuronIter != m_data.getNeuronArray().end();
             ++neuronIter) {
          if (neuronIter->hasSimilarName(*nameIter)) {
            bodyIdArray.push_back(neuronIter->getId());
          }
        }
      }
    } else if (source == "class") {
      vector<string>::const_iterator nameIter = queryWordArray.begin();
      ++nameIter;
      ++nameIter;
      for (; nameIter != queryWordArray.end(); ++nameIter) {
        for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
             m_data.getNeuronArray().begin();
             neuronIter != m_data.getNeuronArray().end();
             ++neuronIter) {
          if (neuronIter->getClass() == *nameIter) {
            bodyIdArray.push_back(neuronIter->getId());
          }
        }
      }
    } else if (source == "all") {
      for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
           m_data.getNeuronArray().begin();
           neuronIter != m_data.getNeuronArray().end();
           ++neuronIter) {
        bodyIdArray.push_back(neuronIter->getId());
      }
    }

    if (target == "model") {
      ZStackFrame *swcFrame = new ZStackFrame;
      swcFrame->createDocument();
      for (size_t i = 0; i < bodyIdArray.size(); ++i) {
        string modelPath = m_data.getModelPath(bodyIdArray[i]);
        if (!modelPath.empty()) {
          swcFrame->document()->loadSwc(modelPath.c_str());

        } else {
         dump(QString("<font size=4 face=Times>No model path found : %1</font>").arg(bodyIdArray[i]).toStdString());
        }
      }
      swcFrame->open3DWindow(NULL);
      delete swcFrame;
    } else if (target == "name" || target == "id") {
      ostringstream stream;
      for (size_t i = 0; i < bodyIdArray.size(); ++i) {
        stream << bodyIdArray[i] << ": \"" << m_data.getName(bodyIdArray[i])
               << "\"" << endl;
      }
      displayQueryOutput(stream.str());
    } else if (target == "summary") {
      ostringstream stream;
      for (size_t i = 0; i < bodyIdArray.size(); ++i) {
        int bodyId = bodyIdArray[i];
        if (m_data.getNeuron(bodyId) == NULL) {
          stream << bodyId << ": no record" << endl;
        } else {
          m_data.getNeuron(bodyId)->printJson(&stream, 0);
        }
      }
      displayQueryOutput(stream.str());
    } else if (target == "connection") {
      ostringstream stream;
      std::map<int, size_t> idMap; //id -> index
      vector<int> idArray; //index -> id
      vector<int> output;
      vector<int> input;

      FlyEm::ZSynapseAnnotationArray *synapseArray =
          m_data.getSynapseAnnotation();

      for (vector<int>::const_iterator bodyIdIter = bodyIdArray.begin();
           bodyIdIter != bodyIdArray.end(); ++bodyIdIter) {
        int bodyId = *bodyIdIter;

        for (size_t i = 0; i < synapseArray->size(); i++) {
          int tBarId = (*synapseArray)[i].getTBarRef()->bodyId();
          vector<FlyEm::SynapseLocation> *partnerArray =
              (*synapseArray)[i].getPartnerArrayRef();
          if (tBarId == bodyId) {
            //Count outputs
            for (size_t j = 0; j < partnerArray->size(); j++) {
              size_t index = get_index(idMap, (*partnerArray)[j].bodyId());
              if (index >= output.size()) {
                output.resize(index + 1, 0);
              }
              output[index]++;
            }
          } else {
            //Count inputs
            for (size_t j = 0; j < partnerArray->size(); j++) {
              if ((*partnerArray)[j].bodyId() == bodyId) {
                size_t index = get_index(idMap, tBarId);
                if (index >= input.size()) {
                  input.resize(index + 1, 0);
                }
                input[index]++;
              }
            }
          }
        }

        for (map<int, size_t>::iterator iter = idMap.begin();
             iter != idMap.end(); ++iter) {
          if (iter->second >= idArray.size()) {
            idArray.resize(iter->second + 1, 0);
          }
          idArray[iter->second] = iter->first;
        }

        int minconn = 5;
        for (size_t i = 0; i < input.size(); i++) {
          if (input[i] >= minconn) {
            int neighborBodyId = idArray[i];
            stream << neighborBodyId << " (" << m_data.getName(neighborBodyId)
                   << ")" << " x " << input[i] << " --> " << bodyId << " ("
                   << m_data.getName(bodyId) << ")" << endl;
          }
        }

        for (size_t i = 0; i < output.size(); i++) {
          if (output[i] >= minconn) {
            int neighborBodyId = idArray[i];
            stream << neighborBodyId << " (" << m_data.getName(neighborBodyId)
                   << ")"  << " x " << output[i] << " <-- " << bodyId << " ("
                   << m_data.getName(bodyId) << ")" << endl;
          }
        }
      }
      displayQueryOutput(stream.str());
    }
  }
}

void ZFlyEmDataFrame::process() const
{
  ParameterDialog dlg;
  dlg.setWindowTitle("process");
  if (dlg.exec()) {
    ZString queryString = dlg.parameter();
    vector<string> queryWordArray = queryString.toWordArray(", >;:\n");
    string source = queryWordArray[0];
    string command = queryWordArray[1];
    double resampleStep = 400.0;

    vector<int> bodyIdArray;
    if (source == "id") {
      bodyIdArray = queryString.toIntegerArray();
    } else if (source == "name") {
      vector<string>::const_iterator nameIter = queryWordArray.begin();
      ++nameIter;
      ++nameIter;
      for (; nameIter != queryWordArray.end(); ++nameIter) {
        for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
             m_data.getNeuronArray().begin();
             neuronIter != m_data.getNeuronArray().end();
             ++neuronIter) {
          if (neuronIter->hasSimilarName(*nameIter)) {
            bodyIdArray.push_back(neuronIter->getId());
          }
        }
      }
    } else if (source == "class") {
      vector<string>::const_iterator nameIter = queryWordArray.begin();
      ++nameIter;
      ++nameIter;
      for (; nameIter != queryWordArray.end(); ++nameIter) {
        for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
             m_data.getNeuronArray().begin();
             neuronIter != m_data.getNeuronArray().end();
             ++neuronIter) {
          if (neuronIter->getClass() == *nameIter) {
            bodyIdArray.push_back(neuronIter->getId());
          }
        }
      }
    }

    ZSwcTreeMatcher matcher;
    ZSwcDistTrunkAnalyzer trunkAnalyzer;

    //ZSwcSizeFeatureAnalyzer analyzer;
    ZSwcLayerFeatureAnalyzer analyzer;
    analyzer.setLayerScale(5000.0);

    /*
    ZSwcShollFeatureAnalyzer analyzer;
    analyzer.setShollRadius(200.0);
    analyzer.setShollEnd(2000.0);
    analyzer.setShollStart(200.0);
*/

    matcher.setTrunkAnalyzer(&trunkAnalyzer);
    matcher.setFeatureAnalyzer(&analyzer);

    if (command == "match") {
      ZSwcTree tree1;
      ZSwcTree tree2;
      tree1.load(m_data.getModelPath(bodyIdArray[0]).c_str());
      tree2.load(m_data.getModelPath(bodyIdArray[1]).c_str());

      ZSwcTree *originalTree1 = tree1.clone();
      ZSwcTree *originalTree2 = tree2.clone();

      originalTree1->setColor(255, 0, 0);
      originalTree2->setColor(0, 0, 255);

      tree1.resample(resampleStep);
      tree2.resample(resampleStep);

      matcher.matchAllG(tree1, tree2, 1);
      ostringstream stream;
      stream << "Matching score: " << matcher.matchingScore();
      dump(stream.str());

      ZSwcTree *matchingSwc = matcher.exportResultAsSwc();

      matchingSwc->resortId();
      matchingSwc->setColor(0, 255, 0);

      ZStackFrame *swcFrame = new ZStackFrame;
      swcFrame->createDocument();
      swcFrame->document()->addSwcTree(originalTree1);
      swcFrame->document()->addSwcTree(originalTree2);
      swcFrame->document()->addSwcTree(matchingSwc);
      Z3DWindow *window = swcFrame->open3DWindow(NULL);
      if (window != NULL) {
        window->getSwcFilter()->setRenderingPrimitive("Line");
        window->getSwcFilter()->setColorMode("Intrinsic");
      }

      delete swcFrame;
    } else if (command == "sort_shape") {
      vector<double> score;
      score.resize(m_data.getNeuronArray().size(), 0.0);

      ostringstream queryStream;
      for (int i = 0; i < bodyIdArray.size(); ++i) {
        int bodyId = bodyIdArray[i];
        const ZFlyEmNeuron *sourceNeuron = m_data.getNeuron(bodyId);

        ZSwcTree *tree1 = sourceNeuron->getModel();
        if (tree1 != NULL) {
          ZSwcTree *tree1ForMatch = tree1->clone();
          tree1ForMatch->resample(resampleStep);

          int index = 0;
          //For each neuron
          for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
               m_data.getNeuronArray().begin();
               neuronIter != m_data.getNeuronArray().end();
               ++neuronIter, ++index) {
#ifdef _DEBUG_2
            cout << neuronIter->getName() << endl;
#endif
            //Compare the neuron with the source
            if (neuronIter->getId() != sourceNeuron->getId()) {
              ZSwcTree *tree2 = neuronIter->getModel();

              if (tree2 != NULL) {
                ZSwcTree *tree2ForMatch = tree2->clone();

                tree2ForMatch->resample(resampleStep);

                matcher.matchAllG(*tree1ForMatch, *tree2ForMatch, 1);
                ostringstream stream;
                stream << "Matching score: " << matcher.matchingScore();
                dump(stream.str());

                score[index] = matcher.matchingScore();

                delete tree2ForMatch;
              } else {
                dump("No model found");
              }
            }
          }
          delete tree1ForMatch;

          //Sort scores
          vector<int> indices(score.size());
          darray_qsort(&(score[0]), &(indices[0]), score.size());

          //Show the sorted results
          ostringstream stream;
          for (size_t i = 0; i < indices.size(); ++i) {
            m_data.getNeuronArray()[indices[i]].printJson(&stream, 0);
          }
          dump(stream.str());
          queryStream << m_data.getNeuron(bodyId)->getName()
              << " : " << m_data.getNeuronArray()[indices.back()].getName()
              << endl;
        } else {
          dump("No model found");
        }

        displayQueryOutput(queryStream.str());
      }
    }
  }
}
#endif

void ZFlyEmDataFrame::showSummary() const
{
#ifdef _DEBUG_2
  std::cout << "showSummary triggered" << std::endl;
#endif
  InformationDialog dlg;
  dlg.setWindowTitle("FlyEM Data");
  dlg.setText(getInformationText());
  dlg.exec();
}

FlyEmDataForm *ZFlyEmDataFrame::getMainWidget() const
{
  FlyEmDataForm *form = dynamic_cast<FlyEmDataForm*>(widget());

  return form;
}

void ZFlyEmDataFrame::dump(const QString &message) const
{
  FlyEmDataForm *form = getMainWidget();
  if (form != NULL) {
    form->appendOutput("<p>" + message + "</p>");
    QApplication::processEvents();
  }
}

QProgressBar* ZFlyEmDataFrame::getProgressBar()
{
  return getMainWidget()->getProgressBar();
}

void ZFlyEmDataFrame::displayQueryOutput(
    const ZFlyEmNeuron *neuron, bool appending) const
{
  FlyEmDataForm *form = dynamic_cast<FlyEmDataForm*>(widget());
  if (form != NULL) {
    if (appending) {
      form->appendQueryOutput(neuron);
      //form->appendQueryOutput(neuron);
    } else {
      form->setQueryOutput(neuron);
    }
    QApplication::processEvents();
  }
}

void ZFlyEmDataFrame::test()
{
#if 1
  //Calculate matching matrix
  std::ofstream stream((GET_DATA_DIR + "/score.txt").c_str());
  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    for (vector<ZFlyEmNeuron>::const_iterator neuronIter =
         data->getNeuronArray().begin();
         neuronIter != data->getNeuronArray().end();
         ++neuronIter) {
      dump(neuronIter->getName().c_str());
      std::vector<double> score = getMatchingScore(
            neuronIter->getId(), -1, MATCH_ALL_NEURON);
      for (size_t i = 0; i < score.size(); ++i) {
        stream << score[i] << ", ";
      }
      stream << std::endl;
    }
  }
  stream.close();
#endif

#if 0
  //FILE *fp = fopen((GET_DATA_DIR + "/flyem/TEM/matching/name.txt").c_str(), "r");
  //ZString str;

  ofstream out((GET_DATA_DIR + "/flyem/TEM/matching/class_label.txt").c_str());
  ofstream nameOut((GET_DATA_DIR + "/flyem/TEM/matching/class_name.txt").c_str());

  QMap<string, int> classMap;

  int currentIndex = 1;

  foreach (ZFlyEmDataBundle *data, m_dataArray) {
    for (std::vector<ZFlyEmNeuron>::const_iterator iter =
         data->getNeuronArray().begin(); iter != data->getNeuronArray().end();
         ++iter) {
      //Update class map
      if (!classMap.contains(iter->getClass())) {
        classMap[iter->getClass()] = currentIndex++;
      }

      //Save the class index
      out << classMap[iter->getClass()] << std::endl;
      nameOut << iter->getClass() << std::endl;
    }
  }

  nameOut.close();

#if 0
  //For each line of the name file
  while (str.readLine(fp)) {
    //Extract the name
    str.trim();

    //Find the class
    const ZFlyEmNeuron *neuron = NULL;
    foreach (ZFlyEmDataBundle *data, m_dataArray) {
      neuron = data->getNeuronFromName(str);
      if (neuron != NULL) {
        break;
      }
    }

    if (neuron == NULL) {
      dump("Cannot find " + str);
    } else {
      //Update class map
      if (!classMap.contains(neuron->getClass())) {
        classMap[neuron->getClass()] = currentIndex++;
      }

      //Save the class index
      out << classMap[neuron->getClass()] << std::endl;
    }
  }

  fclose(fp);
#endif

  out.close();

#endif
}

void ZFlyEmDataFrame::setParameter()
{
  ZSwcLayerTrunkAnalyzer *trunkAnalyzer =
      dynamic_cast<ZSwcLayerTrunkAnalyzer*>(m_trunkAnalyzer);
  if (trunkAnalyzer != NULL) {
    m_optionDialog.setTrunkStep(trunkAnalyzer->getStep());
  }

  ZSwcLayerShollFeatureAnalyzer *featureAnalyzer =
      dynamic_cast<ZSwcLayerShollFeatureAnalyzer*>(m_helperFeatureAnalyzer);

  if (featureAnalyzer != NULL) {
    m_optionDialog.setLayerScale(featureAnalyzer->getLayerScale());
    m_optionDialog.setLayerMargin(featureAnalyzer->getLayerMargin());
  }

  m_optionDialog.setResampleStep(m_resampleStep);

  if (m_optionDialog.exec()) {
    if (trunkAnalyzer != NULL) {
      trunkAnalyzer->setStep(m_optionDialog.getTrunkStep());
    }

    if (featureAnalyzer != NULL) {
      featureAnalyzer->setLayerScale(m_optionDialog.getLayerScale());
      featureAnalyzer->setLayerMargin(m_optionDialog.getLayerMargin());
    }
  }
}

void ZFlyEmDataFrame::setStatusBar(QStatusBar *bar)
{
  m_centralWidget->setStatusBar(bar);
}

void ZFlyEmDataFrame::openVolume(const QString &path)
{
  emit volumeTriggered(path);
}

void ZFlyEmDataFrame::saveBundle(int index, const QString &path)
{
  if (index < m_dataArray.size()) {
    m_dataArray[index]->exportJsonFile(path.toStdString());
  }
}

void ZFlyEmDataFrame::showNearbyNeuron(const ZFlyEmNeuron *neuron)
{
  if (neuron != NULL) {
    ZSwcTree *tree1 = neuron->getModel();

    if (tree1 != NULL) {

      m_geoSearchDlg->setDataBundleWidget(m_dataArray);
      //dlg.setDataBundleWidget(m_dataArray);

      if (m_geoSearchDlg->exec()) {
        startProgress();

        ZStackFrame *swcFrame = new ZStackFrame;
        swcFrame->createDocument();
        swcFrame->document()->blockSignals(true);
        swcFrame->document()->addSwcTree(tree1->clone());

        int index = 0;
        foreach (ZFlyEmDataBundle *bundle, m_dataArray) {
          if (m_geoSearchDlg->isBundleSelected(index++)) {
            std::vector<ZFlyEmNeuron>& neuronArray = bundle->getNeuronArray();

            ZFlyEmNeuronFilter *filter = m_geoSearchDlg->getFilter();

            if (filter != NULL) {
              filter->setReference(neuron);
              filter->print();
              startProgress(0.9 / m_dataArray.size());
              for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
                   iter != neuronArray.end(); ++iter) {
                //std::cout << "Checking " << iter->getId() << "..." << std::endl;
                ZSwcTree *tree2 = iter->getModel();
                if (tree2 != NULL && tree1 != tree2) {
                  if (filter->isPassed(*iter)) {
                    swcFrame->document()->addSwcTree(tree2->clone());
                  }
                }
                advanceProgress(1.0 / neuronArray.size());
              }
              endProgress(0.9 / m_dataArray.size());
            }
          }

#if 0
            double minDist = m_geoSearchDlg->getMinDist();
            double minLength = m_geoSearchDlg->getMinLength();
            double maxLength = m_geoSearchDlg->getMaxLength();
            int minLayer = m_geoSearchDlg->getMinLayer();
            int maxLayer = m_geoSearchDlg->getMaxLayer();
            bool isExclusive = m_geoSearchDlg->isLayerExclusive();

            ZSwcMetric *metric = new ZSwcTerminalSurfaceMetric;

            startProgress(0.9 / m_dataArray.size());
            for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
                 iter != neuronArray.end(); ++iter) {
              //std::cout << "Checking " << iter->getId() << "..." << std::endl;
              ZSwcTree *tree2 = iter->getModel();
              if (tree2 != NULL && tree1 != tree2) {
                double length = tree2->length();
                if (length >= minLength && length <= maxLength) {
                  if (bundle->hitsLayer(*iter, minLayer, maxLayer, isExclusive)) {
                    double dist = metric->measureDistance(tree1, tree2);
                    if (dist <= minDist) {
                      swcFrame->document()->addSwcTree(tree2->clone());
                    }
                  }
                }
              }
              advanceProgress(1.0 / neuronArray.size());
            }
            endProgress(0.9 / m_dataArray.size());
          }
#endif
        }
        swcFrame->document()->blockSignals(false);

        if (swcFrame->document()->getSwcList().size() > 1) {
          swcFrame->document()->swcObjsModel()->updateModelData();
          Z3DWindow *window = swcFrame->open3DWindow(NULL);
          delete swcFrame;
          if (window != NULL) {
            window->getSwcFilter()->setRenderingPrimitive("Normal");
            window->getSwcFilter()->setColorMode("Invididual");
          }
        } else {
          QMessageBox::information(this, "No Neuron Found", "No neuron found.");
          delete swcFrame;
        }
        endProgress();
      }
    }
  }
}

bool ZFlyEmDataFrame::exportSimilarityMatrix(
    const QString &fileName, int bundleIndex)
{
  if (bundleIndex < 0 || bundleIndex >= m_dataArray.size()) {
    return false;
  }

  std::vector<ZFlyEmNeuron> &neuronArray =
      m_dataArray[bundleIndex]->getNeuronArray();

  std::ofstream outStream(fileName.toStdString().c_str());
  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    outStream << iter->getId() << " ";
  }
  outStream << std::endl;

  startProgress();
  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    startProgress(1.0 / neuronArray.size());
    std::vector<double> scoreArray = getMatchingScore(
          iter->getId(), bundleIndex, MATCH_ALL_NEURON);
    for (std::vector<double>::const_iterator scoreIter = scoreArray.begin();
         scoreIter != scoreArray.end(); ++scoreIter) {
      outStream << *scoreIter << " ";
    }
    outStream << std::endl;
    endProgress(1.0 / neuronArray.size());
  }
  endProgress();

  return true;
}
