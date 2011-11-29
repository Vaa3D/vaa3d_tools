#ifndef __V3DITKGenericDialog_H__
#define __V3DITKGenericDialog_H__

#include <QtGui>
#include <QtGui/QLabel>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDialog>


class V3DITKDialogElement
{
public:
  V3DITKDialogElement() {};

  V3DITKDialogElement( const char * label, double  value, double  minimum, double  maximum )
    {
    QString objectName( label );
    QString labelObjectName = objectName+"Label";

    this->nameLabel = new QLabel();
    this->nameLabel->setObjectName( labelObjectName );
    this->nameLabel->setText(QApplication::translate("MainWindow", label, 0, QApplication::UnicodeUTF8));

    this->valueBox = new QDoubleSpinBox();
    this->valueBox->setObjectName( objectName );
    this->valueBox->setMinimum( minimum );
    this->valueBox->setMaximum( maximum );
    this->valueBox->setValue( value );
    }

  ~V3DITKDialogElement()
    {
    delete this->nameLabel;
    delete this->valueBox;
    }

public:
  QLabel * nameLabel;
  QDoubleSpinBox * valueBox;
};



class V3DITKGenericDialog : public QDialog
{
  Q_OBJECT

public:
  V3DITKGenericDialog();
  ~V3DITKGenericDialog();

  explicit V3DITKGenericDialog( QWidget* iParent = 0 );

  V3DITKGenericDialog( const char * name );

  void AddDialogElement( const char * label, double  value, double  minimum, double  maximum );
  void AddDialogElement( const char * label, int  value, int minimum, int maximum );
  void AddDialogElement( const char * label, const char * value );
  void AddDialogElement( const char * label, bool value );
  void AddDialogElement( const char * label, QString stringOption, QStringList & listOfOptions );

  double GetValue( const char * labelText );

  int exec();

public slots:


public:
  std::string   pluginName;

  QGridLayout * gridLayout;
  QPushButton * ok;
  QPushButton * cancel;

  typedef std::map< std::string, V3DITKDialogElement * >  ElementContainerType;

  ElementContainerType elementContainer;

private:

  Q_DISABLE_COPY(V3DITKGenericDialog);

};

#endif
