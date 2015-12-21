// -*- C++ -*-
/* gtkglextmm - C++ Wrapper for GtkGLExt
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

#ifndef _GTKMM_GL_VERSION_H
#define _GTKMM_GL_VERSION_H

#include <gtkmm/gl/defs.h>

/*
 * Compile time version.
 */
#define GTKGLEXTMM_MAJOR_VERSION (1)
#define GTKGLEXTMM_MINOR_VERSION (2)
#define GTKGLEXTMM_MICRO_VERSION (0)
#define GTKGLEXTMM_INTERFACE_AGE (0)
#define GTKGLEXTMM_BINARY_AGE    (0)

/*
 * Check whether a gtkglextmm version equal to or greater than
 * major.minor.micro is present.
 */
#define	GTKGLEXTMM_CHECK_VERSION(major, minor, micro)                            \
  (GTKGLEXTMM_MAJOR_VERSION > (major) ||                                         \
  (GTKGLEXTMM_MAJOR_VERSION == (major) && GTKGLEXTMM_MINOR_VERSION > (minor)) || \
  (GTKGLEXTMM_MAJOR_VERSION == (major) && GTKGLEXTMM_MINOR_VERSION == (minor) && \
   GTKGLEXTMM_MICRO_VERSION >= (micro)))

/*
 * Library version.
 */

GTKMM_GL_VAR const int gtkglextmm_major_version;
GTKMM_GL_VAR const int gtkglextmm_minor_version;
GTKMM_GL_VAR const int gtkglextmm_micro_version;
GTKMM_GL_VAR const int gtkglextmm_interface_age;
GTKMM_GL_VAR const int gtkglextmm_binary_age;

#endif // _GTKMM_GL_VERSION_H
