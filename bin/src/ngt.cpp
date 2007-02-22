using namespace std;

#include <cmath>
#include "mfstream.h"
#include "mempool.h"
#include "htable.h"
#include "dictionary.h"
#include "n_gram.h"
#include "ngramtable.h"
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
  char *dic=NULL;       // dictionary filename 
  char *subdic=NULL;    // subdictionary filename 
  char *filterdict=NULL;    // subdictionary filename 
  char *isym=NULL;      // interruption symbol
  char *aug=NULL;       // augmentation data
  char *hmask=NULL;        // historymask
	int inputgoogleformat=0;    //reads ngrams in Google format
  int outputgoogleformat=0;    //print ngrams in Google format
  int ngsz=0;           // n-gram default size 
  int dstco=0;          // compute distance co-occurrences 
  int bin=NO;
  int ss=NO;            //generate single table
  int LMflag=NO;        //work with LM table
  int inplen=0;         //input length for mask generation
  int tlm=0;           //test lm table
  char* ftlm=NULL;     //file to test LM table
  int memuse=NO;
	
  DeclareParams(
								"Dictionary", CMDSTRINGTYPE, &dic,
								"d", CMDSTRINGTYPE, &dic,
								"IntSymb", CMDSTRINGTYPE, &isym,
								"is", CMDSTRINGTYPE, &isym,
								"NgramSize", CMDSUBRANGETYPE, &ngsz, 1 , MAX_NGRAM,
								"n", CMDSUBRANGETYPE, &ngsz, 1 , MAX_NGRAM,
								"InputFile", CMDSTRINGTYPE, &inp,
								"i", CMDSTRINGTYPE, &inp,
								"OutputFile", CMDSTRINGTYPE, &out,
								"o", CMDSTRINGTYPE, &out,
                "InputGoogleFormat", CMDENUMTYPE, &inputgoogleformat, BooleanEnum,
                "gooinp", CMDENUMTYPE, &inputgoogleformat, BooleanEnum,
								"OutputGoogleFormat", CMDENUMTYPE, &outputgoogleformat, BooleanEnum,
								"gooout", CMDENUMTYPE, &outputgoogleformat, BooleanEnum,
								"SaveBinaryTable", CMDENUMTYPE, &bin, BooleanEnum,
								"b", CMDENUMTYPE, &bin, BooleanEnum,
								"LmTable", CMDENUMTYPE, &LMflag, BooleanEnum,
								"lm", CMDENUMTYPE, &LMflag, BooleanEnum,
								"DistCo", CMDINTTYPE, &dstco, 
								"dc", CMDINTTYPE, &dstco, 
								"AugmentFile", CMDSTRINGTYPE, &aug,
								"aug", CMDSTRINGTYPE, &aug,
								"SaveSingle", CMDENUMTYPE, &ss, BooleanEnum,
								"ss", CMDENUMTYPE, &ss, BooleanEnum,
								"SubDict", CMDSTRINGTYPE, &subdic,
								"sd", CMDSTRINGTYPE, &subdic,
								"FilterDict", CMDSTRINGTYPE, &filterdict,
								"fd", CMDSTRINGTYPE, &filterdict,
								"ConvDict", CMDSTRINGTYPE, &subdic,
								"cd", CMDSTRINGTYPE, &subdic,
								"HistoMask",CMDSTRINGTYPE, &hmask,
								"hm",CMDSTRINGTYPE, &hmask,
								"InpLen",CMDINTTYPE, &inplen,
								"il",CMDINTTYPE, &inplen,
								"tlm", CMDENUMTYPE, &tlm, BooleanEnum,
								"ftlm", CMDSTRINGTYPE, &ftlm,
								"memuse", CMDENUMTYPE, &memuse, BooleanEnum,
								(char *)NULL
								);
  
  GetParams(&argc, &argv, (char*) NULL);
  
  if (inp==NULL){
    cerr <<"No input was specified\n";
    exit(1);
  };
	
  if (out==NULL)
    cerr << "Warning: no output file specified!\n";
	
  
  TABLETYPE table_type=COUNT;
	
  if (LMflag){
    cerr << "Working with LM table\n";
    table_type=LEAFPROB;
  }
	
	//ngramtable* ngt=new ngramtable(inp,ngsz,isym,dic,dstco,hmask,inplen,table_type);
	ngramtable* ngt=new ngramtable(inp,ngsz,isym,dic,filterdict,inputgoogleformat,dstco,hmask,inplen,table_type);
		
  if (aug){
    ngt->dict->incflag(1);
		//    ngramtable ngt2(aug,ngsz,isym,NULL,0,NULL,0,table_type);
    ngramtable ngt2(aug,ngsz,isym,NULL,NULL,0,0,NULL,0,table_type);
    ngt->augment(&ngt2);
    ngt->dict->incflag(0);
  }
	
  if (subdic){
    int c=0;
    dictionary sd(subdic,500000,isym,NULL);
		
		//    ngramtable *ngt2=new ngramtable(NULL,ngsz,NULL,NULL,0,0,NULL,0,table_type);
    ngramtable *ngt2=new ngramtable(NULL,ngsz,NULL,NULL,NULL,0,0,NULL,0,table_type);
		ngt2->dict->load(subdic);
    ngt2->dict->cleanfreq();
		
    //possibly include standard symbols
    if (ngt->dict->encode(ngt->dict->EoS())!=ngt->dict->oovcode()){
      ngt2->dict->incflag(1);
      ngt2->dict->encode(ngt2->dict->EoS());
      ngt2->dict->encode(ngt2->dict->BoS());
      ngt2->dict->incflag(0);
    }
		
    ngram ng(ngt->dict);
    ngram ng2(ngt2->dict);
    
    ngt->scan(ng,INIT,ngsz);
    while (ngt->scan(ng,CONT,ngsz)){
      ng2.trans(ng);
      ngt2->put(ng2);
      if (!(++c % 1000000)) cerr << ".";
    }
		
    //makes ngt2 aware of oov code
    int oov=ngt2->dict->getcode(ngt2->dict->OOV());
    if(oov>=0) ngt2->dict->oovcode(oov);
		
    for (int i=0;i<ngt->dict->size();i++){
      ngt2->dict->incfreq(ngt2->dict->encode(ngt->dict->decode(i)),
													ngt->dict->freq(i));
    }
		
    cerr <<" oov: " << ngt2->dict->freq(ngt2->dict->oovcode()) << "\n";
		
    delete ngt;
    ngt=ngt2;
    
  }
  
  if (ngsz < ngt->maxlevel() && hmask){
    cerr << "start projection of ngramtable " << inp 
		<< " according to hmask\n"; 
		
    int i,c;
    int selmask[MAX_NGRAM];
		
    //parse hmask
    i=0; selmask[i++]=1;
    for (c=0;c< (int)strlen(hmask);c++){
      cerr << hmask[c] << "\n";
      if (hmask[c] == '1')
				selmask[i++]=c+2;
    }
		
    if (i!= ngsz){
      cerr << "wrong mask: 1 bits=" << i << " maxlev=" << ngsz << "\n";
      exit(1);
    }
		
    if (selmask[ngsz-1] >  ngt->maxlevel()){
      cerr << "wrong mask: farest bits=" << selmask[ngsz-1] 
			<< " maxlev=" << ngt->maxlevel() << "\n";
      exit(1);
    }
		
		//    ngramtable* ngt2=new ngramtable(NULL,ngsz,NULL,NULL,0,NULL,0,table_type);
    ngramtable* ngt2=new ngramtable(NULL,ngsz,NULL,NULL,NULL,0,0,NULL,0,table_type);
		
    ngt2->dict->incflag(1);
    
    ngram ng(ngt->dict);
    ngram png(ngt->dict,ngsz);
    ngram ng2(ngt2->dict,ngsz);    
    
    ngt->scan(ng,INIT,ngt->maxlevel());
    while (ngt->scan(ng,CONT,ngt->maxlevel())){
      //projection
      for (i=0;i<ngsz;i++)
				*png.wordp(i+1)=*ng.wordp(selmask[i]);
      png.freq=ng.freq;
      //transfer
      ng2.trans(png);      
      ngt2->put(ng2);
      if (!(++c % 1000000)) cerr << ".";
    }
		
    char info[100];
    sprintf(info,"hm%s",hmask);
    ngt2->ngtype(info);
    
    //makes ngt2 aware of oov code
    int oov=ngt2->dict->getcode(ngt2->dict->OOV());
    if(oov>=0) ngt2->dict->oovcode(oov);
		
    for (int i=0;i<ngt->dict->size();i++){
      ngt2->dict->incfreq(ngt2->dict->encode(ngt->dict->decode(i)),
													ngt->dict->freq(i));
    }
    
    cerr <<" oov: " << ngt2->dict->freq(ngt2->dict->oovcode()) << "\n";
		
    delete ngt;
    ngt=ngt2;
  }
	
	
  if (tlm && table_type==LEAFPROB){
		ngram ng(ngt->dict);
		cout.setf(ios::scientific);
    
    cout << "> ";
    while(cin >> ng){
      ngt->bo_state(0);
      if (ng.size>=ngsz){
				cout << ng << " p= " << log(ngt->prob(ng));
				cout << " bo= " << ngt->bo_state() << "\n";
      }
      else
				cout << ng << " p= NULL\n";
      
      cout << "> ";
    }
    
  }
	
	
  if (ftlm && table_type==LEAFPROB){
    
    ngram ng(ngt->dict);
    cout.setf(ios::fixed);
    cout.precision(2);
    
    mfstream inptxt(ftlm,ios::in);
    int Nbo=0,Nw=0,Noov=0;
    float logPr=0,PP=0,PPwp=0;
    
    int bos=ng.dict->encode(ng.dict->BoS());
		
    while(inptxt >> ng){
			
      // reset ngram at begin of sentence
      if (*ng.wordp(1)==bos){
				ng.size=1;
				continue;
      }
			
      ngt->bo_state(0);
      if (ng.size>=1){
				logPr+=log(ngt->prob(ng));
				if (*ng.wordp(1) == ngt->dict->oovcode())
					Noov++;
				
				Nw++; if (ngt->bo_state()) Nbo++;
      }	
    }
    
    PP=exp(-logPr/Nw);
    PPwp= PP * exp(Noov * log(10000000.0-ngt->dict->size())/Nw);
		
    cout << "%%% NGT TEST OF SMT LM\n";
    cout << "%% LM=" << inp << " SIZE="<< ngt->maxlevel();
    cout << "   TestFile="<< ftlm << "\n";
    cout << "%% OOV PENALTY = 1/" << 10000000.0-ngt->dict->size() << "\n";
    
    
    cout << "%% Nw=" << Nw << " PP=" << PP << " PPwp=" << PPwp
			<< " Nbo=" << Nbo << " Noov=" << Noov 
			<< " OOV=" << (float)Noov/Nw * 100.0 << "%\n";
    
  }
	
	
  if (memuse)  ngt->stat(0);
	
  if (out)
		bin?ngt->savebin(out,ngsz): ngt->savetxt(out,ngsz,outputgoogleformat);
	
	
}

