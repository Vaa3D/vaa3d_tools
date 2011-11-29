//plugin_moviefrom3dviewer.cpp
//by Lei Qu
//2010-11-08

#include <stdio.h>
#include <stdlib.h>

#include "plugin_moviefrom3dviewer.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(moviefrom3dviewer, MovieFrom3DviewerPlugin)

void MovieFrom3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent);
void SnapShoot3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent);

//plugin funcs
const QString title = "Movie From 3D Viewer";
controlPanel* controlPanel::m_pLookPanel = 0;
QStringList MovieFrom3DviewerPlugin::menulist() const
{
	return QStringList()
			<< tr("make movie from 3D viewer")
			<< tr("take a snapshot of 3D viewer");
}

void MovieFrom3DviewerPlugin::domenu(const QString & menu_name,	V3DPluginCallback2 & v3d, QWidget * parent)
{
	if (menu_name == tr("make movie from 3D viewer"))
	{
		MovieFrom3Dviewer(v3d, parent);
	}
	else if (menu_name == tr("take a snapshot of 3D viewer"))
	{
		SnapShoot3Dviewer(v3d, parent);
	}
}

void MovieFrom3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent)
{
	v3dhandle curwin = v3d.currentImageWindow();
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	v3d.open3DWindow(curwin);

	if (controlPanel::m_pLookPanel)
	{
		controlPanel::m_pLookPanel->show();
		return;
	}

	controlPanel* p = new controlPanel(v3d, parent);
	if (p)	p->show();
}

controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
	QDialog(parent), m_v3d(_v3d)
{
	m_pLookPanel = this;
	m_lframeind = 0;

	m_pLineEdit_fps = new QLineEdit(QObject::tr("10"));
	m_pLineEdit_filepath = new QLineEdit();
	QPushButton *pPushButton_start = new QPushButton(QObject::tr("start"));
	QPushButton *pPushButton_stop = new QPushButton(QObject::tr("stop"));
	QPushButton *pPushButton_openFileDlg_output = new QPushButton(QObject::tr("..."));

	QGridLayout *pGridLayout = new QGridLayout();
	pGridLayout->addWidget(new QLabel(QObject::tr("choose fps:")),1,1);
	pGridLayout->addWidget(m_pLineEdit_fps,1,2);
	pGridLayout->addWidget(new QLabel(QObject::tr("choose output dir:")),2,1);
	pGridLayout->addWidget(m_pLineEdit_filepath,2,2);
	pGridLayout->addWidget(pPushButton_openFileDlg_output,2,3);
	pGridLayout->addWidget(pPushButton_start,3,1);
	pGridLayout->addWidget(pPushButton_stop,3,2);

	setLayout(pGridLayout);
	setWindowTitle(QString("make movie"));

	connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
	connect(pPushButton_stop, SIGNAL(clicked()), this, SLOT(_slot_stop()));
	connect(pPushButton_openFileDlg_output, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_output()));

	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
}
controlPanel::~controlPanel()
{
	m_pLookPanel = 0;
}

void controlPanel::_slot_start()
{
	if(m_pLineEdit_filepath->text().isEmpty() || !QDir(m_pLineEdit_filepath->text()).exists())
	{
		v3d_msg("Output dir is empty or do not exist!");
		return;
	}

	long interval = 1.0 / m_pLineEdit_fps->text().toLong() * 1000;
	m_pTimer->start(interval);
}
void controlPanel::_slot_stop()
{
	m_pTimer->stop();
}
void controlPanel::_slots_openFileDlg_output()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose output dir:"));
	d.setFileMode(QFileDialog::Directory);
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath->setText(selectedFile);
	}
}
void controlPanel::_slot_timerupdate()
{
	QString outputdir = m_pLineEdit_filepath->text();

	v3dhandle curwin = m_v3d.currentImageWindow();
	QString BMPfilename = outputdir + QString("/%1").arg(m_lframeind);
	m_v3d.screenShot3DWindow(curwin, BMPfilename);

	printf("%d\n", m_lframeind);
	m_lframeind++;
}

void SnapShoot3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent)
{
	v3dhandle curwin = v3d.currentImageWindow();
	if (!curwin)
	{
		v3d_msg("You don't have any image opened in the main window.");
		return;
	}
	v3d.open3DWindow(curwin);

	QFileDialog d(parent);
	d.setWindowTitle(QObject::tr("Choose output snapshot filename"));
	d.setAcceptMode(QFileDialog::AcceptSave);
	if (!d.exec()) return;

	QString BMPfilename = (d.selectedFiles())[0];
	if (BMPfilename.endsWith(".BMP", Qt::CaseInsensitive))
	   BMPfilename.resize(BMPfilename.length()-4); //by PHC
	v3d.screenShot3DWindow(curwin, BMPfilename);
	QMessageBox::information(0, title, QString("Snapshot was saved to: %1.BMP\n").arg(BMPfilename));
}
