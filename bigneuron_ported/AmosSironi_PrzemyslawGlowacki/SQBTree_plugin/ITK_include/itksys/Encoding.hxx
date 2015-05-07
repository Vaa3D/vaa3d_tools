/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef itksys_Encoding_hxx
#define itksys_Encoding_hxx

#include <itksys/Configure.hxx>
#include <itksys/stl/string>

/* Define these macros temporarily to keep the code readable.  */
#if !defined (KWSYS_NAMESPACE) && !itksys_NAME_IS_KWSYS
# define kwsys_stl itksys_stl
#endif

namespace itksys
{
class itksys_EXPORT Encoding
{
public:
  /**
   * Convert between char and wchar_t
   */

#if itksys_STL_HAS_WSTRING

  // Convert a narrow string to a wide string.
  // On Windows, UTF-8 is assumed, and on other platforms,
  // the current locale is assumed.
  static kwsys_stl::wstring ToWide(const kwsys_stl::string& str);
  static kwsys_stl::wstring ToWide(const char* str);

  // Convert a wide string to a narrow string.
  // On Windows, UTF-8 is assumed, and on other platforms,
  // the current locale is assumed.
  static kwsys_stl::string ToNarrow(const kwsys_stl::wstring& str);
  static kwsys_stl::string ToNarrow(const wchar_t* str);

#endif // itksys_STL_HAS_WSTRING

}; // class Encoding
} // namespace itksys

/* Undefine temporary macros.  */
#if !defined (KWSYS_NAMESPACE) && !itksys_NAME_IS_KWSYS
# undef kwsys_stl
#endif

#endif
