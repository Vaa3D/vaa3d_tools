# Makefile of the nuclei segmentation
# by Fuhui Long
# 2008-05-07
# 20080822: change the local basic_c_fun directory. This compatability issue should be resolved later
# 20080823: change the program name as cellseg
# 20080823: add the cellseg library
# 2008-09-20: for msys/wingw port, by RZC 
# 2009-06-30: for 64-bit compilation on Mac, by YY
# 2010-05-20: PHC & YUY. produce a intermediate VC 2008 64bit library generator makefile:
#             type the folowing command in a VC command console to run
#                  nmake -f FL_cellseg_msvc64bit.makefile lib1

# 64bit compilation on Macx
#CC_FLAGS += -arch x86_64
#LDFLAGS += -arch x86_64
#CC_FLAGS = $(subst x, x,$(ARCH_x86_64) )    
#LDFLAGS = $(subst x, x,$(ARCH_x86_64) )
# additional search path
#CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH)))

#ifneq ($(strip $(ARCH_x86_64)),)
#LIBS = $(patsubst -ltiff,-L$(L_PATH) -ltiff64,$(LIBS))
#LIBS += -L. -L$(L_PATH) -ltiff64 -lz
#else
#LIBS += -L. -ltiff
#endif
 
CC = cl           # change to cl instead g++, yuy 20100520
#CC_FLAGS += -w   # -w for no compiling warning
#CC_FLAGS += -g   # assign -g for gdb debugging

AR = lib          # for generate lib file, yuy 20100520

all: lib1 lib2 lib3 cellseg gvfcellseg brainseg

# change all .o into .obj, yuy 20100520
CELLSEGLIB_OBJS1 = mg_utilities.obj \
       mg_image_lib.obj \
       stackutil.obj \
       FL_cellSegmentation3D.obj
	   
CELLSEGLIB_OBJS2 = mg_utilities.obj \
       mg_image_lib.obj \
       stackutil.obj \
       FL_gvfCellSeg.obj
	   
BRAINSEGLIB_OBJS = mg_utilities.obj \
       mg_image_lib.obj \
       stackutil.obj \
       FL_brainseg_ls.obj


MAIN_OBJ1 = FL_main_cellseg.obj
MAIN_OBJ2 = FL_main_gvfCellSeg.obj
MAIN_OBJ3 = FL_main_brainseg.obj

SHARED_FUNC_DIR = ./local_basic_c_fun/
 
.PHONY: cellseg
cellseg : $(MAIN_OBJ1) lib1
	$(CC) $(CC_FLAGS) $(CELLSEGLIB_OBJS1) $(MAIN_OBJ1)  $(LIBS) -o cellseg

.PHONY: gvfcellseg
gvfcellseg : $(MAIN_OBJ2) lib2
	$(CC) $(CC_FLAGS) $(CELLSEGLIB_OBJS2) $(MAIN_OBJ2)  $(LIBS) -o gvfcellseg

.PHONY: brainseg
brainseg : $(MAIN_OBJ3) lib3
	$(CC) $(CC_FLAGS) $(BRAINSEGLIB_OBJS) $(MAIN_OBJ3)   $(LIBS) -o brainseg

.PHONY: lib1
lib1 : $(CELLSEGLIB_OBJS1)
#	$(AR) -cr libFL_cellseg.a $(CELLSEGLIB_OBJS1)
#	ranlib libFL_cellseg.a
	$(AR) $(CELLSEGLIB_OBJS1) /OUT:"libFL_cellseg.lib"

.PHONY: lib2
lib2 : $(CELLSEGLIB_OBJS2)
#	$(AR) -cr libFL_gvfcellseg.a $(CELLSEGLIB_OBJS2)
#	ranlib libFL_gvfcellseg.a
	$(AR) $(CELLSEGLIB_OBJS2) /OUT:"libFL_gvfcellseg.lib"

.PHONY: lib3
lib3 : $(BRAINSEGLIB_OBJS)
#	$(AR) -cr libFL_brainseg.a $(BRAINSEGLIB_OBJS)
#	ranlib libFL_brainseg.a
	$(AR) $(BRAINSEGLIB_OBJS) /OUT:"libFL_brainseg.lib"

