/*
** $Id: client.c,v 1.46 2003/09/04 06:02:53 weiym Exp $
** 
** client.c: maintain the clients in server.
** 
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/12/20
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

#include <sys/types.h>
#include <signal.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "internals.h"

#include "ourhdr.h"
#include "sockio.h"
#include "client.h"
#include "server.h"
#include "sharedres.h"
#include "drawsemop.h"

#define    NALLOC       4        /* #Client structs to alloc/realloc for */

extern unsigned int __mg_timer_counter;

MG_Client* mgClients = NULL;
int mgClientSize = 0;

MG_Layer* mgLayers = NULL;
MG_Layer* mgTopmostLayer = NULL;

ON_CHANGE_LAYER OnChangeLayer = NULL;

#define CHANGE_TOPMOST_LAYER(layer)             \
    lock_draw_sem ();                           \
    SHAREDRES_TOPMOST_LAYER = (GHANDLE)layer;   \
    unlock_draw_sem ();
    
#define SET_CLIENT_SCREEN(rc)                   \
    lock_draw_sem ();                           \
    SHAREDRES_CLI_SCR_LX = new_rc.left;         \
    SHAREDRES_CLI_SCR_TY = new_rc.top;          \
    SHAREDRES_CLI_SCR_RX = new_rc.right;        \
    SHAREDRES_CLI_SCR_BY = new_rc.bottom;       \
    unlock_draw_sem ();

static void offset_pointers (MG_Client* new_head, int size, int off)
{
    int i;
    MG_Layer* layer;

    if (off == 0) return;

    for (i = 0; i < size; i++) {

        if (new_head [i].next)
            (char*)new_head [i].next += off;

        if (new_head [i].prev)
            (char*)new_head [i].prev += off;
    }

    layer = mgLayers;
    while (layer) {
        if (layer->cli_head)
            (char*)layer->cli_head += off;
        if (layer->cli_active)
            (char*)layer->cli_active += off;

        layer = layer->next;
    }
}

static BOOL client_alloc (void)        /* alloc more entries in the client[] array */
{
    int i;

    if (mgClients == NULL)
        mgClients = malloc (NALLOC * sizeof(MG_Client));
    else {
        char* new_head = realloc (mgClients, (mgClientSize + NALLOC) * sizeof(MG_Client));

        if (new_head) {
            offset_pointers ((MG_Client*) new_head, mgClientSize, 
                            new_head - (char*)mgClients);
        }

        mgClients = (MG_Client*) new_head;
    }

    if (mgClients == NULL)
        return FALSE;

    /* have to initialize the new entries */
    for (i = mgClientSize; i < mgClientSize + NALLOC; i++) {
        memset (mgClients + i, 0, sizeof (MG_Client));
        mgClients[i].fd        = -1;    /* fd of -1 means entry available */
    }

    mgClientSize += NALLOC;
    return TRUE;
}

/* 
 * Called by IdleHandler4Server() when connection request 
 * from a new client arrives 
 */
int client_add (int fd, pid_t pid, uid_t uid)
{
    int i;

    if (mgClients == NULL)        /* first time we're called */
        if (!client_alloc ())
            return -1;
again:
    for (i = 0; i < mgClientSize; i++) {
        if (mgClients[i].fd == -1) {    /* find an available entry */
            mgClients[i].fd = fd;
            mgClients[i].pid = pid;
            mgClients[i].uid = uid;
            mgClients[i].last_live_time = __mg_timer_counter;
            return (i);    /* return index in client[] array */
        }
    }

    /* client array full, time to realloc for more */
    if (!client_alloc ())
        return -1;

    goto again;        /* and search again (will work this time) */
}

