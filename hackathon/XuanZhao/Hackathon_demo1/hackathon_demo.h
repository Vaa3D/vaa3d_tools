#ifndef HACKATHON_DEMO_H
#define HACKATHON_DEMO_H

#include <QWidget>

namespace Ui {
class Hackathon_Demo;
}

class Hackathon_Demo : public QWidget
{
    Q_OBJECT

public:
    explicit Hackathon_Demo(QWidget *parent = 0);
    ~Hackathon_Demo();

private:
    Ui::Hackathon_Demo *ui;
};

#endif // HACKATHON_DEMO_H
