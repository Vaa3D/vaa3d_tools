#include <QtGui>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <cassert>

#include "widgets/tablewidget.h"
#include "dialogs/finetuningdialog.h"
#include "dialogs/createdialog.h"
#include "extends/cell_track_ex.h"
#include "extends/glwidget_ex.h"

#include "at3d_view.h"
using namespace std;

/************************************
 * variables
 ************************************/

/***********************************
 * Realize Member Functions According
 * to Their Declaration
 ***********************************/
AT3DVIEW::AT3DVIEW(QWidget* parent) : QWidget(parent), CellTrackController()
{
	//step 1 : setUi
	ui.setupUi(this);
	//ui.cellWidget->setAttribute(Qt::WA_DeleteOnClose, false);
	
	//step 2 : initialization
	
	m_glWidget = new GLWidget();
	connect(m_glWidget, SIGNAL(mouseClicked(float,float)), this, SLOT(onCellMarked(float, float)));
	m_cellWidget = new CellWidget();
	connect(m_cellWidget, SIGNAL(cellChecked(CellTrack::Cell* )), this, SLOT(onCellMarked(CellTrack::Cell*)));
	
	QLayout * layout = new QVBoxLayout();
	layout->addWidget(m_glWidget);
	ui.glGroupBox->setLayout(layout);

	ui.scrollArea->setWidget(m_cellWidget);	
}

CellTrack::Cell* AT3DVIEW::getClickedCell(float posX, float posY, float posZ)
{
	if(cell_centers.empty())
	{
		setCellCenters();
	}
	int w = this->getWidth();
	int h = this->getHeight();
	int d = this->getDepth();
	float min_dist = (float)INT_MAX;
	CellTrack::Cell* obj_cell = NULL;
	vector<CellTrack::Cell*> cells = celltrack->getFrame(current_time)->getCells();
	vector<CellTrack::Cell*>::iterator it = cells.begin();
	while(it != cells.end())
	{
		vector<float>& center = cell_centers[*it];
		double center_x = center[0];
		double center_y = center[1];
		double center_z = center[2];
		double winX, winY, winZ;
		this->m_glWidget->getProjection(winX, winY, winZ, center_x, center_y, center_z);
		winZ = 0.0;
		float dist = (posX - winX)*(posX - winX) + 
			(posY - winY)*(posY - winY) + 
			(posZ - winZ)*(posZ - winZ);
		if(dist < min_dist)
		{
			min_dist = dist;
			obj_cell = *it;
		}
		it++;
	}
	return obj_cell;
}

/***************************************************
 * onOpen : open data
 ***************************************************/

void AT3DVIEW::onOpen()
{
	CreateCellTrackDialog* createdlg = new CreateCellTrackDialog();
	CellTrackEX* cell_track_ex = new CellTrackEX();
	createdlg->setCellTrack(cell_track_ex);
	createdlg->setModal(true);
	if(createdlg->exec() == QDialog::Accepted && createdlg->getCellTrack() != NULL)
	{
		celltrack = createdlg->getCellTrack();
		this->initTracksState(); // important
		current_time = 0;
		// 1. set m_glWidget
		unsigned char* img = getTexData();
		m_glWidget->loadTexture(img, this->getWidth(), this->getHeight(), this->getDepth(),3);
		m_glWidget->updateGL();

		// 2. set m_cellWidget
		m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
	}
}

/************************************************
 * SLot Functions Begin
 ************************************************/

//File Group
void AT3DVIEW::onLoadResult()
{
}

void AT3DVIEW::onSaveFrames()
{
	if(celltrack == NULL || celltrack->frameNum() == 0 )	return;
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	if(dir == "") return;
	else celltrack->exportImages("", (char*)dir.toStdString().c_str());
	QMessageBox::information(this,"Save Frames", "Successfully Saved!");
}
//Edit Group
void AT3DVIEW::onApplyFilter()
{
}

void AT3DVIEW::onFilter()
{
}

void AT3DVIEW::onSetColor()
{
}

void AT3DVIEW::onSummary()
{
}

