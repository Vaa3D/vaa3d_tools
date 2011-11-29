#include "itkEventToQtSignalTranslator.h"

namespace itk {


EventsToQtSignalTranslator
::EventsToQtSignalTranslator()
{
  this->m_Command = CommandType::New();
  this->m_Command->SetCallbackFunction( this, & EventsToQtSignalTranslator::EmitSignal );
}

EventsToQtSignalTranslator
::~EventsToQtSignalTranslator()
{
}

EventsToQtSignalTranslator::CommandType *
EventsToQtSignalTranslator
::GetCommand()
{
  return this->m_Command;
}

itk::ProcessObject *
EventsToQtSignalTranslator
::GetObservedFilter()
{
  return this->m_ObservedFilter;
}

void
EventsToQtSignalTranslator
::SetObservedFilter( itk::ProcessObject * filter )
{
  this->m_ObservedFilter = filter;
}


//
// Method for the ProgressValuedEventToQtSignalTranslator
//
void
ProgressValuedEventToQtSignalTranslator
::EmitSignal( const EventObject & event )
{
  const ProgressEvent * progressEvent =
    dynamic_cast< const ProgressEvent * >( &event );
  if( progressEvent )
    {
    double progress = this->GetObservedFilter()->GetProgress();
    int integerProgress = static_cast<int>( progress * 100.0 );
    if( integerProgress < 0 )
      {
      integerProgress = 0;
      }
    if( integerProgress > 100 )
      {
      integerProgress = 100;
      }
    this->UpdateProgress(integerProgress);
    }
}


//
// Method for the IterationEventToQtSignalTranslator
//
IterationEventToQtSignalTranslator
::IterationEventToQtSignalTranslator()
{
  this->m_NumberOfIterationsSoFar = 0;
}

IterationEventToQtSignalTranslator
::~IterationEventToQtSignalTranslator()
{
}

void
IterationEventToQtSignalTranslator
::ResetIterationCounter()
{
  this->m_NumberOfIterationsSoFar = 0;
}

void
IterationEventToQtSignalTranslator
::EmitSignal( const EventObject & event )
{
  const IterationEvent * iterationEvent =
    dynamic_cast< const IterationEvent * >( &event );
  if( iterationEvent )
    {
    this->m_NumberOfIterationsSoFar++;
    this->UpdateIterations();
    this->UpdateIterations( this->m_NumberOfIterationsSoFar );
    }
}


} // end namespace itk
