#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QWidget>

class DataSource : public QWidget
{
    Q_OBJECT
public:
    explicit DataSource(QWidget *parent = nullptr);

signals:

};

#endif // DATASOURCE_H
