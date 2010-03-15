// $Id$

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

#ifndef MF_DICTIONARY_H
#define MF_DICTIONARY_H

#include "mfstream.h"
#include <cstring>
#include <iostream>


#define MAX_WORD 1000
#define LOAD_FACTOR  5

#ifndef DICT_INITSIZE
#define DICT_INITSIZE 100000
#endif

//Begin of sentence symbol
const char* const BOS_ = "<s>";

//End of sentence symbol
const char* const EOS_ = "</s>";

//Out-Of-Vocabulary symbol
const char* const OOV_ = "<unk>";

struct dict_entry {
  const char *word;
  int  code;
  long long  freq;
};

class strstack;
class htable;

class dictionary {
  strstack   *st;  //!< stack of strings
  dict_entry *tb;  //!< entry table
  htable    *htb;  //!< hash table
  int          n;  //!< number of entries
  long long    N;  //!< total frequency
  int        lim;  //!< limit of entries
  int   oov_code;  //!< code assigned to oov words
  char*       is;  //!< interruption symbol list
  char       ifl;  //!< increment flag
  int        dubv; //!< dictionary size upper bound
  int in_oov_lex;  //!< flag
  int oov_lex_code; //!< dictionary
  char* oov_str;    //!< oov string

  dictionary(const dictionary&); //!< not implemented
 public:

  friend class dictionary_iter;

  dictionary* oovlex; //<! additional dictionary

  inline int dub() const {return dubv;}
  inline int dub(int value){return (dubv=value);}

  inline const char *OOV() const {return OOV_;}
  inline const char *BoS() const {return BOS_;}
  inline const char *EoS() const {return EOS_;}

  inline int oovcode() const {return oov_code;}
  inline int oovcode(int v) {return oov_code=(v>=0?v:oov_code);}

  const char* intsymb() const { return is; }
  inline const char *intsymb (const char* isymb){
    assert(isymb);
    if (is!=NULL) delete [] is;
    is=new char[strlen(isymb+1)];
    strcpy(is,isymb);
    return is;
  }

  inline int incflag() const {return ifl;}
  inline int incflag(int v) {return ifl=v;}
  inline int oovlexsize() const {return oovlex?oovlex->n:0;}
  inline int inoovlex() const {return in_oov_lex;}
  inline int oovlexcode() const {return oov_lex_code;}

  int getword(fstream& inp , char* buffer);

  inline void genoovcode(){
    int c=encode(OOV());
    //std::cerr << "OOV code is "<< c << std::endl;
    oovcode(c);
  }

  inline dictionary* oovlexp(char *fname=NULL){
    if (fname==NULL) return oovlex;
    if (oovlex!=NULL) delete oovlex;
    oovlex=new dictionary(fname,DICT_INITSIZE);
    return oovlex;
  }

  inline int setoovrate(double oovrate){
    encode(OOV()); //be sure OOV code exists
    int oovfreq=(int)(oovrate * totfreq());
    std::cerr << "setting OOV rate to: " << oovrate << " -- freq= " << oovfreq << std::endl;
    return freq(oovcode(),oovfreq);
  }


  inline long long incfreq(int code,long long value) {
      N+=value;
      return tb[code].freq+=value;
  }

  inline long long multfreq(int code,double value){
    N+=(long long)(value * tb[code].freq)-tb[code].freq;
    return tb[code].freq=(long long)(value * tb[code].freq);
  }

  inline long freq(int code,long long value=-1){
    if (value>=0){
      N+=value-tb[code].freq;
      tb[code].freq=value;
    }
    return tb[code].freq;
  }

  inline long long totfreq() const { return N; }

  void grow();
  //dictionary(int size=400,char* isym=NULL,char* oovlex=NULL);
  dictionary(char *filename=NULL,int size=DICT_INITSIZE,char* isymb=NULL,char* oovlex=NULL);
  dictionary(dictionary* d, int sortflag=1); //flag for sorting wrt to frequency (default=1, i.e. sort)

  ~dictionary();
  void generate(char *filename);
  void load(char *filename);
  void save(char *filename, int freqflag=0);
  void load(std::istream& fd);
  void save(std::ostream& fd);

  int size(){return n;};
  int getcode(const char *w);
  int encode(const char *w);
  const char *decode(int c);
  void stat();

  void print_curve(int curvesize, float* testOOV=NULL);
  float* test(int curvesize, const char *filename, int listflag=0);	// return OOV statistics computed on test set

  void cleanfreq(){
    for (int i=0;i<n;tb[i++].freq=0);
    N=0;
  }

};

class dictionary_iter {
 public:
  dictionary_iter(dictionary *dict);
  dict_entry* next();
 private:
  dictionary* m_dict;
};

#endif

