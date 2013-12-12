/**@file tz_cuboid_i.h
 * @brief cuboid in integer space
 * @author Ting Zhao
 * @date 26-Mar-2010
 */

#ifndef _TZ_CUBOID_I_H_
#define _TZ_CUBOID_I_H_

#include "tz_cdefs.h"
#include "tz_zobject.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

typedef struct _Cuboid_I {
  int cb[3]; /**< First corner */
  int ce[3]; /**< Last corner */
} Cuboid_I;

DECLARE_ZOBJECT_INTERFACE(Cuboid_I)

/**@brief Test if a cuboid is valid.
 *
 * Cuboid_I_Is_Valid() returns true only when the last corner of <cuboid> is not
 * less than its first corner in any dimension.
 */
BOOL Cuboid_I_Is_Valid(const Cuboid_I *cuboid);

/**@brief Size of a cuboid.
 *
 * Cuboid_I_Size() rechieves the size of <cuboid>. <width>, <height> and 
 * <depth> are the places to store the result, but any of them can be NULL, 
 * which means the correponding dimension is ignored.
 */
void Cuboid_I_Size(const Cuboid_I *cuboid, int *width, int *height, int *depth);

/**@brief Set a cuboid.
 *
 * Cuboid_I_Set_S() sets the first corner of <cuboid> to (<x>, <y>, <z>) and its
 * size to (<width>, <height>, <depth>).
 */
void Cuboid_I_Set_S(Cuboid_I *cuboid, int x, int y, int z, int width, 
		    int height, int depth);

/**@brief Volume of a cuboid.
 */
int Cuboid_I_Volume(const Cuboid_I *cuboid);

/**@brief Intersect two cuboids
 *
 * Cuboid_I_Intersect() stores the result of intersecting <c1> and <c2>
 * in <c3>. <c3> can be the same reference as <c1> or <c2> for in-place
 * calculation.
 */
Cuboid_I* Cuboid_I_Intersect(const Cuboid_I *c1, 
			     const Cuboid_I *c2, Cuboid_I *c3);

/**@brief Union of two cuboids
 *
 * Cuboid_I_Intersect() stores the bound box of <c1> and <c2>
 * in <c3>. <c3> can be the same reference as <c1> or <c2> for in-place
 * calculation.
 */
Cuboid_I* Cuboid_I_Union(const Cuboid_I *c1, const Cuboid_I *c2, Cuboid_I *c3);

int Cuboid_I_Overlap_Volume(const Cuboid_I *c1, const Cuboid_I *c2);

void Cuboid_I_Expand_X(Cuboid_I *cuboid, int margin);
void Cuboid_I_Expand_Y(Cuboid_I *cuboid, int margin);
void Cuboid_I_Expand_Z(Cuboid_I *cuboid, int margin);

void Cuboid_I_Expand_P(Cuboid_I *cuboid, const int *pt);

void Cuboid_I_Label_Stack(const Cuboid_I *cuboid, int v, Stack *stack);

BOOL Cuboid_I_Hit(const Cuboid_I *cuboid, int x, int y, int z);

/**@brief Test if a point hit the internal of the cuboid
 */
BOOL Cuboid_I_Hit_Internal(const Cuboid_I *cuboid, int x, int y, int z);


__END_DECLS

#endif
