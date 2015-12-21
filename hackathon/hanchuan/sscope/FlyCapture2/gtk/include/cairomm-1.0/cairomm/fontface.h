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

#ifndef __CAIROMM_FONTFACE_H
#define __CAIROMM_FONTFACE_H

#include <string>
#include <vector>
#include <cairomm/enums.h>
#include <cairomm/types.h>
#include <cairomm/refptr.h>

/* following is required for OS X */

#ifdef nil
#undef nil
#endif

/* end OS X */

#include <cairo.h>
#ifdef CAIRO_HAS_FT_FONT
#include <cairo-ft.h>
#endif //CAIRO_HAS_FT_FONT


namespace Cairo
{

class ScaledFont;
class Context;

/**
 * A FontFace represents a particular font at a particular weight, slant, and
 * other characteristic but no size, transformation, or size.
 *
 * Font faces are created using font-backend-specific constructors or implicitly
 * using the toy text API by way of Context::select_font_face(). The resulting
 * face can be accessed using Context::get_font_face().
 */
class FontFace
{
protected:

  //TODO?: FontFace(cairo_font_face_t *target);

public:
  /** Create a C++ wrapper for the C instance. This C++ instance should then be given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit FontFace(cairo_font_face_t* cobject, bool has_reference = false);


  virtual ~FontFace();

  /* Don't wrap these until we know what they are good for.
  void* get_user_data(const cairo_user_data_key_t *key);

  void set_user_data(const cairo_user_data_key_t *key, void *user_data, cairo_destroy_func_t destroy);
  */

  FontType get_type() const;

  typedef cairo_font_face_t cobject;
  inline cobject* cobj() { return m_cobject; }
  inline const cobject* cobj() const { return m_cobject; }

  #ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_font_face_status(const_cast<cairo_font_face_t*>(cobj())); }
  #endif //DOXYGEN_IGNORE_THIS

  void reference() const;
  void unreference() const;

protected:

  cobject* m_cobject;
};


 /** @example toy-text.cc
  * A relatively simple example of using Cairo::ToyFontFace
  */

/**
 * A simple font face used for the cairo 'toy' font API.
 * @since 1.8
 */
class ToyFontFace : public FontFace
{
public:

  /**
   * Creates a font face from a triplet of family, slant, and weight. These font
   * faces are used in implementation of the the Context "toy" font API.
   *
   * If family is the zero-length string "", the platform-specific default
   * family is assumed. The default family then can be queried using
   * get_family().
   *
   * The Context::select_font_face() function uses this to create font faces.
   * See that function for limitations of toy font faces.
   *
   * @param family a font family name, encoded in UTF-8.
   * @param slant the slant for the font.
   * @param weight the weight for the font.
   */
  static RefPtr<ToyFontFace> create(const std::string& family, FontSlant slant, FontWeight weight);

  /**
   * Gets the familly name of a toy font.
   */
  std::string get_family() const;

  /**
   * Gets the slant a toy font.
   */
  FontSlant get_slant() const;

  /**
   * Gets the weight a toy font.
   */
  FontWeight get_weight() const;

protected:
  ToyFontFace(const std::string& family, FontSlant slant, FontWeight weight);
};