void AT3DVIEW::onSpeed()
{
	if(celltrack == NULL || celltrack->frameNum() == 0 )	return;
	int track_num = celltrack->trackNum();
	int frame_num = celltrack->frameNum();

	int rows = celltrack->trackNum();
	int columns = celltrack->frameNum() +  2 ;
	
	TableWidget* twidget = new TableWidget();
	twidget->show();
	QTableWidget* tableWidget = twidget->getTableWidget() ;//= new QTableWidget(rows, columns, NULL);
	tableWidget->setRowCount(rows);
	tableWidget->setColumnCount(columns);

	QTableWidgetItem *item;
	QStringList strList;
	QString str;
	str = QObject::tr("trackId");
	strList<<str;
	for(int column = 1; column <= frame_num; column++)
	{
		str = QObject::tr("->%1").arg(column);
		strList<<str;
	}
	strList<<"meanSpeed";
	tableWidget->setHorizontalHeaderLabels(strList);
	
	
	for(int row = 0 ; row < rows; row++)
	{
		int frame_id = 0;
		item = new QTableWidgetItem(QObject::tr("%1").arg(row+1,(int)log10(track_num)+1,10,QChar('0')));
		tableWidget->setItem(row, 0 ,item);
		CellTrack::Track* track = celltrack->getTrack(row);
		float sum_distance = 0.0;
		float cx1 = 0.0 , cy1 = 0.0 , cz1 = 0.0;
		float cx2 = 0.0 , cy2 = 0.0 , cz2 = 0.0;
		CellTrack::Cell* cell = track->getStartCell();
		cell->getCenter(cx1,cy1,cz1);
		for(frame_id = track->startTime(); frame_id < frame_num; frame_id++)
		{
			if(frame_id == track->startTime())
			{
				item = new QTableWidgetItem(QObject::tr("%1(%2,%3,%4)").arg(0.0f,4,'f',2,QChar('0')).arg(cx1,4,'f',2,QChar('0')).arg(cy1,4,'f',2,QChar('0')).arg(cz1,4,'f',2,QChar('0')));
				tableWidget->setItem(row,frame_id +1,item);
				continue;
			}
			cell = cell->getNextCell();
			if(cell == NULL) break;
			cell->getCenter(cx2,cy2,cz2);
			float distance = sqrt((cx2 - cx1)*(cx2 - cx1) + (cy2 - cy1)*(cy2 - cy1) + (cz2 - cz1)*(cz2 - cz1));
			sum_distance += distance;
			item = new QTableWidgetItem(QObject::tr("%1(%2,%3,%4)").arg(distance,4,'f',2,QChar('0')).arg(cx2,4,'f',2,QChar('0')).arg(cy2,4,'f',2,QChar('0')).arg(cz2,4,'f',2,QChar('0')));
			tableWidget->setItem(row,frame_id +1,item);
			cx1 = cx2; cy1 = cy2; cz1 = cz2;
		}
		item = new QTableWidgetItem(QObject::tr("%1").arg(sum_distance/(frame_id - track->startTime()-1),4,'f',2,QChar('0')));
		tableWidget->setItem(row,frame_num+1,item);
	}
	tableWidget->setWindowTitle("Speed");
	tableWidget->setSortingEnabled(true);
	tableWidget->sortByColumn(0,Qt::AscendingOrder);
	//tableWidget->show();

}

void AT3DVIEW::onVolume()
{
	if(celltrack == NULL || celltrack->frameNum() == 0 )	return;
	int track_num = celltrack->trackNum();
	int frame_num = celltrack->frameNum();

	int rows = celltrack->trackNum();
	int columns = celltrack->frameNum() +  2 ;
	
	TableWidget* twidget = new TableWidget();
	twidget->show();
	QTableWidget* tableWidget = twidget->getTableWidget() ;//= new QTableWidget(rows, columns, NULL);
	tableWidget->setRowCount(rows);
	tableWidget->setColumnCount(columns);

	QTableWidgetItem *item;
	QStringList strList;
	QString str;
	str = QObject::tr("trackId");
	strList<<str;
	for(int column = 1; column <= frame_num; column++)
	{
		str = QObject::tr("%1").arg(column);
		strList<<str;
	}
	strList<<"meanVolume";
	tableWidget->setHorizontalHeaderLabels(strList);
	
	
	for(int row = 0 ; row < rows; row++)
	{
		int frame_id = 0;
		item = new QTableWidgetItem(QObject::tr("%1").arg(row+1,(int)log10(track_num)+1,10,QChar('0')));
		tableWidget->setItem(row, 0 ,item);
		CellTrack::Track* track = celltrack->getTrack(row);
		CellTrack::Cell* cell = track->getStartCell();
		float volume_sum = 0;
		for(frame_id = track->startTime(); frame_id < frame_num; frame_id++)
		{
			if(cell == NULL) break;
			volume_sum += cell->getVolume();
			item = new QTableWidgetItem(QObject::tr("%1").arg(cell->getVolume(),4,'f',2,QChar('0')));
			tableWidget->setItem(row,frame_id +1,item);
			cell = cell->getNextCell();
		}
		item = new QTableWidgetItem(QObject::tr("%1").arg(volume_sum/(frame_id - track->startTime()),4,'f',2,QChar('0')));
		tableWidget->setItem(row,frame_num+1,item);
	}
	tableWidget->setWindowTitle("Volume");
	tableWidget->setSortingEnabled(true);
	tableWidget->sortByColumn(0,Qt::AscendingOrder);
	
}

