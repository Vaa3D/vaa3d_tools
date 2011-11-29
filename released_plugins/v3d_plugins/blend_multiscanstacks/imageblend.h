/* imageblend.h
 * 2011-07-30: create this program by Yang Yu
 */


#ifndef __IMAGEBLEND_H__
#define __IMAGEBLEND_H__

//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ImageBlendPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
    float getPluginVersion() const {return 1.0f;} // version info 
    
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const;
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent);
	
};

class ImageBlendingDialog : public QDialog
{
    Q_OBJECT
    
public:
    ImageBlendingDialog(V3DPluginCallback &callback, QWidget *parentWidget, QString filename)
	{
		// create a dialog here
        
        //image1
		label_image1 = new QLabel(QObject::tr("Choose the image 1: ")); 
		
		edit_img1 = new QLineEdit(QDir::currentPath());
		pb_browse_img1 = new QPushButton("Browse...");
		
		if(!filename.isNull())
			edit_img1->setText(filename);
        
        // image 2
        label_image2 = new QLabel(QObject::tr("Choose the image 2: ")); 
		
		edit_img2 = new QLineEdit(QDir::currentPath());
		pb_browse_img2 = new QPushButton("Browse...");
		
		if(!filename.isNull())
			edit_img2->setText(filename);
        
		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();
		gridLayout->setSizeConstraint( QLayout::SetFixedSize );
		
		gridLayout->addWidget(label_image1, 0,0); gridLayout->addWidget(edit_img1, 0,1,1,5); gridLayout->addWidget(pb_browse_img1, 0,6);
		gridLayout->addWidget(label_image2, 2,0); gridLayout->addWidget(edit_img2, 2,1,1,5); gridLayout->addWidget(pb_browse_img2, 2,6);
		
		gridLayout->addWidget(cancel, 5,5,Qt::AlignRight); gridLayout->addWidget(ok, 5,6,Qt::AlignRight);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Image Blending"));
		
		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		
		connect(pb_browse_img1, SIGNAL(clicked()), this, SLOT(get_img_name1()));
        connect(pb_browse_img2, SIGNAL(clicked()), this, SLOT(get_img_name2()));
		
	}
	
	~ImageBlendingDialog(){}
	
public slots:
	
	void get_img_name1()
	{
		fn_img1 = QFileDialog::getOpenFileName(0, QObject::tr("Choose the image 1 "),
                                                   QDir::currentPath(),
                                                   QObject::tr("Images (*.raw *.tif *.lsm);;All(*)"));
		
		
		edit_img1->setText(fn_img1);
	}
    
    void get_img_name2()
	{
		fn_img2 = QFileDialog::getOpenFileName(0, QObject::tr("Choose the image 2 "),
                                                   QDir::currentPath(),
                                                   QObject::tr("Images (*.raw *.tif *.lsm);;All(*)"));
		
		
		edit_img2->setText(fn_img2);
	}
	
public:
	QString fn_img1, fn_img2;
	
	QLabel *label_image1;
	QLineEdit *edit_img1;
	QPushButton *pb_browse_img1;
	
	QLabel *label_image2;
	QLineEdit *edit_img2;
	QPushButton *pb_browse_img2;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};

#endif



