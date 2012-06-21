
export root          = $(CURDIR)/dist
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

compile: .compile-caslib

link: .link-caslib

link-try: link .link-try

install: .setup_env .install-caslib

install-try: install .install-try

test: .link-try
	$(cfg_trydir)/caslib/try_caslib_dbg

clean:
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
	-$(RM) $(cfg_trydir)/caslib/try_caslib_dbg

.install-caslib:
	@$(MAKE) -C $(cfg_srcdir)/caslib install

.test-caslib:
	@$(MAKE) -C $(cfg_srcdir)/caslib test

.compile-caslib:
	@$(MAKE) -C $(cfg_srcdir)/caslib compile

.link-caslib:
	@$(MAKE) -C $(cfg_srcdir)/caslib link

.install-try: .link-try
	@$(MAKE) -C $(cfg_trydir)/caslib install

.link-try: .link-caslib
	@$(MAKE) -C $(cfg_trydir)/caslib link

.setup_env:
	@test -d $(root)         || mkdir $(root)
	@test -d $(root)/usr     || mkdir $(root)/usr
	@test -d $(root)/usr/lib || mkdir $(root)/usr/lib
	@test -d $(root)/usr/bin || mkdir $(root)/usr/bin

