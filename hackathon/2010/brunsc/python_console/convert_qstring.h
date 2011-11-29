#ifndef V3D_PYTHON_CONVERT_QSTRING_H_
#define V3D_PYTHON_CONVERT_QSTRING_H_

/*
 * convert_qstring.h
 *
 *  Created on: Dec 23, 2010
 *      Author: cmbruns
 *
 *  Interconvert between QString and python string.
 *  Warning: use of this technique might be incompatible with
 *  PyQt or PySide
 */

// Call this from within BOOST_PYTHON_MODULE block
void register_qstring_conversion();

#endif /* V3D_PYTHON_CONVERT_QSTRING_H_ */
