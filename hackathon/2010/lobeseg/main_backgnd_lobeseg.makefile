# Makefile of the fly brain lobe segmentation
# by Hanchuan Peng
# 2008-09-20: for msys/wingw port, by RZC 
# 2009-06-30: for 64-bit compilation on Mac, by YY
# 64bit compilation on Macx
#CC_FLAGS += -arch x86_64
#LDFLAGS += -arch x86_64
CC_FLAGS += $(subst x, x,$(ARCH_x86_64))    
LDFLAGS += $(subst x, x,$(ARCH_x86_64))
# additional search path
CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH))) -I../../v3d_main/common_lib/include
ifneq ($(strip $(ARCH_x86_64)),)
#LIBS = $(patsubst -ltiff,-L$(L_PATH) -ltiff64,$(LIBS))
LIBS += -L. -L$(L_PATH) -ltiff64 -lz -L../../v3d_main/common_lib/lib
else
LIBS += -L. -L../../v3d_main/common_lib/lib -ltiff 
endif
CC = g++
CC_FLAGS += -w   # -w for no compiling warning
CC_FLAGS += -g   # assign -g for gdb debugging
OBJS = bdb_minus.o \
       lobeseg.o \
       main_backgnd_lobeseg_bl.o \
       mg_utilities.o \
       mg_image_lib.o \
       stackutil.o \
       mst_prim_c.o \
       bfs_1root.o 

SHARED_FUNC_DIR = ../../v3d_main/basic_c_fun/
SHARED_BDB_DIR = ../../v3d_main/worm_straighten_c/
 
all: lobe_seger
lobe_seger : ${OBJS}
	${CC} ${CC_FLAGS} ${OBJS} -B ./ ${LIBS} -o $@
lobeseg.o : lobeseg.h lobeseg.cpp
	${CC} ${CC_FLAGS} -c lobeseg.cpp
main_backgnd_lobeseg_bl.o : main_backgnd_lobeseg_bl.cpp lobeseg.h ${SHARED_BDB_DIR}bdb_minus.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}basic_memory.h 
	${CC} ${CC_FLAGS} -c main_backgnd_lobeseg_bl.cpp
bdb_minus.o : ${SHARED_BDB_DIR}bdb_minus.cpp ${SHARED_BDB_DIR}bdb_minus.h ${SHARED_FUNC_DIR}stackutil.h
	${CC} ${CC_FLAGS} -c ${SHARED_BDB_DIR}bdb_minus.cpp
mst_prim_c.o : ${SHARED_BDB_DIR}mst_prim_c.cpp ${SHARED_BDB_DIR}mst_prim_c.h ${SHARED_BDB_DIR}graphsupport.h ${SHARED_BDB_DIR}graph.h
	${CC} ${CC_FLAGS} -c ${SHARED_BDB_DIR}mst_prim_c.cpp
bfs_1root.o : ${SHARED_BDB_DIR}bfs_1root.cpp ${SHARED_BDB_DIR}bfs.h ${SHARED_BDB_DIR}graphsupport.h ${SHARED_BDB_DIR}graph.h
	${CC} ${CC_FLAGS} -c ${SHARED_BDB_DIR}bfs_1root.cpp
stackutil.o : ${SHARED_FUNC_DIR}stackutil.cpp ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}stackutil.cpp
mg_utilities.o : ${SHARED_FUNC_DIR}mg_utilities.cpp ${SHARED_FUNC_DIR}mg_utilities.h 
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_utilities.cpp
mg_image_lib.o : ${SHARED_FUNC_DIR}mg_image_lib.cpp ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp
clean :
	rm *.o
	rm lobe_seger
