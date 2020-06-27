#ifndef RETRACEDIALOG_H
#define RETRACEDIALOG_H

#include <QtGui>
#include "retracefunction.h"


class RetraceDialog : public QDialog
{
    Q_OBJECT
public:
    RetraceDialog(QWidget* parent, V3DPluginCallback2& callback);

    QString eswcPath = "";
    QString apoPath = "";
    QString brainPath = "";
    int resolution = 1;

    QLineEdit* eswcEdit;
    QLineEdit* apoEdit;
    QLineEdit* brainEdit;
    QLineEdit* resolutionEdit;
    V3DPluginCallback2* callback;

public slots:

    void _slots_readEswc();
    void _slots_readApo();
    void _slots_readBrain();
    void _slots_start();

};

#endif // RETRACEDIALOG_H
