#ifndef QLINETREE_H
#define QLINETREE_H

#include <QWidget>
#include "../control/CPlugin.h"

class teramanager::QLineTree : public QWidget
{
    Q_OBJECT

    private:

        QColor color;
        int lineWidth;
        int branches;
        int verticalMargin;

        QLineTree();

    public:

        QLineTree(QWidget *parent, QColor _color, float _lineWidth, int _branches, int _vertMargin);

        void paintEvent(QPaintEvent * evt);

    signals:

    public slots:
    
};

#endif // QLINETREE_H
