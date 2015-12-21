/* Copyright (C) 2006 The cairomm Development Team
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

#ifndef __CAIROMM_SCALEDFONT_H
#define __CAIROMM_SCALEDFONT_H

#include <cairomm/refptr.h>
#include <cairomm/fontoptions.h>
#include <cairomm/fontface.h>
#include <cairomm/matrix.h>
#include <cairomm/types.h>
#include <vector>

#ifdef CAIRO_HAS_FT_FONT
#include <cairo-ft.h>
#endif // CAIRO_HAS_FT_FONT

namespace Cairo
{

/** A ScaledFont is a font scaled to a particular size and device resolution. It
 * is most useful for low-level font usage where a library or application wants
 * to cache a reference to a scaled font to speed up the computation of metrics.
 */
class ScaledFont
{

public: 
  /** The underlying C cairo object type */
  typedef cairo_scaled_font_t cobject;

  /** Provides acces to the underlying C cairo object */
  inline cobject* cobj() { return m_cobject; }

  /** Provides acces to the underlying C cairo object */
  inline const cobject* cobj() const { return m_cobject; }

#ifndef DOXYGEN_IGNORE_THIS
  // For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_scaled_font_status(const_cast<cairo_scaled_font_t*>(cobj())); }

  // for RefPtr
  void reference() const { cairo_scaled_font_reference(m_cobject); }
  void unreference() const { cairo_scaled_font_destroy(m_cobject); }
#endif //DOXYGEN_IGNORE_THIS

  /** Create a C++ wrapper object from the C instance.  This C++ object should
   * then be given to a RefPtr.
   */
  explicit ScaledFont(cobject* cobj, bool has_reference = false);

  ~ScaledFont();

  /** Creates a ScaledFont object from a font face and matrices that describe
   * the size of the font and the environment in which it will be used.
   *
   * @param font_face A font face.
   * @param font_matrix font space to user space transformation matrix for the
   * font. In the simplest case of a N point font, this matrix is just a scale
   * by N, but it can also be used to shear the font or stretch it unequally
   * along the two axes. See Context::set_font_matrix().
   * @param ctm user to device transformation matrix with which the font will be
   * used.
   * @param options options to use when getting metrics for the font and
   * rendering with it.
   */
  static RefPtr<ScaledFont> create(const RefPtr<FontFace>& font_face, const Matrix& font_matrix,
      const Matrix& ctm, const FontOptions& options = FontOptions());
  // NOTE: the constructor doesn't take a RefPtr<const FontFace> because the
  // FontFace object can be changed in this constructor (in the case of user
  // fonts, the FontFace becomes immutable, i.e. you can't call any set_*_func()
  // functions any longer)

  //We use an output paramter instead of the return value,
  //for consistency with other get_*extents() methods in other classes,
  //though they should probably all use the return value instead.
  //but it is too late to change that now. murrayc:

  /** Gets the metrics for a ScaledFont
   * @since 1.8
   * */
  void get_extents(FontExtents& extents) const;

  /** @deprecated Use get_extents() instead
   * @since 1.2
   * */
  void extents(FontExtents& extents) const;

  /** Gets the extents for a string of text. The extents describe a user-space
   * rectangle that encloses the "inked" portion of the text drawn at the origin
   * (0,0) (as it would be drawn by Context::show_text() if the cairo graphics
   * state were set to the same font_face, font_matrix, ctm, and font_options as
   * the ScaledFont object).  Additionally, the x_advance and y_advance values
   * indicate the amount by which the current point would be advanced by
   * Context::show_text().
   *
   * Note that whitespace characters do not directly contribute to the size of
   * the rectangle (extents.width and extents.height). They do contribute
   * indirectly by changing the position of non-whitespace characters. In
   * particular, trailing whitespace characters are likely to not affect the
   * size of the rectangle, though they will affect the x_advance and y_advance
   * values.
   *
   * @param utf8  a string of text, encoded in UTF-8.
   * @param extents Returns the extents of the given string.
   *
   * @since 1.8
   */
  void get_text_extents(const std::string& utf8, TextExtents& extents) const;
  /** @deprecated Use get_text_extents() instead
   * @since 1.2
   * */
  void text_extents(const std::string& utf8, TextExtents& extents) const;

  /** Gets the extents for an array of glyphs. The extents describe a user-space
   * rectangle that encloses the "inked" portion of the glyphs, (as they would
   * be drawn by Context::show_glyphs() if the cairo graphics state were set to the
   * same font_face, font_matrix, ctm, and font_options as the ScaledFont
   * object).  Additionally, the x_advance and y_advance values indicate the
   * amount by which the current point would be advanced by Context::show_glyphs().
   *
   * Note that whitespace glyphs do not contribute to the size of the rectangle
   * (extents.width and extents.height).
   *
   * @param glyphs A vector of glyphs to calculate the extents of.
   * @param extents Returns the extents for the array of glyphs.
   *
   * @since 1.8
   **/
  void get_glyph_extents(const std::vector<Glyph>& glyphs, TextExtents& extents);

