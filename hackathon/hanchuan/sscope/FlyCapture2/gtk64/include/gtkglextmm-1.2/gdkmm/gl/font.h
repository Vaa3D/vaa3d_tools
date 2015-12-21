// -*- C++ -*-
/* gdkglextmm - C++ Wrapper for GdkGLExt
 * Copyright (C) 2002-2003  Naofumi Yasufuku
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

#ifndef _GDKMM_GL_FONT_H
#define _GDKMM_GL_FONT_H

#include <glibmm.h>

#include <gdkmm/gl/defs.h>

#ifdef GDKGLEXTMM_MULTIHEAD_SUPPORT
#include <gdkmm/display.h>
#endif // GDKGLEXTMM_MULTIHEAD_SUPPORT

#include <pangomm/font.h>

namespace Gdk
{
  namespace GL
  {

    /** Font rendering support stuff.
     *
     *
     */

    class Font
    {
    public:
      virtual ~Font() = 0;

    public:

      /** Create bitmap display lists from a PangoFont.
       *
       * @param desc       a Pango::FontDescription describing the font to use.
       * @param first      the index of the first glyph to be taken.
       * @param count      the number of glyphs to be taken.
       * @param list_base  the index of the first display list to be generated.
       * @return  the Pango::Font used, or NULL RefPtr if no font matched.
       */
      static Glib::RefPtr<Pango::Font> use_pango_font(const Pango::FontDescription& desc,
                                                      int first,
                                                      int count,
                                                      int list_base);

#ifdef GDKGLEXTMM_MULTIHEAD_SUPPORT
      static Glib::RefPtr<Pango::Font> use_pango_font(const Glib::RefPtr<const Gdk::Display>& display,
                                                      const Pango::FontDescription& desc,
                                                      int first,
                                                      int count,
                                                      int list_base);
#endif // GDKGLEXTMM_MULTIHEAD_SUPPORT

    };

    /** @example font.cc
     *
     * Simple bitmap font rendering example.
     *
     */

  } // namespace GL
} // namespace Gdk

#endif // _GDKMM_GL_FONT_H
