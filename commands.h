/** \file commands.h
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

#ifndef COMMANDS_H

void printAll();
void printAllSector();
void printFrom(char*);
void printAllCredits();
void printAllInventory();
void setAllCredits(int);
void addAllCredits(int);
void multAllCredits(float);
void printAllBlock(int);
void setAllBlock(int,int);
void addAllBlock(int,int);
void multAllBlock(int,float);

#endif
