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
#include "readInputFile.h"

static FILE *fp;
/* 10 MB with each byte build with 5 chars, e.g. "255, " */
#define MAX_INPUT_LINE_SIZE 1024+1024*10*5
static char *inputPtr = NULL;
static ssize_t maxLineLength;
static unsigned int endOfFile=0;
static unsigned int endOfLine=0;
static unsigned int line=0;
static unsigned int column=0;

void readNextLine(void){
  char *result;
  if(inputPtr==(char*)NULL){
    inputPtr = (char *)malloc(MAX_INPUT_LINE_SIZE*sizeof(char));
  }
  column=0;
  /* getline does not work with MinGW on Windows, exits at EOF 
     so use fgets instead
    size_t len=0;
    maxLineLength = getline(&inputPtr, &len, fp);
    if(maxLineLength == -1){
    endOfFile=1;
    endOfLine=1;
    }
  */
  result = fgets(inputPtr, MAX_INPUT_LINE_SIZE-1, fp);
  if(result==(char*)NULL){
    endOfFile=1;
    endOfLine=1;
  }
  maxLineLength = strlen(inputPtr);
  if(maxLineLength==0){
    endOfLine=1;
  }
}

void openInputFile(void){
  fp = fopen(clFileIn, "r");
  if (fp == NULL){
    fprintf (stderr, "Can not open input file %s.\n", clFileIn);
    exit(0);
  }
  readNextLine();
}

void closeInputFile(void){
  if(fclose(fp)>0){
    fprintf (stderr, "Can not close input file %s.\n", clFileIn);
    exit(0);
  }
  if(inputPtr!=(char*)NULL){
    free((void *)inputPtr);
  }
}  

void incColumn(unsigned int number){
  column = column + number;
  if (column >= (unsigned int) maxLineLength){
    column = maxLineLength - 1;
    endOfLine=1;
    return;
  }
}

void decColumn(unsigned int number){
  if (number>column){
    column = 0;
  } else {
    column = column - number;
  }
}

void incLine(void){
  readNextLine();
  if(endOfFile==0){
    line++;
  }
}

void incToPtr(char *newPtr){
  while ((&inputPtr[column])!=newPtr){
    column++;
    if (column > (unsigned int) maxLineLength){
      fprintf (stderr, "Can not inc to given pointer, end of input line reached.\n");
      exit(0);
    }
    if (column == (unsigned int) maxLineLength){
      column--; /* bring index back in range again */
      endOfLine=1;
    }
  }
}
  
unsigned int getCurrentLine(void){
  return line+1;
}

unsigned int getCurrentColumn(void){
  return column+1;
}

unsigned int isEndOfLine(void){
  return endOfLine;
}
unsigned int isEndOfFile(void){
  return endOfFile;
}

char getChar(void){
  return inputPtr[column];
}
char *getString(void){
  return &inputPtr[column];
}

/* consumeFlag: 0: do not change file input string
                1: consume string from file input string
   return: 0: string not empty
           1: string is empty 
*/
int extractString(char *to, unsigned int consumeFlag){
  tRoadcUInt32 i=0;
  int extracted=0;
  int isEmpty=0;
  char c;
  
  while((extracted==0) &&(isEndOfLine()==0)){
    c = getChar();
    if((c=='\n') ||
       (c=='\r') ||
       (c==' ') ||
       (c=='\0')){
      extracted=1;
    } else {
      to[i]=c;
      i++;
      incColumn(1);
      isEmpty=1;
    }
  }
  if(consumeFlag==0){
    decColumn(i);
  }
  to[i]='\0';
  return isEmpty;
}

/* consumeFlag: 0: do not change file input string
                1: consume string from file input string
   return: 0: string not empty
           1: string is empty 
   Note:
   encoding for bytes can be:
   decimal     (e.g. 14)
   hexadecimal (e.g. $3d or 0x02)
   binary      (e.g. %0110 or %10001000)
   octal       (e.g. &304)
   char        (e.g. "c" or ';')

*/
int extractNextNumberAsString(char *to, unsigned int consumeFlag){
  tRoadcUInt32 stringIndex=0;
  tRoadcUInt32 consumedBytes=0;
  int extracted=0;
  int isEmpty=1;
  int isDone=0;
  char c;

  /* read over seperators */
  while((isDone==0) && (isEndOfLine()==0)){
    c = getChar();
    if((c==' ') ||
       (c==',') ||
       (c==';') ||
       (c==':') ||
       (c=='#')){
      incColumn(1);
      consumedBytes++;
    } else {
      isDone = 1;
    }
  }

  /* read number */
  while((extracted==0) && (isEndOfLine()==0)){
    c = getChar();
    if(((int)c==39) || /* sign ' */
       (c=='"')){
      /* is a single char, e.g. 'A' or "!" */
      isEmpty=0;
      extracted=1;
      to[stringIndex]=c;
      stringIndex++;
      incColumn(1);
      consumedBytes++;
      if(isEndOfLine()==0){
	c = getChar();
	to[stringIndex]=c;
	stringIndex++;
	incColumn(1);
	consumedBytes++;
	if(isEndOfLine()==0){
	  c = getChar();
	  to[stringIndex]=c;
	  stringIndex++;
	  incColumn(1);
	  consumedBytes++;
	}
      }
    } else {
      if((c=='\n') ||
	 (c=='\r') ||
	 (c=='\0') ||
	 (c==' ')  ||
	 (c==',')  ||
	 (c==';')  ||
	 (c==':')  ||
	 (c=='#')){
	extracted=1;
      } else {
	to[stringIndex]=c;
	stringIndex++;
	incColumn(1);
	consumedBytes++;
	isEmpty=0;
      }
    }
  }
  if(consumeFlag==0){
    decColumn(consumedBytes);
  }
  to[stringIndex]='\0';
	
  return isEmpty;
}


/* empty or comment lines are skipped 
   return:
   0: do not skip line
   1: skip line
*/
int skipLine(void){
  char c;
  if(isEndOfLine()==1){
    return 1;
  }
  while(getChar()==' '){
    incColumn(1);
    if(isEndOfLine()==1){
      return 1;
    }
  }
  c = getChar();
  if((c=='\n') || /* line feed and carriage return */
     (c=='\r') || /* carriage return */
     (c=='\0') || /* end of string */
     (c=='/')  || /* comment token */
     (c=='#')  || /* comment token */
     (c==':')  || /* comment token */
     (c==';')){   /* comment token */
    return 1;
  }
  return 0;
}


