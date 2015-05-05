#include "SegmentationColorifier.h"

extern "C" Q_DECL_EXPORT PluginBase* createPlugin();

PluginBase *createPlugin()
{
    return new SegmentationColorifier();
}
