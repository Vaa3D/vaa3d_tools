include Makefile-def

.PHONY: all

all:
	@make -f Makefile.ctc
	@make -f Makefile.bin OBJDIR=$(DEBUG_DIR)

test:
	@echo $(VPATH)

.PHONY: tags

tags:
	@make -f Makefile.bin tags

clean:
	@make -f Makefile.ctc clean
	@make -f Makefile.bin clean OBJDIR=$(DEBUG_DIR)
	@make -f Makefile.bin clean OBJDIR=$(RELEASE_DIR)

.PHONY: lib

lib:
	@make -f Makefile.ctc lib
	@make -f Makefile.bin lib OBJDIR=$(DEBUG_DIR) VERSION=${VERSION}

.PHONY: dylib

dylib:
	@make -f Makefile.ctc dylib
	@make -f Makefile.bin dylib OBJDIR=$(DEBUG_DIR) VERSION=${VERSION}

.PHONY: release

release:
	@make lib_release

lib_release:
	@make -f Makefile.ctc lib
ifneq (,$(PROG))
	@make -f Makefile.bin  CFLAGS="$(RELEASE_CFLAGS)" OBJDIR=$(RELEASE_DIR) DEBUG_FLAG=false VERSION=${VERSION}
else
	@make lib -f Makefile.bin CFLAGS="$(RELEASE_CFLAGS)" OBJDIR=$(RELEASE_DIR) DEBUG_FLAG=false VERSION=${VERSION}
endif

.PHONY: dylib_release

dylib_release:
	@make -f Makefile.ctc lib
	@make -f Makefile.bin dylib OBJDIR=$(RELEASE_DIR) DEBUG_FLAG=false DEBUG_FLAG=false

.PHONY: doc

doc:
	@if which doxygen; then doxygen; else echo 'Cannot find doxygen. Please make sure it has been installed.'; fi

.PHONY: install

install: 
	make -f Makefile.bin install INSTALL_DIR=$(INSTALL_DIR)

.PHONY: stitch_package

stitch_package: 
	@if test -d ../app/stitch_package; then rm -r ../app/stitch_package; fi
	@make release PROG=stitchstack
	@if test ! -d ../app/stitch_package; then mkdir ../app/stitch_package; fi
	@if test ! -d ../app/stitch_package/bin; then mkdir ../app/stitch_package/bin; fi
	@if test ! -d ../app/stitch_package/tmp; then mkdir ../app/stitch_package/tmp; fi
	#@cp ../app/conn.txt ../app/stitch_package/
	@cp bin/stitchstack ../app/stitch_package/bin
	@cp stitch_all.sh ../app/stitch_package

app_package: 
	@if test -d ../app/stitch_package; then rm -r ../app/stitch_package; fi
	@make release PROG=stitchstack
	@if test ! -d ../app/stitch_package; then mkdir ../app/stitch_package; fi
	@if test ! -d ../app/stitch_package/bin; then mkdir ../app/stitch_package/bin; fi
	@if test ! -d ../app/stitch_package/tmp; then mkdir ../app/stitch_package/tmp; fi
	@if test -f ../app/conn.txt; then cp ../app/conn.txt ../app/stitch_package/; fi
	@cp bin/stitchstack ../app/stitch_package/bin
	@cp stitch_all.sh ../app/stitch_package
	@make release PROG=drawmask
	@cp bin/drawmask ../app/stitch_package/bin
	@make release PROG=edswc
	@cp bin/edswc ../app/stitch_package/bin
	@make release PROG=edswcfc
	@cp bin/edswcfc ../app/stitch_package/bin
	@make release PROG=edtube
	@cp bin/edtube ../app/stitch_package/bin
	#@make release PROG=soma
	#@cp bin/soma ../app/stitch_package/bin
	@echo app_package created

puncta_package:
	@if test -d ../app/puncta_package; then rm -r ../app/puncta_package; fi
	@make release PROG=mask_puncta
	@if test ! -d ../app/puncta_package; then mkdir ../app/puncta_package; fi
	@if test ! -d ../app/puncta_package/bin; then mkdir ../app/puncta_package/bin; fi
	@if test ! -d ../app/puncta_package/tmp; then mkdir ../app/puncta_package/tmp; fi
	@cp bin/mask_puncta ../app/puncta_package/bin
	@make release PROG=detection_score
	@cp bin/detection_score ../app/puncta_package/bin
	@make release PROG=meanshift_puncta
	@cp bin/meanshift_puncta ../app/puncta_package/bin
	@make release PROG=combine_image_channels
	@cp bin/combine_image_channels ../app/puncta_package/bin
	@make release PROG=convert2bitrawto4bitraw
	@cp bin/convert2bitrawto4bitraw ../app/puncta_package/bin
	@make release PROG=assign_puncta
	@cp bin/assign_puncta ../app/puncta_package/bin
	@make release PROG=seperate_puncta
	@cp bin/seperate_puncta ../app/puncta_package/bin
	@make release PROG=test_temporary
	@cp bin/test_temporary ../app/puncta_package/bin
	@echo puncta_package create

