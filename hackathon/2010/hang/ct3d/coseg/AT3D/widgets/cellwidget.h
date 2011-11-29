#ifndef CELL_WIDGET_H_H
#define CELL_WIDGET_H_H

#include "../../CT3D/cell_track.h"

#include <QtGui>
#include <vector>

using namespace std;

/*********************************************************
 * CellWidget
 * Usage:
 * CellWidget* cellwidget = new CellWidget;
 * vector<CellTrack::Cell*> visable_cells = controller->getFrame(current_time)->getCells();
 * vector<CellTrack::Cell*> marked_cells = controller->getMarkedCells();
 * cellwidget->setCells(visable_cells, marked_cells);
 * *******************************************************/

class CellWidget : public QWidget
{
	Q_OBJECT

public:
	CellWidget(QWidget* parent = NULL);
	void orderCells(int orderby = 0);
	void setCells(vector<CellTrack::Cell*> visable_cells, vector<CellTrack::Cell*> marked_cells);
	void clearCells();
	void setLayoutItems(int num);
	void setCellChecked(CellTrack::Cell* cell, bool check_state);
public slots:
	void onCellChecked(int state);
signals:
	void cellChecked(CellTrack::Cell* cell);

private:
	int m_numLayoutItems;  // num checkers and editors
	vector<QCheckBox*> m_checkers;
	vector<QTextEdit*> m_editors;
	vector<CellTrack::Cell*> m_cells;
};

#endif
