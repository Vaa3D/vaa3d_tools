#ifndef NEUTUBECONFIG_H
#define NEUTUBECONFIG_H

#include <string>
#include <vector>

#include "zmessagereporter.h"

class ZXmlNode;

class NeutubeConfig
{
public:
  enum Config_Item {
    DATA, FLYEM_BODY_CONN_CLASSIFIER, FLYEM_BODY_CONN_TRAIN_DATA,
    FLYEM_BODY_CONN_TRAIN_TRUTH, FLYEM_BODY_CONN_EVAL_DATA,
    FLYEM_BODY_CONN_EVAL_TRUTH, SWC_REPOSOTARY, AUTO_SAVE,
    CONFIGURE_FILE
  };

  static NeutubeConfig& getInstance() {
    static NeutubeConfig config;

    return config;
  }

  inline void setApplicationDir(const std::string &str) {
    m_applicationDir = str;
  }

  bool load(const std::string &filePath);
  void print();

  std::string getPath(Config_Item item) const;
  inline const std::string& getApplicatinDir() const {
    return m_applicationDir; }
  inline std::string getConfigPath() const {
    return getApplicatinDir() + "/config.xml"; }
  inline std::string getHelpFilePath() const {
    return getApplicatinDir() + "/doc/shortcut.html"; }

  inline const std::vector<std::string> &getBodyConnectionFeature() {
    return m_bodyConnectionFeature;
  }

  inline double getSegmentationClassifThreshold() {
    return m_segmentationClassifThreshold;
  }

  inline const std::string& getApplication() const {
    return m_application;
  }

  inline bool isStereoEnabled() {
    return m_isStereoOn;
  }

  class MainWindowConfig {
  public:
    MainWindowConfig();

    void loadXmlNode(const ZXmlNode *node);

    inline void enableTracing(bool tracingOn) { m_tracingOn = tracingOn; }
    inline void enableMarkPuncta(bool on) { m_isMarkPunctaOn = on; }
    inline void enableSwcEdit(bool on) { m_isSwcEditOn = on; }
    inline void enableMergeImage(bool on) { m_isMergeImageOn = on; }

    inline bool isTracingOn() const { return m_tracingOn; }
    inline bool isTracingOff() const { return !isTracingOn(); }
    inline bool isMarkPunctaOn() const { return m_isMarkPunctaOn; }
    inline bool isMarkPunctaOff() const { return !isMarkPunctaOn(); }
    inline bool isSwcEditOn() const { return m_isSwcEditOn; }
    inline bool isSwcEditOff() const { return !isSwcEditOn(); }
    inline bool isMergeImageOn() const { return m_isMergeImageOn; }
    inline bool isMergeImageOff() const { return !isMergeImageOn(); }

    inline bool isExpandNeuronNetworkOn() const {
      return m_isExpandNeuronNetworkOn;
    }
    inline bool isExpandTracingResultOn() const {
      return m_isExpandTracingResultOn;
    }

    inline bool isExpandV3dApoOn() const {
      return m_isExpandV3dApoOn;
    }

    inline bool isExpandV3dMarkerOn() const{
      return m_isExpandV3dMarkerOn;
    }

    inline bool isExpandSwcWith3DWindow() const {
      return m_isExpandSwcWith3DWindow;
    }
    inline void setExpandSwcWith3DWindow(bool state) {
      m_isExpandSwcWith3DWindow = state;
    }

    inline bool isProcessBinarizeOn() const {
      return m_isProcessBinarizeOn;
    }
    inline void setProcessBinarize(bool state) {
      m_isProcessBinarizeOn = state;
    }

    inline bool isMaskToSwcOn() const {
      return m_isMaskToSwcOn;
    }
    inline void setMaskToSwc(bool state) {
      m_isMaskToSwcOn = state;
    }

    inline bool isBinaryToSwcOn() const {
      return m_isBinaryToSwcOn;
    }
    inline void setBinaryToSwc(bool state) {
      m_isBinaryToSwcOn = state;
    }

    inline bool isThresholdControlOn() const {
      return m_isThresholdControlOn;
    }
    inline void setThresholdControl(bool state) {
      m_isThresholdControlOn = state;
    }

  private:
    bool m_tracingOn;
    bool m_isMarkPunctaOn;
    bool m_isSwcEditOn;
    bool m_isMergeImageOn;
    bool m_isExpandSwcWith3DWindow;
    bool m_isExpandNeuronNetworkOn;
    bool m_isExpandTracingResultOn;
    bool m_isExpandV3dApoOn;
    bool m_isExpandV3dMarkerOn;
    bool m_isProcessBinarizeOn;
    bool m_isMaskToSwcOn;
    bool m_isBinaryToSwcOn;
    bool m_isThresholdControlOn;
  };

  class Z3DWindowConfig {
  public:
    Z3DWindowConfig();

    void loadXmlNode(const ZXmlNode *node);