  /** @deprecated Use get_glyph_extents() instead
   * @since 1.2
   * */
  void glyph_extents(const std::vector<Glyph>& glyphs, TextExtents& extents);

  /** The FontFace with which this ScaledFont was created.
   * @since 1.2
   */
  RefPtr<FontFace> get_font_face() const;

  /** Gets the FontOptions with which the ScaledFont was created.
   * @since 1.2
   */
  void get_font_options(FontOptions& options) const;

  /** Gets the font matrix with which the ScaledFont was created.
   * @since 1.2
   */
  void get_font_matrix(Matrix& font_matrix) const;

  /** Gets the CTM with which the ScaledFont was created.
   * @since 1.2
   */
  void get_ctm(Matrix& ctm) const;

  /** Gets the type of scaled Font
   * @since 1.2
   */
  FontType get_type() const;

  // FIXME: it'd be really nice not to assume a specific container (e.g.
  // std::vector) here
  /**
   * @param x X position to place first glyph.
   * @param y Y position to place first glyph.
   * @param utf8 a string of text encoded in UTF-8.
   * @param glyphs pointer to array of glyphs to fill.
   * @param clusters pointer to array of cluster mapping information to fill.
   * @cluster_flags cluster mapping flags
   *
   * Converts UTF-8 text to an array of glyphs, with cluster mapping, that can be
   * used to render later.
   *
   * For details of how (@a clusters and @a cluster_flags map input
   * UTF-8 text to the output glyphs see Context::show_text_glyphs().
   *
   * The output values can be readily passed to Context::show_text_glyphs()
   * Context::show_glyphs(), or related functions, assuming that the exact
   * same scaled font is used for the operation.
   *
   * @since 1.8
   **/
  void text_to_glyphs(double x,
                      double y,
                      const std::string& utf8,
                      std::vector<Glyph>& glyphs,
                      std::vector<TextCluster>& clusters,
                      TextClusterFlags& cluster_flags);

  /** Stores the scale matrix of this scaled font into matrix. The scale matrix
   * is product of the font matrix and the ctm associated with the scaled font,
   * and hence is the matrix mapping from font space to device space.
   *
   * @param scale_matrix return value for the matrix.
   *
   * @since 1.8
   */
  void get_scale_matrix(Matrix& scale_matrix) const;

protected:
  ScaledFont(const RefPtr<FontFace>& font_face, const Matrix& font_matrix,
             const Matrix& ctm, const FontOptions& options = FontOptions());
  /** The underlying C cairo object that is wrapped by this ScaledFont */
  cobject* m_cobject;
};

#ifdef CAIRO_HAS_FT_FONT

//TODO: Documentation.
/**
 * @since 1.8
 */
class FtScaledFont : public ScaledFont
{
public:
  /** Creates a ScaledFont From a FtFontFace.
   *
   * @since 1.8
   */
  static RefPtr<FtScaledFont> create(const RefPtr<FtFontFace>& font_face, const Matrix& font_matrix,
      const Matrix& ctm, const FontOptions& options = FontOptions());

  /** Gets the FT_Face object from a FreeType backend font and scales it
   * appropriately for the font. You must release the face with
   * unlock_face() when you are done using it. Since the FT_Face object can be
   * shared between multiple ScaledFont objects, you must not lock any other
   * font objects until you unlock this one. A count is kept of the number of
   * times lock_face() is called.  unlock_face() must be called the same number
   * of times.
   *
   * You must be careful when using this function in a library or in a threaded
   * application, because freetype's design makes it unsafe to call freetype
   * functions simultaneously from multiple threads, (even if using distinct
   * FT_Face objects). Because of this, application code that acquires an
   * FT_Face object with this call must add it's own locking to protect any use
   * of that object, (and which also must protect any other calls into cairo as
   * almost any cairo function might result in a call into the freetype
   * library).
   *
   * @return The FT_Face object for font, scaled appropriately, or NULL if
   * scaled_font is in an error state or there is insufficient memory.
   *
   * @since 1.8
   */
  FT_Face lock_face();

  /** Releases a face obtained with lock_face().
   *
   * @since 1.8
   */
  void unlock_face();

protected:
  FtScaledFont(const RefPtr<FtFontFace>& font_face, const Matrix& font_matrix,
      const Matrix& ctm, const FontOptions& options = FontOptions());
};
#endif // CAIRO_HAS_FT_FONT

}

#endif // __CAIROMM_SCALEDFONT_H
// vim: ts=2 sw=2 et
