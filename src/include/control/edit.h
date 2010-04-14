/*
** $Id: edit.h,v 1.18 2003/09/04 03:40:35 weiym Exp $
**
** edit.h: the head file of Edit Control module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Create date: 1999/8/26
*/

#ifndef GUI_EDIT_H_
#define GUI_EDIT_H_

#ifdef  __cplusplus
extern  "C" {
#endif

#ifdef _FLAT_WINDOW_STYLE
#define WIDTH_EDIT_BORDER       1
#else
#define WIDTH_EDIT_BORDER       2
#endif

#define MARGIN_EDIT_LEFT        1
#define MARGIN_EDIT_TOP         1
#define MARGIN_EDIT_RIGHT       2
#define MARGIN_EDIT_BOTTOM      1

#define LEN_SLEDIT_BUFFER       1024
#define LEN_SLEDIT_UNDOBUFFER   256

#define EST_FOCUSED     0x00000001L
#define EST_MODIFY      0x00000002L
#define EST_REPLACE     0x00000008L

#define EDIT_OP_NONE    0x00
#define EDIT_OP_DELETE  0x01
#define EDIT_OP_INSERT  0x02
#define EDIT_OP_REPLACE 0x03

typedef struct tagSLEDITDATA
{
    DWORD   status;         // status of box

    int     dataEnd;        // data end position
    int     editPos;        // current edit position
    int     startPos;       // start display position
                            
    int     bufferLen;      // length of buffer
    char*   buffer;         // buffer

    int*    pos_chars;	    // postion of each valid unit in buffer
    int*    dx_chars;	    // display postion of each valid unit

    int     nr_chars;	    // number of all units
    SIZE    extent;
    
    int     passwdChar;     // password character
    
    int     leftMargin;     // left margin
    int     topMargin;      // top margin
    int     rightMargin;    // right margin
    int     bottomMargin;   // bottom margin
    
    int     hardLimit;      // hard limit

#if 0
    int     selStart;       // selection start position
    int     selEnd;         // selection end position
    
    int     lastOp;         // last operation
    int     lastPos;        // last operation position
    int     affectedLen;    // affected len of last operation
    int     undoBufferLen;  // undo buffer len
    char    undoBuffer [LEN_SLEDIT_UNDOBUFFER];
                            // Undo buffer;
#endif

}SLEDITDATA;
typedef SLEDITDATA* PSLEDITDATA;
    
BOOL RegisterSLEditControl (void);
void SLEditControlCleanup (void);

#ifdef __cplusplus
}
#endif

#endif /* GUI_EDIT_H_ */

