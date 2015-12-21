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

#ifndef __CAIROMM_SURFACE_H
#define __CAIROMM_SURFACE_H

#include <string>
#include <vector>

/* following is required for OS X */

#ifdef nil
#undef nil
#endif

/* end OS X */

#include <sigc++/slot.h>
#include <cairomm/enums.h>
#include <cairomm/exception.h>
#include <cairomm/fontoptions.h>
#include <cairomm/refptr.h>

//See xlib_surface.h for XlibSurface.
//See win32_surface.h for Win32Surface.
//See quartz_surface.h for QuartzSurface (Mac OS X).

#ifdef CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif // CAIRO_HAS_PDF_SURFACE
#ifdef CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif // CAIRO_HAS_PS_SURFACE
#ifdef CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif // CAIRO_HAS_SVG_SURFACE

// Experimental surfaces
#ifdef CAIRO_HAS_GLITZ_SURFACE
#include <cairo-glitz.h>
#endif // CAIRO_HAS_GLITZ_SURFACE


namespace Cairo
{

/** A cairo surface represents an image, either as the destination of a drawing
 * operation or as source when drawing onto another surface. There are
 * different subtypes of cairo surface for different drawing backends.  This
 * class is a base class for all subtypes and should not be used directly
 *
 * Surfaces are reference-counted objects that should be used via Cairo::RefPtr. 
 */
class Surface
{
public:
  /** For example:
   * <code>
   * ErrorStatus my_write_func(unsigned char* data, unsigned int length);
   * </code>
   *
   * This is the type of function which is called when a backend needs to write
   * data to an output stream. It is passed the data to write and the length of
   * the data in bytes. The write function should return CAIRO_STATUS_SUCCESS if
   * all the data was successfully written, CAIRO_STATUS_WRITE_ERROR otherwise.
   *
   * @param data the buffer containing the data to write
   * @param length the amount of data to write
   * @return the status code of the write operation
   */
  typedef sigc::slot<ErrorStatus, const unsigned char* /*data*/, unsigned int /*length*/> SlotWriteFunc;
  /**
   * This is the type of function which is called when a backend needs to read
   * data from an input stream. It is passed the buffer to read the data into
   * and the length of the data in bytes. The read function should return
   * CAIRO_STATUS_SUCCESS if all the data was successfully read,
   * CAIRO_STATUS_READ_ERROR otherwise.
   *
   * @param data the buffer into which to read the data
   * @param length the amount of data to read
   * @return the status code of the read operation
   */
  typedef sigc::slot<ErrorStatus, unsigned char* /*data*/, unsigned int /*length*/> SlotReadFunc;

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit Surface(cairo_surface_t* cobject, bool has_reference = false);

  virtual ~Surface();

  /** Retrieves the default font rendering options for the surface. This allows
   * display surfaces to report the correct subpixel order for rendering on
   * them, print surfaces to disable hinting of metrics and so forth. The
   * result can then be used with cairo_scaled_font_create().
   *
   * @param options 	a FontOptions object into which to store the retrieved
   * options. All existing values are overwritten
   */
  void get_font_options(FontOptions& options) const;

  /** This function finishes the surface and drops all references to external
   * resources. For example, for the Xlib backend it means that cairo will no
   * longer access the drawable, which can be freed. After calling
   * finish() the only valid operations on a surface are getting and setting
   * user data and referencing and destroying it. Further drawing to the
   * surface will not affect the surface but will instead trigger a
   * CAIRO_STATUS_SURFACE_FINISHED error.
   *
   * When the Surface is destroyed, cairo will call finish() if it hasn't been
   * called already, before freeing the resources associated with the Surface.
   */
  void finish();

  /** Do any pending drawing for the surface and also restore any temporary
   * modifications cairo has made to the surface's state. This function must
   * be called before switching from drawing on the surface with cairo to
   * drawing on it directly with native APIs. If the surface doesn't support
   * direct access, then this function does nothing.
   */
  void flush();

