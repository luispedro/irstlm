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

  // get header (selection field):
  inpMap.getline(line,MAX_LINE,'\n');
  tokenN = parseWords(line,words,MAX_TOKEN_N_MAP);
  if (tokenN != 2 || strcmp(words[0],"FIELD")!=0)
    error("ERROR: wrong header format of map file\n[correct: FIELD <int>]\n");
  selectedField = atoi(words[1]);
  if (selectedField==-1 || selectedField==-2)
    cerr << "no selected field: the whole string is used\n";
  else if (selectedField>=0 && selectedField<10)
    cerr << "selected field n. " << selectedField << "\n";
  else
    error("ERROR: wrong header format of map file\n[correct: FIELD <int>]\n");

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
  //  getDict()->incflag(0);
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

#ifdef DEBUG
  cout << " lmmacro::lprob, parameter = <" <<  micro_ng << ">\n";
#endif

  ngram macro_ng(lmtable::getDict());

  if (micro_ng.dict ==  macro_ng.dict)
    macro_ng.trans(micro_ng);  // micro to macro mapping already done
  else
    map(&micro_ng, &macro_ng); // mapping required

#ifdef DEBUG
  cout <<  "lmmacro::lprob: micro_ng = " << micro_ng << "\n";
  cout <<  "lmmacro::lprob: macro_ng = " << macro_ng << "\n";
#endif

  // ask LM with macro 
  double prob;
  prob = lmtable::lprob(macro_ng);
#ifdef DEBUG
  cout << "prob = " << prob << "\n";
#endif

  return prob; 
}; 


double lmmacro::clprob(ngram micro_ng) {

#ifdef DEBUG
  cout << " lmmacro::clprob, parameter = <" <<  micro_ng << ">\n";
#endif

  double logpr;
  ngram macro_ng(lmtable::getDict());
  map(&micro_ng, &macro_ng);

  ngram prevMicro_ng(micro_ng);
  ngram prevMacro_ng(lmtable::getDict());
  prevMicro_ng.shift();
  map(&prevMicro_ng, &prevMacro_ng);

#ifdef DEBUG
  cout <<  "lmmacro::clprob: micro_ng = " << micro_ng << "\n";
  cout <<  "lmmacro::clprob: macro_ng = " << macro_ng << "\n";
  cout <<  "lmmacro::clprob: prevMicro_ng = " << prevMicro_ng << "\n";
  cout <<  "lmmacro::clprob: prevMacro_ng = " << prevMacro_ng << "\n";
#endif

  // check if we are inside a chunk: in this case, no prob is computed
  if (prevMacro_ng == macro_ng)
    return 0.0;

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
  }

  return logpr;
}; 


void lmmacro::map(ngram *in, ngram *out)
{

  if (selectedField==-2) // the whole token is compatible with the LM words
    One2OneMapping(in, out);

  else if (selectedField==-1) // the whole token is compatible with the LM words
    Micro2MacroMapping(in, out);

  else { // select the field "selectedField" from tokens (separator is assumed to be "#")
    ngram field_ng(getDict());

    int microsize = in->size;
    for (int i=microsize; i>0; i--) {
      char curr_token[BUFSIZ];
      strcpy(curr_token, getDict()->decode(*(in->wordp(i))));
      char *field;
      if (strcmp(curr_token,"<s>") &&
	  strcmp(curr_token,"</s>") &&
	  strcmp(curr_token,"_unk_")) {
	field = strtok(curr_token, "#");
	for (int j=0; j<selectedField; j++)
	  field = strtok(0, "#");
      } else
	field = curr_token;
      if (field)
	field_ng.pushw(field);
      else {
	field_ng.pushw("_unk_");
	//      cerr << *in << "\n";
	//	error("ERROR: no separator # in token\n");
      }
    }
    if (microMacroMapN>0) 
      Micro2MacroMapping(&field_ng, out);
    else
      out->trans(field_ng);
  }
}

void lmmacro::One2OneMapping(ngram *in, ngram *out)
{

  int insize = in->size;

  // map each token of the sequence "in" into the same-length sequence "out" through the map

  for (int i=insize; i>0; i--) {

    char *outtoken = 
      lmtable::getDict()->decode((*(in->wordp(i))<microMacroMapN)?microMacroMap[*(in->wordp(i))]:lmtable::getDict()->oovcode());
    out->pushw(outtoken);
  }
  return;
}


void lmmacro::Micro2MacroMapping(ngram *in, ngram *out)
{

  int microsize = in->size;

  // map microtag sequence (in) into the corresponding sequence of macrotags (possibly shorter) (out)

  for (int i=microsize; i>0; i--) {

    char *prev_microtag = 
      (i<microsize)?getDict()->decode(*(in->wordp(i+1))):NULL;
    char *curr_microtag = getDict()->decode(*(in->wordp(i)));
    char *prev_macrotag =
      (i<microsize)?lmtable::getDict()->decode((*(in->wordp(i+1))<microMacroMapN)?microMacroMap[*(in->wordp(i+1))]:lmtable::getDict()->oovcode()):NULL;
    char *curr_macrotag = lmtable::getDict()->decode((*(in->wordp(i))<microMacroMapN)?microMacroMap[*(in->wordp(i))]:lmtable::getDict()->oovcode());

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
