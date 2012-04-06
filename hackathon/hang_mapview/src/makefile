VAA3D_PATH = /Users/xiaoh10/work/v3d_external
CXX_FLAGS =  -I${VAA3D_PATH}/v3d_main/basic_c_fun \
			 -I${VAA3D_PATH}/v3d_main/common_lib/include \
			 -I${VAA3D_PATH}/v3d_main/jba/newmat11 \
			 -L${VAA3D_PATH}/v3d_main/common_lib/lib_mac64 -lv3dtiff \
			 -L${VAA3D_PATH}/v3d_main/jba/c++ -lv3dnewmat \
			 ${VAA3D_PATH}/v3d_main/basic_c_fun/basic_memory.cpp \
			 ${VAA3D_PATH}/v3d_main/basic_c_fun/mg_utilities.cpp \
			 ${VAA3D_PATH}/v3d_main/basic_c_fun/mg_image_lib.cpp \
			 ${VAA3D_PATH}/v3d_main/basic_c_fun/stackutil.cpp

create_mapview_data:
	g++ ${CXX_FLAGS} mapview.cpp main.cpp -o create_mapview_data
