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

#ifndef _GTKMM_GL_WIDGET_H
#define _GTKMM_GL_WIDGET_H

#include <gtkmm/widget.h>

#include <gdkmm/gl/tokens.h>
#include <gdkmm/gl/config.h>
#include <gdkmm/gl/context.h>
#include <gdkmm/gl/window.h>

namespace Gtk
{
  namespace GL
  {

    /** Set the OpenGL-capability to the Gtk::Widget.
     * The call setup the callbacks to realize a OpenGL-capable window.
     *
     * @param widget       the GtkWidget to be used as the rendering area.
     * @param glconfig     a Gdk::GL::Config.
     * @param share_list   the Gdk::GL::Context which to share display lists.
     * @param direct       whether rendering is to be done with a direct
     *                     connection to the graphics system.
     * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
     *                     (currently not used).
     * @return  <tt>true</tt> if it is successful, <tt>false</tt> otherwise.
     */
    bool widget_set_gl_capability(Gtk::Widget& widget,
                                  const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                                  const Glib::RefPtr<const Gdk::GL::Context>& share_list,
                                  bool direct = true,
                                  int render_type = Gdk::GL::RGBA_TYPE);

    /** Set the OpenGL-capability to the Gtk::Widget.
     * The call setup the callbacks to realize a OpenGL-capable window.
     *
     * @param widget       the GtkWidget to be used as the rendering area.
     * @param glconfig     a Gdk::GL::Config.
     * @param direct       whether rendering is to be done with a direct
     *                     connection to the graphics system.
     * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
     *                     (currently not used).
     * @return  <tt>true</tt> if it is successful, <tt>false</tt> otherwise.
     */
    bool widget_set_gl_capability(Gtk::Widget& widget,
                                  const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                                  bool direct = true,
                                  int render_type = Gdk::GL::RGBA_TYPE);

    /** Return whether the Gtk::Widget is OpenGL-capable.
     *
     * @param widget  a Gtk::Widget.
     * @return  <tt>true</tt> if the Gtk::Widget is OpenGL-capable,
     *          <tt>false</tt> otherwise.
     */
    bool widget_is_gl_capable(const Gtk::Widget& widget);

    /** Return the Gdk::GL::Config referred by the Gtk::Widget.
     *
     * @param widget  a Gtk::Widget.
     * @return  the Gdk::GL::Config.
     */
    Glib::RefPtr<Gdk::GL::Config> widget_get_gl_config(const Gtk::Widget& widget);

    /** Create a new Gdk::GL::Context with the appropriate Gdk::GL::Drawable
     * for this widget.
     *
     * See also get_gl_context().
     *
     * @param widget       a Gtk::Widget.
     * @param share_list   the Gdk::GL::Context which to share display lists.
     * @param direct       whether rendering is to be done with a direct
     *                     connection to the graphics system.
     * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
     *                     (currently not used).
     * @return  the new Gdk::GL::Context.
     */
    Glib::RefPtr<Gdk::GL::Context> widget_create_gl_context(const Gtk::Widget& widget,
                                                            const Glib::RefPtr<const Gdk::GL::Context>& share_list,
                                                            bool direct = true,
                                                            int render_type = Gdk::GL::RGBA_TYPE);

    /** Create a new Gdk::GL::Context with the appropriate Gdk::GL::Drawable
     * for this widget.
     *
     * See also get_gl_context().
     *
     * @param widget       a Gtk::Widget.
     * @param direct       whether rendering is to be done with a direct
     *                     connection to the graphics system.
     * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
     *                     (currently not used).
     * @return  the new Gdk::GL::Context.
     */
    Glib::RefPtr<Gdk::GL::Context> widget_create_gl_context(const Gtk::Widget& widget,
                                                            bool direct = true,
                                                            int render_type = Gdk::GL::RGBA_TYPE);

    /** Return the Gdk::GL::Context with the appropriate Gdk::GL::Drawable
     * for this widget.
     *
     * Unlike the GL context returned by create_gl_context(), this context
     * is owned by the widget.
     *
     * @param widget  a Gtk::Widget.
     * @return  the Gdk::GL::Context.
     */
    Glib::RefPtr<Gdk::GL::Context> widget_get_gl_context(const Gtk::Widget& widget);

    /** Return the Gdk::GL::Window owned by the Gtk::Widget.
     *
     * @param widget  a Gtk::Widget.
     * @return  the Gdk::GL::Window.
     */
    Glib::RefPtr<Gdk::GL::Window> widget_get_gl_window(const Gtk::Widget& widget);

    /** Return the Gdk::GL::Drawable owned by the Gtk::Widget.
     *
     * @param widget  a Gtk::Widget.
     * @return  the Gdk::GL::Drawable.
     */
    Glib::RefPtr<Gdk::GL::Drawable> widget_get_gl_drawable(const Gtk::Widget& widget);

    /** OpenGL extension API to Gtk::Widget.
     *
     *
     */

    template <class T_GtkWidget>
    class Widget
    {
    public:
      virtual ~Widget() = 0;

    public:

      /** Set the OpenGL-capability to the Gtk::Widget.
       * The call setup the callbacks to realize a OpenGL-capable window.
       *
       * @param glconfig     a Gdk::GL::Config.
       * @param share_list   the Gdk::GL::Context which to share display lists.
       * @param direct       whether rendering is to be done with a direct
       *                     connection to the graphics system.
       * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
       *                     (currently not used).
       * @return  <tt>true</tt> if it is successful, <tt>false</tt> otherwise.
       */
      bool set_gl_capability(const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                             const Glib::RefPtr<const Gdk::GL::Context>& share_list,
                             bool direct = true,
                             int render_type = Gdk::GL::RGBA_TYPE)
      { return widget_set_gl_capability(*static_cast<T_GtkWidget*>(this), glconfig, share_list, direct, render_type); }

