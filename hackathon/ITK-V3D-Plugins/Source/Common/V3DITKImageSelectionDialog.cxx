#include "V3DITKImageSelectionDialog.h"

V3DITKImageSelectionDialog::V3DITKImageSelectionDialog()
{
  this->ok     = new QPushButton("OK");
  this->cancel = new QPushButton("Cancel");

  this->gridLayout = new QGridLayout();
}

V3DITKImageSelectionDialog::V3DITKImageSelectionDialog( QWidget * )
{
}


V3DITKImageSelectionDialog::~V3DITKImageSelectionDialog()
{
  delete this->ok;
  delete this->cancel;
  delete this->gridLayout;
}

void V3DITKImageSelectionDialog::SetWindowTitle( const char * name )
{
  this->dialogTitle = name;
}

void V3DITKImageSelectionDialog::SetCallback( V3DPluginCallback * pluginCallback )
{
  this->callback = pluginCallback;
}

void V3DITKImageSelectionDialog::AddImageSelectionLabel( const char * imageLabel )
{
  this->imageLabelList.push_back( imageLabel );
}

Image4DSimple * V3DITKImageSelectionDialog::GetImageFromIndex( unsigned int imageIndex )
{
  return this->listOfSelectedImages[imageIndex];
}

void V3DITKImageSelectionDialog::removeChildrenFromGridLayout()
{
  QLayoutItem *child;
  while ( (child = this->gridLayout->takeAt(0)) != 0)
   {
   this->gridLayout->removeItem(child);
   delete child->widget();
   delete child;
   }
}
void V3DITKImageSelectionDialog::updateImageChanged()
{
	Imageselected0=this->elementContainer.at(0)->comboBox->currentIndex();
	Imageselected1=this->elementContainer.at(1)->comboBox->currentIndex();
}

int V3DITKImageSelectionDialog::exec()
{
  this->removeChildrenFromGridLayout();

  this->setLayout(gridLayout);

  this->setWindowTitle( QObject::tr( this->dialogTitle.c_str() ) );

  unsigned int numberOfExpectedImages = this->imageLabelList.size();

  v3dhandleList wndlist = this->callback->getImageWindowList();

   unsigned int imageEntry = 0;

  for(imageEntry = 0; imageEntry < numberOfExpectedImages; imageEntry++ )
    {
    V3DITKFileDialogElement * element =
      new V3DITKFileDialogElement( this->imageLabelList[imageEntry].c_str(), this->callback );
    

    this->elementContainer.push_back( element );
    
    //element->comboBox->addItem(callback->getImageName(wndlist.at(imageEntry)));
    this->gridLayout->addWidget( element->nameLabel, imageEntry, 0 );
    this->gridLayout->addWidget( element->comboBox, imageEntry, 1 );

    // Here populate the entries of the comboBox, with the list of window names
    //
    for(int i=0;i<wndlist.size();i++)
	{
	element->comboBox->addItem(callback->getImageName(wndlist.at(i)));
    	this->listOfSelectedImages.push_back(callback->getImage( wndlist.at(i) ));
	}
	}

    this->gridLayout->addWidget( this->cancel, imageEntry, 0 );
    this->gridLayout->addWidget( this->ok, imageEntry, 1 );
  this->updateImageChanged();
  this->connect( this->ok,     SIGNAL(clicked()), this, SLOT(accept()) );
  this->connect( this->cancel, SIGNAL(clicked()), this, SLOT(reject()) );
  this->connect(this->elementContainer.at(0)->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateImageChanged()));
  this->connect(this->elementContainer.at(1)->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateImageChanged()));

  return this->QDialog::exec();
}

