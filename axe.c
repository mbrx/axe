/** \file axe.c
    \brief A tool for hacking ENT files for starmade.

    This code is release "as-is" with no warranties whatsoever - it may very well crash your server and destroy all player/shop inventories. But I hope not...

    Additionally, the source code is released mostly as a warning to others. If you read it - Beware of dragons!
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

char **entFileNames;
int nEntFiles;
int simulateOnly=0;
int quiet=0;

#define EOS 0
#define INT8 1
#define INT16 2
#define INT32 3
#define INT64 4
#define FLOAT 5
#define DOUBLE 6
#define BYTEARRAY 7
#define STRING 8
#define FLOAT3 9
#define INT3 10
#define LIST 12
#define STRUCT 13
#define SERIALIZABLE 14


void printUsage(char *programName) {
  printf("Usage: %s (command | option | filename)*\n"
	 "where valid commands are\n"
	 "       --print-all                    Print data-tree for ENT file. Will abort if unhandled datatypes exists in files.\n"
	 "       --print-credits                Print credits for playerstate type ENT files\n"
	 "       --set-credits <integer>        Sets the credits of playerstate type ENT files\n"
	 "       --add-credits <integer>        Adds a given amount of credits (can be negative) to playerstate type ENT files\n"
	 "       --mult-credits <float>         Multiplies credits by a constant\n"
	 "       --print-block <id>             Prints number of blocks matching the ID that is found in the playerstate OR shop type ENT files\n"
	 "       --set-block <id> <integer>     Sets the number of blocks of given ID that is found in the playerstate OR shop type ENT files\n"
	 "       --add-block <id> <integer>     Adds the number of blocks of given ID (can be negative).\n"
	 "       --mult-block <id> <float>      Multiplies the number of blocks of given ID in the playerstate OR shop type ENT files\n"
	 "\n",
	 "valid options are"
	 "       --simulate                     Prevents any changes from beeing written to disk\n",
	 "       --quiet                        Suppresses any informational printouts to simplify script processing\n",
    programName);
  exit(0);
}


void printAll();
void printAllCredits();
void setAllCredits(int);
void addAllCredits(int);
void multAllCredits(float);
void printAllBlock(int);
void setAllBlock(int,int);
void addAllBlock(int,int);
void multAllBlock(int,float);

int min(int a,int b) { return a<b?a:b;}
int max(int a,int b) { return a>b?a:b;}

int main(int argc, char **args) {

  nEntFiles=0;
  entFileNames=(char**) malloc(0);

  /** Loop in two stages over the commandline options.
      Stage 0: Add all left overs to the list of ent's to be hacked.
      Stage 1: Process any commands and apply to all ents.
  */
  for(int stage=0;stage<2;stage++)
    for(int i=1;i<argc;i++) {
      if(strcmp(args[i],"--help") == 0) { if(stage) printUsage(args[0]); }
      else if(strcmp(args[i],"--print-all") == 0) {if(stage) printAll();  }
      else if(strcmp(args[i],"--print-credits") == 0) {if(stage) printAllCredits();  }
      else if(strcmp(args[i],"--set-credits") == 0) {if(stage) setAllCredits(atoi(args[++i])); else i++; }
      else if(strcmp(args[i],"--add-credits") == 0) {if(stage) addAllCredits(atoi(args[++i])); else i++; }
      else if(strcmp(args[i],"--mult-credits") == 0) {if(stage) multAllCredits(atof(args[++i])); else i++; }
      else if(strcmp(args[i],"--print-block") == 0) {if(stage) printAllBlock(atoi(args[i+1])); i += 1; }
      else if(strcmp(args[i],"--set-block") == 0) {if(stage) setAllBlock(atoi(args[i+1]), atoi(args[i+2])); i+=2; }
      else if(strcmp(args[i],"--add-block") == 0) {if(stage) addAllBlock(atoi(args[i+1]), atoi(args[i+2])); i+=2; }
      else if(strcmp(args[i],"--mult-block") == 0) {if(stage) multAllBlock(atoi(args[i+1]), atof(args[i+2])); i+=2; }
      else if(strcmp(args[i],"--simulate") == 0) simulateOnly=1;
      else if(strcmp(args[i],"--quiet") == 0) quiet=1;
      else {
	/* This is a file to process, add it to the ent list */
	if(stage == 0) {
	  entFileNames = (char**) realloc((void*)entFileNames,(nEntFiles+1)*sizeof(char*));
	  entFileNames[nEntFiles++]=args[i];
	} 
      }
    }
}

/** \brief Opens the corresonding ent file, find the tag and copies it into the buffer and returns the buffer length.

    Returns zero on failure, or successfully read tags if non-zero. 
 */
#define MAX_ENTSIZE 100*1024

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

