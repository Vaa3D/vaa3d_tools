//histogram_gui.h
//adapted from this file
/*
 * barFigureDialog.h
 *
 *  Created on: May 1, 2009
 *      Author: ruanzongcai
 */

#ifndef BARFIGUREDIALOG_H_
#define BARFIGUREDIALOG_H_

#include <QtGui>

class histogramDialog : public QDialog
{
public:
	histogramDialog(QVector< QVector<int> >& vvec, QStringList labelsOfLeftTop, QString labelOfRightBottom,
			QWidget *parent=0, QSize figSize=QSize(500, 150), QColor barColor=QColor(100,100,100));
	virtual ~histogramDialog();
	virtual void closeEvent( QCloseEvent* e);
};

QImage drawBarFigure(QVector<int>& vec, QColor barColor=QColor(100,100,100));

#endif /* BARFIGUREDIALOG_H_ */
