#include "PTeraStitcher.h"
#include "ProgressBar.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
	//overriding the current locale with the standard POSIX locale
	setlocale(LC_ALL, "POSIX");

	//launch PTeraStitcher's GUI
	qRegisterMetaType<std::string>("std::string");
	terastitcher::ProgressBar::instance()->setToGUI(true);
	terastitcher::PTeraStitcher::instance(0);

	return a.exec();
}
