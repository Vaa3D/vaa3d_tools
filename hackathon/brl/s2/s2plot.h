#ifndef S2PLOT_H
#define S2PLOT_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
namespace Ui {
class S2Plot;
}

class S2Plot : public QWidget
{
    Q_OBJECT

public:
    explicit S2Plot(QWidget *parent = 0);
    ~S2Plot();
    QGraphicsScene *plotGS;
    QGraphicsRectItem *myRect;
private:
    Ui::S2Plot *ui;
};

#endif // S2PLOT_H
