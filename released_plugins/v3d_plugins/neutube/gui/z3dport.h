#ifndef Z3DPORT_H
#define Z3DPORT_H

#include "QsLog.h"
#include "z3dprocessor.h"

#include <vector>

class Z3DPort
{
  friend class Z3DProcessor;

public:

  Z3DPort(const QString &name, bool allowMultipleConnections = false,
          Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult);
  virtual ~Z3DPort();

  bool allowMultipleConnections() const { return m_allowMultipleConnections; }

  // return the processor this port belongs to.
  Z3DProcessor* getProcessor() const { return m_processor; }

  QString getName() const { return m_name; }

  bool isInitialized() const { return m_isInitialized; }

  virtual void setProcessor(Z3DProcessor* p);

protected:

  virtual void initialize();
  virtual void deinitialize();

  QString m_name;
  Z3DProcessor* m_processor;
  bool m_allowMultipleConnections;

  // how changes from this port affect its processor
  Z3DProcessor::InvalidationState m_invalidationState;

private:
  bool m_isInitialized;
};

class Z3DOutputPortBase;

class Z3DInputPortBase : public Z3DPort
{
  friend class Z3DOutputPortBase;
public:
  Z3DInputPortBase(const QString &name, bool allowMultipleConnections = false,
                   Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult);
  virtual ~Z3DInputPortBase();

  // invalidate processor with the given InvalidationState and set hasChanged=true.
  void invalidate();
  // has the data in this port changed since the last process() call?
  bool hasChanged() const { return m_hasChanged; }
  // mark the port as valid.
  void setValid() { m_hasChanged = false; }

  const std::vector<Z3DOutputPortBase*> getConnected() const { return m_connectedOutputPorts; }
  virtual size_t getNumConnections() const { return m_connectedOutputPorts.size(); }
  bool isConnected() const { return !m_connectedOutputPorts.empty(); }
  bool isConnectedTo(const Z3DOutputPortBase* port) const;

  // return true if the port is connected and contains valid data.
  virtual bool isReady() const = 0;

  virtual bool connect(Z3DOutputPortBase* outport);
  virtual void disconnect(Z3DOutputPortBase* outport);
  virtual void disconnectAll();

protected:
  std::vector<Z3DOutputPortBase*> m_connectedOutputPorts;
  bool m_hasChanged;
};

class Z3DOutputPortBase : public Z3DPort
{
public:
  Z3DOutputPortBase(const QString &name, bool allowMultipleConnections = true,
                    Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult);

  virtual ~Z3DOutputPortBase();

  const std::vector<Z3DInputPortBase*> getConnected() const { return m_connectedInputPorts; }

  // test if this outport can connect to a given inport.
  virtual bool canConnectTo(const Z3DInputPortBase* inport) const;

  // invalidate all connected inports.
  virtual void invalidate();

  virtual size_t getNumConnections() const { return m_connectedInputPorts.size(); }

  bool isConnected() const { return !m_connectedInputPorts.empty(); }

  bool isConnectedTo(const Z3DInputPortBase* port) const;

  // returns whether the port is ready to be used by its owning processor.
  // return true if the port is connected.
  virtual bool isReady() const { return isConnected(); }

  // return true if this output port contains valid data
  virtual bool hasValidData() const = 0;

  virtual bool connect(Z3DInputPortBase* inport);
  virtual void disconnect(Z3DInputPortBase* inport);
  virtual void disconnectAll();

protected:

  std::vector<Z3DInputPortBase*> m_connectedInputPorts;
};

template<class T> class Z3DInputPort;
template<typename T>
class Z3DOutputPort : public Z3DOutputPortBase
{
public:
  Z3DOutputPort(const QString& name, bool allowMultipleConnections = true,
                Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult)
    : Z3DOutputPortBase(name, allowMultipleConnections, invalidationState)
    , m_portData(0)
    , m_ownsData(false)
  {}

  virtual ~Z3DOutputPort()
  {
    if (m_ownsData)
      delete m_portData;
  }

  virtual bool canConnectTo(const Z3DInputPortBase* inport) const
  {
    if (dynamic_cast<const Z3DInputPort<T>*>(inport))
      return Z3DOutputPortBase::canConnectTo(inport);
    else
      return false;
  }

