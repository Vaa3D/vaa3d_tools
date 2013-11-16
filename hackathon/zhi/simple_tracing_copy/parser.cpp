#include "parser.h"
#include <string.h>

int split(const char *paras, char ** &args)
{
	int argc = 0;
	int len = strlen(paras);
	int posb[200];
	char * myparas = new char[len];
	strcpy(myparas, paras);
	for(int i = 0; i < len; i++)
	{
		if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
		{
			posb[argc++]=i;
		}
		else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && 
				(myparas[i] != ' ' && myparas[i] != '\t'))
		{
			posb[argc++] = i;
		}
	}

	args = new char*[argc];
	for(int i = 0; i < argc; i++)
	{
		args[i] = myparas + posb[i];
	}

	for(int i = 0; i < len; i++)
	{
		if(myparas[i]==' ' || myparas[i]=='\t')myparas[i]='\0';
	}
	return argc;
}

ParaParser::ParaParser(string argv, string optstring)
{
	cur_switch_id = -1; 
	if(!set_optstring(optstring))s_error += string("set_optstring error");
	if(!set_args(argv))s_error += "set_argv error";
}

ParaParser::ParaParser(string argv, string* optstrings, int optnum)
{
	cur_switch_id = -1; 
	if(!set_optstring(optstrings, optnum))s_error += string("set_optstring error");
	if(!set_args(argv))s_error += "set_argv error";
}
ParaParser::ParaParser(int argc, char* argv[], string optstring)
{
	cur_switch_id = -1; 
	if(!set_optstring(optstring))s_error += "set_optstring error";
	if(!set_args(argc,argv,0))s_error += "set_argv error";
}

ParaParser::ParaParser(int argc, char* argv[], string * optstrings, int optnum)
{
	cur_switch_id = -1; 
	if(!set_optstring(optstrings, optnum))s_error += "set_optstring error";
	if(!set_args(argc,argv,0))s_error += "set_argv error";
}

bool ParaParser::set_optstring(string options[], int n)
{
	switch_options.clear();
	if(n <= 0) return false;
	for(int i = 0; i < n; i++)
	{
		string option = options[i];
		bool is_need_arg = false;
		if(option.at(option.size() - 1) == ':')
		{
			option = option.substr(0, option.size()-1);
			is_need_arg = true;
		}
		SwitchOption so;
		so.switch_name = option;
		so.is_need_arg = is_need_arg;
		switch_options.push_back(so);
	}
	return true;
}

bool ParaParser::set_optstring(string optstring)
{
	switch_options.clear();
	int i = 0; 
	while(i < optstring.size())
	{
		char c = optstring.at(i);
		if(!(c >= 'A' && c <= 'Z') && !(c >= 'a' && c <= 'z'))
		{
			s_error += string("illegal char : ") + c;
			i++;
			continue;
		}
		if(i+1 < optstring.size() && optstring.at(i+1) == ':')
		{
			SwitchOption switch_option = {string("-") + c, 1};
			switch_options.push_back(switch_option);
			i+=2;
		}
		else 
		{
			SwitchOption switch_option = {string("-") + c, 0};
			switch_options.push_back(switch_option);
			i++;
		}

	}
	return true;
}

bool ParaParser::set_args(int argc, char* argv[], int start_arg_id)
{
	if(argc < 0){s_error += "illegal arg number"; return false;}
	if(start_arg_id < 0 || start_arg_id > argc - 1) {s_error += "invalid start_arg_id"; return false;}

	filelist.clear();
	switchlist.clear();

	int i = start_arg_id;      // switch ind

	while(i < argc)
	{
		Switch para;
		if(argv[i][0] != '-') filelist.push_back(string(argv[i]));
		else if(is_support(argv[i]) && !is_exist(argv[i]))
		{
			if(!is_need_arg(argv[i])) add_option(argv[i]);
			else if(is_need_arg(argv[i]) && i+1 < argc) {add_option(argv[i], argv[i+1]); i++;}
			else {s_error += string("need parameter for ") + argv[i]; return false;}
		}
		else if(!is_support(argv[i]))
		{
			s_error += string(argv[i]) + " is not supported";
			return false;
		}
		else if(is_exist(argv[i]))
		{
			s_error += string("duplicated para ") + argv[i];
			return false;
		}
		else
		{
			s_error += "what is this paramter";
			return false;
		}
		i++;
	}
	return true;
}

