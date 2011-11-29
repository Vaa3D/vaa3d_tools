#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>

namespace Ui {
    class FilterDialog;
}

class FilterDialog : public QDialog {
    Q_OBJECT
public:
    FilterDialog(QWidget *parent = 0);
    ~FilterDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FilterDialog *ui;
};

#endif // FILTERDIALOG_H
