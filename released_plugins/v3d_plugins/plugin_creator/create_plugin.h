#ifndef __CREATE_PLUGIN_H__
#define __CREATE_PLUGIN_H__
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string toupper(string instr)
{
	int n = instr.size();
	string new_str = instr;
	for(int i = 0; i < n; i++)
	{
		new_str[i] = (char) ::toupper(instr.at(i));
	}
	return new_str;
}

struct PluginTemplate
{
	string PLUGIN_NAME;
	string PLUGIN_CLASS;
	string PLUGIN_DESCRIPTION;
	string PLUGIN_DATE;
	string PLUGIN_AUTHOR;
	string PLUGIN_GUI;                              // plugin gui file name,     test_gui.h
	string PLUGIN_HEADER;                           // plugin header file name , test_plugin.h
	string PLUGIN_CPP;
	string FUNC_HEADER;
	string FUNC_CPP;
	string PRO_FILE;

	string WINDOW_TITLE;
	string V3D_MAIN_PATH;
	bool DOFUNC;
	vector<string> MENUS;
	vector<string> FUNCS;
	vector<string> MAINFUNCS;
	vector<string> SYSINVOKES;
};


void create_plugin_pro(PluginTemplate & pt)
{
	cout<<"create "<<pt.PRO_FILE<<" ... "<<endl;
	ofstream ofs(pt.PRO_FILE.c_str());
	ofs<<""<<endl;
	ofs<<"TEMPLATE\t= lib"<<endl;
	ofs<<"CONFIG\t+= qt plugin warn_off"<<endl;
	ofs<<"#CONFIG\t+= x86_64"<<endl;
	ofs<<"VAA3DMAINPATH = "<<pt.V3D_MAIN_PATH<<endl;
	ofs<<"INCLUDEPATH\t+= $$VAA3DMAINPATH/basic_c_fun"<<endl;
	ofs<<""<<endl;
	ofs<<"HEADERS\t+= "<<pt.PLUGIN_HEADER<<endl;
	ofs<<"HEADERS\t+= "<<pt.FUNC_HEADER<<endl;
	if(pt.PLUGIN_GUI != "") ofs<<"HEADERS\t+= "<<pt.PLUGIN_GUI<<endl; 
	ofs<<""<<endl;
	ofs<<"SOURCES\t= "<<pt.PLUGIN_CPP<<endl;
	ofs<<"SOURCES\t+= "<<pt.FUNC_CPP<<endl;
	ofs<<"SOURCES\t+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp"<<endl;
	ofs<<""<<endl;
	ofs<<"TARGET\t= $$qtLibraryTarget("<<pt.PLUGIN_NAME<<")"<<endl;
	ofs<<"#DESTDIR\t= ~/Applications/v3d/plugins/"<<pt.PLUGIN_NAME<<"/"<<endl;
	ofs.close();
}

