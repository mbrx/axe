/** \file ent.c
*/
/*
   Copyright (C) 2013  Mathias Broxvall

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
    

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "axe.h"
#include "ent.h"
#include "commands.h"

int readTag(int entId, char *tag, uint8_t *buffer, int buffLen) {  
  FILE *fp = fopen(entFileNames[entId],"rb");
  if(fp == NULL) return 0;
  else {
    uint8_t *fileBuffer = (uint8_t*) malloc(MAX_ENTSIZE);
    int fileLen = fread((void*)fileBuffer,1,MAX_ENTSIZE,fp);
    int tagLen = strlen(tag);
    for(int position=0;position<fileLen-tagLen;position++) {
      if(strncmp((char*)(fileBuffer+position),tag,tagLen) == 0) {
	/* Found tag */
	int tagStart=position+tagLen;
	int lenToCopy = min(fileLen-tagStart, buffLen);
	memcpy((void*)buffer,(void*)fileBuffer+tagStart,lenToCopy);
	free(fileBuffer);
	fclose(fp);
	return lenToCopy;
      }
    }    
    if(!quiet)
      printf("Warning: could not find tag %s in the file %s\n",tag,entFileNames[entId]);
    free(fileBuffer);
    fclose(fp);
    return 0;
  }
}
int writeTagPart(int entId,char *tag,int offset,uint8_t *buffer,uint8_t len) {
  if(simulateOnly) return len;

  FILE *fp = fopen(entFileNames[entId],"r+b");
  if(fp == NULL) return 0;
  else {
    uint8_t *fileBuffer = (uint8_t*) malloc(MAX_ENTSIZE);
    int fileLen = fread((void*)fileBuffer,1,MAX_ENTSIZE,fp);
    int tagLen = strlen(tag);
    for(int position=0;position<fileLen-tagLen;position++) {
      if(strncmp((char*)(fileBuffer+position),tag,tagLen) == 0) {
	/* Found tag */
	fseek(fp, position+tagLen+offset, SEEK_SET);
	int ret=fwrite(buffer,len,1,fp);
	fclose(fp);
	free(fileBuffer);
	return ret;
      }
    }    
    if(!quiet)
      printf("Warning: could not find tag %s in the file %s\n",tag,entFileNames[entId]);
    free(fileBuffer);
    fclose(fp);
    return 0;
  }
}

uint32_t readCredits(int entId) {
  uint8_t buf[4];
  if(readTag(entId,"credits",buf,sizeof(buf)) != 4) 
    return -1;
  else
    return bufToUInt32(buf);
}
void writeCredits(int entId,uint32_t credits) {
  uint8_t buf[4];
  uint32ToBuf(credits,buf);
  writeTagPart(entId,"credits",0,buf,sizeof(uint32_t));
}

int indentation=0;

int getOrdinal(uint8_t **buffer) {
  return *((*buffer)++);
}
void getLabel(uint8_t **buffer, char *label, int maxLabelLength) {
  int j;
  int strlen = bufToUInt16(*buffer);
  *buffer += 2;
  if(strlen > maxLabelLength-1) { printf("too long label in struct\n"); exit(-1); }
  for(j=0;j<strlen;j++) label[j]=*(*buffer+j);
  label[j]=0;
  *buffer += strlen;
}
/** Skips past the next struct entry, returns 0 if the struct has no more entries. Otherwise 1 */
int skipStructEntry(uint8_t **data) {
  int ordinal = getOrdinal(data);
  char label[256];
  if(ordinal==0) return 0;
  getLabel(data,label,sizeof(data));
  switch(ordinal) {
  case INT8: *data+=1; break;
  case INT16: *data+=2; break;
  case INT32: *data+=4; break;
  case FLOAT: *data+=sizeof(float); break;
  case STRUCT: while(skipStructEntry(data)) {}; break;    
  default:
    printf("Error - unknown datatype %d (or rather, i'm just lazy) in struct\n",ordinal);
    exit(0);
  }  
  return 1;
}

