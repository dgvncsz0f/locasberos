
export cfg_srcroot   = $(CURDIR)
export cfg_srcdir    = $(cfg_srcroot)/src
export cfg_builddir  = $(cfg_srcroot)/build
export CC            = gcc
export CCFLAGS       = 
export CFLAGS        = 
export LD            = gcc
export LDFLAGS       = 

export LIBTOOL       = libtool

compile: .setup_env .compile-lib

lib: .setup_env .lib-lib

dso: .setup_env .dso-apache

clean:
	-find $(cfg_srcdir) -type f -name \*.o -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.a -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.lo -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.la -exec $(RM) \{\} \;
	-$(RM) -r $$(find $(cfg_srcdir) -type d -name .libs)
	-$(RM) -r $(cfg_builddir)

.compile-lib: CFLAGS += -W -Wall -pedantic -std=c99
.compile-lib:
	$(MAKE) -C $(cfg_srcdir)/lib compile

.lib-lib: .compile-lib
	$(MAKE) -C $(cfg_srcdir)/lib lib

.dso-apache:
	$(MAKE) -C $(cfg_srcdir)/apache dso

.setup_env: $(cfg_builddir) $(cfg_builddir)/lib

$(cfg_srcroot)/%:
	mkdir $@
