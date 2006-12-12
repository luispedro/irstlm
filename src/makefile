# makefile
# irstlm
#
# Created by Marcello Federico on 12/12/06.
# Copyright 2006 ITC-irst. All rights reserved.

#CPP=g++ -static -O3 -DMYCODESIZE=3 -DBOS_="\"_start_\"" -DEOS_="\"_end_\"" 
#CPP=g++ -static -O3 -DMYCODESIZE=3 -DBOS_="\"<S>\"" -DEOS_="\"</S>\"" -DOOV_="\"<unkn>\""
#CPP=g++ -pg -g  -DMYCODESIZE=3 

CPP      = g++ -g  
CPPFLAGS = -O3 -DMYCODESIZE=3  

#IRSTLM LIBRARY
LIBOBJ = cmd.o util.o mempool.o htable.o dictionary.o n_gram.o ngramcache.o lmtable.o
LIBDIR = ../bin/$(MACHTYPE)
LIB = $(LIBDIR)/libirstlm.a

BINDIR = ../bin/$(MACHTYPE)

%.o : %.cpp %.h
	$(CPP) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
	
$(LIB): $(LIBOBJ)
	ar r $(LIB) $?

#TARGET COMMANDS
all: compile-lm quantize-lm
clean:;rm *.o

compile-lm: compile-lm.cpp  $(LIB)
	$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $< $(LIB) -lz
	
quantize-lm: quantize-lm.cpp  $(LIB)
	$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $< $(LIB) -lz	
scn 