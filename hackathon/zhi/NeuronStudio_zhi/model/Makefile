NAME    = ns-model0.9.92.lib
O       = .obj
RM      = erase
CFLAGS  = /W4 /O2 /nologo /MT /c /TC
AR      = LIB
ARFLAGS = /nologo /out:$(NAME)
MV      = move
DEST    = ..\..\lib\

OBJS=\
nsmodel-edge$(O)\
nsmodel-filter$(O)\
nsmodel-function$(O)\
nsmodel-graft$(O)\
nsmodel-io-hoc$(O)\
nsmodel-io-nzm$(O)\
nsmodel-io-swc$(O)\
nsmodel-join$(O)\
nsmodel-junctions$(O)\
nsmodel-mask$(O)\
nsmodel-order$(O)\
nsmodel-origs$(O)\
nsmodel-remove$(O)\
nsmodel-section$(O)\
nsmodel-spines$(O)\
nsmodel-stats$(O)\
nsmodel-transform$(O)\
nsmodel-vertex$(O)\
nsmodel$(O)\
nsrayburst$(O)\
nssettings$(O)\
nsshollanalysis$(O)\
nsspines-classifier$(O)\
nsspines-transform$(O)\

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) *$(O)
	$(RM) *$(O)
	$(MV) $(NAME) $(DEST)
