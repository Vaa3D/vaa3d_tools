NAME    = ns-std0.9.92.lib
O       = .obj
RM      = erase
CFLAGS  = /W4 /O2 /nologo /MT /c /TC
AR      = LIB
ARFLAGS = /nologo /out:$(NAME)
MV      = move
DEST    = ..\..\lib\

OBJS=\
nsarray$(O)\
nsascii$(O)\
nsbinheap$(O)\
nsbits$(O)\
nsbytearray$(O)\
nsclosure$(O)\
nsconfigdb$(O)\
nscookie$(O)\
nsdebug$(O)\
nsdir$(O)\
nsenumeration$(O)\
nserror$(O)\
nsfile$(O)\
nsfps$(O)\
nsfunc$(O)\
nsgraph$(O)\
nshashlist$(O)\
nshashtable$(O)\
nshier$(O)\
nshistogram$(O)\
nsinit$(O)\
nsio$(O)\
nslist$(O)\
nslog$(O)\
nsmath$(O)\
nsmemory$(O)\
nsmisc$(O)\
nsmutex$(O)\
nsprimes$(O)\
nsprint$(O)\
nsprocdb$(O)\
nsprofiler$(O)\
nsprogress$(O)\
nsqueue$(O)\
nsrecdir$(O)\
nsset$(O)\
nsstack$(O)\
nsstring$(O)\
nsthread$(O)\
nstime$(O)\
nstree$(O)\
nstypes$(O)\
nsutil$(O)\
nsvalue$(O)\
nsvector$(O)\

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) *$(O)
	$(RM) *$(O)
	$(MV) $(NAME) $(DEST)
