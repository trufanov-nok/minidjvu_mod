CFLAGS:=-pipe -g -I. \
        -Wall -Wshadow -pedantic-errors \
        -Wpointer-arith -Waggregate-return \
        -Wlong-long -Winline -Wredundant-decls -Wcast-qual -Wcast-align \
        -D__STRICT_ANSI__  -fPIC  -DHAVE_TIFF


C_OPTS:=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations

LDFLAGS:=-shared -ltiff # -version-info 1:0:0 #-rpath /usr/local/lib
OBJDIR:=obj
SUBDIRS:=minidjvu \
         minidjvu/base/0porting \
         minidjvu/base/1error \
         minidjvu/base/2io \
         minidjvu/base/3graymap \
         minidjvu/base/4bitmap \
         minidjvu/base/5image \
         minidjvu/alg/smooth \
         minidjvu/alg/split \
         minidjvu/alg/clean \
         minidjvu/alg/nosubst \
         minidjvu/alg/blitsort \
         minidjvu/alg/patterns \
         minidjvu/alg/classify \
         minidjvu/alg/adjust_y \
         minidjvu/alg/erosion \
         minidjvu/alg/jb2 \
         minidjvu/alg/delegate \
         minidjvu/formats/pbm \
         minidjvu/formats/bmp \
         minidjvu/formats/iff \
         minidjvu/formats/tiff \
         minidjvu/formats/djvu
THISFILE:=Makefile

CSOURCES:=$(wildcard $(addsuffix /*.c,$(SUBDIRS)))
CPPSOURCES:=$(wildcard $(addsuffix /*.cpp,$(SUBDIRS)))
HEADERS:=$(wildcard $(addsuffix /*.h,$(SUBDIRS)))
COBJECTS:=$(addprefix $(OBJDIR)/,$(CSOURCES:.c=.o))
CPPOBJECTS:=$(addprefix $(OBJDIR)/,$(CPPSOURCES:.cpp=.o))
OBJECTS:=$(COBJECTS) $(CPPOBJECTS)
CC:=gcc
CXX:=g++

.PHONY: all program install

all: bin/libminidjvu.so program

program:
	@cd src && make
#	$(CXX) $(LDFLAGS) $(CFLAGS) $(OBJECTS) src/minidjvu.c -o bin/minidjvu

install: all
	libtool install -c bin/libminidjvu.la /usr/local/lib/libminidjvu.la
	libtool install -c bin/minidjvu /usr/local/bin/minidjvu

clean:
	rm -f $(OBJECTS)
        
bin/libminidjvu.so: $(OBJECTS)
	mkdir -p $(dir $@)        
	$(CXX) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.c $(HEADERS) $(THISFILE)
	mkdir -p $(dir $@)
	$(CC) $(C_OPTS) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.cpp $(HEADERS) $(THISFILE)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@
