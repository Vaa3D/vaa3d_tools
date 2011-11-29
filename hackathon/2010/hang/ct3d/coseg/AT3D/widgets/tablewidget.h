#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>

namespace Ui {
    class TableWidget;
}

class TableWidget : public QWidget {
    Q_OBJECT
public:
    TableWidget(QWidget *parent = 0);
    ~TableWidget();
    QTableWidget* getTableWidget();
protected:
    void changeEvent(QEvent *e);

private:
    Ui::TableWidget *ui;

private slots:
    void on_exportButton_clicked();
};

#endif // TABLEWIDGET_H
