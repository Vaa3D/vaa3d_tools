#ifndef ZFLYEMDATAFRAME_H
#define ZFLYEMDATAFRAME_H

#include <QMdiSubWindow>
#include <string>
#include <QProgressBar>
#include <QVector>
#include <utility>

#include "zflyemdatabundle.h"
#include "zswctreematcher.h"
#include "flyemdataframeoptiondialog.h"
#include "flyemdataquerydialog.h"
#include "flyemdataprocessdialog.h"
#include "zprogressable.h"
#include "zqtbarprogressreporter.h"

class ZSwcTrunkAnalyzer;
class ZSwcFeatureAnalyzer;
class ZFlyEmNeuronInfoPresenter;
class ZFlyEmNeuronFeaturePresenter;
class ZFlyEmNeuronConnectionPresenter;
class ZFlyEmNeuronVolumePresenter;
class ZFlyEmNeuronTopMatchPresenter;
class FlyEmDataForm;
class QStatusBar;
class FlyEmGeoSearchDialog;
class FlyEmGeoFilterDialog;

class ZFlyEmDataFrame : public QMdiSubWindow, ZProgressable
{
  Q_OBJECT
public:
  explicit ZFlyEmDataFrame(QWidget *parent = 0);
  ~ZFlyEmDataFrame();
  
  enum EDataForm {
    ID, NAME, CLASS, MODEL, SUMMARY, CONNECTION, FEATURE, VOLUME, TOP_MATCH,
    CONNECTION_MODEL, UNKNOWN_DATA_FORM
  };

  enum EAction {
    MATCH, SORT_SHAPE, PREDICT_CLASS, PREDICT_ERROR, UNKNOWN_ACTION
  };

  //Load a data bundle file. It returns false if the file cannot be loaded
  //correctly.
  bool load(const std::string &filePath, bool appending = false);

  void clearData();
  void addData(ZFlyEmDataBundle *data);

  //Return the information of the flyem data
  std::string getInformationText() const;

  void updatePresenter(EDataForm target);

  //Update the query text for showing <target> for <id>
  int updateQuery(int id, int bundleIndex,
                   EDataForm target, bool appending = false);

  //void displayQueryOutput(const std::string &text, bool appending = false) const;
  void displayQueryOutput(const ZFlyEmNeuron *neuron, bool appending = false) const;
  //void dump(const std::string &message) const;
  void dump(const QString &message) const;

  void setStatusBar(QStatusBar *bar);

  void predictClass(ZFlyEmNeuron *neuron);

  /*!
   * \brief Export unnormalized similarity matrix
   *
   * The first row is a list of IDs. The simarity between the same neuron is
   * also calculated.
   *
   * \a return true iff the export succeeds.
   */
  bool exportSimilarityMatrix(const QString &fileName, int bundleIndex = 0);

signals:
  void volumeTriggered(const QString &path);
  
public slots:
  //Show summary information
  void showSummary() const;
  void query();
  void process();
  void test();
  //void generalProcess();
  void setParameter();
  void openVolume(const QString &path);
  /*!
   * \brief Save a bundle.
   * \param index Index of the bundle.
   * \param path Output path.
   */
  void saveBundle(int index, const QString &path);

  void showNearbyNeuron(const ZFlyEmNeuron *neuron);

private:
  FlyEm::ZSynapseAnnotationArray *getSynapseAnnotation();
  std::string getName(int bodyId) const;
  std::string getName(int bodyId, int bundleIndex) const;
  std::string getName(const std::pair<int, int> &bodyId) const;

  const ZFlyEmNeuron* getNeuron(int id, int bundleIndex = -1) const;
  const ZFlyEmNeuron* getNeuron(const std::pair<int, int> &bodyId) const;

  const ZFlyEmNeuron *getNeuronFromIndex(size_t idx, int *bundleIndex) const;
  size_t getNeuronNumber() const;

  ZSwcTree* getModel(int id, int bundleIndex = -1);
  const QColor* getColor(int id, int bundleIndex = -1) const;
  const QColor* getColor(const std::pair<int, int> &bodyId) const;

  FlyEmDataForm *getMainWidget() const;

private:
  void parseCommand(const std::string &command);

  void parseCommand(const std::string &sourceType,
                    const std::string &sourceValue,
                    const std::string &action);

  void parseQuery(const std::string &sourceType,
                  const std::string &sourceValue,
                  const std::string &targetType, bool usingRegexp);

  void updateSource(const std::string &sourceType,
                    const std::string &sourceValue,
                    bool usingRegexp);

  void showModel() const;
  void showConnection() const;
  enum EMatchingPool {
    MATCH_ALL_NEURON, MATCH_WITHOUT_SELF, MATCH_KNOWN_CLASS
  };

  std::vector<double> getMatchingScore(
      const ZFlyEmNeuron *neuron, EMatchingPool pool = MATCH_KNOWN_CLASS);

  std::vector<double> getMatchingScore(
      int id, int bundleIndex, EMatchingPool pool = MATCH_KNOWN_CLASS);

  void clearQueryOutput();

  QVector<const ZFlyEmNeuron*> getTopMatch(
      const ZFlyEmNeuron *neuron, EMatchingPool pool = MATCH_KNOWN_CLASS);

  QProgressBar* getProgressBar();

private:
  QVector<ZFlyEmDataBundle*> m_dataArray;
  
  FlyEmDataForm *m_centralWidget;

  //parsing results
  EDataForm m_target;
  std::vector<std::pair<int, int> > m_sourceIdArray;
  EAction m_action;
  std::string m_condition;

  //configuration
  ZSwcTreeMatcher m_matcher;
  ZSwcTrunkAnalyzer *m_trunkAnalyzer;
  ZSwcFeatureAnalyzer *m_featureAnalyzer;
  ZSwcFeatureAnalyzer *m_helperFeatureAnalyzer;
  double m_resampleStep;
  int m_matchingLevel;

  FlyEmDataFrameOptionDialog m_optionDialog;
  FlyEmDataQueryDialog m_queryDialog;
  FlyEmDataProcessDialog m_processDialog;
  bool m_checkOrientation;

  ZFlyEmNeuronInfoPresenter *m_infoPresenter;
  ZFlyEmNeuronFeaturePresenter *m_featurePresenter;
  ZFlyEmNeuronConnectionPresenter *m_connectionPresenter;
  ZFlyEmNeuronVolumePresenter *m_volumePresenter;
  ZFlyEmNeuronTopMatchPresenter *m_topMatchPresenter;

  ZQtBarProgressReporter m_specialProgressReporter;

  FlyEmGeoFilterDialog *m_geoSearchDlg;
};

#endif // ZFLYEMDATAFRAME_H
