#include "QProgressSender.h"

#ifdef WITH_QT
ts::QProgressSender::QProgressSender() : QWidget(0) {}


/**********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "instance(...)"
***********************************************************************************/
ts::QProgressSender* ts::QProgressSender::instance()
{
	static QProgressSender* uniqueInstance = new QProgressSender();
	return uniqueInstance;
}

#endif