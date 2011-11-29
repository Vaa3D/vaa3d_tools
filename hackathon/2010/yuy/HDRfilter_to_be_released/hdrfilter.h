/* hdrfilter.h
 * 2011-03-21: create this program by Yang Yu
 */


#ifndef __HDRFILTER_H__
#define __HDRFILTER_H__

//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

#include "../../../v3d_main/neuron_annotator/gui/NaZStackWidget.h"

class HDRFilterPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				V3DPluginCallback2 & v3d, QWidget * parent) {return true;}
	float getPluginVersion() const {return 1.01f;} // version info 
};

// dialog to choose the source image
class HDRFilerDialog : public QDialog 
{
	Q_OBJECT
	
public:
	QString fn_image;
	
	QGridLayout *gridLayout;
	
	QLabel* label_image;
	QLineEdit *le_image;
	QPushButton *pb_browse_image;
	
	QPushButton* ok;
	QPushButton* cancel;
	
public:
	HDRFilerDialog(V3DPluginCallback2 &cb, QWidget *parent)
	{
		//create a dialog
		gridLayout = new QGridLayout();
		
		//
		label_image = new QLabel(QObject::tr(" Input Image: "));
		le_image = new QLineEdit(QDir::currentPath());
		pb_browse_image = new QPushButton("Browse...");
		
		gridLayout->addWidget(label_image, 1,0); 
		gridLayout->addWidget(le_image,     1,1);
		gridLayout->addWidget(pb_browse_image, 1,2); 
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout->addWidget(cancel, 6,1,1,1); 
		gridLayout->addWidget(ok,     6,2,1,1);
		
		setLayout(gridLayout);
		setWindowTitle(QString("HDR Filter"));
		
		//slot interface
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(pb_browse_image, SIGNAL(clicked()), this, SLOT(get_image()));
	}
	
	~HDRFilerDialog(){}
	
	public slots:
	void get_image()
	{
		fn_image = QFileDialog::getOpenFileName(0, QObject::tr("Open an image "),
												QDir::currentPath(),
												QObject::tr("Images (*.raw *.tif *.lsm);;All(*)"));
		
		
		le_image->setText(fn_image);
	}
};

