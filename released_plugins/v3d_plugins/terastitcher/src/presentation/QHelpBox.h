#ifndef QHELPBOX_H
#define QHELPBOX_H

#include <QWidget>
#include "../control/CPlugin.h"

class terastitcher::QHelpBox : public QWidget
{
    Q_OBJECT

    private:

        QLabel *helpBox;
        QLabel *helpIcon;
        QWidget *parent;

        string defaultMsg;

    public:

        QHelpBox(QWidget *_parent);

        void setText(string text);
        void setDefaultMessage(string _defMsg){defaultMsg = _defMsg; showDefaultMessage();}
        void showDefaultMessage(){setText(defaultMsg);}

    signals:

    public slots:

};

#endif // QHELPBOX_H
