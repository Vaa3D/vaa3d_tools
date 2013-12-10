#include "zstackstatistics.h"
#include "tz_int_histogram.h"
#include "tz_stack_lib.h"
#include "tz_stack_stat.h"

const double ZStackStatistics::m_lowerQuantile = 0.001;
const double ZStackStatistics::m_upperQuantile = 0.999;

ZStackStatistics::ZStackStatistics()
{
}

void ZStackStatistics::getOptimalBc(const ZStack &stack, int c,
                                 double *greyOffset, double *greyScale)
{
  *greyScale = 1.0;
  *greyOffset = 0.0;

  double smin = 0;
  double smax = 0;
  if (getGreyMapHint(stack, c, &smin, &smax)) {
    if (smax != smin) {
      *greyScale = 255.0 / (smax - smin);
      *greyOffset = -*greyScale * smin;
    }
  }
}

bool ZStackStatistics::getGreyMapHint(
    const ZStack &stack, int c, double *smin, double *smax)
{
  bool succ = false;

  double stackMin;
  double stackMax;

  if (!stack.isVirtual()) {
    if (stack.kind() != COLOR) {
      if ((stack.kind() != FLOAT32) && (stack.kind() != FLOAT64)) {
        int *hist = Stack_Hist(stack.c_stack(c));
        stackMin = Int_Histogram_Min(hist);
        stackMax = Int_Histogram_Max(hist);
        if (stackMax > 1) {
          *smin = Int_Histogram_Quantile(hist, m_lowerQuantile);
          *smax = Int_Histogram_Quantile(hist, m_upperQuantile);
        } else {
          *smin = 0;
          *smax = 1;
        }
        free(hist);
      } else {
        stackMin = stack.min(c);
        stackMax = stack.max(c);
        *smin = stackMin;
        *smax = stackMax;
      }

      succ = true;
    }
  }

  return succ;
}
