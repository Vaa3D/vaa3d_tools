FIND_PROGRAM (AWK NAMES awk gawk PATH /usr/bin /bin /local/bin /usr/local/bin)
MARK_AS_ADVANCED(FORCE AWK)
if (NOT AWK)
  message(FATAL_ERROR "Can not find awk, CMake will exit.")
  return()
endif (NOT AWK)
SET (AWK_FOUND TRUE)

# Syntax: 
#   add_p_file(<outputlist> <p file>)
# Adds a build rule for the specified p file. The absolute paths of the generated 
# files are added to the <outputlist> list.
function(add_p_file outputlist filename)
  # Only process *.p files
  get_filename_component(ext ${filename} EXT)
  if(NOT ext STREQUAL ".p")
    return()
  endif()

  get_filename_component(base ${filename} NAME_WE)
  set(base_abs ${GENELIB_DIR}/${base})
  set(OUTPUT_FILE ${base_abs}.c)
  set(${outputlist} ${${outputlist}} ${OUTPUT_FILE} PARENT_SCOPE)

  add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    COMMAND ${AWK} -f ${GENELIB_DIR}/manager.awk ${filename} > ${OUTPUT_FILE}
    DEPENDS "${filename}"
  )

  set_source_files_properties(${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
endfunction(add_p_file)

set(GENELIB_SRCS_TMP
  cdf.p
  fct_min.p
  image_filters.p
  lu_matrix.p
  xfig.p
  contour_lib.p
  fct_root.p
  image_lib.p
  utilities.p  
  draw_lib.p
  hash.p
  level_set.p
  water_shed.p
  fft.p
  tiff_io.p
  tiff_image.p
  cdf.h
  contour_lib.h
  draw_lib.h
  fct_min.h
  fct_root.h
  fft.h
  hash.h
  image_filters.h
  image_lib.h
  level_set.h
  lu_matrix.h
  tiff_image.h
  tiff_io.h
  utilities.h
  water_shed.h
  xfig.h
)

set(GENELIB_SRCS)

foreach(filename ${GENELIB_SRCS_TMP})
  list(APPEND GENELIB_SRCS ${GENELIB_DIR}/${filename})
endforeach()

unset(GENELIB_SRCS_TMP)

foreach(filename ${GENELIB_SRCS})
  add_p_file(GENERATED_GENELIB_SRCS ${filename})
endforeach()
