#ifndef __PARAPARSER_H_H_
#define __PARAPARSER_H_H_

#include <vector>
#include <list>
#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;
struct SwitchOption
{
	string switch_name;
	bool is_need_arg;
};

struct Switch
{
	string switch_name;          // -resize
	string switch_arg;
	Switch(){switch_name=""; switch_arg=""; }
	Switch(string name, string str){switch_name=name; switch_arg=str;}
};

template<class T> vector<T>& operator +=(vector<T>& the_list, T & item)
{
	the_list.push_back(item);
	return the_list;
}

class ParaParser
{
public:
	ParaParser(){cur_switch_id = -1;}
	ParaParser(string argv, string optstring);
	ParaParser(string argv, string * optstrings, int optnum);
	ParaParser(int argc, char* argv[], string optstring);  // argv[0] is para not prgram name
	ParaParser(int argc, char* argv[], string* optstrings, int optnum);  // argv[0] is para not prgram name

	bool set_optstring(string optstring);
	bool set_optstring(string optstrings[], int n);

	bool set_args(int argc, char* argv[], int start_arg_id = 0);
	bool set_args(string argv, int start_arg_id = 0);

	bool   get_option(string & optname, string & optarg);
	string get_optarg(string switch_name);
	int    get_optarg_int(string switch_name);
	int    get_optarg_int(string switch_name, int index, string sep = "x");
	double get_optarg_double(string switch_name);
	double get_optarg_double(string switch_name, int index, string sep = "x");

	bool error();
	void print_error();
private:
	int cur_switch_id;
	vector<string> filelist;
	vector<Switch> switchlist;
	vector<SwitchOption> switch_options;
	string s_error;

	
	bool is_support(string switch_name);
	
	bool is_need_arg(string switch_name);

	int switch_order(string switch_name)
	{
		if(switchlist.empty()) return -1;
		for(int i = 0; i < switchlist.size(); i++)
		{
			if(switchlist[i].switch_name == switch_name) return i;
		}
		return -1;
	}
	bool is_empty()
	{
		return switchlist.empty();
	}
	bool is_exist(string switch_name)
	{
		if(switch_order(switch_name) != -1) return true;
		return false;
	}
	
	int get_delim_num(string switch_name, string delim="x")
	{
		int count = 0;
		if(!is_support(switch_name) || !is_exist(switch_name)) return 0;
		string str = get_optarg(switch_name);
		size_t found = str.find_first_of(delim);
		while(found != string::npos)
		{
			count++;
			found = str.find_first_of(delim, found + 1);
		}
		return count;
	}
	void add_option(string switch_name, string switch_arg){
		switchlist.push_back(Switch(switch_name, switch_arg));
	}
	void add_option(string switch_name){
		switchlist.push_back(Switch(switch_name, ""));
	}
	void add_option(Switch para){
		switchlist.push_back(para);
	}
};

#endif
