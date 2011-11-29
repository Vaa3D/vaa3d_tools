#ifndef NFMAIN_H
#define NFMAIN_H

#include "Neuron.h"

void nf_main(const V3DPluginArgList & input, V3DPluginArgList & output);
void computeStems(Neuron * n);
void computeBifs(Neuron *n);
void computeBifs(Neuron *n);
void computeBranches(Neuron *n);
void computeTips(Neuron *n);
void computeWidth(Neuron *n);
void computeHeight(Neuron *n);
void computeDepth(Neuron *n);
void avgType(Neuron *n);
void avgDia(Neuron *n);
void computeLength(Neuron *n);
void computeSurface(Neuron *n);
void computeVol(Neuron *n);
void computeHausdorff(Neuron *n);
void computeFractalDim(Neuron *n);

#endif


