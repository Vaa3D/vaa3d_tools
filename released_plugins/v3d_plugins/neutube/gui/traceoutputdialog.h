/**@file traceoutputdialog.h
 * @brief Tracing output dialog
 * @author Ting Zhao
 */
#ifndef TRACEOUTPUTDIALOG_H
#define TRACEOUTPUTDIALOG_H

#include <QDialog>

namespace Ui {
  class TraceOutputDialog;
}

class TraceOutputDialog : public QDialog {
    Q_OBJECT
public:
    TraceOutputDialog(QWidget *parent = 0);
    ~TraceOutputDialog();

public:
    QString dir();
    QString prefix();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TraceOutputDialog *m_ui;

private slots:
    void on_dirPushButton_clicked();
};

#endif // TRACEOUTPUTDIALOG_H
