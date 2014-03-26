Makefile_a: Makefile_a.in
	./configure $(CONFIG_OPTION)

clean:
	rm Makefile_a