#ifndef __NS_RENDER_RENDER_H__
#define __NS_RENDER_RENDER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <math/nsvector.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nscolor.h>
#include <image/nscolordb.h>
#include <image/nssampler-decl.h>
#include <model/nsmodel-decl.h>
#include <model/nssettings-decl.h>
#include <model/nsspines-classifier.h>

NS_DECLS_BEGIN

struct _NsRenderState;
typedef struct _NsRenderState NsRenderState;


typedef void ( *NsRenderPreBordersFunc )( NsRenderState* );

typedef void ( *NsRenderBorderFunc )
   (
   const NsVector3i*,
   const NsVector3i*,
   NsRenderState*
   );

typedef void ( *NsRenderPostBordersFunc )( NsRenderState* );


typedef void ( *NsRenderPrePointsFunc )( NsRenderState* );

typedef void ( *NsRenderPointFunc )
   (
   const NsVector3b*,
   const NsVector3f*,
   NsRenderState*
   );

typedef void ( *NsRenderPostPointsFunc )( NsRenderState* );


typedef void ( *NsRenderPreLinesFunc )( NsRenderState* );

typedef void ( *NsRenderLineFunc )
   (
   const NsVector3f*,
   const NsVector3f*,
   NsRenderState*
   );

typedef void ( *NsRenderPostLinesFunc )( NsRenderState* );


typedef void ( *NsRenderPreImagesFunc )( NsRenderState* );

typedef void ( *NsRenderImageFunc )
   (
   const NsVector3i*,
   const NsImage*,
   const NsImage*,
   const NsImage*,
   NsRenderState*
   );

typedef void ( *NsRenderPostImagesFunc )( NsRenderState* );


typedef void ( *NsRenderPreTextsFunc )( NsRenderState* );

typedef void ( *NsRenderTextFunc )
   (
   const NsVector3f*,
   const nschar*,
   NsRenderState*
   );

typedef void ( *NsRenderPostTextsFunc )( NsRenderState* );


typedef void ( *NsRenderPreTrianglesFunc )( NsRenderState* );

typedef void ( *NsRenderTriangleFunc )
   (
   const NsVector3f*,
   const NsVector3f*,
   const NsVector3f*,
   const NsVector3f*,
   const NsVector3f*,
   const NsVector3f*,
   NsRenderState*
   );

typedef void ( *NsRenderPostTrianglesFunc )( NsRenderState* );


typedef void ( *NsRenderPreSpheresFunc )( NsRenderState* );

typedef void ( *NsRenderSphereFunc )
   (
   const NsVector3f*,
   nsfloat,
   NsRenderState*
   );

typedef void ( *NsRenderPostSpheresFunc )( NsRenderState* );


typedef void ( *NsRenderPrePlanesFunc )( NsRenderState* );

typedef void ( *NsRenderPlaneFunc )
   (
   const NsVector3f*,
   const NsVector3f*,
   NsRenderState*
   );

typedef void ( *NsRenderPostPlanesFunc )( NsRenderState* );


typedef void ( *NsRenderPreEllipsesFunc )( NsRenderState* );

typedef void ( *NsRenderEllipseFunc )
   (
   const NsVector3f*,
   const NsVector3f*,
   NsRenderState*
   );

typedef void ( *NsRenderPostEllipsesFunc )( NsRenderState* );


typedef void ( *NsRenderPreAABBoxesFunc )( NsRenderState* );

typedef void ( *NsRenderAABBoxFunc )
   (
   const NsAABBox3d*,
   NsRenderState*
   );

typedef void ( *NsRenderPostAABBoxesFunc )( NsRenderState* );


typedef void ( *NsRenderPreFrustumsFunc )( NsRenderState* );

typedef void ( *NsRenderFrustumFunc )
   (
	const NsVector3f*,
	nsfloat,
	const NsVector3f*,
	nsfloat,
	NsRenderState*
   );

typedef void ( *NsRenderPostFrustumsFunc )( NsRenderState* );


typedef void ( *NsRenderPreRectanglesFunc )( NsRenderState* );

typedef void ( *NsRenderRectangleFunc )
   (
   const NsVector3f*,
   const NsVector3f*,
   NsRenderState*
   );