void create_func_cpp(PluginTemplate &pt)
{
	ofstream ofs(pt.FUNC_CPP.c_str());
	ofs<<"/* "<<pt.FUNC_CPP<<endl;
	ofs<<" * "<<pt.PLUGIN_DESCRIPTION<<endl;
	ofs<<" * "<<pt.PLUGIN_DATE<<" : by "<<pt.PLUGIN_AUTHOR<<endl;
	ofs<<" */"<<endl;
	ofs<<""<<endl;
	ofs<<"#include <v3d_interface.h>"<<endl;
	ofs<<"#include \"v3d_message.h\""<<endl;
	ofs<<"#include \""<<pt.FUNC_HEADER<<"\""<<endl;
	if(pt.DOFUNC)
	{
		ofs<<"#include <vector>"<<endl;
		ofs<<"#include <iostream>"<<endl;
		ofs<<"using namespace std;"<<endl;
	}
	if(pt.PLUGIN_GUI != "") ofs<<"#include \""<<pt.PLUGIN_GUI<<"\""<<endl;
	ofs<<""<<endl;
	ofs<<"const QString title = QObject::tr(\""<<pt.WINDOW_TITLE<<"\");"<<endl;
	ofs<<""<<endl;
	if(pt.DOFUNC)
	{
		ofs<<"int split(const char *paras, char ** &args)"<<endl;
		ofs<<"{"<<endl;
		ofs<<"    int argc = 0;"<<endl;
		ofs<<"    int len = strlen(paras);"<<endl;
		ofs<<"    int posb[200];"<<endl;
		ofs<<"    char * myparas = new char[len];"<<endl;
		ofs<<"    strcpy(myparas, paras);"<<endl;
		ofs<<"    for(int i = 0; i < len; i++)"<<endl;
		ofs<<"    {"<<endl;
		ofs<<"        if(i==0 && myparas[i] != ' ' && myparas[i] != '\\t')"<<endl;
		ofs<<"        {"<<endl;
		ofs<<"            posb[argc++]=i;"<<endl;
		ofs<<"        }"<<endl;
		ofs<<"        else if((myparas[i-1] == ' ' || myparas[i-1] == '\\t') &&"<<endl;
		ofs<<"                (myparas[i] != ' ' && myparas[i] != '\\t'))"<<endl;
		ofs<<"        {"<<endl;
		ofs<<"            posb[argc++] = i;"<<endl;
		ofs<<"        }"<<endl;
		ofs<<"    }"<<endl;
		ofs<<""<<endl;
		ofs<<"    args = new char*[argc];"<<endl;
		ofs<<"    for(int i = 0; i < argc; i++)"<<endl;
		ofs<<"    {"<<endl;
		ofs<<"        args[i] = myparas + posb[i];"<<endl;
		ofs<<"    }"<<endl;
		ofs<<""<<endl;
		ofs<<"    for(int i = 0; i < len; i++)"<<endl;
		ofs<<"    {"<<endl;
		ofs<<"        if(myparas[i]==' ' || myparas[i]=='\\t')myparas[i]='\\0';"<<endl;
		ofs<<"    }"<<endl;
		ofs<<"    return argc;"<<endl;
		ofs<<"}"<<endl;
	}
	for(int i = 0; i < pt.FUNCS.size(); i++)
	{
		ofs<<"int "<<pt.FUNCS[i]<<"(V3DPluginCallback2 &callback, QWidget *parent)"<<endl;
		ofs<<"{"<<endl;
		if(i == 0)
		{
			ofs<<"\tv3dhandleList win_list = callback.getImageWindowList();"<<endl;
			ofs<<""<<endl;
			ofs<<"\tif(win_list.size()<1)"<<endl;
			ofs<<"\t{"<<endl;
			ofs<<"\t\tQMessageBox::information(0, title, QObject::tr(\"No image is open.\"));"<<endl;
			ofs<<"\t\treturn -1;"<<endl;
			ofs<<"\t}"<<endl;
			ofs<<"\t//TestDialog dialog(callback, parent);"<<endl;
			ofs<<""<<endl;
			ofs<<"\t//if (dialog.exec()!=QDialog::Accepted) return -1;"<<endl;
			ofs<<""<<endl;
			ofs<<"\t//dialog.update();"<<endl;
			ofs<<"\t//int i = dialog.i;"<<endl;
			ofs<<"\t//int c = dialog.channel;"<<endl;
			ofs<<"\t//Image4DSimple *p4DImage = callback.getImage(win_list[i]);"<<endl;

			ofs<<"\t//if(p4DImage->getCDim() <= c) {v3d_msg(QObject::tr(\"The channel isn't existed.\")); return -1;}"<<endl;
			ofs<<"\t//V3DLONG sz[3];"<<endl;
			ofs<<"\t//sz[0] = p4DImage->getXDim();"<<endl;
			ofs<<"\t//sz[1] = p4DImage->getYDim();"<<endl;
			ofs<<"\t//sz[2] = p4DImage->getZDim();"<<endl;
			ofs<<""<<endl;
			ofs<<"\t//unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);"<<endl;
			ofs<<""<<endl;
			ofs<<"\t//v3dhandle newwin;"<<endl;
			ofs<<"\t//if(QMessageBox::Yes == QMessageBox::question(0, \"\", QString(\"Do you want to use the existing windows?\"), QMessageBox::Yes, QMessageBox::No))"<<endl;
			ofs<<"\t\t//newwin = callback.currentImageWindow();"<<endl;
			ofs<<"\t//else"<<endl;
			ofs<<"\t\t//newwin = callback.newImageWindow();"<<endl;
			ofs<<""<<endl;
			ofs<<"\t//p4DImage->setData(inimg1d, sz[0], sz[1], sz[2], sz[3]);"<<endl;
			ofs<<"\t//callback.setImage(newwin, p4DImage);"<<endl;
			ofs<<"\t//callback.setImageName(newwin, QObject::tr(\""<<pt.FUNCS[i]<<"\"));"<<endl;
			ofs<<"\t//callback.updateImageWindow(newwin);"<<endl;
		}
		else ofs<<"\tv3d_msg(\""<<pt.FUNCS[i]<<"\");"<<endl;

		ofs<<"\treturn 1;"<<endl;
		ofs<<"}"<<endl;
		ofs<<""<<endl;
		if(i <  pt.FUNCS.size() && pt.DOFUNC)
		{
			ofs<<"bool "<<pt.FUNCS[i]<<"(const V3DPluginArgList & input, V3DPluginArgList & output)"<<endl;
			ofs<<"{"<<endl;
			ofs<<"\tcout<<\"Welcome to "<<pt.FUNCS[i]<<"\"<<endl;"<<endl;
			ofs<<"\tif(input.size() != 2 || output.size() != 1) return false;"<<endl;
			ofs<<"\tchar * paras = 0;"<<endl;
			ofs<<"\tif(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\\0';}"<<endl;
			ofs<<"\telse paras = (*(vector<char*> *)(input.at(1).p)).at(0);"<<endl;
			ofs<<"\tcout<<\"paras : \"<<paras<<endl;"<<endl;
			ofs<<""<<endl;
			ofs<<"\tfor(int i = 0; i < strlen(paras); i++)"<<endl;
			ofs<<"\t{"<<endl;
			ofs<<"\t\tif(paras[i] == '#') paras[i] = '-';"<<endl;
			ofs<<"\t}"<<endl;
			ofs<<"\tcout<<\"paras : \"<<paras<<endl;"<<endl;
			ofs<<""<<endl;
			if(i < pt.MAINFUNCS.size() && pt.MAINFUNCS[i] != "")
			{
				ofs<<"\tchar ** argv;"<<endl;
				ofs<<"\tint argc = split(paras, argv);"<<endl;
				ofs<<"\tcout<<\""<<pt.MAINFUNCS[i]<<"(argc, argv)\"<<endl;"<<endl;
				ofs<<"\t//"<<pt.MAINFUNCS[i]<<"(argc, argv);"<<endl;
			}
			if(i < pt.SYSINVOKES.size() && pt.SYSINVOKES[i] != "")
			{
				ofs<<"\tcout<<string(\""<<pt.SYSINVOKES[i]<<"\").append(paras).c_str()<<endl;"<<endl;
				ofs<<"\t//system(string(\""<<pt.SYSINVOKES[i]<<"\").append(paras).c_str());"<<endl;
			}
			ofs<<"\treturn true;"<<endl;
			ofs<<"}"<<endl;
			ofs<<""<<endl;
		}
	}
	ofs<<""<<endl;
	ofs.close();

}

