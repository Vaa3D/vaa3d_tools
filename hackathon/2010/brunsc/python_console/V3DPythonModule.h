/*
 * V3DPythonModule.h
 *
 *  Created on: Dec 22, 2010
 *      Author: Christopher M. Bruns
 */

#ifndef V3D_PYTHON_MODULE_H_
#define V3D_PYTHON_MODULE_H_

#include <cstddef> // NULL
#include "v3d_qt_environment.h"

namespace v3d {

// Run initV3DPythonModule *AFTER*
// Py_Initialize() has been called.  i.e., after
// PythonInterpreter instance has been created.
// You will still need to "import v3d" in python to
// use the v3d module.
void initV3DPythonModule(V3DPluginCallback2 *v3d = NULL);

}

#endif /* V3D_PYTHON_MODULE_H_ */
