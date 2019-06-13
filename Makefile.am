ACLOCAL_AMFLAGS = -I m4

SUBDIRS = include doc po

AM_CPPFLAGS = -Iinclude
AM_CPPFLAGS += -D__STRICT_ANSI__
AM_CPPFLAGS += -DNDEBUG

WARNFLAGS = -Wall
WARNFLAGS += -Wshadow
WARNFLAGS += -pedantic-errors -Wpointer-arith -Waggregate-return
WARNFLAGS += -Wlong-long -Wredundant-decls -Wcast-qual -Wcast-align
WARNFLAGS += -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations

AM_CFLAGS = $(WARNFLAGS)
AM_CXXFLAGS = $(WARNFLAGS)

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

lib_LTLIBRARIES = libminidjvu.la

libminidjvu_la_SOURCES = src/matcher/no_mdjvu.h src/matcher/bitmaps.h	\
 src/matcher/common.h src/djvu/bs.h src/jb2/jb2coder.h			\
 src/jb2/bmpcoder.h src/jb2/zp.h src/jb2/jb2const.h			\
 src/base/mdjvucfg.h src/matcher/cuts.c src/matcher/patterns.cpp	\
 src/matcher/frames.c src/matcher/bitmaps.c src/alg/nosubst.c		\
 src/alg/erosion.c src/alg/smooth.c src/alg/delegate.c			\
 src/alg/classify.c src/alg/render.c src/alg/clean.c			\
 src/alg/adjust_y.c src/alg/blitsort.c src/alg/split.c			\
 src/alg/average.c src/alg/compress.c src/djvu/djvuload.c		\
 src/djvu/djvusave.c src/djvu/djvuinfo.c src/djvu/iff.c			\
 src/image-io/tiffload.c src/image-io/tiff.c src/image-io/pbm.c		\
 src/image-io/tiffsave.c src/image-io/bmp.c src/jb2/proto.c		\
 src/base/3graymap.c src/base/2io.c src/base/5image.c			\
 src/base/4bitmap.c src/base/version.c src/base/6string.c		\
 src/base/1error.c src/base/0porting.c src/djvu/djvudir.cpp		\
 src/djvu/bs.cpp src/jb2/jb2coder.cpp src/jb2/bmpcoder.cpp		\
 src/jb2/jb2load.cpp src/jb2/zp.cpp src/jb2/jb2save.cpp

bin_PROGRAMS = minidjvu

minidjvu_SOURCES = tools/minidjvu.c

minidjvu_LDADD = libminidjvu.la

minidjvu.pc:
	echo 'prefix=$(prefix)'			>  $@
	echo 'exec_prefix=$(exec_prefix)'	>> $@
	echo 'includedir=$(includedir)'		>> $@
	echo 'libdir=$(libdir)'			>> $@
	echo					>> $@
	echo 'Name: $(PACKAGE_TARNAME)'		>> $@
	echo 'Description: $(PACKAGE_NAME)'	>> $@
	echo 'URL: $(PACKAGE_URL)'		>> $@
	echo 'Version: $(PACKAGE_VERSION)'	>> $@
	echo 'Cflags: -I$(includedir)'		>> $@
	echo 'Libs: -L$(libdir) -lminidjvu'	>> $@
	echo 'Libs.private: $(LDFLAGS) $(LIBS)'	>> $@

pkgconfig_DATA = minidjvu.pc

MOSTLYCLEANFILES = $(pkgconfig_DATA)
