#include "nvinterface.h"


Mc_Stack *NVInterface::makeStack(const Image4DSimple *v3dImage)
{
    int width = v3dImage->getXDim();
    int height = v3dImage->getYDim();
    int depth = v3dImage->getZDim();
    int channelCount = v3dImage->getCDim();

    int kind = GREY;
    switch (v3dImage->getDatatype()) {
    case V3D_UINT8:
        kind = GREY;
        break;
    case V3D_UINT16:
        kind = GREY16;
        break;
    case V3D_THREEBYTE:
        kind = COLOR;
        break;
    case V3D_FLOAT32:
        kind = FLOAT32;
        break;
    default:
        return NULL;
    }

    Mc_Stack stack;
    C_Stack::setAttribute(&stack, kind, width, height, depth, channelCount);
    stack.array = const_cast<unsigned char*>(v3dImage->getRawData());
    Mc_Stack *out = C_Stack::clone(&stack);

    return out;
}
