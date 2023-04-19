
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#QMAKE_CXXFLAGS += -std=c++0x
CONFIG += C++11
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/sort_neuron_swc/
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/swc_to_maskimage
#INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/v3d_main/worm_straighten_c
#INCLUDEPATH     += $$VAA3DPATH/v3d_main/cellseg
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
#INCLUDEPATH     += ../../zhi/IVSCC_sort_swc
INCLUDEPATH     += $$(BOOST_PATH)

unix {
LIBS += -L$$VAA3DPATH/v3d_main/jba/c++
LIBS += -lv3dnewmat
}
win32 {
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64
LIBS += -llibnewmat
}


HEADERS	+= refinement.h \
           n_class.h \
#    $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc/sort_swc.h\
    $$VAA3DPATH/released_plugins_more/v3d_plugins/swc_to_maskimage/filter_dialog.h \
    ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
    nlohmann/json.hpp \
    nlohmann/adl_serializer.hpp \
    nlohmann/byte_container_with_subtype.hpp \
    nlohmann/json.hpp \
    nlohmann/json_fwd.hpp \
    nlohmann/ordered_map.hpp \
    nlohmann/detail/conversions/from_json.hpp \
    nlohmann/detail/conversions/to_chars.hpp \
    nlohmann/detail/conversions/to_json.hpp \
    nlohmann/detail/input/binary_reader.hpp \
    nlohmann/detail/input/input_adapters.hpp \
    nlohmann/detail/input/json_sax.hpp \
    nlohmann/detail/input/lexer.hpp \
    nlohmann/detail/input/parser.hpp \
    nlohmann/detail/input/position_t.hpp \
    nlohmann/detail/iterators/internal_iterator.hpp \
    nlohmann/detail/iterators/iter_impl.hpp \
    nlohmann/detail/iterators/iteration_proxy.hpp \
    nlohmann/detail/iterators/iterator_traits.hpp \
    nlohmann/detail/iterators/json_reverse_iterator.hpp \
    nlohmann/detail/iterators/primitive_iterator.hpp \
    nlohmann/detail/meta/call_std/begin.hpp \
    nlohmann/detail/meta/call_std/end.hpp \
    nlohmann/detail/meta/cpp_future.hpp \
    nlohmann/detail/meta/detected.hpp \
    nlohmann/detail/meta/identity_tag.hpp \
    nlohmann/detail/meta/is_sax.hpp \
    nlohmann/detail/meta/std_fs.hpp \
    nlohmann/detail/meta/type_traits.hpp \
    nlohmann/detail/meta/void_t.hpp \
    nlohmann/detail/output/binary_writer.hpp \
    nlohmann/detail/output/output_adapters.hpp \
    nlohmann/detail/output/serializer.hpp \
    nlohmann/detail/abi_macros.hpp \
    nlohmann/detail/exceptions.hpp \
    nlohmann/detail/hash.hpp \
    nlohmann/detail/json_custom_base_class.hpp \
    nlohmann/detail/json_pointer.hpp \
    nlohmann/detail/json_ref.hpp \
    nlohmann/detail/macro_scope.hpp \
    nlohmann/detail/macro_unscope.hpp \
    nlohmann/detail/string_concat.hpp \
    nlohmann/detail/string_escape.hpp \
    nlohmann/detail/value_t.hpp \
    nlohmann/thirdparty/hedley/hedley.hpp \
    nlohmann/thirdparty/hedley/hedley_undef.hpp \
    nlohmann/adl_serializer.hpp \
    nlohmann/byte_container_with_subtype.hpp \
    nlohmann/json.hpp \
    nlohmann/json_fwd.hpp \
    nlohmann/ordered_map.hpp

HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h
HEADERS += ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h


SOURCES	+= refinement.cpp \
           n_class.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES +=  $$VAA3DPATH/v3d_main/worm_straighten_c/mst_prim_c.cpp
SOURCES +=  $$VAA3DPATH/v3d_main/worm_straighten_c/bdb_minus.cpp
SOURCES +=  $$VAA3DPATH/v3d_main/worm_straighten_c/bfs_1root.cpp
SOURCES +=  $$VAA3DPATH/v3d_main/graph/dijk.cpp


#SOURCES	+=  $$VAA3DPATH/v3d_main/neuron_editing/neuron_format_converter.cpp
SOURCES	+=  $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp
#    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.cpp \
#    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.cpp \
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/swc_to_maskimage/filter_dialog.cpp


TARGET	= $$qtLibraryTarget(refine_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/refine_swc/
#DESTDIR	= /home/penglab/PBserver/tmp/ding/vaa3d/plugins/compare_swc