    inline bool isUtilsOn() const { return m_isUtilsOn; }
    inline bool isVolumeOn() const { return m_isVolumeOn; }
    inline bool isGraphOn() const { return m_isGraphOn; }
    inline bool isSwcsOn() const { return m_isSwcsOn; }
    inline bool isTubesOn() const { return m_isTubesOn; }
    inline bool isPunctaOn() const { return m_isPunctaOn; }
    inline bool isMeshOn() const { return m_isMeshOn; }
    inline bool isTensorOn() const { return m_isTensorOn; }
    inline bool isAxisOn() const { return m_isAxisOn; }

    inline void enableUtils(bool on) { m_isUtilsOn = on; }
    inline void enableVolume(bool on) { m_isVolumeOn = on; }
    inline void enableGraph(bool on) { m_isGraphOn = on; }
    inline void enableSwcs(bool on) { m_isSwcsOn = on; }
    inline void enableTubes(bool on) { m_isTubesOn = on; }
    inline void enablePuncta(bool on) { m_isPunctaOn = on; }
    inline void enableTensor(bool on) { m_isTensorOn = on; }
    inline void enableMesh(bool on) { m_isMeshOn = on; }
    inline void enableAxis(bool on) { m_isAxisOn = on; }

    class SwcTabConfig {
    public:
      SwcTabConfig();
      void loadXmlNode(const ZXmlNode *node);

      inline std::string getPrimitive() const { return m_primitive; }
      inline std::string getColorMode() const { return m_colorMode; }
      inline double getZScale() const { return m_zscale; }

    private:
      std::string m_primitive;
      std::string m_colorMode;
      double m_zscale;
    };

    const SwcTabConfig& getSwcTabConfig() const {
      return m_swcTabConfig;
    }

  private:
    bool m_isUtilsOn;
    bool m_isVolumeOn;
    bool m_isGraphOn;
    bool m_isSwcsOn;
    bool m_isTubesOn;
    bool m_isPunctaOn;
    bool m_isMeshOn;
    bool m_isTensorOn;
    bool m_isAxisOn;
    SwcTabConfig m_swcTabConfig;
  };

  class ObjManagerConfig {
  public:
    ObjManagerConfig();
    void loadXmlNode(const ZXmlNode *node);

    inline bool isSwcOn() const { return m_isSwcOn; }
    inline bool isCategorizedSwcNodeOn() const { return m_isSwcNodeOn; }
    inline bool isPunctaOn() const { return m_isPunctaOn; }

  private:
    bool m_isSwcOn;
    bool m_isSwcNodeOn;
    bool m_isPunctaOn;
  };

  inline const MainWindowConfig& getMainWindowConfig() const {
    return m_mainWindowConfig;
  }

  inline const Z3DWindowConfig& getZ3DWindowConfig() const {
    return m_z3dWindowConfig;
  }

  inline const ObjManagerConfig& getObjManagerConfig() const {
    return m_objManagerConfig;
  }

  inline ZMessageReporter* getMessageReporter() {
    return m_messageReporter;
  }

  inline bool isSettingOn() const { return m_isSettingOn; }

  inline int getAutoSaveInterval() const { return m_autoSaveInterval; }
  inline void setAutoSaveDir(const std::string str) {
    m_autoSaveDir = str;
  }
  bool isAutoSaveEnabled() const { return m_autoSaveEnabled; }

private:
  NeutubeConfig();
  NeutubeConfig(const NeutubeConfig&);
  ~NeutubeConfig();
  void operator=(const NeutubeConfig&);

private:
  std::string m_application;
  std::string m_applicationDir;
  std::string m_segmentationClassifierPath;
  std::string m_segmentationTrainingTestPath;
  std::string m_segmentationTrainingTruthPath;
  std::string m_segmentationEvaluationTestPath;
  std::string m_segmentationEvaluationTruthPath;
  std::string m_swcRepository;
  double m_segmentationClassifThreshold;
  std::string m_dataPath;
  std::string m_developPath;
  std::vector<std::string> m_bodyConnectionFeature;
  MainWindowConfig m_mainWindowConfig;
  Z3DWindowConfig m_z3dWindowConfig;
  ObjManagerConfig m_objManagerConfig;
  bool m_isSettingOn;
  bool m_isStereoOn;
  std::string m_autoSaveDir;
  int m_autoSaveInterval;
  bool m_autoSaveEnabled;

  ZMessageReporter *m_messageReporter;
};

#define GET_DATA_DIR (NeutubeConfig::getInstance().getPath(NeutubeConfig::DATA))
#if defined(PROJECT_PATH)
#define GET_TEST_DATA_DIR (std::string(PROJECT_PATH) + "/../data")
#endif

#ifndef GET_TEST_DATA_DIR
#define GET_TEST_DATA_DIR GET_DATA_DIR
#endif


#define GET_MESSAGE_REPORTER (NeutubeConfig::getInstance().getMessageReporter())
#define GET_APPLICATION_NAME (NeutubeConfig::getInstance().getApplication())

#endif // NEUTUBECONFIG_H
