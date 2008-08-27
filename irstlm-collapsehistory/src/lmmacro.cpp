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

// #define DEBUG

using namespace std;

// local utilities: start

int parseWords(char *sentence, char **words, int max);

inline void error(char* message){
  cerr << message << "\n";
  throw runtime_error(message);
}

void lmmacro::cutLex(ngram *in, ngram *out)
{
  *out=*in;

  char *curr_macro = out->dict->decode(*(out->wordp(1)));
  out->shift();
  char *p = strrchr(curr_macro, '_');
  int lexLen;
  if (p)
    lexLen=strlen(p);
  else 
    lexLen=0;
  char curr_NoLexMacro[BUFSIZ];
  memset(&curr_NoLexMacro,0,BUFSIZ);
  strncpy(curr_NoLexMacro,curr_macro,strlen(curr_macro)-lexLen);
  out->pushw(curr_NoLexMacro);
  return;
}

// local utilities: end



lmmacro::lmmacro(string lmfilename, istream& inp, istream& inpMap){
  dict = new dictionary((char *)NULL,1000000,(char*)NULL,(char*)NULL); // dict of micro tags
  microMacroMap = NULL;
  microMacroMapN = 0;
  lexicaltoken2classMap = NULL;
  lexicaltoken2classMapN = 0;

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

  // get header (selection field and, possibly, the classes of lemmas):
  inpMap.getline(line,MAX_LINE,'\n');
  tokenN = parseWords(line,words,MAX_TOKEN_N_MAP);
  if (tokenN < 2 || strcmp(words[0],"FIELD")!=0)
    error("ERROR: wrong header format of map file\n[correct: FIELD <int> (file of lexical classes, only if <int> > 9)]\n");
  selectedField = atoi(words[1]);
  if ( (selectedField==-1 || selectedField==-2) && tokenN==2)
    cerr << "no selected field: the whole string is used\n";
  else if ((selectedField>=0 && selectedField<10) && tokenN==2)
    cerr << "selected field n. " << selectedField << "\n";
  else if (selectedField>9 && selectedField<100 && tokenN==2)
    cerr << "selected field is " << selectedField/10 << " lexicalized with field " << selectedField%10 << " (no lexical classes)\n";
  else if (selectedField>9 && selectedField<100 && tokenN==3)
    cerr << "selected field is " << selectedField/10 << " lexicalized with classes from field " << selectedField%10 << "\n";
  else
    error("ERROR: wrong header format of map file\n[correct: FIELD <int> (file of lexical classes, only if <int> > 9)]\n");

  // Load the classes of lexicalization tokens:
  if (tokenN==3)
    loadLexicalClasses(words[2]);

  // Load the dictionary of micro tags (to be put in "dict" of lmmacro class):
  getDict()->incflag(1);
  lmtable::getDict()->incflag(1);
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
    microMacroMap[microMacroMapN++] = lmtable::getDict()->encode(macroW);
  }
  lmtable::getDict()->incflag(0);
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


void lmmacro::loadLexicalClasses(char *fn)
{
  char line[MAX_LINE];
  char* words[MAX_TOKEN_N_MAP];
  int tokenN;

  lexicaltoken2classMap = (int *)calloc(BUFSIZ, sizeof(int));
  lexicaltoken2classMapN = BUFSIZ;

  lmtable::getDict()->incflag(1);

  inputfilestream inp(fn);
  while (inp.getline(line,MAX_LINE,'\n')){
    tokenN = parseWords(line,words,MAX_TOKEN_N_MAP);
    if (tokenN != 2)
      error("ERROR: wrong format of lexical classes file\n");
    else {
      int classIdx = atoi(words[1]);
      int wordCode = lmtable::getDict()->encode(words[0]);

      if (wordCode>=lexicaltoken2classMapN) {
	int r = (wordCode-lexicaltoken2classMapN)/BUFSIZ;
	lexicaltoken2classMapN += (r+1)*BUFSIZ;
	lexicaltoken2classMap = (int *)realloc(lexicaltoken2classMap, sizeof(int)*lexicaltoken2classMapN);
      }
      lexicaltoken2classMap[wordCode] = classIdx;
    }
  }

  lmtable::getDict()->incflag(0);

#ifdef DEBUG
  for (int x=0; x<lmtable::getDict()->size(); x++)
    cout << "class of <" << lmtable::getDict()->decode(x) << "> (code=" << x << ") = " << lexicaltoken2classMap[x] << endl;
#endif

  return;
}


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
  ngram macroNoLex_ng(lmtable::getDict());

  //  cout << "\n\nMAP MICRO TO MACRO:\n";
  map(&micro_ng, &macro_ng);