FL_main_cellseg.obj : FL_cellSegmentation3D.h $(SHARED_FUNC_DIR)stackutil.h $(SHARED_FUNC_DIR)img_definition.h $(SHARED_FUNC_DIR)volimg_proc.h
	$(CC) $(CC_FLAGS) -c FL_main_cellseg.cpp

FL_main_gvfCellSeg.obj : FL_gvfCellSeg.h $(SHARED_FUNC_DIR)stackutil.h $(SHARED_FUNC_DIR)img_definition.h $(SHARED_FUNC_DIR)volimg_proc.h
	$(CC) $(CC_FLAGS) -c FL_main_gvfCellSeg.cpp

FL_main_brainseg.obj : FL_main_brainseg.h FL_downSample3D.h $(SHARED_FUNC_DIR)stackutil.h $(SHARED_FUNC_DIR)img_definition.h $(SHARED_FUNC_DIR)volimg_proc.h \
	$(SHARED_FUNC_DIR)basic_memory.h
	$(CC) $(CC_FLAGS) -c FL_main_brainseg.cpp

FL_cellSegmentation3D.obj : FL_defType.h FL_volimgProcLib.h FL_cellSegmentation3D.h FL_cellSegmentation3D.cpp FL_filter3D.h \
	FL_adaptiveThreshold3D.h FL_filter3D.h FL_bwlabel2D3D.h FL_morphology.h FL_morphology.cpp FL_sort.h FL_watershed_vs.h FL_distanceTransform3D.h \
    FL_threshold.h \
    $(SHARED_FUNC_DIR)stackutil.h $(SHARED_FUNC_DIR)img_definition.h $(SHARED_FUNC_DIR)volimg_proc.h
	$(CC) $(CC_FLAGS) -c FL_cellSegmentation3D.cpp

FL_gvfCellSeg.obj : FL_gvfCellSeg.h FL_defType.h FL_volimgProcLib.h $(SHARED_FUNC_DIR)stackutil.h $(SHARED_FUNC_DIR)img_definition.h $(SHARED_FUNC_DIR)volimg_proc.h
	$(CC) $(CC_FLAGS) -c FL_gvfCellSeg.cpp
	
FL_brainseg_ls.obj : FL_filter3D.h FL_evolve.cpp FL_main_brainseg.h FL_regionProps.h FL_bwlabel2D3D.h FL_morphology.h FL_morphology.cpp FL_bwdist.h $(SHARED_FUNC_DIR)stackutil.h \
	$(SHARED_FUNC_DIR)img_definition.h $(SHARED_FUNC_DIR)volimg_proc.h $(SHARED_FUNC_DIR)basic_memory.h
	$(CC) $(CC_FLAGS) -c FL_brainseg_ls.cpp
	
stackutil.obj : $(SHARED_FUNC_DIR)stackutil.cpp $(SHARED_FUNC_DIR)stackutil.h $(SHARED_FUNC_DIR)mg_image_lib.h
	$(CC) $(CC_FLAGS) -c $(SHARED_FUNC_DIR)stackutil.cpp

mg_utilities.obj : $(SHARED_FUNC_DIR)mg_utilities.cpp $(SHARED_FUNC_DIR)mg_utilities.h 
	$(CC) $(CC_FLAGS) -c $(SHARED_FUNC_DIR)mg_utilities.cpp

mg_image_lib.obj : $(SHARED_FUNC_DIR)mg_image_lib.cpp $(SHARED_FUNC_DIR)mg_image_lib.h
	$(CC) $(CC_FLAGS) -c $(SHARED_FUNC_DIR)mg_image_lib.cpp

# change rm into del for windows system, yuy 20100520
clean :
	del *.obj
	del libFL_cellseg.lib
	del cellseg.exe
	del libFL_gvfcellseg.lib
	del gvfcellseg.exe
	del libFL_brainseg.lib
	del brainseg.exe

