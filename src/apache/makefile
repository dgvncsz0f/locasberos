
my_libexecdir = $(addprefix $(root), $(apxs_libexecdir))
la_module     = $(cfg_srcdir)/apache/mod_authn_locasberos.la

.PHONY: compile
compile: $(la_module)

.PHONY: install
install: $(la_module)
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=install cp $^ $(my_libexecdir)

%.la: %.c
	$(APXS) -Wc,-Wall -Wc,-Wextra -Wc-Wwrite-strings -Wc,-g -c -I$(cfg_srcdir) -L$(root)/usr/lib -lcaslib_dbg $^

.SUFFIX: .la
