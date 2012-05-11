/* mapview_plugin.cpp
 * Used to load large data
 * 2012-03-16 : by Hang Xiao & Jianlong Zhou
 */
#include <iostream> 
#include <fstream>

#include "v3d_message.h"
#include "mapview_plugin.h"
#include "mapview_gui.h"
 
using namespace std;

Q_EXPORT_PLUGIN2(mapview, MapViewPlugin);

int load_hraw_data(V3DPluginCallback2 &callback, QWidget *parent);
int create_hraw_data(V3DPluginCallback2 &callback, QWidget *parent);
int help(V3DPluginCallback2 &callback, QWidget *parent);

// dirname("/dir/name/test.tif") == "/dir/name"
string dirname(string para)
{
	int pos = para.find_last_of("/");
	if(pos == string::npos) return ".";
	else if(pos == 0) return "/";
	else return para.substr(0, pos);
}
 
QStringList MapViewPlugin::menulist() const
{
	return QStringList() 
		<<tr("load hraw data")
		<<tr("create hraw data")
		<<tr("about");
}

QStringList MapViewPlugin::funclist() const
{
	return QStringList()
		<<tr("load_hraw_data")
		<<tr("create_hraw_data")
		<<tr("help");
}

void MapViewPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("load hraw data"))
	{
		load_hraw_data(callback,parent);
	}
	else if(menu_name == tr("create hraw data"))
	{
		create_hraw_data(callback, parent);
		v3d_msg("Not implemented yet.", 0);
	}
	else
	{
		v3d_msg(tr("Used to load large data. "
			"Developed by Hang Xiao, 2012-03-16"));
	}
}

bool MapViewPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("load_hraw_data"))
	{
		v3d_msg("Not implemented yet.", 0);
		//return load_hraw_data(input, output);
	}
	else if(func_name == tr("create_hraw_data"))
	{
		v3d_msg("Not implemented yet.", 0);
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("Not implemented yet.", 0);
		//return help(input,output);
	}
	return false;
}

static string toupper_case(string instr)
{
	string outstr = instr;
	for(int i = 0; i < instr.size(); i++)
	{
		char c = instr[i];
		if(c >= 'a' && c <= 'z')
		{
			c = c - 'a' + 'A';
		}
		outstr[i] = c;
	}
	return outstr;
}

int load_hraw_data(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString hraw_file = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                 ".",
                                                 QObject::tr("Images (*.hraw)"));
	cout<<"hraw_file = "<<hraw_file.toStdString().c_str()<<endl;
	if(hraw_file == QObject::tr("")) return 0;
	ifstream ifs(hraw_file.toStdString().c_str());

	string dir;
	V3DLONG L = 0, M = 0, N = 0;
	V3DLONG l = 0, m = 0, n = 0;
	V3DLONG channel = 0;
	int level_num = 0;
	while(ifs.good())
	{
		string name;
		ifs >> name; name = toupper_case(name);
		if(name == "L0_X_BLOCKS") ifs >> L;
		else if(name == "L0_Y_BLOCKS") ifs >> M;
		else if(name == "L0_Z_BLOCKS") ifs >> N;
		else if(name == "LEVEL_NUM") ifs >> level_num;
	}
	dir = QDir(dirname(hraw_file.toStdString()).c_str()).absolutePath().toStdString();
	cout<<"dir = "<<dir<<endl;
	string raw0000 = dir + "/L0/L0_X0_Y0_Z0.raw";
	cout<<"raw0000 = "<<raw0000<<endl;
	getRawImageSize(raw0000, l, m, n, channel);

	cout<<"dir = "<<dir<<endl;
	cout<<"L = "<<L<<" M = "<<M<<" N = "<<N<<endl;
	cout<<"l = "<<l<<" m = "<<m<<" n = "<<n<<endl;
	cout<<"channel = "<<channel<<endl;
	cout<<"level_num = "<<level_num<<endl;

	Mapview_Paras mapview_paras;
	mapview_paras.hraw_dir = dir.c_str();
	mapview_paras.L = L;
	mapview_paras.M = M;
	mapview_paras.N = N;
	mapview_paras.l = l;
	mapview_paras.m = m;
	mapview_paras.n = n;
	mapview_paras.channel = channel;
	int init_level = log(pow((L*l*M*m*N*n)/(256.0*256.0*64), 1.0/3.0))/log(2.0) + 0.4999;
	init_level = MAX(init_level, 0);
	mapview_paras.level_num = init_level + 1;
	cout<<"init_level = "<<init_level<<endl;
	mapview_paras.level = init_level; // the initial level value, the thumbnail of the whole image

	MapViewWidget * mapview_widget = new MapViewWidget(&callback, mapview_paras, 0);
	mapview_widget->show();
	
	return 1;
}

