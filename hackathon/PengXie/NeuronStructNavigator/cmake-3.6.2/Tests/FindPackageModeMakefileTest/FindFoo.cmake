
find_library(FOO_LIBRARY NAMES foo HINTS "/local1/vaa3d_test/vaa3d_tools/hackathon/MK/NeuronStructNavigator/cmake-3.6.2/Tests/FindPackageModeMakefileTest" )
find_path(FOO_INCLUDE_DIR NAMES foo.h HINTS "/local1/vaa3d_test/vaa3d_tools/hackathon/MK/NeuronStructNavigator/cmake-3.6.2/Tests/FindPackageModeMakefileTest" )

set(FOO_LIBRARIES ${FOO_LIBRARY})
set(FOO_INCLUDE_DIRS "${FOO_INCLUDE_DIR}"  "/some/path/with a space/include" )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Foo  DEFAULT_MSG  FOO_LIBRARY FOO_INCLUDE_DIR )
