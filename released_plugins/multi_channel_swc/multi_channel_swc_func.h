#ifndef MULTI_CHANNEL_SWC_FUNC_H
#define MULTI_CHANNEL_SWC_FUNC_H
#include <v3d_interface.h>

NeuronTree readSWC_file_multichannel(const QString& filename);
NeuronTree convert_SWC_to_render_ESWC(const NeuronTree &swc);

#endif // MULTI_CHANNEL_SWC_FUNC_H
