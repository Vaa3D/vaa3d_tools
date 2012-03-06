// ${plugin_name}_plugin.cpp

#include <iostream>
#include <algorithm>
#include <QInputDialog>

#include "stackutil.h"
#include "basic_surf_objs.h"
#include "v3d_message.h"

#include "common_dialog.h"
#include "my_surf_objs.h"
#include "swc2mask_plugin.h"
#include "advanced_parser.h"

#include "run_main.h"

using namespace std;


Q_EXPORT_PLUGIN2(swc2mask, Cls_swc2maskPlugin);

AdvancedParser parser;       // global parser
string global_outimg_file;

Cls_swc2maskPlugin::Cls_swc2maskPlugin()
{
	int attr_bytes = sizeof(Attribute);

    parser.add_attributes(main_attrs, sizeof(main_attrs)/attr_bytes);
    parser.add_usage_func(main_usage);
    parser.add_demo_func(main_demo);
    parser.add_run_func(run_main);
}

QStringList Cls_swc2maskPlugin::menulist() const
{
	QStringList cmd_list;
	cmd_list.push_back("about");
	vector<string> cmd_vec = parser.get_cmd_list();
	sort(cmd_vec.begin(), cmd_vec.end());
	for(int i = 0; i < cmd_vec.size(); i++)
	{
		string cmd = cmd_vec[i];
		cmd = cmd.substr(1, cmd.size()-1);
		for(int j = 0; j < cmd.size(); j++) if(cmd[j] == '-') cmd[j] = ' ';
		cmd_list.push_back(cmd.c_str());
	}
	return cmd_list;
	/*
	return QStringList()
		<<tr("image dilation")
		<<tr("image erosion")
		<<tr("component mask")
		<<tr("mask to color")
		<<tr("about");
		*/
}

QStringList Cls_swc2maskPlugin::funclist() const
{
	QStringList cmd_list;
	vector<string> cmd_vec = parser.get_cmd_list();
	for(int i = 0; i < cmd_vec.size(); i++)
	{
		string cmd = cmd_vec[i];
		cmd = cmd.substr(1, cmd.size()-1);
		cmd_list.push_back(cmd.c_str());
	}
	return cmd_list;
	/*
	return QStringList()
		<<tr("ta")
		<<tr("about");
		*/
}

