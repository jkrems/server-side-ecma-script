BUILDTYPE ?= Debug
BUILDDIR = build
GYP = build/gyp/gyp

all: build

build/gyp/gyp:
	svn co http://gyp.googlecode.com/svn/trunk build/gyp --revision 1831

deps/v8:
	git clone git@github.com:v8/v8-git-mirror.git deps/v8
	cd deps/v8 && git checkout 3.31.44

deps/uv:
	git clone git@github.com:libuv/libuv.git deps/uv
	cd deps/uv && git checkout v1.0.1

setup: build/gyp/gyp deps/v8 deps/uv

build/Makefile:
	$(GYP)	\
		-Duv_library=static_library \
		-Goutput_dir=. \
		-f make \
		-Iconfig.gypi \
		-Icommon.gypi \
		-Ideps/uv/common.gypi \
		--depth=. \
		--generator-output=$(BUILDDIR) \
		server-side-ecma-script.gyp

clean:
	rm -rf build/$(BUILDTYPE)
	rm -f build/Makefile build/server-side-ecma-script.*

build: build/Makefile
	$(MAKE) -C $(BUILDDIR)

run: build
	@./build/$(BUILDTYPE)/server-side-ecma-script

debug: build
	@lldb ./build/$(BUILDTYPE)/server-side-ecma-script

.PHONY: all build
