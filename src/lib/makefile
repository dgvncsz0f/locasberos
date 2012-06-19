
SOURCE_FILES = $(shell find $(cfg_srcdir)/lib -type f -name \*.c)
OBJECT_FILES = $(addsuffix .o, $(basename $(SOURCE_FILES)))
TARGET_FILE  = liblocasberos.la

cfg_distdir  = $(cfg_builddir)/lib

include $(cfg_srcroot)/makefile.rules
