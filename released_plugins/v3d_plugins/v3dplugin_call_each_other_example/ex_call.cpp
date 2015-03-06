/* ex_call.cpp
 * an example program to test plugin-call-plugin function
 * 2010-08-22: by Ruan ZC
 * 2010-08-23: revised by Hanchuan Peng
 * 2011-02-04: by Hanchuan Peng. update the __i386__ flag as it seems the __MAC_x86_64__ does not work
 */


#include "ex_call.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(ex_push, ExCallPlugin);

const QString title = "Example for plugin calling each other";

//#if defined(__APPLE__) //for Mac
#if defined(Q_WS_MAC)
//#if defined(__MAC_x86_64__) //seems not working!! (by PHC, 2011-02-04)
//#if defined(__i386__) //this works but may not be the best way
#if QT_POINTER_SIZE==4
QString plugin_name = "plugins/Vaa3D_PluginInterface_Demos/call_each_other/libex_matrix_debug.dylib";
//#else
#elif QT_POINTER_SIZE==8
QString plugin_name = "plugins/Vaa3D_PluginInterface_Demos/call_each_other/libex_matrix_debug.dylib";
//#elif QT_POINTER_SIZE==16
#endif

#elif defined (_WIN32) || defined (_WIN64)

QString plugin_name = "plugins/Vaa3D_PluginInterface_Demos/call_each_other/libex_matrix_debug.dll";  //for Windows

#else

QString plugin_name = "plugins/Vaa3D_PluginInterface_Demos/call_each_other/libex_matrix.so";  //for Linux

#endif

QString getAppPath()
{
	QString v3dAppPath("~/Work/v3d_external/bin");
	QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
	if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
		testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
	// In a Mac app bundle, plugins directory could be either
	//  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
	//  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
	if (testPluginsDir.dirName() == "MacOS") {
		QDir testUpperPluginsDir = testPluginsDir;
		testUpperPluginsDir.cdUp();
		testUpperPluginsDir.cdUp();
		testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
		if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
	}
#endif
	
    //testPluginsDir.cdUp();
	v3dAppPath = testPluginsDir.absolutePath();
	return v3dAppPath;
}

QStringList ExCallPlugin::menulist() const
{
    return QStringList()
		<< tr("Call another plugin (ex_matrix) to do matrix computation")
		<< tr("About ex_call");
}

void ExCallPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("Call another plugin (ex_matrix) to do matrix computation"))
    {
    	matrixPanel panel(v3d,parent);
    	panel.exec();
    }
	else
	{
		QMessageBox::information(parent, "Version info", 
                QString("Example Call Plugin %1"
				"\nCall another plugin (ex_matrix) to do 3x3 matrix computation"
				"\n\ndeveloped by Zongcai Ruan 2010. (Janelia Farm Research Campus, HHMI)")
                .arg(getPluginVersion()));

	}
}


