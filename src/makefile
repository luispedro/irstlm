# // $Id$

# makefile
# irstlm
#
# Created by Marcello Federico on 12/12/06.
# Copyright 2006 ITC-irst. All rights reserved.

CPPEXE=g++


#MAC OS X cannot handle -static compilation
ifeq ($(OSTYPE),darwin)
       CPP=$(CPPEXE) 
     else
       CPP=$(CPPEXE) -static
endif

#CPPFLAGS = -g -DMYCODESIZE=3  -Wall
CPPFLAGS = -O3 -DMYCODESIZE=3  -Wall --no-strict-aliasing

#IRSTLM LIBRARY
LIBOBJ = cmd.o util.o mempool.o htable.o dictionary.o n_gram.o ngramcache.o lmtable.o lmmacro.o mfstream.o ngramtable.o
LIBDIR = ../lib/$(MACHTYPE)
LIB = $(LIBDIR)/libirstlm.a

BINDIR = ../bin/$(MACHTYPE)

%.o : %.cpp %.h
	$(CPP) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(LIB): $(LIBOBJ)
	ar r $(LIB) $?

#TARGET COMMANDS
all: version compile-lm quantize-lm prune-lm dict ngt
clean:;rm *.o

compile-lm: compile-lm.cpp  $(LIB)
	$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $< $(LIB) -lz

quantize-lm: quantize-lm.cpp  $(LIB)
	$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $< util.o -lz	

prune-lm: prune-lm.cpp  $(LIB)
	$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $< $(LIB) -lz	

dict: dict.cpp  $(LIB)
	$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $< $(LIB) -lz	

ngt: ngt.cpp  $(LIB)
	$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $< $(LIB) -lz

version: 
	$(CPP) --version	
