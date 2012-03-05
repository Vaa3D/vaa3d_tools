/*****************************************************
 * file : advanced_parser.h  Oct 01, 2011, by HangXiao
 *
 * function : derived from parser.h, it will support much powerful parameter parser work. And it is specially designed for v3d_convert
 * new features :
 * 1. support main command, the command with preffix '+' will be marked as main command, otherwise option. The option command belong to the nearest main command. If no main command before itself, it will be global option command.
 * 2. support abbreviation, each command will seach in the following order
 *    a. if has_main_command , search all its option command and then the global option command
 *    b. if no main_command, search all main command and all global option command
 * 3. the matching between abbreviation and support command will be
 	  a. is exactly same, reture true
 *    b. split the support command into many parts by '-' or '+'
 *    b. if abbr lenth is less or equal to parts number ,
 *          extract the first word of each part to form a new string 
 *          if abbr is the substring of the new string, return true
 *       else 
 *          start from the first part
 *          if no common prefix return false
 *          if common prefix between abbr and curent part is no empty, cutoff the common prefix of abbr, go to next part 
 *****************************************************/

#ifndef __Perfect_PARSER_H__
#define __Perfect_PARSER_H__

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <ctime>

#include "simple_c.h"
#include "basic_parser.h"
#include "common_macro.h"

#ifdef __USE_CURL_DOWNLOAD__
#include <string>
#include <stdio.h>
#include <curl/curl.h>
#endif

using namespace std;

// return common prefix number
int common_prefix(string str1, string str2);

// make sure abbr is abbreviation


struct AdvancedParser : public BasicParser
{
	map<string, int> allglobal_opts;
	map<string, int> allmain_cmds;  // store "-cmd-name" instead of "+cmd-name"
	vector<const Attribute*> attributes;
	vector<int> attrs_num;
	vector<UsageFunc> usage_funcs;
	vector<DemoFunc> demo_funcs;
	vector<RunFunc> run_funcs;
	bool is_silent;

	AdvancedParser() : BasicParser(){is_silent = false;}
	virtual ~AdvancedParser(){}
	//AdvancedParser(ArgumentAttribute * cmds, int n) : BasicParser(cmds, n){}

	bool set_silent(){is_silent = true;}
	virtual bool set_attributes(const ArgumentAttribute * _attrs, int n)
	{
		if(n <= 0) return false;
		attrs.clear();
		// oct 19, store _attrs
		attributes.clear();
		attrs_num.clear();
		attributes.push_back(_attrs);
		attrs_num.push_back(n);
		// over
		for(int i = 0; i < n; i++) attrs.push_back(_attrs[i]);
		allmain_cmds.clear();
		allglobal_opts.clear();
		return check_valid();
	}

	virtual bool add_attributes(const ArgumentAttribute * _attrs, int n)
	{
		if(n <= 0) return false;
		// oct 19, store _attrs
		attributes.push_back(_attrs);
		attrs_num.push_back(n);
		// over
		int i = 0; 
		while(i < n && _attrs[i].para_name[0] == '-') {attrs.insert(attrs.begin(),_attrs[i]);i++;}
		while(i < n){attrs.push_back(_attrs[i]);i++;}

		allmain_cmds.clear();
		allglobal_opts.clear();
		return check_valid();
	}

	virtual void set_usage_func(UsageFunc _func)
	{
		usage_funcs.clear();
		usage_funcs.push_back(_func);
	}
	virtual void set_demo_func(DemoFunc _func)
	{
		demo_funcs.clear();
		demo_funcs.push_back(_func);
	}

	virtual void add_usage_func(UsageFunc _func)
	{
		usage_funcs.push_back(_func);
	}
	virtual void add_demo_func(DemoFunc _func)
	{
		demo_funcs.push_back(_func);
	}

	virtual void set_run_func(RunFunc _func)
	{
		run_funcs.clear();
		run_funcs.push_back(_func);
	}

