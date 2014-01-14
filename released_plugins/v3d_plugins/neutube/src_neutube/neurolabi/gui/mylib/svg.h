/*****************************************************************************************\
*                                                                                         *
*  SVG drawing package                                                                    *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2006                                                                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _SVG_HEADER

#define _SVG_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"

typedef struct
  { double x, y;
  } SVG_Point;

SVG_Point   *G(Make_SVG_Point)(double x, double y);
SVG_Point   *G(Copy_SVG_Point)(SVG_Point *point);
SVG_Point   *Pack_SVG_Point(SVG_Point *R(M(point)));
SVG_Point   *Inc_SVG_Point(SVG_Point *R(I(point)));
void         Free_SVG_Point(SVG_Point *F(point));
void         Kill_SVG_Point(SVG_Point *K(point));
void         Reset_SVG_Point();
int          SVG_Point_Usage();
void         SVG_Point_List(void (*handler)(SVG_Point *));
int          SVG_Point_Refcount(SVG_Point *point);

typedef struct
  { double w, h;
  } SVG_Box;

SVG_Box     *G(Make_SVG_Box)(double w, double h);
SVG_Box     *G(Copy_SVG_Box)(SVG_Box *box);
SVG_Box     *Pack_SVG_Box(SVG_Box *R(M(box)));
SVG_Box     *Inc_SVG_Box(SVG_Box *R(I(box)));
void         Free_SVG_Box(SVG_Box *F(box));
void         Kill_SVG_Box(SVG_Box *K(box));
void         Reset_SVG_Box();
int          SVG_Box_Usage();
void         SVG_Box_List(void (*handler)(SVG_Box *));
int          SVG_Box_Refcount(SVG_Box *box);

typedef struct
  { SVG_Point corner;
    SVG_Box   window; 
  } SVG_Frame;

SVG_Frame   *G(Make_SVG_Frame)(SVG_Point *F(corner), SVG_Box *F(window));
SVG_Frame   *G(Copy_SVG_Frame)(SVG_Frame *frame);
SVG_Frame   *Pack_SVG_Frame(SVG_Frame *R(M(frame)));
SVG_Frame   *Inc_SVG_Frame(SVG_Frame *R(I(frame)));
void         Free_SVG_Frame(SVG_Frame *F(frame));
void         Kill_SVG_Frame(SVG_Frame *K(frame));
void         Reset_SVG_Frame();
int          SVG_Frame_Usage();
void         SVG_Frame_List(void (*handler)(SVG_Frame *));
int          SVG_Frame_Refcount(SVG_Frame *frame);

typedef void SVG_Path;

SVG_Path   *G(Make_SVG_Path)(int n);
SVG_Path   *Append_Move(SVG_Path *R(M(path)), SVG_Point *C(pnt));
SVG_Path   *Append_Line(SVG_Path *R(M(path)), SVG_Point *C(pnt));
SVG_Path   *Append_Bezier(SVG_Path *R(M(path)),
                          SVG_Point *C(in), SVG_Point *C(out), SVG_Point *C(pnt));
SVG_Path   *Append_Arc(SVG_Path *R(M(path)), SVG_Point *C(rad), double xrot, boolean large_arc,
                                             boolean cc_sweep, SVG_Point *C(pnt));
SVG_Path   *Append_Close(SVG_Path *R(M(path)));

SVG_Path   *G(Copy_SVG_Path)(SVG_Path *path);
SVG_Path   *Pack_SVG_Path(SVG_Path *R(M(path)));
SVG_Path   *Inc_SVG_Path(SVG_Path *R(I(path)));
void        Free_SVG_Path(SVG_Path *F(path));
void        Kill_SVG_Path(SVG_Path *K(path));
void        Reset_SVG_Path();
int         SVG_Path_Usage();
void        SVG_Path_List(void (*handler)(SVG_Path *));
int         SVG_Path_Refcount(SVG_Path *path);

typedef void SVG_Context;
typedef void SVG_Canvas;

void        *G(Make_SVG_Context)();
SVG_Context *Set_To_Default_Context(SVG_Context *M(context));

SVG_Context *Get_SVG_Context(SVG_Canvas *canvas);
void         Set_SVG_Context(SVG_Canvas *M(canvas), SVG_Context *C(context));
void         Set_Arrow_Context(SVG_Canvas *M(canvas), SVG_Context *C(context));

SVG_Context *G(Copy_SVG_Context)(SVG_Context *context);
SVG_Context *Pack_SVG_Context(SVG_Context *R(M(context)));
SVG_Context *Inc_SVG_Context(SVG_Context *R(I(context)));
void         Free_SVG_Context(SVG_Context *F(context));
void         Kill_SVG_Context(SVG_Context *K(context));
void         Reset_SVG_Context();
int          SVG_Context_Usage();
void         SVG_Context_List(void (*handler)(SVG_Context *));
int          SVG_Context_Refcount(SVG_Context *context);

  // An SVG_Canvas object is unusual in that it does not have Copy, Free, Kill, Reset, or
  //   Usage routines.  You can simply create one with Begin_SVG_Drawing and then later
  //   kill/destroy it with Finish_SVG_Drawing.  Begin_SVG_Drawing returns NULL if it
  //   cannot create and open the files <name>.head and <name>.tail for writing.

  // Establishes a new xfig drawing that will placed in the file <name>.fig.
  //   Has the side effect of setting the current canvas to the one that is
  //   beginning.  The target frame is assumed to be in *inches.*

SVG_Canvas *G(Begin_SVG_Drawing)(string file_name, SVG_Box *F(viewport), SVG_Frame *F(viewbox));

  // Finishes the creation of the SVG file closing underlying IO and returning
  //   the canvas to the system heap

void Finish_SVG_Drawing(SVG_Canvas *K(canvas));

  // All primitives below affect the graphic state of the current canvas

typedef enum
  { SVG_alice_blue		= 0,        SVG_antique_white		= 1,
    SVG_aqua			= 2,        SVG_aquamarine		= 3,
    SVG_azure			= 4,        SVG_beige			= 5,
    SVG_bisque			= 6,        SVG_black			= 7,
    SVG_blanched_almond		= 8,        SVG_blue			= 9,
    SVG_blue_violet		= 10,       SVG_brown			= 11,
    SVG_burly_wood		= 12,       SVG_cadet_blue		= 13,
    SVG_chartreuse		= 14,       SVG_chocolate		= 15,
    SVG_coral			= 16,       SVG_cornflower_blue		= 17,
    SVG_cornsilk		= 18,       SVG_crimson			= 19,
    SVG_cyan			= 20,       SVG_dark_blue		= 21,
    SVG_dark_cyan		= 22,       SVG_dark_goldenrod		= 23,
    SVG_dark_gray		= 24,       SVG_dark_green		= 25,
    SVG_dark_grey		= 26,       SVG_dark_khaki		= 27,
    SVG_dark_magenta		= 28,       SVG_dark_olive_green	= 29,
    SVG_dark_orange		= 30,       SVG_dark_orchid		= 31,
    SVG_dark_red		= 32,       SVG_dark_salmon		= 33,
    SVG_dark_sea_green		= 34,       SVG_dark_slate_blue		= 35,
    SVG_dark_slate_gray		= 36,       SVG_dark_slate_grey		= 37,
    SVG_dark_turquoise		= 38,       SVG_dark_violet		= 39,
    SVG_deep_pink		= 40,       SVG_deep_sky_blue		= 41,
    SVG_dim_gray		= 42,       SVG_dim_grey		= 43,
    SVG_dodger_blue		= 44,       SVG_fire_brick		= 45,
    SVG_floral_white		= 46,       SVG_forest_green		= 47,
    SVG_fuchsia			= 48,       SVG_gainsboro		= 49,
    SVG_ghost_white		= 50,       SVG_gold			= 51,
    SVG_goldenrod		= 52,       SVG_gray			= 53,
    SVG_grey			= 54,       SVG_green			= 55,
    SVG_green_yellow		= 56,       SVG_honey_dew		= 57,
    SVG_hot_pink		= 58,       SVG_indian_red		= 59,
    SVG_indigo			= 60,       SVG_ivory			= 61,
    SVG_khaki			= 62,       SVG_lavender		= 63,
    SVG_lavender_blush		= 64,       SVG_lawn_green		= 65,
    SVG_lemon_chiffon		= 66,       SVG_light_blue		= 67,
    SVG_light_coral		= 68,       SVG_light_cyan		= 69,
    SVG_light_goldenrod_yellow	= 70,       SVG_light_gray		= 71,
    SVG_light_green		= 72,       SVG_light_grey		= 73,
    SVG_light_pink		= 74,       SVG_light_salmon		= 75,
    SVG_light_sea_green		= 76,       SVG_light_sky_blue		= 77,
    SVG_light_slate_gray	= 78,       SVG_light_slate_grey	= 79,
    SVG_light_steel_blue	= 80,       SVG_light_yellow		= 81,
    SVG_lime			= 82,       SVG_lime_green		= 83,
    SVG_linen			= 84,       SVG_magenta			= 85,
    SVG_maroon			= 86,       SVG_medium_aquamarine	= 87,
    SVG_medium_blue		= 88,       SVG_medium_orchid		= 89,
    SVG_medium_purple		= 90,       SVG_medium_sea_green	= 91,
    SVG_medium_slate_blue	= 92,       SVG_medium_spring_green	= 93,
    SVG_medium_turquoise	= 94,       SVG_medium_violet_red	= 95,
    SVG_midnight_blue		= 96,       SVG_mint_cream		= 97,
    SVG_misty_rose		= 98,       SVG_moccasin		= 99,
    SVG_navajo_white		= 100,      SVG_navy			= 101,
    SVG_old_lace		= 102,      SVG_olive			= 103,
    SVG_olive_drab		= 104,      SVG_orange			= 105,
    SVG_orange_red		= 106,      SVG_orchid			= 107,
    SVG_pale_goldenrod		= 108,      SVG_pale_green		= 109,
    SVG_pale_turquoise		= 110,      SVG_pale_violet_red		= 111,
    SVG_papaya_whip		= 112,      SVG_peach_puff		= 113,
    SVG_peru			= 114,      SVG_pink			= 115,
    SVG_plum			= 116,      SVG_powder_blue		= 117,
    SVG_purple			= 118,      SVG_red			= 119,
    SVG_rosy_brown		= 120,      SVG_royal_blue		= 121,
    SVG_saddle_brown		= 122,      SVG_salmon			= 123,
    SVG_sandy_brown		= 124,      SVG_sea_green		= 125,
    SVG_sea_shell		= 126,      SVG_sienna			= 127,
    SVG_silver			= 128,      SVG_sky_blue		= 129,
    SVG_slate_blue		= 130,      SVG_slate_gray		= 131,
    SVG_slate_grey		= 132,      SVG_snow			= 133,
    SVG_spring_green		= 134,      SVG_steel_blue		= 135,
    SVG_tan			= 136,      SVG_teal			= 137,
    SVG_thistle			= 138,      SVG_tomato			= 139,
    SVG_turquoise		= 140,      SVG_violet			= 141,
    SVG_wheat			= 142,      SVG_white			= 143,
    SVG_white_smoke		= 144,      SVG_yellow			= 145,
    SVG_yellow_green		= 146
  } SVG_Color;

typedef enum {
    SVG_miter_join   = 0,       // Join Styles
    SVG_round_join   = 1,
    SVG_bevel_join   = 2
  } SVG_Join;

typedef enum {
    SVG_butt_cap      = 0,      // Cap Styles
    SVG_round_cap     = 1,
    SVG_square_cap    = 2
  } SVG_Cap;

typedef enum {
    SVG_no_arrow    = 0,        // Arrow Styles
    SVG_line_arrow  = 1,
    SVG_flat_arrow  = 2,
    SVG_plain_arrow = 3,
    SVG_flint_arrow = 4
  } SVG_Arrow;

typedef enum {
    SVG_serif		= 0,
    SVG_sans_serif	= 1,
    SVG_monospace	= 2,

    SVG_Arial		= 3,
    SVG_Arial_Black	= 4,
    SVG_Book_Antiqua	= 5,
    SVG_Charcoal	= 6,
    SVG_Courier		= 7,
    SVG_Gadget		= 8,
    SVG_Geneva		= 9,
    SVG_Georgia		= 10,
    SVG_Helvetica	= 11,
    SVG_Impact		= 12,
    SVG_Lucida_Console	= 13,
    SVG_Lucida_Grande	= 14,
    SVG_Monaco		= 15,
    SVG_Palatino	= 16,
    SVG_Tahoma		= 17,
    SVG_Times		= 18,
    SVG_Verdana		= 19,
  } SVG_Font;

typedef enum
  { SVG_plain        = 0,
    SVG_italic       = 1,
    SVG_bold         = 2,
    SVG_bold_italic  = 3
  } SVG_Style;

typedef enum
  { SVG_left_align    = 0,
    SVG_center_align  = 1,
    SVG_right_align   = 2
  } SVG_Align;

void SVG_Stroke_Off(SVG_Context *M(context));
void SVG_Stroke_Color(SVG_Context *M(context), SVG_Color color);             // default is SVG_black
void SVG_Stroke_RGB(SVG_Context *M(context), int red, int green, int blue);
void SVG_Stroke_Width(SVG_Context *M(context), double thick);                  // default is 1.0
void SVG_Stroke_Opacity(SVG_Context *M(context), double pcnt);                 // default is 1.0

void SVG_Join_Style(SVG_Context *M(context), SVG_Join style);    // default is SVG_miter_join
void SVG_Cap_Style(SVG_Context *M(context), SVG_Cap style);      // default is SVG_square_cap
void SVG_Miter_Limit(SVG_Context *M(context), double limit);     // default is 4.0

void SVG_Dash_Spec(SVG_Context *M(context), double offset, int n, double *dashlens);

void SVG_Fill_Off(SVG_Context *M(context));
void SVG_Fill_Color(SVG_Context *M(context), SVG_Color color);         // default is off
void SVG_Fill_RGB(SVG_Context *M(context), int red, int green, int blue);
void SVG_Fill_Opacity(SVG_Context *M(context), double pcnt);           // default is 0.0 (no fill)

  //  Set beg and end to 0 or 1 depending on whether you want an arrowhead
  //     at the start or the end of a path object

  // default is false,false,SVG_line_arrow,NULL,3.,6.

void SVG_Arrow_Mode(SVG_Context *M(context), boolean bin, SVG_Arrow beg,
                                             boolean ein, SVG_Arrow end);
void SVG_Arrow_Size(SVG_Context *M(context), double width, double height);

 // default is 12,SVG_times,SVG_plain,SVG_left_align,SVG_horizontal

void SVG_Text_Font(SVG_Context *M(context), SVG_Font font, SVG_Style style);
void SVG_Custom_Font(SVG_Context *M(context), string css_font, SVG_Style style);
void SVG_Point_Size(SVG_Context *M(context), double size);
void SVG_Text_Alignment(SVG_Context *M(context), SVG_Align align);

  //  Set roundness > 0 if you want rounded corners, the bigger the
  //    number the rounder the corners.

void SVG_Rectangle(SVG_Canvas *canvas, SVG_Frame *F(rectangle), double roundness);

  //  Draw a path

void Draw_SVG_Path(SVG_Canvas *canvas, SVG_Path *path);

  //  Draw an ellipse rotated by angle and fill it.

void SVG_Ellipse(SVG_Canvas *canvas, SVG_Point *F(center), SVG_Point *F(radii), double angle);

  //  Draw a circle of the given radius and fill it.

void SVG_Circle(SVG_Canvas *canvas, SVG_Point *F(center), double radius);

  // Text Fonts, Styles, and Alignment

void SVG_Text(SVG_Canvas *canvas, SVG_Point *anchor, double angle, string text);

#ifdef __cplusplus
}
#endif

#endif
