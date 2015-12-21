/* Copyright (C) 2008 Jonathon Jongsma
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
#ifndef __CAIROMM_QUARTZ_FONT_H
#define __CAIROMM_QUARTZ_FONT_H

#include <cairo-features.h>

#ifdef CAIRO_HAS_QUARTZ_FONT
#include <cairo-quartz.h>
#include <cairomm/fontface.h>
#include <cairomm/scaledfont.h>

namespace Cairo
{

/** Font support for Quartz (CGFont) fonts on OS X.
 *
 * @since 1.8
 */
class QuartzFontFace : public FontFace
{
public:

  /** Creates a new font for the Quartz font backend based on a CGFontRef. This
   * font can then be used with Context::set_font_face() or
   * ScaledFont::create().
   *
   * @param font a CGFontRef obtained through a method external to cairo.
   *
   * @since 1.8
   */
  static RefPtr<QuartzFontFace> create(CGFontRef font);

  /** Creates a new font for the Quartz font backend based on an ATSUFontID.
   * This font can then be used with Context::set_font_face() or
   * ScaledFont::create().
   *
   * @param font_id an ATSUFontID for the font.
   *
   * @since 1.8
   */
  static RefPtr<QuartzFontFace> create(ATSUFontID font_id);


protected:
  QuartzFontFace(CGFontRef font);
  QuartzFontFace(ATSUFontID font_id);
};

}

#endif // CAIRO_HAS_QUARTZ_FONT

#endif // __CAIROMM_QUARTZ_FONT_H
