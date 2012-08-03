#ifndef __itkEventToQtSignalTranslator_h
#define __itkEventToQtSignalTranslator_h

#include <qobject.h>
#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkCommand.h"
#include "itkProcessObject.h"


namespace itk {


/** \class EventsToQtSignalTranslator
 * \brief Class that interface listen to itk Events and invokes Qt Signals
 **/
class EventsToQtSignalTranslator : public QObject
{

  Q_OBJECT

public:
  typedef ReceptorMemberCommand<EventsToQtSignalTranslator> CommandType;

  EventsToQtSignalTranslator();

  virtual ~EventsToQtSignalTranslator();

  CommandType * GetCommand();

  void SetObservedFilter( itk::ProcessObject * filter );
  itk::ProcessObject * GetObservedFilter();


  virtual void EmitSignal( const EventObject & ) = 0;

private:
  CommandType::Pointer          m_Command;
  itk::ProcessObject::Pointer   m_ObservedFilter;
};


/** \class
 * \brief Class that converts itk::ProgressValuedEvents into Signals
 */
class ProgressValuedEventToQtSignalTranslator :
  public EventsToQtSignalTranslator
{

  Q_OBJECT

  virtual void EmitSignal( const EventObject & event );

signals:

   void UpdateProgress( int value );

};



/** \class
 * \brief Class that converts itk::IterationEvent into Signals
 */
class IterationEventToQtSignalTranslator :
  public EventsToQtSignalTranslator
{

  Q_OBJECT

  virtual void EmitSignal( const EventObject & event );

public:

  IterationEventToQtSignalTranslator();
  ~IterationEventToQtSignalTranslator();

  void ResetIterationCounter();

signals:

   void UpdateIterations();
   void UpdateIterations( int value );

private:

   unsigned int m_NumberOfIterationsSoFar;

};



} // end namespace

#endif
