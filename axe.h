/** \file axe.h
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
    
#ifndef AXE_H

#define MAXMAX_SLOTS 1000
#define MAX_ENTSIZE 100*1024

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

extern char **entFileNames;
extern int nEntFiles;
extern int simulateOnly;
extern int quiet;
extern int maxSlots;

int min(int,int);
int max(int,int);
uint16_t bufToUInt16(uint8_t *buf);
uint32_t bufToUInt32(uint8_t *buf);
uint64_t bufToUInt64(uint8_t *buf);
int16_t bufToInt16(uint8_t *buf);
int32_t bufToInt32(uint8_t *buf);
int64_t bufToInt64(uint8_t *buf);
float bufToFloat(uint8_t *buf);
void uint32ToBuf(uint32_t val, uint8_t *buf);
void uint16ToBuf(uint16_t val, uint8_t *buf);
void int32ToBuf(int32_t val, uint8_t *buf);
void int16ToBuf(int16_t val, uint8_t *buf);

#endif
