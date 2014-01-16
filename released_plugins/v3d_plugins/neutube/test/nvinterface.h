#ifndef NVINTERFACE_H
#define NVINTERFACE_H

#include <v3d_interface.h>
#include "zstack.hxx"

namespace NVInterface {

Mc_Stack* makeStack(const Image4DSimple *v3dImage);

}

#endif // NVINTERFACE_H
