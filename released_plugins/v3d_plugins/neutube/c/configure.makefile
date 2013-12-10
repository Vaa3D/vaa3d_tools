Makefile-def: Makefile-def.in
	./configure $(CONFIG_OPTION)

clean:
	@if test -f Makefile-def; then rm Makefile-def; fi