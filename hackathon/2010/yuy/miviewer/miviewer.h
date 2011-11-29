/* miviewer.h
 * 2010-07-28: create this program by Yang Yu
 */


#ifndef __MIVIEWER__
#define __MIVIEWER__

// miviewer
#include <qwidget.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <vector>
#include <list>
#include <bitset>

#include <set>

// reader 
#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

// multithreads
#include <pthread.h>

// interface v3d
#include <QtGui>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

//
using namespace std;

// define indexed data structures
// Define a lookup table
template <class T>
class LUT
{
public:
	
	LUT(){}
	
	LUT(T *a, T *b, bool offset_region)
	{
		T len = 3; //start.size();
		
		if(offset_region)
		{
			init_by_offset(a,b,len);
		}
		else
		{
			init_by_region(a,b,len);
		}
	}
	
	~LUT(){}
	
public:
	void init_by_offset(T *offsets, T *dims, T len)
	{
		try
		{
			start_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			start_pos[i] = offsets[i];
		
		try
		{
			end_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			end_pos[i] = start_pos[i] + dims[i] - 1;
	}
	
	void init_by_region(T *start, T *end, T len)
	{
		try
		{
			start_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			start_pos[i] = start[i];
		
		try
		{
			end_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			end_pos[i] = end[i];
	}
	
	void clear()
	{
		if(start_pos) {delete []start_pos; start_pos=0;}
		if(end_pos) {delete []end_pos; end_pos=0;}
	}
	
	
public:
	
	T *start_pos;
	T *end_pos;
	
	string fn_img;
	
};

// Define a indexed data structure
template <class T1, class T2>
class indexed_t
{
public:
	indexed_t(T1 *in_offsets)
	{
		T1 len = 3; //in_offsets.size();
		
		try
		{
			offsets = new T1 [len];
			sz_image = new T1 [len+1]; // X Y Z C
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		// init
		for(T1 i=0; i<len; i++)
		{
			offsets[i] = in_offsets[i];
			sz_image[i] = 1;
		}
		sz_image[3] = 1;
		
	}
	~indexed_t(){}
	
public:
	T1 *offsets; // ref
	T1 ref_n; // reference image number
	T1 n;
	
	T2 score;
	
	string fn_image; // absolute path + file name
	T1 *sz_image;
	
	T1 predecessor; // adjacent prior image number | root's predecessor is -1
	bool visited; // init by false
	
	std::vector<indexed_t> record;
	
};


// Virtual Image Class
template <class T1, class T2, class indexed_t, class LUT>
class Y_VIM 
{
	
public:
	
	//init
	// creating a hash table
	Y_VIM(list<string> imgList, T2 dims)
	{
		// finding best global alignment
		
		
	}	
	
	Y_VIM(){}
	
	// destructor
	~Y_VIM(){}
	
public:
	
	//load a virtual image
	void y_load(string fn)
	{
		ifstream pFileLUT(fn.c_str());
		string str;
		
		char letter;
		
		T2 start[3], end[3];
		
		sz = new T2 [3];
		
		char buf[2000];
		string fn_str;
		
		if(pFileLUT.is_open())
		{
			//
			pFileLUT >> letter;
			
			if(letter=='#')
				getline(pFileLUT, str);
			
			// tiles
			pFileLUT >> number_tiles;
			
			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str);
			
			// dimensions
			pFileLUT >> sz[0] >> sz[1] >> sz[2] >> sz[3];
			
			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str);
			
			// lut
			lut = new LUT [number_tiles];
			T2 count=0;
			
			while( !pFileLUT.eof() )
			{
				while( getline(pFileLUT, str) )
				{
					istringstream iss(str);
					
					iss >> buf;
					
					fn_str = buf;
					
					//
					iss >> buf; iss >> start[0];
					iss >> buf; iss >> start[1];
					iss >> buf; iss >> start[2];
					
					iss >> buf;
					
					iss >> buf; iss >> end[0];
					iss >> buf; iss >> end[1];
					iss >> buf; iss >> end[2];
					
					lut[count] = LUT(start, end, false);
					lut[count].fn_img = fn_str;
					
					count++;
					
					//iss >> letter;
				}
				
			}
			
		}
		else
		{
			cout << "Unable to open the file"<<endl;
			return;
		}
		
		pFileLUT.close();
		
		
		// adjusting
		T2 len = 3;
		
		try
		{
			min_vim = new T2 [len];
			max_vim = new T2 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T2 i=0; i<len; i++)
		{
			min_vim[i] = 0; max_vim[i] = 0;
		}
		
		for(T2 i=0; i<number_tiles; i++)
		{
			for(T2 j=0; j<len; j++)
			{
				if(lut[i].start_pos[j] < min_vim[j])
					min_vim[j] = lut[i].start_pos[j];
				
				if(lut[i].start_pos[j] > max_vim[j])
					max_vim[j] = lut[i].start_pos[j];
			}
			
		}
	}
	
	//save as a virtual image
	void y_save(string fn)
	{
		FILE *pFileLUT=0;
		
		pFileLUT = fopen(fn.c_str(),"wt");
		
		fprintf(pFileLUT, "# tiles \n");
		fprintf(pFileLUT, "%d \n\n", tilesList.size());
		
		fprintf(pFileLUT, "# dimensions (XYZC) \n");
		fprintf(pFileLUT, "%ld %ld %ld %ld \n\n", max_vim[0]-min_vim[0]+1, max_vim[1]-min_vim[1]+1, max_vim[2]-min_vim[2]+1, tilesList.at(0).sz_image[3]);
		
		fprintf(pFileLUT, "# image coordinates look up table \n");
		for(int j=0; j<tilesList.size(); j++)
		{
			
			string fn = QString(lut[j].fn_img.c_str()).remove(0, QFileInfo(QString(lut[j].fn_img.c_str())).path().length()+1).toStdString();
			
			fprintf(pFileLUT, "%s  ( %ld, %ld, %ld ) ( %ld, %ld, %ld ) \n", fn.c_str(), lut[j].start_pos[0], lut[j].start_pos[1], lut[j].start_pos[2], lut[j].end_pos[0], lut[j].end_pos[1], lut[j].end_pos[2]);
		}
		
		fclose(pFileLUT);
	}
	
	// when add a new one into tileList, need to update the whole tileList
	void y_update()
	{
		
	}
	
	// make a visual image real and be loaded into memory
	void y_visualize(T2 *start, T2 *end)
	{
		
	}
	
	// make a visual image real and be loaded into memory
	void y_visualize()
	{
		
	}
	
	// show a header info
	void y_info()
	{
		
	}
	
	// construct lookup table given adjusted tilesList
	void y_clut(T2 n)
	{
		lut = new LUT [n];
		
		for(T2 i=0; i<n; i++)
		{
			lut[i] = LUT(tilesList.at(i).offsets, tilesList.at(i).sz_image, true);
			
			lut[i].fn_img = tilesList.at(i).fn_image;
		}
		
		// suppose image dimension is unsigned
		T2 len = 3;
		
		try
		{
			min_vim = new T2 [len];
			max_vim = new T2 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T2 i=0; i<len; i++)
		{
			min_vim[i] = 0; max_vim[i] = 0;
		}
		
		for(T2 i=0; i<n; i++)
		{
			for(T2 j=0; j<len; j++)
			{
				if(lut[i].start_pos[j] < min_vim[j])
					min_vim[j] = lut[i].start_pos[j];
				
				if(lut[i].end_pos[j] > max_vim[j])
					max_vim[j] = lut[i].end_pos[j];
			}
			
		}
		
	}
	
	void y_clear()
	{
		if(pVim) {delete []pVim; pVim=0;}
		if(sz) {delete []sz; sz=0;}
		
		if(min_vim) {delete []min_vim; min_vim=0;}
		if(max_vim) {delete []max_vim; max_vim=0;}
		if(lut) {delete []lut; lut=0;}
	}
	
public:
	
	T1 *pVim;
	T2 *sz;
	
	vector<indexed_t> tilesList;
	bitset<3> relative_dir; // 000 'f', 'u', 'l' ; 111 'b', 'd', 'r'; relative[2] relative[1] relative[0] 
	
	LUT *lut;
	T2 *min_vim, *max_vim;
	
	T2 number_tiles;
	
};

//
class tri_view_plane : public QLabel 
{
public:
	tri_view_plane(QString text)
	{
		QLabel(test);
	}
	
public:
	void paintEvent(QPaintEvent *event)
	{
		//
		//QWidget::paintEvent(event);
		
		//
		QPainter painter(this); 
		
		// atlas image
		
		if(plane_n == 0)
		{
				// xy
				QImage xy_image(cx,cy,QImage::Format_RGB32);
				
				for (long j = 0; j < cy; j ++) 
				{
					long offset = cur_z*cx*cy + j*cx;
					for (long i=0; i<cx; i++) 
					{
						long idx = offset + i;
						
						xy_image.setPixel(i,j,qRgb(compressed1d[idx], compressed1d[idx+channel_compressed_sz], compressed1d[idx+2*channel_compressed_sz]));
					}
				}
				
				painter.setCompositionMode(QPainter::CompositionMode_Source);
				painter.drawImage(0,0, xy_image);
				
		}
		else if(plane_n == 1)
		{
				
				// yz
				QImage zy_image(cz,cy,QImage::Format_RGB32);
				
				for (long j = 0; j < cy; j ++) 
				{
					long offset = j*cx + cur_x;
					for (long k =0; k < cz; k++) 
					{
						long idx = offset + k*cx*cy;
						
						zy_image.setPixel(k,j,qRgb(compressed1d[idx], compressed1d[idx+channel_compressed_sz], compressed1d[idx+2*channel_compressed_sz]));
					}
				}
				
				painter.setCompositionMode(QPainter::CompositionMode_Source);
				painter.drawImage(0,0, zy_image);				
				
		}
		else if( plane_n == 2)
		{
				
				// xz
				QImage xz_image(cx,cz,QImage::Format_RGB32);
				
				for (long k = 0; k < cz; k ++) 
				{
					long offset = k*cx*cy + cur_y*cx;
					for (long i =0; i < cx; i++) 
					{
						long idx = offset + i;
						
						xz_image.setPixel(i,k,qRgb(compressed1d[idx], compressed1d[idx+channel_compressed_sz], compressed1d[idx+2*channel_compressed_sz]));
					}
				}
				
				painter.setCompositionMode(QPainter::CompositionMode_Source);
				painter.drawImage(0,0, xz_image);				
				
		}
		else
			return;
		
		qDebug()<<"draw ..."<<cur_x<<cur_y<<cur_z<<cx<<cy<<cz;
		
		// ROI
		
		painter.setPen( Qt::white );				// draw outline Qt::SolidLine
		painter.setBrush( Qt::NoBrush );	// set random brush color colors[rand() % 255] Qt::NoBrush
		
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // override
		
		QPoint p1( roi_top, roi_left );    // p1 = top left
		QPoint p2( roi_bottom, roi_right );    // p2 = bottom right
		
		QRect r( p1, p2 );
		painter.drawRect( r );
		
		qDebug()<<"paint ..."<<roi_top<<roi_left<<roi_bottom<<roi_right;
		
	}
	
public:
	long roi_top, roi_left, roi_bottom, roi_right;
	//QImage sourceImage;
	
	long channel_compressed_sz;
	long cx, cy, cz;
	long cur_x, cur_y, cur_z;
	long plane_n;
	
	unsigned char *compressed1d;
	
};

// MIViewer interface
class ImageNavigatingWidget : public QWidget
{
	Q_OBJECT
	
public:
	ImageNavigatingWidget(V3DPluginCallback &callback, QString m_FileName, QString curFilePathInput, float scaleFactorInput)
	{
		// load tile configuration file
		curFilePath = curFilePathInput;
		string filename = m_FileName.toStdString();

		vim.y_load(filename);
		
		long sx=vim.sz[0], sy=vim.sz[1], sz=vim.sz[2];
		
		// compressed data
		//****************************************************************
		// suppose compressed image saved as .tif
		QString m_FileName_compressed = m_FileName;
		
		m_FileName_compressed.chop(3); // ".tc"
		m_FileName_compressed.append(".tif");
		
		// loading compressed image files
		V3DLONG *sz_compressed = 0; 
		int datatype_compressed = 0;
		compressed1d = 0;
		
		loadImage(const_cast<char *>(m_FileName_compressed.toStdString().c_str()), compressed1d, sz_compressed, datatype_compressed); //careful
		
		cx=sz_compressed[0], cy=sz_compressed[1], cz=sz_compressed[2], cc=sz_compressed[3];
		channel_compressed_sz = cx*cy*cz;
		
		// initial
		init_x = cx/2, init_y = cy/2, init_z = cz/2; 
		
		scaleFactor = scaleFactorInput;
		cur_x = init_x*scaleFactor, cur_y = init_y*scaleFactor, cur_z = init_z*scaleFactor;
		
		//******************************************************************
		// create a widget
		label_xy = new tri_view_plane(QObject::tr("xy-view")); 
		label_zy = new tri_view_plane(QObject::tr("zy-view")); 
		label_xz = new tri_view_plane(QObject::tr("xz-view")); 
		
		//
		label_x = new QLabel(QObject::tr("current tri-view plane: x"));
		label_y = new QLabel(QObject::tr("y"));
		label_z = new QLabel(QObject::tr("z"));
		
		spin_x = new QSpinBox();
		spin_y = new QSpinBox();
		spin_z = new QSpinBox();
		
		spin_x->setMaximum(sx); spin_x->setMinimum(0); spin_x->setValue(cur_x); spin_x->setSingleStep(int(scaleFactor));
		spin_y->setMaximum(sy); spin_y->setMinimum(0); spin_y->setValue(cur_y); spin_y->setSingleStep(int(scaleFactor));
		spin_z->setMaximum(sz); spin_z->setMinimum(0); spin_z->setValue(cur_z); spin_z->setSingleStep(int(scaleFactor));
		
		// choose focus plane
		label_plane = new QLabel(QObject::tr("choose focus plane: "));
		combo_plane =  new QComboBox(); 
		combo_plane->addItem("xy-view");
		combo_plane->addItem("yz-view");
		combo_plane->addItem("xz-view");
		
		//
		zoom_in  = new QPushButton("Zoom In");
		zoom_out = new QPushButton("Zoom Out");
		
		//
		move_up  = new QPushButton("Up");
		move_down = new QPushButton("Down");
		move_left  = new QPushButton("Left");
		move_right = new QPushButton("Right");
		
		//
		syn = new QPushButton("Syn");
		
		//gridlayout
		QGridLayout* gridLayout = new QGridLayout(this);
		
		gridLayout->addWidget(label_xy, 0,0,1,2); gridLayout->addWidget(label_zy, 0,3,1,2);
		gridLayout->addWidget(label_xz, 1,0,1,2);
		
		gridLayout->addWidget(label_plane, 2,0,1,1); gridLayout->addWidget(combo_plane, 2,1,1,1);
		
		gridLayout->addWidget(label_x, 3,0,1,1); gridLayout->addWidget(spin_x, 3,1,1,1);
		gridLayout->addWidget(label_y, 3,2,1,1); gridLayout->addWidget(spin_y, 3,3,1,1);
		gridLayout->addWidget(label_z, 3,4,1,1); gridLayout->addWidget(spin_z, 3,5,1,1);
		
		gridLayout->addWidget(move_up, 4,1,1,1); gridLayout->addWidget(move_down, 6,1,1,1);
		gridLayout->addWidget(move_left, 5,0,1,1); gridLayout->addWidget(move_right, 5,3,1,1);
		
		gridLayout->addWidget(zoom_in, 7,0,1,1); gridLayout->addWidget(zoom_out, 7,1,1,1); gridLayout->addWidget(syn, 7,2,1,1);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Image Navigation Control"));
		
		//init
		roi_start_x = cx/2, roi_start_y = cy/2, roi_start_z = cz/2; //
		roi_end_x = 0, roi_end_y = 0, roi_end_z = 0;
		
		wx = cx/8, wy = cy/8, wz = cz/4;
		
		//
		update_roi(roi_start_x, roi_start_y, roi_start_z, wx, wy, wz);
		
		// init tri-view
		update_triview(cur_x, cur_y, cur_z);
		
		// init roi
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		
		//
		label_xy->cx = cx; label_xy->cy = cy; label_xy->cz = cz;
		label_xy->channel_compressed_sz = channel_compressed_sz;
		label_xy->compressed1d = compressed1d;
		label_xy->plane_n = 0; // defined
		
		label_zy->cx = cx; label_zy->cy = cy; label_zy->cz = cz;
		label_zy->channel_compressed_sz = channel_compressed_sz;
		label_zy->compressed1d = compressed1d;
		label_zy->plane_n = 1; // defined
		
		label_xz->cx = cx; label_xz->cy = cy; label_xz->cz = cz;
		label_xz->channel_compressed_sz = channel_compressed_sz;
		label_xz->compressed1d = compressed1d;
		label_xz->plane_n = 2; // defined
		
		//
		flag_changed = 0;
		flag_syn = false;
		
		// init v3d windows
		update_v3dviews(callback, roi_start_x*scaleFactor, roi_start_y*scaleFactor, roi_start_z*scaleFactor, roi_end_x*scaleFactor, roi_end_y*scaleFactor, roi_end_z*scaleFactor);
		
		//signal and slot
		connect(combo_plane, SIGNAL(currentIndexChanged(int)), this, SLOT(update_windows()));
		
		connect(move_up,     SIGNAL(clicked()), this, SLOT(update_up()));
		connect(move_down,     SIGNAL(clicked()), this, SLOT(update_down()));
		connect(move_left,     SIGNAL(clicked()), this, SLOT(update_left()));
		connect(move_right,     SIGNAL(clicked()), this, SLOT(update_right()));
	
		connect(spin_x, SIGNAL(valueChanged(int)), this, SLOT(update_windows()));
		connect(spin_y, SIGNAL(valueChanged(int)), this, SLOT(update_windows()));
		connect(spin_z, SIGNAL(valueChanged(int)), this, SLOT(update_windows()));
		
		connect(zoom_in,     SIGNAL(clicked()), this, SLOT(update_zoomin()));
		connect(zoom_out,     SIGNAL(clicked()), this, SLOT(update_zoomout()));
		
		connect(syn,     SIGNAL(clicked()), this, SLOT(update_syn())); //
		
	}
	
public slots:
	void update_windows()
	{
		plane_n = combo_plane->currentIndex();
		
		cur_x = spin_x->text().toInt(); // / scaleFactor;
		cur_y = spin_y->text().toInt(); // / scaleFactor;
		cur_z = spin_z->text().toInt(); // / scaleFactor;
		
		update_triview(cur_x, cur_y, cur_z);
		
		label_xy->update();
		label_xy->repaint();
		
		label_zy->update();
		label_zy->repaint();
		
		label_xz->update();
		label_xz->repaint();
	}
	
	void update_zoomin()
	{
		wx *= 2;
		wy *= 2;
		wz *= 2;
		
		roi_end_x = roi_start_x + wx; roi_end_y = roi_start_y + wy; roi_end_z = roi_start_z + wz;
		
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		
		if(label_xy->roi_bottom>=cx) label_xy->roi_bottom = cx-1;
		if(label_zy->roi_bottom>=cz) label_zy->roi_bottom = cz-1;
		if(label_xz->roi_bottom>=cx) label_xz->roi_bottom = cx-1;
		
		if(label_xy->roi_right>=cy) label_xy->roi_right = cy-1;
		if(label_zy->roi_right>=cy) label_zy->roi_right = cy-1;
		if(label_xz->roi_right>=cz) label_xz->roi_right = cz-1;
		
		label_xy->update();
		label_xy->repaint();
		
		label_zy->update();
		label_zy->repaint();
		
		label_xz->update();
		label_xz->repaint();
		
	}
	
	void update_zoomout()
	{
		wx /= 2;
		wy /= 2;
		wz /= 2;
		
		if(wx<1) wx=1;
		if(wy<1) wy=1;
		if(wz<1) wz=1;
		
		roi_end_x = roi_start_x + wx; roi_end_y = roi_start_y + wy; roi_end_z = roi_start_z + wz;
		
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		
		
		label_xy->update();
		label_xy->repaint();
		
		label_zy->update();
		label_zy->repaint();
		
		label_xz->update();
		label_xz->repaint();
		
	}
	
	void update_up()
	{
		plane_n = combo_plane->currentIndex();
		
		switch (plane_n) 
		{
			case 0:
				// xy
			case 1:
				
				// yz
				roi_start_y--; //roi_end_y--;
				if(roi_start_y<0)
				{
					roi_start_y++; //roi_end_y++;
				}				
				
				break;
				
			case 2:
				
				// xz
				roi_start_z--; //roi_end_z--;
				if(roi_start_z<0)
				{
					roi_start_z++; //roi_end_z++;
				}
				
				break;
				
			default:
				break;
		}
	
		roi_end_x = roi_start_x + wx; roi_end_y = roi_start_y + wy; roi_end_z = roi_start_z + wz;
		
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		
		label_xy->update();
		label_xy->repaint();
		
		label_zy->update();
		label_zy->repaint();
		
		label_xz->update();
		label_xz->repaint();
		
		update_flag();
	}
	
	void update_down()
	{
		plane_n = combo_plane->currentIndex();
		
		switch (plane_n) 
		{
			case 0:
				// xy
			case 1:
				
				// yz
				roi_end_y++; //roi_start_y++; 
				if(roi_end_y>=cy)
				{
					roi_end_y--; //roi_start_y--; 
				}				
				
				break;
				
			case 2:
				
				// xz
				roi_end_z++; //roi_start_z++; 
				if(roi_end_z>=cz)
				{
					 roi_end_z--; //roi_start_z--;
				}
				
				break;
				
			default:
				break;
		}
		
		roi_start_x = roi_end_x-wx; roi_start_y = roi_end_y-wy; roi_start_z = roi_end_z-wz;
		
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		
		label_xy->update();
		label_xy->repaint();
		
		label_zy->update();
		label_zy->repaint();
		
		label_xz->update();
		label_xz->repaint();
		
		update_flag();
		
	}
	
	void update_left()
	{
		plane_n = combo_plane->currentIndex();
		
		switch (plane_n) 
		{
			case 0:
				// xy
			case 2:
				
				// xz
				roi_start_x--; //roi_end_x--;
				if(roi_start_x<0)
				{
					roi_start_x++; //roi_end_x++;
				}
				
				break;
				
			case 1:
				
				// zy
				roi_start_z--; //roi_end_z--;
				if(roi_start_z<0)
				{
					roi_start_z++; //roi_end_z++;
				}
				
				break;
				
			default:
				break;
		}
		
		
		roi_end_x = roi_start_x + wx; roi_end_y = roi_start_y + wy; roi_end_z = roi_start_z + wz;
		
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		
		label_xy->update();
		label_xy->repaint();
		
		label_zy->update();
		label_zy->repaint();
		
		label_xz->update();
		label_xz->repaint();
		
		update_flag();
		
	}
	
	void update_right()
	{
		plane_n = combo_plane->currentIndex();
		
		switch (plane_n) 
		{
			case 0:
				// xy
			case 2:
				
				// xz
				roi_end_x++; //roi_start_x++; 
				if(roi_end_x>=cx)
				{
					roi_end_x--; //roi_start_x--; 
				}
				
				break;
				
			case 1:
				
				// zy
				roi_end_z++; //roi_start_z++; 
				if(roi_end_z>=cz)
				{
					roi_end_z--; //roi_start_z--; 
				}
				
				break;
				
			default:
				break;
		}
		
		roi_start_x = roi_end_x-wx; roi_start_y = roi_end_y-wy; roi_start_z = roi_end_z-wz;
		
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		
		label_xy->update();
		label_xy->repaint();
		
		label_zy->update();
		label_zy->repaint();
		
		label_xz->update();
		label_xz->repaint();
		
		update_flag();
	}
	
	
	void update_roi(long start_x, long start_y, long start_z, long wx, long wy, long wz)
	{
		// ROI
		roi_start_x = start_x, roi_start_y = start_y, roi_start_z = start_z;
		roi_end_x = roi_start_x + wx, roi_end_y = roi_start_y + wy, roi_end_z = roi_start_z + wz;
		
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
	}
	
	void update_triview(long cur_x, long cur_y, long cur_z)
	{
		qDebug()<<"update_triview ..."<<cur_x<<cur_y<<cur_z;
		
		if(!compressed1d)
			return;
		
		cur_x /= scaleFactor;
		cur_y /= scaleFactor;
		cur_z /= scaleFactor;
		
		// xy
		label_xy->cur_x = cur_x;
		label_xy->cur_y = cur_y;
		label_xy->cur_z = cur_z;
		
		// zy
		label_zy->cur_x = cur_x;
		label_zy->cur_y = cur_y;
		label_zy->cur_z = cur_z;
		
		// xz
		label_xz->cur_x = cur_x;
		label_xz->cur_y = cur_y;
		label_xz->cur_z = cur_z;
		
	}
	
	void update_v3dviews(V3DPluginCallback &callback, long start_x, long start_y, long start_z, long end_x, long end_y, long end_z)
	{
		// visualize in v3d tri-view
		
		size_t start_t = clock();
		
		
		//virtual image
		long vx, vy, vz, vc;
		
		vx = end_x - start_x + 1; // suppose the size same of all tiles
		vy = end_y - start_y + 1;
		vz = end_z - start_z + 1;
		vc = vim.sz[3];
		
		long pagesz_vim = vx*vy*vz*vc;
		
		unsigned char *pVImg = 0;
		
		try
		{
			pVImg = new unsigned char [pagesz_vim];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		// init
		for(long i=0; i<pagesz_vim; i++)
		{
			pVImg[i] = 0;
		}
		
		// flu bird algorithm
		// 000 'f', 'l', 'u' ; 111 'b', 'r', 'd'; relative[2] relative[1] relative[0] 
		bitset<3> lut_ss, lut_se, lut_es, lut_ee;
		
		// 
		long x_s = start_x + vim.min_vim[0];
		long y_s = start_y + vim.min_vim[1];
		long z_s = start_z + vim.min_vim[2];
		
		long x_e = end_x + vim.min_vim[0];
		long y_e = end_y + vim.min_vim[1];
		long z_e = end_z + vim.min_vim[2];
		
		//
		ImagePixelType datatype;
		
		// look up lut
		for(long ii=0; ii<vim.number_tiles; ii++)
		{	
			// init
			lut_ss.reset();
			lut_se.reset();
			lut_es.reset();
			lut_ee.reset();
			
			//
			if(x_s < vim.lut[ii].start_pos[0]) lut_ss[1] = 1; // r  0 l
			if(y_s < vim.lut[ii].start_pos[1]) lut_ss[0] = 1; // d  0 u
			if(z_s < vim.lut[ii].start_pos[2]) lut_ss[2] = 1; // b  0 f
			
			if(x_e < vim.lut[ii].start_pos[0]) lut_se[1] = 1; // r  0 l
			if(y_e < vim.lut[ii].start_pos[1]) lut_se[0] = 1; // d  0 u
			if(z_e < vim.lut[ii].start_pos[2]) lut_se[2] = 1; // b  0 f
			
			if(x_s < vim.lut[ii].end_pos[0]) lut_es[1] = 1; // r  0 l
			if(y_s < vim.lut[ii].end_pos[1]) lut_es[0] = 1; // d  0 u
			if(z_s < vim.lut[ii].end_pos[2]) lut_es[2] = 1; // b  0 f
			
			if(x_e < vim.lut[ii].end_pos[0]) lut_ee[1] = 1; // r  0 l
			if(y_e < vim.lut[ii].end_pos[1]) lut_ee[0] = 1; // d  0 u
			if(z_e < vim.lut[ii].end_pos[2]) lut_ee[2] = 1; // b  0 f
			
			// copy data
			if( (!lut_ss.any() && lut_ee.any()) || (lut_es.any() && !lut_ee.any()) || (lut_ss.any() && !lut_se.any()) )
			{
				// 
				cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;
				
				//
				char * curFileSuffix = getSurfix(const_cast<char *>(vim.lut[ii].fn_img.c_str()));
				
				cout << "suffix ... " << curFileSuffix << endl; // tif lsm
				
				QString curPath = curFilePath;
				
				string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();
				
				qDebug()<<"testing..."<<curFilePath<< fn.c_str();
				
				//
				char * imgSrcFile = const_cast<char *>(fn.c_str());
				
				size_t s1_t = clock();
				
				// loading relative imagg files
				V3DLONG *sz_relative = 0; 
				int datatype_relative = 0;
				unsigned char* relative1d = 0;
				
				loadImage(imgSrcFile, relative1d, sz_relative, datatype_relative); //
				
				long rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
				
				if(datatype_relative==1)
					datatype = V3D_UINT8;
				
				size_t e1_t = clock();
				cout<<"time elapse for read tmpstack ... "<<e1_t-s1_t<<endl;
				
				
				
				//
				long tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0]; 
				long tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0]; 
				long tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1]; 
				long tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1]; 
				long tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2]; 
				long tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2]; 
				
				long x_start = (start_x > tile2vi_xs) ? start_x : tile2vi_xs; 
				long x_end = (end_x < tile2vi_xe) ? end_x : tile2vi_xe;
				long y_start = (start_y > tile2vi_ys) ? start_y : tile2vi_ys;
				long y_end = (end_y < tile2vi_ye) ? end_y : tile2vi_ye;
				long z_start = (start_z > tile2vi_zs) ? start_z : tile2vi_zs;
				long z_end = (end_z < tile2vi_ze) ? end_z : tile2vi_ze;
				
				x_end++;
				y_end++;
				z_end++;
				
				//
				//cout << x_start << " " << x_end << " " << y_start << " " << y_end << " " << z_start << " " << z_end << endl;
			
				//
				for(long c=0; c<rc; c++)
				{
					long o_c = c*vx*vy*vz;
					long o_r_c = c*rx*ry*rz;
					for(long k=z_start; k<z_end; k++)
					{
						long o_k = o_c + (k-start_z)*vx*vy;
						long o_r_k = o_r_c + (k-z_start)*rx*ry;
						
						for(long j=y_start; j<y_end; j++)
						{
							long o_j = o_k + (j-start_y)*vx;
							long o_r_j = o_r_k + (j-y_start)*rx;
							for(long i=x_start; i<x_end; i++)
							{
								long idx = o_j + i-start_x;
								long idx_r = o_r_j + (i-x_start);
								
								if(pVImg[idx]>0)
								{
									pVImg[idx] = (pVImg[idx]>relative1d[idx_r])?pVImg[idx]:relative1d[idx_r];
								}
								else
								{
									pVImg[idx] = relative1d[idx_r];
								}
							}
						}
					}
				}
				
				//de-alloc
				if(sz_relative) {delete []sz_relative; sz_relative=0;}
				if(relative1d) {delete []relative1d; relative1d=0;}

			}
			
		}
		
		size_t end1_t = clock();
		
		cout<<"time elapse ... "<<end1_t-start_t<<endl;
		
		//display
		Image4DSimple p4DImage;
		
		p4DImage.setData((unsigned char*)pVImg, vx, vy, vz, vc, datatype);
		
		v3dhandle curwin;
		
		if(!callback.currentImageWindow())
			curwin = callback.newImageWindow();
		else
			curwin = callback.currentImageWindow();
			
		callback.setImage(curwin, &p4DImage);
		callback.setImageName(curwin, "ROI of A Virtual Image");
		callback.updateImageWindow(curwin);
		
		callback.pushImageIn3DWindow(curwin);
		
		// time consumption
		size_t end_t = clock();
		
		cout<<"time elapse after loading configuration info ... "<<end_t-start_t<<endl;
	}
	
	void update_flag()
	{
		flag_changed++;
	}
	
	void update_syn()
	{
		flag_syn = true;
	}
	
	
