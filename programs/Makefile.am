AM_CFLAGS =
AM_CXXFLAGS = -DMYCODESIZE=3 -Wall -I../src
AM_CPPFLAGS = 

bin_PROGRAMS = dict ngt compile_lm interpolate_lm prune_lm quantize_lm prune_lm
dict_SOURCES = dict.cpp
ngt_SOURCES = ngt.cpp
compile_lm_SOURCES = compile-lm.cpp
interpolate_lm_SOURCES = interpolate-lm.cpp
prune_lm_SOURCES = prune-lm.cpp
quantize_lm_SOURCES = quantize-lm.cpp

LIBS = -lz
LIBIRSTLM = ../src/libirstlm.la

dict_LDADD  = $(LIBIRSTLM)
ngt_LDADD  = $(LIBIRSTLM)
compile_lm_LDADD  = $(LIBIRSTLM)
interpolate_lm_LDADD  = $(LIBIRSTLM)
prune_lm_LDADD  = $(LIBIRSTLM)
quantize_lm_LDADD  = $(LIBIRSTLM)