typedef void ( *NsRenderPostRectanglesFunc )( NsRenderState* );


/* Any of the functions can be NULL. */
typedef struct _NsRenderInterface
   {
   NsRenderPreBordersFunc      pre_borders_func;
   NsRenderBorderFunc          border_func;
   NsRenderPostBordersFunc     post_borders_func;

   NsRenderPrePointsFunc       pre_points_func;
   NsRenderPointFunc           point_func;
   NsRenderPostPointsFunc      post_points_func;

   NsRenderPreLinesFunc        pre_lines_func;
   NsRenderLineFunc            line_func;
   NsRenderPostLinesFunc       post_lines_func;

   NsRenderPreImagesFunc       pre_images_func;
   NsRenderImageFunc           image_func;
   NsRenderPostImagesFunc      post_images_func;

   NsRenderPreTextsFunc        pre_texts_func;
   NsRenderTextFunc            text_func;
   NsRenderPostTextsFunc       post_texts_func;

   NsRenderPreTrianglesFunc    pre_triangles_func;
   NsRenderTriangleFunc        triangle_func;
   NsRenderPostTrianglesFunc   post_triangles_func;

   NsRenderPreSpheresFunc      pre_spheres_func;
   NsRenderSphereFunc          sphere_func;
   NsRenderPostSpheresFunc     post_spheres_func;

   NsRenderPrePlanesFunc       pre_planes_func;
   NsRenderPlaneFunc           plane_func;
   NsRenderPostPlanesFunc      post_planes_func;

   NsRenderPreEllipsesFunc     pre_ellipses_func;
   NsRenderEllipseFunc         ellipse_func;
   NsRenderPostEllipsesFunc    post_ellipses_func;

   NsRenderPreAABBoxesFunc     pre_aabboxes_func;
   NsRenderAABBoxFunc          aabbox_func;
   NsRenderPostAABBoxesFunc    post_aabboxes_func;

   NsRenderPreFrustumsFunc     pre_frustums_func;
   NsRenderFrustumFunc         frustum_func;
   NsRenderPostFrustumsFunc    post_frustums_func;

   NsRenderPreRectanglesFunc   pre_rectangles_func;
   NsRenderRectangleFunc       rectangle_func;
   NsRenderPostRectanglesFunc  post_rectangles_func;
   }
   NsRenderInterface;


/* NOTE: Dont use enumerated values that
   begin in underscores. */
typedef enum
   {
   NS_RENDER_NO_SHAPE,

   NS_RENDER_SHAPE_POINT,
   NS_RENDER_SHAPE_IMAGE,
   NS_RENDER_SHAPE_TEXT,

__NS_RENDER_LINE_ART_BEGIN,
   NS_RENDER_SHAPE_BORDER,
   NS_RENDER_SHAPE_LINE,
   NS_RENDER_SHAPE_AABBOX,
__NS_RENDER_LINE_ART_END,

__NS_RENDER_POLYGON_ART_BEGIN,
   NS_RENDER_SHAPE_PLANE,
   NS_RENDER_SHAPE_TRIANGLE,
   NS_RENDER_SHAPE_SPHERE,
   NS_RENDER_SHAPE_ELLIPSE,
   NS_RENDER_SHAPE_FRUSTUM,
   NS_RENDER_SHAPE_RECTANGLE,
__NS_RENDER_POLYGON_ART_END
   }
   NsRenderShape;


typedef enum{
	NS_RENDER_STEP_SAMPLES,
	NS_RENDER_STEP_GRAFTING,
	NS_RENDER_STEP_VERTICES,
	NS_RENDER_STEP_EDGES,
	NS_RENDER_STEP_SPINES,

	NS_RENDER_NUM_STEPS
	}
	NsRenderStep;


NS_IMPEXP void ns_render_state_set_step( NsRenderState *state, nssize index, NsRenderStep step );
NS_IMPEXP void ns_render_state_set_default_steps_order( NsRenderState *state );


