/*
** $Id: request.c,v 1.17 2003/09/04 06:02:53 weiym Exp $
** 
** request.c: handle request of clients.
** 
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/12/21
**
** NOTE: The idea comes from sample code in APUE.
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

#include <signal.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "internals.h"
#include "cursor.h"
#include "gal.h"

#include "ourhdr.h"
#include "sockio.h"
#include "client.h"
#include "server.h"
#include "sharedres.h"

typedef void (* ReleaseProc) (void* );

struct GlobalRes
{
    struct GlobalRes* next;
    struct GlobalRes* prev;

    void* key;
    void* res;
    ReleaseProc release_proc;
};

static void add_global_res (int cli, void* key, void* res, ReleaseProc release_proc)
{
    MG_Client* client = mgClients + cli;
    struct GlobalRes *global_res = malloc (sizeof (struct GlobalRes));

    if (global_res) {
	global_res->key = key;
	global_res->res = res;
	global_res->release_proc = release_proc;

	if (client->global_res == NULL) {
            global_res->next = NULL;
	    global_res->prev = NULL;
            client->global_res = global_res;
	}
	else {
            global_res->next = client->global_res;
	    global_res->prev = NULL;
	    global_res->next->prev = global_res;
            client->global_res = global_res;
	}
    }
}

static void del_global_res (int cli, void* key)
{
    MG_Client* client = mgClients + cli;
    struct GlobalRes *global_res = client->global_res, *next;

    while (global_res) {
	next = global_res->next;

        if (global_res->key == key) {
            global_res->release_proc (global_res->res);

            if (global_res->prev)
                global_res->prev->next = global_res->next;
            if (global_res->next)
                global_res->next->prev = global_res->prev;

            if (global_res == client->global_res) {
                client->global_res = global_res->next;
            }

	    free (global_res);
	}

	global_res = next;
    }
}

void release_global_res (int cli)
{
    MG_Client* client = mgClients + cli;
    struct GlobalRes *global_res = client->global_res, *next;

    while (global_res) {
        next = global_res->next;

        global_res->release_proc (global_res->res);
        free (global_res);

        global_res = next;
    }
}

int send_reply (int clifd, const void* reply, int len)
{
    MSG reply_msg = {HWND_INVALID, 0};

    /* send a reply message to indicate this is a reply of request */
    if (sock_write (clifd, &reply_msg, sizeof (MSG)) < 0)
        return SOCKERR_IO;

    if (sock_write (clifd, reply, len) < 0)
        return SOCKERR_IO;

    return SOCKERR_OK;
}

static int load_cursor (int cli, int clifd, void* buff, size_t len)
{
    HCURSOR hcsr;

    hcsr = LoadCursorFromFile (buff);

#ifdef _CURSOR_SUPPORT
    if (hcsr) {
        add_global_res (cli, (void*) hcsr, (void*)hcsr, (ReleaseProc)DestroyCursor);
    }
#endif
    return send_reply (clifd, &hcsr, sizeof (HCURSOR));
}

static int create_cursor (int cli, int clifd, void* buff, size_t len)
{
    HCURSOR hcsr;
    int* tmp;
    BYTE* and_bits, *xor_bits;

    tmp = (int*)buff;
    and_bits = (BYTE*)(tmp + 6);
    xor_bits = and_bits + tmp [5];

    hcsr = CreateCursor (tmp [0], tmp [1], tmp [2], tmp [3], 
                    and_bits, xor_bits, tmp [4]);

#ifdef _CURSOR_SUPPORT
    if (hcsr) {
        add_global_res (cli, (void*) hcsr, (void*)hcsr, (ReleaseProc)DestroyCursor);
    }
#endif
    return send_reply (clifd, &hcsr, sizeof (HCURSOR));
}

static int destroy_cursor (int cli, int clifd, void* buff, size_t len)
{
    BOOL ret_value;
    HCURSOR hcsr;

    hcsr = *((HCURSOR*)buff);

    ret_value = DestroyCursor (hcsr);

    del_global_res (cli, (void*)hcsr);

    return send_reply (clifd, &ret_value, sizeof (BOOL));
}

