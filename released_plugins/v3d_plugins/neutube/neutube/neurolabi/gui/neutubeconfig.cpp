#include "neutubeconfig.h"

#include <iostream>

#include "tz_cdefs.h"
#include "zxmldoc.h"
#include "zstring.h"
#include "zlogmessagereporter.h"

using namespace std;

NeutubeConfig::NeutubeConfig() : m_segmentationClassifThreshold(0.5),
  m_isSettingOn(true), m_isStereoOn(true), m_autoSaveInterval(600000),
  m_autoSaveEnabled(true)
{
  m_messageReporter = new ZLogMessageReporter;
}

NeutubeConfig::NeutubeConfig(const NeutubeConfig& config) :
  m_segmentationClassifThreshold(0.5), m_isSettingOn(true)
{
  UNUSED_PARAMETER(&config);
  m_messageReporter = new ZLogMessageReporter;
}

NeutubeConfig::~NeutubeConfig()
{
  delete m_messageReporter;
}

void NeutubeConfig::operator=(const NeutubeConfig& config)
{
  UNUSED_PARAMETER(&config);
}

bool NeutubeConfig::load(const std::string &filePath)
{
#ifdef _DEBUG_
  cout << "Loading configuration ..." << endl;
#endif

  if (fexist(filePath.c_str())) {
    ZXmlDoc doc;
    doc.parseFile(filePath);

    doc.printInfo();

    ZXmlNode node =
        doc.getRootElement().queryNode("FlyEmQASegmentationClassifier");
    if (!node.empty()) {
      m_segmentationClassifierPath =
          ZString::absolutePath(m_applicationDir, node.stringValue());
    }

    node = doc.getRootElement().queryNode("dataPath");
    m_dataPath = node.stringValue();

    node = doc.getRootElement().queryNode("developPath");
    m_developPath = node.stringValue();

    if (m_dataPath.empty() && !m_developPath.empty()) {
      m_dataPath = m_developPath + "/neurolabi/data";
    }

    node = doc.getRootElement().queryNode("FlyEmQASegmentationTraining");
    if (!node.empty()) {
      ZXmlNode childNode = node.queryNode("test");
      m_segmentationTrainingTestPath = childNode.stringValue();
      childNode = node.queryNode("truth");
      m_segmentationTrainingTruthPath = childNode.stringValue();
      childNode = node.queryNode("feature");
      ZString str = childNode.stringValue();
      m_bodyConnectionFeature = str.toWordArray();
    }

    node = doc.getRootElement().queryNode("FlyEmQASegmentationEvaluation");
    if (!node.empty()) {
      ZXmlNode childNode = node.queryNode("test");
      m_segmentationEvaluationTestPath = childNode.stringValue();
      childNode = node.queryNode("truth");
      m_segmentationEvaluationTruthPath = childNode.stringValue();
      childNode = node.queryNode("threshold");
      m_segmentationClassifThreshold = childNode.doubleValue();
    }

    node = doc.getRootElement().queryNode("SwcRepository");
    if (!node.empty()) {
      m_swcRepository = node.stringValue();
    }

    node = doc.getRootElement().queryNode("Settings");
    if (!node.empty()) {
      if (node.getAttribute("status") == "off") {
        m_isSettingOn = false;
      }
    }

    node = doc.getRootElement().queryNode("Stereo");
    if (!node.empty()) {
      if (node.getAttribute("status") == "off") {
        m_isStereoOn = false;
      }
    }

    node = doc.getRootElement().queryNode("MainWindow");
    if (!node.empty()) {
      m_mainWindowConfig.loadXmlNode(&node);
    }

    node = doc.getRootElement().queryNode("Z3DWindow");
    if (!node.empty()) {
      m_z3dWindowConfig.loadXmlNode(&node);
    }

    node = doc.getRootElement().queryNode("Application");
    if (!node.empty()) {
      m_application = node.stringValue();
      if (m_application == "Biocytin") {
        m_mainWindowConfig.setExpandSwcWith3DWindow(true);
      }
    }

    node = doc.getRootElement().queryNode("Object_Manager");
    if (!node.empty()) {
      m_objManagerConfig.loadXmlNode(&node);
    }

    node = doc.getRootElement().queryNode("Autosave");
    if (!node.empty()) {
      ZXmlNode childNode = node.queryNode("Enabled");
      if (!childNode.empty()) {
        m_autoSaveEnabled = childNode.intValue();
      }

      childNode = node.queryNode("Interval");
      if (!childNode.empty()) {
        m_autoSaveInterval = childNode.intValue();
      }
    }

    ZLogMessageReporter *reporter =
        dynamic_cast<ZLogMessageReporter*>(m_messageReporter);
    if (reporter != NULL) {
      reporter->setInfoFile(
            ZString::fullPath(getApplicatinDir(), "neutube_appout", "txt"));
      reporter->setWarnFile(
            ZString::fullPath(getApplicatinDir(), "neutube_warn", "txt"));
      reporter->setErrorFile(
            ZString::fullPath(getApplicatinDir(), "neutube_error", "txt"));
    }

    return true;
  }

  return false;
}