bool ParaParser::set_args(string argv, int start_arg_id)
{
	int argc; 
	char ** args = 0;
	argc = split(argv.c_str(), args);
	return set_args(argc, args, start_arg_id);
}
bool ParaParser::error()
{
	if(s_error == "") return false;
	else return true;
}

void ParaParser::print_error()
{
	/*list<string>::iteraotr it = s_error.begin();
	while(it != s_error.end())
	{
		cout<<*it<<endl;
		it++;
	}*/
	cout<<s_error<<endl;
}

bool ParaParser::is_support(string switch_name)
{
	if(switch_options.empty()) return false;
	int cmd_num = switch_options.size();
	for(int i = 0; i < cmd_num; i++)
	{
		if(switch_options[i].switch_name == switch_name) return true;
	}
	return false;
}

bool ParaParser::is_need_arg(string switch_name)
{
	if(!is_support(switch_name)) return false;
	int cmd_num = switch_options.size();
	for(int i = 0; i < cmd_num; i++)
	{
		if(switch_options[i].switch_name == switch_name) return switch_options[i].is_need_arg;
	}
}

bool ParaParser::get_option(string & optname, string & optarg)
{
	if(switchlist.empty()) return false;
	if(cur_switch_id < -1) cur_switch_id = -1;
	if(cur_switch_id >= (int) switchlist.size() - 1 ){optname=""; return false;}
	cur_switch_id++; 
	optname = switchlist[cur_switch_id].switch_name;
	optarg = switchlist[cur_switch_id].switch_arg;
	return true;
}

string ParaParser::get_optarg(string switch_name)
{
	int order = switch_order(switch_name);
	if(order == -1) return string("");
	return switchlist[order].switch_arg;
}


int ParaParser::get_optarg_int(string switch_name)
{
	if(!is_support(switch_name) || !is_exist(switch_name)){s_error += string("unsupported or unexisted switch ") +  switch_name; return -1;}
	return atoi(get_optarg(switch_name).c_str());
}

int ParaParser::get_optarg_int(string switch_name, int index, string sep)
{
	if(!is_support(switch_name) || !is_exist(switch_name)){s_error += string("unsupported or unexist para ") + switch_name; return -1;}
	string par_str = get_optarg(switch_name);
	size_t sp=0, ep=par_str.find_first_of(sep);
	if(index == 0)
	{
		if(ep == string::npos) {return atoi(par_str.c_str());}
		else if(ep == 0){return -1;}
		else {return atol(par_str.substr(sp, ep - sp).c_str());}
	}
	while(index>0)
	{
		if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
		else {s_error += string("index exceed the number of ") + sep; return -1;}
		index--;
	}
	if(ep == string::npos) ep = par_str.size();
	return atol(par_str.substr(sp, ep - sp).c_str());
}

double ParaParser::get_optarg_double(string switch_name)
{
	if(!is_support(switch_name) || !is_exist(switch_name)){s_error += "unsupported or unexist para ";s_error += switch_name; return -1.0;}
	return atof(get_optarg(switch_name).c_str());
}

double ParaParser::get_optarg_double(string switch_name, int index, string sep)
{
	if(!is_support(switch_name) || !is_exist(switch_name)){s_error += "unsupported or unexist para"; return -1.0;}
	string par_str = get_optarg(switch_name);
	size_t sp=0, ep=par_str.find_first_of(sep);
	if(index == 0)
	{
		if(ep == string::npos) {return atof(par_str.c_str()); }
		else if(ep == 0){ return 0.0;}
		else {return atof(par_str.substr(sp, ep - sp).c_str()); }
	}
	while(index>0)
	{
		if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
		else {s_error += "index exceed the number of "; s_error += sep; return -1.0;}
		index--;
	}
	if(ep == string::npos) ep = par_str.size();
	return atof(par_str.substr(sp, ep - sp).c_str());
}

