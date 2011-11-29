#ifndef __DIALOG_TEMPLATE_H__
#define __DIALOG_TEMPLATE_H__

#include <QtGui>
#include <v3d_interface.h>
#include "mouse_monitor.h"

class MouseMonitorWidget : public QWidget
{
	Q_OBJECT

public:
	MouseMonitorWidget(V3DPluginCallback2 & cb, QWidget * parent) : QWidget(parent)
	{
		this->callback = &cb;
		curwin = cb.currentImageWindow();
		monitor = 0;

		start_button = new QPushButton(tr("Start"));
		stop_button = new QPushButton(tr("Stop"));
		exit_button = new QPushButton(tr("Exit"));

		hbox = new QHBoxLayout();

		hbox->addWidget(start_button);
		hbox->addWidget(stop_button);
		hbox->addWidget(exit_button);

		setLayout(hbox);
		connect(start_button, SIGNAL(clicked()), this, SLOT(onStart()));
		connect(stop_button, SIGNAL(clicked()), this, SLOT(onStop()));
		connect(exit_button, SIGNAL(clicked()), this, SLOT(onExit()));
	}

	~MouseMonitorWidget()
	{
		if(monitor) monitor->terminate();
	}

public slots:
	void onMarkChanged(LocationSimple loc)
	{
		v3d_msg(tr("(%1, %2, %3) is clicked !").arg(loc.x).arg(loc.y).arg(loc.z));
	}
	
	void onWinClosed()
	{
		v3d_msg(tr("Stop monitor ! Enjoy!"));
		this->close();
	}

	void onStart()
	{
		if(curwin==0) {v3d_msg("No image is opened!"); return;}
		if(monitor==0)
		{
			monitor = new MouseMonitor(callback, curwin);
			connect(monitor, SIGNAL(mark_changed(LocationSimple)), this, SLOT(onMarkChanged(LocationSimple)));
			connect(monitor, SIGNAL(win_closed()), this, SLOT(onWinClosed()));
			v3d_msg("Try to create landmark in 3d view!");
		}		
		if(!monitor->isRunning())monitor->start();
		callback->open3DWindow(curwin);
	}

	void onStop()
	{
		if(monitor && monitor->isRunning()) monitor->terminate();
	}

	void onExit()
	{
		if(monitor) monitor->terminate();
		this->close();
	}
public:
	QPushButton * start_button;
	QPushButton * stop_button;
	QPushButton * exit_button;

	QHBoxLayout * hbox;
	
	V3DPluginCallback2 * callback;
	v3dhandle curwin;

	MouseMonitor * monitor;
};

#endif
