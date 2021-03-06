
export root            = $(CURDIR)/dist
export cfg_rootdir     = $(CURDIR)
export cfg_srcdir      = $(cfg_rootdir)/src
export cfg_trydir      = $(cfg_rootdir)/try
export nginx_srcdir    = $(CURDIR)/../nginx-1.2.1
export apxs_libexecdir = $(shell $(APXS) -q LIBEXECDIR)
export CCFLAGS         =
export CFLAGS          =
export LDFLAGS         =
export LIBTOOLFLAGS    = --silent
export APXSFLAGS       =

export APXS            = $(foreach apxs, /usr/bin/apxs /usr/sbin/apxs /usr/bin/apxs2 /usr/sbin/apxs2, $(shell [ -x "$(apxs)" ] && echo "$(apxs)"))
export CC              = gcc
export CXX             = g++
export LIBTOOL         = libtool

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

compile-modnginx: install-caslib
	cd $(nginx_srcdir) && ./configure --add-module=$(cfg_srcdir)/nginx/ --prefix=$(cfg_trydir)/tmp/nginx
	@$(MAKE)

install-modapache: .setup_env compile-modapache
	@$(MAKE) -C $(cfg_srcdir)/apache install

install-modnginx: .setup_env compile-modnginx
	@$(MAKE) -C $(nginx_srcdir) install

release:
	$(cfg_srcdir)/mkversion.h

test-caslib: link-try
	env MALLOC_CHECK_=1 $(cfg_trydir)/caslib/try_caslib_dbg

test-modapache: install-modapache
	cd $(cfg_trydir)/functional && WEBSERVERS=Apache bundle exec rspec -O ~/rspec.opts -P "spec/**/*_spec.rb"

test-modnginx: install-modnginx
	cd $(cfg_trydir)/functional && WEBSERVERS=Nginx bundle exec rspec -O ~/rspec.opts -P "spec/**/*_spec.rb"

test-modules: install-modapache install-modnginx
	cd $(cfg_trydir)/functional && WEBSERVERS=Apache,Nginx bundle exec rspec -O ~/rspec.opts -P "spec/**/*_spec.rb"

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
	-$(RM) -r $$(find $(cfg_trydir)/tmp -type d -name "d2*")
	-$(RM) $(cfg_trydir)/caslib/try_caslib_dbg

.setup_env:
	@test -d $(root)            || mkdir $(root)
	@test -d $(root)/usr        || mkdir $(root)/usr
	@test -d $(root)/usr/lib    || mkdir $(root)/usr/lib
	@test -d $(root)/usr/bin    || mkdir $(root)/usr/bin
	@test -d $(root)/$(apxs_libexecdir) || mkdir -p $(root)/$(apxs_libexecdir)

