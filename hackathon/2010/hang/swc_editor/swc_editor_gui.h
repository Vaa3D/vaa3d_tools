#ifndef __SWC_EDITOR_GUI_H__
#define __SWC_EDITOR_GUI_H__

#include <QtGui>
#include <v3d_interface.h>
#include "v3d_monitor.h"
#include "basic_surf_objs.h"


class SWCEditorWidget : public QWidget
{
	Q_OBJECT

enum ModeType {DISABLE_MODE,ADD_EDGE_MODE, REMOVE_EDGE_MODE, ADD_VERTEX_MODE, REMOVE_VERTEX_MODE};

public:
	SWCEditorWidget(V3DPluginCallback2 &callback, QWidget * parent) : QWidget(parent)
	{
		this->callback = &callback;
		curwin = callback.currentImageWindow();
		mode = DISABLE_MODE;

		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

		add_edge_mode_button = new QRadioButton(tr("add edge"));
		remove_edge_mode_button = new QRadioButton(tr("remove edge"));
		add_vertex_mode_button = new QRadioButton(tr("add vertex"));
		remove_vertex_mode_button = new QRadioButton(tr("remove vertex"));
		disable_mode_button = new QRadioButton(tr("disable"));
		grid_layout = new QGridLayout();
		grid_layout->addWidget(add_edge_mode_button, 0, 0);
		grid_layout->addWidget(remove_edge_mode_button, 0, 1);
		grid_layout->addWidget(add_vertex_mode_button, 1, 0);
		grid_layout->addWidget(remove_vertex_mode_button, 1, 1);
		grid_layout->addWidget(disable_mode_button, 2, 0);
		set_mode_box = new QGroupBox(tr("set mode :"));
		set_mode_box->setLayout(grid_layout);

		clear_swc_button = new QPushButton(tr("clear swc"));

		vbox = new QVBoxLayout();
		vbox->addWidget(set_mode_box,1);
		vbox->addWidget(clear_swc_button);
		disable_mode_button->setChecked(true);
		this->setLayout(vbox);

		connect(add_edge_mode_button, SIGNAL(clicked()), this, SLOT(onModeChanged()));
		connect(remove_edge_mode_button, SIGNAL(clicked()), this, SLOT(onModeChanged()));
		connect(add_vertex_mode_button, SIGNAL(clicked()), this, SLOT(onModeChanged()));
		connect(remove_vertex_mode_button, SIGNAL(clicked()), this, SLOT(onModeChanged()));
		connect(disable_mode_button, SIGNAL(clicked()), this, SLOT(onModeChanged()));

		connect(clear_swc_button, SIGNAL(clicked()), this, SLOT(onClearSWC()));

		v3d_monitor = new V3dMonitor(&callback, curwin);
		connect(v3d_monitor, SIGNAL(mark_changed(LocationSimple)), this, SLOT(onMouseClicked(LocationSimple)));
		connect(v3d_monitor, SIGNAL(win_closed()), this, SLOT(onParentWindowClosed()));
		//v3d_monitor->start();
	}

	~SWCEditorWidget()
	{
		delete v3d_monitor; 
	}

	bool isValidHandle(v3dhandle win)
	{
		v3dhandleList win_list = callback->getImageWindowList();
		for(int i = 0; i < win_list.size(); i++)
		{
			if(win_list.at(i) == win) return true;
		}
		return false;
	}

public slots:
	void onParentWindowClosed()
	{
		//if(v3d_monitor && v3d_monitor->isRunning()) v3d_monitor->terminate();
		v3d_msg("Stop editing !");
		this->close();
	}

	void onMouseClicked(LocationSimple loc)
	{
		if(curwin == 0 || !isValidHandle(curwin)) return;
		nt = callback->getSWC(curwin);

		if(mode != DISABLE_MODE && nt.listNeuron.empty())
		{
			v3d_msg(tr("Empty neuron list, please load neuron!"));
		}
		else
		{
			v3d_msg(tr("(%1, %2, %3) is clicked! current mode : %4").arg(loc.x).arg(loc.y).arg(loc.z).arg(mode));
		}
	}