void create_func_header(PluginTemplate &pt)
{
	cout<<"create "<<pt.FUNC_HEADER<<" ... "<<endl;
	ofstream ofs(pt.FUNC_HEADER.c_str());
	ofs<<"/* "<<pt.FUNC_HEADER<<endl;
	ofs<<" * "<<pt.PLUGIN_DESCRIPTION<<endl;
	ofs<<" * "<<pt.PLUGIN_DATE<<" : by "<<pt.PLUGIN_AUTHOR<<endl;
	ofs<<" */"<<endl;
	ofs<<" "<<endl;
	ofs<<"#ifndef __"<<toupper(pt.PLUGIN_NAME)<<"_FUNC_H__"<<endl;
	ofs<<"#define __"<<toupper(pt.PLUGIN_NAME)<<"_FUNC_H__"<<endl;
	ofs<<""<<endl;
	ofs<<"#include <v3d_interface.h>"<<endl;
	ofs<<""<<endl;
	for(int i = 0; i < pt.FUNCS.size(); i++)
	{
		ofs<<"int "<<pt.FUNCS[i]<<"(V3DPluginCallback2 &callback, QWidget *parent);"<<endl;
		if(pt.DOFUNC) ofs<<"bool "<<pt.FUNCS[i]<<"(const V3DPluginArgList & input, V3DPluginArgList & output);"<<endl;
	}
	ofs<<""<<endl;
	ofs<<"#endif"<<endl;
	ofs<<""<<endl;
	ofs.close();
}

