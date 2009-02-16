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
  char *testfile=NULL;
  char *intsymb=NULL;
  int freqflag=0;
  int sortflag=1;
  int curveflag=0;
  int curvesize=10;
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

						"curve", CMDENUMTYPE, &curveflag,BooleanEnum,
						"c", CMDENUMTYPE, &curveflag,BooleanEnum,
						"CurveSize", CMDINTTYPE, &curvesize,
						"cs", CMDINTTYPE, &curvesize,

						"TestFile", CMDSTRINGTYPE, &testfile,
						"t", CMDSTRINGTYPE, &testfile,
						(char *)NULL
						);
  
  GetParams(&argc, &argv, (char*) NULL);
  
  if (inp==NULL)
    {
      std::cerr << "\nUsage: \ndict -i=inputfile [options]\n";
      std::cerr << "(inputfile can be a corpus or a dictionary)\n\n";
      std::cerr << "Options:\n";
      std::cerr << "-o=outputfile\n";
      std::cerr << "-f=[yes|no] (compute word frequencies)\n";
      std::cerr << "-s=[yes|no] (sort dictionary by frequency)\n";
      std::cerr << "-is= (interruption symbol) \n";
      std::cerr << "-c=[yes|no] (show dictionary growth curve)\n";
      std::cerr << "-cs=curvesize (default 10)\n";
      std::cerr << "-t=testfile (compute OOV rates on test corpus)\n\n";
      exit(1);
    };

// options compatibility issues:
  if (sortflag && !freqflag)
		sortflag=0;
  if (curveflag && !freqflag)
		freqflag=1;
  if (testfile!=NULL && !freqflag) {
		freqflag=1;
 		mfstream test(testfile,ios::in);
  		if (!test){
    			cerr << "cannot open testfile: " << testfile << "\n";
    		exit(1);
  		}
		test.close();

  }
  
  
// create dictionary: generating it from training corpus, or loading it from a dictionary file
  dictionary d(inp,size,intsymb);

// show statistics on dictionary growth and OOV rates on test corpus
  if (testfile != NULL)
	d.print_curve(curvesize, d.test(curvesize, testfile));
  else if (curveflag)
	d.print_curve(curvesize);


// if outputfile is provided, write the dictionary into it
  if(out!=NULL){
	if (sortflag){
	  dictionary sortd(&d,sortflag);
	  sortd.save(out,freqflag);
	}
	else
	  d.save(out,freqflag);
  }
}

