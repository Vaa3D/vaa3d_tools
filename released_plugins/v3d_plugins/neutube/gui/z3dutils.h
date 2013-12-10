#ifndef Z3DUTILS_H
#define Z3DUTILS_H

#include "z3dmesh.h"

class Z3DUtils
{
public:
  template<typename Real>
  static bool isApproxEqual(const glm::detail::tvec3<Real> &vertex1, const glm::detail::tvec3<Real> &vertex2, Real epsilon = 1e-6);

  template<typename Real>
  static Real vertexPlaneDistance(const glm::detail::tvec3<Real> &vertex, const glm::detail::tvec4<Real>& plane, Real epsilon = 1e-6);

  //
  // Determine the distance of the current vertex to the edge defined by
  // the vertices provided.  Returns distance squared. Note: line is assumed
  // infinite in extent.
  template<typename Real>
  static Real vertexLineSquaredDistance(const glm::detail::tvec3<Real> &x, const glm::detail::tvec3<Real> &p1, const glm::detail::tvec3<Real> &p2);

  // Compute distance to finite line. Returns parametric coordinate t
  // and point location on line.
  template<typename Real>
  static Real vertexLineSegmentSquaredDistance(const glm::detail::tvec3<Real> &x, const glm::detail::tvec3<Real> &p1, const glm::detail::tvec3<Real> &p2,
                                               Real &t, glm::detail::tvec3<Real> &closestPoint);

  // Note: This method assume that P is on triangle plane. P = A + u * (B - A) + v * (C - A)
  // vertex inside tirangle if (u >= 0) && (v >= 0) && (u + v < 1)
  template<typename Real>
  static bool vertexInsideTriangle(const glm::detail::tvec3<Real> &P, const glm::detail::tvec3<Real> &A, const glm::detail::tvec3<Real> &B,
                                   const glm::detail::tvec3<Real> &C, Real epsilon, Real &u, Real &v);

  // distance between P and Triangle A + s * (B - A) + t * (C - A)
  // return value s and t can be used for intepolation
  template<typename Real>
  static Real vertexTriangleSquaredDistance(glm::detail::tvec3<Real> P, glm::detail::tvec3<Real> A,
                                            glm::detail::tvec3<Real> B, glm::detail::tvec3<Real> C, Real &s, Real &t);

  // from VTK
  static Z3DTriangleList clipClosedSurface(const Z3DTriangleList &mesh, std::vector<glm::vec4> clipPlanes, double epsilon = 1e-6);
};


// -------------------------------------------------------------------------------------------

template<typename Real>
bool Z3DUtils::isApproxEqual(const glm::detail::tvec3<Real> &vertex1, const glm::detail::tvec3<Real> &vertex2, Real epsilon)
{
  return glm::length(vertex1 - vertex2) <= epsilon;
}

template<typename Real>
Real Z3DUtils::vertexPlaneDistance(const glm::detail::tvec3<Real> &vertex, const glm::detail::tvec4<Real> &plane, Real epsilon)
{
  double distance = glm::dot(plane.xyz(), vertex) - plane.w;
  if (std::abs(distance) <= epsilon)
    return 0;
  else
    return distance;
}

