AM_CFLAGS =
AM_CXXFLAGS = -DMYCODESIZE=3 -Wall 
AM_CPPFLAGS = 

h_sources = \
        cmd.h \
        dictionary.h \
	gzfilebuf.h \
        htable.h \
        lmmacro.h \
        lmtable.h \
        mempool.h \
        mfstream.h \
        ngram.h \
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
	ngram.cpp \
	ngramcache.cpp \
	ngramtable.cpp \
	util.cpp

lib_LTLIBRARIES = libirstlm.la
libirstlm_la_SOURCES = $(h_sources) $(c_sources) $(cpp_sources)

#Install the headers in a versioned directory - e.g. examplelib-1.0:
library_includedir=$(includedir)/irstlm
library_include_HEADERS = $(h_sources)
