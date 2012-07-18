#include "toolbox_gui.h"
#include <iostream>
#include "customary_structs/vaa3d_neurontoolbox_para.h"
using namespace std;

static QString toolboxRootPath = QObject::tr("/Users/xiaoh10/Applications/v3d/plugins");
bool setPluginRootPathAutomaticly()
{
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
		testPluginsDir = testUpperPluginsDir;
	//	if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;

	}
#endif
	if (testPluginsDir.cd("plugins")==false)
		return false;
	if (testPluginsDir.cd("neuron_utilities")==false)
		return false;
	toolboxRootPath=testPluginsDir.absolutePath();

	return true;
}

void getAllFiles(QString dirname, QStringList & fileList)
{
	QDir dir(dirname);
	QStringList dirlist = dir.entryList(QDir::Dirs);
	if(dirlist.size() == 2) 
	{
		QStringList files = dir.entryList(QDir::Files);
		QStringList::iterator fit = files.begin();
		while(fit != files.end())
		{
			fileList.append(dir.absoluteFilePath(*fit));
			fit++;
		}
		return;
	}

	for(QStringList::iterator it = dirlist.begin(); it != dirlist.end(); it++)
	{
		if(((*it) == ".") || ((*it) == "..")) continue;
		getAllFiles(dir.absoluteFilePath(*it), fileList);
	}
}

static QStringList v3d_getInterfaceMenuList(QObject *plugin)
{
	QStringList qslist;

	V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
	if (iFilter2_1 )  return (qslist = iFilter2_1->menulist());

	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
	if (iFilter )  return (qslist = iFilter->menulist());

	V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
	if (iface )  return (qslist = iface->menulist());

	V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
	if (iface2 )  return (qslist = iface2->menulist());

	V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	if (iface2_1 )  return (qslist = iface2_1->menulist());

	return qslist;
}

SelectPluginDlg::SelectPluginDlg(QWidget * parent, const V3DPluginCallback2 & _callback)
	: QDialog(parent)
{
	setMinimumWidth(500);
	parent = parent;
	callback = (V3DPluginCallback2 *) (&(_callback));
	
	QVBoxLayout * layout = new QVBoxLayout;
	
	//tree widget GUI
	pluginTreeWidget = new QTreeWidget();
	pluginTreeWidget->setColumnCount(1);
	pluginTreeWidget->header()->hide();
	pluginTreeWidget->setSortingEnabled(false);

	if (!setPluginRootPathAutomaticly())
		v3d_msg("You don't have any plugins on neuron utilities");
	QStringList fileList;
	getAllFiles(toolboxRootPath, fileList);
	root_path = toolboxRootPath;

	for (int i=0;i<fileList.size();i++)
	{
		QString file = fileList[i];
		QPluginLoader* loader = new QPluginLoader(file);
		if(!loader) 
		{
			cerr<<"unable to load plugin: "<<qPrintable(file)<<endl;
			continue;
		}

		QObject * plugin = loader->instance();

		if (plugin)
		{
			//lib - top level item
			QTreeWidgetItem *pluginItem = new QTreeWidgetItem(pluginTreeWidget);
			QString tmp = file.remove(0, root_path.size()+1);
			tmp.chop(file.section("/", -1).size()+1);
			pluginItem->setText(0, tmp);
			pluginTreeWidget->addTopLevelItem(pluginItem);
			name_table.insert(pluginItem,fileList[i]);

			QStringList menulist = v3d_getInterfaceMenuList(plugin);
			foreach(QString menu_name, menulist)
			{
				//menu - second level item
				QTreeWidgetItem * menuItem = new QTreeWidgetItem(pluginItem);
				menuItem->setText(0, menu_name);
			}
			
		}
		loader->unload();
		delete loader;
	}
	layout->addWidget(pluginTreeWidget);

	//run button
	QPushButton * button = new QPushButton("run");
	connect(button, SIGNAL(clicked()), this, SLOT(runMenu()));
	connect(pluginTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(runMenu()));

	layout->addWidget(button);

	setLayout(layout);
}