template<typename Real>
Real Z3DUtils::vertexTriangleSquaredDistance(glm::detail::tvec3<Real> P, glm::detail::tvec3<Real> A,
                                                      glm::detail::tvec3<Real> B, glm::detail::tvec3<Real> C, Real &s, Real &t)
{
  glm::detail::tvec3<Real> diff = A - P;
  glm::detail::tvec3<Real> edge0 = B - A;
  glm::detail::tvec3<Real> edge1 = C - A;
  Real a00 = glm::dot(edge0, edge0);
  Real a01 = glm::dot(edge0, edge1);
  Real a11 = glm::dot(edge1, edge1);
  Real b0 = glm::dot(diff, edge0);
  Real b1 = glm::dot(diff, edge1);
  Real c = glm::dot(diff, diff);
  Real det = std::abs(a00*a11 - a01*a01);
  s = a01*b1 - a11*b0;
  t = a01*b0 - a00*b1;
  Real sqrDistance;

  if (s + t <= det)
  {
    if (s < (Real)0)
    {
      if (t < (Real)0)  // region 4
      {
        if (b0 < (Real)0)
        {
          t = (Real)0;
          if (-b0 >= a00)
          {
            s = (Real)1;
            sqrDistance = a00 + ((Real)2)*b0 + c;
          }
          else
          {
            s = -b0/a00;
            sqrDistance = b0*s + c;
          }
        }
        else
        {
          s = (Real)0;
          if (b1 >= (Real)0)
          {
            t = (Real)0;
            sqrDistance = c;
          }
          else if (-b1 >= a11)
          {
            t = (Real)1;
            sqrDistance = a11 + ((Real)2)*b1 + c;
          }
          else
          {
            t = -b1/a11;
            sqrDistance = b1*t + c;
          }
        }
      }
      else  // region 3
      {
        s = (Real)0;
        if (b1 >= (Real)0)
        {
          t = (Real)0;
          sqrDistance = c;
        }
        else if (-b1 >= a11)
        {
          t = (Real)1;
          sqrDistance = a11 + ((Real)2)*b1 + c;
        }
        else
        {
          t = -b1/a11;
          sqrDistance = b1*t + c;
        }
      }
    }
    else if (t < (Real)0)  // region 5
    {
      t = (Real)0;
      if (b0 >= (Real)0)
      {
        s = (Real)0;
        sqrDistance = c;
      }
      else if (-b0 >= a00)
      {
        s = (Real)1;
        sqrDistance = a00 + ((Real)2)*b0 + c;
      }
      else
      {
        s = -b0/a00;
        sqrDistance = b0*s + c;
      }
    }
    else  // region 0
    {
      // minimum at interior point
      Real invDet = ((Real)1)/det;
      s *= invDet;
      t *= invDet;
      sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
          t*(a01*s + a11*t + ((Real)2)*b1) + c;
    }
  }
  else
  {
    Real tmp0, tmp1, numer, denom;

    if (s < (Real)0)  // region 2
    {
      tmp0 = a01 + b0;
      tmp1 = a11 + b1;
      if (tmp1 > tmp0)
      {
        numer = tmp1 - tmp0;
        denom = a00 - ((Real)2)*a01 + a11;
        if (numer >= denom)
        {
          s = (Real)1;
          t = (Real)0;
          sqrDistance = a00 + ((Real)2)*b0 + c;
        }
        else
        {
          s = numer/denom;
          t = (Real)1 - s;
          sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
              t*(a01*s + a11*t + ((Real)2)*b1) + c;
        }
      }
      else
      {
        s = (Real)0;
        if (tmp1 <= (Real)0)
        {
          t = (Real)1;
          sqrDistance = a11 + ((Real)2)*b1 + c;
        }
        else if (b1 >= (Real)0)
        {
          t = (Real)0;
          sqrDistance = c;
        }
        else
        {
          t = -b1/a11;
          sqrDistance = b1*t + c;
        }
      }
    }
    else if (t < (Real)0)  // region 6
    {
      tmp0 = a01 + b1;
      tmp1 = a00 + b0;
      if (tmp1 > tmp0)
      {
        numer = tmp1 - tmp0;
        denom = a00 - ((Real)2)*a01 + a11;
        if (numer >= denom)
        {
          t = (Real)1;
          s = (Real)0;
          sqrDistance = a11 + ((Real)2)*b1 + c;
        }
        else
        {
          t = numer/denom;
          s = (Real)1 - t;
          sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
              t*(a01*s + a11*t + ((Real)2)*b1) + c;
        }
      }
      else
      {
        t = (Real)0;
        if (tmp1 <= (Real)0)
        {
          s = (Real)1;
          sqrDistance = a00 + ((Real)2)*b0 + c;
        }
        else if (b0 >= (Real)0)
        {
          s = (Real)0;
          sqrDistance = c;
        }
        else
        {
          s = -b0/a00;
          sqrDistance = b0*s + c;
        }
      }
    }
    else  // region 1
    {
      numer = a11 + b1 - a01 - b0;
      if (numer <= (Real)0)
      {
        s = (Real)0;
        t = (Real)1;
        sqrDistance = a11 + ((Real)2)*b1 + c;
      }
      else
      {
        denom = a00 - ((Real)2)*a01 + a11;
        if (numer >= denom)
        {
          s = (Real)1;
          t = (Real)0;
          sqrDistance = a00 + ((Real)2)*b0 + c;
        }
        else
        {
          s = numer/denom;
          t = (Real)1 - s;
          sqrDistance = s*(a00*s + a01*t + ((Real)2)*b0) +
              t*(a01*s + a11*t + ((Real)2)*b1) + c;
        }
      }
    }
  }

  // Account for numerical round-off error.
  if (sqrDistance < (Real)0)
  {
    sqrDistance = (Real)0;
  }

  return sqrDistance;
}

