/* wvWare
 * Copyright (ROMAN_C) Caolan McNamara, Dom Lachowicz, and others
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR ROMAN_A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* roman.c by Adam Rogoyski (apoc@laker.net) Temperanc on EFNet irc
 * Copyright (ROMAN_C) 1998 Adam Rogoyski
 * Converts Decimal numbers to Roman Numerals and Roman Numberals to
 * Decimals on the command line or in Interactive mode.
 * Uses an expanded Roman Numeral set to handle numbers up to 999999999
 * compile: gcc -o roman roman.c -O2
 * --- GNU General Public License Disclamer ---
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR ROMAN_A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "wv.h"

#define ROMAN_I 1
#define ROMAN_V 5
#define ROMAN_X 10
#define ROMAN_L 50
#define ROMAN_C 100
#define ROMAN_D 500
#define ROMAN_M 1000
#define ROMAN_P 5000
#define ROMAN_Q 10000
#define ROMAN_R 50000
#define ROMAN_S 100000
#define ROMAN_T 500000
#define ROMAN_U 1000000
#define ROMAN_B 5000000
#define ROMAN_W 10000000
#define ROMAN_N 50000000
#define ROMAN_Y 100000000
#define ROMAN_Z 500000000

#define LARGEST 999999999


//#include "roman.h"
#define wvError(x)

long
formString (char **r, long v, char a, char b)
{
    *(*r)++ = a;
    if (b)
	*(*r)++ = b;
    return v;
}

char *
decimalToRoman (long decimal, char *roman)
{
    char *r = roman;
    memset (roman, 0, 81);
    r = roman;
    if (decimal > LARGEST || decimal < 1)
      {
	  *r = '\0';
	  wvError (("roman broke\n"));
	  return roman;
      }
    if (decimal >= ROMAN_Z)
	decimal -= formString (&r, ROMAN_Z, 'Z', '\0');
    if (decimal >= ROMAN_Z - ROMAN_Y)
	decimal -= formString (&r, ROMAN_Z - ROMAN_Y, 'Y', 'Z');
    while (decimal >= ROMAN_Y)
	decimal -= formString (&r, ROMAN_Y, 'Y', '\0');
    if (decimal >= ROMAN_Y - ROMAN_W)
	decimal -= formString (&r, ROMAN_Y - ROMAN_W, 'W', 'Y');
    if (decimal >= ROMAN_N)
	decimal -= formString (&r, ROMAN_N, 'N', '\0');
    if (decimal >= ROMAN_N - ROMAN_W)
	decimal -= formString (&r, ROMAN_N - ROMAN_W, 'W', 'N');
    while (decimal >= ROMAN_W)
	decimal -= formString (&r, ROMAN_W, 'W', '\0');
    if (decimal >= ROMAN_W - ROMAN_U)
	decimal -= formString (&r, ROMAN_W - ROMAN_U, 'U', 'W');
    if (decimal >= ROMAN_B)
	decimal -= formString (&r, ROMAN_B, 'B', '\0');
    if (decimal >= ROMAN_B - ROMAN_U)
	decimal -= formString (&r, ROMAN_B - ROMAN_U, 'U', 'B');
    while (decimal >= ROMAN_U)
	decimal -= formString (&r, ROMAN_U, 'U', '\0');
    if (decimal >= ROMAN_U - ROMAN_S)
	decimal -= formString (&r, ROMAN_U - ROMAN_S, 'S', 'U');
    if (decimal >= ROMAN_T)
	decimal -= formString (&r, ROMAN_T, 'T', '\0');
    if (decimal >= ROMAN_T - ROMAN_S)
	decimal -= formString (&r, ROMAN_T - ROMAN_S, 'S', 'T');
    while (decimal >= ROMAN_S)
	decimal -= formString (&r, ROMAN_S, 'S', '\0');
    if (decimal >= ROMAN_S - ROMAN_Q)
	decimal -= formString (&r, ROMAN_S - ROMAN_Q, 'Q', 'S');
    if (decimal >= ROMAN_R)
	decimal -= formString (&r, ROMAN_R, 'R', '\0');
    if (decimal >= ROMAN_R - ROMAN_Q)
	decimal -= formString (&r, ROMAN_R - ROMAN_Q, 'Q', 'R');
    while (decimal >= ROMAN_Q)
	decimal -= formString (&r, ROMAN_Q, 'Q', '\0');
    if (decimal >= ROMAN_Q - ROMAN_M)
	decimal -= formString (&r, ROMAN_Q - ROMAN_M, 'M', 'Q');
    if (decimal >= ROMAN_P)
	decimal -= formString (&r, ROMAN_P, 'P', '\0');
    if (decimal >= ROMAN_P - ROMAN_M)
	decimal -= formString (&r, ROMAN_P - ROMAN_M, 'M', 'P');
    while (decimal >= ROMAN_M)
	decimal -= formString (&r, ROMAN_M, 'M', '\0');
    if (decimal >= ROMAN_M - ROMAN_C)
	decimal -= formString (&r, ROMAN_M - ROMAN_C, 'C', 'M');
    if (decimal >= ROMAN_D)
	decimal -= formString (&r, ROMAN_D, 'D', '\0');
    if (decimal >= ROMAN_D - ROMAN_C)
	decimal -= formString (&r, ROMAN_D - ROMAN_C, 'C', 'D');
    while (decimal >= ROMAN_C)
	decimal -= formString (&r, ROMAN_C, 'C', '\0');
    if (decimal >= ROMAN_C - ROMAN_X)
	decimal -= formString (&r, ROMAN_C - ROMAN_X, 'X', 'C');
    if (decimal >= ROMAN_L)
	decimal -= formString (&r, ROMAN_L, 'L', '\0');
    if (decimal >= ROMAN_L - ROMAN_X)
	decimal -= formString (&r, ROMAN_L - ROMAN_X, 'X', 'L');
    while (decimal >= ROMAN_X)
	decimal -= formString (&r, ROMAN_X, 'X', '\0');
    switch ((int) decimal)
      {
      case 9:
	  *r++ = 'I';
	  *r++ = 'X';
	  break;
      case 8:
	  *r++ = 'V';
	  *r++ = 'I';
	  *r++ = 'I';
	  *r++ = 'I';
	  break;
      case 7:
	  *r++ = 'V';
	  *r++ = 'I';
	  *r++ = 'I';
	  break;
      case 6:
	  *r++ = 'V';
	  *r++ = 'I';
	  break;
      case 4:
	  *r++ = 'I';
      case 5:
	  *r++ = 'V';
	  break;
      case 3:
	  *r++ = 'I';
      case 2:
	  *r++ = 'I';
      case 1:
	  *r++ = 'I';
	  break;
      }
    return roman;
}


long
romanToDecimal (char *roman)
{
    long decimal = 0;
    for (; *roman; roman++)
      {
	  /* Check for four of a letter in a fow */
	  if ((*(roman + 1) && *(roman + 2) && *(roman + 3))
	      && (*roman == *(roman + 1))
	      && (*roman == *(roman + 2)) && (*roman == *(roman + 3)))
	      return 0;
	  /* Check for two five type numbers */
	  if (((*roman == 'V') && (*(roman + 1) == 'V'))
	      || ((*roman == 'L') && (*(roman + 1) == 'L'))
	      || ((*roman == 'D') && (*(roman + 1) == 'D'))
	      || ((*roman == 'P') && (*(roman + 1) == 'P'))
	      || ((*roman == 'R') && (*(roman + 1) == 'R'))
	      || ((*roman == 'T') && (*(roman + 1) == 'T'))
	      || ((*roman == 'B') && (*(roman + 1) == 'B'))
	      || ((*roman == 'N') && (*(roman + 1) == 'N'))
	      || ((*roman == 'Z') && (*(roman + 1) == 'Z')))
	      return 0;
	  /* Check for two lower characters before a larger one */
	  if ((value (*roman) == value (*(roman + 1))) && (*(roman + 2))
	      && (value (*(roman + 1)) < value (*(roman + 2))))
	      return 0;
	  /* Check for the same character on either side of a larger one */
	  if ((*(roman + 1) && *(roman + 2))
	      && (value (*roman) == value (*(roman + 2)))
	      && (value (*roman) < value (*(roman + 1))))
	      return 0;
	  /* Check for illegal nine type numbers */
	  if (!strncmp (roman, "LXL", 3) || !strncmp (roman, "DCD", 3)
	      || !strncmp (roman, "PMP", 3) || !strncmp (roman, "RQR", 3)
	      || !strncmp (roman, "TST", 3) || !strncmp (roman, "BUB", 3)
	      || !strncmp (roman, "NWN", 3) || !strncmp (roman, "VIV", 3))
	      return 0;
	  if (value (*roman) < value (*(roman + 1)))
	    {
		/* check that subtracted value is at least 10% larger, 
		   i.e. 1990 is not MXM, but MCMXC */
		if ((10 * value (*roman)) < value (*(roman + 1)))
		    return 0;
		/* check for double subtraction, i.e. IVX */
		if (value (*(roman + 1)) <= value (*(roman + 2)))
		    return 0;
		/* check for subtracting by a number starting with a 5
		   ie.  VX, LD LM */
		if (*roman == 'V' || *roman == 'L' || *roman == 'D'
		    || *roman == 'P' || *roman == 'R' || *roman == 'T'
		    || *roman == 'B' || *roman == 'N')
		    return 0;
		decimal += value (*(roman + 1)) - value (*roman);
		roman++;
	    }
	  else
	    {
		decimal += value (*roman);
	    }
      }
    return decimal;
}


