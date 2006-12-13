#include <iostream>
#include <fstream>
#include <streambuf>
#include <cstdio>
#include "mfstream.h"

using namespace std;

void mfstream::open(const char *name,openmode mode){
  
  char cmode[10];

  if (strchr(name,' ')!=0){ 
    if (mode & ios::in)
      strcpy(cmode,"r");
    else
      if (mode & ios::out) 
	strcpy(cmode,"w");
      else
	if (mode & ios::app) 
	  strcpy(cmode,"a");    
	else{
	  cerr << "cannot open file\n";
	  exit(1);
	}
    _cmd=1;
    strcpy(_cmdname,name);
    _FILE=popen(name,cmode);
    buf=new fdbuf(fileno(_FILE));
    iostream::rdbuf((streambuf*) buf);
  }
  else{
    _cmd=0;
    fstream::open(name,mode);
  }
  
}


void mfstream::close(){
  if (_cmd==1){
    pclose(_FILE);
    delete buf;
  }
  else {
    fstream::clear();
    fstream::close();
  }
  _cmd=2;
}



int mfstream::swapbytes(char *p, int sz, int n)
{
  char    c,
    *l,
    *h;
  
  if((n<1) ||(sz<2)) return 0;
  for(; n--; p+=sz) for(h=(l=p)+sz; --h>l; l++) { c=*h; *h=*l; *l=c; }
  return 0;

};


mfstream& mfstream::iwritex(streampos loc,void *ptr,int size,int n)
{
  streampos pos=tellp();
   
  seekp(loc);
   
  writex(ptr,size,n);
  
  seekp(pos);
  
  return *this;
   
}


mfstream& mfstream::readx(void *p, int sz,int n)
{
  if(!read((char *)p, sz * n)) return *this;
  
  if(*(short *)"AB"==0x4241){
    swapbytes((char*)p, sz,n);
  }

  return *this;
}

mfstream& mfstream::writex(void *p, int sz,int n)
{
  if(*(short *)"AB"==0x4241){
    swapbytes((char*)p, sz,n);
  }
  
  write((char *)p, sz * n);
  
  if(*(short *)"AB"==0x4241) swapbytes((char*)p, sz,n);
  
  return *this;
}





/*
int main()
{
  
  char word[1000];

  mfstream inp("cat pp",ios::in); 
  mfbstream outp("aa",ios::out,100);

  while (inp >> word){
    outp << word << "\n"; 
    cout << word << "\n";
  }

  
}

*/
