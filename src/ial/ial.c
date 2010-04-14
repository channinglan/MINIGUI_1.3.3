/*
** $Id: ial.c,v 1.46 2003/12/02 02:18:21 tangxf Exp $
** 
** The Input Abstract Layer of MiniGUI.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000, 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/06/13
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
#include <string.h>
#include <sys/time.h>
#ifdef _SVGALIB
    #include <vga.h>
    #include <vgamouse.h>
    #include <vgakeyboard.h>
#endif
#ifdef _LIBGGI
    #include <ggi/ggi.h>
#endif

#include "common.h"
#include "minigui.h"
#include "misc.h"
#include "ial.h"
#ifdef _NATIVE_IAL_ENGINE
#include "native/native.h"
#endif

#ifdef _SVGALIB
    #include "svgalib.h"
#endif
#ifdef _LIBGGI
    #include "libggi.h"
#endif
#ifdef _EP7211_IAL
    #include "ep7211.h"
#endif
#ifdef _ADS_IAL
    #include "ads.h"
#endif
#ifdef _IPAQ_IAL
    #include "ipaq.h"
#endif
#ifdef _VR4181_IAL
    #include "vr4181.h"
#endif
#ifdef _HELIO_IAL
    #include "helio.h"
#endif
#ifdef _TFSTB_IAL
    #include "tf-stb.h"
#endif
#ifdef _T800_IAL
    #include "t800.h"
#endif
#ifdef _DUMMY_IAL
    #include "dummy.h"
#endif
#ifdef _QVFB_IAL
    #include "qvfb.h"
#endif
#ifdef _MPC823_IAL
    #include "mpc823.h"
#endif
#ifdef _PX255B_IAL
    #include "px255b.h"
#endif
#ifdef _MC68X328_IAL
    #include "mc68x328.h"
#endif
#ifdef _SMDK2410_IAL
    #include "2410.h"
#endif


#define LEN_ENGINE_NAME        16
#define LEN_MTYPE_NAME         16
#define LEN_IALENGINE_NAME     16

static INPUT inputs [] = 
{
#ifdef _DUMMY_IAL
    {"Dummy", InitDummyInput, TermDummyInput},
#endif
#ifdef _QVFB_IAL
    {"QVFB", InitQVFBInput, TermQVFBInput},
#endif
#ifdef _SVGALIB
    {"SVGALib", InitSVGALibInput, TermSVGALibInput},
#endif
#ifdef _LIBGGI
    {"LibGGI", InitLibGGIInput, TermLibGGIInput},
#endif
#ifdef _EP7211_IAL
    {"EP7211", InitEP7211Input, TermEP7211Input},
#endif
#ifdef _ADS_IAL
    {"ADS", InitADSInput, TermADSInput},
#endif
#ifdef _IPAQ_IAL
    {"iPAQ", InitIPAQInput, TermIPAQInput},
#endif
#ifdef _VR4181_IAL
    {"VR4181", InitVR4181Input, TermVR4181Input},
#endif
#ifdef _HELIO_IAL
    {"Helio", InitHelioInput, TermHelioInput},
#endif
#ifdef _NATIVE_IAL_ENGINE
    {"Console", InitNativeInput, TermNativeInput},
#endif
#ifdef _TFSTB_IAL
    {"TF-STB", InitTFSTBInput, TermTFSTBInput},
#endif
#ifdef _T800_IAL
    {"T800", InitT800Input, TermT800Input},
#endif
#ifdef _MPC823_IAL
    {"MPC823", InitMPC823Input, TermMPC823Input},
#endif
#ifdef _PX255B_IAL
    {"PX255B", InitPX255BInput, TermPX255BInput},
#endif

#ifdef _MC68X328_IAL
    {"MC68X328", InitMC68X328Input, TermMC68X328Input},
#endif
#ifdef _SMDK2410_IAL
//    {"SMDK2410", Init2410Input, Term2410Input},
    {"soclets", Init2410Input, Term2410Input},
#endif
};

INPUT* cur_input;

#define NR_INPUTS  (sizeof (inputs) / sizeof (INPUT))

int InitIAL (void)
{
    int  i;

    char buff [LEN_ENGINE_NAME + 1];
    char mdev [MAX_PATH + 1];
    char mtype[LEN_MTYPE_NAME + 1];

    if (NR_INPUTS == 0)
        return ERR_NO_ENGINE;

    if (GetMgEtcValue ("system", "ial_engine", buff, LEN_ENGINE_NAME) < 0)
        return ERR_CONFIG_FILE;

    if (GetMgEtcValue ("system", "mdev", mdev, MAX_PATH) < 0)
        return ERR_CONFIG_FILE;

    if (GetMgEtcValue ("system", "mtype", mtype, LEN_MTYPE_NAME) < 0)
        return ERR_CONFIG_FILE;

    for (i = 0; i < NR_INPUTS; i++) {
        if (strncasecmp (buff, inputs[i].id, LEN_IALENGINE_NAME) == 0) {
            cur_input = inputs + i;
            break;
        }
    }
   
//#ifdef _DEBUG
    fprintf (stderr, "buff: Use %s .\n", buff);
    fprintf (stderr, "mdev: Use %s .\n", mdev);
    fprintf (stderr, "mtype: Use %s .\n", mtype);
//#endif
    if (cur_input == NULL) {
        fprintf (stderr, "IAL: Does not find matched engine.\n");
        if (NR_INPUTS) {
            cur_input = inputs;
            fprintf (stderr, "IAL: Use the first engine: %s\n", cur_input->id);
        }
        else
            return ERR_NO_MATCH;
    }

    strcpy (cur_input->mdev, mdev);

    if (!IAL_InitInput (cur_input, mdev, mtype)) {
        fprintf (stderr, "IAL: Init IAL engine failure.\n");
        return ERR_INPUT_ENGINE;
    }

#ifdef _DEBUG
    fprintf (stderr, "IAL: Use %s engine.\n", cur_input->id);
#endif

    return 0;
}

void TerminateIAL (void)
{
    IAL_TermInput ();
}

