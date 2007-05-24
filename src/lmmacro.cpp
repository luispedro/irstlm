/******************************************************************************
IrstLM: IRST Language Model Toolkit
Copyright (C) 2006 Marcello Federico, ITC-irst Trento, Italy

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

******************************************************************************/
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include "math.h"
#include "mempool.h"
#include "htable.h"
#include "ngramcache.h"
#include "dictionary.h"
#include "n_gram.h"
#include "lmtable.h"
#include "lmmacro.h"
#include "util.h"

using namespace std;

// local utilities: start

int parseWords(char *sentence, char **words, int max);

inline void error(char* message){
  cerr << message << "\n";
  throw runtime_error(message);
}

// local utilities: end



lmmacro::lmmacro(string lmfilename, istream& inp, istream& inpMap){
  dict = new dictionary((char *)NULL,1000000,(char*)NULL,(char*)NULL); // dict of micro tags
  microMacroMap = NULL;
  microMacroMapN = 0;

  if (!loadmap(lmfilename, inp, inpMap))
    error("Error in loadmap\n");

};


bool lmmacro::loadmap(string lmfilename, istream& inp, istream& inpMap) {

  char line[MAX_LINE];
  char* words[MAX_TOKEN_N_MAP];
  char *macroW; char *microW;
  int tokenN;

  microMacroMap = (int *)calloc(BUFSIZ, sizeof(int));


  // Load the (possibly binary) LM 
#ifdef WIN32
  lmtable::load(inp); //don't use memory map
#else
  if (lmfilename.compare(lmfilename.size()-3,3,".mm")==0)
    lmtable::load(inp,lmfilename.c_str(),NULL,1);
  else 
    lmtable::load(inp,lmfilename.c_str(),NULL,0);
#endif  

  // Load the dictionary of micro tags (to be put in "dict" of lmmacro class):
  getDict()->incflag(1);
  while (inpMap.getline(line,MAX_LINE,'\n')){
    tokenN = parseWords(line,words,MAX_TOKEN_N_MAP);
    if (tokenN != 2)
      error("ERROR: wrong format of map file\n");
    microW = words[0];
    macroW = words[1];
    getDict()->encode(microW);

#ifdef DEBUG
    cout << "\nmicroW = " << microW << "\n";
    cout << "macroW = " << macroW << "\n";
    cout << "microMacroMapN = " << microMacroMapN << "\n";
    cout << "code of micro = " <<  getDict()->getcode(microW) << "\n";
    cout << "code of macro = " <<  lmtable::getDict()->getcode(macroW) << "\n";
#endif

    if (microMacroMapN && !(microMacroMapN%BUFSIZ))
      microMacroMap = (int *)realloc(microMacroMap, sizeof(int)*(microMacroMapN+BUFSIZ));
    microMacroMap[microMacroMapN++] = lmtable::getDict()->getcode(macroW);
  }
  getDict()->incflag(0);
  getDict()->genoovcode();

#ifdef DEBUG
  cout << "oovcode(micro)=" <<  getDict()->oovcode() << "\n";
  cout << "oovcode(macro)=" <<  lmtable::getDict()->oovcode() << "\n";

  cout << "microMacroMapN = " << microMacroMapN << "\n";
  cout << "macrodictsize  = " << lmtable::getDict()->size() << "\n";
  cout << "microdictsize  = " << getDict()->size() << "\n";

  for (int i=0; i<microMacroMapN; i++) {
    cout << "micro[" << getDict()->decode(i) << "] -> " << lmtable::getDict()->decode(microMacroMap[i]) << "\n";
  }
#endif
  return true; 
};


double lmmacro::lprob(ngram micro_ng) {

  ngram macro_ng(lmtable::getDict());

  if (micro_ng.dict ==  macro_ng.dict)
    macro_ng.trans(micro_ng);  // micro to macro mapping already done
  else
    map(&micro_ng, &macro_ng); // mapping required

  //  cout <<  "micro_ng = " << micro_ng << "\n";
  //  cout <<  "macro_ng = " << macro_ng << "\n";

  // ask LM with macro 
  double prob;
  prob = lmtable::lprob(macro_ng);
  //  cout << "prob = " << prob << "\n";

  return prob; 
}; 


double lmmacro::clprob(ngram micro_ng) {

  double logpr;
  ngram macro_ng(lmtable::getDict());

  map(&micro_ng, &macro_ng);

  if (macro_ng.size==0) return 0.0;

  if (macro_ng.size>maxlev) macro_ng.size=maxlev;


  //cache hit
  if (probcache && macro_ng.size==maxlev && probcache->get(macro_ng.wordp(maxlev),(char *)&logpr)){
    return logpr;
  }

  //cache miss
  logpr=lmmacro::lprob(macro_ng);

  if (probcache && macro_ng.size==maxlev){
     probcache->add(macro_ng.wordp(maxlev),(char *)&logpr);
  };

  return logpr;
}; 



void lmmacro::map(ngram *in, ngram *out)
{

  int microsize = in->size;

  // map microtag sequence (in) into the corresponding sequence of macrotags (possibly shorter) (out)

  for (int i=microsize; i>0; i--) {

    char *prev_microtag = 
      (i<microsize)?getDict()->decode(*(in->wordp(i+1))):NULL;
    char *curr_microtag = getDict()->decode(*(in->wordp(i)));
    char *prev_macrotag =
      (i<microsize)?lmtable::getDict()->decode(microMacroMap[*(in->wordp(i+1))]):NULL;
    char *curr_macrotag = lmtable::getDict()->decode(microMacroMap[*(in->wordp(i))]);

    if (prev_macrotag == NULL ||
	strcmp(curr_macrotag,prev_macrotag) != 0 ||
	!((prev_microtag[strlen(prev_microtag)-1]== '(' &&  curr_microtag[strlen(curr_microtag)-1]==')' ) ||
	  (prev_microtag[strlen(prev_microtag)-1]== '(' &&  curr_microtag[strlen(curr_microtag)-1]=='+' ) ||
	  (prev_microtag[strlen(prev_microtag)-1]== '+' &&  curr_microtag[strlen(curr_microtag)-1]=='+' ) ||
	  (prev_microtag[strlen(prev_microtag)-1]== '+' &&  curr_microtag[strlen(curr_microtag)-1]==')' )))
      out->pushw(curr_macrotag);
  }
  return;
}