  virtual void setData(T* data, bool takeOwnership = false)
  {
    // is it possible that the new allocated data has the same address as the old one???
    if (data != m_portData) {
      if (m_ownsData)
        delete m_portData;
      m_portData = data;
      m_ownsData = takeOwnership;
      invalidate();
    }
  }

  // return the data stored in this port
  virtual T *getData() const
  {
    return m_portData;
  }

  virtual bool hasValidData() const { return m_portData != NULL; }

  std::vector<const Z3DInputPort<T>* > getConnected() const
  {
    std::vector<const Z3DInputPort<T>*> ports;
    for (size_t i = 0; i < m_connectedInputPorts.size(); ++i) {
      Z3DInputPort<T>* p = static_cast<Z3DInputPort<T>*>(m_connectedInputPorts[i]);
      ports.push_back(p);
    }
    return ports;
  }

protected:
  T* m_portData;
  bool m_ownsData;
};

template<typename T>
class Z3DInputPort : public Z3DInputPortBase
{
public:
  Z3DInputPort(const QString& name, bool allowMultipleConnections = true,
               Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult)
    : Z3DInputPortBase(name, allowMultipleConnections, invalidationState)
  {}

  virtual ~Z3DInputPort() {}


  // return first valid data stored in the connected outports
  T *getFirstValidData() const
  {
    for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
      Z3DOutputPort<T>* p = static_cast< Z3DOutputPort<T>* >(m_connectedOutputPorts[i]);
      if (p->hasValidData())
        return p->getData();
    }
    return NULL;
  }

  // return all valid data stored in the connected outports
  std::vector<T*> getAllValidData() const
  {
    std::vector<T*> allData;

    for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
      Z3DOutputPort<T>* p = static_cast<Z3DOutputPort<T>*>(m_connectedOutputPorts[i]);
      if (p->hasValidData())
        allData.push_back(p->getData());
    }

    return allData;
  }

  std::vector<const Z3DOutputPort<T>* > getConnected() const
  {
    std::vector<const Z3DOutputPort<T>*> ports;
    for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
      Z3DOutputPort<T>* p = static_cast<Z3DOutputPort<T>*>(m_connectedOutputPorts[i]);
      ports.push_back(p);
    }
    return ports;
  }

  virtual bool isReady() const { return getFirstValidData() != NULL; }
};




template <typename T>
class Z3DProcessorInputPort : public Z3DInputPortBase
{
public:
  Z3DProcessorInputPort(const QString& name, bool allowMultipleConnections = true,
                        Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult)
    : Z3DInputPortBase(name, allowMultipleConnections, invalidationState)
  {
  }

  std::vector<T*> getAllConnectedProcessors() const
  {
    std::vector<T*> processors;
    for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
      T* p = static_cast<T*>(m_connectedOutputPorts[i]->getProcessor());
      processors.push_back(p);
    }
    return processors;
  }

  T* getFirstConnectedProcessor() const
  {
    if (isConnected())
      return static_cast<T*>(m_connectedOutputPorts[0]->getProcessor());
    else
      return 0;
  }

  virtual bool isReady() const { return isConnected(); }
};

template <typename T>
class Z3DProcessorOutputPort : public Z3DOutputPortBase
{
public:
  Z3DProcessorOutputPort(const QString& name, bool allowMultipleConnections = false,
                         Z3DProcessor::InvalidationState invalidationState = Z3DProcessor::InvalidAllResult)
    : Z3DOutputPortBase(name, allowMultipleConnections, invalidationState)
  {
  }

  virtual bool canConnectTo(const Z3DInputPortBase* inport) const
  {
    if (dynamic_cast<const Z3DProcessorInputPort<T>*>(inport))
      return Z3DOutputPortBase::canConnectTo(inport);
    else
      return false;
  }

  // data is processor itself, so it is always valid
  virtual bool hasValidData() const { return true; }

protected:
  virtual void setProcessor(Z3DProcessor *p)
  {
    Z3DOutputPortBase::setProcessor(p);
    T* tp = dynamic_cast<T*>(p);
    if (!tp) {
      LERROR() << "Port" << getName() << "attached to processor of wrong type" << p->getClassName();
    }
  }
};


#endif // Z3DPORT_H
