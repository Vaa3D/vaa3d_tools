/**@file tz_geo3d_ball.h
 * @brief 3D ball
 * @author Ting Zhao
 * @date 18-Apr-2009
 */

#ifndef _TZ_GEO3D_BALL_H_
#define _TZ_GEO3D_BALL_H_

#include "tz_cdefs.h"
#include "tz_stack_draw.h"
#include "tz_geo3d_scalar_field.h"

__BEGIN_DECLS

/**@struct _Geo3d_Ball tz_geo3d_ball.h
 *
 * A ball in the 3D space.
 */
typedef struct _Geo3d_Ball {
  double r; /**< radius */
  double center[3]; /**< center */
} Geo3d_Ball;

typedef struct _Geo3d_Ball_Label_Workspace {
  Stack_Draw_Workspace sdw;
} Geo3d_Ball_Label_Workspace;

void Default_Geo3d_Ball_Label_Workspace(Geo3d_Ball_Label_Workspace *ws);

typedef Geo3d_Ball* Geo3d_Ball_P;

#define NEUROCOMP_GEO3D_BALL(nc) (((nc)->type == NEUROCOMP_TYPE_GEO3D_BALL) ? ((Geo3d_Ball*) ((nc)->data)) : NULL)

Geo3d_Ball* New_Geo3d_Ball();
void Delete_Geo3d_Ball(Geo3d_Ball *ball);
void Kill_Geo3d_Ball(Geo3d_Ball *ball);

void Geo3d_Ball_Default(Geo3d_Ball *ball);

void Geo3d_Ball_Copy(Geo3d_Ball *dst, const Geo3d_Ball *src);
Geo3d_Ball* Copy_Geo3d_Ball(Geo3d_Ball *src);

void Print_Geo3d_Ball(const Geo3d_Ball *ball);

void Geo3d_Ball_Draw_Stack(const Geo3d_Ball *ball, Stack *stack,
		const Stack_Draw_Workspace *ws);

#define GEO3D_BALL_FIELD_SIZE 515
	
Geo3d_Scalar_Field* Geo3d_Ball_Field(const Geo3d_Ball *ball,
    Geo3d_Scalar_Field *field);

/**@brief Mean shift of a ball. 
 */
void Geo3d_Ball_Mean_Shift(Geo3d_Ball *ball, Stack *stack, double z_scale, 
    double min_shift);

void Geo3d_Ball_Label_Stack(const Geo3d_Ball *ball, Stack *stack, 
    Geo3d_Ball_Label_Workspace *ws);

__END_DECLS

#endif