void Cls_swc2maskPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("about"))
	{
		QLabel * browser = new QLabel(tr("swc2mask plugin developed by Hang Xiao on 2012-03-05\n"));
		browser->show();
		return;
	}

	// 1. define dialog paras
	
	string inimg_file, outimg_file, newimg_file;
	string inmarker_file, outmarker_file;
	string inswc_file, outswc_file;

	v3dhandle curwin = callback.currentImageWindow();

	string main_cmd = menu_name.toStdString();
	for(int i = 0; i < main_cmd.size(); i++) if(main_cmd[i] == ' ') main_cmd[i] = '-';
	main_cmd = "-" + main_cmd;

	vector<string> para_list = parser.get_cmd_para_list(main_cmd);

	if(!callback.getImageWindowList().empty()) 
	{
		inimg_file = callback.getImage(curwin)->getFileName();
		//=============== save swc ==================
		QList<NeuronSWC> nt_lst = callback.getSWC(curwin).listNeuron;
		if(!nt_lst.empty() && find(para_list.begin(), para_list.end(), "-inswc") != para_list.end())
		{
			//inswc_file = (QDir::currentPath() + "/current.swc").toStdString();
			inswc_file = "/tmp/input.swc";
			cout<<"inswc_file = "<<inswc_file<<endl;
			vector<MyMarker*> tmp_swc;
			for(int i = 0; i < nt_lst.size(); i++)
			{
				NeuronSWC ns = nt_lst.at(i);
				MyMarker * marker = new MyMarker((double)(ns.x), (double)(ns.y), (double)(ns.z));
				marker->radius = ns.r;
				tmp_swc.push_back(marker);
			}
			for(int i = 0; i < nt_lst.size(); i++)
			{
				NeuronSWC ns = nt_lst.at(i);
				MyMarker * marker = tmp_swc[i];
				if(ns.pn >=0) marker->parent = tmp_swc[ns.pn];
			}
			saveSWC_file(inswc_file, tmp_swc);
			for(int i = 0; i < tmp_swc.size(); i++) {delete tmp_swc[i]; tmp_swc[i] = 0;}
		}
		//============= save marker ==================
		LandmarkList landmarks = callback.getLandmark(curwin);
		if(!landmarks.empty() && find(para_list.begin(), para_list.end(), "-inmarker") != para_list.end())
		{
			vector<MyMarker> tmp_markers(landmarks.size());
			cout<<"landmarks.size() = "<<landmarks.size()<<endl;
			cout<<"tmp_markers.size() = "<<tmp_markers.size()<<endl;
			for(int i = 0; i < landmarks.size(); i++) 
			{ LocationSimple loc = landmarks.at(i); tmp_markers[i] = MyMarker((double)(loc.x), (double)(loc.y), (double)(loc.z));
			  cout<<"("<<loc.x<<","<<loc.y<<","<<loc.z<<")"<<endl;
			}
			cout<<"tmp_markers.size() = "<<tmp_markers.size()<<endl;
			//inmarker_file = (QDir::currentPath() + "/current.marker").toStdString();
			inmarker_file = "/tmp/input.marker";
			saveMarker_file(inmarker_file, tmp_markers);
		}
	}
	if(find(para_list.begin(), para_list.end(), "-outmarker") != para_list.end()) outmarker_file = "/tmp/output.marker";
	if(find(para_list.begin(), para_list.end(), "-outswc") != para_list.end()) outswc_file = "/tmp/output.swc";
	

	typedef pair<string, string> STRPAIR;
	vector<pair<string, string> > items;

	int main_cmd_type = parser.option_type(main_cmd); 
	if(main_cmd_type != 0) 
	{
		if(main_cmd_type != 2) items.push_back(STRPAIR(menu_name.toStdString(), ""));
		else if(main_cmd_type == 2) items.push_back(STRPAIR(menu_name.toStdString(), "(optional)"));
	}
	items.push_back(STRPAIR("input image", inimg_file));

	for(int i = 0; i < para_list.size(); i++)
	{
		string para_name = para_list[i];
		int opt_type = parser.option_type(para_name, main_cmd);
		string str1 = para_name, str2 = "";

		if(str1[0] == '-') str1 = str1.substr(1, str1.size() - 1);
		if(str1.find("in") == 0) str1 = "input " + str1.substr(2, str1.size() - 2);
		else if(str1.find("out") == 0) str1 = "output " + str1.substr(3, str1.size() - 3);
		for(int j = 0; j < str1.size(); j++) if(str1[j] == '-')str1[j] = ' ';
		if(opt_type == 0) str1 = "is " + str1;

		if(opt_type == 0) str2 = "no";
		if(opt_type == 2) str2 = "(optional)";
		else if(para_name == "-inmarker") str2 = inmarker_file;
		else if(para_name == "-outmarker") str2 = outmarker_file;
		else if(para_name == "-inswc") str2 = inswc_file;
		else if(para_name == "-outswc") str2 = outswc_file;

		items.push_back(STRPAIR(str1, str2));
	}
	if(outimg_file != "") items.push_back(STRPAIR("output image", outimg_file));
	else items.push_back(STRPAIR("output image", "(optional)"));

	CommonDialog dialog(items, parent);
	dialog.setTitle(menu_name);
	// set demo
	{
		ostringstream oss1; parser.print_usage(main_cmd, oss1);
		ostringstream oss2; parser.print_demo(main_cmd, oss2);
		dialog.setHelp(QObject::tr(oss1.str().c_str()));
		vector<string> demo_strs;
		istringstream iss2(oss2.str());
		while(iss2.good())
		{
			string line; getline(iss2, line);
			if(line != "") demo_strs.push_back(line);
		}
		if(!demo_strs.empty()) dialog.setDemo(demo_strs);
	}
	// set history
	{
		string home = getenv("HOME");
		string swc2mask_history = home + "/.swc2mask_history";
		ifstream ifs(swc2mask_history.c_str());
		vector<string> history_strs;
		vector<string> tmp_strs;
		set<string> history_set;
		while(ifs.good())
		{
			string line; getline(ifs, line);
			if(line != "" && line.find(main_cmd) != string::npos) tmp_strs.push_back(line);
		}
		ifs.close();
		for(vector<string>::reverse_iterator it = tmp_strs.rbegin(); it != tmp_strs.rend(); it++)
		{
			if(history_set.find(*it) == history_set.end()) 
			{
				history_strs.push_back(*it);
				history_set.insert(*it);
			}
		}
		if(!history_strs.empty()) dialog.setHistory(history_strs);
	}

	if (dialog.exec()!=QDialog::Accepted) return;

	// 2. get paras from dialog and run
	vector<string> args;
	args.push_back("swc2mask");
	string para_value = dialog.get_para("input image");
	if(para_value != "") 
	{
		char ** argv;
		int argc = split_string(para_value.c_str(), argv);
		for(int j = 0; j < argc; j++) args.push_back(string(argv[j]));
		if(argc>0) newimg_file = argv[0];
	}

	// push main cmd
	args.push_back(main_cmd);
	if(main_cmd_type != 0)
	{
		para_value = dialog.get_para(menu_name.toStdString());
		if(main_cmd_type == 1) 
		{
			if(para_value == "") {v3d_msg("you should set value for " + menu_name); return;}
			args.push_back(para_value);
		}
		else if(main_cmd_type == 2) // 0 or 1 value needed
		{
			if(para_value != "" && para_value != "(optional)") args.push_back(para_value);
		}
		else if(main_cmd_type == 3) // 1 or more value
		{
			if(para_value == "") {v3d_msg("you should set value for " + menu_name); return;}
			if(para_value != "")
			{
				int pos1 = 0, pos2 = 0;
				while(true)
				{
					pos1 = para_value.find_first_not_of(" ", pos2);
					if(pos1 == string::npos) break;
					pos2 = para_value.find_first_of(" ", pos1);
					if(pos2 != string::npos)
					{
						string value = para_value.substr(pos1, pos2 - pos1);
						args.push_back(value);
					}
					else if(pos2 == string::npos)
					{
						string value = para_value.substr(pos1, para_value.size() - pos1);
						args.push_back(value);
						break;
					}
				}
			}
		}
	}

	for(int i = 0; i < para_list.size(); i++)
	{
		string para_name = para_list[i];
		int opt_type = parser.option_type(para_name, main_cmd);

		string str1 = para_name;
		if(str1[0] == '-') str1 = str1.substr(1, str1.size() - 1);
		if(str1.find("in") == 0) str1 = "input " + str1.substr(2, str1.size() - 2);
		else if(str1.find("out") == 0) str1 = "output " + str1.substr(3, str1.size() - 3);
		for(int j = 0; j < str1.size(); j++) if(str1[j] == '-')str1[j] = ' ';
		if(opt_type == 0) str1 = "is " + str1;

		para_value = dialog.get_para(str1);
		if(para_name == "-outmarker") outmarker_file = para_value;
		else if(para_name == "-outswc") outswc_file = para_value;

		if(opt_type == 0) 
		{
			if(para_value == "yes") args.push_back(para_name);
		}
		else if(opt_type == 1) 
		{
			if(para_value != "")
			{
				args.push_back(para_name); 
				args.push_back(para_value);
			}
		}
		else if(opt_type == 2) // 0 or 1 value needed
		{
			if(para_value == "" || para_value == "(optional)") args.push_back(para_name);
			else
			{
				args.push_back(para_name);
				args.push_back(para_value);
			}
		}
		else if(opt_type == 3) // 1 or more value
		{
			if(para_value != "")
			{
				args.push_back(para_name);
				int pos1 = 0, pos2 = 0;
				while(true)
				{
					pos1 = para_value.find_first_not_of(" ", pos2);
					if(pos1 == string::npos) break;
					pos2 = para_value.find_first_of(" ", pos1);
					if(pos2 != string::npos)
					{
						string value = para_value.substr(pos1, pos2 - pos1);
						args.push_back(value);
					}
					else if(pos2 == string::npos)
					{
						string value = para_value.substr(pos1, para_value.size() - pos1);
						args.push_back(value);
						break;
					}
				}
			}
		}
	}
	para_value = dialog.get_para("output image");
	if(para_value != "" && para_value != "(optional)")
	{
		args.push_back(para_value);
		outimg_file = para_value;
	}

	// run
	int argc = args.size();
	char ** argv = new char*[argc];
	string run_str;
	for(int i = 0; i < argc; i++)
	{
		run_str += string(" " + args[i]);
		argv[i] = (char*)args[i].c_str();
	}
	cout<<run_str<<endl;

	if(!parser.run(argc, argv)) {v3d_msg("Failed to run " + menu_name + " !"); return;}
	parser.save_to_history(argc, argv);
	cout<<"global_outimg_file = "<<global_outimg_file<<endl;
	outimg_file = global_outimg_file; global_outimg_file = "";	

	if(outimg_file == "") v3d_msg("Finished successfully !");
	// 3. display result

	if(newimg_file != "" && is_img_file(newimg_file) && newimg_file != inimg_file)
	{
		unsigned char * newimg1d = 0; V3DLONG * new_sz = 0;  int datatype;
		if(loadImage((char*)(newimg_file.c_str()), newimg1d, new_sz, datatype)) 
		{
			Image4DSimple * newobj = new Image4DSimple();
			newobj->setData(newimg1d, new_sz[0], new_sz[1], new_sz[2], new_sz[3], (ImagePixelType)datatype);
			curwin = callback.newImageWindow();
			callback.setImage(curwin, newobj);
			callback.setImageName(curwin, QString(newimg_file.c_str()));
			callback.updateImageWindow(curwin);
		}
	}
	if(outimg_file != "")
	{
		unsigned char * outimg1d = 0; V3DLONG * out_sz = 0;  int datatype;
		if(loadImage((char*)(outimg_file.c_str()), outimg1d, out_sz, datatype)) 
		{
			Image4DSimple * newobj = new Image4DSimple();
			newobj->setData(outimg1d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], (ImagePixelType)datatype);
			v3dhandle newwin = callback.newImageWindow();
			callback.setImage(newwin, newobj);
			callback.setImageName(newwin, QString(outimg_file.c_str()));
			callback.updateImageWindow(newwin);
		}
	}
	if(curwin && outmarker_file != "")
	{
		QList<ImageMarker> all_markers = readMarker_file(QObject::tr(outmarker_file.c_str()));
		LandmarkList marker_list;
		for(int i = 0; i < all_markers.size(); i++) 
		{
			int x = all_markers[i].x + 0.5;
			int y = all_markers[i].y + 0.5;
			int z = all_markers[i].z + 0.5;
			marker_list.push_back(LocationSimple(x,y,z));
		}
		callback.setLandmark(curwin, marker_list);
	}
	if(curwin && outswc_file != "")
	{
		NeuronTree nt = readSWC_file(QObject::tr(outswc_file.c_str()));
		nt.editable = false;
		callback.setSWC(curwin, nt);
		callback.open3DWindow(curwin);
		View3DControl* view3d = callback.getView3DControl(curwin);
		view3d->updateWithTriView(); // display swc
	}

	callback.updateImageWindow(curwin);
}