template<typename Real>
Real Z3DUtils::vertexLineSquaredDistance(const glm::detail::tvec3<Real> &x, const glm::detail::tvec3<Real> &p1, const glm::detail::tvec3<Real> &p2)
{
  Real proj, den;
  glm::detail::tvec3<Real> np1, p1p2;

  np1 = x - p1;
  p1p2 = p1 - p2;

  if ( (den=glm::length(p1p2)) != 0.0 )
  {
    for (int i=0; i<3; i++)
    {
      p1p2[i] /= den;
    }
  }
  else
  {
    return glm::dot(np1,np1);
  }

  proj = glm::dot(np1,p1p2);

  return (glm::dot(np1,np1) - proj*proj);
}

template<typename Real>
Real Z3DUtils::vertexLineSegmentSquaredDistance(const glm::detail::tvec3<Real> &x, const glm::detail::tvec3<Real> &p1, const glm::detail::tvec3<Real> &p2,
                                                Real &t, glm::detail::tvec3<Real> &closestPoint)
{
  Real denom, num;
  glm::detail::tvec3<Real> closest;
  Real tolerance;
  //
  //   Determine appropriate vectors
  //
  glm::detail::tvec3<Real> p21 = p2 - p1;

  //
  //   Get parametric location
  //
  //num = p21[0]*(x[0]-p1[0]) + p21[1]*(x[1]-p1[1]) + p21[2]*(x[2]-p1[2]);
  num = glm::dot(p21, x - p1);
  denom = glm::dot(p21,p21);

  // trying to avoid an expensive fabs
  tolerance = 1e-5*num;
  if (tolerance < 0.0)
  {
    tolerance = -tolerance;
  }
  if ( -tolerance < denom && denom < tolerance ) //numerically bad!
  {
    closest = p1; //arbitrary, point is (numerically) far away
  }
  //
  // If parametric coordinate is within 0<=p<=1, then the point is closest to
  // the line.  Otherwise, it's closest to a point at the end of the line.
  //
  else if ( denom <= 0.0 || (t=num/denom) < 0.0 )
  {
    closest = p1;
  }
  else if ( t > 1.0 )
  {
    closest = p2;
  }
  else
  {
    closest = p21;
    p21 = p1 + p21 * t;
    //p21[0] = p1[0] + t*p21[0];
    //p21[1] = p1[1] + t*p21[1];
    //p21[2] = p1[2] + t*p21[2];
  }

  closestPoint = closest;
  return glm::dot(closest-x, closest-x);
}

