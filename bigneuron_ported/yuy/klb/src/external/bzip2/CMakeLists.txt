project ( bzip2 )
cmake_minimum_required ( VERSION 2.8 )

IF( CMAKE_COMPILER_IS_GNUCC )
	add_definitions ( -D_FILE_OFFSET_BITS=64 -fPIC )
ENDIF()


# Library
set ( BZIP2_SRCS  crctable.c randtable.c compress.c decompress.c bzlib.c blocksort.c huffman.c)

IF ( NOT TARGET bz2lib )
add_library ( bz2lib ${BZIP2_SRCS} libbz2.def )
ENDIF()

IF ( NOT TARGET bzip2 )
add_executable ( bzip2 bzip2.c )
target_link_libraries ( bzip2 bz2lib )
ENDIF()

IF ( NOT TARGET bzip2recover )
add_executable ( bzip2recover bzip2recover.c )
ENDIF()

