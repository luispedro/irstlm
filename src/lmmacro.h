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


#ifndef MF_LMMACRO_H
#define MF_LMMACRO_H

#ifndef WIN32
#include <sys/types.h>
#include <sys/mman.h>
#endif

#include "util.h"
#include "ngramcache.h"
#include "dictionary.h"
#include "n_gram.h"
#include "lmtable.h"

#define MAX_TOKEN_N_MAP 3

class lmmacro: public lmtable {

public:

  dictionary     *dict;
  int            *microMacroMap;
  int             microMacroMapN;
  int             selectedField;

  lmmacro(std::string lmfilename, std::istream& inp, std::istream& inpMap);
  ~lmmacro() {};

  bool loadmap(std::string lmfilename, std::istream& inp, std::istream& inpMap);
  double lprob(ngram ng); 
  double clprob(ngram ng); 
  void map(ngram *in, ngram *out);
  void One2OneMapping(ngram *in, ngram *out);
  void Micro2MacroMapping(ngram *in, ngram *out);

  inline dictionary* getDict() {
    return dict;
  };

};



#endif