template<typename Real>
bool Z3DUtils::vertexInsideTriangle(const glm::detail::tvec3<Real> &P, const glm::detail::tvec3<Real> &A, const glm::detail::tvec3<Real> &B,
                                    const glm::detail::tvec3<Real> &C, Real epsilon, Real &u, Real &v)
{
  // Compute vectors
  glm::detail::tvec3<Real> v0 = C - A;
  glm::detail::tvec3<Real> v1 = B - A;
  glm::detail::tvec3<Real> v2 = P - A;

  // Compute dot products
  Real dot00 = glm::dot(v0, v0);
  Real dot01 = glm::dot(v0, v1);
  Real dot02 = glm::dot(v0, v2);
  Real dot11 = glm::dot(v1, v1);
  Real dot12 = glm::dot(v1, v2);

  // Compute barycentric coordinates
  Real invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
  v = (dot11 * dot02 - dot01 * dot12) * invDenom;
  u = (dot00 * dot12 - dot01 * dot02) * invDenom;

  // Check if point is in triangle
  return (u >= -epsilon) && (v >= -epsilon) && (u + v <= 1+epsilon);
}

#if 0

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkClipClosedSurface.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkClipClosedSurface - Clip a closed surface with a plane collection
// .SECTION Description
// vtkClipClosedSurface will clip a closed polydata surface with a
// collection of clipping planes.  It will produce a new closed surface
// by creating new polygonal faces where the input data was clipped.
// If GenerateOutline is on, it will also generate an outline wherever
// the clipping planes intersect the data.  The ScalarMode option
// will add cell scalars to the output, so that the generated faces
// can be visualized in a different color from the original surface.
// .SECTION Caveats
// The triangulation of new faces is done in O(n) time for simple convex
// inputs, but for non-convex inputs the worst-case time is O(n^2*m^2)
// where n is the number of points and m is the number of 3D cavities.
// The best triangulation algorithms, in contrast, are O(n log n).
// There are also rare cases where the triangulation will fail to produce
// a watertight output.  Turn on TriangulationErrorDisplay to be notified
// of these failures.
// .SECTION See Also
// vtkOutlineFilter vtkOutlineSource vtkVolumeOutlineSource
// .SECTION Thanks
// Thanks to David Gobbi for contributing this class to VTK.

#include "vtkFiltersGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkPlaneCollection;
class vtkUnsignedCharArray;
class vtkDoubleArray;
class vtkIdTypeArray;
class vtkCellArray;
class vtkPointData;
class vtkCellData;
class vtkPolygon;
class vtkIdList;
class vtkCCSEdgeLocator;

enum {
  VTK_CCS_SCALAR_MODE_NONE = 0,
  VTK_CCS_SCALAR_MODE_COLORS = 1,
  VTK_CCS_SCALAR_MODE_LABELS = 2
};

