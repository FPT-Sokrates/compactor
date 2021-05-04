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
#include <string.h>
#include "writeOutputFile.h"
#include "readInput.h"
#include "commandLineParameter.h"
#include "compactor.h"


static tRoadcByte val;
static FILE *fp;

void writeSingleValue(void){
  if(clCompressedDataFormat==CL_FORMAT_HEX){
    fprintf(fp,"$%02x", val);
  } else {
    if(clCompressedDataFormat==CL_FORMAT_OCT){
      fprintf(fp,"&%03o", val);
    } else {
      fprintf(fp,"%d", val);
    }
  }
}

void writeDataAcme(void){
  tRoadcUInt32 i;
  tRoadcUInt32 pos;

  fprintf(fp, "%s\n", clCompressedDataName);
  fprintf(fp, "%s", "!byte ");

  val = compactedData[0];
  writeSingleValue();
  if(compactedDataSize>1){
    for (i=1; i<compactedDataSize; i++){
      fprintf(fp,", ");
      val = compactedData[i];
      writeSingleValue();
    }
  }
  fprintf(fp,"\n");

  for(i=0; i<getInputNumArrays(); i++){
    pos = roadcGetPositionInCompactedData(pRoadc, 
					  getInputArray(i), 
					  NULL, 
					  getInputArraySize(i), 
					  1);
    if(pos==roadcGetCompactedDataSize(pRoadc)){
      fprintf (stderr, "input array not found in compacted data - this should not happen...\n");
      exit(0);
    }

    fprintf(fp, "%s = %s + %lu\n", 
	    getInputArrayName(i),
	    clCompressedDataName, 
	    (unsigned long)pos);
    if(strcmp((const char *)clArraySizePrefix, "")!=0){
      fprintf(fp, "%s%s = %lu\n",
	      getInputArrayName(i),
	      clArraySizePrefix,
	      getInputArraySize(i));
    }
	    
  }
}

void writeOutputFile(void){
  if (compactedDataSize<=0){
    fprintf (stderr, "no compacted data given\n");
    exit(0);
  }

  fp = fopen(clFileOut, "w");
  if (fp == NULL){
    fprintf (stderr, "Can not open output file %s.\n", clFileOut);
    exit(0);
  }

  if(clCompilerType==CL_COMPILER_ACME){
    writeDataAcme();
  }

  if(fclose(fp)>0){
    fprintf (stderr, "Can not close output file %s.\n", clFileOut);
    exit(0);
  }
}

