#include <QtGui>
#include "../createdialog.h"
#include "../../extends/cell_track_ex.h"

int main(int argc, char* argv[])
{
	QApplication* a = new QApplication(argc, argv);
	CreateCellTrackDialog* createdlg = new CreateCellTrackDialog();
	createdlg->setExportButton();
	CellTrackEX* cell_track = new CellTrackEX();
	createdlg->setCellTrack(cell_track);
	createdlg->setModal(true);
	createdlg->exec();
	//return a->exec();
	return 0;
}
