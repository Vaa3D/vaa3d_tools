/* stripremove_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-5-13 : by wanwan
 */
 
#ifndef __STRIPREMOVE_PLUGIN_H__
#define __STRIPREMOVE_PLUGIN_H__
//filter_axis_dir  滤波器方向同横轴夹角
//filter_cutoff  截止频率
//filter_radius 滤波器宽度半径
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <QtGui>
#include <v3d_interface.h>



/*------------------------------------------------------定义plugin------------------------------------------------------------------------------------------------------*/
class StripremovePlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

/*------------------------------------------------------define a simple dialog for choose stripremove parameters------------------------------------------------------------------------------------------------------*/

class StripremoveDialog : public QDialog
{
	Q_OBJECT

public:
	StripremoveDialog(V3DPluginCallback2 &cb, QWidget *parent)
	{
		//initialization of variables
		image = 0;
		gridLayout = 0;

		//load image

		v3dhandleList win_list = cb.getImageWindowList();

		if (win_list.size()<1)
		{
			QMessageBox::information(0, "Stripremove option", QObject::tr("No image is open."));
			return;
		}//图片不存在，失败

		//create a dialog
		gridLayout = new QGridLayout();

		image = cb.getImage(cb.currentImageWindow());

		if (!image || !image->valid())//指针，指向valid
		{
			v3d_msg("The image is not valid yet. Check your data.");
			return;
		}

		pRoiList = cb.getROI(cb.currentImageWindow());
		int chn_num = image->getCDim();//通道数

		filter_axis_dir = new QLineEdit(QString("").setNum(90));
		filter_cutoff = new QLineEdit(QString("").setNum(10));
		filter_radius = new QLineEdit(QString("").setNum(10));
		//sigma_editor = new QLineEdit(QString("").setNum(3));
		channel_spinbox = new QSpinBox();
		channel_spinbox->setRange(1, chn_num);//可以选择通道，但不可以超过图片本身最多通道数
		

		/*wx_editor = new QLineEdit(QString("").setNum(7));
		wy_editor = new QLineEdit(QString("").setNum(7));
		wz_editor = new QLineEdit(QString("").setNum(7));
		sigma_editor = new QLineEdit(QString("").setNum(3));
		channel_spinbox = new QSpinBox();
		channel_spinbox->setRange(1, chn_num);
*/

		gridLayout->addWidget(new QLabel("Angle with the horizontal axis(filter_axis_dir)"), 0, 0);
		gridLayout->addWidget(filter_axis_dir, 0, 1, 1, 5);
		gridLayout->addWidget(new QLabel("Cut_off frequency"), 1, 0);
		gridLayout->addWidget(filter_cutoff, 1, 1, 1, 5);
		gridLayout->addWidget(new QLabel("radius of filter"), 2, 0);
		gridLayout->addWidget(filter_radius, 2, 1, 1, 5);
		//gridLayout->addWidget(new QLabel("Sigma value"), 3, 0);
		/*gridLayout->addWidget(new QLabel("Window size (# voxels) along x"), 0, 0);
		gridLayout->addWidget(wx_editor, 0, 1, 1, 5);
		gridLayout->addWidget(new QLabel("Window size (# voxels) along y"), 1, 0);
		gridLayout->addWidget(wy_editor, 1, 1, 1, 5);
		gridLayout->addWidget(new QLabel("Window size (# voxels) along z"), 2, 0);
		gridLayout->addWidget(wz_editor, 2, 1, 1, 5);
		gridLayout->addWidget(new QLabel("Sigma value"), 3, 0);
		gridLayout->addWidget(sigma_editor, 3, 1, 1, 5);*/
		gridLayout->addWidget(new QLabel("Channel"), 3, 0);
		gridLayout->addWidget(channel_spinbox, 3, 1, 1, 5);

		ok = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout->addWidget(cancel, 4, 0);
		gridLayout->addWidget(ok, 4, 1, 1, 5);;

		setLayout(gridLayout);
		setWindowTitle(QString("Stripremove Option"));

		//slot interface
		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		update();
	}

	~StripremoveDialog(){}

	public slots:
	void update()
	{
		angle = atof(filter_axis_dir->text().toStdString().c_str());
		cutoff = atof(filter_cutoff->text().toStdString().c_str());
		radius = atof(filter_radius->text().toStdString().c_str());
		//sigma = atof(sigma_editor->text().toStdString().c_str());
		ch = channel_spinbox->text().toInt();
	}

public:
	int angle, cutoff, radius, ch;
	double sigma;
	Image4DSimple* image;
	ROIList pRoiList;
	QGridLayout *gridLayout;
	QPushButton* ok;
	QPushButton* cancel;
	QLineEdit * filter_axis_dir;
	QLineEdit * filter_cutoff;
	QLineEdit * filter_radius;
	//QLineEdit * sigma_editor;
	QSpinBox * channel_spinbox;
};


/*------------------------------------------------------define FFT -----------------------------------------------------------------------------------------------------*/


//class highpass3d_fftw : public QObject, public V3DPluginInterface2_1
//{
//	Q_OBJECT
//		Q_INTERFACES(V3DPluginInterface2_1);
//
//public:
//	float getPluginVersion() const { return 1.1f; }
//
//	QStringList menulist() const;
//	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
//
//	QStringList funclist() const;
//	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
//};


#endif

