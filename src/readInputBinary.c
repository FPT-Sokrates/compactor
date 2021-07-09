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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "readInputBinary.h"
#include "readInputFile.h"
#include "stringHandling.h"

static char fileName[1000];
static FILE *fpBin=NULL;

static char tmpString[100];
static int currentNumberBytes=0;
static int numberBytes=-1;
static int skipBytes=0;

static int noMoreValues;
static int fillingActive;
static int fillingWarningPrinted;


/*
  return: 1: is a binary array
          0: no binary array
*/
unsigned int checkAndConsumeBinaryArray(unsigned int consumeFlag){
  if(extractString(tmpString, consumeFlag)==0){
    return 0;
  }

  /* check for binary keywords: "!binary", "!bin" */
  if(strncmp(tmpString, "!binary", 7)==0){
    return 1;
  }
  if(strncmp(tmpString, "!bin", 4)==0){
    return 1;
  }
  return 0;
}

/*
  return: 1: is a binary array
          0: no binary array
*/
unsigned int isBinaryArray(void){
  return checkAndConsumeBinaryArray(0);
}

void setValueNumberBytes(void){
  int result;
  result = extractNextNumberAsString(tmpString, 1);
  if(result==0){
    /* number given, transform it */
    result = convertPositiveDecimal(tmpString);
    if(result<=0){
      fprintf (stderr, "Line: %d: wrong value for numberBytes (<=0).\n", getCurrentLine());
      exit(0);
    }
    /* valid value given */
    numberBytes = result;
  }
}

void setValueSkipBytes(void){
  int result;
  result = extractNextNumberAsString(tmpString, 1);
  if(result==0){
    /* number given, transform it */
    result = convertPositiveDecimal(tmpString);
    if(result<0){
      fprintf (stderr, "Line: %d: wrong value for skipBytes (<0).\n", getCurrentLine());
      exit(0);
    }
    /* valid value given */
    skipBytes = result;
  }
}

void consumeKeywordBinaryAndInit(void){
  int result;
  long fileSize;
  
  checkAndConsumeBinaryArray(1);
  result = extractFilename(fileName, 1);
  if(result==1){
    fprintf (stderr, "Line: %d: Column: %d no valid filename given.\n", getCurrentLine(), getCurrentColumn());
    exit(0);
  }
  numberBytes=-1;
  currentNumberBytes=0;
  skipBytes=0;
  noMoreValues=0;
  fillingActive=0;
  fillingWarningPrinted=0;
  
  if(nextIsComma(1)==1){
    if(nextIsComma(0)==1){
      /* comma directly after comma == numberBytes=-1 */
      setValueSkipBytes();      
    } else {
      setValueNumberBytes();      
      setValueSkipBytes();      
    }
  }
  fpBin = fopen(fileName, "rb");
  if (fpBin == NULL){
    fprintf (stderr, "Line: %d: Can not open binary output file %s.\n", getCurrentLine(), fileName);
    exit(0);
  }
  /* get file size */
  result = fseek(fpBin, (long)0, SEEK_END);
  fileSize = ftell(fpBin);
  result = fseek(fpBin, (long)0, SEEK_SET);
  if(fileSize==0){
    if(numberBytes==-1){
      printf ("WARNING: Line: %d: file is empty: no data to read from file %s.\n", getCurrentLine(), fileName);
    }
  }
  if(skipBytes>0){
    /* skip given number of bytes from start of file stream == SEEK_SET */
    result = fseek(fpBin, (long)skipBytes, SEEK_SET);
    if(skipBytes>=fileSize){
      if(numberBytes==-1){
	printf ("WARNING: Line: %d: skipBytes size >= file size: no data to read from file %s.\n", getCurrentLine(), fileName);
	fillingActive=1;
	fillingWarningPrinted=1;
      }
      if(numberBytes>0){
	printf("WARNING: Line: %d: skipBytes size >= file size, but numberBytes set to a value >0: no data to read from file %s, filling up with value %d. Maybe this was not intended.\n", getCurrentLine(), fileName, (int)RIB_FILL_VALUE);
	fillingActive=1;
      fillingWarningPrinted=1;
      }
    }
  }
}

void binaryClose(void){
  if(fclose(fpBin)>0){
    fprintf (stderr, "Can not close binary input file %s.\n", fileName);
    exit(0);
  }
}


/* 
   return:   -1: no next value
           else: binary value
*/
int getNextValueBinary(void){
  size_t result;
  unsigned char value;

  if(noMoreValues==1){
    /* no more bytes */
    return -1;
  }
  currentNumberBytes++;
  if((numberBytes!=-1) && (currentNumberBytes>numberBytes)){
    /* no more bytes */
    noMoreValues=1;
    return -1;
  }
  if((numberBytes!=-1) && (fillingActive==1)){
    /* fillingActive can only be 1 for numberBytes > 0 */
    return (int) RIB_FILL_VALUE;
  }
  result = fread((void *)&value, 1, 1, fpBin);
  if(result!=1){
    /* read error or EOF */
    if(numberBytes==-1){
      /* read until EOF, now indicate EOF */
      noMoreValues=1;
      return -1;
    }
    fillingActive=1;
    if(fillingWarningPrinted==0){
      printf("WARNING: Line: %d: more bytes requested by numberBytes then bytes in file %s (taking also skipBytes into account), filling up with value %d. Maybe this was not intended.\n", getCurrentLine(), fileName, (int)RIB_FILL_VALUE);
      fillingWarningPrinted=1;
    }
    return (int) RIB_FILL_VALUE;
  } else {
    return (int) value;
  }

  return -1;
}

