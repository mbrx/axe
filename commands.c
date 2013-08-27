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
    printf("%s : ",entFileNames[i]);
    if(!quiet) printf("Printing credits for %s\n",entFileNames[i]);
    int credits = readCredits(i);
    printf("%d\n",credits);
  }
}

void printAllInventory() {
  for(int i=0;i<nEntFiles;i++) {
    printf("%s\n",entFileNames[i]);
    if(!quiet) printf("<blockId> <amount>\n");
    printInventory(i);
  }
}

void printAllSector() {
  int xyz[3];
  for(int i=0;i<nEntFiles;i++) {
    printf("%s : ",entFileNames[i]);
    if(getSector(i,xyz) == 0) printf("fail\n");
    else printf("%d %d %d\n",xyz[0],xyz[1],xyz[2]);
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
    int prevIndex=-1;
    for(int total=0;;) {
      int index=findBlockIndex(i,block, prevIndex);

      if(index == -1) { 
	printf("%d\n",total); 
	return; 
      }
      prevIndex=index;
      total += getValueAsInt(i,index);
    }
  }
}
void setBlockAmount(int entId, int block, int amount) {
  if(amount == 0) {
    printf("Sorry, we cannot delete items yet - for now it will just have to be a zero entry item\n");
  }
  int index=findBlockIndex(entId,block,-1);
  //if(index == -1) createNewBlock
}
int getBlockAmount(int entId, int block) {
  int total=0;
  int prev=-1;
  for(;;) {
    int index=findBlockIndex(entId,block,prev);
    if(index == -1) return total;
    total += getValueAsInt(entId,index);
  }
  return 0;
}
void setAllBlock(int block, int amount) {
  printf("SetAllBlock %d %d\n",block,amount);
  for(int i=0;i<nEntFiles;i++) {
    printf("Setting block %d in %s\n",block,entFileNames[i]);
    int first=findBlockIndex(i,block,-1);
    if(first != -1) {
      setValueAsInt(i,first,max(0,amount));
      while(1) {
	int next=findBlockIndex(i,block,first);
	if(next == -1) break;
	setValueAsInt(i,first,0);
	first=next;
      }
    }
    else {
      if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
      createNewBlockEntry(i, block, max(0,amount));
    }
  }
}
/* TODO - make this function also safe for multiple instances of a block */
void addAllBlock(int block, int amount) {
  printf("AddAllBlock %d %d\n",block,amount);
  for(int i=0;i<nEntFiles;i++) {
    int index=findBlockIndex(i,block,-1);
    if(index != -1) setValueAsInt(i,index,max(0,getValueAsInt(i,index)+amount));
    else {
      if(!quiet) printf("Block %d not previously existing in %s\n",block,entFileNames[i]);
      createNewBlockEntry(i, block, max(0,amount));
    }
  }
}
/* TODO - make this function also safe for multiple instances of a block */
void multAllBlock(int block, float factor) { 
  printf("MultAllBlock %d %f\n",block,factor); 
  for(int i=0;i<nEntFiles;i++) {
    printf("Multiplying block %d in %s\n",block,entFileNames[i]);
    int index=findBlockIndex(i,block,-1);
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

