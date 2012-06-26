
export root            = $(CURDIR)/dist
export cfg_srcroot     = $(CURDIR)
export cfg_srcdir      = $(cfg_srcroot)/src
export cfg_trydir      = $(cfg_srcroot)/try
export apxs_libexecdir = $(shell $(APXS) -q LIBEXECDIR)
export CCFLAGS         =
export CFLAGS          =
export LDFLAGS         =
export LIBTOOLFLAGS    = --silent

export APXS          = apxs
export CC            = gcc
export CXX           = g++
export LIBTOOL       = libtool

compile-caslib:
	@$(MAKE) -C $(cfg_srcdir)/caslib compile

link-caslib: compile-caslib
	@$(MAKE) -C $(cfg_srcdir)/caslib link

install-caslib: .setup_env link-caslib
	@$(MAKE) -C $(cfg_srcdir)/caslib install

compile-try:
	@$(MAKE) -C $(cfg_trydir)/caslib compile

link-try: link-caslib compile-try
	@$(MAKE) -C $(cfg_trydir)/caslib link

install-try: .setup_env link-try
	@$(MAKE) -C $(cfg_trydir)/caslib install

compile-modapache: install-caslib
	@$(MAKE) -C $(cfg_srcdir)/apache compile

install-modapache: .setup_env compile-modapache
	@$(MAKE) -C $(cfg_srcdir)/apache install

release:
	$(cfg_srcdir)/mkversion.h

test-caslib: link-try
	env MALLOC_CHECK_=1 $(cfg_trydir)/caslib/try_caslib_dbg

test-modapache: install-modapache
	cd $(cfg_trydir)/apache && env apxs_libexecdir="$(apxs_libexecdir)" rspec -O ~/rspec.opts -P "spec/**/*_spec.rb"

clean:
	-find $(cfg_srcdir) -type f -name \*.o -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.a -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.lo -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.slo -exec $(RM) \{\} \;
	-find $(cfg_srcdir) -type f -name \*.la -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.o -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.a -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.lo -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.slo -exec $(RM) \{\} \;
	-find $(cfg_trydir) -type f -name \*.la -exec $(RM) \{\} \;
	-$(RM) -r $$(find $(cfg_trydir) -type d -name .libs)
	-$(RM) -r $$(find $(cfg_srcdir) -type d -name .libs)
	-$(RM) $(cfg_trydir)/caslib/try_caslib_dbg

.setup_env:
	@test -d $(root)            || mkdir $(root)
	@test -d $(root)/usr        || mkdir $(root)/usr
	@test -d $(root)/usr/lib    || mkdir $(root)/usr/lib
	@test -d $(root)/usr/bin    || mkdir $(root)/usr/bin
	@test -d $(apxs_libexecdir) || mkdir -p $(apxs_libexecdir)

