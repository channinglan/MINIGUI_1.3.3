/*
** $Id: gif.c,v 1.9 2003/09/04 06:12:04 weiym Exp $
**
** gif.c: the gif module 
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001 Wei Yongming and Zhang Yunfan.
**
** Create date: 2001/2/02
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MINIGUI_LIB__
	#include "common.h"
	#include "minigui.h"
	#include "gdi.h"
#else
    #include <minigui/common.h>
    #include <minigui/minigui.h>
    #include <minigui/gdi.h>
#endif

#ifdef _EXT_FULLGIF

#include "mgext.h"

/*
 * GIF decoding routine
 */
#define MAXCOLORMAPSIZE         256
#define MAX_LWZ_BITS            12
#define INTERLACE               0x40
#define LOCALCOLORMAP           0x80

#define CM_RED                  0
#define CM_GREEN                1
#define CM_BLUE                 2

#define BitSet(byte, bit)              (((byte) & (bit)) == (bit))
#define ReadOK(file,buffer,len)        fread(buffer, len, 1, file)
#define LM_to_uint(a,b)                (((b)<<8)|(a))
#define PIX2BYTES(n)    (((n)+7)/8)

typedef struct tagGIFSCREEN{
    unsigned int Width;
    unsigned int Height;
    RGB ColorMap [MAXCOLORMAPSIZE];
    unsigned int BitPixel;
    unsigned int ColorResolution;
    unsigned int Background;
    unsigned int AspectRatio;
    int transparent;
    int delayTime;
    int inputFlag;
    int disposal;
} GIFSCREEN;

typedef struct tagIMAGEDESC {
	int Top;
	int Left;
	int Width;
	int Height;
	BOOL haveColorMap;
	int bitPixel;
	int grayScale;
    RGB ColorMap [MAXCOLORMAPSIZE];
	BOOL interlace;
} IMAGEDESC;

static int ZeroDataBlock = 0;

static int bmpComputePitch (int bpp, Uint32 width, Uint32* pitch, BOOL does_round);
static int LWZReadByte(FILE *src, int flag, int input_code_size);
static int GetCode(FILE *src, int code_size, int flag);
static int GetDataBlock(FILE *src, unsigned char *buf);
static int DoExtension(FILE *src, int label, GIFSCREEN* GifScreen);
static int ReadColorMap (FILE *src, int number, RGB* ColorMap);
static int ReadImageDesc (FILE *src, IMAGEDESC* ImageDesc, GIFSCREEN* GifScreen);

static int bmpComputePitch (int bpp, Uint32 width, Uint32* pitch, BOOL does_round)
{
    Uint32 linesize;
    int bytespp = 1;

    if(bpp == 1)
        linesize = PIX2BYTES (width);
    else if(bpp <= 4)
        linesize = PIX2BYTES (width << 2);
    else if (bpp <= 8)
        linesize = width;
    else if(bpp <= 16) {
        linesize = width * 2;
        bytespp = 2;
    } else if(bpp <= 24) {
        linesize = width * 3;
        bytespp = 3;
    } else {
        linesize = width * 4;
        bytespp = 4;
    }

    /* rows are DWORD right aligned*/
    if (does_round)
        *pitch = (linesize + 3) & -4;
    else
        *pitch = linesize;
    return bytespp;
}

