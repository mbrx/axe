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
#include "axe.h"
#include "commands.h"

char **entFileNames;
int nEntFiles;
int simulateOnly=0;
int quiet=0;
int maxSlots=45;

void printUsage(char *programName) {
  printf("Usage: %s (command | option | filename)*\n"
	 "where valid commands are\n"
	 "       --print-all                    Print data-tree for ENT file. Will abort if unhandled datatypes exists in files.\n"
	 "       --print-from <string>          Searches for given 'struct tag' in file and starts printing from there.\n"
	 "       --print-credits                Print credits for playerstate type ENT files\n"
	 "       --set-credits <integer>        Sets the credits of playerstate type ENT files\n"
	 "       --add-credits <integer>        Adds a given amount of credits (can be negative) to playerstate type ENT files\n"
	 "       --mult-credits <float>         Multiplies credits by a constant\n"
	 "       --print-block <id>             Prints number of blocks matching the ID that is found in the playerstate OR shop type ENT files\n"
	 "       --set-block <id> <integer>     Sets the number of blocks of given ID that is found in the playerstate OR shop type ENT files\n"
	 "       --add-block <id> <integer>     Adds the number of blocks of given ID (can be negative).\n"
	 "       --mult-block <id> <float>      Multiplies the number of blocks of given ID in the playerstate OR shop type ENT files\n"
	 "       --print-inventory              Prints each blockID and number that is found in the inventory on a new line.\n"
	 "       --print-sector                 Prints <filename> : <x> <y> <z>  where xyz are the last known sector of the player.\n"
	 "\n",
	 "valid options are"
	 "       --simulate                     Prevents any changes from beeing written to disk\n",
	 "       --quiet                        Suppresses any informational printouts to simplify script processing\n",
	 "       --max-slots <integer>          Highest slot-index when creating new block entries (default 45).\n",
    programName);
  exit(0);
}

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
      else if(strcmp(args[i],"--print-from") == 0) {if(stage) printFrom(args[i+1]); i++; }
      else if(strcmp(args[i],"--print-credits") == 0) {if(stage) printAllCredits();  }
      else if(strcmp(args[i],"--print-inventory") == 0) {if(stage) printAllInventory();  }
      else if(strcmp(args[i],"--print-sector") == 0) {if(stage) printAllSector(); }
      else if(strcmp(args[i],"--set-credits") == 0) {if(stage) setAllCredits(atoi(args[++i])); else i++; }
      else if(strcmp(args[i],"--add-credits") == 0) {if(stage) addAllCredits(atoi(args[++i])); else i++; }
      else if(strcmp(args[i],"--mult-credits") == 0) {if(stage) multAllCredits(atof(args[++i])); else i++; }
      else if(strcmp(args[i],"--print-block") == 0) {if(stage) printAllBlock(atoi(args[i+1])); i += 1; }
      else if(strcmp(args[i],"--set-block") == 0) {if(stage) setAllBlock(atoi(args[i+1]), atoi(args[i+2])); i+=2; }
      else if(strcmp(args[i],"--add-block") == 0) {if(stage) addAllBlock(atoi(args[i+1]), atoi(args[i+2])); i+=2; }
      else if(strcmp(args[i],"--mult-block") == 0) {if(stage) multAllBlock(atoi(args[i+1]), atof(args[i+2])); i+=2; }
      else if(strcmp(args[i],"--max-slots") == 0) {maxSlots=atoi(args[i]); i++; }
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
void uint16ToBuf(uint16_t val, uint8_t *buf) {
  buf[0]=(val>>8) & 0xff;
  buf[1]=(val>>0) & 0xff;
}
void int32ToBuf(int32_t val, uint8_t *buf) {
  buf[0]=(val>>24) & 0xff;
  buf[1]=(val>>16) & 0xff;
  buf[2]=(val>>8) & 0xff;
  buf[3]=(val>>0) & 0xff;
}
void int16ToBuf(int16_t val, uint8_t *buf) {
  buf[0]=(val>>8) & 0xff;
  buf[1]=(val>>0) & 0xff;
}

void hexDump(void *data,int datalen) {
  /* Print a hex-dump of the received data */
  int i, row, nRows=datalen/16;
  for(row=0;row<nRows||(row==nRows && datalen%16 != 0);row++) {
    /* Print hex values */
    for(i=0;i<16 && (row < nRows || i < (datalen%16));i++) {
      printf("%02x",((unsigned char*)data)[i+row*16]);
      if(i % 4 == 3) printf(" ");
    }
    /* Fill out remaining space */
    for(;i<16;i++) {
      printf("  ");
      if(i % 4 == 3) printf(" ");
    }
    /* Print chars */
    printf("    ");

    for(i=0;i<16 && (row < nRows || i < (datalen%16));i++) {
      unsigned char c = ((unsigned char*)data)[i+row*16];
      if(isprint(c))
	printf("%c",((unsigned char*)data)[i+row*16]);
      else
	printf("@");
      if(i % 4 == 3) printf(" ");
    }
    printf("\n");
  }
}
