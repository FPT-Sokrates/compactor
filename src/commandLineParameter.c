/*
The MIT License

Copyright (c) 2021 Steffen Görzig

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* SPDX-License-Identifier: MIT */

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif /* __cplusplus */

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "commandLineParameter.h"
#include "printHelpText.h"

char *clCompressedDataName  = (char *)"compactedData";
int clCompressedDataFormat  = CL_FORMAT_INT;
int clCompilerType  = CL_COMPILER_ACME;
char *clArraySizePrefix    = (char *)"";
char *clFileIn = (char *)"";
char *clFileOut = (char *)"compactedData.txt";
char *clFileOutBinary = (char *)"compactedData.bin";
int clFileOutBinaryDefined = 0;
tRoadcUInt32 clTimeout=0;
int clVerbose = 0;

/*
n name - compressed data name
f format - output data format
c compiler - compiler type
s size - array size prefix

b binary outfile
o outfile
t timeout 
v verbose 
h help 
*/

static const struct option clOptions[] = {
  { "binary",   required_argument, 0, 'b' },
  { "compiler", required_argument, 0, 'c' },
  { "format",   required_argument, 0, 'f' },
  { "help",     no_argument,       0, 'h' },
  { "name",     required_argument, 0, 'n' },
  { "outfile",  required_argument, 0, 'o' },
  { "size",     required_argument, 0, 's' },
  { "timeout",  required_argument, 0, 't' },
  { "verbose",  no_argument,       0, 'v' },
  {0, 0, 0, 0}
};

void setCompilerType(char *value){
  if((strncmp((const char *)value, "acme\n", 4)==0) ||
     (strncmp((const char *)value, "ACME\n", 4)==0)){
    clCompilerType  = CL_COMPILER_ACME;
  }
  else {
    fprintf (stderr, "Invalid argument %s for option --compiler.\n", value);
    exit(0);
  }
}

void
commandLineParameter (int argc, char **argv)
{
  int index;
  int val;
  int c;
  int printHelpFlag=0;

  opterr = 0;


  while ((c = getopt_long(argc, argv, "b:c:f:hn:o:s:t:v", clOptions, &index)) != -1){
    switch (c){
      case 'b':
        clFileOutBinary = optarg;
	clFileOutBinaryDefined = 1;
        break;
      case 'c':
	setCompilerType(optarg);
        break;
      case 'f':
	if(strncmp((const char *)optarg, "hex\n", 3)==0){
	  clCompressedDataFormat = CL_FORMAT_HEX;
	} else { 
	  if(strncmp((const char *)optarg, "oct\n", 3)==0){
	    clCompressedDataFormat = CL_FORMAT_OCT;
	  } else { 
	    if(strncmp((const char *)optarg, "int\n", 3)==0){
	      clCompressedDataFormat = CL_FORMAT_INT;
	    } else {
	      if(strncmp((const char *)optarg, "bin\n", 3)==0){
		clCompressedDataFormat = CL_FORMAT_BIN;
	      } else {
		fprintf (stderr, "Invalid argument %s for option --compressedDataFormat.\n", optarg);
		exit(0);
	      }
	    }
	  }
	}
        break;
      case 'h':
	printHelpFlag=1;
	/* do not call printHelpText() here
	   to ensure handling of parameter verbose needed for printing */
        break;
      case 'n':
        clCompressedDataName = optarg;
        break;
      case 'o':
        clFileOut = optarg;
        break;
      case 's':
        clArraySizePrefix = optarg;
        break;
      case 't':
        val = atoi(optarg);
	if(val<0){
	  fprintf (stderr, "Invalid value %s for option --timeout.\n", optarg);
	  exit(0);
	} 
        clTimeout = (tRoadcUInt32) val;
        break;
      case 'v':
	clVerbose = 1;
        break;
      case '?':
        if ((optopt == 'n') || 
	    (optopt == 'f') || 
	    (optopt == 'c') || 
	    (optopt == 's') || 
	    (optopt == 'o') || 
	    (optopt == 't')){
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	}
	else{
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	  fprintf (stderr, "Type 'compactor -h' for help.\n");
	}
	exit(0);
      default:
        break;
    }
  }

  
  if(printHelpFlag==1){
    printHelpText();
    exit(0);
  }
  
  /* process remaining command line arguments (not options). */
  if (optind == argc){
    fprintf (stderr, "No input file.\n");
    fprintf (stderr, "Type 'compactor -h' for help.\n");
    exit(0);
  }
  if (optind+1 == argc){
      clFileIn = argv[optind];
  } else if (optind+1 < argc) {
    fprintf (stderr, "Too many arguments:\n");
    while (optind < argc){
      fprintf (stderr, "%s ", argv[optind++]);
    }
    fprintf (stderr, "\n");
    exit(0);
  }

  /* check binary output consistency */
  if(clVerbose == 1){
    if((clFileOutBinaryDefined == 1) &&
       (clCompressedDataFormat != CL_FORMAT_BIN)){
      printf("WARNING: parameter mismatch - no binary output selected but binary output file set to %s.\n", clFileOutBinary);

    }
  }

}
