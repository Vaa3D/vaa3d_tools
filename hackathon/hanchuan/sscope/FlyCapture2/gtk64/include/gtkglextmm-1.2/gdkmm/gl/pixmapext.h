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

#ifndef _GDKMM_GL_PIXMAPEXT_H
#define _GDKMM_GL_PIXMAPEXT_H

#include <glibmm.h>

#include <gdkmm/gl/defs.h>
#include <gdkmm/gl/pixmap.h>

namespace Gdk
{
  namespace GL
  {

    class PixmapExtProxy
    {
    private:
      PixmapExtProxy(const Glib::RefPtr<Gdk::Pixmap>& pixmap)
        : pixmap_(pixmap)
      {}

    private:
      PixmapExtProxy(const PixmapExtProxy&);
      PixmapExtProxy& operator=(const PixmapExtProxy&);

    public:

      /** Set the OpenGL-capability to the Gdk::Pixmap.
       * This function creates a new Gdk::GL::Pixmap held by the Gdk::Pixmap.
       * attrib_list is currently unused. This must be set to <tt>0</tt> or empty
       * (first attribute of None).
       *
       * @param glconfig A Gdk::GL::Config.
       * @param attrib_list This must be set to <tt>0</tt> or empty (first attribute of None).
       * @return The Gdk::GL::Pixmap used by the Gdk::Pixmap  if it is successful,
       * <tt>0</tt> otherwise.
       */
      Glib::RefPtr<Pixmap> set_gl_capability(const Glib::RefPtr<const Config>& glconfig,
                                             const int* attrib_list = 0);

      /** Unset the OpenGL-capability of the Gdk::Pixmap .
       * This function destroys the Gdk::GL::Pixmap held by the Gdk::Pixmap.
       *
       */
      void unset_gl_capability();

      /** Returns whether the Gdk::Pixmap is OpenGL-capable.
       *
       * @return <tt>true</tt> if the Gdk::Pixmap  is OpenGL-capable, <tt>false</tt> otherwise.
       */
      bool is_gl_capable() const;

      /** Returns the Gdk::GL::Pixmap held by the Gdk::Pixmap .
       *
       * @return The Gdk::GL::Pixmap.
       */
      Glib::RefPtr<Pixmap> get_gl_pixmap() const;

      /** Returns the Gdk::GL::Drawable held by the Gdk::Pixmap .
       *
       * @return The Gdk::GL::Drawable.
       */
      Glib::RefPtr<Gdk::GL::Drawable> get_gl_drawable() const;

    private:
      const Glib::RefPtr<Gdk::Pixmap>& pixmap_;

      friend PixmapExtProxy ext(const Glib::RefPtr<Gdk::Pixmap>&);
      friend const PixmapExtProxy ext(const Glib::RefPtr<const Gdk::Pixmap>&);
    };

    /** OpenGL extension API wrapper.
     * This is a Gdk::Pixmap wrapper to invoke OpenGL extension function.
     * See Gdk::GL::PixmapExtProxy for available functions.
     *
     * Here is a simple example:
     * @code
     * Glib::RefPtr<Gdk::Pixmap> pixmap =
     *   Gdk::Pixmap::create(0, width, height, glconfig->get_depth());
     * Glib::RefPtr<Gdk::GL::Pixmap> glpixmap =
     *   Gdk::GL::ext(pixmap).set_gl_capability(glconfig);
     * std::cout << "OpenGL-capable? "
     *           << Gdk::GL::ext(pixmap).is_gl_capable()
     *           << std::endl;
     * @endcode
     *
     * @param pixmap A Gdk::Pixmap.
     */
    inline
    PixmapExtProxy ext(const Glib::RefPtr<Gdk::Pixmap>& pixmap)
    { return PixmapExtProxy(pixmap); }

    /** OpenGL extension API wrapper.
     * This is a Gdk::Pixmap wrapper to invoke OpenGL extension function.
     * See Gdk::GL::PixmapExtProxy for available functions.
     *
     * @param pixmap A Gdk::Pixmap.
     */
    inline
    const PixmapExtProxy ext(const Glib::RefPtr<const Gdk::Pixmap>& pixmap)
    { return PixmapExtProxy(reinterpret_cast<const Glib::RefPtr<Gdk::Pixmap>&>(pixmap)); }

  } // namespace GL
} // namespace Gdk

#endif // _GDKMM_GL_PIXMAPEXT_H
