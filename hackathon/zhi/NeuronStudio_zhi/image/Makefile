NAME    = ns-image0.9.92.lib
O       = .obj
RM      = erase
CFLAGS  = /W4 /O2 /nologo /MT /c /TC
AR      = LIB
ARFLAGS = /nologo /out:$(NAME)
MV      = move
DEST    = ..\..\lib\

OBJS=\
nscolor$(O)\
nscolordb$(O)\
nsimage$(O)\
nsimagedir$(O)\
nsimagepath$(O)\
nsimageserver$(O)\
nsio-writejpeg$(O)\
nsio-writeraw$(O)\
nsio-writetiff$(O)\
nspixels-blur$(O)\
nspixels-brightnesscontrast$(O)\
nspixels-convert-imp$(O)\
nspixels-convert$(O)\
nspixels-copy$(O)\
nspixels-dynamicrange$(O)\
nspixels-flip$(O)\
nspixels-gammacorrect$(O)\
nspixels-getpixel$(O)\
nspixels-mpfilter$(O)\
nspixels-noise$(O)\
nspixels-orthoproject$(O)\
nspixels-resize$(O)\
nspixels-setpixel$(O)\
nspixels-subsample$(O)\
nspixels$(O)\
nssampler$(O)\
nsseed$(O)\
nssplats$(O)\
nstile$(O)\
nstilemgr$(O)\
nsvoxel-buffer$(O)\
nsvoxel-info$(O)\
nsvoxel-table$(O)\
raw$(O)\
tiff$(O)\

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) *$(O)
	$(RM) *$(O)
	$(MV) $(NAME) $(DEST)
