/* istitch_gui.h
 * 2010-10-15: create this program by Yang Yu
 */


#ifndef __ISTITCH_GUI_H__
#define __ISTITCH_GUI_H__


//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

//pairwise image stitching
class ImageStitchingDialog : public QDialog
{
	Q_OBJECT
	
public:
        ImageStitchingDialog(V3DPluginCallback &callback, QWidget *parentWidget)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		
		QStringList items;
		for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);
		
		// create a dialog
		combo_subject =  new QComboBox(); combo_subject->addItems(items);
		
		label_subject = new QLabel(QObject::tr("Image 1: ")); 
		
		i1 = combo_subject->currentIndex();
		
		combo_target =  new QComboBox(); combo_target->addItems(items);
		
		label_target = new QLabel(QObject::tr("Image 2: ")); 
		
		i2 = combo_target->currentIndex();
		
		//
		label_sub_channel = new QLabel(QObject::tr("Which channel of the image 1: ")); 
		label_tar_channel = new QLabel(QObject::tr("Which channel of the image 2: "));
		
		channel_sub = new QSpinBox(); 
		channel_tar = new QSpinBox();
		
		Image4DSimple* subject = callback.getImage(win_list[i1]);
		Image4DSimple* target = callback.getImage(win_list[i2]);
		
		channel_sub->setMaximum(subject->getCDim()); channel_sub->setMinimum(1); channel_sub->setValue(1);
		channel_tar->setMaximum(target->getCDim()); channel_tar->setMinimum(1); channel_tar->setValue(1);
		
		// scale 1
		v_scale_x = new QDoubleSpinBox(); 
		v_scale_y = new QDoubleSpinBox();
		v_scale_z = new QDoubleSpinBox();
		
		v_scale_x->setMaximum(1); v_scale_x->setMinimum(0); v_scale_x->setValue(0.2); v_scale_x->setSingleStep(0.01);
		v_scale_y->setMaximum(1); v_scale_y->setMinimum(0); v_scale_y->setValue(0.2); v_scale_y->setSingleStep(0.01);
		v_scale_z->setMaximum(1); v_scale_z->setMinimum(0); v_scale_z->setValue(0.2); v_scale_z->setSingleStep(0.01);
		
		label_scale_x = new QLabel(QObject::tr("Downsampling scale    x: "));
		label_scale_y = new QLabel(QObject::tr(" y: "));
		label_scale_z = new QLabel(QObject::tr(" z: "));
		
		//
		label_overlap = new QLabel(QObject::tr("Overlap at least: ")); 
		
		spinbox_overlap = new QDoubleSpinBox();
		
		spinbox_overlap->setMaximum(1); spinbox_overlap->setMinimum(0); spinbox_overlap->setValue(0.01); spinbox_overlap->setSingleStep(0.01); // overlap 1%
		
		overlap = spinbox_overlap->text().toDouble();	
		
		//display tile boundary
		check_axes_show = new QCheckBox();
		check_axes_show->setText(QObject::tr("Showing tile boundary"));
		check_axes_show->setChecked(false);
		
		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();
		
		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject, 0,1,1,5);
		gridLayout->addWidget(label_sub_channel, 1,0); gridLayout->addWidget(channel_sub, 1,1);
		
		gridLayout->addWidget(label_target, 2,0,1,1); gridLayout->addWidget(combo_target, 2,1,1,5);
		gridLayout->addWidget(label_tar_channel, 3,0); gridLayout->addWidget(channel_tar, 3,1);
		
		gridLayout->addWidget(label_scale_x, 7,0,1,1, Qt::AlignRight); gridLayout->addWidget(v_scale_x, 7,1,1,1);
		gridLayout->addWidget(label_scale_y, 7,2,1,1, Qt::AlignRight); gridLayout->addWidget(v_scale_y, 7,3,1,1);
		gridLayout->addWidget(label_scale_z, 7,4,1,1, Qt::AlignRight); gridLayout->addWidget(v_scale_z, 7,5,1,1);
		
		gridLayout->addWidget(label_overlap, 11,0,1,1); gridLayout->addWidget(spinbox_overlap, 11,1,1,1);
		
		gridLayout->addWidget(check_axes_show, 13,0,1,4);
		
		gridLayout->addWidget(cancel, 15,4,Qt::AlignRight); gridLayout->addWidget(ok, 15,5,Qt::AlignRight);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Image Stitching"));
		
		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));	
		connect(combo_target, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));	
		
		connect(channel_sub, SIGNAL(valueChanged(int)), this, SLOT(update()));	
		connect(channel_tar, SIGNAL(valueChanged(int)), this, SLOT(update()));
		
		connect(v_scale_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(v_scale_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(v_scale_z, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(spinbox_overlap, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(check_axes_show, SIGNAL(stateChanged(int)), this, SLOT(update()));
	}
	
	~ImageStitchingDialog(){}
	
public slots:
	void update()
	{
		i1 = combo_subject->currentIndex();
		i2 = combo_target->currentIndex();
		
		sub_c = channel_sub->text().toInt();
		tar_c = channel_tar->text().toInt();
		
		overlap = spinbox_overlap->text().toDouble();
		
		scale_x = v_scale_x->text().toDouble();
		scale_y = v_scale_y->text().toDouble();
		scale_z = v_scale_z->text().toDouble();
		
		if(check_axes_show->isChecked())
		{
			axes_show = true;
		}
		else
		{
			axes_show = false;
		}
	}
	
public:
	
	int i1,i2;	
	int sub_c, tar_c;
	
	double overlap;
	
	double scale_x, scale_y, scale_z;
	double scale_2_x, scale_2_y, scale_2_z;
	
	bool axes_show;
	
	QLabel* label_subject;	
	QComboBox* combo_subject;
	
	QLabel* label_target;	
	QComboBox* combo_target;
	
	QLabel *label_sub_channel;
	QLabel *label_tar_channel;
	
	QSpinBox *channel_sub;
	QSpinBox *channel_tar;
	
	QCheckBox* check_ms;
	QCheckBox* check_axes_show;
	
	QLabel *label_scale_x;
	QLabel *label_scale_y;
	QLabel *label_scale_z;
	
	QDoubleSpinBox* v_scale_x;
	QDoubleSpinBox* v_scale_y;
	QDoubleSpinBox* v_scale_z;
	
	QLabel *label_scale_2_x;
	QLabel *label_scale_2_y;
	QLabel *label_scale_2_z;
	
	QDoubleSpinBox* v_scale_2_x;
	QDoubleSpinBox* v_scale_2_y;
	QDoubleSpinBox* v_scale_2_z;
	
	QLabel* label_overlap;	
	QDoubleSpinBox* spinbox_overlap;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};

//group image stitching
class GroupImageStitchingDialog : public QDialog
{
	Q_OBJECT
	
public:
        GroupImageStitchingDialog(V3DPluginCallback &callback, QWidget *parentWidget, QString filename)
	{
		
		// create a dialog
		label_subject = new QLabel(QObject::tr("Choose the folder of images: ")); 
		
		edit_img = new QLineEdit(QDir::currentPath());
		pb_browse_img = new QPushButton("Browse...");
		
		if(!filename.isNull())
			edit_img->setText(filename);
		
		// choosing channel
		label_channel = new QLabel(QObject::tr("Reference color channel: ")); 
		
		spinbox_channel = new QSpinBox();
		
		spinbox_channel->setMaximum(3); spinbox_channel->setMinimum(1); spinbox_channel->setValue(1);
		
		m_similarity = 0;
		
		// scale
		v_scale_x = new QDoubleSpinBox(); 
		v_scale_y = new QDoubleSpinBox();
		v_scale_z = new QDoubleSpinBox();
		
		v_scale_x->setMaximum(1); v_scale_x->setMinimum(0); v_scale_x->setValue(0.2); v_scale_x->setSingleStep(0.01);
		v_scale_y->setMaximum(1); v_scale_y->setMinimum(0); v_scale_y->setValue(0.2); v_scale_y->setSingleStep(0.01);
		v_scale_z->setMaximum(1); v_scale_z->setMinimum(0); v_scale_z->setValue(0.2); v_scale_z->setSingleStep(0.01);
		
		label_scale = new  QLabel(QObject::tr("Downsampling factors: "));
		label_scale_x = new QLabel(QObject::tr(" x "));
		label_scale_y = new QLabel(QObject::tr(" y "));
		label_scale_z = new QLabel(QObject::tr(" z "));
		
		// overlap
		label_overlap = new QLabel(QObject::tr("Overlap at least: ")); 
		
		spinbox_overlap = new QDoubleSpinBox();
		
		spinbox_overlap->setMaximum(1); spinbox_overlap->setMinimum(0); spinbox_overlap->setValue(0.01); spinbox_overlap->setSingleStep(0.01); // oeverlap
		
		overlap = spinbox_overlap->text().toDouble();	
		
		//display tile boundary
		check_axes_show = new QCheckBox();
		check_axes_show->setText(QObject::tr("Save tiles' relative configutation to a file (.swc format)"));
		check_axes_show->setChecked(Qt::Unchecked);
		
		//display tile boundary
		check_img_show = new QCheckBox();
		check_img_show->setText(QObject::tr("Display the stitching result in V3D immediately"));
		check_img_show->setChecked(Qt::Checked);
		
		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();
		gridLayout->setSizeConstraint( QLayout::SetFixedSize );
		
		gridLayout->addWidget(label_subject, 0,0); gridLayout->addWidget(edit_img, 0,1,1,5); gridLayout->addWidget(pb_browse_img, 0,6);
		gridLayout->addWidget(label_channel, 1,0); gridLayout->addWidget(spinbox_channel, 1,1);
		
		gridLayout->addWidget(label_scale, 7,0,1,1);
		gridLayout->addWidget(label_scale_x, 7,2,1,1, Qt::AlignLeft); gridLayout->addWidget(v_scale_x, 7,1,1,1);
		gridLayout->addWidget(label_scale_y, 8,2,1,1, Qt::AlignLeft); gridLayout->addWidget(v_scale_y, 8,1,1,1);
		gridLayout->addWidget(label_scale_z, 9,2,1,1, Qt::AlignLeft); gridLayout->addWidget(v_scale_z, 9,1,1,1);
		
		gridLayout->addWidget(label_overlap, 10,0,1,1); gridLayout->addWidget(spinbox_overlap, 10,1,1,1);
		
		gridLayout->addWidget(check_axes_show, 13,0); gridLayout->addWidget(check_img_show, 12,0);
		
		gridLayout->addWidget(cancel, 15,5,Qt::AlignRight); gridLayout->addWidget(ok, 15,6,Qt::AlignRight);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Image Stitching"));
		
		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(spinbox_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));	
		
		connect(v_scale_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(v_scale_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(v_scale_z, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(pb_browse_img, SIGNAL(clicked()), this, SLOT(get_img_name()));
		
		connect(spinbox_overlap, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(check_axes_show, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(check_img_show, SIGNAL(stateChanged(int)), this, SLOT(update()));
	}
	
	~GroupImageStitchingDialog(){}
	
public slots:
	void update()
	{
		
		channel1 = spinbox_channel->text().toInt();
		
		overlap = spinbox_overlap->text().toDouble();
		
		scale_x = v_scale_x->text().toDouble();
		scale_y = v_scale_y->text().toDouble();
		scale_z = v_scale_z->text().toDouble();
		
		if(check_axes_show->isChecked())
		{
			axes_show = true;
		}
		else
		{
			axes_show = false;
		}
		
		if(check_img_show->isChecked())
		{
			img_show = true;
		}
		else
		{
			img_show = false;
		}
	}
	
	void get_img_name()
	{
		fn_img = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory including all tiled images "),
											  QDir::currentPath(),
											  QFileDialog::ShowDirsOnly);
		
		
		edit_img->setText(fn_img);
	}
	
public:
	QString fn_img;
	int channel1;
	
	double overlap;
	
	double scale_x, scale_y, scale_z;
	
	bool m_similarity, axes_show, img_show;
	
	QLabel* label_subject;
	QLineEdit *edit_img;
	QPushButton *pb_browse_img;
	
	QLabel *label_channel;
	
	QSpinBox *spinbox_channel;
	
	QLabel* label_overlap;	
	QDoubleSpinBox* spinbox_overlap;
	
	QCheckBox* check_img_show;
	QCheckBox* check_axes_show;
	
	QLabel *label_scale;
	QLabel *label_scale_x;
	QLabel *label_scale_y;
	QLabel *label_scale_z;
	
	QDoubleSpinBox* v_scale_x;
	QDoubleSpinBox* v_scale_y;
	QDoubleSpinBox* v_scale_z;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};

// point
class PointNavigateDialog : public QDialog
{
	Q_OBJECT
	
public:
        PointNavigateDialog(QWidget *parentWidget, long *sz)
	{		
		// create a dialog
		v_x = new QSpinBox(); 
		v_y = new QSpinBox();
		v_z = new QSpinBox();
		
		v_x->setMaximum(sz[0]); v_x->setMinimum(1); v_x->setValue(1); v_x->setSingleStep(1);
		v_y->setMaximum(sz[1]); v_y->setMinimum(1); v_y->setValue(1); v_y->setSingleStep(1);
		v_z->setMaximum(sz[2]); v_z->setMinimum(1); v_z->setValue(1); v_z->setSingleStep(1);
		
		label_x = new QLabel(QObject::tr("Enter the spatial coordinates  x: "));
		label_y = new QLabel(QObject::tr(" y: "));
		label_z = new QLabel(QObject::tr(" z: "));
		
		// button
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		
		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();
		
		gridLayout->addWidget(label_x, 0,0,1,1); gridLayout->addWidget(v_x, 0,1,1,1);
		gridLayout->addWidget(label_y, 0,2,1,1); gridLayout->addWidget(v_y, 0,3,1,1);
		gridLayout->addWidget(label_z, 0,4,1,1); gridLayout->addWidget(v_z, 0,5,1,1);
		
		gridLayout->addWidget(cancel, 3,4,1,1); gridLayout->addWidget(ok, 3,5,1,1);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Check voxel intensity"));
		
		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(v_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(v_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(v_z, SIGNAL(valueChanged(int)), this, SLOT(update()));
		
	}
	
	~PointNavigateDialog(){}
	
	public slots:
	void update()
	{		
		x = v_x->text().toLong() - 1;
		y = v_y->text().toLong() - 1;
		z = v_z->text().toLong() - 1;
	}
	
public:
	long x, y, z;
	
	QLabel *label_x;
	QLabel *label_y;
	QLabel *label_z;
	
	QSpinBox* v_x;
	QSpinBox* v_y;
	QSpinBox* v_z;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};

// region
class RegionNavigateDialog : public QDialog
{
	Q_OBJECT
	
public:
        RegionNavigateDialog(QWidget *parentWidget, long *sz)
	{		
		// create a dialog
		label_rgn = new QLabel(QObject::tr("Choose A Region "));
		
		// start
		xs=1; ys=1; zs=1;
		
		v_xs = new QSpinBox(); 
		v_ys = new QSpinBox();
		v_zs = new QSpinBox();
		
		v_xs->setMaximum(sz[0]); v_xs->setMinimum(1); v_xs->setValue(1); v_xs->setSingleStep(1);
		v_ys->setMaximum(sz[1]); v_ys->setMinimum(1); v_ys->setValue(1); v_ys->setSingleStep(1);
		v_zs->setMaximum(sz[2]); v_zs->setMinimum(1); v_zs->setValue(1); v_zs->setSingleStep(1);
		
		label_xs = new QLabel(QObject::tr(" Start position x: "));
		label_ys = new QLabel(QObject::tr(" y: "));
		label_zs = new QLabel(QObject::tr(" z: "));
		
		// end
		v_xe = new QSpinBox(); 
		v_ye = new QSpinBox();
		v_ze = new QSpinBox();
		
		v_xe->setMaximum(sz[0]); v_xe->setMinimum(xs); v_xe->setValue(sz[0]); v_xe->setSingleStep(1);
		v_ye->setMaximum(sz[1]); v_ye->setMinimum(ys); v_ye->setValue(sz[1]); v_ye->setSingleStep(1);
		v_ze->setMaximum(sz[2]); v_ze->setMinimum(zs); v_ze->setValue(sz[2]); v_ze->setSingleStep(1);
		
		label_xe = new QLabel(QObject::tr(" End position x: "));
		label_ye = new QLabel(QObject::tr(" y: "));
		label_ze = new QLabel(QObject::tr(" z: "));
		
		// button
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		
		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();
		
		gridLayout->addWidget(label_rgn, 0,0);
		
		gridLayout->addWidget(label_xs, 3,0,1,1); gridLayout->addWidget(v_xs, 3,1,1,1);
		gridLayout->addWidget(label_ys, 3,2,1,1); gridLayout->addWidget(v_ys, 3,3,1,1);
		gridLayout->addWidget(label_zs, 3,4,1,1); gridLayout->addWidget(v_zs, 3,5,1,1);
		
		gridLayout->addWidget(label_xe, 5,0,1,1); gridLayout->addWidget(v_xe, 5,1,1,1);
		gridLayout->addWidget(label_ye, 5,2,1,1); gridLayout->addWidget(v_ye, 5,3,1,1);
		gridLayout->addWidget(label_ze, 5,4,1,1); gridLayout->addWidget(v_ze, 5,5,1,1);
		
		gridLayout->addWidget(cancel, 7,4,1,1); gridLayout->addWidget(ok, 7,5,1,1);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Point Navigation"));
		
		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(v_xs, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(v_ys, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(v_zs, SIGNAL(valueChanged(int)), this, SLOT(update()));
		
		connect(v_xe, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(v_ye, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(v_ze, SIGNAL(valueChanged(int)), this, SLOT(update()));
	}
	
	~RegionNavigateDialog(){}
	
	public slots:
	void update()
	{		
		xs = v_xs->text().toLong() - 1;
		ys = v_ys->text().toLong() - 1;
		zs = v_zs->text().toLong() - 1;
		
		v_xe->setMinimum(xs+1); 
		v_ye->setMinimum(ys+1); 
		v_ze->setMinimum(zs+1);
		
		xe = v_xe->text().toLong() - 1;
		ye = v_ye->text().toLong() - 1;
		ze = v_ze->text().toLong() - 1;
	}
	
public:
	long xs, ys, zs, xe, ye, ze;
	
	QLabel *label_rgn;
	
	QLabel *label_xs;
	QLabel *label_ys;
	QLabel *label_zs;
	
	QSpinBox* v_xs;
	QSpinBox* v_ys;
	QSpinBox* v_zs;
	
	QLabel *label_xe;
	QLabel *label_ye;
	QLabel *label_ze;
	
	QSpinBox* v_xe;
	QSpinBox* v_ye;
	QSpinBox* v_ze;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};

#endif



