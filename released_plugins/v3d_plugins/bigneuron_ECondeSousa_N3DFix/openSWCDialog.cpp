#include "openSWCDialog.h"

OpenSWCDialog::OpenSWCDialog(QWidget * parent, V3DPluginCallback2 * _callback)
: QDialog(parent)
{
	callback = _callback;
	QVBoxLayout * dlgLayout = new QVBoxLayout;

	QTabWidget * tabWidget = new QTabWidget(this);
	//page open from 3D viewers
	QDialog * open3DDlg = new QDialog(tabWidget);
	QVBoxLayout* layout  = new QVBoxLayout;

	listWidget = new QListWidget(open3DDlg);
	getAllNeuronTreeNames(callback);
	QStringList fileList;
	for (V3DLONG i=0;i<_3DViewerTreeList.size();i++)
		fileList << _3DViewerTreeList[i].file;
	listWidget->addItems(fileList);
	layout->addWidget(listWidget);

	QPushButton * bt = new QPushButton(tr("choose"));
	connect(bt, SIGNAL(clicked()), this, SLOT(run()));
	//connect(bt, SIGNAL(clicked()), this, SLOT(accept()));
	layout->addWidget(bt);

	open3DDlg->setLayout(layout);
	tabWidget->addTab(open3DDlg, tr("Open from 3D Viewer"));

	//page open from file
	QFileDialog * fileDlg = new QFileDialog(tabWidget, tr("Open File"), "",
				QObject::tr("Supported file (*.swc)"
					";;Neuron structure	(*.swc)"
	//				";;Linker file (*.ano)"
					));
	fileDlg->setOption(QFileDialog::DontUseNativeDialog, true);
	connect(fileDlg, SIGNAL(fileSelected(const QString &)), this, SLOT(setTree(const QString &)));
	//connect(fileDlg, SIGNAL(fileSelected(const QString &)), this, SLOT(accept()));
	connect(fileDlg, SIGNAL(rejected()), this, SLOT(reject()));

	tabWidget->addTab(fileDlg, tr("Open from File"));
	if (fileList.isEmpty()) tabWidget->setTabEnabled(0, false);

	dlgLayout->addWidget(tabWidget);
	setLayout(dlgLayout);
}

bool OpenSWCDialog::run()
{
	int row = listWidget->currentRow();
	nt = _3DViewerTreeList[row];
	file_name = nt.file;
	
	accept();
	return true;
}

bool OpenSWCDialog::setTree(const QString & file)
{
	nt = readSWC_file(file);
	file_name = file;
	
	accept();
	return true;
}

void OpenSWCDialog::getAllNeuronTreeNames(V3DPluginCallback2 * callback)
{
	QList <V3dR_MainWindow *> windowList = callback->getListAll3DViewers();
	for (V3DLONG i=0;i<windowList.size();i++)
	{
		QList<NeuronTree> * ntTreeList = callback->getHandleNeuronTrees_Any3DViewer(windowList[i]);
		for (V3DLONG j=0;j<ntTreeList->size();j++)
			_3DViewerTreeList.append(ntTreeList->at(j));
	}
	return;
}
