NAME    = ns-math0.9.92.lib
O       = .obj
RM      = erase
CFLAGS  = /W4 /O2 /nologo /MT /c /TC
AR      = LIB
ARFLAGS = /nologo /out:$(NAME)
MV      = move
DEST    = ..\..\lib\

OBJS=\
nsaabbox$(O)\
nscircle$(O)\
nsconv$(O)\
nscube$(O)\
nsdragrect$(O)\
nsfrustum$(O)\
nsinterpolate$(O)\
nsline$(O)\
nsmatrix$(O)\
nsoctree$(O)\
nsplane$(O)\
nspoint$(O)\
nsprojector$(O)\
nsrandom$(O)\
nsray$(O)\
nssphere$(O)\
nstriangle$(O)\
nstuple$(O)\
nsvector$(O)\

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) *$(O)
	$(RM) *$(O)
	$(MV) $(NAME) $(DEST)
