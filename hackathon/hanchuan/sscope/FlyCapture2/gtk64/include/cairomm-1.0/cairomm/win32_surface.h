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

#ifndef __CAIROMM_WIN32_SURFACE_H
#define __CAIROMM_WIN32_SURFACE_H

#include <cairomm/surface.h>
#include <cairomm/enums.h>
#include <cairo-features.h>

#ifdef CAIRO_HAS_WIN32_SURFACE
#include <cairo-win32.h>

// This header is not included by cairomm.h because it requires Windows headers that 
// tend to pollute the namespace with non-prefixed #defines and typedefs.
// You may include it directly if you need to use this API.

namespace Cairo
{

/** A Win32Surface provides a way to render within Microsoft Windows.  If you
 * want to draw to the screen within a Microsoft Windows application, you
 * should use this Surface type.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * Win32 support
 */
class Win32Surface : public Surface
{
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit Win32Surface(cairo_surface_t* cobject, bool has_reference = false);
  virtual ~Win32Surface();

  /** Returns the HDC associated with this surface, or NULL if none. Also
   * returns NULL if the surface is not a win32 surface.
   *
   * @return HDC or NULL if no HDC available.
   */
  HDC get_dc() const;

  //TODO: What does owned mean here? murrayc
  //TODO: If this is just a get, shouldn't there be a const version too? murrayc
  /** Returns a ImageSurface that refers to the same bits as the DIB of the
   * Win32 surface. If the passed-in win32 surface is not a DIB surface, the
   * returned surface will be NULL
   *
   * @return a ImageSurface (owned by the Win32Surface), or an empty RefPtr if the win32
   * surface is not a DIB.
   *
   * @since 1.8
   */
  RefPtr<ImageSurface> get_image();

  /** Creates a cairo surface that targets the given DC. The DC will be queried
   * for its initial clip extents, and this will be used as the size of the
   * cairo surface. Also, if the DC is a raster DC, it will be queried for its
   * pixel format and the cairo surface format will be set appropriately.
   *
   * @param hdc the DC to create a surface for.
   * @return the newly created surface.
   */
  static RefPtr<Win32Surface> create(HDC hdc);

  /**
   * @deprecated use create_with_dib()
   */
  static RefPtr<Win32Surface> create(Format format, int width, int height);
  /** Creates a device-independent-bitmap surface not associated with any
   * particular existing surface or device context. The created bitmap will be
   * unititialized.
   *
   * @param format format of pixels in the surface to create.
   * @param width width of the surface, in pixels.
   * @param height height of the surface, in pixels.
   * @return the newly created surface.
   *
   * @since 1.8
   */
  static RefPtr<Win32Surface> create_with_dib(Format format, int width, int height);

  /** Creates a device-independent-bitmap surface not associated with any
   * particular existing surface or device context. The created bitmap will be
   * uninitialized.
   *
   * @param hdc the DC to create a surface for.
   * @param format format of pixels in the surface to create,
   * @param width width of the surface, in pixels.
   * @param height height of the surface, in pixels.
   * @return the newly created surface.
   *
   * @since 1.8
   */
  static RefPtr<Win32Surface> create_with_ddb(HDC hdc, Format format, int width, int height);

};

/** A multi-page vector surface type for printing on Microsoft Windows
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * Win32 support
 *
 * @since 1.8
 */
class Win32PrintingSurface : public Surface
{
public:
  explicit Win32PrintingSurface(cairo_surface_t* cobject, bool has_reference = false);
  virtual ~Win32PrintingSurface();

  /** Creates a cairo surface that targets the given DC. The DC will be queried
   * for its initial clip extents, and this will be used as the size of the
   * cairo surface. The DC should be a printing DC; antialiasing will be
   * ignored, and GDI will be used as much as possible to draw to the surface.
   *
   * The returned surface will be wrapped using the paginated surface to provide
   * correct complex rendering behaviour; show_page() and associated methods
   * must be used for correct output.
   *
   * @param hdc the DC to create a surface for.
   *
   * @since 1.8
   */
  static RefPtr<Win32PrintingSurface> create(HDC hdc);
};

} // namespace Cairo

#endif // CAIRO_HAS_WIN32_SURFACE
#endif //__CAIROMM_WIN32_SURFACE_H
// vim: ts=2 sw=2 et
