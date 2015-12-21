/* Copyright (C) 2005 The cairomm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef __CAIROMM_ENUMS_H
#define __CAIROMM_ENUMS_H

#include <cairo.h>

namespace Cairo
{

#ifndef DOXYGEN_IGNORE_THIS
//This is only used internally, but it must be in a public header because we inline some methods.
//Actually, it is used now by the UserFontFace set_*_func() slots, which are public. murrayc.
typedef cairo_status_t ErrorStatus;
#endif //DOXYGEN_IGNORE_THIS


typedef enum
{
    OPERATOR_CLEAR = CAIRO_OPERATOR_CLEAR,

    OPERATOR_SOURCE = CAIRO_OPERATOR_SOURCE,
    OPERATOR_OVER = CAIRO_OPERATOR_OVER,
    OPERATOR_IN = CAIRO_OPERATOR_IN,
    OPERATOR_OUT = CAIRO_OPERATOR_OUT,
    OPERATOR_ATOP = CAIRO_OPERATOR_ATOP,

    OPERATOR_DEST = CAIRO_OPERATOR_DEST,
    OPERATOR_DEST_OVER = CAIRO_OPERATOR_DEST_OVER,
    OPERATOR_DEST_IN = CAIRO_OPERATOR_DEST_IN,
    OPERATOR_DEST_OUT = CAIRO_OPERATOR_DEST_OUT,
    OPERATOR_DEST_ATOP = CAIRO_OPERATOR_DEST_ATOP,

    OPERATOR_XOR = CAIRO_OPERATOR_XOR,
    OPERATOR_ADD = CAIRO_OPERATOR_ADD,
    OPERATOR_SATURATE = CAIRO_OPERATOR_SATURATE
} Operator;


typedef enum
{
    ANTIALIAS_DEFAULT = CAIRO_ANTIALIAS_DEFAULT,
    ANTIALIAS_NONE = CAIRO_ANTIALIAS_NONE,
    ANTIALIAS_GRAY = CAIRO_ANTIALIAS_GRAY,
    ANTIALIAS_SUBPIXEL = CAIRO_ANTIALIAS_SUBPIXEL
} Antialias;

typedef enum
{
    FILL_RULE_WINDING = CAIRO_FILL_RULE_WINDING,
    FILL_RULE_EVEN_ODD = CAIRO_FILL_RULE_EVEN_ODD
} FillRule;


typedef enum
{
    LINE_CAP_BUTT = CAIRO_LINE_CAP_BUTT,
    LINE_CAP_ROUND = CAIRO_LINE_CAP_ROUND,
    LINE_CAP_SQUARE = CAIRO_LINE_CAP_SQUARE
} LineCap;


typedef enum
{
    LINE_JOIN_MITER = CAIRO_LINE_JOIN_MITER,
    LINE_JOIN_ROUND = CAIRO_LINE_JOIN_ROUND,
    LINE_JOIN_BEVEL = CAIRO_LINE_JOIN_BEVEL
} LineJoin;


typedef enum
{
  FONT_SLANT_NORMAL = CAIRO_FONT_SLANT_NORMAL,
  FONT_SLANT_ITALIC = CAIRO_FONT_SLANT_ITALIC,
  FONT_SLANT_OBLIQUE = CAIRO_FONT_SLANT_OBLIQUE
} FontSlant;

typedef enum
{
  FONT_WEIGHT_NORMAL = CAIRO_FONT_WEIGHT_NORMAL,
  FONT_WEIGHT_BOLD = CAIRO_FONT_WEIGHT_BOLD
} FontWeight;


typedef enum
{
    CONTENT_COLOR = CAIRO_CONTENT_COLOR,
    CONTENT_ALPHA = CAIRO_CONTENT_ALPHA,
    CONTENT_COLOR_ALPHA = CAIRO_CONTENT_COLOR_ALPHA
} Content;


typedef enum
{
    FORMAT_ARGB32 = CAIRO_FORMAT_ARGB32,
    FORMAT_RGB24 = CAIRO_FORMAT_RGB24,
    FORMAT_A8 = CAIRO_FORMAT_A8,
    FORMAT_A1 = CAIRO_FORMAT_A1,
    FORMAT_RGB16_565 = CAIRO_FORMAT_RGB16_565 /* @< @deprecated This format value is deprecated. It has never been properly implemented in cairo and is unnecessary. */
} Format;


typedef enum
{
    EXTEND_NONE = CAIRO_EXTEND_NONE,
    EXTEND_REPEAT = CAIRO_EXTEND_REPEAT,
    EXTEND_REFLECT = CAIRO_EXTEND_REFLECT,
    EXTEND_PAD = CAIRO_EXTEND_PAD
} Extend;


typedef enum
{
    FILTER_FAST = CAIRO_FILTER_FAST,
    FILTER_GOOD = CAIRO_FILTER_GOOD,
    FILTER_BEST = CAIRO_FILTER_BEST,
    FILTER_NEAREST = CAIRO_FILTER_NEAREST,
    FILTER_BILINEAR = CAIRO_FILTER_BILINEAR,
    FILTER_GAUSSIAN = CAIRO_FILTER_GAUSSIAN
} Filter;

/**
 * The subpixel order specifies the order of color elements within each pixel on
 * the display device when rendering with an antialiasing mode of
 * ANTIALIAS_SUBPIXEL.
 **/
