CC = g++ -g
CC_FLAGS += -I../../../../v3d_main/basic_c_fun -I../../../../v3d_main/common_lib/include 
LIBS +=  -L../../../../v3d_main/common_lib/lib -lv3dtiff

SHARED_FUNC_DIR = ../../../../v3d_main/basic_c_fun/

COMMON_OBJS = main.o basic_memory.o stackutil.o mg_image_lib.o mg_utilities.o  basic_types.o utils.o gaussian_blur.o 

## main
partial_align: ${COMMON_OBJS}
	${CC}  ${CC_FLAGS} ${COMMON_OBJS} ${LIBS} -o partial_align
main.o : main.cpp
	${CC} ${CC_FLAGS} -c main.cpp
basic_memory.o : ${SHARED_FUNC_DIR}basic_memory.h ${SHARED_FUNC_DIR}basic_memory.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}basic_memory.cpp
stackutil.o : ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}stackutil.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}stackutil.cpp
mg_image_lib.o : ${SHARED_FUNC_DIR}mg_image_lib.h ${SHARED_FUNC_DIR}mg_image_lib.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp
mg_utilities.o : ${SHARED_FUNC_DIR}mg_utilities.h ${SHARED_FUNC_DIR}mg_utilities.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_utilities.cpp

basic_types.o: basic_types.cpp basic_types.h
	${CC} ${CC_FLAGS} -c basic_types.cpp 
utils.o: utils.cpp utils.h basic_types.h ../edge_detection.h
	${CC} ${CC_FLAGS} -c utils.cpp
gaussian_blur.o: ../../v3d_convert/gaussian_blur.h ../../v3d_convert/gaussian_blur.cpp
	${CC} ${CC_FLAGS} -c ../../v3d_convert/gaussian_blur.cpp

clean:
	rm *.o
