#ifndef Q_PROGRESS_SENDER_H
#define Q_PROGRESS_SENDER_H

#ifdef WITH_QT
#include <QWidget>
#include "config.h"
#include "ProgressBar.h"

namespace terastitcher
{
	class QProgressSender : public QWidget
	{
		public:

			Q_OBJECT

		private:

			// disable default constructor
			QProgressSender();

		public:

			/**********************************************************************************
			* Singleton design pattern: this class can have one instance only,  which must be
			* instantiated by calling static method "instance(...)"
			***********************************************************************************/
			static QProgressSender* instance();
			friend class ProgressBar;

		signals:

			/*********************************************************************************
			* Carries progress bar informations (progress percentage and remaining minutes).
			**********************************************************************************/
			void sendProgressBarChanged(int val, int minutes, int seconds, std::string message);
	};
}
#endif

#endif