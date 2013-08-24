/** \file timber.c
    \brief A tool for parsing the StarMade 'log' files to check status of various events
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


void printUsage(char *programName) {
  printf("Usage: %s <logfile> (command | option | filename)*\n"
	 "where valid commands are\n"
	 "       --player-status <username>                Prints 1 if player is currently logged in, 0 otherwise.\n"
	 "\n",
	 "valid options are\n"
	 "        none\n",
    programName);
  exit(0);
}

int min(int a,int b) { return a<b?a:b;}
int max(int a,int b) { return a>b?a:b;}

void checkPlayerStatus(char *logFile, char *playerName);

int main(int argc, char **args) {

  if(argc < 2) printUsage(args[0]); 
  char *logFile = args[1];

  for(int i=2;i<argc;i++) {
    if(strcmp(args[i],"--help") == 0) { printUsage(args[0]);  }
    else if(strcmp(args[i],"--player-status") == 0) { checkPlayerStatus(logFile, args[++i]); }
  }
}

void checkPlayerStatus(char *logFile, char *playerName) {
  char str[10240];
  FILE *fp = fopen(logFile,"rb");
  if(!fp) {
    printf("Warning, could not open %s\n",logFile);
  }
  int i=0;
  int isIn=0;
  char *head1 = "[SERVER] UNREGISTERING CLIENT RegisteredClient: ";
  char *head2 = "Revived PlayerState PlS[";
  int head1Len = strlen(head1);
  int head2Len = strlen(head2);
  int playerNameLen = strlen(playerName);

  while(feof(fp) == 0) {
    char c, *p;
    int count=0;
    for(p=str;p<&str[10000];count++) {
      c=fgetc(fp);
      if(c == EOF) { fclose(fp); printf("%d\n",isIn); return; }
      if(c == '\n') break;
      else *(p++) = c;
    }
    *(p++) = 0;
    
    //printf("Line %d: %s\n",i++,str);
    
    if(count > head1Len && strncmp(str,head1,head1Len) == 0) {
      if(strncmp(str+head1Len, playerName,playerNameLen) == 0 && str[head1Len + playerNameLen] == ' ') {
	//printf("logout\n");
	isIn=0;
      }
    }
    if(count > head2Len && strncmp(str,head2,head2Len) == 0) {      
      if(strncmp(str+head2Len, playerName,playerNameLen) == 0 && str[head2Len + playerNameLen] == ';') {
	//printf("login\n");
	isIn=1;
      }
    }
  }
  printf("%d\n",isIn);
  fclose(fp);
}