void printEntry(int ordinal, char *label, uint8_t **data);
void printIndentation() { for(int i=0;i<indentation;i++) printf("  "); }
void printList(uint8_t **data) {
  int ordinal=getOrdinal(data);
  int nEntries=bufToInt32(*data);
  *data += 4;
  printf("[%d] { \n",nEntries);
  indentation++;
  for(int j=0;j<nEntries;j++) {
    printIndentation();
    char fakeLabel[256];
    sprintf(fakeLabel,"%d",j);
    printEntry(ordinal,fakeLabel,data);
  }
  indentation--;
  printIndentation();
  printf("}\n");
}
void printStruct(uint8_t **data) {
  int i,j;
  char label[256];
  printf("{\n");
  indentation++;
  for(int entry=0;;entry++) {
    int ordinal = getOrdinal(data);
    if(ordinal == 0) break;
    getLabel(data,label,sizeof(label));
    printIndentation();
    printEntry(ordinal, label, data);
  }
  indentation--;
  for(int i=0;i<indentation;i++) printf("  ");
  printf("}\n");
}
void printEntry(int ordinal, char *label, uint8_t **data) {
  switch(ordinal) {
  case INT8: printf("byte %s : %d\n",label,**data); *data+=1; break;
  case INT16: printf("short %s : %d\n",label,bufToInt16(*data)); *data+=2; break;
  case INT32: printf("int %s : %d\n",label,bufToInt32(*data)); *data+=4; break;
  case INT64: printf("long %s : %ld\n",label,bufToInt64(*data)); *data+=8; break;
  case FLOAT: printf("float %s : %f\n",label,bufToFloat(*data)); *data+=sizeof(float); break;
  case FLOAT3: {
    float *f=(float*) (*data); printf("float3 %s : %f %f %f\n",label,f[0],f[1],f[2]); *data+=3*sizeof(float); 
    break;
  }
  case INT3: {
    printf("int3 %s : %d %d %d\n",label,bufToInt32(*data),bufToInt32(*data+4),bufToInt32(*data+8)); *data += 12;
    break;
  }
  case STRING: {
    char str[1024];
    getLabel(data,str,sizeof(str));
    printf("%s : %s\n",label,str);
    break;
  }
  case LIST: printf("list %s : ",label); printList(data); break;
  case STRUCT: printf("struct %s : ",label); printStruct(data); break;
  case BYTEARRAY: {
    int nBytes = bufToInt32(*data); data += 4;
    data += nBytes;
    printf("bytearray %s : <binary size %d>\n",label,nBytes);
    break;
  }
  case SERIALIZABLE: {
    printf("error - contains a raw serialized java object. I can't recover\n");
    exit(0);
  }
  default:
    printf("Error - unknown datatype %d (or rather, i'm just lazy) in struct\n",ordinal);
    exit(0);
  }
}

int findBlockIndex(int entId, int blockId,int previous) {
  uint8_t typesBuffer[100000];
  int bufLen=readTag(entId,"types",typesBuffer,sizeof(typesBuffer));
  if(!bufLen) return -1;

  int contentOrdinal = typesBuffer[0];
  if(contentOrdinal != INT16) {
    fprintf(stderr,"Error, something has changed in the fileformat. The Ents are fighting back. Better to flee like a coward than risk messing with it.\n");
    exit(-1);
  }
  int numTypes = bufToUInt32(typesBuffer+1);
  int contentPos = 1+4;
  for(int i=0;i<numTypes && contentPos < bufLen;i++, contentPos+=2)
    if(i>previous && bufToInt16(typesBuffer+contentPos) == blockId) {
      /*if(!quiet) 
	printf("Block %d is at index %d\n",blockId,i);*/
      return i;
    }
  return -1;
}