  /** Tells cairo to consider the data buffer dirty.
   *
   * In particular, if you've created an ImageSurface with a data buffer that
   * you've allocated yourself and you draw to that data buffer using means
   * other than cairo, you must call mark_dirty() before doing any additional
   * drawing to that surface with cairo.
   *
   * Note that if you do draw to the Surface outside of cairo, you must call
   * flush() before doing the drawing.
   */
  void mark_dirty();

  /** Marks a rectangular area of the given surface dirty.
   *
   * @param x 	 X coordinate of dirty rectangle
   * @param y 	Y coordinate of dirty rectangle
   * @param width 	width of dirty rectangle
   * @param height 	height of dirty rectangle
   */
  void mark_dirty(int x, int y, int width, int height);

  /** Sets an offset that is added to the device coordinates determined by the
   * CTM when drawing to surface. One use case for this function is when we
   * want to create a Surface that redirects drawing for a portion of
   * an onscreen surface to an offscreen surface in a way that is completely
   * invisible to the user of the cairo API. Setting a transformation via
   * cairo_translate() isn't sufficient to do this, since functions like
   * Cairo::Context::device_to_user() will expose the hidden offset.
   *
   * Note that the offset only affects drawing to the surface, not using the
   * surface in a surface pattern.
   *
   * @param x_offset 	the offset in the X direction, in device units
   * @param y_offset 	the offset in the Y direction, in device units
   */
  void set_device_offset(double x_offset, double y_offset);

  /** Returns a previous device offset set by set_device_offset().
   */
  void get_device_offset(double& x_offset, double& y_offset) const;

  /** Sets the fallback resolution of the image in dots per inch
   *
   * @param x_pixels_per_inch   Pixels per inch in the x direction
   * @param y_pixels_per_inch   Pixels per inch in the y direction
   */
  void set_fallback_resolution(double x_pixels_per_inch, double y_pixels_per_inch);

  /** This function returns the previous fallback resolution set by
   * set_fallback_resolution(), or default fallback resolution if never set.
   *
   * @param x_pixels_per_inch horizontal pixels per inch
   * @param y_pixels_per_inch vertical pixels per inch
   *
   * @since 1.8
   */
  void get_fallback_resolution(double& x_pixels_per_inch, double& y_pixels_per_inch) const;

  SurfaceType get_type() const;

  /**This function returns the content type of surface which indicates whether
   * the surface contains color and/or alpha information.
   *
   * @since 1.8
   */
  Content get_content() const;

  /**
   * Emits the current page for backends that support multiple pages,
   * but doesn't clear it, so that the contents of the current page will
   * be retained for the next page.  Use show_page() if you want to get an empty
   * page after the emission.
   *
   * @since 1.6
   */
  void copy_page();

  /**
   * Emits and clears the current page for backends that support multiple pages.
   * Use copy_page() if you don't want to clear the page.
   *
   * @since 1.6
   */
  void show_page();

  /** Returns whether the surface supports sophisticated
   * Context::show_text_glyphs() operations.  That is, whether it actually uses
   * the provided text and cluster data to a Context::show_text_glyphs() call.
   *
   * Note: Even if this function returns %FALSE, a Context::show_text_glyphs()
   * operation targeted at this surface will still succeed.  It just will act
   * like a Context::show_glyphs() operation.  Users can use this function to
   * avoid computing UTF-8 text and cluster mapping if the target surface does
   * not use it.
   *
   * @since 1.8
   **/
  bool has_show_text_glyphs() const;

#ifdef CAIRO_HAS_PNG_FUNCTIONS

  /** Writes the contents of surface to a new file filename as a PNG image.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support
   *
   * @param filename	the name of a file to write to
   */
  void write_to_png(const std::string& filename);

  /** Writes the Surface to the write function.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   *
   * @since 1.8
   */
  void write_to_png_stream(const SlotWriteFunc& write_func);

  /** @deprecated Use write_to_png_stream instead */
  void write_to_png(cairo_write_func_t write_func, void *closure);

#endif // CAIRO_HAS_PNG_FUNCTIONS


  /** The underlying C cairo surface type
   */
  typedef cairo_surface_t cobject;
  /** Provides acces to the underlying C cairo surface
   */
  inline cobject* cobj() { return m_cobject; }
  /** Provides acces to the underlying C cairo surface
   */
  inline const cobject* cobj() const { return m_cobject; }

