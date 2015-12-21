// -*- C++ -*-
/* gdkglextmm - C++ Wrapper for GdkGLExt
 * Copyright (C) 2002-2003  Naofumi Yasufuku
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

#ifndef _GDKMM_GL_VERSION_H
#define _GDKMM_GL_VERSION_H

#include <gdkmm/gl/defs.h>

/*
 * Compile time version.
 */
#define GDKGLEXTMM_MAJOR_VERSION (1)
#define GDKGLEXTMM_MINOR_VERSION (2)
#define GDKGLEXTMM_MICRO_VERSION (0)
#define GDKGLEXTMM_INTERFACE_AGE (0)
#define GDKGLEXTMM_BINARY_AGE    (0)

/*
 * Check whether a gdkglextmm version equal to or greater than
 * major.minor.micro is present.
 */
#define	GDKGLEXTMM_CHECK_VERSION(major, minor, micro)                            \
  (GDKGLEXTMM_MAJOR_VERSION > (major) ||                                         \
  (GDKGLEXTMM_MAJOR_VERSION == (major) && GDKGLEXTMM_MINOR_VERSION > (minor)) || \
  (GDKGLEXTMM_MAJOR_VERSION == (major) && GDKGLEXTMM_MINOR_VERSION == (minor) && \
   GDKGLEXTMM_MICRO_VERSION >= (micro)))

/*
 * Library version.
 */

GDKMM_GL_VAR const int gdkglextmm_major_version;
GDKMM_GL_VAR const int gdkglextmm_minor_version;
GDKMM_GL_VAR const int gdkglextmm_micro_version;
GDKMM_GL_VAR const int gdkglextmm_interface_age;
GDKMM_GL_VAR const int gdkglextmm_binary_age;

#endif // _GDKMM_GL_VERSION_H