/* Called by IdleHandler4Server() when we're done with a client */
void client_del (int cli)
{
    MG_Client* client;
    MG_Client* deleting = mgClients + cli;
    MG_Layer* layer = deleting->layer;

#ifdef _DEBUG
    printf ("Remove a client: %s\n", deleting->name);
#endif

    if (layer == NULL)
        goto ret;

    if (deleting->global_res)
        release_global_res (cli);

    if (deleting->next)
        deleting->next->prev = deleting->prev;
    if (deleting->prev) 
        deleting->prev->next = deleting->next;

    if (deleting == layer->cli_head) {
        layer->cli_head = deleting->next;
        if (layer->cli_head == NULL) {
            if (remove_layer (layer)) {
                SendMessage (HWND_DESKTOP, MSG_TOPMOSTCHANGED, 0, 0);
                PostMessage (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);
            }
            goto ret;
        }
    }

    if (deleting == layer->cli_active) {
        layer->cli_active = deleting->next;
        if (layer->cli_active == NULL) {
            layer->cli_active = layer->cli_head;
        }
    }

    // Recalc spare rects of this layer
    SetClipRgn (layer->spare_rects, SHAREDRES_LAYER_RC);
    client = layer->cli_head;
    while (client) {
        SubtractClipRect (layer->spare_rects, &client->rc);
        client = client->next;
    }

    if (layer == mgTopmostLayer) {
        // erase server area here
        SendMessage (HWND_DESKTOP, MSG_DELCLIENT, (WPARAM)cli, (LPARAM)(&deleting->rc));
        SendMessage (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, (LPARAM)(&deleting->rc));
    }

ret:
    memset (deleting, 0, sizeof (MG_Client));
    deleting->fd = -1;
}

int my_send2client (MSG* msg, MG_Client* client)
{
    int ret;

    if (__mg_timer_counter < (client->last_live_time + THRES_LIVE)) {
        ret = sock_write_t (client->fd, msg, sizeof (MSG), TO_SOCKIO);

        switch (ret) {
        case SOCKERR_TIMEOUT:
            client->last_live_time = 0;
            break;
        case SOCKERR_CLOSED:
            {
                int cli = client - mgClients;
                if (OnNewDelClient) OnNewDelClient (LCO_DEL_CLIENT, cli);
                remove_client (cli, client->fd);
                break;
            }
        }

        return ret;
    }
    else
        return SOCKERR_OK;
}

MG_Layer* get_layer (const char* layer_name, BOOL does_create)
{
    MG_Layer* layer = mgLayers;
    MG_Layer* new_layer;

    while (layer) {
        if (strncmp (layer->name, layer_name, LEN_LAYER_NAME) == 0)
            return layer;

        layer = layer->next;
    }

    if (!does_create)
        return NULL;

    if (!(new_layer = malloc (sizeof (MG_Layer))))
        return NULL;

    if (!(new_layer->spare_rects = malloc (sizeof (CLIPRGN)))) {
        free (new_layer);
        return NULL;
    }

#ifdef _DEBUG
    printf ("Create a new layer: %s\n", layer_name);
#endif

    strcpy (new_layer->name, layer_name);
    new_layer->cli_head = NULL;
    new_layer->cli_active = NULL;

    // chain it
    if (mgLayers)
        mgLayers->prev = new_layer;
    new_layer->prev = NULL;
    new_layer->next = mgLayers;
    mgLayers = new_layer;

    // Notify that a new layer created.
    if (OnChangeLayer) OnChangeLayer (LCO_NEW_LAYER, new_layer, NULL);

    mgTopmostLayer = mgLayers;

    // Topmost layer changed
    lock_draw_sem ();
    SHAREDRES_TOPMOST_LAYER = (GHANDLE)mgTopmostLayer;
    unlock_draw_sem ();

    SendMessage (HWND_DESKTOP, MSG_TOPMOSTCHANGED, 0, 0);
    PostMessage (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);

    InitClipRgn (new_layer->spare_rects, &__mg_free_spare_rect_list);
    SetClipRgn (new_layer->spare_rects, SHAREDRES_LAYER_RC);

    // Notify that a new topmost layer have been set.
    if (OnChangeLayer) OnChangeLayer (LCO_TOPMOST_CHANGED, mgTopmostLayer, NULL);

    return new_layer;
}

