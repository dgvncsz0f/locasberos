# Copyright (c) 2012, Diego Souza
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#   * Neither the name of the <ORGANIZATION> nor the names of its contributors
#     may be used to endorse or promote products derived from this software
#     without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
