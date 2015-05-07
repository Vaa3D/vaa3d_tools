/* plugin_creator_func.h
 * This plugin is used to produce v3d plugin project from a template file
 * 2012-01-27 : by Hang Xiao
 */
 
#ifndef __PLUGIN_CREATOR_FUNC_H__
#define __PLUGIN_CREATOR_FUNC_H__

#include <v3d_interface.h>

int create_plugin(V3DPluginCallback2 &callback, QWidget *parent);
int create_plugin_neuronrec(V3DPluginCallback2 &callback, QWidget *parent);
int create_demo1(V3DPluginCallback2 &callback, QWidget *parent);
int create_demo2(V3DPluginCallback2 &callback, QWidget *parent);

void produce_demo1(string dir, string vaa3d_path);
void produce_demo2(string dir, string vaa3d_path, string plugin_name, string menu_name, string func_name);

QString getVaa3dPath();

#endif

