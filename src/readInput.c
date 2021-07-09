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
#include <stdio.h>
#include <stdlib.h>
#include "commandLineParameter.h"
#include "readInput.h"
#include "readInputFile.h"
#include "readInputByte.h"
#include "readInputBinary.h"

#define RI_FORMAT_NONE 0 
#define RI_FORMAT_BYTE 1 
#define RI_FORMAT_BINARY 2

#define RI_PADDING_FILL_VALUE 0


#define MALLOC_NUM 65536
tRoadcBytePtr *inputArrays=(tRoadcBytePtr *)NULL;
tRoadcBytePtr *inputPaddingByteMaskArrays=(tRoadcBytePtr *)NULL;
char **inputArraysAddressName=(char **)NULL;
tRoadcUInt32 *inputArraysSize=(tRoadcUInt32 *)NULL;
tRoadcUInt32 *inputArraysAlignment=(tRoadcUInt32 *)NULL;
tRoadcUInt32 numInputArrays=0;
tRoadcUInt32 numInputBytes=0;
static tRoadcUInt32 maxNumInputArrays=0;

static tRoadcByte *inputLineArray=(tRoadcByte *)NULL;
static tRoadcUInt32 numInputLineArray=0;
static tRoadcUInt32 maxNumInputLineArray=0;

static char tmpString[100];

void addInputArraysAddressName(char *val, int index){
  size_t entrySize;
  entrySize = strlen(val) + 1;

  inputArraysAddressName[index]=(char *)malloc(entrySize*sizeof(char));
  if(inputArraysAddressName[index]==(char *)NULL){
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }
  strcpy(inputArraysAddressName[index], val);
  inputArraysAddressName[index][entrySize-1]='\0'; /* terminate string in any case */
}

void addInputArraysAlignment(tRoadcUInt32 val, int index){

  inputArraysAlignment[index]=val;
}

void resetProcessNextLine(void){
  numInputLineArray=0;
}

int determineInputFormatType(void){
  int inputFormatType;
  inputFormatType = RI_FORMAT_NONE;
  if(isByteArray()==1){
    inputFormatType = RI_FORMAT_BYTE;
  } else {
    if(isBinaryArray()==1){
      inputFormatType = RI_FORMAT_BINARY;
    }
  }
  return inputFormatType;
}

void setArrayNameAndSize(int index){
  int inputFormatType;
  int tmpVal;

  addInputArraysAlignment(1, index); /* default value */

  inputFormatType = determineInputFormatType();
  if(inputFormatType==RI_FORMAT_BYTE){
    fprintf (stderr, "Line: %d: Array name expected, but byte array values given. Set array name first.\n", getCurrentLine());
    exit(0);
  }
  if(inputFormatType==RI_FORMAT_BINARY){
    fprintf (stderr, "Line: %d: Array name expected, but binary array values given. Set array name first.\n", getCurrentLine());
    exit(0);
  }

  if(extractString(tmpString, 1)==0){
    fprintf (stderr, "Line: %d: Column: %d no array name given\n", getCurrentLine(), getCurrentColumn());
    exit(0);
  }
  addInputArraysAddressName(tmpString, index);

  while((isEndOfLine()==0) && (getChar()==' ')){
    incColumn(1);
  }

  tmpVal = getNextValueByte();
  if(tmpVal!=-1){
    /* there is a number */
    addInputArraysAlignment((tRoadcUInt32)tmpVal, index);
    return;
  }
}

void allocInputArrays(){
  maxNumInputArrays=MALLOC_NUM;
  inputArraysSize = (tRoadcUInt32 *)malloc(MALLOC_NUM*sizeof(tRoadcUInt32)); 
  if(inputArraysSize==(tRoadcUInt32 *)NULL){                                                
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }
  inputArraysAlignment = (tRoadcUInt32 *)malloc(MALLOC_NUM*sizeof(tRoadcUInt32)); 
  if(inputArraysAlignment==(tRoadcUInt32 *)NULL){                                                
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }
  inputArrays=(tRoadcBytePtr *)malloc(MALLOC_NUM*sizeof(tRoadcBytePtr));
  if(inputArrays==(tRoadcBytePtr *)NULL){
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }
  inputPaddingByteMaskArrays=(tRoadcBytePtr *)malloc(MALLOC_NUM*sizeof(tRoadcBytePtr));
  if(inputPaddingByteMaskArrays==(tRoadcBytePtr *)NULL){
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }
  inputArraysAddressName=(char **)malloc(MALLOC_NUM*sizeof(char *));
  if(inputArraysAddressName==(char **)NULL){
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }
  maxNumInputLineArray=MALLOC_NUM;
  inputLineArray=(tRoadcByte *)malloc(MALLOC_NUM*sizeof(tRoadcByte));
  if(inputLineArray==(tRoadcByte *)NULL){
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }
}                              

