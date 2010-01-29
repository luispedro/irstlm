# what flags you want to pass to the C compiler & linker
AM_CFLAGS =
AM_CXXFLAGS = -DMYCODESIZE=3 -Wall 
AM_CPPFLAGS = 

h_sources = \
        cmd.h \
        dictionary.h \
	gzfilebuf.h \
        htable.h \
	index.h \
        lmmacro.h \
        lmtable.h \
        mempool.h \
        mfstream.h \
        n_gram.h \
        ngramcache.h \
        ngramtable.h \
        util.h

c_sources = \
	cmd.c

cpp_sources = \
	dictionary.cpp \
	htable.cpp \
	lmmacro.cpp \
	lmtable.cpp \
	mempool.cpp \
	mfstream.cpp \
	n_gram.cpp \
	ngramcache.cpp \
	ngramtable.cpp \
	util.cpp

lib_LTLIBRARIES = libirstlm.la
libirstlm_la_SOURCES = $(h_sources) $(c_sources) $(cpp_sources)

#Install the headers in a versioned directory - e.g. examplelib-1.0:
library_includedir=$(includedir)
library_include_HEADERS = $(h_sources)

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = dict ngt compile_lm interpolate_lm prune_lm quantize_lm prune_lm
dict_SOURCES = dict.cpp
ngt_SOURCES = ngt.cpp
compile_lm_SOURCES = compile-lm.cpp
interpolate_lm_SOURCES = interpolate-lm.cpp
prune_lm_SOURCES = prune-lm.cpp
quantize_lm_SOURCES = quantize-lm.cpp

LIBS = -lz
LIBIRSTLM = libirstlm.la

dict_LDADD  = $(LIBIRSTLM)
ngt_LDADD  = $(LIBIRSTLM)
compile_lm_LDADD  = $(LIBIRSTLM)
interpolate_lm_LDADD  = $(LIBIRSTLM)
prune_lm_LDADD  = $(LIBIRSTLM)
quantize_lm_LDADD  = $(LIBIRSTLM)
