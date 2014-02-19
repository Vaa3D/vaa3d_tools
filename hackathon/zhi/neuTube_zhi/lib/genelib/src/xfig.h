/*****************************************************************************************\
*                                                                                         *
*  Xfig drawing package                                                                   *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2006                                                                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _XFIG_HEADER

#define _XFIG_HEADER

#include "utilities.h"

typedef struct
  { double x, y;
  } Point;

Point     *Make_Point(double x, double y);
Point     *Copy_Point(Point *point);
void       Free_Point(Point *point);
void       Kill_Point(Point *point);
void       Reset_Point();
int        Point_Usage();

typedef struct
  { double w, h;
  } Box;

Box       *Make_Box(double w, double h);
Box       *Copy_Box(Box *box);
void       Free_Box(Box *box);
void       Kill_Box(Box *box);
void       Reset_Box();
int        Box_Usage();

typedef struct
  { Point corner;
    Box   window; 
  } Frame;

Frame     *Make_Frame(Point *corner, Box *window);
Frame     *Copy_Frame(Frame *frame);
void       Free_Frame(Frame *frame);
void       Kill_Frame(Frame *frame);
void       Reset_Frame();
int        Frame_Usage();

typedef struct
  { double xscale, xoffset;
    double yscale, yoffset;
  } Transform;

Transform *Make_Transform(Frame *from, Frame *to);
Transform *Copy_Transform(Transform *xform);
void       Free_Transform(Transform *xform);
void       Kill_Transform(Transform *xform);
void       Reset_Transform();
int        Transform_Usage();


/* Establishes a new xfig drawing that will placed in the file <name>.fig.
   Has the side effect of setting the current canvas to the one that is
   beginning.  The target frame is assumed to be in *inches.*               */

typedef void Xfig_Canvas;

Xfig_Canvas *Begin_Xfig_Drawing(char *name, Transform *xform);

void Set_Current_Canvas(Xfig_Canvas *canvas);

/* Finishes the creation of the Xfig file closing underlying IO, etc.     */

void Finish_Xfig_Drawing(Xfig_Canvas *canvas);

/* All primitives below affect the graphic state of the current canvas    */

#define BLACK    0   // Pre-defined colors (holes are Green2-4, Cyan2-4, etc.)
#define BLUE     1
#define GREEN    2
#define CYAN     3
#define RED      4
#define MAGENTA  5
#define YELLOW   6
#define WHITE    7
#define LT_BLUE 11
#define BROWN   24
#define PINK    30
#define GOLD    31

/* Add a user defined color to the given canvases palette
     and return code for the new color.                    */

int Add_Color(double red, double green, double blue);   // rgb values in [0,1]

#define SOLID             0    // Line Styles
#define DASH              1
#define DOT               2
#define DASH_DOT          3
#define DASH_DOT_DOT      4
#define DASH_DOT_DOT_DOT  5

void Set_Pen_Color(int color);                           // default is BLACK
void Set_Thickness(int thick);                           // default is 1
void Set_Line_Style(int line_style, double dash_width);  // default is SOLID, 3.0

#define POINTED_JOIN   0       // Join Styles
#define ROUNDED_JOIN   1
#define SQUARED_JOIN   2

#define SQUARED_CAP    0       // Cap Styles
#define ROUNDED_CAP    1
#define CENTERED_CAP   2

void Set_Join_Style(int join_style);   // default is POINTED
void Set_Cap_Style(int cap_style);     // default is SQUARED

#define NO_FILL      -1
#define FULLY_SHADED  0  // Black       // Between 0 and 40 you get a continuum
#define HALF_SHADED  10
#define SOLID_COLOR  20  // Solid Color
#define HALF_CLEAR   30
#define FULLY_CLEAR  40  // White
#define LAST_PATTEN  62                // See the Xfig palette for the defined patterns

/* Fill Style sets a translucency level for the fill or
     turns filling off if the number is less than 0.      */

void Set_Fill_Style(int style);   // default is off
void Set_Fill_Color(int color);   // default is WHITE

void Set_Depth(int depth);        // default is 50

#define LINE_STYLE  0             // Arrow Styles
#define FLAT_BASE   1
#define ARROW       2
#define FLINT       3

/*  Set beg and end to 0 or 1 depending on whether you want an arrowhead
       at the start or the end of a poly-line object                      */

void Set_Arrow_Mode(int beg, int end);                             // default is 0,0
void Set_Arrow_Style(int a_style, int fill);                       // default is LINE_STYLE, 0
void Set_Arrow_Sizes(double thick, double width, double height);   // default is 1.,120.,60.

/*  Set roundness > 0 if you want rounded corners, the bigger the
      number the rounder the corners.                                */

void Etch_Rectangle(Point *ul_corner, Point *lr_corner, int roundness);

/*  To draw an open or closed poly-line do First(Next)*Last sequence of calls.
    If closed, then a line is drawn from the last point to the first and the
    object is filled.                                                             */

void First_Line_Point(Point *p);
void Next_Line_Point(Point *p);
void Last_Line_Point(Point *p, int closed);

void Etch_Ellipse(Point *center, double xradius, double yradius, double angle);
void Etch_Circle(Point *center, double radius);

  // Text Fonts

#define TIMES                   0
#define AVANT_GARDE             4
#define BOOKMAN                 8
#define COURIER                12
#define HELVETICA              16
#define HELVETICA_NARROW       20
#define NEW_CENTURY_SCHOOLBOOK 24
#define PALATINO               28
#define SYMBOL                 32    // Styles do not apply

   // Latex Fonts

#define LATEX_ROMAN 33         // Style BOLD_ITALIC does not apply
#define SANS_SERIF  36         // Styles do not apply
#define TYPEWRITER  37         // Styles do not apply

   // Styles

#define PLAIN        0
#define ITALIC       1
#define BOLD         2
#define BOLD_ITALIC  3

  // Alignment of text (w.r.t anchor point)

#define LEFT_ALIGN    0
#define CENTER_ALIGN  1
#define RIGHT_ALIGN   2

void Set_Point_Size(int size);          // default is 12
void Set_Font(int font, int style);     // default is TIMES,PLAIN
void Set_Alignment(int alignment);      // default is LEFT_ALIGN

void Etch_Text(Point *anchor, double angle, char *text);

#endif