void NeutubeConfig::print()
{
  cout << m_dataPath << endl;
  cout << "SWC repository: " << m_swcRepository << endl;
  cout << "Body connection classifier: " << m_segmentationClassifierPath << endl;
  cout << "Body connection training: " << endl;
  cout << "  data: " << m_segmentationTrainingTestPath << endl;
  cout << "  ground truth: " << m_segmentationTrainingTruthPath << endl;
  cout << "Body connection evaluation: " << endl;
  cout << "  data: " << m_segmentationEvaluationTestPath << endl;
  cout << "  ground truth: " << m_segmentationEvaluationTruthPath << endl;

  cout << "Bcf: ";
  for (vector<string>::const_iterator iter = m_bodyConnectionFeature.begin();
       iter != m_bodyConnectionFeature.end(); ++iter) {
    cout << *iter << " ";
  }
  cout << endl;
  cout << "Application dir: " << getApplicatinDir() << endl;
  cout << "Autosave dir: " << getPath(AUTO_SAVE) << endl;
  cout << "Autosave interval: " << m_autoSaveInterval << endl;
  cout << endl;
}

std::string NeutubeConfig::getPath(Config_Item item) const
{
  switch (item) {
  case DATA:
    return m_dataPath;
  case FLYEM_BODY_CONN_CLASSIFIER:
    return m_segmentationClassifierPath;
  case FLYEM_BODY_CONN_TRAIN_DATA:
    return m_segmentationTrainingTestPath;
  case FLYEM_BODY_CONN_TRAIN_TRUTH:
    return m_segmentationTrainingTruthPath;
  case FLYEM_BODY_CONN_EVAL_DATA:
    return m_segmentationEvaluationTestPath;
  case FLYEM_BODY_CONN_EVAL_TRUTH:
    return m_segmentationEvaluationTruthPath;
  case CONFIGURE_FILE:
    return getConfigPath();
  case SWC_REPOSOTARY:
    return m_swcRepository;
  case AUTO_SAVE:
    if (m_autoSaveDir.empty()) {
      return getApplicatinDir() + ZString::FileSeparator + "autosave";
    }
    return m_autoSaveDir;
  default:
    break;
  }

  return "";
}

NeutubeConfig::MainWindowConfig::MainWindowConfig() : m_tracingOn(true),
  m_isMarkPunctaOn(true), m_isSwcEditOn(true), m_isExpandSwcWith3DWindow(false),
  m_isProcessBinarizeOn(true), m_isMaskToSwcOn(true), m_isBinaryToSwcOn(true),
  m_isThresholdControlOn(true)
{
}

void NeutubeConfig::MainWindowConfig::loadXmlNode(const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("tracing");
  if (!childNode.empty()) {
    enableTracing(childNode.getAttribute("status") != "off");
  } else {
    enableTracing(true);
  }

  childNode = node->queryNode("markPuncta");
  if (!childNode.empty()) {
    enableMarkPuncta(childNode.getAttribute("status") != "off");
  } else {
    enableMarkPuncta(true);
  }

  childNode = node->queryNode("swcEdit");
  if (!childNode.empty()) {
    enableSwcEdit(childNode.getAttribute("status") != "off");
  } else {
    enableSwcEdit(true);
  }

  childNode = node->queryNode("mergeImage");
  if (!childNode.empty()) {
    enableMergeImage(childNode.getAttribute("status") != "off");
  } else {
    enableMergeImage(true);
  }

  childNode = node->queryNode("Expand");
  if (!childNode.empty()) {
    ZXmlNode expandNode = childNode.queryNode("Neuron_Network");
    if (!expandNode.empty()) {
      m_isExpandNeuronNetworkOn = (expandNode.getAttribute("status") != "off");
    }

    expandNode = childNode.queryNode("Tracing_Result");
    if (!expandNode.empty()) {
      m_isExpandTracingResultOn = (expandNode.getAttribute("status") != "off");
    }

    expandNode = childNode.queryNode("V3D_APO");
    if (!expandNode.empty()) {
      m_isExpandV3dApoOn = (expandNode.getAttribute("status") != "off");
    }

    expandNode = childNode.queryNode("V3D_Marker");
    if (!expandNode.empty()) {
      m_isExpandV3dMarkerOn = (expandNode.getAttribute("status") != "off");
    }
  }

  childNode = node->queryNode("Process");
  if (!childNode.empty()) {
    ZXmlNode processNode = childNode.queryNode("Binarize");
    if (!processNode.empty()) {
      m_isProcessBinarizeOn = (processNode.getAttribute("status") != "off");
    }
  }

  childNode = node->queryNode("Trace");
  if (!childNode.empty()) {
    ZXmlNode traceNode = childNode.queryNode("Binary_To_Swc");
    if (!traceNode.empty()) {
      m_isBinaryToSwcOn = (traceNode.getAttribute("status") != "off");
    }

    traceNode = childNode.queryNode("Mask_To_Swc");
    if (!traceNode.empty()) {
      m_isMaskToSwcOn = (traceNode.getAttribute("status") != "off");
    }
  }

  childNode = node->queryNode("Threshold_Control");
  if (!childNode.empty()) {
    setThresholdControl(childNode.getAttribute("status") != "off");
  }
}