void printInventory(int entId) { 
  uint8_t valuesBuffer[100000];
  uint8_t typesBuffer[100000];
  /* Reading in the data through readTag is safer since we can avoid getting stuck on serialized objects */
  int valLen=readTag(entId,"values",valuesBuffer,sizeof(valuesBuffer));
  int typeLen=readTag(entId,"types",typesBuffer,sizeof(typesBuffer));
  if(valLen<1 || typeLen<1) {
    printf("Error finding all tags needed to print inventory\n");
  }
  char label[256];

//  hexDump(typesBuffer,typeLen);

  int typesOrdinal = typesBuffer[0];
  if(typesOrdinal != INT16) {
    fprintf(stderr,"Error, something has changed in the fileformat. The Ents are fighting back. Better to flee like a coward than risk messing with it.\n");
    exit(-1);
  }
  int numTypes = bufToUInt32(typesBuffer+1);
  uint8_t *typesPnt=typesBuffer+5;
  uint8_t *valuesPnt=valuesBuffer;

  for(int i=0;i<numTypes;i++) {
    int type = bufToInt16(typesPnt); 
    typesPnt += 2;

    uint8_t *pnt = valuesPnt;
    int valueOrdinal=getOrdinal(&pnt);
    getLabel(&pnt,label,sizeof(label));
    if(valueOrdinal == INT32) {
      int amount = bufToInt32(pnt);
      printf("%d %d\n",type,amount);
    }
    skipStructEntry(&valuesPnt);
  }
}
int getValueAsInt(int entId, int index) {
  uint8_t buffer[100000];
  uint8_t *pnt=buffer;
  char label[256];
  int bufLen=readTag(entId,"values",buffer,sizeof(buffer));
  if(!bufLen) {
    printf("Error - ENT file does not contain a tag 'values'\n");
    return -1;
  }

  while(index --> 0) skipStructEntry(&pnt);

  int ordinal=getOrdinal(&pnt);
  getLabel(&pnt,label,sizeof(label));
  if(ordinal != INT32) {
    printf("Error - requested value is not a normal non-meta block, or some other error in filestructure.\n");
    exit(0);
  }
  return bufToInt32(pnt);
}
void setValueAsInt(int entId, int index, int value) {

  uint8_t buffer[100000];
  uint8_t *pnt=buffer;
  char label[256];
  int bufLen=readTag(entId,"values",buffer,sizeof(buffer));

  while(index --> 0) skipStructEntry(&pnt);

  int ordinal=getOrdinal(&pnt);
  getLabel(&pnt,label,sizeof(label));
  if(ordinal != INT32) {
    printf("Error - requested value is not a normal non-meta block, or some other error in filestructure.\n");
    exit(0);
  }
  uint8_t buf[4];
  uint32ToBuf(value,buf);
  writeTagPart(entId, "values", pnt-buffer, buf, 4);
}

int findFreeSlot(int entId) {
  int isUsed[MAXMAX_SLOTS];
  int i;
  uint8_t buf[100000];
  for(i=0;i<MAXMAX_SLOTS && i<maxSlots;i++) isUsed[i]=0;
  int len = readTag(entId,"slots",buf,sizeof(buf));
  if(!len) return -1;
  int contentOrdinal = buf[0];
  if(contentOrdinal != INT32) {
    fprintf(stderr,"Error, something has changed in the fileformat. The Ents are fighting back. Better to flee like a coward than risk messing with it.\n");
    exit(-1);
  }
  int numSlots = bufToUInt32(buf+1);
  uint8_t *pnt = buf + 5;
  for(int i=0;i<numSlots;i++) {
    int slot = bufToInt32(pnt);
    pnt += 4;
    if(slot >= 0 && slot < MAXMAX_SLOTS) {
      isUsed[slot]=1;
    }
  }
  for(i=0;i<MAXMAX_SLOTS && i<maxSlots;i++) if(!isUsed[i]) return i;
  return -1;
}


int readEntToBuffer(int entId, uint8_t *buf,int bufsize) {
  FILE *fp = fopen(entFileNames[entId],"rb");
  if(fp == NULL) return -1;
  else {
    int len=fread((void*)buf,1,bufsize,fp);
    if(len == bufsize) {
      printf("Warning - this is suspicous. The ENT file was _exactly_ as large as the buffer, or we ran out of buffer space which means that Mathias has been lazy for not doing it dynamically allocated\n");
    }
    fclose(fp);
    return len;
  }
}
int writeBufferToEnt(int entId, uint8_t *buf,int length) {
  FILE *fp = fopen(entFileNames[entId],"wb");
  if(fp == NULL) return -1;
  else {
    int len=fwrite((void*)buf,1,length,fp);
    fclose(fp);
    return len;
  }  
}