void get_layer_info (int cli, const char* layer_name, LAYERINFO* info)
{
    MG_Layer* layer;
    MG_Client* client;
    PCLIPRECT spare, max = NULL;
    unsigned int max_area = 0, area;

    memset (info, 0, sizeof (LAYERINFO));
    info->cli_active = -1;

    if (layer_name [0])
        layer = get_layer (layer_name, FALSE);
    else
        layer = mgClients[cli].layer;

    info->handle = (GHANDLE)layer;
    if (info->handle) {
        spare = layer->spare_rects->head;
        while (spare) {
            area = (RECTW (spare->rc) * RECTH (spare->rc));
            if (area > max_area) {
                max_area = area;
                max = spare;
            }

            spare = spare->next;
        }

        client = layer->cli_head;
        while (client) {
            info->nr_clients ++;

            client = client->next;
        }

        if (max)
            info->max_rect = max->rc;
        if (mgTopmostLayer == layer)
            info->is_topmost = TRUE;
        info->cli_active = layer->cli_active - mgClients;
    }
}

BOOL remove_layer (MG_Layer* layer)
{
#ifdef _DEBUG
    printf ("Remove a layer: %s\n", layer->name);
#endif

    if (layer->prev)
        layer->prev->next = layer->next;
    if (layer->next)
        layer->next->prev = layer->prev;

    if (layer == mgLayers) {
        mgLayers = layer->next;
    }

    // Notify that a layer will be deleted.
    if (OnChangeLayer) OnChangeLayer (LCO_DEL_LAYER, layer, NULL);

    EmptyClipRgn (layer->spare_rects);
    free (layer->spare_rects);
    free (layer);

    if (mgTopmostLayer != layer)
        return FALSE;

    // set new topmost layer
    mgTopmostLayer = mgLayers;
    CHANGE_TOPMOST_LAYER (mgTopmostLayer);
    if (mgTopmostLayer) {
        MG_Client* client;
        MSG msg = {HWND_DESKTOP, MSG_PAINT, 0, 0, __mg_timer_counter};

#ifdef _DEBUG
        printf ("New topmost layer: %s\n", mgTopmostLayer->name);
#endif

        client = mgTopmostLayer->cli_head;
        while (client) {
            my_send2client (&msg, client);
            client->last_live_time = __mg_timer_counter;

            client = client->next;
        }

        if (mgTopmostLayer->cli_active) {
            MSG msg = {HWND_DESKTOP, MSG_SETFOCUS, 0, 0, __mg_timer_counter};

            my_send2client (&msg, mgTopmostLayer->cli_active);
        }
    }


    // Notify that a new topmost layer have been set.
    if (OnChangeLayer) OnChangeLayer (LCO_TOPMOST_CHANGED, mgTopmostLayer, NULL);

#ifdef _DEBUG
    printf ("layer removed\n");
#endif
    return TRUE;
}

void set_active_client (MG_Client* client)
{
    MG_Layer* layer;

    if (client)
        layer = client->layer;
    else
        layer = mgTopmostLayer;

    if (layer == NULL || layer->cli_active == client)
        return;

    if (layer->cli_active) {
        MSG msg = {HWND_DESKTOP, MSG_SETFOCUS, 0, 0, __mg_timer_counter};

        my_send2client (&msg, layer->cli_active);
    }

    // Notify that active client changed.
    if (OnChangeLayer) OnChangeLayer (LCO_ACTIVE_CHANGED, layer, client);
    layer->cli_active = client;
}

