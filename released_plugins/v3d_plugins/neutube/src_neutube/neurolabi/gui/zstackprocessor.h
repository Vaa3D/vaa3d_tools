/**@file zstackprocessor.h
 * @brief Stack processor
 * @author Ting Zhao
 */

#ifndef ZSTACKPROCESSOR_H
#define ZSTACKPROCESSOR_H

#include "itkimagedefs.h"
#include <string>

class ZStack;

class ZStackProcessor
{
public:
  ZStackProcessor();

public:
  void medianFilter(ZStack *stack, int radius = 1);
  void mexihatFilter(ZStack *stack, double sigma = 1.0);
  void cannyEdge(ZStack *stack, double variance = 1.0,
                 double low = 0.0, double high = 1.0);
  void anisotropicDiffusion(ZStack *stack, double timeStep = 0.125,
                            double conductance = 20.0, int niter = 5);
  void curvatureFlow(ZStack *stack, double timeStep = 0.125, int niter = 10);
  void minMaxCurvatureFlow(ZStack *stack, double timeStep = 0.125,
                           double radius = 1.0, int niter = 10);
  void connectedThreshold(ZStack *stack, int x, int y, int z,
                          int lower, int upper);
  void distanceTransform(ZStack *stack, bool isSquared = false,
                         bool sliceWise = false);
  void shortestPathFlow(ZStack *stack);
  void expandRegion(ZStack *stack, int r);

  void removeIsolatedObject(ZStack *stack, int r, int dr);

  static void invert(ZStack *stack);

  // noiseModel: "GAUSSIAN" or "RICIAN" or "POISSON"
  //
  void patchBasedDenoising(ZStack *stack, const int numIterations = 2, const int numThreads = 2,
                           const int numToSample = 1000, const float sigmaMultiplicationFactor = 1.f,
                           const std::string noiseModel = "POISSON", const float fidelityWeight = 0.1f);

//private:
  static void convertStack(ZStack *stack, Uint8Image3DType *image);
  static void convertStack(ZStack *stack, Uint16Image3DType *image);
  static void convertStack(ZStack *stack, FloatImage3DType *image);
  static void copyData(Uint8Image3DType *src, ZStack *dst, int ch = 0);
  static void copyData(Uint16Image3DType *src, ZStack *dst, int ch = 0);
  static void copyData(FloatImage3DType *src, ZStack *dst, int ch = 0);
  static void convertStack(const ZStack *stack, int ch, Uint8Image3DType *image);
  static void convertStack(const ZStack *stack, int ch, Uint16Image3DType *image);
  static void convertStack(const ZStack *stack, int ch, FloatImage3DType *image);
};

#endif // ZSTACKPROCESSOR_H
