/** \file commands.c
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
    writeCredits(i,max(0,credits));
  }
}

void addAllCredits(int credits) {
  for(int i=0;i<nEntFiles;i++) {
    if(!quiet) printf("Adding %d credits to %s\n",credits,entFileNames[i]);
    writeCredits(i,max(0,readCredits(i)+credits));
  }
}

void multAllCredits(float factor) {
  for(int i=0;i<nEntFiles;i++) {
    if(!quiet) printf("Multiplying credits of %s by %.3f\n",entFileNames[i],factor);
    writeCredits(i,max(0,(int)(readCredits(i)*factor)));
  }
}

void printAllBlock(int block) {
  for(int i=0;i<nEntFiles;i++) {
    printf("%s : ",entFileNames[i]);
    int index=findBlockIndex(i,block);
    printf("%d\n",getValueAsInt(i,index));    
  }
}
void setBlockAmount(int entId, int block, int amount) {
  if(amount == 0) {
    printf("Sorry, we cannot delete items yet - for now it will just have to be a zero entry item\n");
  }
  int index=findBlockIndex(entId,block);
  //if(index == -1) createNewBlock
}
int getBlockAmount(int entId, int block) {
  int index=findBlockIndex(entId,block);
  return getValueAsInt(entId,index);
}
void setAllBlock(int block, int amount) {
  printf("SetAllBlock %d %d\n",block,amount);
  for(int i=0;i<nEntFiles;i++) {
    printf("Setting block %d in %s\n",block,entFileNames[i]);
    int index=findBlockIndex(i,block);
    if(index != -1) setValueAsInt(i,index,max(0,amount));
    else {
      if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
      createNewBlockEntry(i, block, max(0,amount));
    }
  }
}
void addAllBlock(int block, int amount) {
  printf("AddAllBlock %d %d\n",block,amount);
  for(int i=0;i<nEntFiles;i++) {
    int index=findBlockIndex(i,block);
    if(index != -1) setValueAsInt(i,index,max(0,getValueAsInt(i,index)+amount));
    else {
      if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
      createNewBlockEntry(i, block, max(0,amount));
    }
  }
}
void multAllBlock(int block, float factor) { 
  printf("MultAllBlock %d %f\n",block,factor); 
  for(int i=0;i<nEntFiles;i++) {
    printf("Multiplying block %d in %s\n",block,entFileNames[i]);
    int index=findBlockIndex(i,block);
    if(index != -1) setValueAsInt(i,index,max(0,getValueAsInt(i,index)*factor));
    else {
      //if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
      //createNewBlockEntry(i, block, 0);
    }
  }
}
void printAll() {
  uint8_t buf[100000];
  for(int i=0;i<nEntFiles;i++) {
    printf("%s\n",entFileNames[i]);
    FILE *fp = fopen(entFileNames[i],"rb");
    int len = fread((void*)buf,1,sizeof(buf),fp);
    fclose(fp);

    uint8_t *pnt=buf+2;
    printStruct(&pnt);
  }
}
void printFrom(char *tag) {
  uint8_t buf[100000];
  for(int i=0;i<nEntFiles;i++) {
    printf("%s\n",entFileNames[i]);
    int len=readTag(i,tag,buf,sizeof(buf));    
    if(len>0) {
      uint8_t *pnt=buf;
      printStruct(&pnt);
    }
  }
}

