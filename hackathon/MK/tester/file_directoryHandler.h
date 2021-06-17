#ifndef FILE_DIRECTORYHANDLER_H
#define FILE_DIRECTORYHANDLER_H

#include <iostream>
#include <vector>
#include <string>

#include <qstring.h>
#include <qfile.h>
#include <qdir.h>
#include <qdebug.h>

using namespace std;

class File_DirectoryHandler
{
public:
	vector<QString> getFiles(QString path, QString filter = "none");
};




#endif