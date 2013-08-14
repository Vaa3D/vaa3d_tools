# Makefile of the atlas builder
# by Fuhui Long
# 20090426

# for msys/wingw port, by RZC 
CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH))) #
#CC_FLAGS += -I/usr/local/include -L/usr/local/lib #

CC = g++
CC_FLAGS += -w   # -w for no compiling warning
CC_FLAGS += -g   # assign -g for gdb debugging
#LIBS += -ltiff
LIBS += -lnewmat

SHARED_FUNC_DIR = ../basic_c_fun/
SHARED_FUNC_DIR1 = ../v3d/

LIB_OBJS = FL_registerAffine.o FL_atlasBuilder.o converter_pcatlas_data.o

MAIN_OBJ = FL_main_atlasBuilder.o


.PHONY: all
all : $(MAIN_OBJ) lib
	${CC} ${CC_FLAGS} ${LIB_OBJS} $(MAIN_OBJ)  -B ../jba/c++/ ${LIBS} -o atlasbuilder


.PHONY: lib
lib : ${LIB_OBJS}
	$(AR) -cr libFL_atlasbuilder.a ${LIB_OBJS}
	ranlib libFL_atlasbuilder.a

FL_main_atlasBuilder.o : ${SHARED_FUNC_DIR1}pointcloud_atlas_io.h FL_atlasBuilder.h converter_pcatlas_data.h FL_registerAffine.cpp FL_atlasBuilder.cpp FL_main_atlasBuilder.cpp converter_pcatlas_data.cpp ${SHARED_FUNC_DIR1}pointcloud_atlas_io.cpp 
	${CC} ${CC_FLAGS} -c FL_main_atlasBuilder.cpp

FL_registerAffine.o: FL_registerAffine.h FL_registerAffine.cpp
	${CC} ${CC_FLAGS} -c FL_registerAffine.cpp

converter_pcatlas_data.o: converter_pcatlas_data.h ${SHARED_FUNC_DIR1}pointcloud_atlas_io.h FL_atlasBuilder.h converter_pcatlas_data.cpp ${SHARED_FUNC_DIR1}pointcloud_atlas_io.cpp
	${CC} ${CC_FLAGS} -c converter_pcatlas_data.cpp

FL_atlasBuilder.o: ${SHARED_FUNC_DIR}color_xyz.h FL_registerAffine.h FL_atlasBuilder.h FL_registerAffine.cpp FL_atlasBuilder.cpp
	${CC} ${CC_FLAGS} -c FL_atlasBuilder.cpp
clean :
	rm *.o
	rm libFL_atlasbuilder.a
	rm atlasbuilder
