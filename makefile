
export cfg_srcroot   = $(CURDIR)
export cfg_srcdir    = $(cfg_srcroot)/src
export cfg_builddir  = $(cfg_srcroot)/build
export cfg_trydir    = $(cfg_srcroot)/try
export CCFLAGS       = 
export CFLAGS        = 
export LDFLAGS       = 
export LIBTOOLFLAGS  = --silent

export CC            = gcc
export CXX           = g++
export LIBTOOL       = libtool

compile: .setup_env .compile-lib

lib: .setup_env .lib-lib

dso: .setup_env .dso-apache

test: .setup_env .test-lib

clean:
	-find $(cfg_srcdir) -type f -name \*.o -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.a -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.lo -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.la -exec $(RM) \{\} \;
	-$(RM) -r $$(find $(cfg_srcdir) -type d -name .libs)
	-$(RM) -r $(cfg_builddir)

.test-lib:
	$(MAKE) -C $(cfg_srcdir)/lib test

.compile-lib:
	$(MAKE) -C $(cfg_srcdir)/lib compile

.lib-lib:
	$(MAKE) -C $(cfg_srcdir)/lib lib

.dso-apache:
	$(MAKE) -C $(cfg_srcdir)/apache dso

.setup_env:
	test -d $(cfg_builddir) || mkdir $(cfg_builddir)
	test -d $(cfg_builddir)/lib || mkdir $(cfg_builddir)/lib
