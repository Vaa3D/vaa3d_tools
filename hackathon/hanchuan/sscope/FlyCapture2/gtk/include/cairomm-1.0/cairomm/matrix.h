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
#ifndef __CAIROMM_MATRIX_H
#define __CAIROMM_MATRIX_H

#include <cairo.h>

namespace Cairo
{

/** @class cairo_matrix_t
 * See the <a
 * href="http://www.cairographics.org/manual/cairo-matrix.html">cairo_matrix_t
 * reference</a> in the cairo manual for more information
 */

/** A Transformation matrix.
 *
 * Cairo::Matrix is used throughout cairomm to convert between different
 * coordinate spaces. A Matrix holds an affine transformation, such as
 * a scale, rotation, shear, or a combination of these. The transformation of
 * a point (x,y) is given by:
 *
 * @code
 * x_new = xx * x + xy * y + x0;
 * y_new = yx * x + yy * y + y0;
 * @endcode
 *
 * The current transformation matrix of a Context, represented as a
 * Matrix, defines the transformation from user-space coordinates to
 * device-space coordinates.
 * @sa Context::get_matrix()
 * @sa Context::set_matrix()
 */
class Matrix : public cairo_matrix_t
{
public:
  /** Creates an uninitialized matrix.  If you want a matrix initialized to a
   * certain value, either specify the values explicitly with the other
   * constructor or use one of the free functions for initializing matrices with
   * specific scales, rotations, etc.
   *
   * @sa identity_matrix()
   * @sa rotation_matrix()
   * @sa translation_matrix()
   * @sa scaling_matrix()
   */
  Matrix();

  /** Creates a matrix Sets to be the affine transformation given by xx, yx, xy,
   * yy, x0, y0. The transformation is given by:
   *
   * @code
   * x_new = xx * x + xy * y + x0;
   * y_new = yx * x + yy * y + y0;
   * @endcode
   *
   * @param xx xx component of the affine transformation
   * @param yx yx component of the affine transformation
   * @param xy xy component of the affine transformation
   * @param yy yy component of the affine transformation
   * @param x0 X translation component of the affine transformation
   * @param y0 Y translation component of the affine transformation
   */
  Matrix(double xx, double yx, double xy, double yy, double x0, double y0);

  /** Applies a translation by tx, ty to the transformation in matrix. The
   * effect of the new transformation is to first translate the coordinates by
   * tx and ty, then apply the original transformation to the coordinates.
   *
   * @param tx amount to translate in the X direction
   * @param ty amount to translate in the Y direction
   */
  void translate(double tx, double ty);

  /** Applies scaling by sx, sy to the transformation in matrix. The effect of
   * the new transformation is to first scale the coordinates by sx and sy, then
   * apply the original transformation to the coordinates.
   *
   * @param sx scale factor in the X direction
   * @param sy scale factor in the Y direction
   */
  void scale(double sx, double sy);

  /** Applies rotation by radians to the transformation in matrix. The effect of
   * the new transformation is to first rotate the coordinates by radians, then
   * apply the original transformation to the coordinates.
   *
   * @param radians angle of rotation, in radians. The direction of rotation is
   * defined such that positive angles rotate in the direction from the positive
   * X axis toward the positive Y axis. With the default axis orientation of
   * cairo, positive angles rotate in a clockwise direction.
   */
  void rotate(double radians);

  /** Changes matrix to be the inverse of it's original value. Not all
   * transformation matrices have inverses; if the matrix collapses points
   * together (it is degenerate), then it has no inverse and this function will
   * throw an exception.
   */
  void invert(); // throws exception

  /** Multiplies the affine transformations in a and b together and stores the
   * result in this matrix. The effect of the resulting transformation is to first
   * apply the transformation in a to the coordinates and then apply the
   * transformation in b to the coordinates.
   *
   * It is allowable for result to be identical to either a or b.
   *
   * @param a a Matrix
   * @param b a Matrix
   *
   * @sa operator*()
   */
  void multiply(Matrix& a, Matrix& b);

  /** Transforms the distance vector (dx,dy) by matrix. This is similar to
   * transform_point() except that the translation components of the
   * transformation are ignored. The calculation of the returned vector is as
   * follows:
   *
   * @code
   * dx2 = dx1 * a + dy1 * c;
   * dy2 = dx1 * b + dy1 * d;
   * @endcode
   *
   * Affine transformations are position invariant, so the same vector always
   * transforms to the same vector. If (x1,y1) transforms to (x2,y2) then
   * (x1+dx1,y1+dy1) will transform to (x1+dx2,y1+dy2) for all values of x1 and
   * x2.
   *
   * @param dx X component of a distance vector. An in/out parameter
   * @param dy Y component of a distance vector. An in/out parameter
   */
  void transform_distance(double& dx, double& dy) const;

  /** Transforms the point (x, y) by this matrix.
   *
   * @param x X position. An in/out parameter
   * @param y Y position. An in/out parameter
   */
  void transform_point(double& x, double& y) const;
};

/** Returns a Matrix initialized to the identity matrix
 *
 * @relates Matrix
 */
Matrix identity_matrix();

/** Returns a Matrix initialized to a transformation that translates by tx and
 * ty in the X and Y dimensions, respectively.
 *
 * @param tx amount to translate in the X direction
 * @param ty amount to translate in the Y direction
 *
 * @relates Matrix
 */
Matrix translation_matrix(double tx, double ty);

/** Returns a Matrix initialized to a transformation that scales by sx and sy in
 * the X and Y dimensions, respectively.
 *
 * @param sx scale factor in the X direction
 * @param sy scale factor in the Y direction
 *
 * @relates Matrix
 */
Matrix scaling_matrix(double sx, double sy);

/** Returns a Matrix initialized to a transformation that rotates by radians.
 *
 * @param radians angle of rotation, in radians. The direction of rotation is
 * defined such that positive angles rotate in the direction from the positive X
 * axis toward the positive Y axis. With the default axis orientation of cairo,
 * positive angles rotate in a clockwise direction.
 *
 * @relates Matrix
 */
Matrix rotation_matrix(double radians);

/** Multiplies the affine transformations in a and b together and returns the
 * result. The effect of the resulting transformation is to first
 * apply the transformation in a to the coordinates and then apply the
 * transformation in b to the coordinates.
 *
 * It is allowable for result to be identical to either a or b.
 *
 * @param a a Matrix
 * @param b a Matrix
 *
 * @relates Matrix
 */
Matrix operator*(const Matrix& a, const Matrix& b);

} // namespace Cairo

#endif // __CAIROMM_MATRIX_H
