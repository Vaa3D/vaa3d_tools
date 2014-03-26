#$@ target $^ dependencies $< first dep

include ../c/Makefile-def

# For Mac OS 
MEXSUFFIX = mexmaci

# Matlab root directory. 
# Pass a different one if yours is not /Applications/MATLAB74
MATLABHOME = /Applications/MATLAB.app

MEX = $(MATLABHOME)/bin/mex
CXX =  g++
CFLAGS = -fno-common -no-cpp-precomp -fexceptions
CDIR = ../c
CLIBDIR = ../c/lib
LIBDIR = ../lib
#LIBS = -L$(CDIR) -lm -lneurolabi -L./ -L../c/lib -ltzhelper  -L$(MYERS_DIR) -lmyers -L/usr/lib -L/Applications/MATLAB_R2012b.app/bin/maci64
LIBS = -L$(MATLABHOME)/bin/maci64 -L$(CDIR) -lneurolabi -L./ -L../c/lib -ltzhelper  -L$(MYERS_DIR) -L/usr/lib $(LIBFLAGS) 
LOCALLIBDIR = /usr/local/lib
LIBFILES = $(CDIR)/libneurolabi.a $(MYERS_DIR)/libmyers.a libtzhelper.a
SRCS = tz_fftr2c.c tz_fftc2r.c tz_conv.c tz_corr.c tz_tifbg.c tz_tifthresh.c tz_tifhist.c tz_stackthresh.c tz_blocksum_dm.c tz_blockmean_dm.c tz_stackbg.c tz_stacklocmax.c tz_segdetector.c tz_segdetector2.c tz_writetifstack.c tz_alignstack.c tz_readtifstack.c
HELPERS = tz_mexarraytrans.c
OBJS = $(SRCS:.c=.o)
FUNS = $(SRCS:.c=.$(MEXSUFFIX))
INCLUDE = -I$(MATLABHOME)/extern/include -I$(MYERS_DIR) -I../c -I../c/include -I/usr/local/include -I/usr/include
MEXFLAGS = -g  #-cxx CC=$(CXX) CXX=$(CXX) LD=$(CXX) 

all: libtzhelper.a $(FUNS)

*.mexmaci: tz_mexutils.h $(CLIBDIR)/libneurolabi.a

%.mexmaci: %.c
	$(MEX) $(MEXFLAGS) $(INCLUDE) $(LIBS) $<

%.o: %.c
	gcc $(INCLUDE) -c $<

libtzhelper.a: $(HELPERS:.c=.o)
	libtool -static -o libtzhelper.a $(HELPERS:.c=.o)

clean:
	rm *.mexmaci *.o libtzhelper.a
