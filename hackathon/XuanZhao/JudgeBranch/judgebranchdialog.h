#ifndef JUDGEBRANCHDIALOG_H
#define JUDGEBRANCHDIALOG_H

#include <QtGui>

#include "v3d_interface.h"
//#include "judgebranchfunction.h"
#include "randomforest.h"
#include "pixelclassification.h"

class JudgeBranchDialog : public QDialog
{
    Q_OBJECT
public:
    JudgeBranchDialog(QWidget* parent, V3DPluginCallback2& callback);

    RandomForest* rf;
    QLineEdit* saveEdit;
    QLineEdit* readEdit;
    V3DPluginCallback2* callback;

public slots:

    void _slots_judgeBranch();
    void _slots_train();
    void _slots_saveRandomForest();
    void _slots_readRandomForest();



};

#endif // JUDGEBRANCHDIALOG_H
