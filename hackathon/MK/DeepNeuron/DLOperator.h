#ifndef DLOPERATOR_H
#define DLOPERATOR_H

#ifndef BOOST_DATE_TIME_NO_LIB
#define BOOST_DATE_TIME_NO_LIB
#endif

#ifndef BOOST_FILESYSTEM_NO_LIB
#define BOOST_FILESYSTEM_NO_LIB
#endif

#ifndef BOOST_SYSTEM_NO_LIB
#define BOOST_SYSTEM_NO_LIB
#endif

#include "v3d_interface.h"
#include "imgOperator.h"

class DLOperator : public QObject
{
	Q_OBJECT

signals:
	void progressBarReporter(QString taskName, int percentage);

public:
	DLOperator();
	DLOperator(imgOperator*);

	string deployName;
	string modelName;
	string meanName;

	void predictSWCstroke(V3DPluginCallback2*&, v3dhandle);

private:
	V3DPluginCallback2* mainCallBack;	
	imgOperator* curImgOperator;
};

#endif