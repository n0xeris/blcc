#####################################
## Configuration

SRCDIR = src
BUILDDIR = dist
EXECNAME = blcc
EXECPATH = $(BUILDDIR)/$(EXECNAME)

SRCFILES = $(shell find $(SRCDIR) -name *.c)

CC = clang
DBG = gdb
CCFLAGS = -std=c17 -Wall -Wextra -Wpedantic -Wshadow -Wwrite-strings \
	-Wnested-externs -Wredundant-decls -Wstrict-prototypes -Wno-unused-parameter \
	-Wold-style-definition -Wmissing-include-dirs

INCLUDES = -I$(SRCDIR)
LIBRARIES =

DEBUGFLAGS = -g -O0
RELEASEFLAGS = -O3


#####################################
## Building

all: build-project
	@chmod +x $(EXECPATH)

build-project: $(SRCFILES)
	@-compiledb -n make
	@mkdir -p $(BUILDDIR)
	$(CC) $(SRCFILES) -o $(EXECPATH) $(INCLUDES) $(LIBRARIES) $(CCFLAGS)


#####################################
## Utils

.PHONY: run debug docs clean

run: build-project
	@echo -----------------------------------------
	@./$(EXECPATH)

debug: build-project
	@$(DBG) $(EXECPATH)

clean:
	@rm -rf $(BUILDDIR) .cache/


#####################################
## Safeguards

check-build-type:
ifndef build
	build=release
endif


apply-build-flags:
ifeq ($(build), release)
	$(eval CCFLAGS += $(RELEASEFLAGS))
else
	$(eval CCFLAGS += $(DEBUGFLAGS))
endif