static int LWZReadByte (FILE *src, int flag, int input_code_size)
{
    int code, incode;
    register int i;
    static int fresh = FALSE;
    static int code_size, set_code_size;
    static int max_code, max_code_size;
    static int firstcode, oldcode;
    static int clear_code, end_code;
    static int table[2][(1 << MAX_LWZ_BITS)];
    static int stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;

    if (flag) {
        set_code_size = input_code_size;
        code_size = set_code_size + 1;
        clear_code = 1 << set_code_size;
        end_code = clear_code + 1;
        max_code_size = 2 * clear_code;
        max_code = clear_code + 2;

        GetCode(src, 0, TRUE);

        fresh = TRUE;

        for (i = 0; i < clear_code; ++i) {
            table[0][i] = 0;
            table[1][i] = i;
        }
        for (; i < (1 << MAX_LWZ_BITS); ++i)
            table[0][i] = table[1][0] = 0;

        sp = stack;

        return 0;
    } else if (fresh) {
        fresh = FALSE;
        do {
            firstcode = oldcode = GetCode(src, code_size, FALSE);
        } while (firstcode == clear_code);
        return firstcode;
    }
    if (sp > stack)
        return *--sp;

    while ((code = GetCode(src, code_size, FALSE)) >= 0) {
        if (code == clear_code) {
            for (i = 0; i < clear_code; ++i) {
                table[0][i] = 0;
                table[1][i] = i;
            }
            for (; i < (1 << MAX_LWZ_BITS); ++i)
                table[0][i] = table[1][i] = 0;
            code_size = set_code_size + 1;
            max_code_size = 2 * clear_code;
            max_code = clear_code + 2;
            sp = stack;
            firstcode = oldcode = GetCode(src, code_size, FALSE);
            return firstcode;
        } else if (code == end_code) {
            int count;
            unsigned char buf[260];

            if (ZeroDataBlock)
                return -2;

            while ((count = GetDataBlock(src, buf)) > 0);

            if (count != 0) {
                /*
                 * fprintf (stderr,"missing EOD in data stream (common occurence)");
                 */
            }
            return -2;
        }
        incode = code;

        if (code >= max_code) {
            *sp++ = firstcode;
            code = oldcode;
        }
        while (code >= clear_code) {
            *sp++ = table[1][code];
            if (code == table[0][code])
                fprintf (stderr,"load_gif: circular table entry\n");
            code = table[0][code];
        }

        *sp++ = firstcode = table[1][code];

        if ((code = max_code) < (1 << MAX_LWZ_BITS)) {
            table[0][code] = oldcode;
            table[1][code] = firstcode;
            ++max_code;
            if ((max_code >= max_code_size) &&
                (max_code_size < (1 << MAX_LWZ_BITS))) {
                max_code_size *= 2;
                ++code_size;
            }
        }
        oldcode = incode;

        if (sp > stack)
            return *--sp;
    }
    return code;
}


static int GetCode(FILE *src, int code_size, int flag)
{
    static unsigned char buf[280];
    static int curbit, lastbit, done, last_byte;
    int i, j, ret;
    unsigned char count;

    if (flag) {
        curbit = 0;
        lastbit = 0;
        done = FALSE;
        return 0;
    }
    if ((curbit + code_size) >= lastbit) {
        if (done) {
            if (curbit >= lastbit)
#ifdef _DEBUG
                fprintf (stderr,"load_gif: bad decode\n");
#endif
            return -1;
        }
        buf[0] = buf[last_byte - 2];
        buf[1] = buf[last_byte - 1];

        if ((count = GetDataBlock(src, &buf[2])) == 0)
            done = TRUE;

        last_byte = 2 + count;
        curbit = (curbit - lastbit) + 16;
        lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = curbit, j = 0; j < code_size; ++i, ++j)
        ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}


static int GetDataBlock(FILE *src, unsigned char *buf)
{
    unsigned char count;

    if (!ReadOK(src, &count, 1))
        return 0;
    ZeroDataBlock = count == 0;

    if ((count != 0) && (!ReadOK(src, buf, count)))
        return 0;
    return count;
}



static int DoExtension(FILE *src, int label, GIFSCREEN* GifScreen)
{
    static unsigned char buf[256];

    switch (label) {
    case 0x01:                        /* Plain Text Extension */
    	while (GetDataBlock(src, (unsigned char *) buf) != 0);
        break;
    case 0xff:                        /* Application Extension */
    	while (GetDataBlock(src, (unsigned char *) buf) != 0);
        break;
    case 0xfe:                        /* Comment Extension */
        while (GetDataBlock(src, (unsigned char *) buf) != 0);
        return 0;
    case 0xf9:                        /* Graphic Control Extension */
        GetDataBlock(src, (unsigned char *) buf);
        GifScreen->disposal = (buf[0] >> 2) & 0x7;//000 000 0 0 the middle 2 bit is disposal
        GifScreen->inputFlag = (buf[0] >> 1) & 0x1;//000 000 0 0 the secand last bit 
							//is user input flag
        GifScreen->delayTime = LM_to_uint(buf[1], buf[2]);
        if ((buf[0] & 0x1) != 0)// 000 000 0 0 the last bit is transparent flag
            GifScreen->transparent = buf[3];

        while (GetDataBlock(src, (unsigned char *) buf) != 0);
        return 0;
    default:
    	while (GetDataBlock(src, (unsigned char *) buf) != 0);
        break;
    }


    return 0;
}


static int ReadColorMap (FILE *src, int number, RGB* ColorMap)
{
    int i;
    unsigned char rgb[3];

    for (i = 0; i < number; ++i) {
        if (!ReadOK (src, rgb, sizeof(rgb))) {
            return -1;
	    }

        ColorMap [i].r = rgb[0];
        ColorMap [i].g = rgb[1];
        ColorMap [i].b = rgb[2];
    }

    return 0;
}