struct _NsRenderState
   {
   struct
      {
      NsRenderInterface      iface;

		NsRenderStep           steps[ NS_RENDER_NUM_STEPS ];

		nsint                  dimensions;

		const NsImage         *dataset;
		const NsImage         *proj_xy;
		const NsImage         *proj_zy;
		const NsImage         *proj_xz;
      NsModel               *model;
      NsSampler             *sampler;
      const NsVoxelInfo     *voxel_info;
      const NsSettings      *settings;
		NsSpinesClassifier    *classifier;

      nsboolean              flicker_free_rendering;
      nsboolean              lighting;
		nsboolean              draw_dataset_only;
		nsboolean              draw_hidden_objects;
		nsboolean              is_model_mask;

		nsfloat                model_mask_factor;

      nsboolean              view_sampler_text;
      nsboolean              view_graft_thresholding;
      nsboolean              view_graft_all_vertices;
      nsboolean              view_spine_vectors;
      nsboolean              view_spine_attachments;
      nsboolean              spines_color_by_layer;
      nsboolean              view_sampler_thresholding;
      nsboolean              view_sampler_surface_mesh;
      nsboolean              view_spine_maxima_voxels;
      nsboolean              view_spine_candidate_voxels;
      nsboolean              view_maximum_spine_height;
      nsboolean              spines_soft_corners;

      nsint                  vertex_shape_mode;
      nsint                  VERTEX_NO_SHAPE;
      nsint                  VERTEX_SHAPE_HOLLOW_ELLIPSES;
      nsint                  VERTEX_SHAPE_SOLID_ELLIPSES;
      nsint                  VERTEX_SHAPE_POINTS;

      nsint                  edge_shape_mode;
      nsint                  EDGE_NO_SHAPE;
      nsint                  EDGE_SHAPE_HOLLOW_FRUSTUMS;
      nsint                  EDGE_SHAPE_SOLID_FRUSTUMS;
      nsint                  EDGE_SHAPE_LINES;

      nsint                  spine_shape_mode;
      nsint                  SPINE_NO_SHAPE;
      nsint                  SPINE_SHAPE_POINTS;
      nsint                  SPINE_SHAPE_SOLID_RECTANGLES;
      nsint                  SPINE_SHAPE_HOLLOW_RECTANGLES;
      nsint                  SPINE_SHAPE_SOLID_VOLUMETRIC;
      nsint                  SPINE_SHAPE_HOLLOW_VOLUMETRIC;
      nsint                  SPINE_SHAPE_SOLID_SURFACE;
      nsint                  SPINE_SHAPE_HOLLOW_SURFACE;
		nsint                  SPINE_SHAPE_SOLID_ELLIPSES;
		nsint                  SPINE_SHAPE_HOLLOW_ELLIPSES;
		nsint                  SPINE_SHAPE_SOLID_BALL_AND_STICK;
		nsint                  SPINE_SHAPE_HOLLOW_BALL_AND_STICK;
		nsint                  SPINE_SHAPE_SOLID_ELLIPSES_HD;
		nsint                  SPINE_SHAPE_HOLLOW_ELLIPSES_HD;
		nsint                  SPINE_SHAPE_CLUMPS;

      nsint                  vertex_color_scheme;
      nsint                  VERTEX_SINGLE_COLOR;
		nsint                  VERTEX_COLOR_BY_FUNCTION;
      nsint                  VERTEX_COLOR_BY_TYPE;
      nsint                  VERTEX_COLOR_BY_COMPONENT;
      nsint                  VERTEX_COLOR_BY_FILE_NUMBER;
      nsint                  VERTEX_COLOR_IS_MANUALLY_TRACED;
      nsint                  VERTEX_COLOR_BY_ORDER;
      nsint                  VERTEX_COLOR_BY_SECTION;
      NsColor4ub             vertex_single_color;
      NsColor4ub             manually_traced_color;
      const NsColor4ub      *vertex_type_colors;
      nssize                 num_vertex_type_colors;

      const NsColor4ub      *component_colors;
      nssize                 num_component_colors;

      nsint                  edge_color_scheme;
      nsint                  EDGE_SINGLE_COLOR;
		nsint                  EDGE_COLOR_BY_FUNCTION;
      nsint                  EDGE_COLOR_BY_COMPONENT;
      nsint                  EDGE_COLOR_BY_ORDER;
      nsint                  EDGE_COLOR_BY_SECTION;
      NsColor4ub             edge_single_color;
      const NsColor4ub      *edge_order_colors;
      nssize                 num_edge_order_colors;
      const NsColor4ub      *edge_section_colors;
      nssize                 num_edge_section_colors;

      nsint                  spine_color_scheme;
      nsint                  SPINE_SINGLE_COLOR;
      nsint                  SPINE_COLOR_BY_TYPE;
      nsint                  SPINE_COLOR_RANDOM;
		nsint                  SPINE_COLOR_IS_MANUAL;
		nsint                  SPINE_COLOR_BY_ANGLE;
      NsColor4ub             spine_single_color;
      const NsColor4ub      *spine_type_colors;
      nssize                 num_spine_type_colors;
      const NsColor4ub      *spine_random_colors;
      nssize                 num_spine_random_colors;

		const NsColor4ub      *function_type_colors;
		nssize                 num_function_type_colors;

      NsColorDb             *brush_db;
      nspointer              ( *get_brush )( NsColorDb*, nsuint );
      void                   ( *set_brush )( NsColorDb*, nsuint, nspointer, NsFinalizeFunc );
      NsColorDb             *pen_db;
      nspointer              ( *get_pen )( NsColorDb*, nsuint );
      void                   ( *set_pen )( NsColorDb*, nsuint, nspointer, NsFinalizeFunc );

      nssize                 line_size;
      nssize                 point_size;
      nsint                  corner_x;
      nsint                  corner_y;
      nsint                  display;
      nsfloat                zoom;
      nsint                  polygon_complexity;
      nspointer              graphics;
      const NsColor4ub      *palette_colors;
      nssize                 palette_size;
      nsboolean              polygon_borders;
		nsboolean              anti_aliasing;
      nspointer              user_data;
      }
      constants;

   struct
      {
      nsboolean      filled;
      NsColor4ub     color;
		NsColor4ub     altered_color;
      NsRenderShape  shape;
      nsfloat        line_size;
      nsfloat        point_size;
      nsuint         display_lists;
      nsint          polygon_mode_front;
      nsint          polygon_mode_back;
      nspointer      prev_brush;
      nspointer      curr_brush;
      nspointer      prev_pen;
      nspointer      curr_pen;
      nsuint         prev_pen_color;
      nsuint         curr_pen_color;
      nsuint         prev_brush_color;
      nsuint         curr_brush_color;
      nsint          bk_mode;
      nsuint         text_color;
      nspointer      palette;
      nsboolean      lighting;
      nsboolean      blend_enabled;
      nsint          blend_src;
      nsint          blend_dest;
		nsboolean      line_smooth;
		nsboolean      point_smooth;
      }
      variables;
   };