void AT3DVIEW::onDeformation()
{
	if(celltrack == NULL || celltrack->frameNum() == 0 )	return;
	int track_num = celltrack->trackNum();
	int frame_num = celltrack->frameNum();

	int rows = celltrack->trackNum();
	int columns = celltrack->frameNum() +  2 ;
	
	TableWidget* twidget = new TableWidget();
	twidget->show();
	QTableWidget* tableWidget = twidget->getTableWidget() ;//= new QTableWidget(rows, columns, NULL);
	tableWidget->setRowCount(rows);
	tableWidget->setColumnCount(columns);

	QTableWidgetItem *item;
	QStringList strList;
	QString str;
	str = QObject::tr("trackId");
	strList<<str;
	for(int column = 1; column <= frame_num; column++)
	{
		str = QObject::tr("->%1").arg(column);
		strList<<str;
	}
	strList<<"meanDeform";
	tableWidget->setHorizontalHeaderLabels(strList);
	
	
	for(int row = 0 ; row < rows; row++)
	{
		int frame_id = 0;
		item = new QTableWidgetItem(QObject::tr("%1").arg(row+1,(int)log10(track_num)+1,10,QChar('0')));
		tableWidget->setItem(row, 0 ,item);
		CellTrack::Track* track = celltrack->getTrack(row);
		float accum_deform = 1.0;
		float deform = 1.0;
		CellTrack::Cell* prev_cell = track->getStartCell();
		item = new QTableWidgetItem(QObject::tr("%1").arg(1.0f,4,'f',2,QChar('0')));
		tableWidget->setItem(row,track->startTime() +1,item);
		for(frame_id = track->startTime() + 1; frame_id < frame_num; frame_id++)
		{
			CellTrack::Cell* next_cell = prev_cell->getNextCell();
			if(next_cell == NULL) break;
			int overlap = prev_cell->getOverlap(next_cell);
			deform = prev_cell->getSize() + next_cell->getSize() - 2*overlap;
			deform /= (prev_cell->getSize() + 2* overlap + next_cell->getSize())/2;// + next_cell->getSize() - overlap;
			accum_deform *= deform;
			item = new QTableWidgetItem(QObject::tr("%1").arg(deform,4,'f',2,QChar('0')));
			tableWidget->setItem(row,frame_id +1,item);
			prev_cell = next_cell;
		}
		item = new QTableWidgetItem(QObject::tr("%1").arg(pow((double)accum_deform,(double)(1.0/(frame_id - track->startTime()-1))),4,'f',2,QChar('0')));
		tableWidget->setItem(row,frame_num+1,item);
	}
	tableWidget->setWindowTitle("Deformation");
	tableWidget->setSortingEnabled(true);
	tableWidget->sortByColumn(0,Qt::AscendingOrder);
	
}

void AT3DVIEW::onTrajectory()
{
	if(celltrack == NULL || celltrack->frameNum() == 0 )	return;
	GLWidgetEX* glwidget_ex = new GLWidgetEX(NULL);
	//GLWidget* glwidget_ex = new GLWidget(NULL);
	int w = celltrack->getFrame(0)->width();
	int h = celltrack->getFrame(0)->height();
	int d = celltrack->getFrame(0)->depth();
	glwidget_ex->setWidgetSize(w,h,d);
	vector<CellTrack::Track*> tracks = getMarkedTracks();
	vector<CellTrack::Track*>::iterator it = tracks.begin();
	while(it != tracks.end())
	{
		CellTrack::Track* track = *it;
		int color = track->getColor();
		vector<float> trajectory;
		CellTrack::Cell* cell = track->getStartCell();
		while(cell != NULL)
		{
			float cx, cy, cz;
			cell->getCenter(cx,cy,cz);
			trajectory.push_back(cx);
			trajectory.push_back(cy);
			trajectory.push_back(cz);
			cell = cell->getNextCell();
		}
		glwidget_ex->addTrajectory(trajectory, color);
		it++;
	}
	glwidget_ex->show();
	glwidget_ex->updateGL();
}
//View Group
void AT3DVIEW::onNew3D()
{
}