void flushToAfterTag(uint8_t **buf, char *tag, uint8_t *endOfBuffer) {
  int pos;
  for(pos=0;*buf<endOfBuffer;(*buf)++) {
    if(tag[pos]==0) break;
    if(**buf != tag[pos++]) pos=0;
  }
}
/** Inserts the space that is used for each struct entry (datatype&label), but no actual data. Returns the length with which the buffer has grown */
int insertStructEntry(uint8_t **pnt,uint8_t *end,int length,int type,char *label) {
  int tagLen = strlen(label);
  if(*pnt+tagLen+3 >= end) {
    printf("Error, running out of buffer space while adding int32 to buffer\n");
    exit(0);
  }
  printf("length=%d\n",length);
  printf("Shifting %d steps\n",tagLen+3);
  memmove((*pnt)+tagLen+3,*pnt, length);
  **pnt = type; *pnt += 1;
  int16ToBuf(tagLen, *pnt); *pnt += 2;
  memcpy(*pnt,label,tagLen); *pnt += tagLen;
  return tagLen+3;
}
/** Inserts an int32 into buffer. Returns the length with which the buffer has grown. */
int insertInt32(uint8_t **pnt,uint8_t *end,int length,int value) {
  if(*pnt+4 >= end) {
    printf("Error, running out of buffer space while adding int32 to buffer\n");
    exit(0);
  }
  memmove((*pnt)+4,*pnt, length);
  int32ToBuf(value, *pnt); *pnt += 4;
  return 4;
}
/** Inserts an int16 into buffer. Returns the length with which the buffer has grown. */
int insertInt16(uint8_t **pnt,uint8_t *end,int length,int value) {
  if(*pnt+2 >= end) {
    printf("Error, running out of buffer space while adding int32 to buffer\n");
    exit(0);
  }
  memmove((*pnt)+2,*pnt, length);
  int16ToBuf(value, *pnt); *pnt += 2;
  return 2;
}
int addInt32ToList(uint8_t **pnt,uint8_t *end,int length,int value) {
  int type=**pnt;
  if(type != INT32) {
    printf("Attempting to add int32 to list not containing int32 (type is: %d)'s\n",type);
    exit(0);
  }
  *pnt += 1;
  int oldLen = bufToInt32(*pnt);
  int32ToBuf(oldLen+1,*pnt);
  *pnt += 4;
  return insertInt32(pnt,end,length,value);
}
int addInt16ToList(uint8_t **pnt,uint8_t *end,int length,int value) {
  int type=**pnt;
  if(type != INT16) {
    printf("Attempting to add int32 to list not containing int16's (type is: %d)\n",type);
    exit(0);
  }
  *pnt += 1;
  int oldLen = bufToInt32(*pnt);
  int32ToBuf(oldLen+1,*pnt);
  *pnt += 4;
  return insertInt16(pnt,end,length,value);
}

int createNewBlockEntry(int entId,int blockId,int amount) {
  uint8_t buf[100000];
  uint8_t *pnt;

  printf("creating a new block entry\n");

  int blockSlot = findFreeSlot(entId);
  if(blockSlot == -1) {
    if(!quiet) printf("Warning, could not find a free slot to place blocks in\n");
    return -1;
  }
  int len=readEntToBuffer(entId,buf,sizeof(buf));
  printf("read ent to buffer -> %d\n",len);

  /* Write fresh slot value at the first index of 'slots' */
  pnt = buf;
  flushToAfterTag(&pnt,"slots",buf+len);
  if(pnt == buf+len) {
    printf("Error creating new block, there are no tag 'slots' in file\n");
    exit(0);
  }
  len+=addInt32ToList(&pnt,buf+sizeof(buf),len-(pnt-buf),blockSlot);

  /* Write the correct block type at the first index of 'types' */
  pnt = buf;
  flushToAfterTag(&pnt,"types",buf+len);
  if(pnt == buf+len) {
    printf("Error creating new block, there are no tag 'types' in file\n");
    exit(0);
  }
  len+=addInt16ToList(&pnt,buf+sizeof(buf),len-(pnt-buf),blockId);

 
  /* Write the correct amount at the first index of 'values' */
  pnt = buf;
  flushToAfterTag(&pnt,"values",buf+len);
  if(pnt == buf+len) {
    printf("Error creating new block, there are no tag 'values' in file\n");
    exit(0);
  }
  //printf("len=%d, current position: %d\n",len,pnt-buf);
  len+=insertStructEntry(&pnt,buf+sizeof(buf),len-(pnt-buf)+100,INT32,"null");
  len+=insertInt32(&pnt,buf+sizeof(buf),len-(pnt-buf)+100,amount);
  
  writeBufferToEnt(entId,buf,len);
}

int getSector(int entId, int *xyz) {
  uint8_t buffer[1024];
  int len = readTag(entId, "sector", buffer, sizeof(buffer));
  if(len < 4*3) return 0;
  uint8_t *pnt = buffer;
  xyz[0] = bufToInt32(pnt);
  xyz[1] = bufToInt32(pnt+4);
  xyz[2] = bufToInt32(pnt+8);
  return 1;
}
