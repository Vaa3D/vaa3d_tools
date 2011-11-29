# Makefile of the fly brain lobe segmentation
# by Hanchuan Peng
# 2008-09-20: for msys/wingw port, by RZC 
# 2009-06-30: for 64-bit compilation on Mac, by YY

CC = cl

OBJS = bdb_minus.obj\
       lobeseg.obj\
       main_backgnd_lobeseg_bl.obj\
       mg_utilities.obj\
       mg_image_lib.obj\
       stackutil.obj\
       mst_prim_c.obj\
       bfs_1root.obj

SHARED_FUNC_DIR = ../basic_c_fun/
SHARED_BDB_DIR = ../worm_straighten_c/
 
all: lobe_seger

lobe_seger : ${OBJS}
	${CC} ${CC_FLAGS} ${OBJS} -B ./ ${LIBS} -o $@

lobeseg.obj: lobeseg.h lobeseg.cpp
	${CC} ${CC_FLAGS} -c lobeseg.cpp

main_backgnd_lobeseg_bl.obj: main_backgnd_lobeseg_bl.cpp lobeseg.h ${SHARED_BDB_DIR}bdb_minus.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}basic_memory.h 
	${CC} ${CC_FLAGS} -c main_backgnd_lobeseg_bl.cpp

bdb_minus.obj: ${SHARED_BDB_DIR}bdb_minus.cpp ${SHARED_BDB_DIR}bdb_minus.h ${SHARED_FUNC_DIR}stackutil.h
	${CC} ${CC_FLAGS} -c ${SHARED_BDB_DIR}bdb_minus.cpp

mst_prim_c.obj: ${SHARED_BDB_DIR}mst_prim_c.cpp ${SHARED_BDB_DIR}mst_prim_c.h ${SHARED_BDB_DIR}graphsupport.h ${SHARED_BDB_DIR}graph.h
	${CC} ${CC_FLAGS} -c ${SHARED_BDB_DIR}mst_prim_c.cpp

bfs_1root.obj: ${SHARED_BDB_DIR}bfs_1root.cpp ${SHARED_BDB_DIR}bfs.h ${SHARED_BDB_DIR}graphsupport.h ${SHARED_BDB_DIR}graph.h
	${CC} ${CC_FLAGS} -c ${SHARED_BDB_DIR}bfs_1root.cpp

stackutil.obj: ${SHARED_FUNC_DIR}stackutil.cpp ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}stackutil.cpp

mg_utilities.obj: ${SHARED_FUNC_DIR}mg_utilities.cpp ${SHARED_FUNC_DIR}mg_utilities.h 
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_utilities.cpp

mg_image_lib.obj: ${SHARED_FUNC_DIR}mg_image_lib.cpp ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp

#point_array.o: point_array.cpp point_array.h
#	${CC} ${CC_FLAGS} -c $<

clean :
	del *.obj
	del lobe_seger


