#ifndef __SIMPLE_C_H__
#define __SIMPLE_C_H__

#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

bool   is_lowercase(char c);
bool   is_number(string str);
string file_type(string para); // file_type("test.tif") == ".tif"
string basename(string para); // basename("test.tif") == "test"
bool   is_marker_file(string para);
bool   is_img_file(string para);

template<class T> string num2str(T value)
{
	std::ostringstream o;
	if (!(o << value))
		return "";
	return o.str();
}

template<class T> string num2str(T value, char fill, int wid)
{
	std::ostringstream o;
	o.fill(fill);
	o.width(wid);
	if (!(o << value))
		return "";
	return o.str();
}
template<class T> T str2num(string str)
{
	T value;
	std::istringstream iss(str);
	iss >> value;
	return value;
}

template<class T> T str2num(string str, T &value)
{
	std::istringstream iss(str);
	iss >> value;
	return value;
}
int common_prefix(string str1, string str2);

// trim the head and tail with sep
string trim_border(string str, char sep = ' ');
// trim duplicate char
string trim_dump(string str, char sep = ' ');

vector<string> strsplit(string str, char sep = ' ');

template<class T> vector<T> str2arr(string str, char sep = ' ')
{
	str = trim_border(str, sep);
	str = trim_dump(str,sep);
	vector<T> values;
	T value;
	int pos;
	while((pos = str.find(sep)) != string::npos)
	{
		istringstream iss(str.substr(0, pos));
		iss>>value;
		values.push_back(value);
		str=str.substr(pos+1, str.size() -pos -1);
	}
	istringstream iss(str);
	iss>>value;
	values.push_back(value);
	return values;
}


#endif
