#ifndef GCDIALOG_H
#define GCDIALOG_H

#include <QDialog>

namespace Ui {
class GCDialog;
}

class GCDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit GCDialog(QWidget *parent = 0);
    ~GCDialog();
    
    float getEdgeWeight();

    float getVariance();

private:
    Ui::GCDialog *ui;
};

#endif // GCDIALOG_H