uint16_t bufToUInt16(uint8_t *buf) {
  return (buf[0]<<8) | (buf[1]<<0);
}
uint32_t bufToUInt32(uint8_t *buf) {
  return (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | (buf[3]<<0);
}
uint64_t bufToUInt64(uint8_t *buf) {
  return (((uint64_t)buf[0])<<56) | (((uint64_t)buf[1])<<48) | (((uint64_t)buf[2])<<40) | (((uint64_t)buf[3])<<32) | (((uint64_t)buf[4])<<24) | (((uint64_t)buf[5])<<16) | (((uint64_t)buf[6])<<8) | (((uint64_t)buf[7])<<0);
}
int16_t bufToInt16(uint8_t *buf) {
  return (buf[0]<<8) | (buf[1]<<0);
}
int32_t bufToInt32(uint8_t *buf) {
  return (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | (buf[3]<<0);
}
int64_t bufToInt64(uint8_t *buf) {
  return (((int64_t)buf[0])<<56) | (((int64_t)buf[1])<<48) | (((int64_t)buf[2])<<40) | (((int64_t)buf[3])<<32) | (((int64_t)buf[4])<<24) | (((int64_t)buf[5])<<16) | (((int64_t)buf[6])<<8) | (((int64_t)buf[7])<<0);
}
float bufToFloat(uint8_t *buf) {
  return *((float*) buf);
}
void uint32ToBuf(uint32_t val, uint8_t *buf) {
  buf[0]=(val>>24) & 0xff;
  buf[1]=(val>>16) & 0xff;
  buf[2]=(val>>8) & 0xff;
  buf[3]=(val>>0) & 0xff;
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

int findBlockIndex(int entId, int blockId) {
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
    if(bufToUInt16(typesBuffer+contentPos) == blockId) {
      /*if(!quiet) 
	printf("Block %d is at index %d\n",blockId,i);*/
      return i;
    }
  return -1;
}

int getValueAsInt(int entId, int index) {
  uint8_t buffer[100000];
  uint8_t *pnt=buffer;
  char label[256];
  int bufLen=readTag(entId,"values",buffer,sizeof(buffer));

  /*
  printf("Values : ");
  uint8_t *pnt2=buffer;
  printStruct(&pnt2);
  */
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

void printAllCredits() {
  for(int i=0;i<nEntFiles;i++) {
    if(!quiet) printf("Printing credits for %s\n",entFileNames[i]);
    int credits = readCredits(i);
    printf("%d\n",credits);
  }
}

void setAllCredits(int credits) {
  for(int i=0;i<nEntFiles;i++) {
    if(!quiet) printf("Setting %d credits to %s\n",credits,entFileNames[i]);
    writeCredits(i,credits);
  }
}

void addAllCredits(int credits) {
  for(int i=0;i<nEntFiles;i++) {
    if(!quiet) printf("Adding %d credits to %s\n",credits,entFileNames[i]);
    writeCredits(i,readCredits(i)+credits);
  }
}

void multAllCredits(float factor) {
  for(int i=0;i<nEntFiles;i++) {
    if(!quiet) printf("Multiplying credits of %s by %.3f\n",entFileNames[i],factor);
    writeCredits(i,(int)(readCredits(i)*factor));
  }
}

void printAllBlock(int block) {
  for(int i=0;i<nEntFiles;i++) {
    printf("%s : ",entFileNames[i]);
    int index=findBlockIndex(i,block);
    printf("%d\n",getValueAsInt(i,index));    
  }
}
void setAllBlock(int block, int amount) {
  printf("SetAllBlock %d %d\n",block,amount);
  for(int i=0;i<nEntFiles;i++) {
    printf("Setting block %d in %s\n",block,entFileNames[i]);
    int index=findBlockIndex(i,block);
    if(index != -1) setValueAsInt(i,index,amount);
    else if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
  }
}
void addAllBlock(int block, int amount) {
  printf("AddAllBlock %d %d\n",block,amount);
  for(int i=0;i<nEntFiles;i++) {
    int index=findBlockIndex(i,block);
    if(index != -1) setValueAsInt(i,index,getValueAsInt(i,index)+amount);
    else if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
  }
}
void multAllBlock(int block, float factor) { 
  printf("MultAllBlock %d %f\n",block,factor); 
  for(int i=0;i<nEntFiles;i++) {
    printf("Multiplying block %d in %s\n",block,entFileNames[i]);
    int index=findBlockIndex(i,block);
    if(index != -1) setValueAsInt(i,index,getValueAsInt(i,index)*factor);
    else if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
  }
}
void printAll() {
  uint8_t buf[100000];
  for(int i=0;i<nEntFiles;i++) {
    FILE *fp = fopen(entFileNames[i],"rb");
    int len = fread((void*)buf,1,sizeof(buf),fp);
    fclose(fp);
    printf("%s\n",entFileNames[i]);
    uint8_t *pnt=buf+2;
    printStruct(&pnt);
  }
}