	void onModeChanged()
	{
		if(curwin == 0 || !isValidHandle(curwin)) return;
		QRadioButton* button = (QRadioButton * )sender();
		if(button == disable_mode_button)
		{
			mode = DISABLE_MODE;
			if(v3d_monitor && v3d_monitor->isRunning()) v3d_monitor->terminate();
		}
		else
		{
			if(v3d_monitor && !v3d_monitor->isRunning()) v3d_monitor->start();
			if(button == add_edge_mode_button) mode = ADD_EDGE_MODE;
			else if(button == remove_edge_mode_button) mode = REMOVE_EDGE_MODE;
			else if(button == add_vertex_mode_button) mode = ADD_VERTEX_MODE;
			else if(button == remove_vertex_mode_button) mode = REMOVE_VERTEX_MODE; 
		}
	}

	void onClearSWC()
	{
		if(curwin == 0 || !isValidHandle(curwin)) return;
		NeuronTree empty_nt;
		callback->close3DWindow(curwin);
		callback->setSWC(curwin, empty_nt);
		callback->updateImageWindow(curwin);
		callback->open3DWindow(curwin);
	}

public:
	QRadioButton * add_edge_mode_button;
	QRadioButton * remove_edge_mode_button;
	QRadioButton * add_vertex_mode_button;
	QRadioButton * remove_vertex_mode_button;
	QRadioButton * disable_mode_button;
	QGridLayout * grid_layout;
	QGroupBox * set_mode_box;

	QPushButton * clear_swc_button;

	QVBoxLayout * vbox;

	V3DPluginCallback2 * callback;
	v3dhandle curwin;
	V3dMonitor * v3d_monitor;
	ModeType mode;
	NeuronTree nt;
};


class CreateNetworkWidget : public QWidget
{
	Q_OBJECT

	public:
		CreateNetworkWidget(V3DPluginCallback2 &callback, QWidget * parent)
		{
			this->callback = &callback;
			curwin = callback.currentImageWindow();

			v3dhandleList win_list = callback.getImageWindowList();
			QStringList items;
			for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

			ori_x_label = new QLabel(tr("ori x :"));
			ori_x_edit = new QLineEdit("100.00");

			ori_y_label = new QLabel(tr("ori y :"));
			ori_y_edit = new QLineEdit("100.00");

			ori_z_label = new QLabel(tr("ori z :"));
			ori_z_edit = new QLineEdit("1.00");

			nrows_label = new QLabel(tr("rows :"));
			nrows_spin = new QSpinBox();
			nrows_spin->setValue(15);

			ncols_label = new QLabel(tr("cols :"));
			ncols_spin = new QSpinBox();
			ncols_spin->setValue(9);

			nlayers_label = new QLabel(tr("layers :"));
			nlayers_spin = new QSpinBox();
			nlayers_spin->setValue(1);

			radius_label = new QLabel(tr("radius :"));
			radius_edit = new QLineEdit("5.0");

			step_length_label = new QLabel(tr("step length :"));
			step_length_edit = new QLineEdit("20");

			clear_swc_button = new QPushButton(tr("clear swc"));
			create_network_button = new QPushButton(tr("create network"));

			grid_layout = new QGridLayout();
			grid_layout->addWidget(ori_x_label, 0, 0);
			grid_layout->addWidget(ori_x_edit, 0, 1);
			grid_layout->addWidget(ori_y_label, 1, 0);
			grid_layout->addWidget(ori_y_edit, 1, 1);
			grid_layout->addWidget(ori_z_label, 2, 0);
			grid_layout->addWidget(ori_z_edit, 2, 1);
			grid_layout->addWidget(nrows_label, 3, 0);
			grid_layout->addWidget(nrows_spin, 3, 1);
			grid_layout->addWidget(ncols_label, 4, 0);
			grid_layout->addWidget(ncols_spin, 4, 1);
			grid_layout->addWidget(nlayers_label, 5, 0);
			grid_layout->addWidget(nlayers_spin, 5, 1);
			grid_layout->addWidget(radius_label, 6, 0);
			grid_layout->addWidget(radius_edit, 6, 1);
			grid_layout->addWidget(step_length_label, 7, 0);
			grid_layout->addWidget(step_length_edit, 7, 1);
			grid_layout->addWidget(create_network_button, 8, 0, 1, 2);
			grid_layout->addWidget(clear_swc_button, 9, 0, 1, 2);
			group_box = new QGroupBox();
			group_box->setLayout(grid_layout);

			vbox = new QVBoxLayout();
			vbox->addWidget(group_box);

			this->setLayout(vbox);

			connect(create_network_button, SIGNAL(clicked()), this, SLOT(onCreateNetwork()));
			connect(clear_swc_button, SIGNAL(clicked()), this, SLOT(onClearSWC()));

		}

