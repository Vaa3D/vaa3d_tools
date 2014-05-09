NAME    = ns-ext0.9.92.lib
O       = .obj
RM      = erase
CFLAGS  = /W4 /O2 /nologo /MT /c /TC
AR      = LIB
ARFLAGS = /nologo /out:$(NAME)
MV      = move
DEST    = ..\..\lib\

OBJS=\
circles$(O)\
filtermgr$(O)\
lock$(O)\
mc$(O)\
naas$(O)\
normal$(O)\
pageloader$(O)\
rotate$(O)\
splat$(O)\
subsample$(O)\
threshold$(O)\

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) *$(O)
	$(RM) *$(O)
	$(MV) $(NAME) $(DEST)