SelectPluginDlg::SelectPluginDlg(QWidget * parent, const V3DPluginCallback2 & _callback, const V3DPluginArgList & _input)
	: QDialog(parent)
{
	setMinimumWidth(500);
	input = new V3DPluginArgList;
	*input = _input;
	const char* test_str4 = ((vaa3d_neurontoolbox_paras *)(input->at(0).p))->nt.file.toStdString().c_str();

	parent = parent;
	callback = (V3DPluginCallback2 *) (&(_callback));
	
	QVBoxLayout * layout = new QVBoxLayout;
	
	//tree widget GUI
	pluginTreeWidget = new QTreeWidget();
	pluginTreeWidget->setColumnCount(1);
	pluginTreeWidget->header()->hide();
	pluginTreeWidget->setSortingEnabled(true);

	if (!setPluginRootPathAutomaticly())
		v3d_msg("You don't have any plugins on neuron utilities");
	QStringList fileList;
	getAllFiles(toolboxRootPath, fileList);
	root_path = toolboxRootPath;

	for(int i=0;i<fileList.size();i++)
	{
		QString file = fileList[i];
		QPluginLoader* loader = new QPluginLoader(file);
		if(!loader) 
		{
			cerr<<"unable to load plugin: "<<qPrintable(file)<<endl;
			continue;
		}

		//pluginLoaderList.push_back(loader);

		QObject * plugin = loader->instance();

		if (plugin)
		{
			//lib - top level item
			QTreeWidgetItem *pluginItem = new QTreeWidgetItem(pluginTreeWidget);
			QString tmp = file.remove(0, root_path.size()+1);
			tmp.chop(file.section("/", -1).size()+1);
			pluginItem->setText(0, tmp);
			pluginTreeWidget->addTopLevelItem(pluginItem);
			name_table.insert(pluginItem, fileList[i]);

			QStringList menulist = v3d_getInterfaceMenuList(plugin);
			foreach(QString menu_name, menulist)
			{
				//menu - second level item
				QTreeWidgetItem * menuItem = new QTreeWidgetItem(pluginItem);
				menuItem->setText(0, menu_name);
			}
		}
		loader->unload();
		delete loader;
	}
	layout->addWidget(pluginTreeWidget);

	//run button
	QPushButton * button = new QPushButton("run");
	connect(button, SIGNAL(clicked()), this, SLOT(runFunc()));
	connect(pluginTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(runFunc()));

	layout->addWidget(button);

	setLayout(layout);

	connect(this, SIGNAL(rejected()), this, SLOT(freeMem()));
}

bool SelectPluginDlg::runMenu()
{
	QTreeWidgetItem * menuItem = pluginTreeWidget->currentItem();
	if (!menuItem)
	{
		v3d_msg("Please select an item to run");
		return false;
	}
	QString menu_name = menuItem->text(0);
	QTreeWidgetItem * pluginItem = menuItem->parent();
	if (!pluginItem)
	{
		v3d_msg("Please select a menu name");
		return false;
	}
	//QString plugin_name = root_path + "/" + pluginItem->text(0);
	QString plugin_name = name_table[pluginItem];

	QPluginLoader* loader = new QPluginLoader(plugin_name);
	cout<<"plugin_file = "<<plugin_name.toStdString()<<endl;
	if(!loader)
	{
		v3d_msg("Unable to load this plugin!");
		return true;
	}

	loader->unload();
	QObject * plugin = loader->instance();
	V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
	if (iFilter2_1 )
	{
		QMessageBox::information(0,"","This is V3DSingleImageInterface2_1 plugin , only V3DPluginInterface2 and V3DPluginInterface2_1 is supported!");
		return false;
	}
	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
	if (iFilter )
	{
		QMessageBox::information(0,"","This is V3DSingleImageInterface plugin , only V3DPluginInterface2 and V3DPluginInterface2_1 is supported!");
		return false;
	}
	V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	if (iface2_1 )
	{
		iface2_1->domenu(menu_name, *callback, parent);
	}
	V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
	if (iface2 )
	{
		iface2->domenu(menu_name, *callback,parent);
	}
	V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
	if (iface )
	{
		iface->domenu(menu_name, *callback, parent);
	}
	//while(loader->isLoaded()) 
	loader->unload();
	delete loader;

	return true;
}

bool SelectPluginDlg::runFunc()
{
	V3DPluginArgList output;
	QTreeWidgetItem * menuItem = pluginTreeWidget->currentItem();
	if (!menuItem)
	{
		v3d_msg("Please select an item to run");
		return false;
	}
	QString menu_name = menuItem->text(0);
	QTreeWidgetItem * pluginItem = menuItem->parent();
	if (!pluginItem)
	{
		v3d_msg("Please select a menu name");
		return false;
	}
	//QString plugin_name = root_path + "/" + pluginItem->text(0);
	QString plugin_name = name_table[pluginItem];

	if (!callback->callPluginFunc(plugin_name, "TOOLBOX" + menu_name, *input, output))
	{
		runMenu();
	}
	
	return true;
}

bool SelectPluginDlg::freeMem()
{
	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *)(input->at(0).p);
	if (paras) {delete paras; paras=NULL;}
	
	accept();
}

