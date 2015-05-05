#ifndef TEXTINFODIALOG_H
#define TEXTINFODIALOG_H

#include <QDialog>

namespace Ui {
    class TextInfoDialog;
}

class TextInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextInfoDialog(QWidget *parent = 0);
    ~TextInfoDialog();

    void setText(const QString &s);

private:
    Ui::TextInfoDialog *ui;
};

#endif // TEXTINFODIALOG_H
