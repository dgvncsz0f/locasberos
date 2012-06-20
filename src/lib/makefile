
cfg_distdir   = $(cfg_builddir)/lib
TRY_SRC_FILES = $(shell find $(cfg_trydir)/lib -type f -name \*.cc)
SRC_FILES     = $(shell find $(cfg_srcdir)/lib -type f -name \*.c)
LIB_NAME      = liblocasberos
TRY_EXE_NAME  = try_caslib

include $(cfg_srcroot)/makefile.rules

.PHONY:
compile: CFLAGS += -W -Wall -pedantic -std=c99
compile: $(O_OBJ_FILES) $(G_OBJ_FILES)

.PHONY:
lib: LD = gcc
lib: CFLAGS += -W -Wall -pedantic -std=c99
lib: $(O_LIB_FILE) $(G_LIB_FILE)

.PHONY: test
test: LD        = g++
test: CFLAGS   += -W -Wall -pedantic -std=c99
test: CXXFLAGS += -W -Wall -I$(cfg_srcdir)/lib -I/usr/include/unittest++
test: LDFLAGS  += -lUnitTest++
test: $(TRY_EXE_FILE)
	$(TRY_EXE_FILE)