  #ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_surface_status(const_cast<cairo_surface_t*>(cobj())); }

  void reference() const;
  void unreference() const;
  #endif //DOXYGEN_IGNORE_THIS

  /** Create a new surface that is as compatible as possible with an existing
   * surface. The new surface will use the same backend as other unless that is
   * not possible for some reason.
   *
   * @param other 	an existing surface used to select the backend of the new surface
   * @param content 	the content for the new surface
   * @param width 	width of the new surface, (in device-space units)
   * @param height 	height of the new surface (in device-space units)
   * @return 	a RefPtr to the newly allocated surface.
   */
  static RefPtr<Surface> create(const RefPtr<Surface> other, Content content, int width, int height);

protected:
  /** The underlying C cairo surface type that is wrapped by this Surface
   */
  cobject* m_cobject;
};

/** @example image-surface.cc
 * An example of using Cairo::ImageSurface class to render to PNG
 */

/** Image surfaces provide the ability to render to memory buffers either
 * allocated by cairo or by the calling code. The supported image formats are
 * those defined in Cairo::Format
 *
 * An ImageSurface is the most generic type of Surface and the only one that is
 * available by default.  You can either create an ImageSurface whose data is
 * managed by Cairo, or you can create an ImageSurface with a data buffer that
 * you allocated yourself so that you can have full access to the data.  
 *
 * When you create an ImageSurface with your own data buffer, you are free to
 * examine the results at any point and do whatever you want with it.  Note that
 * if you modify anything and later want to continue to draw to the surface
 * with cairo, you must let cairo know via Cairo::Surface::mark_dirty() 
 *
 * Note that like all surfaces, an ImageSurface is a reference-counted object that should be used via Cairo::RefPtr.
 */
class ImageSurface : public Surface
{
protected:
  //TODO?: Surface(cairo_surface_t *target);

public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit ImageSurface(cairo_surface_t* cobject, bool has_reference = false);

  virtual ~ImageSurface();

  /** Gets the width of the ImageSurface in pixels
   */
  int get_width() const;

  /** Gets the height of the ImageSurface in pixels
   */
  int get_height() const;

  /**
   * Get a pointer to the data of the image surface, for direct
   * inspection or modification.
   *
   * Return value: a pointer to the image data of this surface or NULL
   * if @surface is not an image surface.
   */
  unsigned char* get_data();
  const unsigned char* get_data() const;

  /** gets the format of the surface
   */
  Format get_format() const;

  /**
   * Return value: the stride of the image surface in bytes (or 0 if
   * @surface is not an image surface). The stride is the distance in
   * bytes from the beginning of one row of the image data to the
   * beginning of the next row.
   */
  int get_stride() const;

  /**
   * This function provides a stride value that will respect all
   * alignment requirements of the accelerated image-rendering code
   * within cairo. Typical usage will be of the form:
   *
   * @code
   * int stride;
   * unsigned char *data;
   * Cairo::RefPtr<Cairo::ImageSurface> surface;
   *
   * stride = Cairo::ImageSurface::format_stride_for_width (format, width);
   * data = malloc (stride * height);
   * surface = Cairo::ImageSurface::create (data, format, width, height);
   * @endcode
   *
   * @param format A Cairo::Format value
   * @param width The desired width of an image surface to be created.
   * @return the appropriate stride to use given the desired format and width, or
   * -1 if either the format is invalid or the width too large.
   *
   * @since 1.6
   **/
  static int format_stride_for_width (Cairo::Format format, int width);

  /**
   * Creates an image surface of the specified format and dimensions. Initially
   * the surface contents are all 0. (Specifically, within each pixel, each
   * color or alpha channel belonging to format will be 0. The contents of bits
   * within a pixel, but not belonging to the given format are undefined).
   *
   * @param format 	format of pixels in the surface to create
   * @param width 	width of the surface, in pixels
   * @param height 	height of the surface, in pixels
   * @return 	a RefPtr to the newly created surface.
   */
  static RefPtr<ImageSurface> create(Format format, int width, int height);

