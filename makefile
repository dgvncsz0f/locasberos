
export install_root  = $(CURDIR)/build
export cfg_srcroot   = $(CURDIR)
export cfg_srcdir    = $(cfg_srcroot)/src
export cfg_trydir    = $(cfg_srcroot)/try
export CCFLAGS       = 
export CFLAGS        = 
export LDFLAGS       = 
export LIBTOOLFLAGS  = --silent

export CC            = gcc
export CXX           = g++
export LIBTOOL       = libtool

compile: .setup_env .compile-lib

link: .setup_env .link-lib .link-try

dso: .setup_env .dso-apache

install: .setup_env .install-lib .install-try

test: .setup_env .install-try
	$(install_root)/usr/bin/try_caslib_dbg

clean: .clean-try
	-find $(cfg_srcdir) -type f -name \*.o -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.a -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.lo -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.la -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.o -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.a -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.lo -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.la -exec $(RM) \{\} \;
	-$(RM) -r $$(find $(cfg_trydir) -type d -name .libs)
	-$(RM) -r $$(find $(cfg_srcdir) -type d -name .libs)
	-$(RM) -r $(install_root)

.install-lib:
	@$(MAKE) -C $(cfg_srcdir)/caslib install

.test-lib:
	@$(MAKE) -C $(cfg_srcdir)/caslib test

.compile-lib:
	@$(MAKE) -C $(cfg_srcdir)/caslib compile

.link-lib:
	@$(MAKE) -C $(cfg_srcdir)/caslib link

.install-try: .link-lib
	@$(MAKE) -C $(cfg_trydir)/caslib install

.link-try:
	@$(MAKE) -C $(cfg_trydir)/caslib link

.clean-try:
	@$(MAKE) -C $(cfg_trydir)/caslib clean

.dso-apache:
	@$(MAKE) -C $(cfg_srcdir)/apache dso

.setup_env:
	test -d $(install_root)         || mkdir $(install_root)
	test -d $(install_root)/usr     || mkdir $(install_root)/usr
	test -d $(install_root)/usr/lib || mkdir $(install_root)/usr/lib
	test -d $(install_root)/usr/bin || mkdir $(install_root)/usr/bin
