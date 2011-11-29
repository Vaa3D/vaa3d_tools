#ifndef __DIALOG_TEMPLATE_H__
#define __DIALOG_TEMPLATE_H__

#include <QtGui>
#include <v3d_interface.h>

class TwoSidesDialog : public QDialog
{
	Q_OBJECT

public:
	TwoSidesDialog(V3DPluginCallback &callback, QWidget * parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

		label_image = new QLabel(tr("select image :"));
		combo_image = new QComboBox();
		combo_image->addItems(items);

		label_channel = new QLabel(tr("channel :"));
		spin_channel = new QSpinBox();
		spin_channel->setMaximum(3);
		spin_channel->setValue(0);

		label_alpha = new QLabel(tr("alpha :"));
		editor_alpha = new QLineEdit(tr("1.0"));

		label_beta = new QLabel(tr("beta :"));
		editor_beta = new QLineEdit(tr("0.5"));

		label_gamma = new QLabel(tr("gama :"));
		editor_gamma = new QLineEdit(tr("0.5"));

		label_nloops = new QLabel(tr("num of loops :"));
		spin_nloops = new QSpinBox();
		spin_nloops->setMaximum(10000);
		spin_nloops->setSingleStep(20);
		spin_nloops->setValue(500);

		label_radius = new QLabel(tr("local win radius :"));
		spin_radius = new QSpinBox();
		spin_radius->setMaximum(100);
		spin_radius->setValue(20);

		label_newwin = new QLabel(tr("new window :"));
		check_newwin = new QCheckBox();
		check_newwin->setCheckState(Qt::Checked);

		ok = new QPushButton(tr("ok"));
		cancel = new QPushButton(tr("cancel"));

		gridLayout = new QGridLayout();

		gridLayout->addWidget(label_image, 0, 0);
		gridLayout->addWidget(combo_image, 0, 1, 1, 4);
		gridLayout->addWidget(label_channel, 1, 0, 1, 1);
		gridLayout->addWidget(spin_channel, 1, 1, 1, 1);
		gridLayout->addWidget(label_alpha, 1, 3, 1, 1);
		gridLayout->addWidget(editor_alpha, 1, 4, 1, 1);
		gridLayout->addWidget(label_beta, 2, 0, 1, 1);
		gridLayout->addWidget(editor_beta, 2, 1, 1, 1);
		gridLayout->addWidget(label_gamma, 2, 3, 1, 1);
		gridLayout->addWidget(editor_gamma, 2, 4, 1, 1);
		gridLayout->addWidget(label_nloops, 3, 0, 1, 1);
		gridLayout->addWidget(spin_nloops, 3, 1, 1, 1);
		gridLayout->addWidget(label_radius, 3, 3);
		gridLayout->addWidget(spin_radius, 3, 4, 1, 1);
		gridLayout->addWidget(label_newwin, 4, 0);
		gridLayout->addWidget(check_newwin, 4, 1);
		gridLayout->addWidget(cancel, 6, 3, Qt::AlignRight);
		gridLayout->addWidget(ok, 6, 4, Qt::AlignRight);

		setLayout(gridLayout);
		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		connect(combo_image, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(spin_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));

		connect(editor_alpha, SIGNAL(textChanged()), this, SLOT(update()));
		connect(editor_beta, SIGNAL(textChanged()), this, SLOT(update()));
		connect(editor_gamma, SIGNAL(textChanged()), this, SLOT(update()));

		connect(spin_nloops, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(spin_radius, SIGNAL(valueChanged(int)), this, SLOT(update()));


	}

	~TwoSidesDialog(){}

public slots:
	void update()
	{
		i =  combo_image->currentIndex();
		channel =  spin_channel->text().toInt();
		alpha =  editor_alpha->text().toDouble();
		beta =  editor_beta->text().toDouble();
		gamma =  editor_gamma->text().toDouble();
		nloops =  spin_nloops->text().toInt();
		radius =  spin_radius->text().toInt();
		is_newwin =  (check_newwin->checkState() == Qt::Checked);

	}

public:
	int i;
	int channel;
	double alpha;
	double beta;
	double gamma;
	int nloops;
	int radius;
	bool is_newwin;
	QLabel * label_image;
	QComboBox * combo_image;

	QLabel * label_channel;
	QSpinBox * spin_channel;

	QLabel * label_alpha;
	QLineEdit * editor_alpha;

	QLabel * label_beta;
	QLineEdit * editor_beta;

	QLabel * label_gamma;
	QLineEdit * editor_gamma;

	QLabel * label_nloops;
	QSpinBox * spin_nloops;

	QLabel * label_radius;
	QSpinBox * spin_radius;

	QLabel * label_newwin;
	QCheckBox * check_newwin;

	QPushButton * ok;
	QPushButton * cancel;

	QGridLayout * gridLayout;

};


class OneSideOnlyDialog : public QDialog
{
	Q_OBJECT

public:
	OneSideOnlyDialog(V3DPluginCallback &callback, QWidget * parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

		label_image = new QLabel(tr("select image :"));
		combo_image = new QComboBox();
		combo_image->addItems(items);

		label_channel = new QLabel(tr("channel :"));
		spin_channel = new QSpinBox();
		spin_channel->setMaximum(3);
		spin_channel->setValue(0);

		label_alpha = new QLabel(tr("alpha :"));
		editor_alpha = new QLineEdit(tr("1.0"));

		label_beta = new QLabel(tr("beta :"));
		editor_beta = new QLineEdit(tr("0.5"));

		label_gamma = new QLabel(tr("gama :"));
		editor_gamma = new QLineEdit(tr("0.5"));

		label_nloops = new QLabel(tr("num of loops :"));
		spin_nloops = new QSpinBox();
		spin_nloops->setMaximum(10000);
		spin_nloops->setSingleStep(20);
		spin_nloops->setValue(500);

		label_radius = new QLabel(tr("local win radius :"));
		spin_radius = new QSpinBox();
		spin_radius->setMaximum(100);
		spin_radius->setValue(20);

		label_x0 = new QLabel(tr("x0 :"));
		spin_x0 = new QSpinBox();
		spin_x0->setMaximum(100);
		spin_x0->setValue(30);

		label_y0 = new QLabel(tr("y0 :"));
		spin_y0 = new QSpinBox();
		spin_y0->setMaximum(100);
		spin_y0->setValue(0);

		label_x1 = new QLabel(tr("x1 :"));
		spin_x1 = new QSpinBox();
		spin_x1->setMaximum(100);
		spin_x1->setValue(100);

		label_y1 = new QLabel(tr("y1 :"));
		spin_y1 = new QSpinBox();
		spin_y1->setMaximum(100);
		spin_y1->setValue(50);

		label_keep = new QLabel(tr("keep which :"));
		combo_keep = new QComboBox();
		combo_keep->addItem("left");
		combo_keep->addItem("right");

		label_nctrls = new QLabel(tr("control points num :"));
		spin_nctrls = new QSpinBox();
		spin_nctrls->setMaximum(100);
		spin_nctrls->setValue(20);

		label_surface = new QLabel(tr("seprating surface :"));
		check_surface = new QCheckBox();
		check_surface->setCheckState(Qt::Checked);

		label_newwin = new QLabel(tr("new window :"));
		check_newwin = new QCheckBox();
		check_newwin->setCheckState(Qt::Checked);

		ok = new QPushButton(tr("ok"));
		cancel = new QPushButton(tr("cancel"));

		gridLayout = new QGridLayout();

		gridLayout->addWidget(label_image, 0, 0);
		gridLayout->addWidget(combo_image, 0, 1, 1, 4);
		gridLayout->addWidget(label_channel, 1, 0, 1, 1);
		gridLayout->addWidget(spin_channel, 1, 1, 1, 1);
		gridLayout->addWidget(label_alpha, 1, 3, 1, 1);
		gridLayout->addWidget(editor_alpha, 1, 4, 1, 1);
		gridLayout->addWidget(label_beta, 2, 0, 1, 1);
		gridLayout->addWidget(editor_beta, 2, 1, 1, 1);
		gridLayout->addWidget(label_gamma, 2, 3, 1, 1);
		gridLayout->addWidget(editor_gamma, 2, 4, 1, 1);
		gridLayout->addWidget(label_nloops, 3, 0, 1, 1);
		gridLayout->addWidget(spin_nloops, 3, 1, 1, 1);
		gridLayout->addWidget(label_radius, 3, 3);
		gridLayout->addWidget(spin_radius, 3, 4, 1, 1);
		gridLayout->addWidget(label_x0, 4, 0, 1, 1);
		gridLayout->addWidget(spin_x0, 4, 1, 1, 1);
		gridLayout->addWidget(label_y0, 4, 3, 1, 1);
		gridLayout->addWidget(spin_y0, 4, 4, 1, 1);
		gridLayout->addWidget(label_x1, 5, 0, 1, 1);
		gridLayout->addWidget(spin_x1, 5, 1, 1, 1);
		gridLayout->addWidget(label_y1, 5, 3, 1, 1);
		gridLayout->addWidget(spin_y1, 5, 4, 1, 1);
		gridLayout->addWidget(label_keep, 6, 0);
		gridLayout->addWidget(combo_keep, 6, 1);
		gridLayout->addWidget(label_nctrls, 6, 3);
		gridLayout->addWidget(spin_nctrls, 6, 4);
		gridLayout->addWidget(label_surface, 7, 0);
		gridLayout->addWidget(check_surface, 7, 1);
		gridLayout->addWidget(label_newwin, 7, 3);
		gridLayout->addWidget(check_newwin, 7, 4);
		gridLayout->addWidget(cancel, 10, 3, Qt::AlignRight);
		gridLayout->addWidget(ok, 10, 4, Qt::AlignRight);

		setLayout(gridLayout);
		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		connect(combo_image, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(spin_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));

		connect(editor_alpha, SIGNAL(textChanged()), this, SLOT(update()));
		connect(editor_beta, SIGNAL(textChanged()), this, SLOT(update()));
		connect(editor_gamma, SIGNAL(textChanged()), this, SLOT(update()));

		connect(spin_nloops, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(spin_radius, SIGNAL(valueChanged(int)), this, SLOT(update()));

		connect(spin_x0, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(spin_y0, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(spin_x1, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(spin_y1, SIGNAL(valueChanged(int)), this, SLOT(update()));

	}

	~OneSideOnlyDialog(){}

public slots:
	void update()
	{
		i =  combo_image->currentIndex();
		channel =  spin_channel->text().toInt();
		alpha =  editor_alpha->text().toDouble();
		beta =  editor_beta->text().toDouble();
		gamma =  editor_gamma->text().toDouble();
		nloops =  spin_nloops->text().toInt();
		radius =  spin_radius->text().toInt();
		x0 =  spin_x0->text().toInt();
		y0 =  spin_y0->text().toInt();
		x1 =  spin_x1->text().toInt();
		y1 =  spin_y1->text().toInt();
		keep_which =  combo_keep->currentIndex();
		nctrls =  spin_nctrls->text().toInt();
		is_surf =  (check_surface->checkState() == Qt::Checked);
		is_newwin =  (check_newwin->checkState() == Qt::Checked);

	}

public:
	int i;
	int channel;
	double alpha;
	double beta;
	double gamma;
	int nloops;
	int radius;
	int x0;
	int y0;
	int x1;
	int y1;
	int keep_which;
	int nctrls;
	bool is_surf;
	bool is_newwin;
	QLabel * label_image;
	QComboBox * combo_image;

	QLabel * label_channel;
	QSpinBox * spin_channel;

	QLabel * label_alpha;
	QLineEdit * editor_alpha;

	QLabel * label_beta;
	QLineEdit * editor_beta;

	QLabel * label_gamma;
	QLineEdit * editor_gamma;

	QLabel * label_nloops;
	QSpinBox * spin_nloops;

	QLabel * label_radius;
	QSpinBox * spin_radius;

	QLabel * label_x0;
	QSpinBox * spin_x0;

	QLabel * label_y0;
	QSpinBox * spin_y0;

	QLabel * label_x1;
	QSpinBox * spin_x1;

	QLabel * label_y1;
	QSpinBox * spin_y1;

	QLabel * label_keep;
	QComboBox * combo_keep;

	QLabel * label_nctrls;
	QSpinBox * spin_nctrls;

	QLabel * label_surface;
	QCheckBox * check_surface;

	QLabel * label_newwin;
	QCheckBox * check_newwin;

	QPushButton * ok;
	QPushButton * cancel;

	QGridLayout * gridLayout;

};

#endif