static int clip_cursor (int cli, int clifd, void* buff, size_t len)
{
    RECT cliprc;

    memcpy (&cliprc, buff, sizeof (RECT));
    ClipCursor (&cliprc);

    return SOCKERR_OK;
}

static int get_clip_cursor (int cli, int clifd, void* buff, size_t len)
{
    RECT cliprc;

    GetClipCursor (&cliprc);

    return send_reply (clifd, &cliprc, sizeof (RECT));
}

static int set_cursor (int cli, int clifd, void* buff, size_t len)
{
    HCURSOR hcsr;
    HCURSOR old;

    memcpy (&hcsr, buff, sizeof (HCURSOR));

    old = SetCursorEx (hcsr, FALSE);

    return send_reply (clifd, &old, sizeof (HCURSOR));
}

static int get_current_cursor (int cli, int clifd, void* buff, size_t len)
{
    HCURSOR hcsr;

    hcsr = GetCurrentCursor ();

    return send_reply (clifd, &hcsr, sizeof (HCURSOR));
}

#if 0
static int show_cursor_for_gdi (int cli, int clifd, void* buff, size_t len)
{
    RECT rc;
    BOOL show_hide;
    BOOL ret_value = TRUE;

    memcpy (&show_hide, buff, sizeof (BOOL));
    memcpy (&rc, buff + sizeof (BOOL), sizeof (RECT));

    ShowCursorForClientGDI (show_hide, &rc);

    return send_reply (clifd, &ret_value, sizeof (BOOL));
}
#endif

static int show_cursor (int cli, int clifd, void* buff, size_t len)
{
    BOOL show_hide;
    int ret_value;

    memcpy (&show_hide, buff, sizeof (BOOL));
    ret_value = ShowCursor (show_hide);

    return send_reply (clifd, &ret_value, sizeof (BOOL));
}

static int set_cursor_pos (int cli, int clifd, void* buff, size_t len)
{
    POINT pt;

    memcpy (&pt, buff, sizeof (POINT));

    SetCursorPos (pt.x, pt.y);

    return SOCKERR_OK;
}

static int layer_info (int cli, int clifd, void* buff, size_t len)
{
    LAYERINFO info;

    get_layer_info (cli, (const char*) buff, &info);

    return send_reply (clifd, &info, sizeof (LAYERINFO));
}

static int join_layer (int cli, int clifd, void* buff, size_t len)
{
    static int nr_layers = 0, nr_clients = 0;
    JOINLAYERINFO* info;
    JOINEDCLIENTINFO joined_info = {INV_LAYER_HANDLE};
    MG_Layer* layer;

    info = (JOINLAYERINFO*) buff;
    
    if (info->layer_name [0] == '\0') {
        sprintf (info->layer_name, "Layer-%d", nr_layers);
        nr_layers ++;
    }
    if (info->client_name [0] == '\0') {
        sprintf (info->client_name, "Client-%d", nr_clients);
        nr_clients ++;
    }

    if (!(layer = get_layer (info->layer_name, TRUE))) {
        return send_reply (clifd, &joined_info, sizeof (JOINEDCLIENTINFO));
    }

    joined_info.layer_handle = (GHANDLE)layer;
    client_join_layer (cli, info, &joined_info);

    return send_reply (clifd, &joined_info, sizeof (JOINEDCLIENTINFO));
}

static int topmost_layer (int cli, int clifd, void* buff, size_t len)
{
    BOOL ret_value = FALSE;
    MG_Layer* layer = *((MG_Layer**)buff);

    if (is_valid_layer (layer) && layer->cli_head) {
        SetTopMostLayer (layer);
        ret_value = TRUE;
    }

    return send_reply (clifd, &ret_value, sizeof (BOOL));
}

