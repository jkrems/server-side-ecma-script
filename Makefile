BUILDTYPE ?= Debug
BUILDDIR = build
GYP = build/gyp/gyp

all: build

build/gyp/gyp:
	svn co http://gyp.googlecode.com/svn/trunk build/gyp --revision 1831

build/Makefile: build/gyp/gyp
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

.PHONY: all build
