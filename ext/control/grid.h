#ifndef _GRID_H
#define _GRID_H

#ifdef __cplusplus
extern "C"
{
#endif

#define GRID_EDITENTER MSG_USER+1

#define GRID_ROW_MAX      	100      // maximum row number
#define GRID_COL_MAX            100      //maximum column number
#define GRID_COLW_DEF	        62  // default column width


//default header height
#define GRID_HDRH_DEF(hwnd)       (GetWindowFont(hwnd)->size + 6)
#define GRID_HDRW_DEF(hwnd)       (GetWindowFont(hwnd)->size * 2)
//default item height
#define GRID_ITEMH_DEF(hwnd)      (GetWindowFont(hwnd)->size + 6)

#define GRID_HDR_TOP		0   // top of the header
#define GRID_HDR_LEFT           0   // left of the header

#define COLWIDTHMIN             10  // minimum column width
#define ROWHEIGHTMIN            10  // minimum row width
#define HSCROLL                 5   // h scroll value
#define VSCROLL                 15  // v scroll value

#define LIGHTBLUE	        ( RGB2Pixel(HDC_SCREEN, 0, 0, 180) )
#define GRAYBLUE	        ( RGB2Pixel(HDC_SCREEN, 156, 166, 189) )

#define GRIDIF_NORMAL	0x0000L
#define GRIDIF_BITMAP  	0x0001L
#define GRIDIF_ICON  	0x0002L

#define GRIDST_NORMAL     0x0000    //normal status
#define GRIDST_BDDRAG     0x0001    //the border is being dragged
#define GRIDST_HEADSELECT  0x0002    //the header is being selected
#define GRIDST_INHEAD     0x0004    //mouse move in header

typedef struct _celldata
{
  struct _celldata *pNext;  // points to the next cell
  struct _celldata *pRight;    // points to the right cell
  struct _gridcolhdr *pColHdr;
  struct _gridrowhdr *pRowHdr;

  DWORD  dwFlags;              // item flags
  char   *pszInfo;             // text of the cell
  int    nTextColor;           // text color of the cell
  int status;                  // item status, focus/highlighted etc.
  DWORD addData;
} CELLDATA;
typedef CELLDATA *PCELLDATA;


/* column header struct */
typedef struct _gridcolhdr
{
  int nCols;
  struct _gridcolhdr *pNext;        // points to the next header
  int x;                        // x position of the header
  int nWidth;                    // width of the header/column/subitem
  //SORTTYPE sort;                // sort status
  char *pTitle;                 // title text of the column header
  //PFNGRIDCOMPARE pfnCmp;          // pointer to the application-defined or default
                                  // comparision function
  DWORD  Image;                 // image of the header
  DWORD flags;                  // header and column flags
  PCELLDATA pHeadCell;
} GRIDCOLHDR;
typedef GRIDCOLHDR *PGRIDCOLHDR;

/* row header struct */
typedef struct _gridrowhdr
{
  int nRows;
  struct _gridrowhdr *pNext;
  int y;
  int nHeight;
  //SORTTYPE sort;
  char *pTitle;
  //PFNGRIDCOMPARE pfnCmp;          // pointer to the application-defined or default
                                  // comparision function
  DWORD  Image;                 // image of the header
  DWORD flags;                  // header and row flags
  PCELLDATA pHeadCell;
}GRIDROWHDR;
typedef GRIDROWHDR *PGRIDROWHDR;


#define GRID_NORMAL	0x0000    //normal status
#define GRID_BDDRAG	0x0001    //the border is being dragged
#define GRID_HEADSELECT	0x0002    //the header is being selected
#define GRID_INHEAD	0x0004    //mouse move in header

/* this macro doesn't check the pointer, so, be careful */
#define GRIDSTATUS(hwnd)  ( ((PGRIDDATA)GetWindowAdditionalData2(hwnd))->status )

typedef struct _griddata
{
  int nHeadWidth;             // row header width
  int nHeadHeight;             // column header height
  int bkc_selected;            // background color of the selected item
  
  int nCols;                   // current column number
  int nRows;                   // current item number
  int nOriginalX;              // scroll x pos
  int nOriginalY;              // scroll y pos

  PCELLDATA pCellFocus;        // focused cell
  PCELLDATA pCellEdit;         // the cell is being edited
  //int nColCurSel;	         // current column selected.
  
  //int nItemDraged;             // the header is being dragged
  DWORD status;                // status: dragged, clicked
  DWORD flags;
  //PGridHDR pHdrClicked;         // the header being clicked
  
  PGRIDCOLHDR pGridColHdr;            // points to the column header list
  PGRIDROWHDR pGridRowHdr;            // points to the row header list
  HWND hWnd;                   // the control handle
  HWND hwnd_edit;

  PGRIDCOLHDR pColDraged;     //the column is being draged
  PGRIDROWHDR pRowDraged;     //the row is being draged
  PGRIDCOLHDR pColSelected;   //the column is being selected
  PGRIDROWHDR pRowSelected;   //the row is being selected
  
  //STRCMP str_cmp;              // default strcmp function
} GRIDDATA;
typedef GRIDDATA *PGRIDDATA;

BOOL RegisterGridControl (void);
void GridControlCleanup (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _GRID_H */