  /**
   * Creates an image surface for the provided pixel data. The output buffer
   * must be kept around until the surface is destroyed or finish() is called on
   * the surface. The initial contents of buffer will be used as the initial
   * image contents; you must explicitly clear the buffer, using, for example,
   * Context::rectangle() and Context::fill() if you want it cleared.
   *
   * Note that the stride may be larger than @a width * @a bytes_per_pixel to
   * provide proper alignment for each pixel and row. This alignment is required
   * to allow high-performance rendering within cairo. The correct way to obtain
   * a legal stride value is to call format_stride_for_width() with the desired
   * format and maximum image width value, and the use the resulting stride
   * value to allocate the data and to create the image surface. See
   * format_stride_for_width() for example code.
   *
   * @param data a pointer to a buffer supplied by the application in which to write contents. This pointer must be suitably aligned for any kind of variable, (for example, a pointer returned by malloc).
   * @param format the format of pixels in the buffer
   * @param width the width of the image to be stored in the buffer
   * @param height the height of the image to be stored in the buffer
   * @param stride the number of bytes between the start of rows in the buffer as allocated. This value should always be computed by cairo_format_stride_for_width() before allocating the data buffer.
   * @return a RefPtr to the newly created surface.
   */
  static RefPtr<ImageSurface> create(unsigned char* data, Format format, int width, int height, int stride);

#ifdef CAIRO_HAS_PNG_FUNCTIONS

  /** Creates a new image surface and initializes the contents to the given PNG
   * file.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support.
   *
   * @param filename	name of PNG file to load
   * @return	a RefPtr to the new cairo_surface_t initialized with the
   * contents of the PNG image file.
   */
  static RefPtr<ImageSurface> create_from_png(std::string filename);

  /** Creates a new image surface from PNG data read incrementally via the
   * read_func function.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support.
   *
   * @param read_func function called to read the data of the file
   * @return a RefPtr to the new cairo_surface_t initialized with the
   * contents of the PNG image file.
   */
  static RefPtr<ImageSurface> create_from_png_stream(const SlotReadFunc& read_func);
  /** @deprecated Use create_from_png_stream instead */
  static RefPtr<ImageSurface> create_from_png(cairo_read_func_t read_func, void *closure);

#endif // CAIRO_HAS_PNG_FUNCTIONS

};


#ifdef CAIRO_HAS_PDF_SURFACE

/** @example pdf-surface.cc
 * An example of using Cairo::PdfSurface class to render to PDF
 */

/** A PdfSurface provides a way to render PDF documents from cairo.  This
 * surface is not rendered to the screen but instead renders the drawing to a
 * PDF file on disk.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * PDF support
 */
class PdfSurface : public Surface
{
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit PdfSurface(cairo_surface_t* cobject, bool has_reference = false);
  virtual ~PdfSurface();

  /** Creates a PdfSurface with a specified dimensions that will be saved as
   * the given filename
   *
   * @param filename    The name of the PDF file to save the surface to
   * @param width_in_points   The width of the PDF document in points
   * @param height_in_points   The height of the PDF document in points
   */
  static RefPtr<PdfSurface> create(std::string filename, double width_in_points, double height_in_points);

  /** Creates a PdfSurface with a specified dimensions that will be written to
   * the given write function instead of saved directly to disk
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   * @param width_in_points   The width of the PDF document in points
   * @param height_in_points   The height of the PDF document in points
   *
   * @since 1.8
   */
  static RefPtr<PdfSurface> create_for_stream(const SlotWriteFunc& write_func, double width_in_points, double height_in_points);
  /** @deprecated use PdfSurface::create_for_stream() instead */
  static RefPtr<PdfSurface> create(cairo_write_func_t write_func, void *closure, double width_in_points, double height_in_points);

/**
 * Changes the size of a PDF surface for the current (and subsequent) pages.
 *
 * This function should only be called before any drawing operations have been
 * performed on the current page. The simplest way to do this is to call this
 * function immediately after creating the surface or immediately after
 * completing a page with either Context::show_page() or Context::copy_page().
 *
 * @param width_in_points new surface width, in points (1 point == 1/72.0 inch)
 * @param height_in_points new surface height, in points (1 point == 1/72.0 inch)
 **/
  void set_size(double width_in_points, double height_in_points);

};

#endif  // CAIRO_HAS_PDF_SURFACE


#ifdef CAIRO_HAS_PS_SURFACE

/** @example ps-surface.cc
 * An example of using Cairo::PsSurface class to render to PostScript
 */

/**
 * describes the language level of the PostScript Language Reference that a
 * generated PostScript file will conform to.
 */
typedef enum {
    PS_LEVEL_2 = CAIRO_PS_LEVEL_2,
    PS_LEVEL_3 = CAIRO_PS_LEVEL_3
} PsLevel;

/** A PsSurface provides a way to render PostScript documents from cairo.  This
 * surface is not rendered to the screen but instead renders the drawing to a
 * PostScript file on disk.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * PostScript support
 */
class PsSurface : public Surface
{
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit PsSurface(cairo_surface_t* cobject, bool has_reference = false);
  virtual ~PsSurface();

