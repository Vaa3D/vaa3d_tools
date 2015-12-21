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

#ifndef _GDKMM_GL_WINDOWEXT_H
#define _GDKMM_GL_WINDOWEXT_H

#include <glibmm.h>

#include <gdkmm/gl/defs.h>
#include <gdkmm/gl/window.h>

namespace Gdk
{
  namespace GL
  {

    class WindowExtProxy
    {
    private:
      WindowExtProxy(const Glib::RefPtr<Gdk::Window>& window)
        : window_(window)
      {}

    private:
      WindowExtProxy(const WindowExtProxy&);
      WindowExtProxy& operator=(const WindowExtProxy&);

    public:

      /** Set the OpenGL-capability to the Gdk::Window.
       * This function creates a new Gdk::GL::Window held by the Gdk::Window.
       * attrib_list is currently unused. This must be set to <tt>0</tt> or empty
       * (first attribute of None).
       *
       * @param glconfig A Gdk::GL::Config.
       * @param attrib_list This must be set to <tt>0</tt> or empty (first attribute of None).
       * @return The Gdk::GL::Window used by the @a window  if it is successful,
       * <tt>0</tt> otherwise.
       */
      Glib::RefPtr<Window> set_gl_capability(const Glib::RefPtr<const Config>& glconfig,
                                             const int* attrib_list = 0);

      /** Unset the OpenGL-capability of the Gdk::Window.
       * This function destroys the Gdk::GL::Window held by the Gdk::Window.
       *
       */
      void unset_gl_capability();

      /** Returns whether the Gdk::Window is OpenGL-capable.
       *
       * @return <tt>true</tt> if the Gdk::Window is OpenGL-capable, <tt>false</tt> otherwise.
       */
      bool is_gl_capable() const;

      /** Returns the Gdk::GL::Window held by the Gdk::Window .
       *
       * @return The Gdk::GL::Window.
       */
      Glib::RefPtr<Window> get_gl_window() const;

      /** Returns the Gdk::GL::Drawable held by the Gdk::Window .
       *
       * @return The Gdk::GL::Drawable.
       */
      Glib::RefPtr<Gdk::GL::Drawable> get_gl_drawable() const;

    private:
      const Glib::RefPtr<Gdk::Window>& window_;

      friend WindowExtProxy ext(const Glib::RefPtr<Gdk::Window>&);
      friend const WindowExtProxy ext(const Glib::RefPtr<const Gdk::Window>&);
    };

    /** OpenGL extension API wrapper.
     * This is a Gdk::Window wrapper to invoke OpenGL extension function.
     * See Gdk::GL::WindowExtProxy for available functions.
     *
     * Here is a simple example:
     * @code
     * Glib::RefPtr<Gdk::Window> window = widget.get_window();
     * Glib::RefPtr<Gdk::GL::Window> glwindow =
     *   Gdk::GL::ext(window).set_gl_capability(glconfig);
     * std::cout << "OpenGL-capable? "
     *           << Gdk::GL::ext(window).is_gl_capable()
     *           << std::endl;
     * @endcode
     *
     * @param window A Gdk::Window.
     */
    inline
    WindowExtProxy ext(const Glib::RefPtr<Gdk::Window>& window)
    { return WindowExtProxy(window); }

    /** OpenGL extension API wrapper.
     * This is a Gdk::Window wrapper to invoke OpenGL extension function.
     * See Gdk::GL::WindowExtProxy for available functions.
     *
     * @param window A Gdk::Window.
     */
    inline
    const WindowExtProxy ext(const Glib::RefPtr<const Gdk::Window>& window)
    { return WindowExtProxy(reinterpret_cast<const Glib::RefPtr<Gdk::Window>&>(window)); }

  } // namespace GL
} // namespace Gdk

#endif // _GDKMM_GL_WINDOWEXT_H
