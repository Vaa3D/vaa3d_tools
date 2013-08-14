# point cloud atlas plugin`
# 2009-10-22
# 2010-08-09

TEMPLATE      = lib
CONFIG       += plugin qt 
INCLUDEPATH  += ../v3d_main/basic_c_fun
HEADERS       = pointcloud_atlas_builder.h  \
        	dialog_pointcloudatlas_linkerloader.h \
#		dialog_pointcloudatlas_detect_coexpression_input.h \
#		dialog_pointcloudatlas_detect_coexpression_output.h \
		dialog_pointcloudatlas_detect_coexpression.h \
		dialog_pointcloudatlas_merge_coexpression.h \
		dialog_pointcloudatlas_adjust_preference.h \
		dialog_pointcloudatlas_build_atlas.h \
		$$INCLUDEPATH/v3d_message.h \
		$$INCLUDEPATH/basic_surf_objs.h \
		$$INCLUDEPATH/color_xyz.h \
		$$INCLUDEPATH/v3d_interface.h \
		../pointcloud_atlas_builder/pointcloud_atlas_io.h \
		../pointcloud_atlas_builder/FL_atlasBuilder.h \
		../pointcloud_atlas_builder/converter_pcatlas_data.h \
		../pointcloud_atlas_builder/FL_registerAffine.h \
		../v3d_main/cellseg/FL_sort2.h

SOURCES       = pointcloud_atlas_builder.cpp  \
        	dialog_pointcloudatlas_linkerloader.cpp \
		dialog_pointcloudatlas_detect_coexpression.cpp \
		dialog_pointcloudatlas_merge_coexpression.cpp \
		dialog_pointcloudatlas_adjust_preference.cpp \
		dialog_pointcloudatlas_build_atlas.cpp \
		$$INCLUDEPATH/v3d_message.cpp \
		$$INCLUDEPATH/basic_surf_objs.cpp \
		../pointcloud_atlas_builder/pointcloud_atlas_io.cpp \
		../pointcloud_atlas_builder/FL_atlasBuilder.cpp \
		../pointcloud_atlas_builder/converter_pcatlas_data.cpp \
		../pointcloud_atlas_builder/FL_registerAffine.cpp \
		../pointcloud_atlas_builder/pbetai.cpp

FORMS += dialog_pointcloudatlas_linkerloader.ui \
	dialog_pointcloudatlas_adjust_preference.ui \
#	dialog_pointcloudatlas_detect_coexpression_output.ui \
#	dialog_pointcloudatlas_detect_coexpression_input.ui \
#	dialog_pointcloudatlas_buildatlas.ui \
	dialog_pointcloudatlas_detect_coexpression.ui \
	dialog_pointcloudatlas_merge_coexpression.ui \
	dialog_pointcloudatlas_build_atlas.ui

TARGET        = $$qtLibraryTarget(pointcloud_atlas_builder)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = /Users/longf/work/v3d_external/bin/plugins/atlas_builder/pointcloud_atlas_builder #better set a subdirectory here so that the plugin will be arranged nicely 

macx:LIBS += -lm \
    -L../v3d_main/jba/c++ \
    -lv3dnewmat \

win32:LIBS += \
    -L../v3d_main/jba/c++ \
    -lv3dnewmat \

unix:LIBS += \
    -L../v3d_main/jba/c++ \
    -lv3dnewmat \

LIBS = $$unique(LIBS)