// draw a 2D viewer showing the control window
class HDRwidget : public QWidget 
{
	Q_OBJECT
	
public:
	HDRwidget(QWidget *parent, Image4DSimple &p4DImage)
	{		
		// 
		sx = p4DImage.getXDim();
		sy = p4DImage.getYDim();
		sz = p4DImage.getZDim();
		sc = p4DImage.getCDim();
		
		cur_z = sz/2;
		
		ratio_x2y = float(sx)/float(sy);
		
		cr = 25;
		
		// color
		QGroupBox *colorGroup = new QGroupBox;
		colorGroup->setTitle(tr("Color Channel and ROI Size"));
		
		rb_r = new QRadioButton(tr("Red"), colorGroup);
		connect(rb_r, SIGNAL(clicked()), this, SLOT(setRedChannel()));
		rb_r->setChecked(true);
		
		rb_g = new QRadioButton(tr("Green"), colorGroup);
		connect(rb_g, SIGNAL(clicked()), this, SLOT(setGreenChannel()));
		
		rb_b = new QRadioButton(tr("Blue"), colorGroup);
		connect(rb_b, SIGNAL(clicked()), this, SLOT(setBlueChannel()));
		
		// view
		viewGroup = new QGroupBox;
		
		pNaZW = new NaZStackWidget(parent);
		//pNaZW->loadMy4DImage(const_cast<My4DImage*>(p4DImage));
		V3DLONG imageSize[4] = {sx, sy, sz, sc};
		pNaZW->initHDRViewer(imageSize, p4DImage.getRawData(), p4DImage.getDatatype());
		
		pNaZW->setFocusPolicy(Qt::ClickFocus);
		
		cur_c = 1;
		pNaZW->setRedChannel(); 
		
		QPalette palette_r = rb_r->palette();
		palette_r.setColor(rb_r->backgroundRole(), Qt::red);
		palette_r.setColor(rb_r->foregroundRole(), Qt::red);
		rb_r->setPalette(palette_r);
		
		if(sc<=1)
		{
			qDebug()<<"# color channel is ..."<<sc;
			
			rb_g->setCheckable(false);
			rb_b->setCheckable(false);
		}
		else if(sc<=2)
		{
			qDebug()<<"# color channel is ..."<<sc;
			
			rb_b->setCheckable(false);
		}
		
		viewGroup->setTitle(QString("z %1/%2    x %3 y %4").arg(cur_z+1).arg(sz).arg(sx).arg(sy));
		
		// z slice
		sb_z = new QScrollBar(Qt::Horizontal);
		sb_z->setMaximum(sz); sb_z->setMinimum(1); sb_z->setValue(cur_z); sb_z->setSingleStep(1);
		
		QGroupBox *sliceGroup = new QGroupBox;
		sliceGroup->setTitle(tr("Z Slice"));
		
		// search box size
		l_sb = new QLabel("Search Box Size ");
		sb_sb = new QSpinBox;
		sb_sb->setMaximum(sx>sy?sy:sx); sb_sb->setMinimum(1); sb_sb->setValue(25); sb_sb->setSingleStep(1);
		
		// layout		
		QGridLayout *viewGroupLayout = new QGridLayout(viewGroup);
		viewGroupLayout->addWidget(pNaZW, 0, 0);
		//viewGroupLayout->setVerticalSpacing(4);
		
		QGridLayout *sliceGroupLayout = new QGridLayout(sliceGroup);
		sliceGroupLayout->addWidget(sb_z, 0, 0);
		
		QGridLayout *colorLayout = new QGridLayout(colorGroup); 
		colorLayout->addWidget(rb_r, 0, 0, Qt::AlignLeft);
		colorLayout->addWidget(rb_g, 0, 1, Qt::AlignLeft);
		colorLayout->addWidget(rb_b, 0, 2, Qt::AlignLeft);
		
		colorLayout->addWidget(l_sb, 0, 3, Qt::AlignRight);
		colorLayout->addWidget(sb_sb, 0, 4, Qt::AlignLeft);
		
		QGridLayout *gridLayout = new QGridLayout(this);
		gridLayout->addWidget(viewGroup, 0,0, 10, 1);
		gridLayout->addWidget(sliceGroup, 11,0, 1, 1);
		gridLayout->addWidget(colorGroup, 12,0, 1, 1);
		setLayout(gridLayout);
		setWindowTitle(QString("HDR Filtering"));
		
		//
		connect(sb_z, SIGNAL(valueChanged(int)), this, SLOT(updateZslice()));
		connect(this, SIGNAL(sizeChanged(QSize)), this, SLOT(sizeHandler()));
		
		//
		connect(pNaZW, SIGNAL(curZsliceChanged(int)), this, SLOT(update()));
		connect(pNaZW, SIGNAL(boxSizeChanged(int)), this, SLOT(updateCurWin()));
		connect(pNaZW, SIGNAL(curColorChannelChanged()), this, SLOT(update()));
		
		//
		connect(rb_r, SIGNAL(clicked()), this, SLOT(setRedChannel()));
		connect(rb_g, SIGNAL(clicked()), this, SLOT(setGreenChannel()));
		connect(rb_b, SIGNAL(clicked()), this, SLOT(setBlueChannel()));
		
		//
		connect(this, SIGNAL(colorChanged()), pNaZW, SLOT(colorChannelChanged()));
		
		//
		connect(sb_sb, SIGNAL(valueChanged(int)), this, SLOT(updateROIsize()));
	}
	
	~HDRwidget(){}
	
signals:
	void sizeChanged(QSize size);
	void colorChanged();
	
public slots:
	void update()
        {
		cur_z = pNaZW->getCurrentZSlice();	
		
		viewGroup->setTitle(QString("z %1/%2 x %3 y %4").arg(cur_z+1).arg(sz).arg(sx).arg(sy));
		
		sb_z->setValue(cur_z+1); // keep synchronized
		
		pNaZW->updatePixmap();
		
	}
	
	void updateZslice()
	{
		cur_z = sb_z->value()-1;
		pNaZW->setCurrentZSlice(cur_z);

                pNaZW->do_HDRfilter_zslice();
		
		update();
	}
	
	void updateROIsize()
	{
		cr = sb_sb->value();
		
		pNaZW->updateROIsize(cr);
		
		pNaZW->update();
		pNaZW->repaint();
	}
	
