#include "cellwidget.h"

#include <cassert>

QString getColorStr(unsigned int color);
QString hex2str(int v);

CellWidget::CellWidget(QWidget* parent) : QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	this->setLayout(layout);
	layout->setSpacing(10);
	m_numLayoutItems = 0;
}

void CellWidget::orderCells(int orderby)
{
	if(orderby == 0) // default, sort by trackId
	{
		int cell_num = m_cells.size();
		for(int i = 0; i < cell_num; i++)
		{
			int minId = i;
			for(int j = i; j < cell_num; j++)
			{
				if(m_cells[j]->getTrack()->trackId() < m_cells[minId]->getTrack()->trackId())
				{
					minId = j;
				}
			}
			CellTrack::Cell* temp = m_cells[i];
			m_cells[i] = m_cells[minId];
			m_cells[minId] = temp;
		}
	}
}
void CellWidget::setLayoutItems(int num)
{
	QVBoxLayout* layout = (QVBoxLayout*) this->layout();
	if(m_numLayoutItems < num)
	{
		m_checkers.resize(num);
		m_editors.resize(num);
		for(int i = m_numLayoutItems; i < num; i++)
		{
			QCheckBox* checker = new QCheckBox();
			QTextEdit* editor = new QTextEdit();
			editor->setReadOnly(true);
			editor->setFixedHeight(20);
			editor->setFixedWidth(120);
			QHBoxLayout* child_layout = new QHBoxLayout(); 
			child_layout->addWidget(checker);
			child_layout->addWidget(editor);
			layout->addLayout(child_layout);

			m_checkers[i] = checker;
			m_editors[i] = editor;
		}
	}
	else if(m_numLayoutItems > num)
	{
		for(int i = num ; i < m_numLayoutItems; i++)
		{
			m_checkers[i]->setHidden(true);
			//m_checkers[i]->setChecked(false);
			m_editors[i]->setHidden(true);
		}
	}
	m_numLayoutItems = num;
}
void CellWidget::setCellChecked(CellTrack::Cell* cell, bool check_state)
{
	vector<CellTrack::Cell*>::iterator it = m_cells.begin();
	int i = 0;
	while(it != m_cells.end())
	{
		if(cell == (CellTrack::Cell*)(*it))
		{
			break;
		}
		it++;
		i++;
	}
	m_checkers[i]->setChecked(check_state);
}

void CellWidget::clearCells()
{
	if(!m_cells.empty())
	{
		for(int i = 0; i < (int)m_cells.size(); i++)
		{
			disconnect(m_checkers[i],SIGNAL(stateChanged(int)), this, SLOT(onCellChecked(int)));
		}
		m_cells.clear();	
	}
}

void CellWidget::setCells(vector<CellTrack::Cell*> visable_cells, vector<CellTrack::Cell*> marked_cells)
{
	clearCells();
	m_cells = visable_cells;
	orderCells();
	int cell_num = m_cells.size();
	setLayoutItems(cell_num);
	set<CellTrack::Cell*> marked_cells_set(marked_cells.begin(), marked_cells.end());
	for(int i = 0; i < cell_num; i++)
	{
		CellTrack::Cell* cell = m_cells[i];
		QCheckBox* checker = m_checkers[i];
		QTextEdit* editor = m_editors[i];
		editor->setText(tr("<span style=\" color:#%1;\">%2</span>")
				.arg(getColorStr(cell->getColor()))
				.arg(cell->getSize()));
		checker->setText(tr("%1 : ").arg(cell->getTrack()->trackId() + 1));
		if(marked_cells_set.find(m_cells[i]) != marked_cells_set.end())
		{
			checker->setChecked(true);
		}
		else checker->setChecked(false);
		connect(checker,SIGNAL(stateChanged(int)), this, SLOT(onCellChecked(int)));
	}
}

void CellWidget::onCellChecked(int state)
{
	int index = 0;
	QCheckBox * checker = (QCheckBox*) this->sender();
	vector<QCheckBox*>::iterator it = m_checkers.begin();
	while(it != m_checkers.end())
	{
		if((*it) == checker) break; 
		index++;
		it++;
	}
	emit cellChecked(m_cells[index]);
}
// color =  a*255^3 + b*255^2 + g*255 + r
QString getColorStr(unsigned int color)
{
    int high,low;
    QString out;
	int r = color % 256;
	int g = (color / 256) % 256;
	int b = (color / 256/ 256) % 256;
    high=r/16;
    low=r%16;
    out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
    high=g/16;
    low=g%16;
    out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
    high=b/16;
    low=b%16;
    out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
    return out;
}

QString hex2str(int v)
{
    if(v<0 || v>15)return QString("");
    else
    {
        switch(v)
        {
            case 10: return QString("a");
            case 11: return QString("b");
            case 12: return QString("c");
            case 13: return QString("d");
            case 14: return QString("e");
            case 15: return QString("f");
            default: return QObject::tr("%1").arg(v);
        }
    }
}