typedef enum
{
    SUBPIXEL_ORDER_DEFAULT = CAIRO_SUBPIXEL_ORDER_DEFAULT, /**< Use the default subpixel order for for the target device */
    SUBPIXEL_ORDER_RGB = CAIRO_SUBPIXEL_ORDER_RGB, /**< Subpixel elements are arranged horizontally with red at the left */
    SUBPIXEL_ORDER_BGR = CAIRO_SUBPIXEL_ORDER_BGR, /**<  Subpixel elements are arranged horizontally with blue at the left */
    SUBPIXEL_ORDER_VRGB = CAIRO_SUBPIXEL_ORDER_VRGB, /**< Subpixel elements are arranged vertically with red at the top */
    SUBPIXEL_ORDER_VBGR = CAIRO_SUBPIXEL_ORDER_VBGR /**< Subpixel elements are arranged vertically with blue at the top */
} SubpixelOrder;


/**
 * Specifies the type of hinting to do on font outlines. Hinting is the process
 * of fitting outlines to the pixel grid in order to improve the appearance of
 * the result. Since hinting outlines involves distorting them, it also reduces
 * the faithfulness to the original outline shapes. Not all of the outline
 * hinting styles are supported by all font backends.
 *
 * New entries may be added in future versions.
 **/
typedef enum
{
    HINT_STYLE_DEFAULT = CAIRO_HINT_STYLE_DEFAULT, /**< Use the default hint style for font backend and target device */
    HINT_STYLE_NONE = CAIRO_HINT_STYLE_NONE, /**< Do not hint outlines */
    HINT_STYLE_SLIGHT = CAIRO_HINT_STYLE_SLIGHT, /**< Hint outlines slightly to improve contrast while retaining good fidelity to the original shapes. */
    HINT_STYLE_MEDIUM = CAIRO_HINT_STYLE_MEDIUM, /**< Hint outlines with medium strength giving a compromise between fidelity to the original shapes and contrast */
    HINT_STYLE_FULL = CAIRO_HINT_STYLE_FULL /**< Hint outlines to maximize contrast */
} HintStyle;


/**
 * Specifies whether to hint font metrics; hinting font metrics means quantizing
 * them so that they are integer values in device space. Doing this improves the
 * consistency of letter and line spacing, however it also means that text will
 * be laid out differently at different zoom factors.
 **/
typedef enum
{
    HINT_METRICS_DEFAULT = CAIRO_HINT_METRICS_DEFAULT,
    /**< Hint metrics in the default manner for the font backend and target device */
    HINT_METRICS_OFF = CAIRO_HINT_METRICS_OFF,
    /**< Do not hint font metrics */
    HINT_METRICS_ON = CAIRO_HINT_METRICS_ON
    /**< Hint font metrics */
} HintMetrics;

typedef enum
{
    SURFACE_TYPE_IMAGE = CAIRO_SURFACE_TYPE_IMAGE,
    SURFACE_TYPE_PDF = CAIRO_SURFACE_TYPE_PDF,
    SURFACE_TYPE_PS = CAIRO_SURFACE_TYPE_PS,
    SURFACE_TYPE_XLIB = CAIRO_SURFACE_TYPE_XLIB,
    SURFACE_TYPE_XCB = CAIRO_SURFACE_TYPE_XCB,
    SURFACE_TYPE_GLITZ = CAIRO_SURFACE_TYPE_GLITZ,
    SURFACE_TYPE_QUARTZ = CAIRO_SURFACE_TYPE_QUARTZ,
    SURFACE_TYPE_WIN32 = CAIRO_SURFACE_TYPE_WIN32,
    SURFACE_TYPE_BEOS = CAIRO_SURFACE_TYPE_BEOS,
    SURFACE_TYPE_DIRECTFB = CAIRO_SURFACE_TYPE_DIRECTFB,
    SURFACE_TYPE_SVG = CAIRO_SURFACE_TYPE_SVG,
    SURFACE_TYPE_OS2 = CAIRO_SURFACE_TYPE_OS2,
    SURFACE_TYPE_WIN32_PRINTING = CAIRO_SURFACE_TYPE_WIN32_PRINTING,
    SURFACE_TYPE_QUARTZ_IMAGE = CAIRO_SURFACE_TYPE_QUARTZ_IMAGE
} SurfaceType;

typedef enum
{
    PATTERN_TYPE_SOLID = CAIRO_PATTERN_TYPE_SOLID,
    PATTERN_TYPE_SURFACE = CAIRO_PATTERN_TYPE_SURFACE,
    PATTERN_TYPE_LINEAR = CAIRO_PATTERN_TYPE_LINEAR,
    PATTERN_TYPE_RADIAL = CAIRO_PATTERN_TYPE_RADIAL
} PatternType;

typedef enum
{
    FONT_TYPE_TOY = CAIRO_FONT_TYPE_TOY,
    FONT_TYPE_FT = CAIRO_FONT_TYPE_FT,
    FONT_TYPE_WIN32 = CAIRO_FONT_TYPE_WIN32,
    FONT_TYPE_ATSUI = CAIRO_FONT_TYPE_QUARTZ, /**< @deprecated Use FONT_TYPE_QUARTZ instead. */
    FONT_TYPE_QUARTZ = CAIRO_FONT_TYPE_QUARTZ,
    FONT_TYPE_USER = CAIRO_FONT_TYPE_USER
} FontType;

/** Specifies properties of a text cluster mapping.
 *
 * @since 1.8
 **/
typedef enum
{
    TEXT_CLUSTER_FLAG_BACKWARD = CAIRO_TEXT_CLUSTER_FLAG_BACKWARD /**< The clusters in the cluster array map to glyphs in the glyph array from end to start. */
} TextClusterFlags;

} // namespace Cairo

#endif //__CAIROMM_ENUMS_H

// vim: ts=2 sw=2 et
