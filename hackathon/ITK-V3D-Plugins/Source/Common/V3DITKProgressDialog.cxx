#include "V3DITKProgressDialog.h"

// DEBUGGING
#include <iostream>

V3DITKProgressDialog::V3DITKProgressDialog()
{
}

V3DITKProgressDialog::V3DITKProgressDialog( QWidget * )
{
}


V3DITKProgressDialog::~V3DITKProgressDialog()
{
  delete this->progressBar;
  delete this->cancel;
  delete this->gridLayout;
  delete this->progressTranslator;
}

V3DITKProgressDialog::V3DITKProgressDialog( const char * name )
{
  this->pluginName = name;

  this->CancelButtonHasBeenPressed = false;

  this->progressBar = new QProgressBar(this);
  this->cancel = new QPushButton("Cancel");

  this->progressBar->setObjectName(QString::fromUtf8("progressBar"));
  this->progressBar->setGeometry(QRect(1, 1, 200, 25));
  this->progressBar->setRange(0, 100);
  this->progressBar->setValue(0);

  this->gridLayout = new QGridLayout();

  this->CancelButtonHasBeenPressed = false;

  this->gridLayout->addWidget( this->progressBar, 0,1);
  this->gridLayout->addWidget( this->cancel, 0,0);

  this->setLayout(gridLayout);

  this->setWindowTitle( QObject::tr( this->pluginName.c_str() ) );

  this->progressTranslator = new itk::ProgressValuedEventToQtSignalTranslator;

  this->connect( this->cancel, SIGNAL(clicked()), this, SLOT(reject()) );
  this->connect( this->progressTranslator , SIGNAL(UpdateProgress(int)),this, SLOT(SetProgressValue(int)));
}

int V3DITKProgressDialog::exec()
{
  return this->QDialog::exec();
}


V3DITKProgressDialog::CommandType *
V3DITKProgressDialog::GetCommand()
{
  return this->progressTranslator->GetCommand();
}


void
V3DITKProgressDialog::ObserveFilter( itk::ProcessObject * filter )
{
  this->progressTranslator->SetObservedFilter( filter );
}

void V3DITKProgressDialog::setFilter(itk::ProcessObject* filter)
{
  this->m_filter = filter;
}
void V3DITKProgressDialog::stopFilter()
{
  qDebug() << " stop filter ";
  //this->progressTranslator->GetObservedFilter()->SetAbortGenerateData(true); 
  this->m_filter->SetAbortGenerateData(true);
  this->close();
}

void V3DITKProgressDialog::SetProgressValue( int progressValue )
{
  this->progressBar->setValue( progressValue );
  if (this->CancelButtonHasBeenPressed) {
    this->progressTranslator->GetObservedFilter()->SetAbortGenerateData(true); 
    this->close();
  }
  // Give a chance to process other events (including the potential click on the cancel button)
  QApplication::processEvents( QEventLoop::AllEvents );
}

void V3DITKProgressDialog::reject()
{
  this->CancelButtonHasBeenPressed = true;
  qDebug() << "cancel button clicked";
  //emit cancelButtonClicked();
}

bool V3DITKProgressDialog::HasCancelBeenPressed() const
{
  return this->CancelButtonHasBeenPressed;
}

