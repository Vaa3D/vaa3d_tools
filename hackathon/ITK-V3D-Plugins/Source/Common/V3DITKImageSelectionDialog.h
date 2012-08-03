#ifndef __V3DITKImageSelectionDialog_H__
#define __V3DITKImageSelectionDialog_H__

#include <QtGui>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include "v3d_interface.h"
#include "basic_4dimage.h"
#include <vector>


class V3DITKFileDialogElement
{
public:
  V3DITKFileDialogElement() {};

  V3DITKFileDialogElement( const char * label, V3DPluginCallback * callback )
    {
    QString objectName( label );
    QString labelObjectName = objectName+"Label";

    this->nameLabel = new QLabel();
    this->nameLabel->setObjectName( labelObjectName );
    this->nameLabel->setText(QApplication::translate("MainWindow", label, 0, QApplication::UnicodeUTF8));

    this->comboBox = new QComboBox();
    this->comboBox->setObjectName( objectName );
    }

  ~V3DITKFileDialogElement()
    {
    delete this->nameLabel;
    }

public:
  QLabel * nameLabel;
  QComboBox * comboBox;
};



class V3DITKImageSelectionDialog : public QDialog
{
  Q_OBJECT

public:
  V3DITKImageSelectionDialog();
  ~V3DITKImageSelectionDialog();

  explicit V3DITKImageSelectionDialog( QWidget* iParent );

  void SetWindowTitle( const char * windowTitle );

  void SetCallback( V3DPluginCallback * pluginCallback );

  void AddImageSelectionLabel( const char * imageLabel );

  Image4DSimple * GetImageFromIndex( unsigned int imageIndex );

  int exec();

public slots:
void updateImageChanged();

public:

  std::string   dialogTitle;

  QGridLayout * gridLayout;
  QPushButton * ok;
  QPushButton * cancel;
  int Imageselected0;
  int Imageselected1;

  typedef std::vector< V3DITKFileDialogElement * >  ElementContainerType;

  ElementContainerType elementContainer;

private:

  void removeChildrenFromGridLayout();

  V3DPluginCallback * callback;

  Q_DISABLE_COPY(V3DITKImageSelectionDialog);

  typedef std::vector< std::string >  ImageLabeListType;

  ImageLabeListType  imageLabelList;

  typedef std::vector< Image4DSimple * > SelectedImagesListType;

  SelectedImagesListType  listOfSelectedImages;

};

#endif
