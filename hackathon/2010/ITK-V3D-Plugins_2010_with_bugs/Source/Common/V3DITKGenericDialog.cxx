#include "V3DITKGenericDialog.h"

// DEBUGGING
#include <iostream>

V3DITKGenericDialog::V3DITKGenericDialog()
{
}

V3DITKGenericDialog::V3DITKGenericDialog( QWidget * )
{
}


V3DITKGenericDialog::~V3DITKGenericDialog()
{
  ElementContainerType::iterator eitr = this->elementContainer.begin();
  ElementContainerType::iterator eend = this->elementContainer.end();
  while( eitr != eend )
    {
    delete eitr->second;
    ++eitr;
    }
  this->elementContainer.clear();

  delete this->ok;
  delete this->cancel;
  delete this->gridLayout;
}

V3DITKGenericDialog::V3DITKGenericDialog( const char * name )
{
  this->pluginName = name;

  this->ok     = new QPushButton("OK");
  this->cancel = new QPushButton("Cancel");

  this->gridLayout = new QGridLayout();
}

int V3DITKGenericDialog::exec()
{
  // remove existing children from the gridLayout
  QLayoutItem *child;
  while ((child = this->gridLayout->takeAt(0)) != 0) {
     this->gridLayout->removeItem(child);
     delete child->widget();
     delete child;
  }

  int row = 0;

  ElementContainerType::iterator eitr = this->elementContainer.begin();
  ElementContainerType::iterator eend = this->elementContainer.end();
  while( eitr != eend )
    {
    this->gridLayout->addWidget( eitr->second->nameLabel, row,0);
    this->gridLayout->addWidget( eitr->second->valueBox, row++,1);

    ++eitr;
    }

  this->gridLayout->addWidget( this->cancel, row,0);
  this->gridLayout->addWidget( this->ok, row,1);

  this->setLayout(gridLayout);

  this->setWindowTitle( QObject::tr( this->pluginName.c_str() ) );

  this->connect( this->ok,     SIGNAL(clicked()), this, SLOT(accept()) );
  this->connect( this->cancel, SIGNAL(clicked()), this, SLOT(reject()) );

  return this->QDialog::exec();
}


void V3DITKGenericDialog::AddDialogElement( const char * label, double  value, double  minimum, double  maximum )
{
  std::string labelIndex( label );
  this->elementContainer[labelIndex] = new V3DITKDialogElement( label, value, minimum, maximum );
}


void V3DITKGenericDialog::AddDialogElement( const char * label, int  value, int minimum, int maximum )
{
  // IMPLEMENT ME
}

void V3DITKGenericDialog::AddDialogElement( const char * label, const char * value )
{
  // IMPLEMENT ME
}

void V3DITKGenericDialog::AddDialogElement( const char * label, bool value )
{
  // IMPLEMENT ME
}

void V3DITKGenericDialog::AddDialogElement( const char * label, QString stringOption, QStringList & listOfOptions )
{
  // IMPLEMENT ME
}

double V3DITKGenericDialog::GetValue( const char * labelText )
{
  std::string labelIndex( labelText );
  const V3DITKDialogElement * dialogElement = this->elementContainer[labelIndex];
  const double value = dialogElement->valueBox->value();
  return value;
}