class VTKFILTERSGENERAL_EXPORT vtkClipClosedSurface : public vtkPolyDataAlgorithm
{
public:
  static vtkClipClosedSurface *New();
  vtkTypeMacro(vtkClipClosedSurface,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the vtkPlaneCollection that holds the clipping planes.
  virtual void SetClippingPlanes(vtkPlaneCollection *planes);
  vtkGetObjectMacro(ClippingPlanes,vtkPlaneCollection);

  // Description:
  // Set the tolerance for creating new points while clipping.  If the
  // tolerance is too small, then degenerate triangles might be produced.
  // The default tolerance is 1e-6.
  vtkSetMacro(Tolerance, double);
  vtkGetMacro(Tolerance, double);

  // Description:
  // Pass the point data to the output.  Point data will be interpolated
  // when new points are generated.  This is off by default.
  vtkSetMacro(PassPointData, int);
  vtkBooleanMacro(PassPointData, int);
  vtkGetMacro(PassPointData, int);

  // Description:
  // Set whether to generate an outline wherever an input face was
  // cut by a plane.  This is off by default.
  vtkSetMacro(GenerateOutline, int);
  vtkBooleanMacro(GenerateOutline, int);
  vtkGetMacro(GenerateOutline, int);

  // Description:
  // Set whether to generate polygonal faces for the output.  This is
  // on by default.  If it is off, then the output will have no polys.
  vtkSetMacro(GenerateFaces, int);
  vtkBooleanMacro(GenerateFaces, int);
  vtkGetMacro(GenerateFaces, int);

  // Description:
  // Set whether to add cell scalars, so that new faces and outlines
  // can be distinguished from original faces and lines.  The options
  // are "None", "Colors", and "Labels".  For the "Labels" option,
  // a scalar value of "0" indicates an original cell, "1" indicates
  // a new cell on a cut face, and "2" indicates a new cell on the
  // ActivePlane as set by the SetActivePlane() method.  The default
  // scalar mode is "None".
  vtkSetClampMacro(ScalarMode, int,
                   VTK_CCS_SCALAR_MODE_NONE, VTK_CCS_SCALAR_MODE_LABELS);
  void SetScalarModeToNone() {
    this->SetScalarMode(VTK_CCS_SCALAR_MODE_NONE); };
  void SetScalarModeToColors() {
    this->SetScalarMode(VTK_CCS_SCALAR_MODE_COLORS); };
  void SetScalarModeToLabels() {
    this->SetScalarMode(VTK_CCS_SCALAR_MODE_LABELS); };
  vtkGetMacro(ScalarMode, int);
  const char *GetScalarModeAsString();

  // Description:
  // Set the color for all cells were part of the original geometry.
  // If the the input data already has color cell scalars, then those
  // values will be used and parameter will be ignored.  The default color
  // is red.  Requires SetScalarModeToColors.
  vtkSetVector3Macro(BaseColor, double);
  vtkGetVector3Macro(BaseColor, double);

  // Description:
  // Set the color for any new geometry, either faces or outlines, that are
  // created as a result of the clipping. The default color is orange.
  // Requires SetScalarModeToColors.
  vtkSetVector3Macro(ClipColor, double);
  vtkGetVector3Macro(ClipColor, double);

  // Description:
  // Set the active plane, so that the clipping from that plane can be
  // displayed in a different color.  Set this to -1 if there is no active
  // plane.  The default value is -1.
  vtkSetMacro(ActivePlaneId, int);
  vtkGetMacro(ActivePlaneId, int);

  // Description:
  // Set the color for any new geometry produced by clipping with the
  // ActivePlane, if ActivePlaneId is set.  Default is yellow.
  // Requires SetScalarModeToColors.
  vtkSetVector3Macro(ActivePlaneColor, double);
  vtkGetVector3Macro(ActivePlaneColor, double);

  // Description:
  // Generate errors when the triangulation fails.  Usually the
  // triangulation errors are too small to see, but they result in
  // a surface that is not watertight.  This option has no impact
  // on performance.
  vtkSetMacro(TriangulationErrorDisplay, int);
  vtkBooleanMacro(TriangulationErrorDisplay, int);
  vtkGetMacro(TriangulationErrorDisplay, int);

protected:
  vtkClipClosedSurface();
  ~vtkClipClosedSurface();

  vtkPlaneCollection *ClippingPlanes;

  double Tolerance;

  int PassPointData;
  int GenerateOutline;
  int GenerateFaces;
  int ActivePlaneId;
  int ScalarMode;
  double BaseColor[3];
  double ClipColor[3];
  double ActivePlaneColor[3];

  int TriangulationErrorDisplay;

  vtkIdList *IdList;
  vtkCellArray *CellArray;
  vtkPolygon *Polygon;

  virtual int ComputePipelineMTime(
      vtkInformation* request, vtkInformationVector** inputVector,
      vtkInformationVector* outputVector, int requestFromOutputPort,
      unsigned long* mtime);

  virtual int RequestData(
      vtkInformation* request, vtkInformationVector** inputVector,
      vtkInformationVector* outputVector);

  // Description:
  // Method for clipping lines and copying the scalar data.
  void ClipLines(
      vtkPoints *points, vtkDoubleArray *pointScalars,
      vtkPointData *pointData, vtkCCSEdgeLocator *edgeLocator,
      vtkCellArray *inputCells, vtkCellArray *outputLines,
      vtkCellData *inCellData, vtkCellData *outLineData);

  // Description:
  // Clip and contour polys in one step, in order to guarantee
  // that the contour lines exactly match the new free edges of
  // the clipped polygons.  This exact correspondence is necessary
  // in order to guarantee that the surface remains closed.
  void ClipAndContourPolys(
      vtkPoints *points, vtkDoubleArray *pointScalars, vtkPointData *pointData,
      vtkCCSEdgeLocator *edgeLocator, int triangulate,
      vtkCellArray *inputCells, vtkCellArray *outputPolys,
      vtkCellArray *outputLines, vtkCellData *inPolyData,
      vtkCellData *outPolyData, vtkCellData *outLineData);

  // Description:
  // A helper function for interpolating a new point along an edge.  It
  // stores the index of the interpolated point in "i", and returns 1 if
  // a new point was added to the points.  The values i0, i1, v0, v1 are
  // the edge enpoints and scalar values, respectively.
  static int InterpolateEdge(
      vtkPoints *points, vtkPointData *pointData,
      vtkCCSEdgeLocator *edgeLocator, double tol,
      vtkIdType i0, vtkIdType i1, double v0, double v1, vtkIdType &i);

  // Description:
  // A robust method for triangulating a polygon.  It cleans up the polygon
  // and then applies the ear-cut method that is implemented in vtkPolygon.
  // A zero return value indicates that triangulation failed.
  int TriangulatePolygon(
      vtkIdList *polygon, vtkPoints *points, vtkCellArray *triangles);

  // Description:
  // Given some closed contour lines, create a triangle mesh that
  // fills those lines.  The input lines must be single-segment lines,
  // not polylines.  The input lines do not have to be in order.
  // Only lines from firstLine to will be used.  Specify the normal
  // of the clip plane, which will be opposite the the normals
  // of the polys that will be produced.  If outCD has scalars, then color
  // scalars will be added for each poly that is created.
  void MakePolysFromContours(
      vtkPolyData *data, vtkIdType firstLine, vtkIdType numLines,
      vtkCellArray *outputPolys, const double normal[3]);

  // Description:
  // Break polylines into individual lines, copying scalar values from
  // inputScalars starting at firstLineScalar.  If inputScalars is zero,
  // then scalars will be set to color.  If scalars is zero, then no
  // scalars will be generated.
  static void BreakPolylines(
      vtkCellArray *inputLines, vtkCellArray *outputLines,
      vtkUnsignedCharArray *inputScalars, vtkIdType firstLineScalar,
      vtkUnsignedCharArray *outputScalars, const unsigned char color[3]);

  // Description:
  // Copy polygons and their associated scalars to a new array.
  // If inputScalars is set to zero, set polyScalars to color instead.
  // If polyScalars is set to zero, don't generate scalars.
  static void CopyPolygons(
      vtkCellArray *inputPolys, vtkCellArray *outputPolys,
      vtkUnsignedCharArray *inputScalars, vtkIdType firstPolyScalar,
      vtkUnsignedCharArray *outputScalars, const unsigned char color[3]);

  // Description:
  // Break triangle strips and add the triangles to the output. See
  // CopyPolygons for more information.
  static void BreakTriangleStrips(
      vtkCellArray *inputStrips, vtkCellArray *outputPolys,
      vtkUnsignedCharArray *inputScalars, vtkIdType firstStripScalar,
      vtkUnsignedCharArray *outputScalars, const unsigned char color[3]);

  // Description:
  // Squeeze the points and store them in the output.  Only the points that
  // are used by the cells will be saved, and the pointIds of the cells will
  // be modified.
  static void SqueezeOutputPoints(
      vtkPolyData *output, vtkPoints *points, vtkPointData *pointData,
      int outputPointDataType);

  // Description:
  // Take three colors as doubles, and convert to unsigned char.
  static void CreateColorValues(
      const double color1[3], const double color2[3], const double color3[3],
      unsigned char colors[3][3]);

private:
  vtkClipClosedSurface(const vtkClipClosedSurface&);  // Not implemented.
  void operator=(const vtkClipClosedSurface&);  // Not implemented.
};

#endif // reference


#endif // Z3DUTILS_H