void create_plugin_cpp(PluginTemplate & pt)
{
	cout<<"create "<<pt.PLUGIN_CPP<<" ... "<<endl;
	ofstream ofs(pt.PLUGIN_CPP.c_str());
	ofs<<"/* "<<pt.PLUGIN_CPP<<endl;
	ofs<<" * "<<pt.PLUGIN_DESCRIPTION<<endl;
	ofs<<" * "<<pt.PLUGIN_DATE<<" : by "<<pt.PLUGIN_AUTHOR<<endl;
	ofs<<" */"<<endl;
	ofs<<" "<<endl;
	ofs<<"#include \"v3d_message.h\""<<endl;
	ofs<<""<<endl;
	ofs<<"#include \""<<pt.PLUGIN_HEADER<<"\""<<endl;
	ofs<<"#include \""<<pt.FUNC_HEADER<<"\""<<endl;
	ofs<<" "<<endl;
	ofs<<"Q_EXPORT_PLUGIN2("<<pt.PLUGIN_NAME<<", "<<pt.PLUGIN_CLASS<<");"<<endl;
	ofs<<" "<<endl;
	ofs<<"QStringList "<<pt.PLUGIN_CLASS<<"::menulist() const"<<endl;
	ofs<<"{"<<endl;
	ofs<<"\treturn QStringList() "<<endl;
	for(int i = 0; i < pt.MENUS.size(); i++) ofs<<"\t\t<<tr(\""<<pt.MENUS[i]<<"\")"<<endl;
	ofs<<"\t\t<<tr(\"about\");"<<endl;
	ofs<<"}"<<endl;
	ofs<<""<<endl;
	if(pt.DOFUNC)
	{
		ofs<<"QStringList "<<pt.PLUGIN_CLASS<<"::funclist() const"<<endl;
		ofs<<"{"<<endl;
		ofs<<"\treturn QStringList()";
		for(int i = 0; i < pt.FUNCS.size(); i++) ofs<<endl<<"\t\t<<tr(\""<<pt.FUNCS[i]<<"\")";
		ofs<<";"<<endl;
		ofs<<"}"<<endl;
		ofs<<""<<endl;
	}
	ofs<<"void "<<pt.PLUGIN_CLASS<<"::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)"<<endl;
	ofs<<"{"<<endl;
	ofs<<"\tif (menu_name == tr(\""<<pt.MENUS[0]<<"\"))"<<endl;
	ofs<<"\t{"<<endl;
	ofs<<"\t\t"<<pt.FUNCS[0]<<"(callback,parent);"<<endl;
	ofs<<"\t}"<<endl;
	for(int i = 1; i < pt.MENUS.size(); i++)
	{
		ofs<<"\telse if (menu_name == tr(\""<<pt.MENUS[i]<<"\"))"<<endl;
		ofs<<"\t{"<<endl;
		ofs<<"\t\t"<<pt.FUNCS[i]<<"(callback,parent);"<<endl;
		ofs<<"\t}"<<endl;
	}
	ofs<<"\telse"<<endl;
	ofs<<"\t{"<<endl;
	ofs<<"\t\tv3d_msg(tr(\""<<pt.PLUGIN_DESCRIPTION<<". \"\n\t\t\t\"Developed by "<<pt.PLUGIN_AUTHOR<<", "<<pt.PLUGIN_DATE<<"\"));"<<endl;
	ofs<<"\t}"<<endl;
	ofs<<"}"<<endl;
	ofs<<""<<endl;
	if(pt.DOFUNC)
	{
		ofs<<"bool "<<pt.PLUGIN_CLASS<<"::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)"<<endl;
		ofs<<"{"<<endl;
		ofs<<"\tif (func_name == tr(\""<<pt.FUNCS[0]<<"\"))"<<endl;
		ofs<<"\t{"<<endl;
		ofs<<"\t\treturn "<<pt.FUNCS[0]<<"(input, output);"<<endl;
		ofs<<"\t}"<<endl;
		for(int i = 1; i < pt.FUNCS.size(); i++)
		{
			ofs<<"\telse if (func_name == tr(\""<<pt.FUNCS[i]<<"\"))"<<endl;
			ofs<<"\t{"<<endl;
			ofs<<"\t\treturn "<<pt.FUNCS[i]<<"(input,output);"<<endl;
			ofs<<"\t}"<<endl;
		}
		ofs<<"}"<<endl;
		ofs<<""<<endl;
	}
	ofs.close();
}

