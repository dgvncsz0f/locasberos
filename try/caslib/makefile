
SRC_FILES = $(shell find $(cfg_trydir)/caslib -type f -name \*.cc)
LNK_NAME  = try_caslib

include $(cfg_srcroot)/makefile.rules

.PHONY: compile
compile: CXXFLAGS += -W -Wall -pedantic -I$(cfg_srcdir)/caslib -I/usr/include/unittest++
compile: $(G_OBJ_FILES)

.PHONY: link
link: LD        = g++
link: LDFLAGS  += $(cfg_srcdir)/caslib/.libs/libcaslib_dbg.a -lUnitTest++
link: compile $(G_EXE_FILE)

.PHONY: install
install: link
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=install cp $(G_EXE_FILE) $(install_root)/usr/bin/$(G_EXE_FILE)

.PHONY: clean
clean:
	$(RM) $(G_EXE_FILE)