	void sizeHandler()
	{
		if( (this->size().height() != oriSize.height()) || (this->size().width() != oriSize.width()) )
		{
			oriSize.setHeight(this->size().height());
			oriSize.setWidth(int(float(this->size().height())*ratio_x2y));
			//emit sizeChanged(this->size());
			
			resize(oriSize.width(), oriSize.height()); // keep ratio of x to y
			
		}
	}
	
	void resizeEvent(QResizeEvent *re )
	{
		emit sizeChanged(re->size());
	}
	
	void updateCurWin()
	{	
		cr = pNaZW->getCurrentBoxSize();
		sb_sb->setValue(cr);
	}
	
	//
	void setRedChannel()
	{
		cur_c = 1; // red
		pNaZW->setRedChannel();
		
		QPalette palette_r = rb_r->palette();
		palette_r.setColor(rb_r->backgroundRole(), Qt::red);
		palette_r.setColor(rb_r->foregroundRole(), Qt::red);
		rb_r->setPalette(palette_r);
		
		QPalette palette_g = rb_g->palette();
		palette_g.setColor(rb_g->backgroundRole(), Qt::black);
		palette_g.setColor(rb_g->foregroundRole(), Qt::black);
		rb_g->setPalette(palette_g);
		
		QPalette palette_b = rb_b->palette();
		palette_b.setColor(rb_b->backgroundRole(), Qt::black);
		palette_b.setColor(rb_b->foregroundRole(), Qt::black);
		rb_b->setPalette(palette_b);
		
		pNaZW->copydata2disp();
		
		emit colorChanged();
		update();
	}
	
	void setGreenChannel()
	{
		if(rb_g->isCheckable())
		{
			cur_c = 2; // green
			pNaZW->setGreenChannel();
			
			QPalette palette_r = rb_r->palette();
			palette_r.setColor(rb_r->backgroundRole(), Qt::black);
			palette_r.setColor(rb_r->foregroundRole(), Qt::black);
			rb_r->setPalette(palette_r);
			
			QPalette palette_g = rb_g->palette();
			palette_g.setColor(rb_g->backgroundRole(), Qt::green);
			palette_g.setColor(rb_g->foregroundRole(), Qt::green);
			rb_g->setPalette(palette_g);
			
			QPalette palette_b = rb_b->palette();
			palette_b.setColor(rb_b->backgroundRole(), Qt::black);
			palette_b.setColor(rb_b->foregroundRole(), Qt::black);
			rb_b->setPalette(palette_b);
			
			pNaZW->copydata2disp();
			
			emit colorChanged();
			update();
		}
	}
	
	void setBlueChannel()
	{
		if(rb_b->isCheckable())
		{
			cur_c = 3; // blue
			pNaZW->setBlueChannel();
			
			QPalette palette_r = rb_r->palette();
			palette_r.setColor(rb_r->backgroundRole(), Qt::black);
			palette_r.setColor(rb_r->foregroundRole(), Qt::black);
			rb_r->setPalette(palette_r);
			
			QPalette palette_g = rb_g->palette();
			palette_g.setColor(rb_g->backgroundRole(), Qt::black);
			palette_g.setColor(rb_g->foregroundRole(), Qt::black);
			rb_g->setPalette(palette_g);
			
			QPalette palette_b = rb_b->palette();
			palette_b.setColor(rb_b->backgroundRole(), Qt::blue);
			palette_b.setColor(rb_b->foregroundRole(), Qt::blue);
			rb_b->setPalette(palette_b);
			
			pNaZW->copydata2disp();
			
			emit colorChanged();
			update();
		}
		
	}
	
public:
	QGroupBox *viewGroup;
	
	QScrollBar *sb_z;
	
	NaZStackWidget *pNaZW;
	
	QRadioButton *rb_r;
	QRadioButton *rb_g;
	QRadioButton *rb_b;
	
	QLabel *l_sb;
	QSpinBox *sb_sb;
	
	V3DLONG sx, sy, sz, sc;
	V3DLONG cur_x, cur_y, cur_z, cur_c;
	
	QSize oriSize;
	
	int cr;
	float ratio_x2y;
	
	float scale; 
	
};


#endif