		~CreateNetworkWidget(){}

		bool isValidHandle(v3dhandle win)
		{
			v3dhandleList win_list = callback->getImageWindowList();
			for(int i = 0; i < win_list.size(); i++)
			{
				if(win_list.at(i) == win) return true;
			}
			return false;
		}

		public slots:
			void update()
			{
				ori_x = ori_x_edit->text().toDouble();
				ori_y = ori_y_edit->text().toDouble();
				ori_z = ori_z_edit->text().toDouble();
				nrows =  nrows_spin->text().toInt();
				ncols =  ncols_spin->text().toInt();
				nlayers =  nlayers_spin->text().toInt();
				radius = radius_edit->text().toDouble();
				step_length = step_length_edit->text().toDouble();

			}

		void onCreateNetwork()
		{
			if(curwin == 0 || !isValidHandle(curwin)) return;
			this->update();
			NeuronTree nt;
			QList <NeuronSWC> listNeuron;
			//QHash <int, int>  hashNeuron;

			//v3d_msg(tr("nrows = %1, ncols = %2, nlayers = %3").arg(nrows).arg(ncols).arg(nlayers));

			int count = 0;
			for(int k = 0; k < nlayers; k++)
			{
				for(int j = 0; j < nrows; j++)
				{
					for(int i = 0; i < ncols; i++)
					{
						NeuronSWC ns;
						ns.x = ori_x + step_length * i;
						ns.y = ori_y + step_length * j;
						ns.z = ori_z + step_length * k;
						ns.r = radius;
						ns.type = 2;
						ns.pn = count - 1;
						//ns.pn =  - 1;
						ns.n = count;
						listNeuron.push_back(ns);
						//hashNeuron[count] = count;
						count++;
					}
				}
			}
			nt.listNeuron = listNeuron;
			//nt.hashNeuron = hashNeuron;
			nt.name = tr("Network");
			writeSWC_file("/tmp/testswc.swc", nt);
			//nt = readSWC_file("/tmp/testswc.swc");
			//v3d_msg(tr("size : %1").arg(nt.listNeuron.size()));
			callback->setSWC(curwin, nt);
			callback->updateImageWindow(curwin);
			callback->open3DWindow(curwin);
		}

		void onClearSWC()
		{
			if(curwin == 0 || !isValidHandle(curwin)) return;
			NeuronTree nt;
			callback->close3DWindow(curwin);
			callback->setSWC(curwin, nt);
			callback->updateImageWindow(curwin);
			callback->open3DWindow(curwin);
		}

	public:
		double ori_x;
		double ori_y;
		double ori_z;

		int nrows;
		int ncols;
		int nlayers;

		double radius;
		double step_length;

		QLabel * ori_x_label;
		QLineEdit * ori_x_edit;

		QLabel * ori_y_label;
		QLineEdit * ori_y_edit;

		QLabel * ori_z_label;
		QLineEdit * ori_z_edit;

		QLabel * nrows_label;
		QSpinBox * nrows_spin;

		QLabel * ncols_label;
		QSpinBox * ncols_spin;

		QLabel * nlayers_label;
		QSpinBox * nlayers_spin;

		QLabel * radius_label;
		QLineEdit * radius_edit;

		QLabel * step_length_label;
		QLineEdit * step_length_edit;

		QPushButton * clear_swc_button;
		QPushButton * create_network_button;

		QGridLayout * grid_layout;
		QGroupBox * group_box;

		QVBoxLayout * vbox;

		V3DPluginCallback2 * callback;
		v3dhandle curwin;
};

#endif
