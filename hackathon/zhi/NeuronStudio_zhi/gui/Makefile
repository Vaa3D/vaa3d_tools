NAME    = ns-gui0.9.92.lib
O       = .obj
RM      = erase
CFLAGS  = /W3 /O2 /nologo /MT /c /TC
AR      = LIB
ARFLAGS = /nologo /out:$(NAME)
MV      = move
DEST    = ..\..\lib\

OBJS=\
app_data$(O)\
app_display$(O)\
app_lib$(O)\
app_main$(O)\
banner$(O)\
clrdlg$(O)\
dirdlg$(O)\
interactor$(O)\
nsruler$(O)\
nsseed3d$(O)\
nssubsample$(O)\
oglwin$(O)\
progress$(O)\
statusbar$(O)\
tabcontrol$(O)\

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) *$(O)
	$(RM) *$(O)
	$(MV) $(NAME) $(DEST)
