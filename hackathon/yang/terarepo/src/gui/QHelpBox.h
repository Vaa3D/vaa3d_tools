#ifndef QHELPBOX_H
#define QHELPBOX_H

#include <QWidget>
#include "CTeraStitcher.h"

class terastitcher::QHelpBox : public QWidget
{
    Q_OBJECT

    private:

        QLabel *helpBox;
        QLabel *helpIcon;
        QWidget *parent;

        std::string defaultMsg;

    public:

        QHelpBox(QWidget *_parent);

        void setText(std::string text);
        void setDefaultMessage(std::string _defMsg){defaultMsg = _defMsg; showDefaultMessage();}
        void showDefaultMessage(){setText(defaultMsg);}

    signals:

    public slots:

};

#endif // QHELPBOX_H
