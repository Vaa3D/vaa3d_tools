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

#ifndef _GTKMM_GL_INIT_H
#define _GTKMM_GL_INIT_H

namespace Gtk
{
  namespace GL
  {

    /** Initialize gtkglextmm library.
     * 
     * This function does the same work as Gdk::GL::init() with only 
     * a single change: It does not terminate the program if the library can't be 
     * initialized. Instead it returns <tt>false</tt> on failure.
     *
     * This way the application can fall back to some other means of communication 
     * with the user - for example a curses or command line interface.
     * 
     * @param argc  Reference of the <tt>argc</tt> parameter of your 
     *              <tt>main()</tt> function. Changed if any arguments were 
     *              handled.
     * @param argv  Reference of the <tt>argv</tt> parameter of 
     *              <tt>main()</tt>. Any parameters understood by Gtk::GL::init() 
     *              are stripped before return.
     * @return  <tt>true</tt> if the library has been successfully initialized, 
     *          <tt>false</tt> otherwise.
     */
    bool init_check(int& argc, char**& argv);

    /** Initialize gtkglextmm library.
     * 
     * This function does the same work as Gdk::GL::init() with only 
     * a single change: It does not terminate the program if the library can't be 
     * initialized. Instead it returns <tt>false</tt> on failure.
     *
     * This way the application can fall back to some other means of communication 
     * with the user - for example a curses or command line interface.
     * 
     * @param argc  Address of the <tt>argc</tt> parameter of your 
     *              <tt>main()</tt> function. Changed if any arguments were 
     *              handled.
     * @param argv  Address of the <tt>argv</tt> parameter of 
     *              <tt>main()</tt>. Any parameters understood by Gtk::GL::init() 
     *              are stripped before return.
     * @return  <tt>true</tt> if the library has been successfully initialized, 
     *          <tt>false</tt> otherwise.
     */
    bool init_check(int* argc, char*** argv);

    /** Initialize gtkglextmm library.
     * 
     * Call this function before using any other gtkglextmm functions in your 
     * applications.  It will initialize everything needed to operate the library
     * and parses some standard command line options. @a argc and 
     * @a argv are adjusted accordingly so your own code will 
     * never see those standard arguments.
     *
     * This function will terminate your program if it was unable to initialize 
     * the library for some reason. If you want your program to fall back to a 
     * textual interface you want to call Gtk::GL::init_check() instead.
     *
     * @param argc  Reference of the <tt>argc</tt> parameter of your 
     *              <tt>main()</tt> function. Changed if any arguments were 
     *              handled.
     * @param argv  Reference of the <tt>argv</tt> parameter of 
     *              <tt>main()</tt>. Any parameters understood by Gtk::GL::init() 
     *              are stripped before return.
     */
    void init(int& argc, char**& argv);

    /** Initialize gtkglextmm library.
     * 
     * Call this function before using any other gtkglextmm functions in your 
     * applications.  It will initialize everything needed to operate the library
     * and parses some standard command line options. @a argc and 
     * @a argv are adjusted accordingly so your own code will 
     * never see those standard arguments.
     *
     * This function will terminate your program if it was unable to initialize 
     * the library for some reason. If you want your program to fall back to a 
     * textual interface you want to call Gtk::GL::init_check() instead.
     *
     * @param argc  Address of the <tt>argc</tt> parameter of your 
     *              <tt>main()</tt> function. Changed if any arguments were 
     *              handled.
     * @param argv  Address of the <tt>argv</tt> parameter of 
     *              <tt>main()</tt>. Any parameters understood by Gtk::GL::init() 
     *              are stripped before return.
     */
    void init(int* argc, char*** argv);

  } // namespace GL
} // namespace Gtk

#endif // _GTKMM_GL_INIT_H