matrixPanel::matrixPanel(V3DPluginCallback2 &_v3d, QWidget *parent)
	: v3d(_v3d), QDialog(parent)
{
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		a[i][j] = new QDoubleSpinBox();  a[i][j]->setRange(-1000,1000);
		b[i][j] = new QDoubleSpinBox();  b[i][j]->setRange(-1000,1000);
		c[i][j] = new QLabel("_______");
	}

	QGroupBox* a_group = new QGroupBox(); a_group->setTitle("A");
	QGridLayout* a_layout = new QGridLayout(a_group);
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		a_layout->addWidget(a[i][j], i,j);
	}
	QGroupBox* b_group = new QGroupBox(); b_group->setTitle("B");
	QGridLayout* b_layout = new QGridLayout(b_group);
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		b_layout->addWidget(b[i][j], i,j);
	}
	QGroupBox* c_group = new QGroupBox(); c_group->setTitle("C");
	QGridLayout* c_layout = new QGridLayout(c_group);
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		c_layout->addWidget(c[i][j], i,j);
	}

	QGroupBox* btn_group = new QGroupBox();
	QGridLayout* btn_layout = new QGridLayout(btn_group);
	QPushButton* btn_ra     = new QPushButton("random A");
	QPushButton* btn_rb     = new QPushButton("random B");
	QPushButton* btn_add     = new QPushButton("C = A + B");
	QPushButton* btn_mul     = new QPushButton("C = A * B");
	QPushButton* btn_at     = new QPushButton("C = transpose(A)");
	QPushButton* btn_bt     = new QPushButton("C = transpose(B)");
	QPushButton* cancel = new QPushButton("Close");
	btn_layout->addWidget(btn_ra, 1,1);
	btn_layout->addWidget(btn_rb, 1,2);
	btn_layout->addWidget(btn_add, 2,1);
	btn_layout->addWidget(btn_mul, 2,2);
	btn_layout->addWidget(btn_at, 3,1);
	btn_layout->addWidget(btn_bt, 3,2);
	btn_layout->addWidget(cancel, 5,1, 1,2);

	QGridLayout *formLayout = new QGridLayout(this);
	formLayout->addWidget(a_group, 1,1);
	formLayout->addWidget(b_group, 1,2);
	formLayout->addWidget(c_group, 2,1);
	formLayout->addWidget(btn_group, 2,2);
	setLayout(formLayout);
	setWindowTitle(QString("Call another plugin (ex_matrix) to do matrix computation"));

	connect(btn_ra, SIGNAL(clicked()), this, SLOT(rand_a()));
	connect(btn_rb, SIGNAL(clicked()), this, SLOT(rand_b()));
	connect(btn_add, SIGNAL(clicked()), this, SLOT(add()));
	connect(btn_mul, SIGNAL(clicked()), this, SLOT(mul()));
	connect(btn_at, SIGNAL(clicked()), this, SLOT(at()));
	connect(btn_bt, SIGNAL(clicked()), this, SLOT(bt()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
}

void matrixPanel::begin()
{
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		A[i][j] = a[i][j]->value();
		B[i][j] = b[i][j]->value();
		c[i][j]->setText("_______");
	}
}
void matrixPanel::end()
{
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		c[i][j]->setNum(C[i][j]);
	}
}

void matrixPanel::rand_a()
{
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		A[i][j] = (rand()%1000-500)/100.0;
		a[i][j]->setValue(A[i][j]);
	}
}
void matrixPanel::rand_b()
{
	for (int i=0; i<3; i++)
	for (int j=0; j<3; j++)
	{
		B[i][j] = (rand()%1000-500)/100.0;
		b[i][j]->setValue(B[i][j]);
	}
}

void matrixPanel::add()
{
	begin();

	V3DPluginArgItem arg;
	V3DPluginArgList input;
	V3DPluginArgList output;
	arg.type = "double3x3"; arg.p = A;  input << arg;
	arg.type = "double3x3"; arg.p = B;  input << arg;
	arg.type = "double3x3"; arg.p = C;  output << arg;

	QString full_plugin_name = getAppPath() + "/" + plugin_name;
	QString func_name = "add_3x3";
	if (! v3d.callPluginFunc(full_plugin_name, func_name, input, output) )
		return;

	end();
}

void matrixPanel::mul()
{
	begin();

	V3DPluginArgItem arg;
	V3DPluginArgList input;
	V3DPluginArgList output;
	arg.type = "double3x3"; arg.p = A;  input << arg;
	arg.type = "double3x3"; arg.p = B;  input << arg;
	arg.type = "double3x3"; arg.p = C;  output << arg;

	QString full_plugin_name = getAppPath() + "/" + plugin_name;
	QString func_name = "multiply_3x3";
	if (! v3d.callPluginFunc(full_plugin_name, func_name, input, output) )
		return;

	end();
}

void matrixPanel::at()
{
	begin();

	V3DPluginArgItem arg;
	V3DPluginArgList input;
	V3DPluginArgList output;
	arg.type = "double3x3"; arg.p = A;  input << arg;
	arg.type = "double3x3"; arg.p = C;  output << arg;

	QString full_plugin_name = getAppPath() + "/" + plugin_name;
	QString func_name = "transpose_3x3";
	if (! v3d.callPluginFunc(full_plugin_name, func_name, input, output) )
		return;

	end();
}

void matrixPanel::bt()
{
	begin();

	V3DPluginArgItem arg;
	V3DPluginArgList input;
	V3DPluginArgList output;
	arg.type = "double3x3"; arg.p = B;  input << arg;
	arg.type = "double3x3"; arg.p = C;  output << arg;

	QString full_plugin_name = getAppPath() + "/" + plugin_name;
	QString func_name = "transpose_3x3";
	if (! v3d.callPluginFunc(full_plugin_name, func_name, input, output) )
		return;

	end();
}

