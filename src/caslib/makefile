
SRC_FILES = $(shell find $(cfg_srcdir)/caslib -type f -name \*.c)
LNK_NAME  = libcaslib

include $(cfg_srcroot)/makefile.rules

.PHONY: compile
compile: CFLAGS += -W -Wall -pedantic -std=c99
compile: $(O_OBJ_FILES) $(G_OBJ_FILES)

.PHONY: link
link: LD      = gcc
link: CFLAGS += -W -Wall -pedantic -std=c99
link: $(O_LNK_FILE) $(G_LNK_FILE)

.PHONY:
install: link
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=install cp $(G_LNK_FILE) $(install_root)/usr/lib/$(G_LNK_FILE)
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=install cp $(O_LNK_FILE) $(install_root)/usr/lib/$(O_LNK_FILE)

.PHONY:
clean:
