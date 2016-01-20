#ifndef STACKANALYZER_H
#define STACKANALYZER_H

#include <QObject>
#include <v3d_interface.h>
#include <QString>>

class stackAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit stackAnalyzer(QObject *parent = 0);

signals:
    void analysisDone(QList<int[3]> newTargets);
    void messageSignal(QString msg);
public slots:
    void processStack(Image4DSimple * pInputImage);

};

#endif // STACKANALYZER_H
