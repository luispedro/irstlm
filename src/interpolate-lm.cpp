// $Id: compile-lm.cpp 252 2009-05-08 08:58:20Z mfederico $

/******************************************************************************
 IrstLM: IRST Language Model Toolkit, compile LM
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

using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include "util.h"
#include "math.h"
#include "lmtable.h"


/* GLOBAL OPTIONS ***************/

std::string slearn = "";
std::string seval = "";
std::string sscore = "no";
std::string sdebug = "0";
std::string smemmap = "0";
std::string sdub = "0";
/********************************/

void usage(const char *msg = 0) {
  if (msg) { std::cerr << msg << std::endl; }
  std::cerr << "Usage: interpolate-lm [options] lm-list-file [lm-list-file.out]" << std::endl;
  if (!msg) std::cerr << std::endl
            << "  interpolate-lm reads a list of LM files with interpolation weights " << std::endl
            << "  with the format: N\\n w1 lm1 \\n w2 lm2 ...\\n wN lmN\n" << std::endl
            << "  and performs training of weights and computation of probabilities." << std::endl
			<< "  So far it manages LM in ARPA format and IRSTLM binary format." << std::endl  << std::endl;
			
  std::cerr << "Options:\n"
            << "--learn text-file -l=text-file (learns new weights and creates a new lm-list-file)"<< std::endl
            << "--eval text-file -e=text-file (computes perplexity of the interpolated LM on text-file)"<< std::endl
            << "--dub dict-size (dictionary upperbound to compute OOV word penalty: default 0)"<< std::endl
            << "--score [yes|no] -s=[yes|no] (computes log-prob scores with the interpolated LM)"<< std::endl
            << "--debug 1 -d 1 (verbose output for --eval option)"<< std::endl
            << "--memmap 1 --mm 1 (uses memory map to read a binary LM)\n" ;
}

bool starts_with(const std::string &s, const std::string &pre) {
  if (pre.size() > s.size()) return false;

  if (pre == s) return true;
  std::string pre_equals(pre+'=');
  if (pre_equals.size() > s.size()) return false;
  return (s.substr(0,pre_equals.size()) == pre_equals);
}

std::string get_param(const std::string& opt, int argc, const char **argv, int& argi)
{
  std::string::size_type equals = opt.find_first_of('=');
  if (equals != std::string::npos && equals < opt.size()-1) {
    return opt.substr(equals+1);
  }
  std::string nexto;
  if (argi + 1 < argc) { 
    nexto = argv[++argi]; 
  } else {
    usage((opt + " requires a value!").c_str());
    exit(1);
  }
  return nexto;
}

void handle_option(const std::string& opt, int argc, const char **argv, int& argi)
{
  if (opt == "--help" || opt == "-h") { usage(); exit(1); }
  
  if (starts_with(opt, "--learn") || starts_with(opt, "-l"))
    slearn = get_param(opt, argc, argv, argi);
  else
    if (starts_with(opt, "--eval") || starts_with(opt, "-e"))
      seval = get_param(opt, argc, argv, argi);
  else
    if (starts_with(opt, "--score") || starts_with(opt, "-s"))
      sscore = get_param(opt, argc, argv, argi);  
  else
    if (starts_with(opt, "--debug") || starts_with(opt, "-d"))
      sdebug = get_param(opt, argc, argv, argi);
  
  else
    if (starts_with(opt, "--memmap") || starts_with(opt, "-mm"))
      smemmap = get_param(opt, argc, argv, argi);     
  
  else
    if (starts_with(opt, "--dub") || starts_with(opt, "-dub"))
      sdub = get_param(opt, argc, argv, argi);     
  
  else {
    usage(("Don't understand option " + opt).c_str());
    exit(1);
  }
}

int main(int argc, const char **argv)
{
  
  if (argc < 2) { usage(); exit(1); }
  std::vector<std::string> files;
  for (int i=1; i < argc; i++) {
    std::string opt = argv[i];
    if (opt[0] == '-') { handle_option(opt, argc, argv, i); }
      else files.push_back(opt);
  }
  
  if (files.size() > 2) { usage("Too many arguments"); exit(1); }
  if (files.size() < 1) { usage("Please specify a LM list file to read from"); exit(1); }

  bool learn = (slearn == "yes"? true : false);
  
  //int debug = atoi(sdebug.c_str()); 
  //int memmap = atoi(smemmap.c_str());
  //int dub = atoi(sdub.c_str());
    
  std::string infile = files[0];
  std::string outfile="";
  
  if (files.size() == 1) {  
    outfile=infile;    
    //remove path information
    std::string::size_type p = outfile.rfind('/');
    if (p != std::string::npos && ((p+1) < outfile.size()))           
      outfile.erase(0,p+1);
      outfile+=".out";
  }
  else
     outfile = files[1];
 
  std::cerr << "inpfile: " << infile << std::endl;
  if (learn) std::cerr << "outfile: " << outfile << std::endl;
  std::cerr << "interactive: " << sscore << std::endl;
  
  lmtable **lmt; //interpolated language models
  std::string *lmf; //lm filenames
  float *w; //interpolation weights
  int N;
  
  
  std::cerr << "Reading " << infile << "..." << std::endl;
  
  std::fstream inptxt(infile.c_str(),std::ios::in);

  inptxt >> N;
  std::cerr << "Number of LMs: " << N << "..." << std::endl;
   
  lmt=new lmtable*[N]; //interpolated language models
  lmf=new std::string[N]; //lm filenames
    w=new float[N]; //interpolation weights
  
  for (int i=0;i<N;i++) inptxt >> w[i] >> lmf[i];
  inptxt.close();



  

  
  return 0;
}

