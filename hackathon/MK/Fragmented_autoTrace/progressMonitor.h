#ifndef PROGRESSMONITOR_H
#define PROGRESSMONITOR_H

#include <iostream>

#include "ui_progressMonitor.h"

class ProgressMonitor : public QDialog
{
	Q_OBJECT

public:
	ProgressMonitor(bool& terminationSwitch);

private:
	Ui::progressDlg* progressDlgPtr;
};

#endif