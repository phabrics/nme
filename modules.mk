# $Id: modules,v 1.1 2003/05/16 21:48:04 fredette Exp $

## This is an automake include file.

# Copyright (c) 2003 Matt Fredette
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Matt Fredette.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# this automake include file must be included in all directories that
# build modules.  it updates the modules index that will be installed
# along with all of the modules, and also handles some static building
# details.  this is done with an all-local target:

#
# first, all source files in the current directory are searched for
# element "new" function declarations.  these declarations provide
# enough information to determine the published module name, the
# libtool module name, and "submodule" name.  this information is
# appended to a tme-plugins.txt file, which will be installed in
# $(pkglibdir).
#
# the choice of $(top_builddir)/tme/tme-plugins.txt is important.
# when debugging, the person doing the debugging is expected to have
# the good sense to set LTDL_LIBRARY_PATH to $(top_builddir), so
# module.c will correctly use the uninstalled plugins list for all
# "tme/" modules.
#
# this is abuse of the $(top_builddir)/tme directory, yes, since
# this directory was originally just for include files and to make
# #include <tme/FOO.h> work right when building:
modules-local:

#
# next, when building statically, either for debugging purposes or
# because we're on a weak platform, we have to use libtool's "preopen"
# mechanism.
#
# this means at least specifying all of the modules that could
# potentially be lt_dlopen'ed on any main program's link command line,
# using libtool's -dlopen option.
#
# so we append suitable -dlopen options for all modules built in
# this directory to a file, in this case
# $(top_builddir)/tme-preopen.txt.  when building statically,
# configure.ac will then substitute @TME_PREOPEN@ with: 
#
#  `cat $(top_builddir)/tme-preopen.txt`
#
# else it will substitute the empty string.  when linking programs,
# @TME_PREOPEN@ is then used on the program's link command line.
# 
# Note: this works differently now - please see comment in top Makefile for modules target
	$(top_builddir)/tme-modules.sh $(subdir) $(top_srcdir) $(pkglib_LTLIBRARIES)

# additionally, libtool, at least through version 1.5, has a
# limitation in that the pseudo-library (the .la file) must be present
# even for a preloaded module.  if we aren't debugging, everything is
# installed, so this is not a problem.
#
# however, if we are debugging, nothing is installed, so we have to do
# something to make sure that the .la files can be found.
#
# the person doing the debugging is already expected to set
# LTDL_LIBRARY_PATH to the top of the build directory, so that
# module.c can find the uninstalled tme-plugins.txt file, and libtool
# will want to look in this directory for .la files, so we simply copy
# all of the .la files into that same directory:
AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL
AM_LDFLAGS = -module -version-info 0:0:0
TME_LIBS = $(top_builddir)/generic/libtme-generic.la $(top_builddir)/libtme/libtme.la
