#include "utils.h"

void splitpath(string path, char* fname, char* ext)
{
  string nameWithExt =  path.substr(path.find_last_of("/\\")+1);
  int dotIdx = nameWithExt.find_last_of(".");
  string nameWithoutExt = nameWithExt.substr(0,dotIdx);
  strcpy(ext, nameWithoutExt.c_str());

  string sExt = nameWithExt.substr(0,dotIdx);
  strcpy(ext, sExt.c_str());
}

/*
string getNameFromPathWithoutExtension(string path){
  string nameWith =  path.substr(path.find_last_of("/\\")+1);
  string nameWithout = nameWith.substr(0,nameWith.find_last_of("."));
  return nameWithout;
}

string getExtension(string path){
  return path.substr(path.find_last_of(".")+1);
}
*/