  /** Creates a PsSurface with a specified dimensions that will be saved as the
   * given filename
   *
   * @param filename    The name of the PostScript file to save the surface to
   * @param width_in_points   The width of the PostScript document in points
   * @param height_in_points   The height of the PostScript document in points
   */
  static RefPtr<PsSurface> create(std::string filename, double width_in_points, double height_in_points);

  /** Creates a PsSurface with a specified dimensions that will be written to
   * the given write function instead of saved directly to disk
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   * @param width_in_points   The width of the PostScript document in points
   * @param height_in_points   The height of the PostScript document in points
   *
   * @since 1.8
   */
  static RefPtr<PsSurface> create_for_stream(const SlotWriteFunc& write_func, double width_in_points, double height_in_points);
  /** @deprecated use PsSurface::create_for_stream() instead */
  static RefPtr<PsSurface> create(cairo_write_func_t write_func, void *closure, double width_in_points, double height_in_points);

  /**
   * Changes the size of a PostScript surface for the current (and
   * subsequent) pages.
   *
   * This function should only be called before any drawing operations have been
   * performed on the current page. The simplest way to do this is to call this
   * function immediately after creating the surface or immediately after
   * completing a page with either Context::show_page() or Context::copy_page().
   *
   * @param width_in_points new surface width, in points (1 point == 1/72.0 inch)
   * @param height_in_points new surface height, in points (1 point == 1/72.0 inch)
   */
  void set_size(double width_in_points, double height_in_points);

  /** Emit a comment into the PostScript output for the given surface.  See the
   * cairo reference documentation for more information.
   *
   * @param comment a comment string to be emitted into the PostScript output
   */
  void dsc_comment(std::string comment);

  /**
   * This function indicates that subsequent calls to dsc_comment() should direct
   * comments to the Setup section of the PostScript output.
   *
   * This function should be called at most once per surface, and must be called
   * before any call to dsc_begin_page_setup() and before any drawing is performed
   * to the surface.
   */
  void dsc_begin_setup();

  /** This function indicates that subsequent calls to dsc_comment() should
   * direct comments to the PageSetup section of the PostScript output.
   *
   * This function call is only needed for the first page of a surface. It
   * should be called after any call to dsc_begin_setup() and before any drawing
   * is performed to the surface.
   */
  void dsc_begin_page_setup();

  /**
   * If eps is true, the PostScript surface will output Encapsulated
   * PostScript.
   *
   * This function should only be called before any drawing operations
   * have been performed on the current page. The simplest way to do
   * this is to call this function immediately after creating the
   * surface. An Encapsulated Postscript file should never contain more
   * than one page.
   *
   * @since 1.6
   **/
  void set_eps(bool eps);

  /** Check whether the PostScript surface will output Encapsulated PostScript.
   *
   * @since 1.8
   */
  bool get_eps() const;

  /**
   * Restricts the generated PostSript file to @level. See get_levels() for a
   * list of available level values that can be used here.
   *
   * This function should only be called before any drawing operations have been
   * performed on the given surface. The simplest way to do this is to call this
   * function immediately after creating the surface.
   *
   * @param level PostScript level
   *
   * @since 1.6
   **/
  void restrict_to_level(PsLevel level);

