# CMake Project Wrapper Makefile

.PHONY: all
all:
	mkdir -p build
	mkdir -p bin
	cmake -H. -Bbuild
	${MAKE} -C build
	${MAKE} -C build install
	${MAKE} -C build image

libc:
	mkdir -p build
	mkdir -p bin
	cmake -H. -Bbuild
	${MAKE} -C build libc
	${MAKE} -C build install

.PHONY: run
run: all
	${MAKE} -C build run

.PHONY: debug
debug: all
	${MAKE} -C build debug

.PHONY: clean
clean:
	rm -rf build
	rm -rf bin