bool Cls_swc2maskPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("about"))
	{
		cout<<"Hello, this is swc2mask plugin"<<endl;
		return true;
	}
	vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *)input.at(0).p : 0;
	vector<char*> * pinparas = (input.size() >= 2) ? (vector<char*> *)input.at(1).p : 0;
	vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *)output.at(0).p : 0;
	vector<char*> infiles; if(pinfiles) infiles = *pinfiles;
	vector<char*> inparas; if(pinparas) inparas = *pinparas;
	vector<char*> outfiles; if(poutfiles) outfiles = *poutfiles;

	vector<string> args;
	args.push_back("swc2mask");
	args.push_back("-" + func_name.toStdString());
	for(int i = 0; i < infiles.size(); i++)
	{
		string file = infiles[i];
		string ft = file_type(file);
		if(is_img_file(file)) args.push_back(file);
		else if(ft == ".marker") {args.push_back("-inmarker"); args.push_back(file);}
		else if(ft == ".swc") {args.push_back("-inswc"); args.push_back(file);}
	}

	for(int i = 0; i < outfiles.size(); i++)
	{
		string file = outfiles[i];
		string ft = file_type(file);
		if(is_img_file(file)) args.push_back(file);
		else if(ft == ".marker") {args.push_back("-outmarker"); args.push_back(file);}
		else if(ft == ".swc") {args.push_back("-outswc"); args.push_back(file);}
	}

	for(int i = 0; i < inparas.size(); i++)
	{
		string para = inparas[i];
		if(para[0] == '#') para[0] = '-';
		args.push_back(para);
	}
	int argc = args.size();
	char ** argv = new char*[argc];
	for(int i = 0; i < argc; i++)
	{
		cout<<args[i]<<" ";
		argv[i] = (char*)args[i].c_str();
	}
	cout<<endl;

	return parser.run(argc, argv);
}
