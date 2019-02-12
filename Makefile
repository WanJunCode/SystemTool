.PHONY: all common main module
COMPILE_TIME = $(shell date +"%Y-%M-%d-%H:%M:%S")


all: main

common:
	$(MAKE) -C $@

module:
	$(MAKE) -C $@

main: common
	$(MAKE) -C $@

clean:
	$(MAKE) -C main clean
	$(MAKE) -C common clean
	rm -rf *.log
