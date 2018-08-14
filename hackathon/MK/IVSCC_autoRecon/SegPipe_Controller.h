#ifndef SEGPIPE_CONTROLLER_H
#define SEGPIPE_CONTROLLER_H

#include <string>
#include <vector>

#include <qstring.h>
#include <qstringlist.h>

using namespace std;

class SegPipe_Controller
{
public:
	/***************** Constructors and Basic Data Members *****************/
	SegPipe_Controller(QString inputPath, QString outputPath);

	QString inputCaseRootPath;
	QString outputRootPath;
	QStringList caseList;
	/***********************************************************************/

	void sliceDownSample2D(int downFactor, string method = "");
};

#endif