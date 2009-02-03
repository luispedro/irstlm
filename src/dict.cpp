// $Id$

using namespace std;

#include "mfstream.h"
#include "mempool.h"
#include "htable.h"
#include "dictionary.h"
#include "cmd.h"


#define YES   1
#define NO    0

#define END_ENUM    {   (char*)0,  0 }



static Enum_T BooleanEnum [] = {
  {    "Yes",    YES }, 
  {    "No",     NO},
  {    "yes",    YES }, 
  {    "no",     NO},
  {    "y",    YES }, 
  {    "n",     NO},
  END_ENUM
};


int main(int argc, char **argv)
{
  char *inp=NULL; 
  char *out=NULL;
  char *intsymb=NULL;
  int freqflag=0;
	int sortflag=1;
  int size=100000;

  DeclareParams(
								"InputFile", CMDSTRINGTYPE, &inp,
								"i", CMDSTRINGTYPE, &inp,
								"OutputFile", CMDSTRINGTYPE, &out,
								"o", CMDSTRINGTYPE, &out,
								"f", CMDENUMTYPE, &freqflag,BooleanEnum,
								"Freq", CMDENUMTYPE, &freqflag,BooleanEnum,
								"sort", CMDENUMTYPE, &sortflag,BooleanEnum,
								"Size", CMDINTTYPE, &size,
								"s", CMDINTTYPE, &size,
								"IntSymb", CMDSTRINGTYPE, &intsymb,
								"is", CMDSTRINGTYPE, &intsymb,
								(char *)NULL
								);
  
  GetParams(&argc, &argv, (char*) NULL);
  
  if ((inp==NULL) || (out==NULL))
    {
      std::cerr << "Missing parameters\n";
      exit(1);
    };
  
  dictionary d(inp,size,intsymb);

	if (sortflag && !freqflag)
		sortflag=0;
	
  if (sortflag){
    dictionary sortd(&d,sortflag);
    sortd.save(out,freqflag);
  }
  else
    d.save(out,freqflag);
}










