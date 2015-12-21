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
#ifndef __CAIROMM_WIN32_FONT_H
#define __CAIROMM_WIN32_FONT_H

#include <cairo-features.h>

#ifdef CAIRO_HAS_WIN32_FONT
#include <cairo-win32.h>
#include <cairomm/fontface.h>
#include <cairomm/scaledfont.h>

namespace Cairo
{

/** Font support for Microsoft Windows.
 *
 * @since 1.8
 */
class Win32FontFace : public FontFace
{
public:

  /** Creates a new font for the Win32 font backend based on a LOGFONT. This
   * font can then be used with Context::set_font_face() or
   * Win32ScaledFont::create().
   *
   * @param logfont A LOGFONTW structure specifying the font to use. The
   * lfHeight, lfWidth, lfOrientation and lfEscapement fields of this structure
   * are ignored.
   *
   * @since 1.8
   */
  static RefPtr<Win32FontFace> create(LOGFONTW* logfont);

  /** Creates a new font for the Win32 font backend based on a HFONT. This font
   * can then be used with Context::set_font_face() or Win32ScaledFont::create().
   *
   * @param font An HFONT structure specifying the font to use.
   *
   * @since 1.8
   */
  static RefPtr<Win32FontFace> create(HFONT font);

  /** Creates a new font for the Win32 font backend based on a LOGFONT. This
   * font can then be used with Context::set_font_face() or
   * Win32ScaledFont::create().
   *
   * @param logfont A LOGFONTW structure specifying the font to use. If hfont is
   * null then the lfHeight, lfWidth, lfOrientation and lfEscapement fields of
   * this structure are ignored. Otherwise lfWidth, lfOrientation and
   * lfEscapement must be zero.
   * @param font An HFONT that can be used when the font matrix is a scale by
   * -lfHeight and the CTM is identity. 
   *
   * @since 1.8
   */
  static RefPtr<Win32FontFace> create(LOGFONTW* logfont, HFONT font);

protected:
  Win32FontFace(LOGFONTW* logfont);
  Win32FontFace(HFONT font);
  Win32FontFace(LOGFONTW* logfont, HFONT font);
};


/** Scaled Font implementation for Microsoft Windows fonts.
 *
 * @since 1.8
 */
class Win32ScaledFont : public ScaledFont
{
public:

  /** Creates a scaled font for the given Win32FontFace.
   *
   * @since 1.8
   */
  static RefPtr<Win32ScaledFont> create(const RefPtr<Win32FontFace>& font_face,
                                        const Matrix& font_matrix,
                                        const Matrix& ctm,
                                        const FontOptions& options = FontOptions());

  /** Selects the font into the given device context and changes the map mode
   * and world transformation of the device context to match that of the font.
   * This function is intended for use when using layout APIs such as Uniscribe
   * to do text layout with the cairo font. After finishing using the device
   * context, you must call done_font() to release any resources allocated by
   * this function.
   *
   * See get_metrics_factor() for converting logical coordinates from the device
   * context to font space.
   *
   * Normally, calls to SaveDC() and RestoreDC() would be made around the use of
   * this function to preserve the original graphics state.
   *
   * @param scaled_font A cairo_scaled_font_t from the Win32 font backend. Such
   * an object can be created with Win32FontFace::create_for_logfontw().
   * @param hdc a device context
   *
   * @since 1.8
   */
  void select_font(HDC hdc);

  /** Releases any resources allocated by select_font()
   *
   * @since 1.8
   */
  void done_font();

  /** Gets a scale factor between logical coordinates in the coordinate space
   * used by select_font() (that is, the coordinate system used by the Windows
   * functions to return metrics) and font space coordinates.
   *
   * @return factor to multiply logical units by to get font space coordinates.
   *
   * @since 1.8
   */
  double get_metrics_factor() const;

  /** Gets the transformation mapping the logical space used by this scaled font
   * to device space.
   *
   * @param logical_to_device matrix to return
   *
   * @since 1.8
   */
  void get_logical_to_device(Matrix& logical_to_device) const;

  /** Gets the transformation mapping device space to the logical space used by
   * this scaled font.
   *
   * @param device_to_logical matrix to return
   *
   * @since 1.8
   */
  void get_device_to_logical(Matrix& device_to_logical) const;

protected:
  Win32ScaledFont(const RefPtr<Win32FontFace>& font_face,
                  const Matrix& font_matrix,
                  const Matrix& ctm,
                  const FontOptions& options = FontOptions());
};

}

#endif // CAIRO_HAS_WIN32_FONT

#endif // __CAIROMM_WIN32_FONT_H
