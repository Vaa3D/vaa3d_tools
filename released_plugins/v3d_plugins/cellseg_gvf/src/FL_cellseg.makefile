# Makefile of the nuclei segmentation
# by Fuhui Long
# 2008-05-07
# 20080822: change the local basic_c_fun directory. This compatability issue should be resolved later
# 20080823: change the program name as cellseg
# 20080823: add the cellseg library
# 2008-09-20: for msys/wingw port, by RZC 
# 2009-06-30: for 64-bit compilation on Mac, by YY


# 64bit compilation on Macx
#CC_FLAGS += -arch x86_64
#LDFLAGS += -arch x86_64
CC_FLAGS += $(subst x, x,$(ARCH_x86_64))    
LDFLAGS += $(subst x, x,$(ARCH_x86_64))
# additional search path
CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH)))

ifneq ($(strip $(ARCH_x86_64)),)
#LIBS = $(patsubst -ltiff,-L$(L_PATH) -ltiff64,$(LIBS))
LIBS += -L. -L$(L_PATH) -ltiff64 -lz
else
LIBS += -L. -ltiff
endif

CC = g++
CC_FLAGS += -w   # -w for no compiling warning
CC_FLAGS += -g   # assign -g for gdb debugging



all: lib1 lib2 lib3 cellseg gvfcellseg brainseg


CELLSEGLIB_OBJS1 = mg_utilities.o \
       mg_image_lib.o \
       stackutil.o \
       FL_cellSegmentation3D.o
	   
CELLSEGLIB_OBJS2 = mg_utilities.o \
       mg_image_lib.o \
       stackutil.o \
       FL_gvfCellSeg.o
	   
BRAINSEGLIB_OBJS = mg_utilities.o \
       mg_image_lib.o \
       stackutil.o \
       FL_brainseg_ls.o


MAIN_OBJ1 = FL_main_cellseg.o
MAIN_OBJ2 = FL_main_gvfCellSeg.o
MAIN_OBJ3 = FL_main_brainseg.o

INCLUDE += ../../v3d_main/basic_c_fun/
INCLUDE += ../../v3d_main/common_lib/include/
SHARED_FUNC_DIR = ../../v3d_main/basic_c_fun/
 
.PHONY: cellseg
cellseg : $(MAIN_OBJ1) lib1
	${CC} ${CC_FLAGS} ${CELLSEGLIB_OBJS1} $(MAIN_OBJ1)  ${LIBS} -o cellseg

.PHONY: gvfcellseg
gvfcellseg : $(MAIN_OBJ2) lib2
	${CC} ${CC_FLAGS} ${CELLSEGLIB_OBJS2} $(MAIN_OBJ2)  ${LIBS} -o gvfcellseg

.PHONY: brainseg
brainseg : $(MAIN_OBJ3) lib3
	${CC} ${CC_FLAGS} ${BRAINSEGLIB_OBJS} $(MAIN_OBJ3)   ${LIBS} -o brainseg

.PHONY: lib1
lib1 : ${CELLSEGLIB_OBJS1}
	$(AR) -cr libFL_cellseg.a ${CELLSEGLIB_OBJS1}
	ranlib libFL_cellseg.a

.PHONY: lib2
lib2 : ${CELLSEGLIB_OBJS2}
	$(AR) -cr libFL_gvfcellseg.a ${CELLSEGLIB_OBJS2}
	ranlib libFL_gvfcellseg.a

.PHONY: lib3
lib3 : ${BRAINSEGLIB_OBJS}
	$(AR) -cr libFL_brainseg.a ${BRAINSEGLIB_OBJS}
	ranlib libFL_brainseg.a

FL_main_cellseg.o : FL_cellSegmentation3D.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}img_definition.h ${SHARED_FUNC_DIR}volimg_proc.h
	${CC} ${CC_FLAGS} -c FL_main_cellseg.cpp

FL_main_gvfCellSeg.o : FL_gvfCellSeg.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}img_definition.h ${SHARED_FUNC_DIR}volimg_proc.h
	${CC} ${CC_FLAGS} -c FL_main_gvfCellSeg.cpp

FL_main_brainseg.o : FL_main_brainseg.h FL_downSample3D.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}img_definition.h ${SHARED_FUNC_DIR}volimg_proc.h \
	${SHARED_FUNC_DIR}basic_memory.h
	${CC} ${CC_FLAGS} -c FL_main_brainseg.cpp

FL_cellSegmentation3D.o : FL_defType.h FL_volimgProcLib.h FL_cellSegmentation3D.h FL_cellSegmentation3D.cpp FL_filter3D.h \
	FL_adaptiveThreshold3D.h FL_filter3D.h FL_bwlabel2D3D.h FL_morphology.h FL_morphology.cpp FL_sort.h FL_watershed_vs.h FL_distanceTransform3D.h \
    FL_threshold.h \
    ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}img_definition.h ${SHARED_FUNC_DIR}volimg_proc.h
	${CC} ${CC_FLAGS} -c FL_cellSegmentation3D.cpp

FL_gvfCellSeg.o : FL_gvfCellSeg.h FL_defType.h FL_volimgProcLib.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}img_definition.h ${SHARED_FUNC_DIR}volimg_proc.h
	${CC} ${CC_FLAGS} -c FL_gvfCellSeg.cpp
	
FL_brainseg_ls.o : FL_filter3D.h FL_evolve.cpp FL_main_brainseg.h FL_regionProps.h FL_bwlabel2D3D.h FL_morphology.h FL_morphology.cpp FL_bwdist.h ${SHARED_FUNC_DIR}stackutil.h \
	${SHARED_FUNC_DIR}img_definition.h ${SHARED_FUNC_DIR}volimg_proc.h ${SHARED_FUNC_DIR}basic_memory.h
	${CC} ${CC_FLAGS} -c FL_brainseg_ls.cpp
	
stackutil.o : ${SHARED_FUNC_DIR}stackutil.cpp ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}stackutil.cpp

mg_utilities.o : ${SHARED_FUNC_DIR}mg_utilities.cpp ${SHARED_FUNC_DIR}mg_utilities.h 
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_utilities.cpp

mg_image_lib.o : ${SHARED_FUNC_DIR}mg_image_lib.cpp ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp

clean :
	rm *.o
	rm libFL_cellseg.a
	rm cellseg
	rm libFL_gvfcellseg.a
	rm gvfcellseg
	rm libFL_brainseg.a
	rm brainseg

