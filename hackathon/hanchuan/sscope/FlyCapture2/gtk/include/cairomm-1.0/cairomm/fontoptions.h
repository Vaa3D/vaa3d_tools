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

#ifndef __CAIROMM_FONTOPTIONS_H
#define __CAIROMM_FONTOPTIONS_H

#include <cairomm/enums.h>
#include <string>
//#include <cairo.h>
#ifdef CAIRO_HAS_FT_FONT
#include <cairo-ft.h>
#endif // CAIRO_HAS_FT_FONT

namespace Cairo
{

/** How a font should be rendered.
 */
class FontOptions
{
public:
  FontOptions();
  explicit FontOptions(cairo_font_options_t* cobject, bool take_ownership = false);
  FontOptions(const FontOptions& src);

  virtual ~FontOptions();

  FontOptions& operator=(const FontOptions& src);

  bool operator ==(const FontOptions& src) const;
  //bool operator !=(const FontOptions& src) const;

  /**
   * Merges non-default options from @a other into this, replacing existing
   * values. This operation can be thought of as somewhat similar to compositing
   * @a other onto this with the operation of OPERATION_OVER.
   *
   * @param other another FontOptions
   **/
  void merge(const FontOptions& other);

  /**
   * Compute a hash for the font options object; this value will be useful when
   * storing an object containing a FontOptions in a hash table.
   *
   * @return the hash value for the font options object.  The return value can
   * be cast to a 32-bit type if a 32-bit hash value is needed.
   **/
  unsigned long hash() const;

  /**
   * Sets the antialiasing mode for the font options object. This
   * specifies the type of antialiasing to do when rendering text.
   *
   * @param antialias the new antialiasing mode.
   **/
  void set_antialias(Antialias antialias);

  /**
   * Gets the antialiasing mode for the font options object.
   *
   * @return the antialiasing mode
   **/
  Antialias get_antialias() const;

  /**
   * Sets the subpixel order for the font options object. The subpixel order
   * specifies the order of color elements within each pixel on the display device
   * when rendering with an antialiasing mode of ANTIALIAS_SUBPIXEL. See the
   * documentation for SubpixelOrder for full details.
   *
   * @param subpixel_order the new subpixel order.
   **/
  void set_subpixel_order(SubpixelOrder subpixel_order);

  /**
   * Gets the subpixel order for the font options object.  See the documentation
   * for SubpixelOrder for full details.
   *
   * @return the subpixel order for the font options object.
   **/
  SubpixelOrder get_subpixel_order() const;

  /**
   * Sets the hint style for font outlines for the font options object.  This
   * controls whether to fit font outlines to the pixel grid, and if so, whether
   * to optimize for fidelity or contrast.  See the documentation for
   * HintStyle for full details.
   *
   * @param hint_style the new hint style.
   **/
  void set_hint_style(HintStyle hint_style);

  /**
   * Gets the hint style for font outlines for the font options object.
   * See the documentation for HintStyle for full details.
   *
   * @return the hint style for the font options object.
   **/
  HintStyle get_hint_style() const;

  /**
   * Sets the metrics hinting mode for the font options object. This
   * controls whether metrics are quantized to integer values in
   * device units.
   * See the documentation for HintMetrics for full details.
   *
   * @param hint_metrics the new metrics hinting mode.
   **/
  void set_hint_metrics(HintMetrics hint_metrics);

  /**
   * Gets the metrics hinting mode for the font options object.  See the
   * documentation for HintMetrics for full details.
   *
   * Return value: the metrics hinting mode for the font options object.
   **/
  HintMetrics get_hint_metrics() const;

#ifdef CAIRO_HAS_FT_FONT
  /** Add options to a FcPattern based on a cairo_font_options_t font options
   * object. Options that are already in the pattern, are not overridden, so you
   * should call this function after calling FcConfigSubstitute() (the user's
   * settings should override options based on the surface type), but before
   * calling FcDefaultSubstitute().
   *
   * @param pattern an existing FcPattern.
   *
   * @since 1.8
   */
  void substitute(FcPattern* pattern);
#endif // CAIRO_HAS_FT_FONT

  typedef cairo_font_options_t cobject;
  inline cobject* cobj() { return m_cobject; }
  inline const cobject* cobj() const { return m_cobject; }

  #ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_font_options_status(const_cast<cairo_font_options_t*>(cobj())); }
  #endif //DOXYGEN_IGNORE_THIS

protected:

  cobject* m_cobject;
};

} // namespace Cairo

#endif //__CAIROMM_FONTOPTIONS_H

// vim: ts=2 sw=2 et