long
value (char c)
{
    switch (c)
      {
      case 'I':
	  return ROMAN_I;
      case 'V':
	  return ROMAN_V;
      case 'X':
	  return ROMAN_X;
      case 'L':
	  return ROMAN_L;
      case 'C':
	  return ROMAN_C;
      case 'D':
	  return ROMAN_D;
      case 'M':
	  return ROMAN_M;
      case 'P':
	  return ROMAN_P;
      case 'Q':
	  return ROMAN_Q;
      case 'R':
	  return ROMAN_R;
      case 'S':
	  return ROMAN_S;
      case 'T':
	  return ROMAN_T;
      case 'U':
	  return ROMAN_U;
      case 'B':
	  return ROMAN_B;
      case 'W':
	  return ROMAN_W;
      case 'N':
	  return ROMAN_N;
      case 'Y':
	  return ROMAN_Y;
      case 'Z':
	  return ROMAN_Z;
      default:
	  return 0;
      }
}


/* chomp carriage return off end of string */
char *
chomp (char *str)
{
    int i = 0;
    while (*(str + i))
      {
	  if ((*(str + i) == '\n') || (*(str + i) == '\r')
	      || (*(str + i) == '\0'))
	    {
		*(str + i) = 0;
		break;
	    }
	  else
	      i++;
      }
    return (str - i);
}
