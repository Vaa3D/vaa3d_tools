#include <iostream>

#include "basic_surf_objs.h"
#include "snapshots_3dviewer.h"

using namespace std;
//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(snapshots_3dviewer, SnapShots_3Dviewer)


void snapShots3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent);

//plugin funcs
const QString title = "Saving Snapshots From 3D Viewer";
controlPanel* controlPanel::m_pLookPanel = 0;
QStringList SnapShots_3Dviewer::menulist() const
{
	return QStringList()
			<< tr("take a snapshot of 3D viewer");
}

void SnapShots_3Dviewer::domenu(const QString & menu_name,	V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("take a snapshot of 3D viewer"))
	{
        snapShots3Dviewer(v3d, parent);
	}
}

void SnapShots_3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent)
{
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

void snapShots3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent)
{
    QFileDialog inANO_d(parent);
    inANO_d.setWindowTitle(QObject::tr("Choose input ano filename"));
    inANO_d.setAcceptMode(QFileDialog::AcceptOpen);
    if (!inANO_d.exec()) return;
    QString inANO_fn = (inANO_d.selectedFiles())[0];

    P_ObjectFileType cc;
    if(! loadAnoFile(inANO_fn,cc)){
        cout <<"Fail to load ano file" <<endl;
        return;
    }


    QString output_d=QFileDialog::getExistingDirectory(parent,
    QString(QObject::tr("Choose the outpu directory, where the snapshots would be saved.")));
    QDir output_dir = QFileInfo(output_d).absoluteDir();


    float x_rot = 90;
    float y_rot = 0 ;
    float z_rot = 90;

    for(int i = 0; i <cc.swc_file_list.size(); i++)
    {
        QString swc_file = cc.swc_file_list[i];

        v3d.open3DViewerForSingleSurfaceFile(swc_file);

        QList<V3dR_MainWindow * > list_3dviewer = v3d.getListAll3DViewers();
        V3dR_MainWindow * surface_win = list_3dviewer[i];
        if (!surface_win)
        {
            cout << "surface_win is empty"<<endl;
        }
        View3DControl  *view = v3d.getView3DControl_Any3DViewer(surface_win);

        view->doAbsoluteRot(x_rot,y_rot,z_rot);
        swc_file = QFileInfo(swc_file).fileName();
        QString BMPfilename = output_dir.absolutePath()+ '/'+swc_file;
        //v3d_msg(BMPfilename);
        v3d.screenShot_Any3DViewer(surface_win, BMPfilename);
        //surface_win->close();
    }

}
