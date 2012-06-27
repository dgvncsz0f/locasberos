
my_libexecdir = $(addprefix $(root), $(apxs_libexecdir))
la_module     = $(cfg_srcdir)/apache/mod_locasberos.la

.PHONY: compile
compile: $(la_module)

.PHONY: install
install: $(la_module)
	$(LIBTOOL) --mode=install cp $^ $(my_libexecdir)

%.la: %.c
	$(APXS) -c -I$(cfg_srcdir) -L$(root)/usr/lib -lcaslib $^

.SUFFIX: .la