void reallocInputLineArray(){
  if(numInputLineArray==maxNumInputLineArray){
    maxNumInputLineArray=maxNumInputLineArray+MALLOC_NUM;
    inputLineArray = (tRoadcByte *)realloc((void*)inputLineArray, maxNumInputLineArray*sizeof(tRoadcByte)); 
    if(inputLineArray==(tRoadcByte *)NULL){                                                
      fprintf (stderr, "Can not allocate new array for input data\n");
      exit(0);
    }
  }
}

void reallocInputArrays(){
  if(numInputArrays==maxNumInputArrays){
    maxNumInputArrays=maxNumInputArrays+MALLOC_NUM;
    inputArraysSize = (tRoadcUInt32 *)realloc((void*)inputArraysSize, maxNumInputArrays*sizeof(tRoadcUInt32)); 
    if(inputArraysSize==(tRoadcUInt32 *)NULL){                                                
      fprintf (stderr, "Can not allocate new array for input data\n");
      exit(0);
    }
    inputArraysAlignment = (tRoadcUInt32 *)realloc((void*)inputArraysAlignment, maxNumInputArrays*sizeof(tRoadcUInt32)); 
    if(inputArraysAlignment==(tRoadcUInt32 *)NULL){                                                
      fprintf (stderr, "Can not allocate new array for input data\n");
      exit(0);
    }
    inputArrays=(tRoadcBytePtr *)realloc((void*)inputArrays, maxNumInputArrays*sizeof(tRoadcBytePtr));
    if(inputArrays==(tRoadcBytePtr *)NULL){
      fprintf (stderr, "Can not allocate new array for input data\n");
      exit(0);
    }
    inputPaddingByteMaskArrays=(tRoadcBytePtr *)realloc((void*)inputPaddingByteMaskArrays, maxNumInputArrays*sizeof(tRoadcBytePtr));
    if(inputPaddingByteMaskArrays==(tRoadcBytePtr *)NULL){
      fprintf (stderr, "Can not allocate new array for input data\n");
      exit(0);
    }
    inputArraysAddressName=(char **)realloc((void*)inputArraysAddressName, maxNumInputArrays*sizeof(char *));
    if(inputArraysAddressName==(char **)NULL){
      fprintf (stderr, "Can not allocate new array for input data\n");
      exit(0);
    }
  }
}

void readInputFreeArrays(void){
  tRoadcUInt32 i;
  for(i=0;i<numInputArrays;i++){
    free((void *)inputArrays[i]);
  }
  free((void *)inputArrays);
  for(i=0;i<numInputArrays;i++){
    if(inputPaddingByteMaskArrays[i]!=NULL){;
      free((void *)inputPaddingByteMaskArrays[i]);
    }
  }
  free((void *)inputPaddingByteMaskArrays);
  free((void *)inputArraysSize);
  free((void *)inputArraysAlignment);
  free((void *)inputLineArray);
  for(i=0;i<numInputArrays;i++){
    free((void *)inputArraysAddressName[i]);
  }
  free((void *)inputArraysAddressName);
}

void addNewArray(){
  tRoadcBytePtr pTmp;
  tRoadcUInt32 i;

  pTmp = (tRoadcBytePtr)malloc(numInputLineArray*sizeof(tRoadcByte)); 
  if(pTmp==(tRoadcBytePtr)NULL){                                                
    fprintf (stderr, "Can not allocate new array for input data\n");
    exit(0);
  }                                                              
  for(i=0; i<numInputLineArray; i++){
    // copy data
    pTmp[i]=inputLineArray[i];
  }
  reallocInputArrays();
  inputArraysSize[numInputArrays]=numInputLineArray;
  inputArrays[numInputArrays]=pTmp;
  numInputArrays++;
  numInputBytes = numInputBytes + numInputLineArray;
}

