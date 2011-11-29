#ifndef __V3DITKProgressDialog_H__
#define __V3DITKProgressDialog_H__

#include <QtGui>
#include <QtGui/QLabel>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDialog>
#include <QtGui/QProgressBar>

#include "itkEventToQtSignalTranslator.h"


class V3DITKProgressDialog : public QDialog
{
  Q_OBJECT

public:
  V3DITKProgressDialog();
  ~V3DITKProgressDialog();

  explicit V3DITKProgressDialog( QWidget* iParent = 0 );

  V3DITKProgressDialog( const char * name );

  bool HasCancelBeenPressed() const;

  typedef itk::EventsToQtSignalTranslator  EventToQtSignalTranslatorType;
  typedef EventToQtSignalTranslatorType::CommandType    CommandType;

  CommandType * GetCommand();

  void ObserveFilter( itk::ProcessObject * filter );

  int exec();

public slots:

  /** progress value in the range from 0 to 100  */
  void SetProgressValue( int progress );

  void reject();

private:

  std::string   pluginName;

  bool CancelButtonHasBeenPressed;

  QGridLayout  * gridLayout;
  QProgressBar * progressBar;
  QPushButton  * cancel;

  Q_DISABLE_COPY(V3DITKProgressDialog);

  itk::ProgressValuedEventToQtSignalTranslator  * progressTranslator;
};

#endif
