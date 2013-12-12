#ifndef ZFLYEMDATABUNDLE_H
#define ZFLYEMDATABUNDLE_H

#include <vector>
#include <string>
#include <map>

#include "zflyemneuron.h"
#include "flyem/zsynapseannotationarray.h"
#include "neutube.h"

class ZFlyEmDataBundle
{
public:
  ZFlyEmDataBundle();
  ~ZFlyEmDataBundle();

  enum EComponent {
    SYNAPSE_ANNOTATION, COLOR_MAP, ALL_COMPONENT
  };

  bool isDeprecated(EComponent comp) const;
  void deprecate(EComponent comp);
  void deprecateDependent(EComponent comp);

  bool loadJsonFile(const std::string &filePath);
  std::string toSummaryString() const;
  std::string toDetailString() const;
  void print() const;

  std::string getModelPath(int bodyId) const;
  ZSwcTree* getModel(int bodyId) const;
  std::string getName(int bodyId) const;
  int getIdFromName(const std::string &name) const;

  bool hasNeuronName(const std::string &name) const;

  const std::vector<ZFlyEmNeuron>& getNeuronArray() const {
    return m_neuronArray;
  }

  std::vector<ZFlyEmNeuron>& getNeuronArray() {
    return m_neuronArray;
  }

  inline const std::string& getSource() const { return m_source; }

  //Return the pointer to the neuron with id <bodyId>. It returns NULL if no
  //such id is found.
  const ZFlyEmNeuron* getNeuron(int bodyId) const;
  ZFlyEmNeuron* getNeuron(int bodyId);
  const ZFlyEmNeuron* getNeuronFromName(const std::string &name) const;


  FlyEm::ZSynapseAnnotationArray *getSynapseAnnotation() const;
  std::map<int, QColor> *getColorMap() const;
  inline const std::map<std::string, double>& getMatchThresholdMap() const {
    return m_matchThreshold;
  }

  int countClass() const;
  int countNeuronByClass(const std::string &className) const;
  //double getZResolution() const { return m_swcResolution[2]; }

  void updateNeuronConnection();

  /*!
   * \brief Get image resolution along a certain axis
   */
  double getImageResolution(NeuTube::EAxis axis);

  /*!
   * \brief Get SWC resolution along a certain axis
   */
  double getSwcResolution(NeuTube::EAxis axis);

  /*!
   * \brief Export the bundle into a json file
   */
  void exportJsonFile(const std::string &path) const;

  /*!
   * \brief Get the Z coordinate of the start of a layer
   *
   * \a layer must be in [1, layer number].
   */
  double getLayerStart(int layer);

  /*!
   * \brief Get the Z coordinate of the end of a layer
   *
   * \a layer must be in [1, layer number].
   */
  double getLayerEnd(int layer);

  /*!
   * \brief Test if a neuron hits layer
   *
   * \param bodyId ID of the neuron
   * \param top The top layer
   * \param bottom The bottom layer
   * \param isExclusive Exclusively in the range or not
   * \return true iff the skeleton of neuron hits any point between the layer
   *         \a top and the layer \a bottom, and does not hit any other layer
   *         when \a isExclusive is true.
   */
  bool hitsLayer(int bodyId, int top, int bottom, bool isExclusive);

  bool hitsLayer(const ZFlyEmNeuron &neuron, int top, int bottom,
                 bool isExclusive);

private:
  std::vector<ZFlyEmNeuron> m_neuronArray;
  std::string m_synapseAnnotationFile;
  std::string m_grayScalePath;
  //std::string m_configFile;
  std::string m_neuronColorFile;
  double m_swcResolution[3];
  double m_imageResolution[3];
  int m_sourceOffset[3];
  int m_sourceDimension[3];
  double m_synapseScale;

  std::string m_source;
  std::map<std::string, double> m_matchThreshold;

  mutable FlyEm::ZSynapseAnnotationArray *m_synaseAnnotation;
  mutable std::map<int, QColor> *m_colorMap;

  static const char *m_synapseKey;
  static const char *m_grayScaleKey;
  static const char *m_configKey;
  static const char *m_neuronColorKey;
  static const char *m_synapseScaleKey;
  static const char *m_sourceOffsetKey;
  static const char *m_sourceDimensionKey;
  static const char *m_imageResolutionKey;
  static const char *m_neuronKey;
  static const char *m_swcResolutionKey;
  static const char *m_matchThresholdKey;

  const static int m_layerNumber;
  const static double m_layerRatio[11];
};

#endif // ZFLYEMDATABUNDLE_H