int create_hraw_data(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString filename = QFileDialog::getOpenFileName(0, QObject::tr("Open Image File"),
                                                 ".",
                                                 QObject::tr("Image Files (*.raw *.tiff *.tif *.lsm)"));
	if(filename == QObject::tr("")) return 0;
	// Dialog
	V3DLONG bs0 = 256;
	V3DLONG bs1 = 256;
	V3DLONG bs2 = 128;
	QString qhraw_file = filename + ".hraw";
	{
		QDialog * dialog = new QDialog();
		QLineEdit * bs0_editor = new QLineEdit(QObject::tr("256"));
		QLineEdit * bs1_editor = new QLineEdit(QObject::tr("256"));
		QLineEdit * bs2_editor = new QLineEdit(QObject::tr("128"));
		QLineEdit * hraw_editor = new QLineEdit(qhraw_file);
		QPushButton * ok = new QPushButton(QObject::tr("  ok  "));
		QPushButton * cancel = new QPushButton(QObject::tr("cancel"));

		QGridLayout * layout = new QGridLayout();
		layout->addWidget(new QLabel("bs0"), 0, 0, 1, 1);
		layout->addWidget(bs0_editor, 0, 1, 1, 5);
		layout->addWidget(new QLabel("bs1"), 1, 0, 1, 1);
		layout->addWidget(bs1_editor, 1, 1, 1, 5);
		layout->addWidget(new QLabel("bs2"), 2, 0, 1, 1);
		layout->addWidget(bs2_editor, 2, 1, 1, 5);
		layout->addWidget(new QLabel("hraw file"), 3, 0, 1, 1);
		layout->addWidget(hraw_editor, 3, 1, 1, 5);
		layout->addWidget(ok, 4, 0, 1, 3); 
		layout->addWidget(cancel, 4, 3, 1, 3); 
		dialog->setLayout(layout);
		dialog->setWindowTitle(QObject::tr("Set block size"));
		QObject::connect(ok, SIGNAL(clicked(bool)), dialog, SLOT(accept()));
		QObject::connect(cancel, SIGNAL(clicked(bool)), dialog, SLOT(reject()));
		if(dialog->exec() != QDialog::Accepted) return 0;
		bs0 = atoi(bs0_editor->text().toStdString().c_str());
		bs1 = atoi(bs1_editor->text().toStdString().c_str());
		bs2 = atoi(bs2_editor->text().toStdString().c_str());
		qhraw_file = hraw_editor->text();
		QMessageBox::information(0,QObject::tr(""), QObject::tr("bs0 = %1, bs1 = %2, bs2 = %3, hraw_file = %4").arg(bs0).arg(bs1).arg(bs2).arg(qhraw_file));
	}

	string infile = filename.toStdString();
	V3DLONG insz0 = 0, insz1 = 0, insz2 = 0, channel = 0;
	getRawImageSize(infile, insz0, insz1, insz2, channel);

	V3DLONG ts0 = (insz0 % bs0 == 0) ? insz0/bs0 : insz0/bs0 + 1;
	V3DLONG ts1 = (insz1 % bs1 == 0) ? insz1/bs1 : insz1/bs1 + 1;
	V3DLONG ts2 = (insz2 % bs2 == 0) ? insz2/bs2 : insz2/bs2 + 1;

	V3DLONG level_num = log(MIN(MIN(insz0, insz1), insz2))/log(2.0) - 1;

	string hraw_file = qhraw_file.toStdString(); 
	string dir = dirname(infile);
	
	raw_split((char*)(infile.c_str()), (char*)dir.c_str(), bs0, bs1, bs2);
	createMapViewFiles((char*)dir.c_str(), ts0, ts1, ts2);

	ofstream ofs(hraw_file.c_str());
	if(ofs.fail()){cerr<<"Unable to open "<<hraw_file<<endl; return false;}
	
	ofs<<"L0_X_BLOCKS "<<ts0<<endl;
	ofs<<"L0_Y_BLOCKS "<<ts1<<endl;
	ofs<<"L0_Z_BLOCKS "<<ts2<<endl;
	ofs<<"CHANNEL "<<channel<<endl;
	ofs<<"LEVEL_NUM "<<level_num<<endl;
	ofs.close();

}

int help(V3DPluginCallback2 &callback, QWidget *parent)
{
	return 1;
}