 /** @example user-font.cc
  * A relatively simple example of using Cairo::UserFontFace
  */

/** Font support with font data provided by the user.
 *
 * The user-font feature allows the cairo user to provide drawings for glyphs in
 * a font. This is most useful in implementing fonts in non-standard formats,
 * like SVG fonts and Flash fonts, but can also be used by games and other
 * application to draw "funky" fonts.
 *
 * To use user fonts, you must derive from this class and implement the virtual
 * functions below.  The only virtual function that absolutely must be
 * implemented is render_glyph().  You should make the constructor protected and
 * provide a factory function that returns a new object in a RefPtr since it is
 * a refcounted object
 *
 * @code
 * class MyUserFont : public UserFontFace {
 *
 * public:
 *   static Cairo::RefPtr<MyUserFont> create() {
 *     return Cairo::RefPtr<MyUserFont>(new MyUserFont);
 *
 * protected:
 *   // implement render_glyph() and any other virtual functions you want to override
 *   ErrorStatus render_glyph(const RefPtr<ScaledFont>& scaled_font,
 *                            unsigned long glyph,
 *                            const RefPtr<Context>& cr,
 *                            TextExtents& metrics) {
 *     // render the glyph into cr here
 *   }
 *
 *   MyUserFont() : UserFontFace() {
 *     // constructor implementation
 *   }
 * };
 * @endcode
 *
 * @warning
 * Because of a design flaw in cairomm, it is currently necessary to keep the
 * the UserFontFace object around until as long as you are rendering text with
 * the user font.  The following code illustrates the issue:
 *
 * @code
 * {
 *   Cairo::RefPtr<MyUserFont> face = MyUserFont::create();
 *   cr->set_font_face(face);
 * }  // scope for demonstration purposes
 *
 * // the following call will cause a crash because your user font is no longer
 * // in scope but it needs to call the virtual functions in face
 * cr->show_text("hello, world");
 * @endcode
 *
 * The preceding is obviously a very contrived example, but the important thing
 * to know is that you *must* cache all userfont objects yourself as long as you
 * intend to render text with that font.  A future release of cairomm will fix
 * this requirement, but that will require ABI-incompatible changes.
 *
 * @since 1.8
 */
class UserFontFace : public FontFace
{
public:

  virtual ~UserFontFace();

  /*
  static RefPtr<UserFontFace> create();
  static RefPtr<UserFontFace> create(cairo_font_face_t* cobject, bool has_reference = false);
  */

protected:

  /** This function is called when a scaled-font needs to be created for a user
   * font-face.
   *
   * The Context @a cr is not used by the caller, but is prepared in font space,
   * similar to what the cairo contexts passed to the render_glyph method will
   * look like. The callback can use this context for extents computation for
   * example. After the callback is called, @a cr is checked for any error status.
   *
   * The @a extents argument is where the user font sets the font extents for
   * scaled_font. It is in font space, which means that for most cases its
   * ascent and descent members should add to 1.0. @a extents is preset to hold a
   * value of 1.0 for ascent, height, and max_x_advance, and 0.0 for descent and
   * max_y_advance members.
   *
   * The default implementation sets the font extents as described in the
   * previous paragraph.  If you need different extents, you can override this
   * function in your derived class.
   *
   * Note that @a scaled_font is not fully initialized at this point and trying
   * to use it for text operations in the callback will result in deadlock.
   *
   * @param scaled_font the scaled-font being created
   * @param cr  cairo context, in font space
   * @param extents extents to fill in, in font space
   * @return  CAIRO_STATUS_SUCCESS upon success, or CAIRO_STATUS_USER_FONT_ERROR or any other error status on error.
   *
   * @since 1.8
   */
  virtual ErrorStatus init(const RefPtr<ScaledFont>& scaled_font,
                           const RefPtr<Context>& cr,
                           FontExtents& extents);

  /** This function is called to convert an input Unicode character to a single
   * glyph.  This is used by the Context::show_text() operation.
   *
   * This function is used to provide the same functionality as the
   * text_to_glyphs callback does but has much less control on the output, in
   * exchange for increased ease of use. The inherent assumption to using this
   * callback is that each character maps to one glyph, and that the mapping is
   * context independent. It also assumes that glyphs are positioned according
   * to their advance width. These mean no ligatures, kerning, or complex
   * scripts can be implemented using this callback.
   *
   * The default implementation of this function is an identity mapping from
   * Unicode code-points to glyph indices.  If you need different behavior, you
   * may override this virtual function in your derived class.
   *
   * Note: While cairo does not impose any limitation on glyph indices, some
   * applications may assume that a glyph index fits in a 16-bit unsigned
   * integer. As such, it is advised that user-fonts keep their glyphs in the 0
   * to 65535 range. Furthermore, some applications may assume that glyph 0 is a
   * special glyph-not-found glyph. User-fonts are advised to use glyph 0 for
   * such purposes and do not use that glyph value for other purposes.
   *
   * @param scaled_font the scaled-font being created
   * @param unicode input unicode character code-point
   * @param glyph_index output glyph index
   * @return CAIRO_STATUS_SUCCESS upon success, or CAIRO_STATUS_USER_FONT_ERROR or any other error status on error.
   * @since 1.8
   */
  virtual ErrorStatus unicode_to_glyph(const RefPtr<ScaledFont>& scaled_font,
                                       unsigned long unicode,
                                       unsigned long& glyph);