static int ReadGIFGlobal ( FILE *src, GIFSCREEN* GifScreen)
{
	unsigned char buf[9];
	unsigned char version[4];
    fseek(src, 0L, 0);

    if (!ReadOK (src, buf, 6))
        return -1;                /* not gif image*/
    if (strncmp((char *) buf, "GIF", 3) != 0)
        return -1;
    strncpy (version, (char *) buf + 3, 3);
    version [3] = '\0';

    if (strcmp(version, "87a") != 0 && strcmp(version, "89a") != 0) {
#ifdef _DEBUG
        fprintf (stderr, "load_gif: GIF version number not 87a or 89a\n");
#endif
        return -1;                /* image loading error*/
    }
    GifScreen->transparent = -1;
    GifScreen->delayTime = -1;
    GifScreen->inputFlag = -1;
    GifScreen->disposal = 0;

    if (!ReadOK (src, buf, 7)) {
#ifdef _DEBUG
        fprintf (stderr, "load_gif: bad screen descriptor\n");
#endif
        return -1;                /* image loading error*/
    }
    GifScreen->Width = LM_to_uint (buf[0], buf[1]);
    GifScreen->Height = LM_to_uint (buf[2], buf[3]);
    GifScreen->BitPixel = 2 << (buf[4] & 0x07);
    GifScreen->ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
    GifScreen->Background = buf[5];
    GifScreen->AspectRatio = buf[6];

    if (BitSet(buf[4], LOCALCOLORMAP)) {        /* Global Colormap */
        if (ReadColorMap (src, GifScreen->BitPixel, GifScreen->ColorMap)) {
#ifdef _DEBUG
            fprintf (stderr, "load_gif: bad global colormap\n");
#endif
            return -1;                /* image loading error*/
        }
    }

		return 0;
}

static int ReadImageDesc (FILE *src, IMAGEDESC* ImageDesc, GIFSCREEN* GifScreen)
{
	unsigned char buf[16];
	if (!ReadOK (src, buf, 9)) {
#ifdef _DEBUG
		fprintf (stderr, "load_gif: bad image size\n");
#endif
		return -1;
	}
	ImageDesc->Top= LM_to_uint (buf[0], buf[1]);
	ImageDesc->Left= LM_to_uint (buf[2], buf[3]);
	ImageDesc->Width = LM_to_uint (buf[4], buf[5]);
	ImageDesc->Height = LM_to_uint (buf[6], buf[7]);
	ImageDesc->haveColorMap = BitSet (buf[8], LOCALCOLORMAP);

	ImageDesc->bitPixel = 1 << ((buf[8] & 0x07) + 1);

    ImageDesc->interlace = BitSet(buf[8], INTERLACE);

	if (ImageDesc->haveColorMap) {
		if (ReadColorMap(src, ImageDesc->bitPixel, ImageDesc->ColorMap) < 0) {
#ifdef _DEBUG
			fprintf (stderr, "load_gif: bad local colormap\n");
#endif
			return -1;
		}
	} else {
		memcpy (ImageDesc->ColorMap, GifScreen->ColorMap, MAXCOLORMAPSIZE*3);
	}

	return 0;
}

static int ReadImage (FILE* src, MYBITMAP* bmp, IMAGEDESC* ImageDesc, GIFSCREEN* GifScreen, int ignore)
{

    unsigned char c;
    int v;
    int xpos = 0, ypos = 0, pass = 0;

    /*
     *        initialize the compression routines
     */
    if (!ReadOK(src, &c, 1)) {
        fprintf (stderr,"load_gif: eof on image data\n");
        return -1;
    }
    if (LWZReadByte(src, TRUE, c) < 0) {
        fprintf (stderr,"load_gif: error reading image\n");
        return -1;
    }

    /*
     *        if this is an "uninteresting picture" ignore it.
     */
    if (ignore) {
        while (LWZReadByte(src, FALSE, c) >= 0);
        return -0;
    }
    /*image = imagenewcmap(len, height, cmapsize);*/

    bmp->w = ImageDesc->Width;
    bmp->h = ImageDesc->Height;

    bmp->flags = MYBMP_FLOW_DOWN;
    if (GifScreen->transparent > 0) {
        bmp->flags |= MYBMP_TRANSPARENT;
        bmp->transparent = GifScreen->transparent;
    }
    bmp->frames = 1;
    bmp->depth = 8;
    bmpComputePitch (bmp->depth, bmp->w, &bmp->pitch, TRUE);
    bmp->bits = malloc (bmp->h * bmp->pitch);

    if(!bmp->bits)
        return -1;

    while ((v = LWZReadByte(src, FALSE, c)) >= 0) {
#if 0
		int index = (ypos) * bmp->pitch + (xpos)*3;
		bmp->bits[index + 0 ] = ImageDesc->ColorMap[CM_RED][v]; 
		bmp->bits[index + 1 ] = ImageDesc->ColorMap[CM_GREEN][v];
		bmp->bits[index + 2 ] = ImageDesc->ColorMap[CM_BLUE][v];
#else		
    	 bmp->bits[ypos * bmp->pitch + xpos] = v;
#endif
        ++xpos;
        if (xpos == ImageDesc->Width) {
            xpos = 0;
            if (ImageDesc->interlace) {
                switch (pass) {
                case 0:
                case 1:
                    ypos += 8;
                    break;
                case 2:
                    ypos += 4;
                    break;
                case 3:
                    ypos += 2;
                    break;
                }

                if (ypos >= ImageDesc->Height) {
                    ++pass;
                    switch (pass) {
                    case 1:
                        ypos = 4;
                        break;
                    case 2:
                        ypos = 2;
                        break;
                    case 3:
                        ypos = 1;
                        break;
                    default:
                        goto fini;
                    }
                }
            } else {
                ++ypos;
            }
        }
        if (ypos >= ImageDesc->Height)
            break;
    }

fini:
    return 0;
}