void create_plugin_header(PluginTemplate & pt)  // PLUGIN_HEADER
{
	cout<<"create "<<pt.PLUGIN_HEADER<<" ... "<<endl;
	ofstream ofs(pt.PLUGIN_HEADER.c_str());
	ofs<<"/* "<<pt.PLUGIN_HEADER<<endl;
	ofs<<" * "<<pt.PLUGIN_DESCRIPTION<<endl;
	ofs<<" * "<<pt.PLUGIN_DATE<<" : by "<<pt.PLUGIN_AUTHOR<<endl;
	ofs<<" */"<<endl;
	ofs<<" "<<endl;
	ofs<<"#ifndef __"<<toupper(pt.PLUGIN_NAME)<<"_PLUGIN_H__"<<endl;
	ofs<<"#define __"<<toupper(pt.PLUGIN_NAME)<<"_PLUGIN_H__"<<endl;
	ofs<<""<<endl;
	ofs<<"#include <QtGui>"<<endl;
	ofs<<"#include <v3d_interface.h>"<<endl;
	ofs<<""<<endl;
	ofs<<"class "<<pt.PLUGIN_CLASS<<" : public QObject, public V3DPluginInterface2_1"<<endl;
	ofs<<"{"<<endl;
	ofs<<"\tQ_OBJECT"<<endl;
	ofs<<"\tQ_INTERFACES(V3DPluginInterface2_1);"<<endl;
	ofs<<""<<endl;
	ofs<<"public:"<<endl;
	ofs<<"\tfloat getPluginVersion() const {return 1.1f;}"<<endl;
	ofs<<""<<endl;
	ofs<<"\tQStringList menulist() const;"<<endl;
	ofs<<"\tvoid domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);"<<endl;
	ofs<<""<<endl;
	if(!pt.DOFUNC)
	{
		ofs<<"\tQStringList funclist() const {return QStringList();}"<<endl;
		ofs<<"\tbool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)"<<endl;
		ofs<<"\t{return false;}"<<endl;
	}
	else
	{
		ofs<<"\tQStringList funclist() const ;"<<endl; 
		ofs<<"\tbool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);"<<endl;
	}
	ofs<<"};"<<endl;
	ofs<<""<<endl;
	ofs<<"#endif"<<endl;
	ofs<<""<<endl;
	ofs.close();
}

string demo_plugin_template()
{
	ostringstream oss;
	oss<<"PLUGIN_NAME = \"test\""<<endl;
	oss<<"PLUGIN_CLASS = \"TestPlugin\""<<endl;
	oss<<"WINDOW_TITLE = \"Test Plugin\""<<endl;
	oss<<"PLUGIN_DESCRIPTION = \"This is a test plugin\""<<endl;
	oss<<"PLUGIN_DATE = \"2011-06-16\""<<endl;
	oss<<"PLUGIN_AUTHOR = \"Your name\""<<endl;
	oss<<"#PLUGIN_GUI = \"test_gui.h\""<<endl;
	oss<<"VAA3D_MAIN_PATH = \"../../work/v3d_external/v3d_main\""<<endl;
	oss<<endl;
	oss<<"MENUS=(\"first item\" \"second item\")"<<endl;
	oss<<"FUNCS=(\"first_item\" \"second_item\")"<<endl;
	oss<<endl;
	oss<<"#DOFUNC = \"yes\""<<endl;
	oss<<endl;
	oss<<"#MAINFUNCS=(\"mymain\" \"\")"<<endl;
	oss<<"#SYSINVOKES=(\"\" \"mycmd\")"<<endl;
	return oss.str();
}

