#ifndef QHELPBOX_H
#define QHELPBOX_H

#include <QWidget>
#include "control/CPlugin.h"

class teramanager::QHelpBox : public QWidget
{
    Q_OBJECT

    private:

        QLabel *helpBox;
        QLabel *helpIcon;
        QWidget *parent;

    public:

        QHelpBox(QWidget *_parent);

        void setText(string text);

    signals:

    public slots:

};

#endif // QHELPBOX_H
