
SRC_FILES = $(shell find $(cfg_srcdir)/caslib -type f -name \*.c)
LNK_NAME  = libcaslib

include $(cfg_srcdir)/makefile.rules

.PHONY: compile
compile: CFLAGS += -Wall -Wextra -Wwrite-strings -Wconversion -Wno-sign-conversion -Werror -pedantic -std=c99 -I$(cfg_srcdir) -I/usr/include/libxml2
compile: $(O_OBJ_FILES) $(G_OBJ_FILES)

.PHONY: link
link: LD       = gcc
link: LDFLAGS += -lcurl -lxml2
link: $(O_LNK_FILE) $(G_LNK_FILE)

.PHONY:
install: link
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=install cp $(G_LNK_FILE) $(root)/usr/lib/$(G_LNK_FILE)
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=install cp $(O_LNK_FILE) $(root)/usr/lib/$(O_LNK_FILE)
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=finish $(root)/usr/lib

.PHONY:
clean:
