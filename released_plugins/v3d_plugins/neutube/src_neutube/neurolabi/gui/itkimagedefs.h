/**@file itkimagedefs.h
 * @brief ITK image definition
 * @author Ting Zhao
 */
#ifndef ITKIMAGEDEFS_H
#define ITKIMAGEDEFS_H

// On mac system,
// qglobal.h force min mac version to 1040 which breaks ITK
// To make ITK work, change itkLightObject.h:
// (force use volatile int64_t since MAC_OS_X_VERSION_MIN_REQUIRED used
// while building ITK would be the currently build-on mac version which
// is larger than 1050)

//#elif defined( __GLIBCPP__ ) || defined( __GLIBCXX__ )
//  //typedef _Atomic_word InternalReferenceCountType;
//  typedef volatile int64_t InternalReferenceCountType;
//#else

// see http://www.itk.org/pipermail/insight-users/2011-October/042755.html
// see https://bugreports.qt-project.org/browse/QTBUG-22154

typedef unsigned char    CharPixelType;
typedef double           RealPixelType;

#if defined(_USE_ITK_) //ITK is available

#include <itkMacro.h>
#include <itkImage.h>

typedef itk::Image<CharPixelType, 3> Uint8Image3DType;
typedef itk::Image<unsigned short, 3> Uint16Image3DType;
typedef itk::Image<float, 3> FloatImage3DType;
typedef itk::Image<RealPixelType, 3> DoubleImage3DType;

#else //ITK is NOT available

typedef unsigned char Uint8Image3DType;
typedef unsigned short Uint16Image3DType;
typedef float FloatImage3DType;
typedef double DoubleImage3DType;

#endif


#endif // ITKIMAGEDEFS_H
