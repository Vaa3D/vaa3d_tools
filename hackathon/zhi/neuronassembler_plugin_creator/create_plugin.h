//last changed by Hanchuan Peng, 2013-08-06. fix a return value bug and also add some v3d_msg calls to indicate where the code should be added

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
	string VAA3D_PATH;
	bool DOFUNC;
    vector<string> PARA_NAME;
    vector<string> PARA_TYPE;
    vector<string> PARA_VALUE;

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
	ofs<<"VAA3DPATH = "<<pt.VAA3D_PATH<<endl;
    ofs<<"INCLUDEPATH\t+= $$VAA3DPATH/basic_c_fun"<<endl;
    ofs<<"INCLUDEPATH\t+= $$VAA3DPATH/common_lib/include"<<endl;
	ofs<<endl;
	ofs<<"HEADERS\t+= "<<pt.PLUGIN_HEADER<<endl;
	if(pt.PLUGIN_GUI != "") ofs<<"HEADERS\t+= "<<pt.PLUGIN_GUI<<endl; 
    ofs<<"SOURCES\t+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.h"<<endl;

    ofs<<""<<endl;

	ofs<<"SOURCES\t+= "<<pt.PLUGIN_CPP<<endl;
    ofs<<"SOURCES\t+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp"<<endl;
    ofs<<"SOURCES\t+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp"<<endl;
    ofs<<"SOURCES\t+= $$VAA3DPATH/../released_plugins_more/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp"<<endl;

    ofs<<"SOURCES\t+= $$VAA3DPATH/basic_c_fun/stackutil.cpp"<<endl;
    ofs<<"SOURCES\t+= $$VAA3DPATH/basic_c_fun/mg_utilities.cpp"<<endl;
    ofs<<"SOURCES\t+= $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp"<<endl;
    ofs<<"SOURCES\t+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.cpp"<<endl;
	ofs<<""<<endl;

    ofs<<"LIBS\t+= -lm -L$$VAA3DPATH/common_lib/lib -lv3dtiff"<<endl;
    ofs<<"LIBS\t+= -lpthread"<<endl;
    ofs<<"LIBS\t+= -lv3dfftw3f -lv3dfftw3f_threads"<<endl;

    ofs<<"TARGET\t= $$qtLibraryTarget(neuronassembler_single)"<<endl;
    ofs<<"DESTDIR\t= $$VAA3DPATH/../bin/plugins/neuron_tracing/neuronassembler_single/"<<endl;
	ofs.close();
}

