#ifndef QGRADIENTBAR_H
#define QGRADIENTBAR_H

#include <QWidget>
#include "control/CPlugin.h"

class teramanager::QGradientBar : public QWidget
{
    Q_OBJECT

    private:

        int nSteps;
        int step;

    public:

        QGradientBar(QWidget *parent = 0) : QWidget(parent){nSteps = -1; step = 0;}
        void setNSteps(int _nSteps){nSteps = _nSteps;}
        void setStep(int _step){step = _step;}

    protected:

        //needed to draw the arrow with QPainter
        void paintEvent(QPaintEvent * evt);

    signals:

    public slots:

};

#endif // QGRADIENTBAR_H
