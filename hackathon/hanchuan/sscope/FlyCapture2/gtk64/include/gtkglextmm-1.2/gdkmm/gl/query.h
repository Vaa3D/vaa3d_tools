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

#ifndef _GDKMM_GL_QUERY_H
#define _GDKMM_GL_QUERY_H

#include <glibmm.h>

#include <gdkmm/gl/defs.h>

#ifdef GDKGLEXTMM_MULTIHEAD_SUPPORT
#include <gdkmm/display.h>
#endif // GDKGLEXTMM_MULTIHEAD_SUPPORT

#include <gdk/gdkgltypes.h>

namespace Gdk
{
  namespace GL
  {

    /** OpenGL support information query stuff.
     *
     *
     */

    /** Indicates whether the window system supports the OpenGL extension
     * (GLX, WGL, etc.).
     *
     * @return  <tt>true</tt> if the OpenGL is supported,
     *          <tt>false</tt> otherwise.
     */
    bool query_extension();

#ifdef GDKGLEXTMM_MULTIHEAD_SUPPORT
    bool query_extension(const Glib::RefPtr<const Gdk::Display>& display);
#endif // GDKGLEXTMM_MULTIHEAD_SUPPORT

    /** Returns the version numbers of the OpenGL extension to the window system.
     *
     * In the X Window System, it returns the GLX version.
     *
     * In the Microsoft Windows, it returns the Windows version.
     *
     * @param major  returns the major version number of the OpenGL extension.
     * @param minor  returns the minor version number of the OpenGL extension.
     * @return  <tt>false</tt> if it fails, <tt>true</tt> otherwise.
     */
    bool query_version(int& major, int& minor);

#ifdef GDKGLEXTMM_MULTIHEAD_SUPPORT
    bool query_version(const Glib::RefPtr<const Gdk::Display>& display,
                       int& major, int& minor);
#endif // GDKGLEXTMM_MULTIHEAD_SUPPORT

    /** Determines whether a given OpenGL extension is supported.
     *
     * There must be a valid current Gdk::GL::Drawable to call
     * Gdk::GL::Query::gl_extension().
     *
     * Gdk::GL::Query::gl_extension() returns information about OpenGL extensions
     * only. This means that window system dependent extensions (for example,
     * GLX extensions) are not reported by Gdk::GL::Query::gl_extension().
     *
     * @param extension  name of OpenGL extension.
     * @return  <tt>false</tt> if the OpenGL extension is supported,
     *          <tt>true</tt> if not supported.
     */
    bool query_gl_extension(const char* extension);

    /** Determines whether a given OpenGL extension is supported.
     *
     * There must be a valid current Gdk::GL::Drawable to call
     * Gdk::GL::Query::gl_extension().
     *
     * Gdk::GL::Query::gl_extension() returns information about OpenGL extensions
     * only. This means that window system dependent extensions (for example,
     * GLX extensions) are not reported by Gdk::GL::Query::gl_extension().
     *
     * @param extension  name of OpenGL extension.
     * @return  <tt>false</tt> if the OpenGL extension is supported,
     *          <tt>true</tt> if not supported.
     */
    bool query_gl_extension(const Glib::ustring& extension);

    /** Returns the address of the OpenGL extension functions.
     *
     * @param proc_name  extension function name.
     * @return  the address of the extension function named by @a proc_name.
     */
    GdkGLProc get_proc_address(const char* proc_name);

    /** Returns the address of the OpenGL extension functions.
     *
     * @param proc_name  extension function name.
     * @return  the address of the extension function named by @a proc_name.
     */
    GdkGLProc get_proc_address(const Glib::ustring& proc_name);

  } // namespace GL
} // namespace Gdk

#endif // _GDKMM_GL_QUERY_H
