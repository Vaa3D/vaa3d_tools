NAME    = ns-render0.9.92.lib
O       = .obj
RM      = erase
CFLAGS  = /W4 /O2 /nologo /MT /c /TC
AR      = LIB
ARFLAGS = /nologo /out:$(NAME)
MV      = move
DEST    = ..\..\lib\

OBJS=\
nsrender$(O)\
nsrender2d-os-windows$(O)\
nsrender3d-opengl$(O)\

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) *$(O)
	$(RM) *$(O)
	$(MV) $(NAME) $(DEST)
