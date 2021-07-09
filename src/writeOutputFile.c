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

void writeCompactedArrayBinary(void){
  FILE *fpBin;
  size_t numWritten;
  size_t numBytes;

  fprintf(fp, "!binary %c%s%c\n", '"', clFileOutBinary, '"');

  fpBin = fopen(clFileOutBinary, "wb");
  if (fpBin == NULL){
    fprintf (stderr, "Can not open binary output file %s.\n", clFileOutBinary);
    exit(0);
  }

  if(compactedDataSize>1){
    numBytes=(size_t)compactedDataSize;
    numWritten = fwrite((void *)compactedData, 1, numBytes,fpBin);
    if(numWritten!=numBytes){
      fprintf (stderr, "Write data to binary output file %s failed. %u of %u data written\n", clFileOutBinary, (unsigned int)numWritten, (unsigned int)numBytes);
      exit(0);
    }
  }
  
  if(fclose(fpBin)>0){
    fprintf (stderr, "Can not close binary output file %s.\n", clFileOutBinary);
    exit(0);
  }
}

void writeCompactedArrayAscii(void){
  tRoadcUInt32 i;

  fprintf(fp, "!byte ");

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
}

void writeCompactedArray(void){
  fprintf(fp, "%s\n", clCompressedDataName);
  if(clCompressedDataFormat==CL_FORMAT_BIN){
    writeCompactedArrayBinary();
  } else {
    writeCompactedArrayAscii();
  }
}

void writeDataAcme(void){
  tRoadcUInt32 numArrays;
  tRoadcUInt32 pos;
  tRoadcUInt32 i;

  if(clVerbose){
    printf("Write compacted array...");
  }
  writeCompactedArray();
  if(clVerbose){
    printf("done.\n");
  }

  numArrays = getInputNumArrays();
  for(i=0; i<numArrays; i++){
    if(clVerbose){
      printf("\rFind and write input array position in compacted array: %lu/%lu.",i+1, numArrays);
      fflush(stdout);
    }

    pos = roadcGetPositionInCompactedData(pRoadc, 
					  getInputArray(i), 
					  getInputPaddingByteMaskArray(i),
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
  if(clVerbose){
    printf("\n");
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

