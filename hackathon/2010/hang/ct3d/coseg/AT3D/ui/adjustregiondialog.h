#ifndef ADJUSTREGIONDIALOG_H
#define ADJUSTREGIONDIALOG_H

#include <QDialog>

namespace Ui {
    class AdjustRegionDialog;
}

class AdjustRegionDialog : public QDialog {
    Q_OBJECT
public:
    AdjustRegionDialog(QWidget *parent = 0);
    ~AdjustRegionDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AdjustRegionDialog *ui;

private slots:
};

#endif // ADJUSTREGIONDIALOG_H
