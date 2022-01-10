#ifndef _IO_AFFINE_TRANSFORM_H_
#define _IO_AFFINE_TRANSFORM_H_ 

#include <newmat.h>
#include <QString>

/* transform.txt
 r11  r12  r13   tx
 r21  r22  r23   ty
 r31  r32  r33   tz

*/

Matrix readTransform(QString filename);

#endif