#if 1
  if (macro_ng.size==0) return 0.0;

  if (macro_ng.size>maxlev) macro_ng.size=maxlev;

  //cache hit
  if (probcache && macro_ng.size==maxlev && probcache->get(macro_ng.wordp(maxlev),(char *)&logpr))
    return logpr;

  //cache miss
  logpr=lmmacro::lprob(macro_ng);

  if (probcache && macro_ng.size==maxlev)
    probcache->add(macro_ng.wordp(maxlev),(char *)&logpr);

#else /* deactived code following */
  ngram prevMicro_ng(micro_ng);
  ngram prevMacro_ng(lmtable::getDict());
  prevMicro_ng.shift();

  //  cout << "\n\nMAP PREVMICRO TO PREVMACRO:\n";
  map(&prevMicro_ng, &prevMacro_ng); // for saving time, prevMacro_ng could be extracted directly
                                     // during the mapping from micro_ng to macro_ng

#ifdef DEBUG
  cout <<  "lmmacro::clprob: micro_ng = " << micro_ng << "\n";
  cout <<  "lmmacro::clprob: macro_ng = " << macro_ng << "\n";
  cout <<  "lmmacro::clprob: prevMicro_ng = " << prevMicro_ng << "\n";
  cout <<  "lmmacro::clprob: prevMacro_ng = " << prevMacro_ng << "\n";
#endif

  // check if we are inside a chunk: in this case, no prob is computed

  //  cout << "\n\nCHECK:\n";
  //  cout << "  prevMacro_ng " << prevMacro_ng << endl;
  //  for (int i=prevMacro_ng.size;i>0;i--) {
  //cout << "word[" << i << "] = " << *(prevMacro_ng.wordp(i)) << endl;
  //}
  //cout << "  macro_ng " << macro_ng << endl;
  //for (int i=macro_ng.size;i>0;i--)
  //  cout << "word[" << i << "] = " << *(macro_ng.wordp(i)) << endl;

  if (selectedField<10) {

    if (prevMacro_ng == macro_ng) 
      return 0.0;

#ifdef DEBUG
    cout << "  QUERY MACRO LM on " << macro_ng << "\n";
#endif

    if (macro_ng.size==0) return 0.0;

    if (macro_ng.size>maxlev) macro_ng.size=maxlev;

    //cache hit
    if (probcache && macro_ng.size==maxlev && probcache->get(macro_ng.wordp(maxlev),(char *)&logpr))
      return logpr;

    //cache miss
    logpr=lmmacro::lprob(macro_ng);

    if (probcache && macro_ng.size==maxlev)
      probcache->add(macro_ng.wordp(maxlev),(char *)&logpr);

  } else {

    cutLex(&macro_ng, &macroNoLex_ng);
#ifdef DEBUG
    cout << "  macroNoLex_ng = " << macroNoLex_ng << endl;
#endif
    if (prevMacro_ng == macroNoLex_ng) 
      {
#ifdef DEBUG
	cout << "  DO NOT QUERY MACRO LM " << endl;
#endif
	return 0.0;
      }

#ifdef DEBUG
    cout << "  QUERY MACRO LM on " << prevMacro_ng << "\n";
#endif

    if (prevMacro_ng.size==0) return 0.0;

    if (prevMacro_ng.size>maxlev) prevMacro_ng.size=maxlev;

    //cache hit
    if (probcache && prevMacro_ng.size==maxlev && probcache->get(prevMacro_ng.wordp(maxlev),(char *)&logpr))
      return logpr;

    //cache miss
    logpr=lmmacro::lprob(prevMacro_ng);

    if (probcache && prevMacro_ng.size==maxlev)
      probcache->add(prevMacro_ng.wordp(maxlev),(char *)&logpr);
  }
#endif /* old code */

  return logpr;
}; 

//maxsuffptr returns the largest suffix of an n-gram that is contained 
//in the LM table. This can be used as a compact representation of the 
//(n-1)-gram state of a n-gram LM. if the input k-gram has k>=n then it 
//is trimmed to its n-1 suffix.

