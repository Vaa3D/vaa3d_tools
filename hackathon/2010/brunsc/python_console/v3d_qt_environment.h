/*
 * v3d_qt_environment.h
 *
 *  Created on: Jan 11, 2011
 *      Author: brunsc
 */

#ifndef V3D_QT_ENVIRONMENT_H_
#define V3D_QT_ENVIRONMENT_H_

class QThread;
class QWidget;
class V3DPluginCallback2;

namespace v3d {

    // Expose Qt symbols for use by python plugins
    QThread* get_qt_gui_thread();
    QWidget* get_qt_gui_parent();

    V3DPluginCallback2* get_plugin_callback();

} // namespace v3d

#endif /* V3D_QT_ENVIRONMENT_H_ */
