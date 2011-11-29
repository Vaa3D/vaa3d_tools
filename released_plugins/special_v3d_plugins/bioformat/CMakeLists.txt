
add_subdirectory( dependent )

add_library(bioformats SHARED
  bioFormat_ex.cpp
  imageIO_bioformat.cpp
  )

