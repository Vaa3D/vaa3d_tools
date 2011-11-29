#include <QtGui>
#include "../finetuningdialog.h"

int main(int argc, char* argv[])
{
	QApplication* a = new QApplication(argc, argv);
	FineTuningDialog* fine_tuning_dlg = new FineTuningDialog();
	fine_tuning_dlg->setModal(true);
	fine_tuning_dlg->exec();
	//return a->exec();
	return 0;
}