NeutubeConfig::Z3DWindowConfig::Z3DWindowConfig() : m_isUtilsOn(true),
  m_isVolumeOn(true), m_isGraphOn(true), m_isSwcsOn(true), m_isTubesOn(true),
  m_isPunctaOn(true), m_isMeshOn(true), m_isTensorOn(true), m_isAxisOn(true)
{
}

void NeutubeConfig::Z3DWindowConfig::loadXmlNode(const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("Utils");
  if (!childNode.empty()) {
    enableUtils(childNode.getAttribute("status") != "off");
  } else {
    enableUtils(true);
  }

  childNode = node->queryNode("Volume");
  if (!childNode.empty()) {
    enableVolume(childNode.getAttribute("status") != "off");
  } else {
    enableVolume(true);
  }

  childNode = node->queryNode("Graph");
  if (!childNode.empty()) {
    enableGraph(childNode.getAttribute("status") != "off");
  } else {
    enableGraph(true);
  }

  childNode = node->queryNode("Swcs");
  if (!childNode.empty()) {
    enableSwcs(childNode.getAttribute("status") != "off");
    m_swcTabConfig.loadXmlNode(&childNode);
  } else {
    enableSwcs(true);
  }

  childNode = node->queryNode("Tubes");
  if (!childNode.empty()) {
    enableTubes(childNode.getAttribute("status") != "off");
  } else {
    enableTubes(true);
  }

  childNode = node->queryNode("Puncta");
  if (!childNode.empty()) {
    enablePuncta(childNode.getAttribute("status") != "off");
  } else {
    enablePuncta(true);
  }

  childNode = node->queryNode("Tensor");
  if (!childNode.empty()) {
    enableTensor(childNode.getAttribute("status") != "off");
  } else {
    enableTensor(true);
  }

  childNode = node->queryNode("Mesh");
  if (!childNode.empty()) {
    enableMesh(childNode.getAttribute("status") != "off");
  } else {
    enableMesh(true);
  }
}

NeutubeConfig::Z3DWindowConfig::SwcTabConfig::SwcTabConfig() : m_primitive("Normal"),
  m_colorMode("Branch Type"), m_zscale(1.0)
{
}


void NeutubeConfig::Z3DWindowConfig::SwcTabConfig::loadXmlNode(const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("Primitive");
  if (!childNode.empty()) {
    m_primitive = childNode.stringValue();
  }

  childNode = node->queryNode("Color_Mode");
  if (!childNode.empty()) {
    m_colorMode = childNode.stringValue();
  }

  childNode = node->queryNode("ZScale");
  if (!childNode.empty()) {
    m_zscale = childNode.doubleValue();
  }
}

NeutubeConfig::ObjManagerConfig::ObjManagerConfig() :
  m_isSwcOn(true), m_isSwcNodeOn(true), m_isPunctaOn(true)
{

}

void NeutubeConfig::ObjManagerConfig::loadXmlNode(
    const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("Swc");
  if (!childNode.empty()) {
    m_isSwcOn = (childNode.getAttribute("status") != "off");
  } else {
    m_isSwcOn = true;
  }

  childNode = node->queryNode("Swc_Node");
  if (!childNode.empty()) {
    m_isSwcNodeOn = (childNode.getAttribute("status") != "off");
  } else {
    m_isSwcNodeOn = true;
  }

  childNode = node->queryNode("Puncta");
  if (!childNode.empty()) {
    m_isPunctaOn = (childNode.getAttribute("status") != "off");
  } else {
    m_isPunctaOn = true;
  }
}
