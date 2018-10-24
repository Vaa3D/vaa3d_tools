#ifndef __SIMPLE_C_CPP__
#define __SIMPLE_C_CPP__

#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;


bool is_lowercase(char c)
{
	return (c >= 'a' && c <= 'z');
}
bool is_number(string str)
{
	bool isnum = true;
	int i = 0;
	if(str[0] == '-' || str[0] == '+') i++;
	int dot_num = 0;
	for(; i < str.size(); i++)
	{
		if(str[i] == '.'){dot_num++; continue;}
		else if(!isdigit(str[i])){isnum = 0; break;}
	}
	if(dot_num >=2) return false;
	return isnum;
}

// file_type("test.tif") == ".tif"
string file_type(string para)
{
    int pos = para.find_last_of(".");
    if(pos == string::npos) return string("unknown");
    else return para.substr(pos, para.size() - pos);
}

// basename("test.tif") == "test"
string basename(string para)
{
	int pos = para.find_last_of(".");
	if(pos == string::npos) return para;
	else return para.substr(0, pos);
}

bool is_marker_file(string para)
{
	string right7 = (para.size() >= 7) ? para.substr(para.size() -7, 7) : "";
	if(right7 == ".marker") return true;
	else return false;
}

bool is_img_file(string para)
{
	string right4 = (para.size() >= 4) ? para.substr(para.size() -4, 4) : "";
	string right5 = (para.size() >= 5) ? para.substr(para.size() -5, 5) : "";

	if(right4 == ".ppm" || right4 == ".bmp" || right4 == ".raw" || right4 == ".tif" || right4 == ".lsm" || right5 == ".tiff") return true;
	else return false;
}

int common_prefix(string str1, string str2)
{
	string short_str = str1;
	string long_str = str2;
	if(str1.size() > str2.size())
	{
		short_str = str2;
		long_str = str1;
	}
	int i = 0;
	while(i  < short_str.size() && short_str.at(i) == long_str.at(i)) i++;
	return i;
}

string trim_border(string str, char sep = ' ')
{
	while(str[0] == sep)
	{
		str = str.substr(1, str.size()-1);
	}
	while(str[str.size()-1] == sep)
	{
		str = str.substr(0, str.size()-1);
	}
	return str;
}

string trim_dump(string str, char sep = ' ')
{
	string sep2; 
	sep2 += sep;
	sep2 += sep;
	int pos = 0;
	while((pos = str.find(sep2)) != string::npos)
	{
		string str1 = str.substr(0, pos);
		string str2 = str.substr(pos + 1, str.size() - pos -1);
		str = str1 + str2;
	}
	return str;
}

vector<string> strsplit(string str, char sep)
{
	str = trim_border(str, sep);
	str = trim_dump(str,sep);
	vector<string> splits;
	string split;
	int pos;
	while((pos = str.find(sep)) != string::npos)
	{
		split = str.substr(0, pos);
		splits.push_back(split);
		str=str.substr(pos+1, str.size() -pos -1);
	}
	splits.push_back(str);
	return splits;
}

#endif