      /** Set the OpenGL-capability to the Gtk::Widget.
       * The call setup the callbacks to realize a OpenGL-capable window.
       *
       * @param glconfig     a Gdk::GL::Config.
       * @param direct       whether rendering is to be done with a direct
       *                     connection to the graphics system.
       * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
       *                     (currently not used).
       * @return  <tt>true</tt> if it is successful, <tt>false</tt> otherwise.
       */
      bool set_gl_capability(const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                             bool direct = true,
                             int render_type = Gdk::GL::RGBA_TYPE)
      { return widget_set_gl_capability(*static_cast<T_GtkWidget*>(this), glconfig, direct, render_type); }

      /** Return whether the Gtk::Widget is OpenGL-capable.
       *
       * @return  <tt>true</tt> if the Gtk::Widget is OpenGL-capable,
       *          <tt>false</tt> otherwise.
       */
      bool is_gl_capable() const
      { return widget_is_gl_capable(*static_cast<const T_GtkWidget*>(this)); }

      /** Return the Gdk::GL::Config referred by the Gtk::Widget.
       *
       * @return  the Gdk::GL::Config.
       */
      Glib::RefPtr<Gdk::GL::Config> get_gl_config()
      { return widget_get_gl_config(*static_cast<T_GtkWidget*>(this)); }

      /** Return the Gdk::GL::Config referred by the Gtk::Widget.
       *
       * @return  the Gdk::GL::Config.
       */
      Glib::RefPtr<const Gdk::GL::Config> get_gl_config() const
      { return widget_get_gl_config(*static_cast<const T_GtkWidget*>(this)); }

      /** Create a new Gdk::GL::Context with the appropriate Gdk::GL::Drawable
       * for this widget.
       *
       * See also get_gl_context().
       *
       * @param share_list   the Gdk::GL::Context which to share display lists.
       * @param direct       whether rendering is to be done with a direct
       *                     connection to the graphics system.
       * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
       *                     (currently not used).
       * @return  the new Gdk::GL::Context.
       */
      Glib::RefPtr<Gdk::GL::Context> create_gl_context(const Glib::RefPtr<const Gdk::GL::Context>& share_list,
                                                       bool direct = true,
                                                       int render_type = Gdk::GL::RGBA_TYPE)
      { return widget_create_gl_context(*static_cast<T_GtkWidget*>(this), share_list, direct, render_type); }

      /** Create a new Gdk::GL::Context with the appropriate Gdk::GL::Drawable
       * for this widget.
       *
       * See also get_gl_context().
       *
       * @param direct       whether rendering is to be done with a direct
       *                     connection to the graphics system.
       * @param render_type  Gdk::GL::RGBA_TYPE or Gdk::GL::COLOR_INDEX_TYPE
       *                     (currently not used).
       * @return  the new Gdk::GL::Context.
       */
      Glib::RefPtr<Gdk::GL::Context> create_gl_context(bool direct = true,
                                                       int render_type = Gdk::GL::RGBA_TYPE)
      { return widget_create_gl_context(*static_cast<T_GtkWidget*>(this), direct, render_type); }

      /** Return the Gdk::GL::Context with the appropriate Gdk::GL::Drawable
       * for this widget.
       *
       * Unlike the GL context returned by create_gl_context(), this context
       * is owned by the widget.
       *
       * @return  the Gdk::GL::Context.
       */
      Glib::RefPtr<Gdk::GL::Context> get_gl_context()
      { return widget_get_gl_context(*static_cast<T_GtkWidget*>(this)); }

      /** Return the Gdk::GL::Window owned by the Gtk::Widget.
       *
       * @return  the Gdk::GL::Window.
       */
      Glib::RefPtr<Gdk::GL::Window> get_gl_window()
      { return widget_get_gl_window(*static_cast<T_GtkWidget*>(this)); }

      /** Return the Gdk::GL::Window owned by the Gtk::Widget.
       *
       * @return  the Gdk::GL::Window.
       */
      Glib::RefPtr<const Gdk::GL::Window> get_gl_window() const
      { return widget_get_gl_window(*static_cast<const T_GtkWidget*>(this)); }

      /** Return the Gdk::GL::Drawable owned by the Gtk::Widget.
       *
       * @return  the Gdk::GL::Drawable.
       */
      Glib::RefPtr<Gdk::GL::Drawable> get_gl_drawable()
      { return widget_get_gl_drawable(*static_cast<T_GtkWidget*>(this)); }

      /** Return the Gdk::GL::Drawable owned by the Gtk::Widget.
       *
       * @return  the Gdk::GL::Drawable.
       */
      Glib::RefPtr<const Gdk::GL::Drawable> get_gl_drawable() const
      { return widget_get_gl_drawable(*static_cast<const T_GtkWidget*>(this)); }

    };

    template <class T_GtkWidget>
    Widget<T_GtkWidget>::~Widget()
    {}

    /** @example simple.cc
     *
     * Simple gtkglextmm example.
     *
     */

    /** @example simple-mixed.cc
     *
     * Simple gtkglextmm example for mixing OpenGL and GDK rendering.
     *
     */

  } // namespace GL
} // namespace Gtk

#endif // _GTKMM_GL_WIDGET_H