void client_join_layer (int cli, const JOINLAYERINFO* info, JOINEDCLIENTINFO* joined_info)
{
    PCLIPRECT spare, max = NULL;
    RECT tmp;
    unsigned int max_area = 0, area;
    MG_Layer* layer = (MG_Layer*)(joined_info->layer_handle);
    MG_Client* new_client = mgClients + cli;

#ifdef _DEBUG
    printf ("Join a client (%s) to layer %s\n", info->client_name, layer->name);
#endif

    strcpy (new_client->name, info->client_name);
    SetRectEmpty (&joined_info->desktop_rc);
    
    spare = layer->spare_rects->head;
    while (spare) {
        if (IntersectRect (&tmp, &info->desktop_rc, &spare->rc)) {
            area = (RECTW (tmp) * RECTH (tmp));
            if (area > max_area) {
                max_area = area;
                max = spare;
            }
        }

        spare = spare->next;
    }

    if (max) {
        IntersectRect (&joined_info->desktop_rc, &info->desktop_rc, &max->rc);
        SubtractClipRect (layer->spare_rects, &joined_info->desktop_rc);
        new_client->layer = layer;

        // Notify that a new client joined to this layer.
        if (OnChangeLayer) OnChangeLayer (LCO_JOIN_CLIENT, layer, new_client);
    }
    else {
        new_client->layer = NULL;
        if (layer->cli_head == NULL)
            remove_layer (layer);
        return;
    }

    new_client->prev = NULL;
    new_client->next = layer->cli_head;
    if (layer->cli_head)
        layer->cli_head->prev = new_client;
    layer->cli_head = new_client;

    set_active_client (new_client);

    // set rect of new client
    new_client->rc = joined_info->desktop_rc;

#ifdef _DEBUG
    printf ("Rect of client (%s): (%d, %d, %d, %d)\n", 
                    new_client->name,
                    new_client->rc.left, new_client->rc.top,
                    new_client->rc.right, new_client->rc.bottom);
#endif

    if (layer == mgTopmostLayer) {
        // tell desktop to exclude this client from background
        SendMessage (HWND_DESKTOP, MSG_NEWCLIENT, (WPARAM)cli, (LPARAM)(&new_client->rc));
    }
}

BOOL is_valid_layer (MG_Layer* layer)
{
    MG_Layer* myLayer;

    myLayer = mgLayers;
    while (myLayer) {
        if (layer == myLayer)
            return TRUE;

        myLayer = myLayer->next;
    }

    return FALSE;
}

/* send message to client(s) */
int GUIAPI Send2Client (MSG* msg, int cli)
{
	
    int i, n;
    MG_Client* client;

    if (cli >= 0 && cli < mgClientSize && mgClients [cli].fd != -1) {
        return my_send2client (msg, mgClients + cli);
    }
    else if (cli == CLIENT_ACTIVE) {   /* send to active client */
        if (mgTopmostLayer && mgTopmostLayer->cli_active) {
            return my_send2client (msg, mgTopmostLayer->cli_active);
        }
    }
    else if (cli == CLIENTS_TOPMOST) {   /* send to topmost clients */
        if (mgTopmostLayer) {
            client = mgTopmostLayer->cli_head;
            while (client) {
                if ((n = my_send2client (msg, client)) < 0)
                    return n;
                client = client->next;
            }
        }
    }
    else if (cli == CLIENTS_EXCEPT_TOPMOST) {   /* send to all clients except topmost client */
        client = mgClients;
        for (i = 0; i < mgClientSize; i++, client++) {
            if (client->layer != mgTopmostLayer) {
                if ((n = my_send2client (msg, client)) < 0)
                    return n;
            }
        }
    }
    else if (cli == CLIENTS_ALL) {    /* send to all clients */
        client = mgClients;
        for (i = 0; i < mgClientSize; i++, client++) {
            if ((n = my_send2client (msg, client)) < 0)
                return n;
        }
    }
    else
        return SOCKERR_INVARG;

    return SOCKERR_OK;
}

BOOL GUIAPI Send2TopMostClients (int iMsg, WPARAM wParam, LPARAM lParam)
{
    MSG msg = {HWND_DESKTOP, iMsg, wParam, lParam, __mg_timer_counter};

    if (send2client (&msg, CLIENTS_TOPMOST) < 0)
        return FALSE;
    return TRUE;
}

BOOL GUIAPI Send2ActiveClient (int iMsg, WPARAM wParam, LPARAM lParam)
{
    MSG msg = {HWND_DESKTOP, iMsg, wParam, lParam, __mg_timer_counter};

    if (send2client (&msg, CLIENT_ACTIVE) < 0)
        return FALSE;
    return TRUE;
}

