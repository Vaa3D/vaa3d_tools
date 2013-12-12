#ifndef Z3DPROCESSOR_H
#define Z3DPROCESSOR_H

#include <QObject>
#include <vector>
#include <map>
#include <set>
#include "z3dgl.h"
#include "z3dcanvaseventlistener.h"

class Z3DInputPortBase;
class Z3DOutputPortBase;
class Z3DInteractionHandler;
class ZParameter;
class ZEventListenerParameter;
class QString;

class Z3DProcessor : public QObject, public Z3DCanvasEventListener
{
  Q_OBJECT

  friend class Z3DNetworkEvaluator;

public:
  // specifies the invalidation status of the processor.
  // The networkEvaluator use this value to mark processors that has to be processed
  enum ___InvalidationState {
    Valid = 0x00,
    InvalidMonoViewResult = 0x01,
    InvalidLeftEyeResult = 0x02,
    InvalidRightEyeResult = 0x04,
    InvalidStereoResult = InvalidLeftEyeResult | InvalidRightEyeResult,
    InvalidAllResult = InvalidMonoViewResult | InvalidStereoResult
  };
  Q_DECLARE_FLAGS(InvalidationState, ___InvalidationState)

  Z3DProcessor(const QString &name = "");
  virtual ~Z3DProcessor();

  virtual QString getClassName() const {return metaObject()->className();}
  void setName(const QString& name) { m_name = name; }
  QString getName() const { return m_name; }

  // returns all parameters
  const std::vector<ZParameter*>& getParameters() const { return m_parameters; }
  // returns first parameter with the given name. return NULL if not found
  ZParameter* getParameter(const QString& name) const;
  // returns all parameters matching the specified type T, including subtypes.
  template<class T>
  std::vector<T*> getParametersByType() const;

  bool isInitialized() const { return m_initialized; }

  virtual void invalidate(InvalidationState inv = InvalidAllResult);

  // returns the port with the given name, or null if such a port does not exist.
  Z3DInputPortBase* getInputPort(const QString &name) const;
  Z3DOutputPortBase* getOutputPort(const QString& name) const;
  // return all inputports or outputports as vector
  const std::vector<Z3DInputPortBase*>& getInputPorts() const { return m_inputPorts; }
  const std::vector<Z3DOutputPortBase*>& getOutputPorts() const { return m_outputPorts; }

  virtual void onEvent(QEvent* e, int w, int h);

  const std::vector<ZEventListenerParameter*> getEventListeners() const { return m_eventListeners; }
  const std::vector<Z3DInteractionHandler*>& getInteractionHandlers() const { return m_interactionHandlers; }

  // removes all port connections
  void disconnectAllPorts();

public slots:
  inline void invalidateResult() { invalidate(InvalidAllResult); }

protected:
  // mark that the output of current processor for certain eye is valid.
  // if process function (e.g. prepare data) is not related to stereo view or mono view, you should rewrite this
  // function in subclass and set the invalidstate to VALID to avoid being processed again for
  // a different eye parameter
  // this function will be called by networkevaluator after process(eye) is called
  virtual void setValid(Z3DEye eye);

  // return true if the output of current processor for certain eye is valid.
  // will be used by networkevalutor to decide whether is neccessary to call process(eye)
  virtual bool isValid(Z3DEye eye) const;

  // returns true if processor is ready to do rendering
  // The default implementation checks, whether the processor has been initialized and
  // all input ports and output ports are ready. This is not always necessary since not all
  // input or output ports are needed depending on rendering context.
  virtual bool isReady(Z3DEye eye) const;

  // do neccessary initiation here, especially opengl related, load shaders or renderers ...
  // superclass's version must be called as first statement
  // this method will be called by networkevaluator if neccessary
  virtual void initialize();

  // basically do the opposite of what initialize() do.
  // superclass's version must be called as last statement
  // this method will be called by networkevaluator if neccessary
  virtual void deinitialize();

  // this is the place to do rendering related work
  // the networkevaluator will sets its invalidation level to VALID after calling this
  // input is current camera (eye), can be left or right in stereo case
  virtual void process(Z3DEye eye) = 0;

  void addPort(Z3DInputPortBase *port);
  void addPort(Z3DInputPortBase &port)
    { addPort(&port); }
  void addPort(Z3DOutputPortBase *port);
  void addPort(Z3DOutputPortBase &port)
    { addPort(&port); }

  void removePort(Z3DInputPortBase *port);
  void removePort(Z3DInputPortBase &port)
    { removePort(&port); }
  void removePort(Z3DOutputPortBase *port);
  void removePort(Z3DOutputPortBase &port)
    { removePort(&port); }

  virtual void addParameter(ZParameter *para, InvalidationState inv = InvalidAllResult);
  virtual void addParameter(ZParameter &para, InvalidationState inv = InvalidAllResult)
    { addParameter(&para, inv); }
  virtual void removeParameter(ZParameter *para);
  virtual void removeParameter(ZParameter &para)
    { removeParameter(&para); }

  // listen to some events
  void addEventListener(ZEventListenerParameter *para);
  void addEventListener(ZEventListenerParameter &para)
    { addEventListener(&para); }

  // react to interaction
  void addInteractionHandler(Z3DInteractionHandler *handler);
  void addInteractionHandler(Z3DInteractionHandler &handler)
    { addInteractionHandler(&handler); }

  virtual void enterInteractionMode() {}
  virtual void exitInteractionMode() {}
  virtual bool isInInteractionMode() const;
  virtual void toggleInteractionMode(bool isInInteractionMode, void *source);

  // set to true after successful initialization.
  bool m_initialized;

  // used for the detection of duplicate port names.
  std::map<QString, Z3DInputPortBase*> m_inputPortMap;
  std::map<QString, Z3DOutputPortBase*> m_outputPortMap;

  InvalidationState m_invalidationState;
  std::set<void*> m_interactionModeSources;

private:
  QString m_name;

  // all parameters that can change the render behavior
  std::vector<ZParameter*> m_parameters;

  // input the processor expects.
  std::vector<Z3DInputPortBase*> m_inputPorts;
  // output the processor generates.
  std::vector<Z3DOutputPortBase*> m_outputPorts;

  std::vector<ZEventListenerParameter*> m_eventListeners;
  std::vector<Z3DInteractionHandler*> m_interactionHandlers;

  // used for cycle prevention during invalidation propagation
  bool m_invalidationVisited;
};

template<class T>
std::vector<T*> Z3DProcessor::getParametersByType() const {
  std::vector<T*> result;
  for (size_t i=0; i<m_parameters.size(); ++i) {
    if (dynamic_cast<T*>(m_parameters[i]))
      result.push_back(dynamic_cast<T*>(m_parameters[i]));
  }
  return result;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Z3DProcessor::InvalidationState)

#endif // Z3DPROCESSOR_H
