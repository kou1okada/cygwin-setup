## Process this file with automake to produce Makefile.in
#
# $Id$
#
# Copyright (c) 2000, Red Hat, Inc.
# Copyright (c) 2002, Robert Collins
#
#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.
#
#     A copy of the GNU General Public License can be found at
#     http://www.gnu.org/
#
# Written by Christopher Faylor <cgf@redhat.com>
# and Robert Collins  <rbtcollins@hotmail.com>
#
# Makefile for Cygwin installer

AUTOMAKE_OPTIONS = dist-bzip2 subdir-objects 1.6
SUBDIRS = bz2lib zlib libgetopt++

## DISTCLEANFILES = include/stamp-h include/stamp-h[0-9]*

AM_CFLAGS	= -Werror -Winline -Wall -Wpointer-arith \
  -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
  -Wmissing-declarations -Wcomments
AM_CXXFLAGS	= $(AM_CFLAGS) 

WINDRES		:= @WINDRES@

INCLUDES	= -I$(srcdir)/bz2lib -I$(srcdir)/libgetopt++/include

noinst_PROGRAMS = \
  setup

EXTRA_PROGRAMS = inilint

# to avoid false errors that assembly generates
WARNONLY_CFLAGS	= -Winline -Wall -Wpointer-arith -Wcast-align \
	-Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
	-Wmissing-declarations -Wcomments $(INCLUDES)

EXTRA_DIST = \
  check-na.bmp \
  check-no.bmp \
  check-yes.bmp \
  choose-rtarrow.bmp \
  choose-spin.bmp \
  cygwin.ico \
  inilex.l \
  iniparse.y 

BUILT_SOURCES = setup_version.c \
  	inilex.cc \
	iniparse.cc

inilint_LDADD = libgetopt++/libgetopt++.la -lstdc++ -luser32 -lkernel32 
inilint_SOURCES = \
  inilex.cc \
  inilintmain.cc \
  iniparse.cc \
  iniparse.h \
  rfc1738.cc \
  rfc1738.h \
  String++.cc \
  String++.h

setup_LDADD = res.o zlib/libzcygw.a bz2lib/libbz2.a libgetopt++/libgetopt++.la \
  -lstdc++ -luser32 -lkernel32 \
  -lcomctl32 -lole32 -lwsock32 -lnetapi32 -ladvapi32 -luuid
setup_LDFLAGS = -mwindows
setup_SOURCES = \
	archive.cc \
	archive.h \
	archive_tar.cc \
	archive_tar.h \
	archive_tar_file.cc \
	autoload.c \
	category.cc \
	category.h \
	choose.cc \
	choose.h \
	cistring.cc \
	cistring.h \
	compress.cc \
	compress.h \
	compress_bz.cc \
	compress_bz.h \
	compress_gz.cc \
	compress_gz.h \
	concat.cc \
	concat.h \
	cygpackage.cc \
	cygpackage.h \
	desktop.cc \
	desktop.h \
	dialog.cc \
	dialog.h \
	diskfull.cc \
	diskfull.h \
	download.cc \
	download.h \
	Exception.cc \
	Exception.h \
	find.cc \
	find.h \
	filemanip.cc \
	filemanip.h \
	fromcwd.cc \
	geturl.cc \
	geturl.h \
	hash.cc \
	hash.h \
	ini.cc \
	ini.h \
	inilex.cc \
	iniparse.cc \
	iniparse.h \
	install.cc \
	io_stream.cc \
	io_stream.h \
	io_stream_cygfile.cc \
	io_stream_cygfile.h \
	io_stream_file.cc \
	io_stream_file.h \
	io_stream_memory.cc \
	io_stream_memory.h \
	list.h \
	localdir.cc \
	localdir.h \
	log.cc \
	log.h \
	main.cc \
	md5.c \
	md5.h \
	MD5++.cc \
	MD5++.h \
	mkdir.cc \
	mkdir.h \
	mklink2.cc \
	mklink2.h \
	mount.cc \
	mount.h \
	msg.cc \
	msg.h \
	net.cc \
	net.h \
	netio.cc \
	netio.h \
	nio-ie5.cc \
	nio-ie5.h \
	nio-file.cc \
	nio-file.h \
	nio-ftp.cc \
	nio-ftp.h \
	nio-http.cc \
	nio-http.h \
	package_db.cc \
	package_db.h \
	package_meta.cc \
	package_meta.h \
	package_source.cc \
	package_source.h \
	package_version.cc \
	package_version.h \
	PickCategoryLine.cc \
	PickCategoryLine.h \
	PickLine.cc \
	PickLine.h \
	PickPackageLine.cc \
	PickPackageLine.h \
	PickView.cc \
	PickView.h \
	port.h \
	postinstall.cc \
	proppage.cc \
	proppage.h \
	propsheet.cc \
	propsheet.h \
	res.rc \
	resource.h \
	rfc1738.cc \
	rfc1738.h \
	root.cc \
	root.h \
	script.cc \
	script.h \
	setup_version.c \
	simpsock.cc \
	simpsock.h \
	site.cc \
	site.h \
	source.cc \
	source.h \
	splash.cc \
	splash.h \
	state.cc \
	state.h \
	String++.cc \
	String++.h \
	threebar.cc \
	threebar.h \
	version.cc \
	version.h \
	win32.cc \
	win32.h \
	window.cc \
	window.h
	
setup_version.c : $(srcdir)/ChangeLog Makefile
	V=`grep '^\$$Revision.*' $(srcdir)/ChangeLog \
	| sed -e 's/\$$Revision:* *//' \
	  -e 's/ *$$.*//'` ;\
	(echo '#define VERSION_PREFIX "%%% setup-version"';\
        echo 'static const char version_store[] = VERSION_PREFIX " '$$V'";';\
        echo 'const char *version = version_store + sizeof (VERSION_PREFIX);') > version.tmp;\
	mv version.tmp setup_version.c

%.o: %.rc
	$(WINDRES) --include-dir $(srcdir) -o $@ $<

autoload.o: autoload.c
@AMDEP_TRUE@	source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' @AMDEPBACKSLASH@
@AMDEP_TRUE@	$(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(WARNONLY_CFLAGS) $(CFLAGS) -c -o $@ $< -fno-inline-functions

inilex.o: inilex.cc
@AMDEP_TRUE@	source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' @AMDEPBACKSLASH@
@AMDEP_TRUE@	$(CXXDEPMODE) $(depcomp) @AMDEPBACKSLASH@
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
       $(AM_CPPFLAGS) $(CPPFLAGS) $(WARNONLY_CFLAGS) $(CXXFLAGS) -c -o $@ $< 

iniparse.cc iniparse.h : iniparse.y
	bison -d -o iniparse.cc $(srcdir)/iniparse.y
	@mv iniparse.cc.h iniparse.h 2>/dev/null || mv iniparse.hh iniparse.h

inilex.cc : inilex.l iniparse.h
	flex -8 $(srcdir)/inilex.l
	mv lex.yy.c inilex.cc