int ReadGIF(FILE *src, GIFBITMAP* gifbmp, HDC hdc)
{
    unsigned char c;
    int ok = 0;
	MYBITMAP mybmp;
	PBITMAP pbmp;
	GIFSCREEN GifScreen;
	IMAGEDESC ImageDesc;
	GIFBITMAPELEMENT* pgifel;

	if ( ReadGIFGlobal ( src, &GifScreen) < 0 ) {
		return -1;
	}

    if ( (ok = ReadOK (src, &c, 1)) == 0) {
#ifdef _debug
        fprintf (stderr, "load_gif: eof on image data\n");
#endif
        return -1;
    }

	while ( c != ';' && ok>0 ){
		switch (c) {
		case '!':
    			if ( (ok = ReadOK (src, &c, 1)) == 0) {
#ifdef _debug
       				 fprintf (stderr, "load_gif: eof on image data\n");
#endif
      				  return -1;
    			}
			DoExtension(src, c, &GifScreen);
			break;
		case ',':
			gifbmp->count ++ ;
			if (ReadImageDesc ( src, &ImageDesc, &GifScreen) < 0) {
				return -1;
			} else {
				if (ReadImage ( src, &mybmp, &ImageDesc, &GifScreen, 0 ) < 0)
					return -1;
			}

			pgifel = (GIFBITMAPELEMENT*) malloc (sizeof (GIFBITMAPELEMENT));
			if (!pgifel)
				return -1;
			else {
				pgifel->prev = pgifel->next = NULL;
				pgifel->top = ImageDesc.Top;
				pgifel->left = ImageDesc.Left;
				pbmp = &(pgifel->Bitmap);
#ifdef _USE_NEWGAL
                if (ExpandMyBitmap (hdc, pbmp, &mybmp, ImageDesc.ColorMap, 0) < 0) {
#else
                if (ExpandMyBitmap (hdc, &mybmp, ImageDesc.ColorMap, pbmp) < 0) {
#endif
                    free (mybmp.bits);
					goto error;
                }
                free (mybmp.bits);
				
				if (gifbmp->first == NULL) {
					gifbmp->first = gifbmp->current = pgifel;
				} else {
					pgifel->prev = gifbmp->current;
					gifbmp->current->next = pgifel;
					gifbmp->current = gifbmp->current->next;
				}
			}
			break;
		default:
			break;
		}

		ok = ReadOK (src, &c, 1);
	
	}
	gifbmp->delaytime = GifScreen.delayTime;
	return 0;

error:
    CleanGIF (gifbmp);
    return -1;
}

void InitGIF (GIFBITMAP* gifbmp)
{
	gifbmp->first = gifbmp->current = NULL;
	
	return;
}

void CleanGIF (GIFBITMAP* gifbmp)
{
	GIFBITMAPELEMENT *tmpel, *pel;
		
	pel = gifbmp->first;
	while (pel) {
		tmpel= pel->next;
		if (pel->Bitmap.bmBits)
			free (pel->Bitmap.bmBits);
		free (pel);

		pel = tmpel;
	}

	return;
}

void RewindGIF (GIFBITMAP* gifbmp)
{
	gifbmp->current = gifbmp->first;
}

PBITMAP GetCurrentGIFBmp (GIFBITMAP* gifbmp)
{
	return &(gifbmp->current->Bitmap);
}

PBITMAP GetNextGIFBmp (GIFBITMAP* gifbmp)
{
	gifbmp->current = gifbmp->current->next;
	return GetCurrentGIFBmp (gifbmp);
}

#endif /* _EXT_FULLGIF */

