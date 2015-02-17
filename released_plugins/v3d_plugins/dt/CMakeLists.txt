
set(PLUGIN_SOURCES dt.cpp my_surf_objs.cpp)
set(PLUGIN_HEADERS dt.h  my_surf_objs.h FL_bwdist.h)
set(PLUGIN_DIRECTORY_NAME image_filters/Fast_Distance_Transform)

configure_v3d_plugin(dt)