	virtual void add_run_func(RunFunc _func)
	{
		run_funcs.push_back(_func);
	}
	// abbr and full should better prefixed by '-'
	bool is_abbr_match(string abbr, string full, int level)
	{
		if(abbr[0] != '-') abbr = "-" + abbr;
		if(full[0] != '-') full = "-" + full;

		// level 0
		if(level == 0) return (abbr == full);

		//get splits from abbr and full
		vector<string> abbr_splits, full_splits;
		int ps = 0, pe = 0;
		while((pe = abbr.find_first_of("-", ps+1)) != string::npos)
		{
			abbr_splits.push_back(abbr.substr(ps+1, pe - ps -1));
			ps = pe;
		}
		abbr_splits.push_back(abbr.substr(ps+1, abbr.size() - ps -1));

		ps = 0; pe = 0;
		while((pe = full.find_first_of("-", ps+1)) != string::npos)
		{
			full_splits.push_back(full.substr(ps+1, pe - ps -1));
			ps = pe;
		}
		full_splits.push_back(full.substr(ps+1, full.size() - ps - 1));


		if(abbr_splits.size() == 1)
		{
			abbr = abbr_splits[0];

			int common_num = 0;
			for(int i = 0; i < full_splits.size(); i++)
			{
				if(abbr == "") return true;
				common_num = common_prefix(abbr, full_splits[i]);
				if(common_num <= 0) return false;
				abbr = abbr.substr(common_num, abbr.size() - common_num);
			}
			// level 1
			if(level >= 1 && abbr == "") return true;

			// level 2 for abbr != ""

			string last_split = full_splits[full_splits.size() -1];
			last_split = last_split.substr(common_num, last_split.size() - common_num);
			int pos = -1;
			bool is_find = true;
			for(int i = 0; i < abbr.size(); i++)
			{
				pos = last_split.find_first_of(abbr[i],pos+1);
				if(pos == string::npos){is_find = false;break;}
			}
			if(level >= 2 && is_find) return true;
		}
		else if(abbr_splits.size() > full_splits.size()) return false;
		else // abbr_splits.size() <= full_splits.size() && abbr_splits.size() > 1
		{
			bool is_find = true;
			for(int i = 0; i < abbr_splits.size(); i++)
			{
				string abbr_str = abbr_splits[i];
				string full_str = full_splits[i];

				if(abbr_str.size() > full_str.size()) {is_find = false; break;}
				if(abbr_str[0] != full_str[0]){is_find = false; break;}
				int pos = 0;
				for(int j = 1; j < abbr_str.size(); j++)
				{
					pos = full_str.find_first_of(abbr_str[j],pos+1);
					if(pos == string::npos){is_find = false;break;}
				}
				if(!is_find) break;
			}
			if(level >= 1 && is_find) return true;
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

	virtual int option_type(string para_name, string main_cmd)
	{
		int main_id = allmain_cmds[main_cmd];
		int i = main_id;
		if(para_name == main_cmd)
		{
			return attrs[i].option_type;
		}
		while(++i < attrs.size() && attrs[i].para_name[0] != '+')
		{
			if(attrs[i].para_name == para_name) return attrs[i].option_type;
		}
		int global_id = allglobal_opts[para_name];
		return attrs[global_id].option_type;

		return -1;
	}

	bool find_abbr_in_range(string abbr, map<string, int> & range, vector<string> & candidates)
	{
		if(abbr[0] != '-') abbr = "-" + abbr;
		int level = 0;
		int is_find = 0;
		while(level <= 2)
		{
			for(map<string,int>::iterator it = range.begin(); it != range.end(); it++)
			{
				string cur_cmd = (*it).first;
				if(is_abbr_match(abbr, cur_cmd, level))
				{
					is_find = true;
					candidates.push_back(cur_cmd);
				}
			}
			if(is_find) break;
			level++;
		}
		if(candidates.empty()) return 0;
		return (level >= 1);
	}

	virtual bool check_valid()
	{
		int i = 0;
		bool is_ok = true;
		set<string> allchild_cmds;
		while(i < attrs.size() && attrs[i].para_name[0] == '-') 
		{
			string cur_opt = attrs[i].para_name;
			if(allglobal_opts.find(cur_opt) != allglobal_opts.end())
			{
				cout<<"duplicated global option "<<cur_opt<<endl;
				is_ok = false;
			}
			allglobal_opts[cur_opt] = i;
			i++;
		}
		set<string> localchild_opts;
		string cur_main = "";
		while(i < attrs.size())
		{
			string cur_opt = attrs[i].para_name;
			if(cur_opt[0] == '+')
			{
				cur_main = cur_opt;
				localchild_opts.clear();
				string cpy_opt = cur_opt; cpy_opt[0] = '-';
				if(allmain_cmds.find(cpy_opt) != allmain_cmds.end())
				{
					cout<<"duplicated main command "<<cur_opt<<endl;
					is_ok = false;
				}
				allmain_cmds[cpy_opt] = i;

				if(allglobal_opts.find(cpy_opt) != allglobal_opts.end())
				{
					cout<<"main command "<<cur_opt<<" duplicat with global option "<<cpy_opt<<endl;
					is_ok = false;
				}
			}
			else
			{
				if(localchild_opts.find(cur_opt) != localchild_opts.end())
				{
					cout<<"duplicated child option "<<cur_opt<<"  in main cmd "<<cur_main<<endl;
					return false;
				}
				localchild_opts.insert(cur_opt);
				allchild_cmds.insert(cur_opt);
			}
			i++;
		}
		map<string, int>::iterator it = allmain_cmds.begin();
		while(it != allmain_cmds.end())
		{
			string main_cmd = (*it).first;
			if(allchild_cmds.find(main_cmd) != allchild_cmds.end())
			{
				string child_opt = main_cmd;
				main_cmd[0] = '+';
				cout<<"main command "<<main_cmd<<" duplicate with child option "<<child_opt<<endl;
				is_ok = false;
			}
			it++;
		}
		return is_ok;
	}

	// return true is successfully
	// return false if find main cmd but sub option error
	// return -1 if no main cmd and sub option error
	int abbr2full(int argc, char* argv[], int arg_begin, vector<string> &args, bool &is_abbr_exist)
	{
		is_abbr_exist = false;
		args.clear();
		if(arg_begin == 1) args.push_back(argv[0]);
		string abbr;
		bool has_main_cmd;
		int main_cmd_id;
		string main_cmd;
		int i = arg_begin;
		for(i = arg_begin; i < argc; i++) if(argv[i][0] == '+') argv[i][0] = '-';
		i = arg_begin;
		while(i < argc && argv[i][0] != '-')
		{
			args.push_back(argv[i]);
			i++;
		}
		if(i == argc) return true; // empty paras
		abbr = argv[i];
		// first command should be analysed as main command
		int find_num = 0;
		vector<string> candidates;
		int id = 1;
		is_abbr_exist |= find_abbr_in_range(abbr, allmain_cmds, candidates);
		if(!candidates.empty())
		{
			find_num = candidates.size();
			id = 1;
			if(find_num >1)
			{
				cout<<"Do you mean "<<abbr<<endl;
				cout<<endl;
				for(int j = 0; j < find_num; j++)
				{
					cout<<" "<<j+1<<": "<<candidates[j]<<endl;
				}
				cout<<endl;
				if(is_silent) return false;
				cout<<"choose : [1] ";
				cin>>id;
			}
			if(id < 1 || id > find_num) id = 1;
			main_cmd_id = allmain_cmds[candidates[id-1]];
			main_cmd = attrs[main_cmd_id].para_name;
			main_cmd[0] = '-';
			args.push_back(main_cmd);
			has_main_cmd = true;
			i++;
		}
		else
		{
			has_main_cmd = false;
		}
		while(i < argc)
		{
			if(argv[i][0] != '-' || is_number(string(argv[i])))
			{
				args.push_back(argv[i]);
				i++;
				continue;
			}
			else
			{
				abbr = argv[i];
				if(has_main_cmd)
				{
					map<string, int> range;
					int j = main_cmd_id + 1; 
					while(j < attrs.size() && attrs[j].para_name[0] != '+')
					{
						string local_opt = attrs[j].para_name;
						range[local_opt] = j;
						j++;
					}
					candidates.clear();
					is_abbr_exist |= find_abbr_in_range(abbr, range, candidates);
					if(!candidates.empty())
					{
						find_num = candidates.size();
						id = 1;
						if(find_num >1)
						{
							cout<<"Do you mean "<<abbr<<endl;
							cout<<endl;
							for(int j = 0; j < find_num; j++)
							{
								cout<<" "<<j+1<<": "<<candidates[j]<<endl;
							}
							cout<<endl;
							if(is_silent) return false;
							cout<<"choose : [1] ";
							cin>>id;
						}
						if(id < 1 || id > find_num) id = 1;

						string local_opt = candidates[id-1];
						args.push_back(local_opt);
						i++;
						continue;
					}
				}
				candidates.clear();
				is_abbr_exist |= find_abbr_in_range(abbr, allglobal_opts, candidates);
				if(!candidates.empty())
				{
					find_num = candidates.size();
					id = 1;
					if(find_num >1)
					{
						cout<<"Do you mean "<<abbr<<endl;
						cout<<endl;
						for(int j = 0; j < find_num; j++)
						{
							cout<<" "<<j+1<<": "<<candidates[j]<<endl;
						}
						cout<<endl;
						if(is_silent) return false;
						cout<<"choose : [1] ";
						cin>>id;
					}
					if(id < 1 || id > find_num) id = 1;
					string global_opt = candidates[id-1];
					args.push_back(global_opt);
					i++;
					continue;
				}
				else
				{
					args.clear();
					if(has_main_cmd) {
						s_error += "unknow sub option " + abbr + " for " + main_cmd;
						print_usage(main_cmd,cout);
						return false;
					}
					else {s_error += "no such option " + abbr; return -1;}
				}
			}
		}

		return true;
	}
	virtual int parse(int argc, char* argv[], int arg_begin = 1)
	{
		filelist.clear();
		paras.clear();

		int i = arg_begin;      // switch ind
		bool is_abbr_exist = false;
		vector<string> args;

		int rt = abbr2full(argc, argv, arg_begin, args, is_abbr_exist);
		if(rt != true) return rt;
		if(is_abbr_exist)
		{
			for(int j = 0; j < argc; j++) 
				cout<<args[j]<<" ";
			cout<<endl;
		}

#ifdef __USE_CURL_DOWNLOAD__
		for(int j = 0; j < args.size(); j++)
		{
			string file = args[j];
			if(file.find("http://") == 0 || file.find("ftp://") == 0)
			{
				string url = file;
				while(file[file.size() - 1] == '/') file = file.substr(0, file.size() - 1);
				int pos = file.find_last_of("/");
				string save_file = "/tmp/" + file.substr(pos+1, file.size() - pos - 1);
				args[j] = save_file;
				ifstream ifile(save_file.c_str());
				if(!ifile) HTTP_DOWNLOAD(url, save_file);
			}
		}
#endif
		string main_cmd;
		for(int j = arg_begin; j < argc; j++)
		{
			if(args[j][0] == '-' && allmain_cmds.find(args[j]) != allmain_cmds.end()) {main_cmd = args[j]; break;}
		}
		while(i < argc)
		{
			Argument para;
			if(args[i][0] != '-' || is_number(args[i])) filelist.push_back(string(args[i]));
			else if(is_support(args[i]) && !is_exist(args[i]))
			{
				int opt_type = option_type(args[i], main_cmd);

				if(opt_type == 0) add_para(args[i]);
				else if((opt_type == 1) && i+1 < argc && (args[i+1][0] != '-' || is_number(args[i+1]))) {add_para(args[i], args[i+1]); i++;}
				else if((opt_type == 2)) 
				{
					if(i+1 < argc && (args[i+1][0] != '-' || is_number(args[i+1]))){add_para(args[i], args[i+1]); i++;}
					else add_para(args[i]);
				}
				else if((opt_type == 3) && i+1 < argc && (args[i+1][0] != '-' || is_number(args[i+1])))
				{
					string opt = args[i+1];
					int j = i+2;
					while(j < argc && (args[j][0] != '-' || is_number(args[j])))
					{
						opt = opt + " " + args[j];
						j++;
					}
					add_para(args[i], opt);
					i = j - 1;
				}
				else {
					s_error += "need parameter for "; 
					s_error += args[i]; 
					/*for(int i = 0; i < usage_funcs.size(); i++) 
					{
						if((*(usage_funcs[i]))(args[i], cout)) break;
					}*/
					print_usage(args[i],cout);
					return false;}
			}
			// check other error reason
			else if(!is_support(args[i]))
			{
				s_error += args[i];
				s_error += " is not supported";
				return -1;
			}
			else if(is_exist(args[i]))
			{
				s_error += "duplicated para ";
				s_error += args[i];
				return false;
			}
			else
			{
				s_error += "what is this paramter";
				return -1;
			}
			i++;
		}

		return true;
	}

	// just to link all the usage function
	virtual bool print_usage(string cmd_name, ostream & out)
	{
		if(cmd_name == "") return false;
		bool is_usage_found = 0;
		out<<"Usage :"<<endl<<endl;
		for(int i = 0; i < usage_funcs.size(); i++) if((*(usage_funcs[i]))(cmd_name, out)) return true;

		vector<string> candidates;
		bool is_abbr = find_abbr_in_range(cmd_name, allmain_cmds, candidates);
		int find_num = candidates.size();
		if(candidates.empty())
		{
			out<<"Invalid command : "<<cmd_name<<endl;
			return false;
		}
		else if(find_num == 1)
		{
			cmd_name = candidates[0];
		}
		else
		{
			string abbr = cmd_name.substr(1, cmd_name.size() -1);
			int id = 1;
			cout<<"Do you mean "<<abbr<<endl;
			cout<<endl;
			for(int j = 0; j < find_num; j++)
			{
				cout<<" "<<j+1<<": "<<candidates[j]<<endl;
			}
			cout<<endl;
			if(is_silent) return false;
			cout<<"choose : [1] ";
			cin>>id;
			if(id < 1 || id > find_num) id = 1;
			cmd_name = candidates[id -1];
		}
		for(int i = 0; i < usage_funcs.size(); i++) if((*(usage_funcs[i]))(cmd_name, out)) return true;

		int main_cmd_id = allmain_cmds[cmd_name];
		int i = main_cmd_id;
		out<<"swc2mask [<img_file>] ";
		while(i < attrs.size())
		{
			string para_name = attrs[i].para_name;
			para_name[0] = '-';
			int opt_type = attrs[i].option_type;
			out<<para_name<<" ";
			if(opt_type == 0)
			{
			}
			else if(opt_type == 1)
			{
				out<<"<para> ";
			}
			else if(opt_type == 2)
			{
				out<<"[<para>] ";
			}
			else if(opt_type > 2)
			{
				out<<"<para> ... ";
			}
			i++;
			if( i >= attrs.size() || attrs[i].para_name[0] == '+') break; 
		}
		out<<endl<<endl;
	}
	virtual bool print_demo(string cmd_name, ostream & out)
	{
		if(cmd_name == "" || cmd_name == "-")
		{
			int j=1;
			for(int i = 0; i < attrs.size(); i++)
			{
				string name = attrs[i].para_name;
				if(name == "+") continue;
				if(name[0] == '+')
				{
					name[0] = '-';
					print_demo(name, out);
				}
			}
			return true;
		}
		bool is_demo_found = 0;
		for(int i = 0; i < demo_funcs.size(); i++) if((*(demo_funcs[i]))(cmd_name, out)) return true;

		vector<string> candidates;
		bool is_abbr = find_abbr_in_range(cmd_name, allmain_cmds, candidates);
		int find_num = candidates.size();
		if(candidates.empty())
		{
			out<<"No demo for "<<cmd_name<<endl;
			return false;
		}
		else if(find_num == 1)
		{
			cmd_name = candidates[0];
			for(int i = 0; i < demo_funcs.size(); i++) (*(demo_funcs[i]))(cmd_name, out);
		}
		else
		{
			for(int j = 0; j < find_num; j++)
			{
				cmd_name = candidates[j];
				for(int i = 0; i < demo_funcs.size(); i++) (*(demo_funcs[i]))(cmd_name, out);
			}
		}
	}
	virtual vector<string> search(string query, int level)
	{
		set<string> result;
		vector<string> output;
		// 1. search abbr in main commands, return matched main commands
		vector<string> candidates;
		find_abbr_in_range(query, allmain_cmds, candidates);
		if(!candidates.empty())
		{
			for(int i = 0; i < candidates.size(); i++)
			{
				result.insert(candidates[i]);
				output.push_back(candidates[i]);
			}
		}
		if(level <= 1) return output;
		// 2. search abbr in options of each main command and return matched main command and the option
		map<string, int>::iterator it = allmain_cmds.begin();
		while(it != allmain_cmds.end())
		{
			string main_cmd = (*it).first;
			int next_id = (*it).second;
			map<string, int> range;
			while(next_id+1 < attrs.size() && attrs[next_id+1].para_name[0] != '+')
			{
				string cur_opt = attrs[next_id+1].para_name;
				range[cur_opt] = next_id+1;
				next_id++;
			}
			candidates.clear();
			find_abbr_in_range(query, range, candidates);
			if(!candidates.empty())
			{
				for(int i = 0; i < candidates.size(); i++)
				{
					string out_str = main_cmd + " " + candidates[i];
					if(result.find(out_str) == result.end())
					{
						result.insert(out_str);
						output.push_back(out_str);
					}
				}
			}
			it++;
		}
		if(level <= 2) return output;
		// 3. search in usage return the main command whose usage contain query string
		it = allmain_cmds.begin();
		while(it != allmain_cmds.end())
		{
			string main_cmd = (*it).first;
			ostringstream oss;
			for(int i = 0; i < usage_funcs.size(); i++)
			{
				if((*(usage_funcs[i]))(main_cmd, oss))
				{
					string help_str = oss.str();
					if(help_str.find(query) != string::npos)
					{
						string out_str = "-help " + main_cmd.substr(1, main_cmd.size());
						if(result.find(out_str) == result.end())
						{
							result.insert(out_str);
							output.push_back(out_str);
						}
					}
					break;
				}
			}
			it++;
		}
		if(level <= 3) return output;
		return output;
	}
	// oct 19, find which run_func
	int get_run_func_id()
	{
		string main_cmd = get_main_cmd();
		if(main_cmd.empty()) main_cmd = "+";
		main_cmd[0] = '+';
		int i = 0;
		for(; i < attributes.size(); i++)
		{
			const Attribute* _attrs = attributes[i];
			int n = attrs_num[i];
			for(int j = 0; j < n; j++)
			{
				if(_attrs[j].para_name == main_cmd) {return i;}
			}
		}
		return -1;
	}
	virtual bool run(int argc, char * argv[], int arg_begin = 1)
	{
		int rt = parse(argc, argv, arg_begin);
		if(rt != 1) return false;

		int i = get_run_func_id();
		if(i == -1) return false;

		rt = (*run_funcs[i])((*this));
		if(rt == -1) return -1;
		else if(rt == 0)
		{
			string main_cmd = get_main_cmd();
			cout<<endl;
			print_usage(main_cmd, cout);
			return false;
		}
		else return true;
	}

	virtual void list()
	{
		int j=1;
		for(int i = 0; i < attrs.size(); i++)
		{
			string cmd_name = attrs[i].para_name;
			if(cmd_name == "+") continue;
			if(cmd_name[0] == '+')
			{
				cmd_name[0] = '-';
				cout<<j++<<" : "<<cmd_name<<endl;
			}
		}
	}
	// get all main cmds
	vector<string> get_cmd_list()
	{
		vector<string> all_cmds;
		for(int i = 0; i < attrs.size(); i++)
		{
			string cmd_name = attrs[i].para_name;
			if(cmd_name == "+") continue;
			if(cmd_name[0] == '+')
			{
				cmd_name[0] = '-';
				all_cmds.push_back(cmd_name);
			}
		}
		return all_cmds;
	}
	// get the parent list for main command
	vector<string> get_cmd_para_list(string main_cmd)
	{
		int main_id = allmain_cmds[main_cmd];
		int i = main_id;
		vector<string> para_list;
		while(++i < attrs.size() && attrs[i].para_name[0] != '+')
		{
			para_list.push_back(attrs[i].para_name);
		}
		/*for(map<string, int>::iterator it=allglobal_opts.begin(); it != allglobal_opts.end(); it++)
		{
			para_list.push_back(it->first);
		}*/

		return para_list;
	}

	void save_to_history(int argc, char ** argv)
	{
		int arg_begin = 1;      // switch ind
		bool is_abbr_exist = false;
		vector<string> args;

		string main_cmd = get_main_cmd();
		if(main_cmd == "-demo" || main_cmd == "-help" || main_cmd == "-list" || main_cmd == "-search" || main_cmd == "-history" || main_cmd == "-make-ano" || main_cmd == "-make-alias" || main_cmd == "-info") return;
		if(!abbr2full(argc, argv, arg_begin, args, is_abbr_exist)) return;

		string pwd = getenv ("PWD");
		string home = getenv("HOME");
		string history = home + "/.swc2mask_history";
		string log = home + "/.swc2mask_log";

		ofstream ofs1(history.c_str(),ios_base::app);
		ofstream ofs2(log.c_str(),ios_base::app);
		if(ofs1.fail() || ofs2.fail()) return;
		string local_time; GET_LOCAL_TIME_STR(local_time);
		ofs1<<args[0];
		ofs2<<local_time<<" "<<args[0];
		for(int i = 1; i < argc; i++)
		{
			if(args[i][0] != '-' && args[i][0] != '/')
			{
				string file = args[i], ft = file_type(file);
				if(ft != "unknown" && !is_number(ft) && file.find("http://") != 0 && file.find("ftp://") != 0) args[i] = pwd + "/" + file; 
			}
			ofs1<<" "<<args[i];
			ofs2<<" "<<args[i];
		}
		ofs1<<endl;
		ofs2<<endl;
		ofs1.close();
		ofs2.close();
	}
};

#endif