static int active_client (int cli, int clifd, void* buff, size_t len)
{
    BOOL ret_value = FALSE;
    int active = *((int*)buff);

    if (active >= 0 && active < mgClientSize && mgClients [active].fd != -1) {
        set_active_client (mgClients + active);
        ret_value = TRUE;
    }

    return send_reply (clifd, &ret_value, sizeof (BOOL));
}

static int im_live (int cli, int clifd, void* buff, size_t len)
{
    unsigned int time;

    memcpy (&time, buff, sizeof (unsigned int));
    mgClients [cli].last_live_time = time;

    return SOCKERR_OK;
}

extern HWND __mg_ime_wnd;
static int open_ime_wnd (int cli, int clifd, void* buff, size_t len)
{
    BOOL open;

    memcpy (&open, buff, sizeof (BOOL));
    if (__mg_ime_wnd) {
        if (open)
            SendNotifyMessage (__mg_ime_wnd, MSG_IME_OPEN, 0, 0);
        else
            SendNotifyMessage (__mg_ime_wnd, MSG_IME_CLOSE, 0, 0);
    }

    return SOCKERR_OK;
}

#ifdef _USE_NEWGAL

void release_HWS (REQ_HWSURFACE* allocated)
{
    REP_HWSURFACE reply;

    GAL_RequestHWSurface (allocated, &reply);

    free (allocated);
}

static int req_hw_surface (int cli, int clifd, void* buff, size_t len)
{
    REQ_HWSURFACE* request = (REQ_HWSURFACE*) buff;
    REP_HWSURFACE reply;

    GAL_RequestHWSurface (request, &reply);

    if (request->bucket == NULL) {
        REQ_HWSURFACE* allocated;
        allocated = malloc (sizeof (REQ_HWSURFACE));
        if (allocated) {
            allocated->w = request->w;
            allocated->h = request->h;
            allocated->pitch = reply.pitch;
            allocated->offset = reply.offset;
            allocated->bucket = reply.bucket;

            add_global_res (cli, allocated->bucket, allocated, (ReleaseProc)release_HWS);
        }
    }
    else {
       del_global_res (cli, request->bucket);
    }

    return send_reply (clifd, &reply, sizeof (REP_HWSURFACE));
}
#endif

static REQ_HANDLER handlers [MAX_REQID] =
{
    load_cursor,
    create_cursor,
    destroy_cursor,
    clip_cursor,
    get_clip_cursor,
    set_cursor,
    get_current_cursor,
#if 0
    show_cursor_for_gdi,
#endif
    show_cursor,
    set_cursor_pos,
    layer_info,
    join_layer,
    topmost_layer,
    active_client,
    im_live,
    open_ime_wnd,
#ifdef _USE_NEWGAL
    req_hw_surface,
#endif
};

BOOL GUIAPI RegisterRequestHandler (int req_id, REQ_HANDLER your_handler)
{
    if (req_id <= MAX_SYS_REQID || req_id > MAX_REQID)
        return FALSE;

    handlers [req_id - 1] = your_handler;
    return TRUE;
}

REQ_HANDLER GUIAPI GetRequestHandler (int req_id)
{
    if (req_id <= 0 || req_id > MAX_REQID)
        return NULL;

    return handlers [req_id - 1];
}

int handle_request (int clifd, int req_id, int cli)
{
    int n;
    unsigned char buff [MAXDATALEN];
    size_t len_data;

    if ((n = sock_read (clifd, &len_data, sizeof (size_t))) == SOCKERR_IO)
        return SOCKERR_IO;
    else if (n == SOCKERR_CLOSED) {
        goto error;
    }

    if ((n = sock_read (clifd, buff, len_data)) == SOCKERR_IO)
        return SOCKERR_IO;
    else if (n == SOCKERR_CLOSED) {
        goto error;
    }

    if (req_id > MAX_REQID || req_id <= 0 || handlers [req_id - 1] == NULL)
        return SOCKERR_INVARG;

    n = handlers [req_id - 1] (cli, clifd, buff, len_data);

    if (n == SOCKERR_IO)
        goto error;

    return n;

error:
    remove_client (cli, clifd);
    return SOCKERR_CLOSED;
}