BOOL GUIAPI SetClientScreen (int lx, int ty, int rx, int by)
{
    MG_Client* client;
    MSG msg = {HWND_DESKTOP, MSG_PAINT, 0, 0, __mg_timer_counter};
    RECT new_rc = {lx, ty, rx, by};
    RECT old_rc;

    if (!IntersectRect (&new_rc, &new_rc, &g_rcScr))
        return FALSE;

    old_rc.left = SHAREDRES_CLI_SCR_LX;
    old_rc.top = SHAREDRES_CLI_SCR_TY;
    old_rc.right = SHAREDRES_CLI_SCR_RX;
    old_rc.bottom = SHAREDRES_CLI_SCR_BY;

    SET_CLIENT_SCREEN (new_rc);

    if (mgTopmostLayer == NULL 
            || (client = mgTopmostLayer->cli_head) == NULL)
        return TRUE;

    while (client) {
        if (!IsCovered (&new_rc, &old_rc)) 
            my_send2client (&msg, client);
        client = client->next;
    }

    return TRUE;
}

static int onlyme_count = 0;

BOOL GUIAPI OnlyMeCanDraw (void)
{
    MG_Client* client;

    if (onlyme_count ++)
        return TRUE;

    CHANGE_TOPMOST_LAYER (NULL);

    if (mgTopmostLayer == NULL 
            || (client = mgTopmostLayer->cli_head) == NULL)
        return FALSE;

    return TRUE;
}

BOOL GUIAPI ClientCanDrawNowEx (BOOL bRepaint, const RECT* invrc)
{
    MG_Client* client;
    MSG msg = {HWND_DESKTOP, MSG_PAINT, 0, 0, __mg_timer_counter};

    if (--onlyme_count)
        return TRUE;

    if (bRepaint && invrc) {
        msg.wParam = MAKELONG (invrc->left, invrc->top);
        msg.lParam = MAKELONG (invrc->right, invrc->bottom);
    }

    CHANGE_TOPMOST_LAYER (mgTopmostLayer);

    if (mgTopmostLayer == NULL 
            || (client = mgTopmostLayer->cli_head) == NULL)
        return FALSE;

    while (client) {
        if (bRepaint) {
            my_send2client (&msg, client);
        }
        client = client->next;
    }

    return TRUE;
}

BOOL GUIAPI SetTopMostLayer (MG_Layer* layer)
{
    MG_Client* client;
    MSG msg = {HWND_DESKTOP, MSG_PAINT, 0, 0, __mg_timer_counter};

    if (layer == mgTopmostLayer)
        return FALSE;

    mgTopmostLayer = layer;
    CHANGE_TOPMOST_LAYER (layer);

    client = layer->cli_head;
    while (client) {
        my_send2client (&msg, client);
        client->last_live_time = __mg_timer_counter;

        client = client->next;
    }

    if (layer->cli_active) {
        MSG msg = {HWND_DESKTOP, MSG_SETFOCUS, 0, 0, __mg_timer_counter};

        my_send2client (&msg, layer->cli_active);
    }

    SendMessage (HWND_DESKTOP, MSG_TOPMOSTCHANGED, 0, 0);
    PostMessage (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);

    // Notify that a new topmost layer have been set.
    if (OnChangeLayer) OnChangeLayer (LCO_TOPMOST_CHANGED, mgTopmostLayer, NULL);

    return TRUE;
}

BOOL GUIAPI SetTopMostClient (int cli)
{
    if (cli < 0 || cli >= mgClientSize || mgClients [cli].fd == -1)
        return FALSE;
                 
    return SetTopMostLayer (mgClients [cli].layer);
}

int GUIAPI GetClientByPID (int pid)
{
    int i;

    for (i = 0; i < mgClientSize; i++) {
        if (mgClients[i].pid == pid) {
            if (mgClients[i].fd == -1)
                return -1;
            return i;
        }
    }

    return -1;
}