void readInputAcme(void){
  tRoadcUInt32 i;
  int value;
  int skipLineFlag;
  int inputFormatType;
  int paddingMaskWaringPrinted;

  allocInputArrays();

  while (isEndOfFile() == 0) {
    resetProcessNextLine();
    skipLineFlag = skipLine();
    if (skipLineFlag==0){

      setArrayNameAndSize(numInputArrays);

      incLine();

      inputFormatType = determineInputFormatType();

      if(inputFormatType == RI_FORMAT_NONE){
        extractString(tmpString, 0);
	fprintf (stderr, "Line: %d Column: %d: unknown array type keyword: '%s'.\n", getCurrentLine(), getCurrentColumn(), tmpString);
	exit(0);
      }

      if(inputFormatType == RI_FORMAT_BYTE){
	consumeKeywordByte();
	value = getNextValueByte();
      }
      if(inputFormatType == RI_FORMAT_BINARY){
	consumeKeywordBinaryAndInit();
	value = getNextValueBinary();
      }


      if (value>=0){
	while(value>=0){
	  inputLineArray[numInputLineArray]=(tRoadcByte)value;
	  
	  if(inputFormatType == RI_FORMAT_BYTE){
	    value = getNextValueByte();
	  }
	  if(inputFormatType == RI_FORMAT_BINARY){
	    value = getNextValueBinary();
	  }
	  
	  numInputLineArray++;
	  reallocInputLineArray();
	}
      }
      if(inputFormatType == RI_FORMAT_BINARY){
	binaryClose();
      }
      incLine();
      if(numInputLineArray>0){
	/* check for padding byte mask input */
	inputFormatType = determineInputFormatType();
	if(inputFormatType == RI_FORMAT_NONE){
	  /* no padding byte mask */
	  inputPaddingByteMaskArrays[numInputArrays]=(tRoadcBytePtr)NULL;
	} else {
	  if(inputFormatType == RI_FORMAT_BYTE){
	    consumeKeywordByte();
	  }
	  if(inputFormatType == RI_FORMAT_BINARY){
	    consumeKeywordBinaryAndInit();
	  }
	  inputPaddingByteMaskArrays[numInputArrays] =
	    (tRoadcBytePtr)malloc(numInputLineArray*sizeof(tRoadcByte)); 
	  if(inputPaddingByteMaskArrays[numInputArrays]==(tRoadcBytePtr)NULL){
	    fprintf (stderr, "Can not allocate new array for input data\n");
	    exit(0);
	  }
	  paddingMaskWaringPrinted=0;
	  for(i=0; i<numInputLineArray;i++){
	    if(inputFormatType == RI_FORMAT_BYTE){
	      value = getNextValueByte();
	    }
	    if(inputFormatType == RI_FORMAT_BINARY){
	      value = getNextValueBinary();
	    }
	    if(value<0){
	      value = RI_PADDING_FILL_VALUE;
	      if(paddingMaskWaringPrinted==0){
		printf("WARNING: Line: %d: less bytes for padding byte mask given than array bytes given in line before, filling up with value %d. Maybe this was not intended.\n", getCurrentLine(), (int)RI_PADDING_FILL_VALUE);
		paddingMaskWaringPrinted=1;
	      }
	    }
	    inputPaddingByteMaskArrays[numInputArrays][i]=value;
	  }
	  if(inputFormatType == RI_FORMAT_BYTE){
	    value = getNextValueByte();
	  }
	  if(inputFormatType == RI_FORMAT_BINARY){
	    value = getNextValueBinary();
	  }
	  if(value>=0){
	    printf("WARNING: Line: %d: more bytes for padding byte mask given than array bytes given in line before. Maybe this was not intended.\n", getCurrentLine());

	  }
	  if(inputFormatType == RI_FORMAT_BINARY){
	    binaryClose();
	  }
	}
	addNewArray();
      }
    } else {
      incLine();
    }
  }
}  

void readInput(void)
{
  openInputFile();

  if(clCompilerType==CL_COMPILER_ACME){
    readInputAcme();
  }
  
  closeInputFile();

  if (numInputBytes==0){
    fprintf (stderr, "no input data in input file %s.\n", clFileIn);
    exit(0);
  }
}

tRoadcBytePtr getInputArray(tRoadcUInt32 index){
  if(index>=numInputArrays){
    fprintf (stderr, "index too large %ld.\n", index);
    exit(0);
  }
  return inputArrays[index];
}
tRoadcBytePtr getInputPaddingByteMaskArray(tRoadcUInt32 index){
  if(index>=numInputArrays){
    fprintf (stderr, "index too large %ld.\n", index);
    exit(0);
  }
  return inputPaddingByteMaskArrays[index];
}
char * getInputArrayName(tRoadcUInt32 index){
  if(index>=numInputArrays){
    fprintf (stderr, "index too large %ld.\n", index);
    exit(0);
  }
  return inputArraysAddressName[index];
}
tRoadcUInt32 getInputArraySize(tRoadcUInt32 index){
  if(index>=numInputArrays){
    fprintf (stderr, "index too large %ld.\n", index);
    exit(0);
  }
  return inputArraysSize[index];
}
tRoadcUInt32 getInputArrayAlignment(tRoadcUInt32 index){
  if(index>=numInputArrays){
    fprintf (stderr, "index too large %ld.\n", index);
    exit(0);
  }
  return inputArraysAlignment[index];
}

tRoadcUInt32 getInputNumArrays(void){
  return numInputArrays;
}
tRoadcUInt32 getInputNumBytes(void){
  return numInputBytes;
}
