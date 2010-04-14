/*
** $Id: ial.h,v 1.21 2003/11/22 03:48:15 weiym Exp $
**
** ial.h: the head file of Input Abstract Layer
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Create data: 2000/06/13
*/

#ifndef GUI_IAL_H
    #define GUI_IAL_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define IAL_MOUSE_LEFTBUTTON    4
#define IAL_MOUSE_MIDDLEBUTTON  2
#define IAL_MOUSE_RIGHTBUTTON   1
#define IAL_MOUSE_FOURTHBUTTON  8
#define IAL_MOUSE_FIFTHBUTTON   16
#define IAL_MOUSE_SIXTHBUTTON   32
#define IAL_MOUSE_RESETBUTTON   64

#define IAL_MOUSEEVENT          1
#define IAL_KEYEVENT            2

typedef struct tagINPUT
{
    char*   id;

    // Initialization and termination
    BOOL (*init_input) (struct tagINPUT *input, const char* mdev, const char* mtype);
    void (*term_input) (void);

    // Mouse operations
    int  (*update_mouse) (void);
    void (*get_mouse_xy) (int* x, int* y);
    void (*set_mouse_xy) (int x, int y);
    int  (*get_mouse_button) (void);
    void (*set_mouse_range) (int minx, int miny, int maxx, int maxy);
    void (*suspend_mouse) (void);
    int (*resume_mouse) (void);

    // Keyboard operations
    int  (*update_keyboard) (void);
    const char* (*get_keyboard_state) (void);
    void (*suspend_keyboard) (void);
    int (*resume_keyboard) (void);
    void (*set_leds) (unsigned int leds);

    // Event
#ifdef _LITE_VERSION
    int (*wait_event) (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
            struct timeval *timeout);
#else
    int (*wait_event) (int which, fd_set *in, fd_set *out, fd_set *except,
            struct timeval *timeout);
#endif

    char mdev [MAX_PATH + 1];
}INPUT;

extern INPUT* cur_input;

#define IAL_InitInput           (*cur_input->init_input)
#define IAL_TermInput           (*cur_input->term_input)
#define IAL_UpdateMouse         (*cur_input->update_mouse)
#define IAL_GetMouseXY          (*cur_input->get_mouse_xy)
#define IAL_GetMouseButton      (*cur_input->get_mouse_button)
#define IAL_SetMouseXY          if (cur_input->set_mouse_xy) (*cur_input->set_mouse_xy)
#define IAL_SetMouseRange       if (cur_input->set_mouse_range) (*cur_input->set_mouse_range)
#define IAL_SuspendMouse        if (cur_input->suspend_mouse) (*cur_input->suspend_mouse)
#define IAL_UpdateKeyboard      (*cur_input->update_keyboard)
#define IAL_GetKeyboardState    (*cur_input->get_keyboard_state)
#define IAL_SuspendKeyboard     if (cur_input->suspend_keyboard) (*cur_input->suspend_keyboard)
#define IAL_SetLeds(leds)       if (cur_input->set_leds) (*cur_input->set_leds) (leds)

static inline int IAL_ResumeMouse (void)
{
    if (cur_input->resume_mouse)
        return cur_input->resume_mouse ();
    else
        return -1;
}

static inline int IAL_ResumeKeyboard (void)
{
    if (cur_input->resume_keyboard)
        return cur_input->resume_keyboard ();
    else
        return -1;
}

#define IAL_WaitEvent           (*cur_input->wait_event)

#define IAL_MType               (cur_input->mtype)
#define IAL_MDev                (cur_input->mdev)

int InitIAL (void);
void TerminateIAL (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_IAL_H */