  /**
   * Used to retrieve the list of supported levels. See
   * restrict_to_level().
   *
   * @since 1.6
   **/
  static const std::vector<PsLevel> get_levels();

  /**
   * Get the string representation of the given level id. This function will
   * return an empty string if level id isn't valid. See get_levels() for a way
   * to get the list of valid level ids.
   *
   * @return the string associated to given level.
   *
   * @param level a level id
   *
   * @since 1.6
   **/
  static std::string level_to_string(PsLevel level);
};

#endif // CAIRO_HAS_PS_SURFACE


#ifdef CAIRO_HAS_SVG_SURFACE

/** @example svg-surface.cc
 * An example of using Cairo::SvgSurface class to render to SVG
 */

typedef enum
{
  SVG_VERSION_1_1 = CAIRO_SVG_VERSION_1_1,
  SVG_VERSION_1_2 = CAIRO_SVG_VERSION_1_2
} SvgVersion;

/** A SvgSurface provides a way to render Scalable Vector Graphics (SVG) images
 * from cairo.  This surface is not rendered to the screen but instead renders
 * the drawing to an SVG file on disk.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * SVG support
 */
class SvgSurface : public Surface
{
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit SvgSurface(cairo_surface_t* cobject, bool has_reference = false);
  virtual ~SvgSurface();


  /** Creates a SvgSurface with a specified dimensions that will be saved as the
   * given filename
   *
   * @param filename    The name of the SVG file to save the surface to
   * @param width_in_points   The width of the SVG document in points
   * @param height_in_points   The height of the SVG document in points
   */
  static RefPtr<SvgSurface> create(std::string filename, double width_in_points, double height_in_points);

  /** Creates a SvgSurface with a specified dimensions that will be written to
   * the given write function instead of saved directly to disk
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   * @param width_in_points   The width of the SVG document in points
   * @param height_in_points   The height of the SVG document in points
   *
   * @since 1.8
   */
  static RefPtr<SvgSurface> create_for_stream(const SlotWriteFunc& write_func, double width_in_points, double height_in_points);

  /** @deprecated Use SvgSurface::create_for_stream() instead */
  static RefPtr<SvgSurface> create(cairo_write_func_t write_func, void *closure, double width_in_points, double height_in_points);

  /** 
   * Restricts the generated SVG file to the given version. See get_versions()
   * for a list of available version values that can be used here.
   *
   * This function should only be called before any drawing operations have been
   * performed on the given surface. The simplest way to do this is to call this
   * function immediately after creating the surface.
   *
   * @since 1.2
   */
  void restrict_to_version(SvgVersion version);

  /** Retrieves the list of SVG versions supported by cairo. See
   * restrict_to_version().
   * 
   * @since 1.2
   */
  static const std::vector<SvgVersion> get_versions();

  /** Get the string representation of the given version id. The returned string
   * will be empty if version isn't valid. See get_versions() for a way to get
   * the list of valid version ids.
   *
   * since: 1.2
   */
  static std::string version_to_string(SvgVersion version);
};

#endif // CAIRO_HAS_SVG_SURFACE


/*******************************************************************************
 * THE FOLLOWING SURFACE TYPES ARE EXPERIMENTAL AND NOT FULLY SUPPORTED
 ******************************************************************************/

#ifdef CAIRO_HAS_GLITZ_SURFACE

/** A GlitzSurface provides a way to render to the X Window System using Glitz.
 * This provides a way to use OpenGL-accelerated graphics from cairo.  If you
 * want to use hardware-accelerated graphics within the X Window system, you
 * should use this Surface type.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * Glitz support
 *
 * @warning This is an experimental surface.  It is not yet marked as a fully
 * supported surface by the cairo library
 */
class GlitzSurface : public Surface
{

public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit GlitzSurface(cairo_surface_t* cobject, bool has_reference = false);

  virtual ~GlitzSurface();

  /** Creates a new GlitzSurface
   *
   * @param surface  a glitz surface type
   */
  static RefPtr<GlitzSurface> create(glitz_surface_t *surface);

};

#endif // CAIRO_HAS_GLITZ_SURFACE

} // namespace Cairo

#endif //__CAIROMM_SURFACE_H

// vim: ts=2 sw=2 et
