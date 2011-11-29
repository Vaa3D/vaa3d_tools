/* ifusion.h
 * 2011-08-31: create this program by Yang Yu
 */


#ifndef __IFUSION_H__
#define __IFUSION_H__

//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ImageFusionPlugin : public QObject, public V3DPluginInterface2_1
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

class ImageFusionDialog : public QDialog
{
    Q_OBJECT
    
public:
    ImageFusionDialog(V3DPluginCallback &callback, QWidget *parentWidget, QString folder)
	{
		// create a dialog here
        
        //folder with tiled images and stitching configurations
		label_imagefusionfolder = new QLabel(QObject::tr("Open Directory: ")); 
        QLabel *label_tip = new QLabel(QObject::tr("Note: \nChoose a folder \nwith all tiles and \ntheir stitching \nconfigurations. "));
		
		edit_folder = new QLineEdit(QDir::currentPath());
		pb_browse_folder = new QPushButton("Browse...");
		
		if(!folder.isNull())
			edit_folder->setText(folder);
        
		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();
		gridLayout->setSizeConstraint( QLayout::SetFixedSize );
		
		gridLayout->addWidget(label_imagefusionfolder, 0,0); gridLayout->addWidget(edit_folder, 0,1,1,5); gridLayout->addWidget(pb_browse_folder, 0,6);
        gridLayout->addWidget(label_tip, 1,0);
		
		gridLayout->addWidget(cancel, 3,5,Qt::AlignRight); gridLayout->addWidget(ok, 3,6,Qt::AlignRight);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Image Fusion"));
		
		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		connect(pb_browse_folder, SIGNAL(clicked()), this, SLOT(getExitingfolder()));
		
	}
	
	~ImageFusionDialog(){}
	
public slots:
	
	void getExitingfolder()
	{
		foldername = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                          QDir::currentPath(),
                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		
		
		edit_folder->setText(foldername);
	}
	
public:
	QString foldername;
	
	QLabel *label_imagefusionfolder;
	QLineEdit *edit_folder;
	QPushButton *pb_browse_folder;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};

#endif // __IFUSION_H__



