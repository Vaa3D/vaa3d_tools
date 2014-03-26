all:
	g++ -g -I../../../v3d_external/v3d_main/basic_c_fun \
   		-I../../../v3d_external/v3d_main/common_lib/include \
		-L../../../v3d_external/v3d_main/common_lib/lib -lv3dtiff\
		../../../v3d_external/v3d_main/basic_c_fun/stackutil.cpp \
		../../../v3d_external/v3d_main/basic_c_fun/mg_utilities.cpp \
		../../../v3d_external/v3d_main/basic_c_fun/mg_image_lib.cpp \
		*.cpp -o app2
