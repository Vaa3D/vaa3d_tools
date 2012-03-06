/**********************************************
 * file : basic_parser.h  Oct 01, 2011, by Hang Xiao
 * 
 * This is very general parser. It contanin the
 * basic function of parser.
 * 
 * If you want to use it, there is a demo in 
 * parser_demo folder
 *************************************/
#ifndef __PARSER_H_H_
#define __PARSER_H_H_

#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>

#include "simple_c.h"

using namespace std;

//extern bool IS_VERBOSE;

typedef struct ArgumentAttribute 
{
	string para_name;
	int option_type;  
	// For sub_cmd
	// 0 : no arg
	// 1 : one arg
	// 2 : zero or one arg
	// 3 : at least one args, stop at next "-cmd" or end of string
	// For main_cmd, option_type is always 0 in v3d_convert
	// but meaningful in v3d_convert plugin
	// 0 : no input image
	// 1 : one input image
	// 2 : could be more than two input images

} Attribute;

struct Argument
{
	string para_name;          // -resize
	string para_string;
	Argument(){para_name=""; para_string=""; }
	Argument(string name, string str){para_name=name; para_string=str; }
};

struct BasicParser;

typedef bool (*UsageFunc)(string,ostream&);
typedef bool (*DemoFunc)(string,ostream&);
typedef int (*RunFunc)(BasicParser &parser); // return value
// 0 : parser/run error, -1 : not found, 1 : run ok 

struct BasicParser
{
	vector<string> filelist;
	vector<Argument> paras;
	vector<ArgumentAttribute> attrs;
	string s_error;
	UsageFunc usage_func;
	DemoFunc demo_func;
	RunFunc run_func;

	BasicParser(){usage_func = 0; demo_func = 0; attrs.reserve(1000);}
	virtual void set_usage_func(UsageFunc _func)
	{
		usage_func = _func;
	}
	// set_demo_func make v3d_convert work very unstable
	// virtual void set_demo_func(DemoFunc _func)
	// {
	// 	demo_func = _func;
	//}
	virtual void set_run_func(RunFunc _func)
	{
		run_func = _func;
	}
	// no distinc between global_opt, main_cmd and child_opt
	virtual bool set_attributes(const ArgumentAttribute * cmds, int n)
	{
		attrs.clear();
		if(n <= 0) return false;
		bool is_ok = true;
		set<string> allcmds;
		for(int i = 0; i < n; i++) 
		{
			if(allcmds.find(cmds[i].para_name) != allcmds.end())
			{
				cout<<"warning : duplicated command option "<<cmds[i].para_name<<endl;
				is_ok = false;
			}
			allcmds.insert(cmds[i].para_name);
			attrs.push_back(cmds[i]);
		}
		return false;
	}

	virtual bool is_main_cmd(string para_name)
	{
		if(attrs.empty()) return false;
		int cmd_num = attrs.size();
		for(int i = 0; i < cmd_num; i++)
		{
			string sup_cmd = attrs[i].para_name;
			if(sup_cmd[0] != '+') continue;

			string cur_cmd = para_name;
			sup_cmd.at(0) = '-';
			cur_cmd.at(0) = '-';
			if(sup_cmd == cur_cmd) return true;
		}
		return false;

	}
	virtual bool is_support(string para_name)
	{
		if(attrs.empty()) return false;
		int cmd_num = attrs.size();
		for(int i = 0; i < cmd_num; i++)
		{
			string sup_cmd = attrs[i].para_name;
			string cur_cmd = para_name;
			sup_cmd.at(0) = '-';
			cur_cmd.at(0) = '-';
			if(sup_cmd == cur_cmd) return true;
		}
		return false;
	}

	virtual int option_type(string para_name)
	{
		if(!is_support(para_name)) return false;
		int cmd_num = attrs.size();
		for(int i = 0; i < cmd_num; i++)
		{
			string cmd_opt = attrs[i].para_name;
			cmd_opt[0] = '-';
			if(cmd_opt == para_name) return attrs[i].option_type;
		}
	}