string demo_gui_template()
{
	ostringstream oss;
	oss<<"CLASS QDialog TestDialog"<<endl;
	oss<<""<<endl;
	oss<<"D QLabel label_subject \"Subject Image :\""<<endl;
	oss<<"D QComboBox combo_subject | addItems(items)"<<endl;
	oss<<""<<endl;
	oss<<"D QLabel label_target \"Target Image :\""<<endl;
	oss<<"D QComboBox combo_target | addItems(items)"<<endl;
	oss<<""<<endl;
	oss<<"D QLabel label_sub_channel \"Subject Channel :\""<<endl;
	oss<<"D QLabel label_tar_channel \"Target Channel :\""<<endl;
	oss<<""<<endl;
	oss<<"D QSpinBox channel_sub | setMaximum(3) setValue(0)"<<endl;
	oss<<"D QSpinBox channel_tar | setMaximum(3) setValue(0)"<<endl;
	oss<<""<<endl;
	oss<<"D QPushButton ok \"ok\""<<endl;
	oss<<"D QPushButton cancel \"cancel\""<<endl;
	oss<<""<<endl;
	oss<<"D QGridLayout gridLayout"<<endl;
	oss<<"L 0 0 label_subject"<<endl;
	oss<<"L 0 1 combo_subject 1 5"<<endl;
	oss<<"L 1 0 label_sub_channel"<<endl;
	oss<<"L 1 1 channel_sub 1 1"<<endl;
	oss<<"L 2 0 label_target"<<endl;
	oss<<"L 2 1 combo_target 1 5"<<endl;
	oss<<"L 3 0 label_tar_channel"<<endl;
	oss<<"L 3 1 channel_tar"<<endl;
	oss<<"L 5 4 cancel Qt::AlignRight"<<endl;
	oss<<"L 5 5 ok     Qt::AlignRight"<<endl;
	oss<<""<<endl;
	oss<<"C ok clicked() this accept()"<<endl;
	oss<<"C cancel clicked() this reject()"<<endl;
	oss<<""<<endl;
	oss<<"C combo_subject currentIndexChanged(int) this update()"<<endl;
	oss<<"C combo_target currentIndexChanged(int) this update()"<<endl;
	oss<<""<<endl;
	oss<<"C channel_sub valueChanged(int) this update()"<<endl;
	oss<<"C channel_tar valueChanged(int) this update()"<<endl;
	oss<<""<<endl;
	oss<<"R this | setLayout(gridLayout)"<<endl;
	oss<<"R this | setWindowTitle(\"Test Widget\")"<<endl;
	oss<<""<<endl;
	oss<<"U int i1 | combo_subject->currentIndex()"<<endl;
	oss<<"U int i2 | combo_target->currentIndex()"<<endl;
	oss<<"U int c1 | channel_sub->text().toInt()"<<endl;
	oss<<"U int c2 | channel_tar->text().toInt()"<<endl;
	oss<<"================================================="<<endl;
	oss<<"CLASS QWidget TestWidget"<<endl;
	oss<<""<<endl;
	oss<<"D QLabel label_subject \"Subject Image :\""<<endl;
	oss<<"D QComboBox combo_subject "<<endl;
	oss<<""<<endl;
	oss<<"D QLabel label_target \"Target Image :\""<<endl;
	oss<<"D QComboBox combo_target "<<endl;
	oss<<""<<endl;
	oss<<"D QLabel label_sub_channel \"Subject Channel :\""<<endl;
	oss<<"D QLabel label_tar_channel \"Target Channel :\""<<endl;
	oss<<""<<endl;
	oss<<"D QSpinBox channel_sub"<<endl;
	oss<<"D QSpinBox channel_tar"<<endl;
	oss<<""<<endl;
	oss<<"D QPushButton ok \"ok\""<<endl;
	oss<<"D QPushButton cancel \"cancel\""<<endl;
	oss<<""<<endl;
	oss<<"D QVBoxLayout gridLayout"<<endl;
	oss<<""<<endl;
	oss<<"L 0 0 label_subject"<<endl;
	oss<<"L 0 1 combo_subject 1 5"<<endl;
	oss<<"L 1 0 label_sub_channel"<<endl;
	oss<<"L 1 1 channel_sub 1 1"<<endl;
	oss<<"L 2 0 label_target"<<endl;
	oss<<"L 2 1 combo_target 1 5"<<endl;
	oss<<"L 3 0 label_tar_channel"<<endl;
	oss<<"L 3 1 channel_tar"<<endl;
	oss<<"L 5 4 cancel Qt::AlignRight"<<endl;
	oss<<"L 5 5 ok     Qt::AlignRight"<<endl;
	oss<<""<<endl;
	oss<<"C ok clicked() this accept()"<<endl;
	oss<<"C cancel clicked() this reject()"<<endl;
	oss<<""<<endl;
	oss<<"C combo_subject currentIndexChanged(int) this onSubjectChanged()"<<endl;
	oss<<"C combo_target currentIndexChanged(int) this onTargetChanged()"<<endl;
	oss<<""<<endl;
	oss<<"C channel_sub valueChanged(int) this onChannelChanged()"<<endl;
	oss<<"C channel_tar valueChanged(int) this onChannelChanged()"<<endl;
	return oss.str();
}

