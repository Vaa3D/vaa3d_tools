// -*- C++ -*-
/* gtkglextmm - C++ Wrapper for GtkGLExt
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

#ifndef _GTKMM_GL_DRAWINGAREA_H
#define _GTKMM_GL_DRAWINGAREA_H

#include <gtkmm/drawingarea.h>

#include <gtkmm/gl/widget.h>

namespace Gtk
{
  namespace GL
  {

    /** OpenGL-capable DrawingArea widget.
     *
     *
     */

    class DrawingArea : public Gtk::DrawingArea,
                        public Gtk::GL::Widget<DrawingArea>
    {
    private:
      // noncopyable
      DrawingArea(const DrawingArea&);
      DrawingArea& operator=(const DrawingArea&);

    public:
      virtual ~DrawingArea();

    public:

      /** Create an DrawingArea widget which supports OpenGL rendering.
       * This constructor does *NOT* call set_gl_capability() method,
       * so that you should call it yourself to make instantiated DrawingArea
       * OpenGL-capable.
       *
       */
      DrawingArea() {}

      /** Create an OpenGL-capable DrawingArea widget.
       *
       * @param glconfig     a Gdk::GL::Config.
       * @param share_list   the Gdk::GL::Context which to share display lists.
       * @param direct       whether rendering is to be done with a direct
       *                     connection to the graphics system.
       * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
       *                     (currently not used).
       */
      DrawingArea(const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                  const Glib::RefPtr<const Gdk::GL::Context>& share_list,
                  bool direct = true,
                  int render_type = Gdk::GL::RGBA_TYPE)
      { set_gl_capability(glconfig, share_list, direct, render_type); }

      /** Create an OpenGL-capable DrawingArea widget.
       *
       * @param glconfig     a Gdk::GL::Config.
       * @param direct       whether rendering is to be done with a direct
       *                     connection to the graphics system.
       * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
       *                     (currently not used).
       */
      DrawingArea(const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                  bool direct = true,
                  int render_type = Gdk::GL::RGBA_TYPE)
      { set_gl_capability(glconfig, direct, render_type); }

    };

    /** @example simple-darea.cc
     *
     * Simple Gtk::GL::DrawingArea example.
     *
     */

    /** @example share-lists.cc
     *
     * Simple display list sharing example.
     *
     */

    /** @example logo.h
     *
     * GtkGLExt logo demo.
     *
     */

    /** @example logo.cc
     *
     * GtkGLExt logo demo.
     *
     */

    /** @example gears.cc
     *
     * 3-D gear wheels demo.
     *
     */

  } // namespace GL
} // namespace Gtk

#endif // _GTKMM_GL_DRAWINGAREA_H