  /** This function is called when a user scaled-font needs to render a
   * glyph.
   *
   * You must implement this in your derived class, and it is expected to draw
   * the glyph with code @a glyph to the Context @a cr. @a cr is prepared such
   * that the glyph drawing is done in font space. That is, the matrix set on @a
   * cr is the scale matrix of @a scaled_font, The @a extents argument is where
   * the user font sets the font extents for scaled_font. However, if user
   * prefers to draw in user space, they can achieve that by changing the matrix
   * on @a cr. All cairo rendering operations to @a cr are permitted, however,
   * the result is undefined if any source other than the default source on @a
   * cr is used. That means, glyph bitmaps should be rendered using
   * Context::mask() instead of Context::paint().
   *
   * Other non-default settings on @a cr include a font size of 1.0 (given that
   * it is set up to be in font space), and font options corresponding to @a
   * scaled_font.
   *
   * The @a extents argument is preset to have x_bearing, width, and y_advance
   * of zero, y_bearing set to -font_extents.ascent, height to
   * font_extents.ascent+font_extents.descent, and x_advance to
   * font_extents.max_x_advance. The only field user needs to set in majority of
   * cases is x_advance. If the width field is zero upon this function returning
   * (which is its preset value), the glyph extents are automatically computed
   * based on the drawings done to @a cr. This is in most cases exactly what the
   * desired behavior is. However, if for any reason this function sets the
   * extents, it must be ink extents, and include the extents of all drawing
   * done to @a cr.
   *
   * @param scaled_font user scaled-font
   * @param glyph glyph code to render
   * @param cr Context to draw to, in font space
   * @param extents glyph extents to fill in, in font space
   * @return CAIRO_STATUS_SUCCESS upon success, or CAIRO_STATUS_USER_FONT_ERROR or any other error status on error.
   *
   * @since 1.8
   */
  virtual ErrorStatus render_glyph(const RefPtr<ScaledFont>& scaled_font,
                                   unsigned long glyph,
                                   const RefPtr<Context>& cr,
                                   TextExtents& metrics) = 0;


  /** This function is called to convert input text to an array of glyphs. This
   * is used by the Context::show_text() operation.
   *
   * Using this function, the user-font has full control on glyphs and their
   * positions. That means, it allows for features like ligatures and kerning,
   * as well as complex shaping required for scripts like Arabic and Indic.
   *
   * This function should populate the glyph indices and positions (in font
   * space) assuming that the text is to be shown at the origin.
   *
   * If clusters is not empty, cluster mapping should be computed.
   *
   * If you do not override this virtual function in your derived class, 
   * the unicode_to_glyph function is used instead.
   *
   * Note: While cairo does not impose any limitation on glyph indices, some
   * applications may assume that a glyph index fits in a 16-bit unsigned
   * integer. As such, it is advised that user-fonts keep their glyphs in the 0
   * to 65535 range. Furthermore, some applications may assume that glyph 0 is a
   * special glyph-not-found glyph. User-fonts are advised to use glyph 0 for
   * such purposes and do not use that glyph value for other purposes.
   *
   * @param scaled_font the scaled-font being created
   * @param utf8 a string of text encoded in UTF-8
   * @param glyphs array of glyphs to fill, in font space
   * @param clusters array of cluster mapping information to fill
   * @param cluster_flags a variable to set the cluster flags corresponding to the output clusters
   * @return CAIRO_STATUS_SUCCESS upon success, or CAIRO_STATUS_USER_FONT_ERROR or any other error status on error.
   * @since 1.8
   */
  virtual ErrorStatus text_to_glyphs(const RefPtr<ScaledFont>& scaled_font,
                                     const std::string& utf8,
                                     std::vector<Glyph>& glyphs,
                                     std::vector<TextCluster>& clusters,
                                     TextClusterFlags& cluster_flags);



  UserFontFace();

private:
  static cairo_status_t
  init_cb(cairo_scaled_font_t* scaled_font,
          cairo_t *cr,
          cairo_font_extents_t* metrics);