const char *lmmacro::maxsuffptr(ngram micro_ng){  
//cerr << "lmmacro::maxsuffptr\n";
//cerr << "micro_ng: " << micro_ng	
//	<< " -> micro_ng.size: " << micro_ng.size << "\n";

//the LM working on the selected field = 0
//contributes to the LM state
//  if (selectedField>0)    return NULL;

  ngram macro_ng(lmtable::getDict());

  if (micro_ng.dict ==  macro_ng.dict)
    macro_ng.trans(micro_ng);  // micro to macro mapping already done
  else
    map(&micro_ng, &macro_ng); // mapping required

#ifdef DEBUG
  cout <<  "lmmacro::lprob: micro_ng = " << micro_ng << "\n";
  cout <<  "lmmacro::lprob: macro_ng = " << macro_ng << "\n";
#endif

  if (macro_ng.size==0) return (char*) NULL;
  if (macro_ng.size>=maxlev) macro_ng.size=maxlev-1;
  
  ngram ng=macro_ng;
  //ngram ng(lmtable::getDict()); //eventually use the <unk> word
  //ng.trans(macro_ng);
  
  if (get(ng,ng.size,ng.size))
    return ng.link;
  else{ 
    macro_ng.size--;
    return maxsuffptr(macro_ng);
  }
}

const char *lmmacro::cmaxsuffptr(ngram micro_ng){
//cerr << "lmmacro::CMAXsuffptr\n";
//cerr << "micro_ng: " << micro_ng	
//	<< " -> micro_ng.size: " << micro_ng.size << "\n";

//the LM working on the selected field = 0
//contributes to the LM state
//  if (selectedField>0)    return NULL;

  ngram macro_ng(lmtable::getDict());

  if (micro_ng.dict ==  macro_ng.dict)
    macro_ng.trans(micro_ng);  // micro to macro mapping already done
  else
    map(&micro_ng, &macro_ng); // mapping required

#ifdef DEBUG
  cout <<  "lmmacro::lprob: micro_ng = " << micro_ng << "\n";
  cout <<  "lmmacro::lprob: macro_ng = " << macro_ng << "\n";
#endif

  if (macro_ng.size==0) return (char*) NULL;
  if (macro_ng.size>=maxlev) macro_ng.size=maxlev-1;

  char* found;
  
  if (statecache && (macro_ng.size==maxlev-1) && statecache->get(macro_ng.wordp(maxlev-1),(char *)&found))
    return found;
  
  found=(char *)maxsuffptr(macro_ng);
  
  if (statecache && macro_ng.size==maxlev-1){
    //if (statecache->isfull()) statecache->reset();
    statecache->add(macro_ng.wordp(maxlev-1),(char *)&found);    
  }; 
  
  return found;
}


