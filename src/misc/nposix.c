/*
** $Id: nposix.c,v 1.9 2003/11/23 07:08:54 weiym Exp $
**
** nposix.c: This file include some miscelleous functions not provided by POSIX. 
**
** Copyright (C) 2003 Feynman Software.
**
** Create date: 2003/11/22
**
** Current maintainer: Wei Yongming.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
** TODO:
*/ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>

#include "common.h"

#ifndef HAVE_STRDUP
char* strdup (const char *string)
{
    char *new_string = malloc(strlen(string)+1);
    strcpy(new_string,string);
    return new_string;
}
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2)
{
    char c1, c2;
    while ((c1 = tolower(*s1++)) == (c2 = tolower(*s2++)))
        if (c1 == 0)
            return (0);
    return ((unsigned char)c1 - (unsigned char)c2);
}
#endif

#ifndef HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval *tv, void* tz)
{
    tv->tv_usec = 0;
    tv->tv_sec = time(NULL);
    return(0);
}
#endif

#ifdef __ECOS
#include <math.h>
double hypot(double x, double y)
{
    return sqrt (x*x + y*y);
}
#endif

