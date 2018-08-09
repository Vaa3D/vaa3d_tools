#ifndef NEURON_CONNECTOR_FUNC_H
#define NEURON_CONNECTOR_FUNC_H

#include <QtGui>
#include <QDialog>
#include <v3d_interface.h>

void printHelp();
void connectall(NeuronTree* nt, QList<NeuronSWC>& newNeuron, double xscale, double yscale, double zscale, double angThr, double disThr, int matchtype, bool minusradius, int rootID);
double getswcdiameter(NeuronTree* nt);
bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName);

#endif // NEURON_CONNECTOR_FUNC_H