void lmmacro::map(ngram *in, ngram *out)
{

#ifdef DEBUG
  cout << "In lmmacro::map, in = " << *in << endl;
  cout << " (selectedField = " << selectedField << " )\n";
#endif
  if (selectedField==-2) // the whole token is compatible with the LM words
    One2OneMapping(in, out);

  else if (selectedField==-1) // the whole token has to be mapped before querying the LM
    Micro2MacroMapping(in, out);

  else if (selectedField<10) { // select the field "selectedField" from tokens (separator is assumed to be "#")
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
  } else { 
    // selectedField>=10: tens=idx of micro tag (possibly to be mapped to
    // macro tag), unidx=idx of lemma to be concatenated by "_" to the
    // (mapped) tag

    int tagIdx = selectedField/10;
    int lemmaIdx = selectedField%10;

    // micro (or mapped to macro) sequence construction:
    ngram tag_ng(getDict());
    char *lemmas[BUFSIZ];

    int microsize = in->size;
    for (int i=microsize; i>0; i--) {
      char curr_token[BUFSIZ];
      strcpy(curr_token, getDict()->decode(*(in->wordp(i))));
      char *tag = NULL, *lemma = NULL;

      if (strcmp(curr_token,"<s>") &&
	  strcmp(curr_token,"</s>") &&
	  strcmp(curr_token,"_unk_")) {
	
	if (tagIdx<lemmaIdx) {
	  tag = strtok(curr_token, "#");
	  for (int j=0; j<tagIdx; j++)
	    tag = strtok(0, "#");
	  for (int j=tagIdx; j<lemmaIdx; j++)
	    lemma = strtok(0, "#");
	} else {
	  lemma = strtok(curr_token, "#");
	  for (int j=0; j<lemmaIdx; j++)
	    lemma = strtok(0, "#");
	  for (int j=lemmaIdx; j<tagIdx; j++)
	    tag = strtok(0, "#");
	}

#ifdef DEBUG
	printf("(tag,lemma) = %s %s\n", tag, lemma);
#endif
      } else {
	tag = curr_token;
	lemma = curr_token;
#ifdef DEBUG
	printf("(tag=lemma) = %s %s\n", tag, lemma);
#endif
      }
      if (tag) {
	tag_ng.pushw(tag);
	lemmas[i] = strdup(lemma);
      } else {
	tag_ng.pushw("_unk_");
	lemmas[i] = strdup("_unk_");
	//      cerr << *in << "\n";
	//	error("ERROR: no separator # in token\n");
      }
    }
    if (microMacroMapN>0) 
      Micro2MacroMapping(&tag_ng, out, lemmas);
    else
      out->trans(tag_ng); // qui si dovrebbero sostituire i tag con tag_lemma, senza mappatura!

#ifdef DEBUG
    cout << "In lmmacro::map, FINAL out = " << *out << endl;
#endif

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


#undef LEXICALIZE_PREPS_ONLY
#define LEXICALIZE_PC_AND_VC

void lmmacro::Micro2MacroMapping(ngram *in, ngram *out, char **lemmas)
{

  int microsize = in->size;

  // map microtag sequence (in) into the corresponding sequence of macrotags (possibly shorter) (out)

  char *in_chunk = NULL, *chunk_lex = NULL;
  bool collapse = false;

  for (int i=1; i<=microsize; i++) {

    int curr_code = *(in->wordp(i));
    char *curr_macrotag = lmtable::getDict()->decode((curr_code<microMacroMapN)?microMacroMap[curr_code]:lmtable::getDict()->oovcode());
    char *curr_microtag = getDict()->decode(curr_code);
    int curr_len = strlen(curr_microtag)-1;

    char *curr_lemma = lemmas ? lemmas[i] : NULL;
#if defined(LEXICALIZE_PREPS_ONLY)
    if(chunk_lex) chunk_lex = curr_lemma;
#elif defined(LEXICALIZE_PC_AND_VC)
    if(chunk_lex && !strcmp(in_chunk, "[PC]")) chunk_lex = curr_lemma;
#endif

    if (i==1) {
      out->addw(curr_microtag);

      if(curr_microtag[curr_len] == ')' || curr_microtag[curr_len] == '+') {
        in_chunk = curr_macrotag;
        chunk_lex = curr_lemma;
        if(curr_microtag[0] == '(' && curr_microtag[curr_len] != ')')
          collapse = true;
      } else
        collapse = true;
    } else {
      if(in_chunk && !strcmp(curr_macrotag, in_chunk)) {
        if(curr_microtag[curr_len] == '+') {
          if(!collapse)
            out->addw(curr_microtag);
        } else {
          if(!collapse) {
            collapse = true;
            if(curr_microtag[0] == '(' && curr_microtag[curr_len] != ')')
              out->addw(curr_microtag);
          } else
            add_macrotag(out, in_chunk, chunk_lex);

          if(curr_microtag[0] == '(') {
            in_chunk = chunk_lex = NULL;
            if(curr_microtag[curr_len] == ')')
              add_macrotag(out, curr_macrotag, curr_lemma);
          } else
            chunk_lex = curr_lemma;
        }
      } else if(in_chunk) {
        if(collapse)
          add_macrotag(out, in_chunk, chunk_lex);
        else
          collapse = true;

        if(curr_microtag[curr_len] == ')' || curr_microtag[curr_len] == '+') {
          in_chunk = curr_macrotag;
          chunk_lex = curr_lemma;
        } else {
          in_chunk = chunk_lex = NULL;
          add_macrotag(out, curr_macrotag, curr_lemma);
        }
      } else {
        if(curr_microtag[0] == '(') {
          add_macrotag(out, curr_macrotag, curr_lemma);
        } else if(curr_microtag[curr_len] == '+' || curr_microtag[curr_len] == ')') {
          in_chunk = curr_macrotag;
          chunk_lex = curr_lemma;
        } else
          out->addw(curr_microtag);
      }
    }

    if(lemmas) free(lemmas[i]);
  }

  if(in_chunk)
    out->addw(in_chunk);

#ifdef DEBUG
  cout << "In Micro2MacroMapping, in    = " <<  *in  << "\n";
  cout << "                       out   = " <<  *out  << "\n";
#endif

  return;
}

int lmmacro::add_macrotag(ngram *out, const char *macrotag, const char *chunk_lex) {
#if defined(LEXICALIZE_PREPS_ONLY)
  if(chunk_lex && !strcmp(macrotag, "[PC]")) {
#elif defined(LEXICALIZE_PC_AND_VC)
  if(chunk_lex && (!strcmp(macrotag, "[PC]") || !strcmp(macrotag, "[VC]"))) {
#else
  if(chunk_lex) {
#endif
    size_t len = strlen(macrotag) + 1 + strlen(chunk_lex) + 1;
    char buf[len];
    sprintf(buf, "%s_%s", macrotag, chunk_lex);
    return out->addw(buf);
  } else
    return out->addw(macrotag);
}