  static cairo_status_t
  unicode_to_glyph_cb(cairo_scaled_font_t *scaled_font,
                      unsigned long        unicode,
                      unsigned long       *glyph);

  static cairo_status_t
  render_glyph_cb(cairo_scaled_font_t  *scaled_font,
                  unsigned long         glyph,
                  cairo_t              *cr,
                  cairo_text_extents_t *metrics);

  static cairo_status_t
  text_to_glyphs_cb (cairo_scaled_font_t *scaled_font,
                     const char *utf8,
                     int utf8_len,
                     cairo_glyph_t **glyphs,
                     int *num_glyphs,
                     cairo_text_cluster_t **clusters,
                     int *num_clusters,
                     cairo_text_cluster_flags_t *cluster_flags);
};


// font system support
#ifdef CAIRO_HAS_FT_FONT

class FtFontFace : public FontFace
{
public:
  /** Creates a new font face for the FreeType font backend from a pre-opened
   * FreeType face. This font can then be used with Context::set_font_face() or
   * FtScaledFont::create().
   *
   * As an example, here is how one might correctly couple the lifetime of the
   * FreeType face object to the FtFontFace:
   * @code
   * static const cairo_user_data_key_t key;

   * font_face = cairo_ft_font_face_create_for_ft_face (ft_face, 0);
   * status = cairo_font_face_set_user_data (font_face, &key,
   *                                ft_face, (cairo_destroy_func_t) FT_Done_Face);
   * if (status) {
   *    cairo_font_face_destroy (font_face);
   *    FT_Done_Face (ft_face);
   *    return ERROR;
   * }
   * @endcode
   *
   * @param face A FreeType face object, already opened. This must be kept
   * around until the face's ref_count drops to zero and it is freed. Since the
   * face may be referenced internally to Cairo, the best way to determine when
   * it is safe to free the face is to pass a cairo_destroy_func_t to
   * cairo_font_face_set_user_data().
   * @param load_flags flags to pass to FT_Load_Glyph when loading glyphs from
   * the font. These flags are OR'ed together with the flags derived from the
   * cairo_font_options_t passed to cairo_scaled_font_create(), so only a few
   * values such as FT_LOAD_VERTICAL_LAYOUT, and FT_LOAD_FORCE_AUTOHINT are
   * useful. You should not pass any of the flags affecting the load target,
   * such as FT_LOAD_TARGET_LIGHT.
   *
   * @since 1.8
   */
  static RefPtr<FtFontFace> create(FT_Face face, int load_flags);
  //TODO: Add a suitable default value for load_flags?

  /** Creates a new font face for the FreeType font backend based on a
   * fontconfig pattern. This font can then be used with Context::set_font_face()
   * or FtScaledFont::create().
   *
   * Font rendering options are represented both here and when you call
   * FtScaledFont::create(). Font options that have a representation in a
   * FcPattern must be passed in here; to modify FcPattern appropriately to
   * reflect the options in a FontOptions, call FontOptions::substitute().
   *
   * The pattern's FC_FT_FACE element is inspected first and if that is set,
   * that will be the FreeType font face associated with the returned cairo font
   * face. Otherwise the FC_FILE and FC_INDEX elements of pattern are used to
   * load a font face from file.
   *
   * If the FC_FT_FACE element of pattern is set, the user is responsible for
   * making sure that the referenced FT_Face remains valid for the life time of
   * the returned FtFontFace. See FtFontFace::create() for an exmaple of how to
   * couple the life time of the FT_Face to that of the cairo font-face.
   *
   * @param pattern A fully resolved fontconfig pattern. A pattern can be
   * resolved, by, among other things, calling FcConfigSubstitute(),
   * FcDefaultSubstitute(), then FcFontMatch(). Cairo will call
   * FcPatternReference() on this pattern, so you should not further modify the
   * pattern, but you can release your reference to the pattern with
   * FcPatternDestroy() if you no longer need to access it.
   *
   * @since 1.8
   */
  static RefPtr<FtFontFace> create(FcPattern* pattern);

protected:
  FtFontFace(FT_Face face, int load_flags);
  FtFontFace(FcPattern* pattern);
};

#endif // CAIRO_HAS_FT_FONT

} // namespace Cairo

#endif //__CAIROMM_FONTFACE_H

// vim: ts=2 sw=2 et