void AT3DVIEW::onResetView()
{
}


void AT3DVIEW::onViewTree()
{
}

//Control Group
void AT3DVIEW::onFirst()
{
	if(celltrack == NULL || celltrack->frameNum() == 0 )	return;
	setFirst();
	ui.glGroupBox->setTitle(tr("Time : %1").arg(current_time + 1));
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

void AT3DVIEW::onLast()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	setLast();
	ui.glGroupBox->setTitle(tr("Time : %1").arg(current_time + 1));
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

void AT3DVIEW::onPrevious()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	setPrev();
	ui.glGroupBox->setTitle(tr("Time : %1").arg(current_time + 1));
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

void AT3DVIEW::onNext()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	setNext();
	ui.glGroupBox->setTitle(tr("Time : %1").arg(current_time + 1));
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

//Cell Widget
void AT3DVIEW::onReverse()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	  map<CellTrack::Track*, bool>::iterator it = tracks_state.begin();
	  while(it != tracks_state.end())
	  {
		  (*it).second = 1 - (*it).second;
		  it++;
	  }
	  m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	  m_glWidget->updateGL();
	  m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

void AT3DVIEW::onUndo()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	if(history.empty()) return;
	undo();
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}
/************************************************
 * When checkbox are checked and clicked the choose button
 ************************************************/
void AT3DVIEW::onChoose()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	choose();
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

void AT3DVIEW::onDelete()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	remove();
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

/************************************************
 * SLot Functions End
 ************************************************/


/*******************************************
 * functions begins
 *******************************************/


void AT3DVIEW::clear()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	while(!history.empty())
	{
		CellTrack* ct = history.back();
		history.pop_back();
		ct->releaseFrames();
	}
	if(celltrack != NULL)
	{
		celltrack->releaseFrames();
		celltrack->releaseTracks();
		celltrack->releaseAllCells();
	}
}

// signal flow : glwidget -> cellwidget -> AT3DVIEW
void AT3DVIEW::onCellMarked(float x, float y)
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	CellTrack::Cell* cell = this->getClickedCell(x,y,0.0);
	if(!tracks_state[cell->getTrack()])
	{
		m_cellWidget->setCellChecked(cell, true);
	}
	else 
	{
		m_cellWidget->setCellChecked(cell, false);
	}
	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();

}

// signal from cellwidget
void AT3DVIEW::onCellMarked(CellTrack::Cell* cell)
{
//	if(cell_centers.empty())
//	{
//		setCellCenters();
//	}
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
	if(!tracks_state[cell->getTrack()])
	{
		this->markCell(cell);
	}
	else 
	{
		unMarkCell(cell);
	}

	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	//m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
}

void AT3DVIEW::on_fineTuningButton_clicked()
{
	if(celltrack == NULL || celltrack->frameNum() == 0) return;
    vector<CellTrack::Cell*> cells = this->getMarkedCells();
    if(cells.size() != 1)
    {
            QMessageBox::information(this,"","Please choose only one cell!");
            return;
    }
    CellTrack::Cell* cell = cells[0];
	ComponentTree* tree = cell->getTree();
    int node_label = cell->getNodeLabel();
	cout<<"node_label = "<<node_label<<" size = "<<tree->getNode(node_label)->getBetaSize()<<endl;

    FineTuningDialog* dlg = new FineTuningDialog();
    dlg->setModal(true);
    dlg->setParameters(tree, node_label);
    if(dlg->exec() == QDialog::Accepted)
	{
	cell->setModNodeLabel(dlg->getLabel());
	cell->setVertices();
	cell->setCenterArea();

	m_glWidget->loadTexture(this->getTexData(), this->getWidth(), this->getHeight(), this->getDepth(),3);
	m_glWidget->updateGL();
	m_cellWidget->setCells(celltrack->getFrame(current_time)->getCells(), this->getMarkedCells());
	}
}