trace_package: 
	@if test -d ../app/trace_package; then rm -r ../app/trace_package; fi
	@if test ! -d ../app/trace_package; then mkdir ../app/trace_package; fi
	@if test ! -d ../app/trace_package/bin; then mkdir ../app/trace_package/bin; fi
	@if test ! -d ../app/trace_package/lib; then mkdir ../app/trace_package/lib; fi
	@sh -x deploy_trace_package.sh

diadem_d1:
	make release PROG=diadem_d1
	@cp bin/diadem_d1 ../app

diadem_package: 
	@if test ! -d /Users/zhaot/Work/diadem/bin; then mkdir /Users/zhaot/Work/diadem/bin; fi
	@cp ../shell/assert_succ.sh /Users/zhaot/Work/diadem/shell
	@cp $(EXT_LIB_DIR)/libgsl*.dylib /Users/zhaot/Work/diadem/bin
	@make release PROG=diadem_c1_data
	@cp bin/diadem_c1_data /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/diadem_c1_data
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/diadem_c1_data
	@make release PROG=diadem_d1_data
	@cp bin/diadem_d1_data /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/diadem_d1_data
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/diadem_d1_data
	@make release PROG=diadem_d1_stitch
	@cp bin/diadem_d1_stitch /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/diadem_d1_stitch
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/diadem_d1_stitch
	@make release PROG=diadem_e1_data
	@cp bin/diadem_e1_data /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/diadem_e1_data
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/diadem_e1_data
	@make release PROG=imop
	@cp bin/imop /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/imop
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/imop
	@make release PROG=edswc
	@cp bin/edswc /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/edswc
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/edswc
	@make release PROG=imbinarize
	@cp bin/imbinarize /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/imbinarize
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/imbinarize
	@make release PROG=bwsolid
	@cp bin/bwsolid /Users/zhaot/Work/diadem/bin
	@cp $(EXT_LIB_DIR)/libgsl*.dylib /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/bwsolid
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/bwsolid
	@make release PROG=rmsobj
	@cp bin/rmsobj /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/rmsobj
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/rmsobj
	@make release PROG=extract_line
	@cp bin/extract_line /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/extract_line
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/extract_line
	@make release PROG=imexpr
	@cp bin/imexpr /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/imexpr
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/imexpr
	@make release PROG=trace_seed
	@cp bin/trace_seed /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/trace_seed
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/trace_seed
	@make release PROG=sort_seed
	@cp bin/sort_seed /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/sort_seed
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/sort_seed
	@make release PROG=drawmask
	@cp bin/drawmask /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/drawmask
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/drawmask
	@make release PROG=trace_neuron3
	@cp bin/trace_neuron3 /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/trace_neuron3
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/trace_neuron3
	@make release PROG=reconstruct_neuron
	@cp bin/reconstruct_neuron /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/reconstruct_neuron
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/reconstruct_neuron
	@cp ../app/Extension_Chains /Users/zhaot/Work/diadem/bin
	@install_name_tool -change /usr/local/lib/64bit//libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/Extension_Chains
	@install_name_tool -change /usr/local/lib/64bit//libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/Extension_Chains
	@cp ../app/GTracing /Users/zhaot/Work/diadem/bin
	@install_name_tool -change /usr/local/lib/64bit//libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/GTracing
	@install_name_tool -change /usr/local/lib/64bit//libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/GTracing
	@cp ../../jun/grow_d1 /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/grow_d1
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/grow_d1
	@cp bin/stitchstack /Users/zhaot/Work/diadem/bin
	@install_name_tool -change $(EXT_LIB_DIR)/libgsl.0.dylib @executable_path/libgsl.0.dylib /Users/zhaot/Work/diadem/bin/stitchstack
	@install_name_tool -change $(EXT_LIB_DIR)/libgslcblas.0.dylib @executable_path/libgslcblas.0.dylib /Users/zhaot/Work/diadem/bin/stitchstack
	@echo diadem_package created

diadem_final_package:
	@make PROG=bwsolid
	@make PROG=diadem_c1_data
	@make PROG=diadem_d1
	@make PROG=diadem_d1_merge
	@make PROG=diadem_d1_split
	@make PROG=diadem_d1_watershed
	@make PROG=diadem_e1_data
	@make PROG=drawmask
	@make PROG=edswc
	@make PROG=extract_line
	@make PROG=imbinarize
	@make PROG=imexpr
	@make PROG=imop
	@make PROG=reconstruct_neuron
	@make PROG=regionmerge
	@make PROG=rmsobj
	@make PROG=sort_seed
	@make PROG=stitchstack
	@make PROG=trace_neuron3
	@make PROG=trace_seed
