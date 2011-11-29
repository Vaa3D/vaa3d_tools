/* hdrfilter_gui.h
 * 2011-03-21: create this program by Yang Yu
 */


#ifndef __HDRFILTER_GUI_H__
#define __HDRFILTER_GUI_H__

//
#include "hdrfilter.h"

//
#define INF 1e9

// funcs for converting data and create pixmap
// func convering kernel
template <class Tpre, class Tpost>
void convering(Tpre *pre1d, Tpost *pPost, V3DLONG imsz)
{
	
	for(V3DLONG i=0; i<imsz; i++)
	{
		pPost[i] = (Tpost) pre1d[i];
	}
	
}

template <class T> QPixmap getXYPlane(const T * pdata, V3DLONG sx, V3DLONG sy, V3DLONG sz, V3DLONG sc, V3DLONG curz, float *p_vmax, float *p_vmin)
{
	QImage tmpimg = QImage(sx, sy, QImage::Format_RGB32);
	
	qDebug()<<curz;
	
	int tr,tg,tb;
	
	V3DLONG i,j;
	float tmpr,tmpg,tmpb;
	float tmpr_min, tmpg_min, tmpb_min;
	
	if (sc>=3)
	{
		tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
		tmpb_min = p_vmin[2];
	}
	
	if (sc>=2)
	{
		tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
		tmpg_min = p_vmin[1];
	}
	
	if (sc>=1)
	{
		tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
		tmpr_min = p_vmin[0];
	}
	int pagesz = sx*sy*sz;
	long offset_k = sx*sy;
	
	switch (sc) 
	{
		case 1:
			
			qDebug()<<"r ...";
			
			for (long j = 0; j < sy; j ++) 
			{
				long offset = curz*offset_k + j*sx;
				for (long i=0; i<sx; i++) 
				{
					long idx = offset + i;
					
					tb = tg = tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;
			
		case 2:
			
			qDebug()<<"rg ...";
			
			tb = 0;
			for (long j = 0; j < sy; j ++) 
			{
				long offset = curz*offset_k + j*sx;
				for (long i=0; i<sx; i++) 
				{
					long idx = offset + i;
					tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tg = floor((pdata[idx+pagesz]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;
			
		case 3:
		case 4:
			
			qDebug()<<"rgb ..."<<tmpr<<tmpg<<tmpb;
			
			for (long j = 0; j < sy; j ++) 
			{
				long offset = curz*offset_k + j*sx;
				for (long i=0; i<sx; i++) 
				{
					long idx = offset + i;
					
					tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tg = floor((pdata[idx+pagesz]-tmpg_min)/tmpg*255.0);
					tb = floor((pdata[idx+2*pagesz]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;
			
		default:
			break;
	}
	return QPixmap::fromImage(tmpimg);
	
}

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


// xy-plane view
class HDRViewer : public QWidget 
{
	Q_OBJECT
	
public:
	HDRViewer(QWidget *parent):QWidget(parent)
	{
		pData1d = NULL; 
		pDispData1d = NULL;
		
		pixmap = QPixmap(256, 256);
		pixmap.fill(Qt::red);
		
		scale_x = 1.0; scale_y = 1.0;
		dispscale = 0.9;
		
		translateMouse_scale = 1;
		
		b_mouseleft = false;
		b_mouseright = false;
		b_mousemove = false;
		
		bMouseCurorIn = false;
		bMouseDone = false;
		
		m_square_pos.setX(sx/2);
		m_square_pos.setY(sy/2);
		cr = 25;
		
		connect(this, SIGNAL(curColorChannelChanged()), this, SLOT(do_HDRfilter));
		connect(this, SIGNAL(roiChanged()), this, SLOT(updatePixmap()));
	}
	
	~HDRViewer(){}
	
public:	
	void paintEvent(QPaintEvent *event)
	{
		qDebug()<<"painting ...";
		
		QPainter painter(this);
		//painter.save();
		
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		
		// image
		scale_x = (float)dispwidth/(float)sx;
		scale_y = (float)dispheight/(float)sy;
		
//		float cx = 0.45*(float)dispwidth/dispscale;
//		float cy = 0.45*(float)dispheight/dispscale;
//		
//		QPointF center(cx, cy);
		
		//painter.translate(center);
		painter.scale(scale_x, scale_y);
		//painter.translate(-center);
		
//		float translate_px = cx-(float)dispwidth/2.0;
//		float translate_py = cy-(float)dispheight/2.0;
		
		painter.drawPixmap(QPointF(1, 1), pixmap); // 1-based
		
		// ROI
		drawROI(&painter);
		
	}
	
public:
	// ROI controller functions
	void drawROI(QPainter *painter)
	{
		if (bMouseCurorIn)
		{
			painter->setPen(Qt::yellow );
			painter->setBrush(Qt::NoBrush);
			
			if(b_mousemove)
			{			
				if(b_mouseright)
				{
					QRect r( startMousePosR, curMousePosR );
					painter->drawRect( r );
				}
				else if(b_mouseleft)
				{
					if(bMouseDone)
					{	
						QSizeF sz(2*cr, 2*cr);
						QRectF square = rectangle_around(m_square_pos, sz);
						painter->drawRect(square);
					}					
				}
			}
			else if(bMouseDone)
			{		
				QSizeF sz(2*cr, 2*cr);
				QRectF square = rectangle_around(m_square_pos,sz);
				painter->drawRect(square);
			}
		
			if( checkROIchanged() )
			{
				do_HDRfilter();
			}
		}
		else
		{
			qDebug()<<"initial a square roi ...";
			
			// init a square
			cx = sx/2;
			cy = sy/2;
			cr = 25;
			
			start_x = cx - cr;
			end_x = cx + cr;
			
			if(start_x<0) start_x = 0;
			if(end_x>=sx) end_x = sx-1;
			
			start_y = cy -cr;
			end_y = cy + cr;
			
			if(start_y<0) start_y = 0;
			if(end_y>=sy) end_y = sy-1;
			
			startMousePos.setX(start_x);
			startMousePos.setY(start_y);
			
			endMousePos.setX(end_x);
			endMousePos.setY(end_y);
			
			QSizeF sz(cr, cr);
			QRectF square = rectangle_around(m_square_pos,sz);
			painter->drawRect(square);
		}
		
	}
	
	void enterEvent (QEvent * e) // mouse found
	{
		bMouseCurorIn = true;
		update();
	}
	
	void mouseLeftButtonPressEvent(QMouseEvent *e) // mouse left button
	{
		if (bMouseCurorIn)
		{
			//if(cursorInRoi(e))
			{
				b_mouseleft = true;
				b_mouseright = false;
				b_mousemove = false;
				bMouseDone = true;
				
				startMousePosL = e->pos();
				
				startMousePosL.setX( startMousePosL.x()/scale_x );
				startMousePosL.setY( startMousePosL.y()/scale_y );

				QRectF square = rectangle_around(m_square_pos);
				
				if (square.contains(startMousePosL)) 
				{
					m_offset = square.center() - startMousePosL;
				}
				
				qDebug()<<"left press ..."<< startMousePosL.x() << startMousePosL.y();
				
				setCursor(Qt::CrossCursor);
				update();
			}
		}
	}
	
	void mouseRightButtonPressEvent(QMouseEvent *e) // mouse right button
	{
		if (bMouseCurorIn)
		{
			b_mouseright = true;
			b_mouseleft = false;
			b_mousemove = false;
			bMouseDone = false;
			
			startMousePosR = e->pos(); //
			
			startMousePosR.setX( startMousePosR.x()/scale_x );
			startMousePosR.setY( startMousePosR.y()/scale_y );
			
			qDebug()<<"right press ..."<< startMousePosR.x() << startMousePosR.y();
			
			setCursor(Qt::CrossCursor);
			update();
		}
	}
	
	void wheelEvent(QWheelEvent * e) // mouse wheel
	{
		b_mousemove = false;
		
		int numDegrees = e->delta()/8;
		int numTicks = numDegrees/15;
		
		cur_z += numTicks;
		
		if(cur_z<0) cur_z = 0;
		if(cur_z>sz-1) cur_z = sz-1;
		
		emit curZsliceChanged();
		
		return;
	}
	
	void mouseMoveEvent (QMouseEvent * e) // mouse move
	{
		if (bMouseCurorIn)
		{
			b_mousemove = true;
			
			if(b_mouseright)
			{
				bMouseDone = false;
				
				curMousePosR = e->pos();
				
				if (curMousePosR.x() < 0)
				{
					curMousePosR.setX(0);
				}
				else if (curMousePosR.x() > pixmap.width()*scale_x)
				{
					curMousePosR.setX(pixmap.width()*scale_x);
				}
				
				if (curMousePosR.y()<0)
				{
					curMousePosR.setY(0);
				}
				else if (curMousePosR.y() > pixmap.height()*scale_y)
				{
					curMousePosR.setY(pixmap.height()*scale_y);
				}
				
				curMousePosR.setX( curMousePosR.x()/scale_x );
				curMousePosR.setY( curMousePosR.y()/scale_y );
				
				qDebug()<<"right press move ..."<< curMousePosR.x() << curMousePosR.y();
			}
			
			if(b_mouseleft)
			{
				bMouseDone = true;
				
				curMousePosL = e->pos();
				
				if (curMousePosL.x() < 0)
				{
					curMousePosL.setX(0);
				}
				else if (curMousePosL.x() > pixmap.width()*scale_x)
				{
					curMousePosL.setX(pixmap.width()*scale_x);
				}
				
				if (curMousePosL.y()<0)
				{
					curMousePosL.setY(0);
				}
				else if (curMousePosL.y() > pixmap.height()*scale_y)
				{
					curMousePosL.setY(pixmap.height()*scale_y);
				}
				
				curMousePosL.setX( curMousePosL.x()/scale_x );
				curMousePosL.setY( curMousePosL.y()/scale_y );
				
				setSquarePos(curMousePosL + m_offset);
			}
			
		}
		update();
	}
	
	void mousePressEvent(QMouseEvent *e) // mouse button press
	{
		switch (e->button())
		{
			case Qt::LeftButton:
				mouseLeftButtonPressEvent(e);
				break;
			case Qt::RightButton:
				mouseRightButtonPressEvent(e);
			default:
				break;
		}
	}
	
	void mouseReleaseEvent(QMouseEvent * e) // mouse button release
	{
		b_mousemove = false;
		
		if (bMouseCurorIn)
		{
			if(b_mouseright)
			{
				endMousePosR = e->pos();
				setCursor(Qt::ArrowCursor);
				
				if (endMousePosR.x() < 0)
				{
					endMousePosR.setX(0);
				}
				else if (endMousePosR.x() > pixmap.width()*scale_x)
				{
					endMousePosR.setX(pixmap.width()*scale_x);
				}
				
				if (endMousePosR.y()<0)
				{
					endMousePosR.setY(0);
				}
				else if (endMousePosR.y() > pixmap.height()*scale_y)
				{
					endMousePosR.setY(pixmap.height()*scale_y);
				}
				
				endMousePosR.setX( endMousePosR.x()/scale_x );
				endMousePosR.setY( endMousePosR.y()/scale_y );
				
				qDebug()<<"right release ..."<< endMousePosR.x() << endMousePosR.y();
				
				// adjust pos
				float top_x, top_y, bottom_x, bottom_y;
				
				top_x = qMin( startMousePosR.x(), endMousePosR.x() );
				top_y = qMin( startMousePosR.y(), endMousePosR.y() );
				
				bottom_x = qMax( startMousePosR.x(), endMousePosR.x() );
				bottom_y = qMax( startMousePosR.y(), endMousePosR.y() );
				
				startMousePos.setX( top_x );
				startMousePos.setY( top_y );
				
				endMousePos.setX( bottom_x );
				endMousePos.setY( bottom_y );
				
				m_square_pos.setX( startMousePos.x() + (endMousePos.x() - startMousePos.x())/2 );
				m_square_pos.setY( startMousePos.y() + (endMousePos.y() - startMousePos.y())/2 );
				
				cr = qMax((endMousePos.x() - startMousePos.x())/2, (endMousePos.y() - startMousePos.y())/2); 
				
				bMouseDone = true;

				qDebug()<<"x, y ..."<<startMousePos.x()<<startMousePos.y()<<endMousePos.x()<<endMousePos.y();	
				
				//
				b_mouseright = false;
			}
			
			
			if(b_mouseleft)
			{
				//
				b_mouseleft = false;
				bMouseDone = true;
			}
			
		}
		
		update();
	}
	
	void setSquarePos(const QPointF &pos)
	{
		const QRect oldRect = rectangle_around(m_square_pos).toAlignedRect();
		m_square_pos = pos;
		
		//
		float top_x, top_y, bottom_x, bottom_y;
		
		top_x = m_square_pos.x() - cr;
		top_y = m_square_pos.y() - cr;
		
		bottom_x = m_square_pos.x() + cr;
		bottom_y = m_square_pos.y() + cr;
		
		if(top_x<1) top_x = 1;
		if(top_y<1) top_y = 1;
		
		if(bottom_x>sx) bottom_x = sx;
		if(bottom_y>sy) bottom_y = sy;
		
		startMousePos.setX( top_x );
		startMousePos.setY( top_y );
		
		endMousePos.setX( bottom_x );
		endMousePos.setY( bottom_y );
		
	}
	
	QRectF rectangle_around(const QPointF &p, const QSizeF &size = QSize(25, 25))
	{
		QRectF rect(p, size);
		rect.translate(-size.width()/2, -size.height()/2);
		return rect;
	}
	
	// 
	bool checkROIchanged()
	{
		bool flag = false;
		
		if(start_x != startMousePos.x()-1)
		{
			start_x = startMousePos.x()-1;
			flag = true;
		}	
			
		if(end_x != endMousePos.x()-1)
		{
			end_x = endMousePos.x()-1;
			flag = true;
		}
		
		if(start_y != startMousePos.y()-1)
		{
			start_y = startMousePos.y()-1;
			flag = true;
		}
		
		if(end_y != endMousePos.y()-1)
		{
			end_y = endMousePos.y()-1;
			flag = true;
		}
			
		//	
		return flag;
	}
	
public slots:
	// HDR filter
	void do_HDRfilter()
	{
		start_x = startMousePos.x()-1;
		end_x = endMousePos.x()-1;
		
		start_y = startMousePos.y()-1;
		end_y = endMousePos.y()-1;
		
		V3DLONG tmpsum = start_x + end_x;
		start_x = start_x<=end_x?start_x:end_x;
		end_x = tmpsum - start_x;
		
		tmpsum = start_y + end_y;
		start_y = start_y<=end_y?start_y:end_y;
		end_y = tmpsum - start_y;		
		
		if(start_x<0) start_x = 0;
		if(start_y<0) start_y = 0;
		
		if(end_x>sx) end_x = sx-1;
		if(end_y>sy) end_y = sy-1;
		
		V3DLONG pagesz = sx*sy*sz;
		
		// min_max
		V3DLONG c = cur_c-1;
				
		min_roi[c] = INF;
		max_roi[c] = -INF;
		
		V3DLONG offset_c = c*pagesz;
		
		V3DLONG offset_z = cur_z * sx * sy; // current slice
		
		for(V3DLONG j=start_y; j<end_y; j++)
		{
			V3DLONG offset_j = offset_c + offset_z + j*sx;
			for(V3DLONG i=start_x; i<end_x; i++)
			{
				V3DLONG idx = offset_j + i;
				
				float curval = pData1d[idx];
				
				if(min_roi[c]>curval) min_roi[c] = curval;
				if(max_roi[c]<curval) max_roi[c] = curval;
			}
		}
		
		scale_roi[c] = max_roi[c] - min_roi[c];
		
		// filter			
		for(V3DLONG i=0; i<pagesz; i++)
		{
			V3DLONG idx = offset_c + i;
			
			float curval = pData1d[idx];
			
			if(curval<min_roi[c])
			{
				pDispData1d[idx] = min_roi[c];
			}
			else if(curval>max_roi[c])
			{
				pDispData1d[idx] = max_roi[c];
			}
			else
			{
				pDispData1d[idx] = curval;
			}
		}
		
		for(V3DLONG ic=0; ic<sc; ic++)
		{
			if(ic==c) continue;
			
			min_roi[ic] = min_img[ic];
			max_roi[ic] = max_img[ic];
			scale_roi[ic] = scale_img[ic];
		}

		//
		emit roiChanged();
		
	}
	
	// copy data 
	void copydata2disp()
	{
		V3DLONG c = cur_c - 1;
		
		V3DLONG pagesz = sx*sy*sz;
		
		for(V3DLONG ic = 0; ic<sc; ic++)
		{
			if(ic==c) continue;
			
			V3DLONG offset_c = ic*pagesz;
			
			for(V3DLONG i=0; i<pagesz; i++)
			{
				V3DLONG idx = offset_c + i;
				pDispData1d[idx] = pData1d[idx];
			}
			
			min_roi[ic] = min_img[ic];
			max_roi[ic] = max_img[ic];
			scale_roi[ic] = scale_img[ic];
			
		}
	}
	
	// update pixelmap
	void updatePixmap()
	{
		pixmap = getXYPlane((float *)pDispData1d, sx, sy, sz, sc, cur_z, max_roi, min_roi);
		update();
	}

	//
	void colorChannelChanged()
	{
		emit curColorChannelChanged();
	}
	
signals:
	void roiChanged();
	void curZsliceChanged();
	void curColorChannelChanged();
	
public:	
	V3DLONG roi_top, roi_left, roi_bottom, roi_right; // ROI boundary of the search box
	float roi_min, roi_max; // local min and max in the search box

	// mouse events handler
	bool b_mouseleft, b_mouseright;
	bool b_mousemove;
	
	bool bMouseCurorIn, bMouseDone;
	
	QPoint startMousePosL, curMousePosL, startMousePosR, curMousePosR;
	QPoint endMousePosL, endMousePosR;
	QPoint startMousePos, endMousePos;
	int translateMouse_x, translateMouse_y;
	float translateMouse_scale;
	
	QPointF m_square_pos;
    QPointF m_offset;
	
	// disp geometry
	int dispwidth, dispheight;
	float dispscale;
	
	V3DLONG cx, cy, cz, cc, cr;
	V3DLONG cur_x, cur_y, cur_z, cur_c;
	
	float *pDispData1d; // display
	float *pData1d; // ori
	
	QPixmap pixmap;
	
	float min_img[5], max_img[5], scale_img[5]; // assume max color channel is 3
	V3DLONG sx, sy, sz, sc;
	
	// roi
	float min_roi[5], max_roi[5], scale_roi[5]; //
	V3DLONG start_x, end_x, start_y, end_y;
	
	// gui
	float ratio_x2y; // x/y
	float scale_x, scale_y; // assume scale_x = scale_y, i.e. keep the ratio x to y
	
};

// draw a 2D viewer showing the control window
class HDRwidget : public QWidget 
{
	Q_OBJECT
	
public:
	HDRwidget(V3DPluginCallback2 *cb, QWidget *parent, V3DLONG *imgsz, unsigned char *data1d, int datatype)
	{	
		// update current triview in V3D
		callback = (void *)cb;		
		
		b_showinv3d = false;
		//cur_c = 1; // init here
		//		sb_c = new QScrollBar(Qt::Horizontal);
		//		sb_c->setMaximum(sc); sb_c->setMinimum(1); sb_c->setValue(cur_c); sb_c->setSingleStep(1);
		//		QPalette p = palette();
		//		p.setColor(
		//				   QPalette::Base,
		//				   Qt::black);
		//		p.setColor(
		//				   QPalette::Text,
		//				   Qt::yellow);
		//		
		//		sb_c->setPalette(p);
		
		// gui
		
		// color
		QGroupBox *colorGroup = new QGroupBox;
		colorGroup->setTitle(tr("Color Channel"));
		
		rb_r = new QRadioButton(tr("Red"), colorGroup);
		connect(rb_r, SIGNAL(clicked()), this, SLOT(setRedChannel()));
		rb_r->setChecked(true);
		
		rb_g = new QRadioButton(tr("Green"), colorGroup);
		connect(rb_g, SIGNAL(clicked()), this, SLOT(setGreenChannel()));
		
		rb_b = new QRadioButton(tr("Blue"), colorGroup);
		connect(rb_b, SIGNAL(clicked()), this, SLOT(setBlueChannel()));
		
		// view
		viewGroup = new QGroupBox;
		
		pHDRview = new HDRViewer(viewGroup);
		initHDRViwer(imgsz, data1d, datatype);

		pHDRview->dispheight = sx;
		pHDRview->dispwidth = sy;
		pHDRview->setFocusPolicy(Qt::ClickFocus);
		
		pHDRview->cur_c = cur_c = 1;
		
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
		
		// layout		
		QGridLayout *viewGroupLayout = new QGridLayout(viewGroup);
		viewGroupLayout->addWidget(pHDRview, 0, 0);
		//viewGroupLayout->setVerticalSpacing(4);
		
		QGridLayout *sliceGroupLayout = new QGridLayout(sliceGroup);
		sliceGroupLayout->addWidget(sb_z, 0, 0);
		
		QGridLayout *colorLayout = new QGridLayout(colorGroup); 
		colorLayout->addWidget(rb_r, 0, 0);
		colorLayout->addWidget(rb_g, 0, 1);
		colorLayout->addWidget(rb_b, 0, 2);
		
		QGridLayout *gridLayout = new QGridLayout(this);
		gridLayout->addWidget(viewGroup, 0,0, 10, 1);
		gridLayout->addWidget(sliceGroup, 11,0, 1, 1);
		gridLayout->addWidget(colorGroup, 12,0, 1, 1);
		setLayout(gridLayout);
		setWindowTitle(QString("HDR Filtering"));
		
		
		//
		if(pHDRview->pDispData1d && b_showinv3d)
		{
			// syncronizing in triview
			Image4DSimple p4DImage;
			p4DImage.setData((unsigned char*)(pHDRview->pDispData1d), sx, sy, sz, sc, V3D_FLOAT32); //
			
			v3dhandle newwin = ((V3DPluginCallback2 *)callback)->newImageWindow();
			((V3DPluginCallback2 *)callback)->setImage(newwin, &p4DImage);
			((V3DPluginCallback2 *)callback)->setImageName(newwin, "HDR Filtered Image");
			((V3DPluginCallback2 *)callback)->updateImageWindow(newwin);
		}
		
		//
		this->setMinimumSize(64,64);
		resize((float)pHDRview->dispwidth/pHDRview->dispscale, (float)pHDRview->dispheight/pHDRview->dispscale);
		
		//
		connect(sb_z, SIGNAL(valueChanged(int)), this, SLOT(updateZslice()));
		connect(this, SIGNAL(sizeChanged(QSize)), this, SLOT(sizeHandler()));
		
		//
		connect(pHDRview, SIGNAL(curZsliceChanged()), this, SLOT(update()));
		connect(pHDRview, SIGNAL(roiChanged()), this, SLOT(updateCurWin()));
		connect(pHDRview, SIGNAL(curColorChannelChanged()), this, SLOT(update()));
		
		//
		connect(rb_r, SIGNAL(clicked()), this, SLOT(setRedChannel()));
		connect(rb_g, SIGNAL(clicked()), this, SLOT(setGreenChannel()));
		connect(rb_b, SIGNAL(clicked()), this, SLOT(setBlueChannel()));
		
		//
		connect(this, SIGNAL(colorChanged()), pHDRview, SLOT(colorChannelChanged()));
	}
	
	~HDRwidget(){}
	
public:
	void initHDRViwer(V3DLONG *imgsz, unsigned char *data1d, int datatype)
	{
		pHDRview->sx = sx = imgsz[0]; 
		pHDRview->sy = sy = imgsz[1]; 
		pHDRview->sz = sz = imgsz[2]; 
		pHDRview->sc = sc = imgsz[3];
		
		pHDRview->ratio_x2y = (float)sx/(float)sy;
		
		pHDRview->cur_z = cur_z = sz/2; // init here
		
		V3DLONG pagesz = sx*sy*sz;
		V3DLONG imagesz = pagesz*sc;
		
		try
		{
			float *pFloatData = new float [imagesz];
			pHDRview->pDispData1d = new float [imagesz]; //
			
			//
			qDebug()<<"run HDR filtering ...";
			
			if(datatype == V3D_UINT8)
			{
				convering<unsigned char, float>((unsigned char *)data1d, pFloatData, imagesz);
				
				if(data1d) {delete []data1d; data1d = NULL;} 
				
				pHDRview->pData1d = pFloatData; // keep original data in float datatype
				
				convering<float, float>(pHDRview->pData1d, pHDRview->pDispData1d, imagesz); // copy data
				
				// min_max
				for (V3DLONG c=0; c<sc; c++) 
				{
					pHDRview->min_img[c] = INF;
					pHDRview->max_img[c] = -INF;
					
					V3DLONG offset_c = c*pagesz;
					for(V3DLONG i=0; i<pagesz; i++)
					{
						if(pHDRview->min_img[c]>pFloatData[i+offset_c]) pHDRview->min_img[c] = pFloatData[i+offset_c];
						if(pHDRview->max_img[c]<pFloatData[i+offset_c]) pHDRview->max_img[c] = pFloatData[i+offset_c];
					}
					pHDRview->scale_img[c] = pHDRview->max_img[c] - pHDRview->min_img[c];
					
					pHDRview->min_roi[c] = pHDRview->min_img[c];
					pHDRview->max_roi[c] = pHDRview->max_img[c];
					pHDRview->scale_roi[c] = pHDRview->scale_img[c];
					
					qDebug()<<"max min ..."<<pHDRview->max_img[c] << pHDRview->min_img[c];
				}
				
				pHDRview->pixmap = getXYPlane((float *)pFloatData, sx, sy, sz, sc, cur_z, pHDRview->max_img, pHDRview->min_img); // initial focus plane
				
				pHDRview->update();
				pHDRview->repaint();
				
			}
			else if(datatype == V3D_UINT16)
			{
				convering<unsigned short, float>((unsigned short *)data1d, pFloatData, imagesz);
				
				if(data1d) {delete []data1d; data1d = NULL;}
				
				pHDRview->pData1d = pFloatData; // keep original data in float datatype
				
				convering<float, float>(pHDRview->pData1d, pHDRview->pDispData1d, imagesz); // copy data
				
				// min_max
				for (V3DLONG c=0; c<sc; c++) 
				{
					pHDRview->min_img[c] = INF;
					pHDRview->max_img[c] = -INF;
					
					V3DLONG offset_c = c*pagesz;
					for(V3DLONG i=0; i<pagesz; i++)
					{
						if(pHDRview->min_img[c]>pFloatData[i+offset_c]) pHDRview->min_img[c] = pFloatData[i+offset_c];
						if(pHDRview->max_img[c]<pFloatData[i+offset_c]) pHDRview->max_img[c] = pFloatData[i+offset_c];
					}
					pHDRview->scale_img[c] = pHDRview->max_img[c] - pHDRview->min_img[c];
					
					pHDRview->min_roi[c] = pHDRview->min_img[c];
					pHDRview->max_roi[c] = pHDRview->max_img[c];
					pHDRview->scale_roi[c] = pHDRview->scale_img[c];
					
					qDebug()<<"max min ..."<<pHDRview->max_img[c] << pHDRview->min_img[c];
				}
				
				pHDRview->pixmap = getXYPlane((float *)pFloatData, sx, sy, sz, sc, cur_z, pHDRview->max_img, pHDRview->min_img); // initial focus plane
				
				pHDRview->update();
				pHDRview->repaint();
				
			}
			else if(datatype == V3D_FLOAT32)
			{
				convering<float, float>((float *)data1d, pFloatData, imagesz);
				
				if(data1d) {delete []data1d; data1d = NULL;}
			}
			
		}
		catch(...)
		{
			printf("Error allocating memory. \n");
			return;
		}
	}
	
signals:
	void sizeChanged(QSize size);
	void colorChanged();
	
public slots:
	void update()
	{
		viewGroup->setTitle(QString("z %1/%2 x %3 y %4").arg(pHDRview->cur_z+1).arg(sz).arg(sx).arg(sy));
		
		sb_z->setValue(pHDRview->cur_z+1); // keep synchronized
		
		pHDRview->pixmap = getXYPlane((float *)pHDRview->pDispData1d, sx, sy, sz, sc, pHDRview->cur_z, pHDRview->max_roi, pHDRview->min_roi); // initial focus plane
		
		pHDRview->update();
		pHDRview->repaint();

	}
	
	void updateZslice()
	{
		pHDRview->cur_z = cur_z = sb_z->value()-1;
		update();
	}
	
	void sizeHandler()
	{
		qDebug()<<"size changed ...";
		if( (this->size().height() != oriSize.height()) || (this->size().width() != oriSize.width()) )
		{
			oriSize.setHeight(this->size().height());
			oriSize.setWidth(int(float(this->size().height())*pHDRview->ratio_x2y));
			//emit sizeChanged(this->size());
			
			resize(oriSize.width(), oriSize.height()); // keep ratio of x to y
			
			pHDRview->dispheight = oriSize.height() * pHDRview->dispscale;
			
			if(pHDRview->dispheight<1) pHDRview->dispheight = 1;
			pHDRview->dispwidth = pHDRview->dispheight*pHDRview->ratio_x2y;
		}
	}
	
	void resizeEvent(QResizeEvent *re )
	{
		emit sizeChanged(re->size());
	}
		
	void updateCurWin()
	{
		if(b_showinv3d)
		{
			v3dhandle curwin = ((V3DPluginCallback2 *)callback)->currentImageWindow();
			((V3DPluginCallback2 *)callback)->updateImageWindow(curwin);
			
			((V3DPluginCallback2 *)callback)->pushImageIn3DWindow(curwin);
		}
		
	}
	
	//
	void setRedChannel()
	{
		pHDRview->cur_c = cur_c = 1; // red
		
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
		
		pHDRview->copydata2disp();
		
		emit colorChanged();
		update();
	}
	
	void setGreenChannel()
	{
		if(rb_g->isCheckable())
		{
			pHDRview->cur_c = cur_c = 2; // green
			
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
			
			pHDRview->copydata2disp();
			
			emit colorChanged();
			update();
		}
	}
	
	void setBlueChannel()
	{
		if(rb_b->isCheckable())
		{
			pHDRview->cur_c = cur_c = 3; // blue
			
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
			
			pHDRview->copydata2disp();
			
			emit colorChanged();
			update();
		}
		
	}
	
public:
	QGroupBox *viewGroup;
	
	QScrollBar *sb_z;
	
	HDRViewer *pHDRview;
	
	QRadioButton *rb_r;
    QRadioButton *rb_g;
    QRadioButton *rb_b;
	
	V3DLONG sx, sy, sz, sc;
	V3DLONG cur_x, cur_y, cur_z, cur_c;
	
	QSize oriSize;
	
	float scale; 
	
	void *callback;
	
	bool b_showinv3d;
};

#endif



