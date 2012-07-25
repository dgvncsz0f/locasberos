
SRC_FILES = $(shell find $(cfg_trydir)/caslib -type f -name \*.cc) $(cfg_trydir)/fixtures.cc
LNK_NAME  = try_caslib

include $(cfg_srcdir)/makefile.rules

.PHONY: compile
compile: CXXFLAGS += -Wall -Wextra -Wwrite-strings -Wconversion -Werror -pedantic -std=c++0x -I$(cfg_srcdir) -I$(cfg_trydir) -I/usr/include/unittest++
compile: $(G_OBJ_FILES)

.PHONY: link
link: LD        = g++
link: LDFLAGS  += $(cfg_srcdir)/caslib/.libs/libcaslib_dbg.a -lUnitTest++ -lcurl -lxml2
link: compile $(G_EXE_FILE)

.PHONY: install
install: link
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=install cp $(G_EXE_FILE) $(root)/usr/bin/$(G_EXE_FILE)
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=finish $(root)/usr/bin

$(G_EXE_FILE): $(cfg_srcdir)/caslib/.libs/libcaslib_dbg.a