void create_plugin_cpp(PluginTemplate & pt)
{
	cout<<"create "<<pt.PLUGIN_CPP<<" ... "<<endl;
	ofstream ofs(pt.PLUGIN_CPP.c_str());
    string line;
    string template_path = pt.VAA3D_PATH + "/../../vaa3d_tools/hackathon/zhi/neuronassembler_plugin_creator/neuronassembler_template.cpp";
    ifstream templatefile (template_path.c_str());
    if (templatefile.is_open())
    {
        for(int i = 0; i< 7; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        ofs<<"#include \""<<pt.PLUGIN_HEADER<<"\""<<endl;
        ofs<<"#include \""<<pt.VAA3D_PATH<< "/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readRawfile_func.h"<<"\""<<endl;
        ofs<<"#include \""<<pt.VAA3D_PATH<< "/../released_plugins_more/v3d_plugins/istitch/y_imglib.h"<<"\""<<endl;
        ofs<<"#include \""<<pt.VAA3D_PATH<< "/../released_plugins_more/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"<<"\""<<endl;
        ofs<<"Q_EXPORT_PLUGIN2("<<pt.PLUGIN_NAME<<", "<<pt.PLUGIN_CLASS<<");"<<endl;

        for(int i = 7;i< 114; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        ofs<<"QStringList "<<pt.PLUGIN_CLASS<<"::menulist() const"<<endl;

        for(int i = 114;i< 122; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        ofs<<"QStringList "<<pt.PLUGIN_CLASS<<"::funclist() const"<<endl;

        for(int i = 123;i< 130; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        ofs<<"void "<<pt.PLUGIN_CLASS<<"::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)"<<endl;
        for(int i = 130;i< 208; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        ofs<<"bool "<<pt.PLUGIN_CLASS<<"::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)"<<endl;
        for(int i = 208;i< 1258; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        templatefile.close();

    }

//	ofs<<"/* "<<pt.PLUGIN_CPP<<endl;
//	ofs<<" * "<<pt.PLUGIN_DESCRIPTION<<endl;
//	ofs<<" * "<<pt.PLUGIN_DATE<<" : by "<<pt.PLUGIN_AUTHOR<<endl;
//	ofs<<" */"<<endl;
//	ofs<<" "<<endl;
//	ofs<<"#include \"v3d_message.h\""<<endl;
//	ofs<<"#include <vector>"<<endl;
//	ofs<<"#include \""<<pt.PLUGIN_HEADER<<"\""<<endl;
//	ofs<<"using namespace std;"<<endl;
//	ofs<<"Q_EXPORT_PLUGIN2("<<pt.PLUGIN_NAME<<", "<<pt.PLUGIN_CLASS<<");"<<endl;
//	ofs<<" "<<endl;
//	ofs<<"QStringList "<<pt.PLUGIN_CLASS<<"::menulist() const"<<endl;
//	ofs<<"{"<<endl;
//	ofs<<"\treturn QStringList() "<<endl;
//	for(int i = 0; i < pt.MENUS.size(); i++) ofs<<"\t\t<<tr(\""<<pt.MENUS[i]<<"\")"<<endl;
//	ofs<<"\t\t<<tr(\"about\");"<<endl;
//	ofs<<"}"<<endl;
//	ofs<<""<<endl;
//	ofs<<"QStringList "<<pt.PLUGIN_CLASS<<"::funclist() const"<<endl;
//	ofs<<"{"<<endl;
//	ofs<<"\treturn QStringList()";
//	for(int i = 0; i < pt.FUNCS.size(); i++) ofs<<endl<<"\t\t<<tr(\""<<pt.FUNCS[i]<<"\")";
//	ofs<<";"<<endl;
//	ofs<<"}"<<endl;
//	ofs<<""<<endl;
//	ofs<<"void "<<pt.PLUGIN_CLASS<<"::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)"<<endl;
//	ofs<<"{"<<endl;
//	ofs<<"\tif (menu_name == tr(\""<<pt.MENUS[0]<<"\"))"<<endl;
//	ofs<<"\t{"<<endl;
//    ofs<<"\t\tv3d_msg(\"To be implemented.\");"<<endl;
//	ofs<<"\t}"<<endl;
//	for(int i = 1; i < pt.MENUS.size(); i++)
//	{
//		ofs<<"\telse if (menu_name == tr(\""<<pt.MENUS[i]<<"\"))"<<endl;
//		ofs<<"\t{"<<endl;
//  	    ofs<<"\t\tv3d_msg(\"To be implemented.\");"<<endl;
//		ofs<<"\t}"<<endl;
//	}
//	ofs<<"\telse"<<endl;
//	ofs<<"\t{"<<endl;
//	ofs<<"\t\tv3d_msg(tr(\""<<pt.PLUGIN_DESCRIPTION<<". \"\n\t\t\t\"Developed by "<<pt.PLUGIN_AUTHOR<<", "<<pt.PLUGIN_DATE<<"\"));"<<endl;
//	ofs<<"\t}"<<endl;
//	ofs<<"}"<<endl;
//	ofs<<""<<endl;

//	ofs<<"bool "<<pt.PLUGIN_CLASS<<"::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)"<<endl;
//	ofs<<"{"<<endl;
//	ofs<<"\tvector<char*> infiles, inparas, outfiles;"<<endl;
//	ofs<<"\tif(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);"<<endl;
//	ofs<<"\tif(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);"<<endl;
//	ofs<<"\tif(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);"<<endl;
//	ofs<<endl;
//	ofs<<"\tif (func_name == tr(\""<<pt.FUNCS[0]<<"\"))"<<endl;
//	ofs<<"\t{"<<endl;
//	ofs<<"\t\tv3d_msg(\"To be implemented.\");"<<endl;
//	ofs<<"\t}"<<endl;
//	for(int i = 1; i < pt.FUNCS.size(); i++)
//	{
//		ofs<<"\telse if (func_name == tr(\""<<pt.FUNCS[i]<<"\"))"<<endl;
//		ofs<<"\t{"<<endl;
//  	    ofs<<"\t\tv3d_msg(\"To be implemented.\");"<<endl;
//		ofs<<"\t}"<<endl;
//	}
//	ofs<<"\telse return false;"<<endl;
//	ofs<<endl<<"\treturn true;"<<endl;
//	ofs<<"}"<<endl;
//	ofs<<""<<endl;
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

    string line;
    string template_path = pt.VAA3D_PATH + "/../../vaa3d_tools/hackathon/zhi/neuronassembler_plugin_creator/neuronassembler_template.h";
    ifstream templatefile (template_path.c_str());
    if (templatefile.is_open())
    {
        for(int i = 0; i< 37; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        int d = 0;
        if(pt.PARA_NAME.size() >0)
        {
            for(d = 0; d < pt.PARA_NAME.size() ; d++)
            {
                if(pt.PARA_TYPE.at(d) == "string")
                {
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_edit = new QLineEdit();"<<endl;
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_edit->setText(\""<<pt.PARA_VALUE.at(d)<<"\");"<<endl;
                    ofs<<"\t\t\tlayout->addWidget(new QLabel(QObject::tr(\""<<pt.PARA_NAME.at(d)<<":\"))," << d+3 <<",0);"<<endl;
                    ofs<<"\t\t\tlayout->addWidget("<<pt.PARA_NAME.at(d)<<"_edit,"<<d+3<<",1,1,5);\n"<<endl;

                }
                else
                {
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_box = new QSpinBox();"<<endl;
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_box->setValue("<<pt.PARA_VALUE.at(d)<<");"<<endl;
                    ofs<<"\t\t\tlayout->addWidget(new QLabel(QObject::tr(\""<<pt.PARA_NAME.at(d)<<":\"))," << d+3 <<",0);"<<endl;
                    ofs<<"\t\t\tlayout->addWidget("<<pt.PARA_NAME.at(d)<<"_box,"<<d+3<<",1,1,5);\n"<<endl;

                }
            }

        }

        ofs<<"\t\t\tlayout->addLayout(hbox2,"<<d+3<<",0,3,6);"<<endl;

        for(int i = 37; i< 46; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        if(pt.PARA_NAME.size() >0)
        {
            for(int i = 0; i < pt.PARA_NAME.size() ; i++)
            {
                if(pt.PARA_TYPE.at(i) != "string")
                {
                    ofs<<"\t\t\tconnect("<<pt.PARA_NAME.at(i)<<"_box, SIGNAL(valueChanged("<< pt.PARA_TYPE.at(i)<<")), this, SLOT(update()));"<<endl;
                }
            }

        }

        for(int i = 46; i< 60; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        if(pt.PARA_NAME.size() >0)
        {
            for(int i = 0; i < pt.PARA_NAME.size() ; i++)
            {
                if(pt.PARA_TYPE.at(i) == "string")
                {
                        ofs<<"\t\t\t"<< pt.PARA_NAME.at(i)<<" = "<<pt.PARA_NAME.at(i) <<"_edit->text();"<<endl;
                }
                else
                {
                        ofs<<"\t\t\t"<< pt.PARA_NAME.at(i)<<" = "<<pt.PARA_NAME.at(i) <<"_box->value();"<<endl;

                }
            }

        }
        for(int i = 60; i< 117; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        if(pt.PARA_NAME.size() >0)
        {
            for(int i = 0; i < pt.PARA_NAME.size() ; i++)
            {
                if(pt.PARA_TYPE.at(i) == "string")
                {
                     ofs<<"\t\tQLineEdit *"<<pt.PARA_NAME.at(i)<<"_edit;"<<endl;
                     ofs<<"\t\tQString "<<pt.PARA_NAME.at(i)<<";"<<endl;
                }
                else
                {
                    ofs<<"\t\tQSpinBox *"<<pt.PARA_NAME.at(i)<<"_box;"<<endl;
                    ofs<<"\t\t"<<pt.PARA_TYPE.at(i)<<" "<<pt.PARA_NAME.at(i)<<";"<<endl;
                }
            }

        }

        for(int i = 117; i< 174; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        if(pt.PARA_NAME.size() >0)
        {
            for(d = 0; d < pt.PARA_NAME.size() ; d++)
            {
                if(pt.PARA_TYPE.at(d) == "string")
                {
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_edit = new QLineEdit();"<<endl;
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_edit->setText(\""<<pt.PARA_VALUE.at(d)<<"\");"<<endl;
                    ofs<<"\t\t\tlayout->addWidget(new QLabel(QObject::tr(\""<<pt.PARA_NAME.at(d)<<":\"))," << d+2 <<",0);"<<endl;
                    ofs<<"\t\t\tlayout->addWidget("<<pt.PARA_NAME.at(d)<<"_edit,"<<d+2<<",1,1,5);\n"<<endl;

                }
                else
                {
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_box = new QSpinBox();"<<endl;
                    ofs<<"\t\t\t"<<pt.PARA_NAME.at(d)<<"_box->setValue("<<pt.PARA_VALUE.at(d)<<");"<<endl;
                    ofs<<"\t\t\tlayout->addWidget(new QLabel(QObject::tr(\""<<pt.PARA_NAME.at(d)<<":\"))," << d+2 <<",0);"<<endl;
                    ofs<<"\t\t\tlayout->addWidget("<<pt.PARA_NAME.at(d)<<"_box,"<<d+2<<",1,1,5);\n"<<endl;

                }
            }

        }

        ofs<<"\t\t\tlayout->addLayout(hbox2,"<<d+2<<",0,2,6);"<<endl;

        for(int i = 174; i< 181; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        if(pt.PARA_NAME.size() >0)
        {
            for(int i = 0; i < pt.PARA_NAME.size() ; i++)
            {
                if(pt.PARA_TYPE.at(i) != "string")
                {
                    ofs<<"\t\t\tconnect("<<pt.PARA_NAME.at(i)<<"_box, SIGNAL(valueChanged("<< pt.PARA_TYPE.at(i)<<")), this, SLOT(update()));"<<endl;
                }
            }

        }

        for(int i = 181; i< 194; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        if(pt.PARA_NAME.size() >0)
        {
            for(int i = 0; i < pt.PARA_NAME.size() ; i++)
            {
                if(pt.PARA_TYPE.at(i) == "string")
                {
                        ofs<<"\t\t\t"<< pt.PARA_NAME.at(i)<<" = "<<pt.PARA_NAME.at(i) <<"_edit->text();"<<endl;
                }
                else
                {
                        ofs<<"\t\t\t"<< pt.PARA_NAME.at(i)<<" = "<<pt.PARA_NAME.at(i) <<"_box->value();"<<endl;

                }
            }

        }
        for(int i = 194; i< 225; i++)
        {
            getline (templatefile,line);
            ofs<<line<<endl;
        }

        if(pt.PARA_NAME.size() >0)
        {
            for(int i = 0; i < pt.PARA_NAME.size() ; i++)
            {
                if(pt.PARA_TYPE.at(i) == "string")
                {
                     ofs<<"\t\tQLineEdit *"<<pt.PARA_NAME.at(i)<<"_edit;"<<endl;
                     ofs<<"\t\tQString "<<pt.PARA_NAME.at(i)<<";"<<endl;
                }
                else
                {
                    ofs<<"\t\tQSpinBox *"<<pt.PARA_NAME.at(i)<<"_box;"<<endl;
                    ofs<<"\t\t"<<pt.PARA_TYPE.at(i)<<" "<<pt.PARA_NAME.at(i)<<";"<<endl;
                }
            }

        }
        ofs<<"\t};"<<endl;
        templatefile.close();

    }
	ofs.close();
}

void create_plugin_all(PluginTemplate & pt)
{
	create_plugin_header(pt);
	create_plugin_cpp(pt);
	//create_func_header(pt);             // May 11, 2012, no longer export func_header
	//create_func_cpp(pt);                // May 11, 2012, no longer export func_cpp
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


#endif
