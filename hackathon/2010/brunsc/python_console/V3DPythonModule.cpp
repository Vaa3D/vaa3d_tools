/*
 * V3DPythonModule.cpp
 *
 *  Created on: Dec 22, 2010
 *      Author: Christopher M. Bruns
 */

#include "V3DPythonModule.h"
#include <boost/python.hpp>
#include "v3d_interface.h"
#include <exception>
#include <iostream>

// Store a permanent pointer to the callback the V3DConsolePlugin was launched with.
V3DPluginCallback2 *v3d_callbackPtr;

#include "generated_code/v3d.main.cpp"

namespace v3d {

V3DPluginCallback2* get_plugin_callback() {
    return v3d_callbackPtr;
}

void initV3DPythonModule(V3DPluginCallback2 *callback)
{
    // load module of automatically generated wrappers
    initv3d();

    if (callback) v3d_callbackPtr = callback;
}

} // namespace v3d