void create_plugin_all(PluginTemplate & pt)
{
	create_plugin_header(pt);
	create_plugin_cpp(pt);
	create_func_header(pt);
	create_func_cpp(pt);
	create_plugin_pro(pt);
}

bool get_next_string(string &val, istream &is)
{
	int c = is.get(); 
	while(c == ' ' || c == '\t') c = is.get();
	if(c == '"') {getline(is, val, '"'); is.ignore();} 
	else if(c == ')' || c == '\n') return false;
	else is >> val;
	return true;
}

PluginTemplate read_plugin_template(string plugin_tmpl_file)
{
	PluginTemplate pt;
	ifstream ifs(plugin_tmpl_file.c_str());
	string str, name, value;
	vector<string> values;
	while(ifs.good())
	{
		getline(ifs, str, '\n');
		int pos = str.find('#'); if(pos != string::npos) str = str.substr(0, pos);
		pos = str.find('='); if(pos == string::npos) continue;
		name = str.substr(0, pos); value = str.substr(pos+1, str.size() - pos - 1);
		cout<<name<<" = ";
		if(value[0] == '"') 
		{
			value = value.substr(1, value.size() - 2);
			cout<<"\""<<value<<"\""<<endl;
		}
		else if(value[0] == '(') 
		{
			values.clear();
			value = value.substr(0, value.size() - 1);
			int pos1 = value.find('"');
			cout<<"(";
			while(pos1 != string::npos)
			{
				int pos2 = value.find('"',pos1+1); 
				str = value.substr(pos1+1, pos2 - pos1 - 1);
				values.push_back(str);
				pos1 = value.find('"', pos2+1);
				cout<<"\""<<str<<"\" "; cout.flush();
			}
			cout<<")"<<endl;
		}
		pt.PLUGIN_NAME = (name == "PLUGIN_NAME") ? value : pt.PLUGIN_NAME;
		pt.PLUGIN_CLASS = (name == "PLUGIN_CLASS") ? value : pt.PLUGIN_CLASS;
		pt.PLUGIN_DESCRIPTION = (name == "PLUGIN_DESCRIPTION") ? value : pt.PLUGIN_DESCRIPTION;
		pt.PLUGIN_DATE = (name == "PLUGIN_DATE") ? value : pt.PLUGIN_DATE;
		pt.PLUGIN_AUTHOR = (name == "PLUGIN_AUTHOR") ? value : pt.PLUGIN_AUTHOR;
		pt.PLUGIN_GUI = (name == "PLUGIN_GUI") ? value : pt.PLUGIN_GUI;
		pt.WINDOW_TITLE = (name == "WINDOW_TITLE") ? value : pt.WINDOW_TITLE;
		pt.V3D_MAIN_PATH = (name == "VAA3D_MAIN_PATH") ? value : pt.V3D_MAIN_PATH;
		pt.MENUS = (name == "MENUS") ? values : pt.MENUS;
		pt.FUNCS = (name == "FUNCS") ? values : pt.FUNCS;
		pt.DOFUNC = (name == "DOFUNC") ? (value=="yes") : pt.DOFUNC;
	}
	ifs.close();

	pt.PLUGIN_HEADER = pt.PLUGIN_NAME + "_plugin.h";
	pt.PLUGIN_CPP = pt.PLUGIN_NAME + "_plugin.cpp";
	pt.FUNC_HEADER = pt.PLUGIN_NAME + "_func.h";
	pt.FUNC_CPP = pt.PLUGIN_NAME + "_func.cpp";
	pt.PRO_FILE = pt.PLUGIN_NAME + ".pro";

	cout<<endl;
	return pt;
}

#endif