	virtual int get_order(string para_name)
	{
		if(paras.empty()) return -1;
		for(int i = 0; i < paras.size(); i++)
		{
			if(paras[i].para_name == para_name) return i;
		}
		return -1;
	}
	virtual bool is_exist(string para_name)
	{
		if(get_order(para_name) != -1) return true;
		return false;
	}
	virtual string get_main_cmd()
	{
		if(paras.empty()) return "";
		for(int i = 0; i < paras.size(); i++)
		{
			if(is_main_cmd(paras[i].para_name))
			{
				string main_cmd = paras[i].para_name;
				if(main_cmd[0] == '-' || main_cmd[0] == '+') main_cmd[0] = '-';
				return main_cmd;
			}
		}
	}
	virtual string get_para(string para_name)
	{
		int order = get_order(para_name);
		if(order == -1) return string("");
		return paras[order].para_string;
	}
	virtual int get_delim_num(string para_name, string delim="x")
	{
		int count = 0;
		if(!is_support(para_name) || !is_exist(para_name)) return 0;
		string str = get_para(para_name);
		size_t found = str.find_first_of(delim);
		while(found != string::npos)
		{
			count++;
			found = str.find_first_of(delim, found + 1);
		}
		return count;
	}
	virtual int get_int_para(string para_name)
	{
		int v = 0;
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return v;}
		v = atol(get_para(para_name).c_str());
		return v;
	}
	virtual int get_int_para(string para_name, int index, string sep = "x")
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return 0;}
		if(get_delim_num(para_name, sep) < index){s_error += "exceed the number of delim '" + sep + "'"; return 0;}
		string par_str = get_para(para_name);
		size_t sp=0, ep=par_str.find_first_of(sep);
		if(index == 0)
		{
			if(ep == string::npos) {return atoi(par_str.c_str());}
			else if(ep == 0){return 0;}
			else { return atoi(par_str.substr(sp, ep - sp).c_str());}
		}
		while(index>0)
		{
			if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
			else {s_error += "index exceed the number of "; s_error += sep; return 0;}
			index--;
		}
		if(ep == string::npos) ep = par_str.size();
		return atoi(par_str.substr(sp, ep - sp).c_str());
	}
	virtual double get_double_para(string para_name)
	{
		double v = 0.0;
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return v;}
		return atof(get_para(para_name).c_str());
	}
	virtual bool get_double_para(double &v, string para_name, int index, string &s_error, string sep = "x")
	{
	}
	virtual double get_double_para(string para_name, int index, string sep = "x")
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para"; return 0.0;}
		if(get_delim_num(para_name, sep) < index){s_error += "exceed the number of delim " + sep; return 0.0;}
		string par_str = get_para(para_name);
		size_t sp=0, ep=par_str.find_first_of(sep);
		if(index == 0)
		{
			if(ep == string::npos) {return atof(par_str.c_str());}
			else if(ep == 0){return 0.0;}
			else return atof(par_str.substr(sp, ep - sp).c_str());
		}
		while(index>0)
		{
			if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
			else {s_error += "index exceed the number of "; s_error += sep; return 0.0;}
			index--;
		}
		if(ep == string::npos) ep = par_str.size();
		return atof(par_str.substr(sp, ep - sp).c_str());
	}
	virtual void add_para(string para_name, string para_string){
		paras.push_back(Argument(para_name, para_string));
	}
	virtual void add_para(string para_name){
		paras.push_back(Argument(para_name, ""));
	}
	virtual void add_para(Argument para){
		paras.push_back(para);
	}

	// return true if parsed correctly
	// return false if parsed incorrectly
	// return -1 if not find
	virtual int parse(int argc, char* argv[], int arg_begin = 1)
	{
		filelist.clear();
		paras.clear();
		int i = arg_begin;      // switch ind

		while(i < argc)
		{
			Argument para;
			if(argv[i][0] != '-') filelist.push_back(string(argv[i]));
			else if(is_support(argv[i]) && !is_exist(argv[i]))
			{
				int opt_type = option_type(argv[i]);
				if(opt_type == 0) add_para(argv[i]);
				else if((opt_type == 1) && i+1 < argc && argv[i+1][0] != '-') {add_para(argv[i], argv[i+1]); i++;}
				else if(opt_type == 2) 
				{
					if(i+1 < argc && argv[i+1][0] != '-'){add_para(argv[i], argv[i+1]); i++;}
					else add_para(argv[i]);
				}
				else if((opt_type == 3) && i+1 < argc && argv[i+1][0] != '-')
				{
					string opt = argv[i+1];
					int j = i+2;
					while(j < argc && argv[j][0] != '-')
					{
						opt = opt + " " + argv[j];
						j++;
					}
					add_para(argv[i], opt);
					i = j - 1;
				}
				else {s_error += "need parameter for "; s_error += argv[i]; print_usage(argv[i], cout);return false;}
			}
			// check other error reason
			else if(!is_support(argv[i]))
			{
				s_error += argv[i];
				s_error += " is not supported";
				return -1;  // the other parser may recognize it
			}
			else if(is_exist(argv[i]))
			{
				s_error += "duplicated para ";
				s_error += argv[i];
				return false;
			}
			else
			{
				s_error += "what is this paramter";
				return -1; // unknown error
			}
			i++;
		}
		return true;
	}
	virtual bool run(int argc, char * argv[], int arg_begin = 1)
	{
		int rt = parse(argc, argv, arg_begin);
		if(rt != 1) return false;

		rt = (*run_func)((*this));
		if(rt == -1) return rt;

		if(!rt)
		{
			string main_cmd = get_main_cmd();
			cout<<endl;
			print_usage(main_cmd, cout);
			return false;
		}
		return true;
	}
	virtual bool print_usage(string cmd_name, ostream & out)
	{
		if(cmd_name == "") return false;
		if(usage_func) return (*(usage_func))(cmd_name, out); 
	}
	virtual bool print_demo(string cmd_name, ostream & out)
	{
		if(cmd_name == "") return false;
		if(demo_func) return (*(demo_func))(cmd_name, out); 
	}
	virtual vector<string> search(string query, int level){}
	virtual void list(){}
};
#endif