public:
	tri_view_plane* label_xy;
	tri_view_plane* label_zy;
	tri_view_plane* label_xz;
	
	QPushButton* zoom_in;
	QPushButton* zoom_out;
	
	QLabel* label_x;
	QSpinBox* spin_x;
	QLabel* label_y;
	QSpinBox* spin_y;
	QLabel* label_z;
	QSpinBox* spin_z;
	
	QLabel* label_plane;
	QComboBox* combo_plane;
	
	QPushButton* move_up;
	QPushButton* move_down;
	QPushButton* move_left;
	QPushButton* move_right;
	
	QPushButton* syn;
	
public:
	long cx, cy, cz, cc; // compressed data
	long cur_x, cur_y, cur_z;
	long channel_compressed_sz;
	long init_x, init_y, init_z; // control window
	long wx, wy, wz;
	long roi_start_x, roi_start_y, roi_start_z;
	long roi_end_x, roi_end_y, roi_end_z;
	
	unsigned char *compressed1d;
	QString curFilePath;
	
	//control
	int plane_n; // 0 xy 1 zy 2 xz
	float scaleFactor;
	
	// virtual image
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	
	// syn
	long flag_changed;
	bool flag_syn;
	
};


// interface v3d plugin
class MIViewerPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
// v3d interface	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

};


// indexed data structure
class IndexedData
{
	
};


#endif



