
.PHONY: compile
compile:
	$(APXS) -c -I$(cfg_srcdir) -L$(root)/usr/lib -lcaslib $(cfg_srcdir)/apache/mod_locasberos.c

.PHONY: install
install:
	$(LIBTOOL) --mode=install cp $(cfg_srcdir)/apache/mod_locasberos.la $(apxs_libexecdir)
