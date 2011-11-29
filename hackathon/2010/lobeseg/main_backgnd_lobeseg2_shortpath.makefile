# Makefile of the fly brain lobe segmentation
# by Hanchuan Peng
# update Aug 28 2008 by BL, add do_shortestpath()

CC = g++
DEBUG_FLAG = -g -pg -Wall -I../neurolabi/c/include -I../neurolabi/lib/myers.pack     # assign -g for debugging

OBJS = bdb_minus.o \
       lobeseg2.o \
       main_backgnd_lobeseg_bl_tz.o \
       mg_utilities.o \
       mg_image_lib.o \
       stackutil.o \
       mst_prim_c.o \
       bfs_1root.o 

SHARED_FUNC_DIR = ../basic_c_fun/
SHARED_BDB_DIR = ../worm_straighten_c/
SHARED_GRAPH_DIR = ../neurolabi/c/
 
LIBS = -lneurolabi -lmyers -ltiff -lfftw3 -lfftw3f -lgsl -lz
       

all: lobe_seger

lobe_seger : ${OBJS}
	${CC} ${DEBUG_FLAG} ${OBJS} -B ./ ${LIBS} -L$(SHARED_GRAPH_DIR)lib -L../neurolabi/lib/myers.pack -o $@

lobeseg2.o : lobeseg2.h lobeseg2.cpp ${SHARED_GRAPH_DIR}tz_stack_graph.h
	${CC} ${DEBUG_FLAG} -c lobeseg2.cpp

main_backgnd_lobeseg_bl_tz.o : main_backgnd_lobeseg_bl_tz.cpp lobeseg2.h ${SHARED_BDB_DIR}bdb_minus.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}basic_memory.h 
	${CC} ${DEBUG_FLAG} -c main_backgnd_lobeseg_bl_tz.cpp

bdb_minus.o : ${SHARED_BDB_DIR}bdb_minus.cpp ${SHARED_BDB_DIR}bdb_minus.h ${SHARED_FUNC_DIR}stackutil.h
	${CC} ${DEBUG_FLAG} -c ${SHARED_BDB_DIR}bdb_minus.cpp

mst_prim_c.o : ${SHARED_BDB_DIR}mst_prim_c.cpp ${SHARED_BDB_DIR}mst_prim_c.h ${SHARED_BDB_DIR}graphsupport.h ${SHARED_BDB_DIR}graph.h
	${CC} ${DEBUG_FLAG} -c ${SHARED_BDB_DIR}mst_prim_c.cpp

bfs_1root.o : ${SHARED_BDB_DIR}bfs_1root.cpp ${SHARED_BDB_DIR}bfs.h ${SHARED_BDB_DIR}graphsupport.h ${SHARED_BDB_DIR}graph.h
	${CC} ${DEBUG_FLAG} -c ${SHARED_BDB_DIR}bfs_1root.cpp

stackutil.o : ${SHARED_FUNC_DIR}stackutil.cpp ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${DEBUG_FLAG} -c ${SHARED_FUNC_DIR}stackutil.cpp

mg_utilities.o : ${SHARED_FUNC_DIR}mg_utilities.cpp ${SHARED_FUNC_DIR}mg_utilities.h 
	${CC} ${DEBUG_FLAG} -c ${SHARED_FUNC_DIR}mg_utilities.cpp

mg_image_lib.o : ${SHARED_FUNC_DIR}mg_image_lib.cpp ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${DEBUG_FLAG} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp

#point_array.o: point_array.cpp point_array.h
#	${CC} ${DEBUG_FLAG} -c $<

clean :
	rm *.o