NS_IMPEXP void ns_render_state_init( NsRenderState *state );

NS_IMPEXP void ns_render( NsRenderState *state );


/* The minimum size of any dimension of a bounding box
   in client space. */
#define NS_RENDER_AABBOX_MIN_CLIENT_SIZE  6


#define ns_render_state_is_line_art( state )\
   ( __NS_RENDER_LINE_ART_BEGIN < (state)->variables.shape &&\
   (state)->variables.shape < __NS_RENDER_LINE_ART_END )

#define ns_render_state_is_polygon_art( state )\
   ( __NS_RENDER_POLYGON_ART_BEGIN < (state)->variables.shape &&\
   (state)->variables.shape < __NS_RENDER_POLYGON_ART_END )


/* Helper macro. The pre and post functions if NULL are not called.
   Saves having to write the if statement all the time. */
#define NS_RENDER_PRE_OR_POST( state, func )\
   if( NULL != (state)->constants.iface.func )( (state)->constants.iface.func )( (state) )

#define NS_RENDER( state, func )\
   ( (state)->constants.iface.func )


NS_IMPEXP NsColor4ub ns_render_state_spine_color( NsRenderState *state, nspointer );

NS_IMPEXP NsColor4ub ns_render_state_vertex_color( NsRenderState *state, nspointer );

NS_IMPEXP NsColor4ub ns_render_state_edge_color( NsRenderState *state, nspointer );
NS_IMPEXP NsColor4ub ns_render_state_edge_color_ex( NsRenderState *state, nsint edge_color_scheme, nspointer );

NS_DECLS_END

#endif/* __NS_RENDER_RENDER_H__ */


