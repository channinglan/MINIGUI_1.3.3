#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MINIGUI_LIB__
#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "control.h"
#else
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#endif

#ifdef _EXT_CTRL_GRID

#include "mgext.h"
#include "grid.h"

#define IDC_EDIT 101

/********************** interface functions declaration **********************/
//add and remove header
static PGRIDDATA NewGrid(HWND hwnd, int nRows, int nCols);
static int DeleteGrid(PGRIDDATA pGridData);
static int DeleteCol(int nCols, PGRIDDATA pGridData);
static int DeleteRow(int nRows, PGRIDDATA pGridData);
static int AddCol(HWND hwnd, PGRIDCOLHEADER pHeader, PGRIDDATA pGridData);
static int AddRow(HWND hwnd, PGRIDROWHEADER pHeader, PGRIDDATA pGridData);

/*//width and height
static int SetWidth(PGRIDCOLHDR pGridColHdr, PGRIDDATA pGridData);
static int GetWidth(PGRIDCOLHDR pGridColHdr, PGRIDDATA pGridData);
static int SetHeight(PGRIDROWHDR pGridRowHdr, PGRIDDATA pGridData);
static int GetHeight(PGRIDROWHDR pGridRowHdr, PGRIDDATA pGridData);
*/

//draw function
static void GridDrawCells(HWND hwnd, HDC hdc);
static void GridDrawHeader(HWND hwnd, HDC hdc);

//
/************************ internal functions declaration *******************/
static void freecell(PCELLDATA pCell);
static void freerow(PGRIDROWHDR pRow);
static void freecol(PGRIDCOLHDR pCol);
static PCELLDATA newcell(PGRIDCELL pCell);
static PGRIDCOLHDR newcolhdr(PGRIDCOLHEADER pHeader);
static PGRIDROWHDR newrowhdr(PGRIDROWHEADER pHeader);
static void showGrid(PGRIDDATA pGridData);

/**************************************************************************/
static WNDPROC old_edit_proc;
static int EnterEditBox(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
  HWND parent;
  if(message == MSG_KEYDOWN) {
    if(wParam == SCANCODE_ENTER) {
      parent = ((PGRIDDATA)GetWindowAdditionalData(hwnd))->hWnd;
      PostMessage(parent, GRID_EDITENTER, 0, 0);
    }
  }
  return (*old_edit_proc)(hwnd, message, wParam, lParam);
}
/**********************def ***********************************************/
#define GRID_H_OUTWND(pGridData, rcClient)    \
               ( GetGridWidth (pGridData) - 2 > rcClient.right - rcClient.left)
#define GRID_V_OUTWND(pGridData, rcClient)    \
               (GetGridHeight(pGridData) -2 > rcClient.bottom - rcClient.top)
static PCELLDATA GetCellByNum(int nCols, int nRows, PGRIDDATA pgriddata)
{
  PCELLDATA pCell;
  PGRIDCOLHDR pColHdr = pgriddata->pGridColHdr;
  PGRIDROWHDR pRowHdr = pgriddata->pGridRowHdr;

  while(pColHdr && (pColHdr->nCols != nCols))
    pColHdr = pColHdr->pNext;

  if(pColHdr) {
    pCell = pColHdr->pHeadCell;
    while(pRowHdr && (pRowHdr->nRows != nRows)) {
      pRowHdr = pRowHdr->pNext;
      pCell = pCell->pRight;
    }
    if(pRowHdr)
      return pCell;
  }
  return NULL;
}
static PGRIDCOLHDR GetColHdrByNum(int nCols, PGRIDDATA pgriddata)
{
  PGRIDCOLHDR p1 = pgriddata->pGridColHdr;

  if(nCols <= pgriddata->nCols) {
    while(p1) {
      if(p1->nCols == nCols) {
	return p1;
	break;
      }
      p1 = p1->pNext;
    }
  }
  return NULL;
}

static PGRIDROWHDR GetRowHdrByNum(int nRows, PGRIDDATA pgriddata)
{
  PGRIDROWHDR p1 = pgriddata->pGridRowHdr;

  if(nRows <= pgriddata->nRows) {
    while(p1) {
      if(p1->nRows == nRows) {
	return p1;
	break;
      }
      p1 = p1->pNext;
    }
  }
  return NULL;
}

static int IsInGridCell(int mouseX, int mouseY, PCELLDATA *pRet, PGRIDDATA pgriddata)
{
  int ret = -1;
  PGRIDCOLHDR pColHdr = pgriddata->pGridColHdr;
  PGRIDROWHDR pRowHdr = pgriddata->pGridRowHdr;
  PCELLDATA pCell = NULL;

  //mouseX -= GRID_TOP;

  if((mouseX < pgriddata->nHeadWidth) || (mouseY < pgriddata->nHeadHeight))
    return ret;

  while(pColHdr) {
    if(mouseX > pColHdr->x - pgriddata->nOriginalX + 1 && mouseX > pgriddata->nHeadWidth
       && mouseX < pColHdr->x - pgriddata->nOriginalX + pColHdr->nWidth -1)
      break;
    else
      pColHdr = pColHdr->pNext;
  }
  if(pColHdr) {
    pCell = pColHdr->pHeadCell;
    while(pRowHdr) {
      if(mouseY > pRowHdr->y - pgriddata->nOriginalY + 1 && mouseY > pgriddata->nHeadHeight
	 && mouseY < pRowHdr->y - pgriddata->nOriginalY + pRowHdr->nHeight -1) {
	ret = 1;
	if(pRet)
	  *pRet = pCell;
	break;
      }
      else {
	pRowHdr = pRowHdr->pNext;
	pCell = pCell->pNext;
      }
    }
  }
  return ret;
}
static int IsInGridColHdr(int mouseX, int mouseY, PGRIDCOLHDR *pRet, PGRIDDATA pgriddata)
{
  int nPosition = 0;
  PGRIDCOLHDR p1 = pgriddata->pGridColHdr;
  RECT rect;
                                                                                                      
  while (p1){
    rect.left = p1->x - pgriddata->nOriginalX + 1;
    rect.top = GRID_HDR_TOP;
    rect.right = p1->x + p1->nWidth -pgriddata->nOriginalX - 1;
    rect.bottom = pgriddata->nHeadHeight;

    if (PtInRect (&rect, mouseX, mouseY))
      break;
    p1 = p1->pNext;
    nPosition++;
  }
  //not in head
  if (!p1 || (nPosition > pgriddata->nCols) || (nPosition < 0)) {
    if (pRet)
      *pRet = NULL;
    return -1;
  }
  //in head
  if (pRet)
    *pRet = p1;
  return nPosition;
}

static int IsInGridRowHdr(int mouseX, int mouseY, PGRIDROWHDR *pRet, PGRIDDATA pgriddata)
{
  int nPosition = -1;
  PGRIDROWHDR p1 = pgriddata->pGridRowHdr;
  RECT rect;
                                                                                                      
  while (p1){
    nPosition++;
    rect.left = GRID_HDR_LEFT;
    rect.top = p1->y - pgriddata->nOriginalY - 1;
    rect.right = GRID_HDR_LEFT + pgriddata->nHeadWidth;
    rect.bottom = p1->y + p1->nHeight - pgriddata->nOriginalY - 1;

    if (PtInRect (&rect, mouseX, mouseY))
      break;
    p1 = p1->pNext;
  }
  //not in head
  if (!p1 || (nPosition > pgriddata->nRows) || (nPosition < 0)) {
    if (pRet)
      *pRet = NULL;
    return -1;
  }
  //in head
  if (pRet)
    *pRet = p1;
  return nPosition;
}
static int GridInWhichColHdrBorder(int mouseX, int mouseY, PGRIDCOLHDR *pRet, PGRIDDATA pgriddata)
{
  int nPosition = -1;
  PGRIDCOLHDR p1 = pgriddata->pGridColHdr;
  
  while (p1){
    nPosition++;
    if((mouseX >= (p1->x + p1->nWidth - pgriddata->nOriginalX - 2))
       && (mouseX <= (p1->x + p1->nWidth - pgriddata->nOriginalX))
       && (mouseY >= 0) && (mouseY <= pgriddata->nHeadHeight) )
      break;
    p1 = p1->pNext;
  }
  if (!p1) {
    if (pRet)
      *pRet = NULL;
    return -1;
  }
  if (pRet)
    *pRet = p1;
  return nPosition;
}
static int GridInWhichRowHdrBorder(int mouseX, int mouseY, PGRIDROWHDR *pRet, PGRIDDATA pgriddata)
{
  int nPosition = 0;
  PGRIDROWHDR p1 = pgriddata->pGridRowHdr;
  
  while (p1 != NULL){
    if((mouseY >= (p1->y + p1->nHeight - pgriddata->nOriginalY - 2))
       && (mouseY <= (p1->y + p1->nHeight - pgriddata->nOriginalY))
       && (mouseX >= 0) && (mouseX <= pgriddata->nHeadWidth) )
      break;
    p1 = p1->pNext;
    nPosition++;
  }
  if (!p1) {
    if (pRet)
      *pRet = NULL;
    return -1;
  }
  if (pRet)
    *pRet = p1;
  return nPosition;
}
static int GridSetColWidth (PGRIDCOLHDR pColHdr, int width, PGRIDDATA pgriddata)
{
  PGRIDCOLHDR p1 = pgriddata->pGridColHdr;
  if(!pColHdr)
    return -1;
  
  while(p1 && p1 != pColHdr)
    p1 = p1->pNext;
  if(!p1)
    return -1;

  if(width < COLWIDTHMIN)
    width = COLWIDTHMIN;
  p1->nWidth = width;
  while(p1->pNext) {
    p1->pNext->x = p1->x + p1->nWidth;
    p1 = p1->pNext;
  }
  return 1;
}
static int GridSetRowHeight (PGRIDROWHDR pRowHdr, int height, PGRIDDATA pgriddata)
{
  PGRIDROWHDR p1 = pgriddata->pGridRowHdr;
  
  if(!pRowHdr)
    return -1;

  while(p1 && p1 != pRowHdr)
    p1 = p1->pNext;
  if(!p1)
    return -1;

  if(height < ROWHEIGHTMIN)
    height = ROWHEIGHTMIN;
  p1->nHeight = height;
  while(p1->pNext) {
    p1->pNext->y = p1->y + p1->nHeight;
    p1 = p1->pNext;
  }
  return 1;
}
static void GridColBorderDrag (HWND hwnd, int x, int y)
{
  int mouseX = x, mouseY = y;
  PGRIDDATA pgriddata;
  RECT rcClient;
  int offset;
  PGRIDCOLHDR pDrag;

  pgriddata = (PGRIDDATA)GetWindowAdditionalData2(hwnd);
  pDrag = pgriddata->pColDraged;

  GetClientRect(hwnd, &rcClient);
  ScreenToClient(hwnd, &mouseX, &mouseY);

  if((pDrag->x - pgriddata->nOriginalX + COLWIDTHMIN) > mouseX - 1)
    return;

  offset = mouseX - (pDrag->x + pDrag->nWidth-pgriddata->nOriginalX);
  GridSetColWidth(pgriddata->pColDraged, pDrag->nWidth+offset, pgriddata);
  
  /*rect.left = rcClient.left;
  rect.right = rcClient.right;
  rect.top = rcClient.top;
  rect.bottom = rect.top + pgriddata->nHeadHeight+1;*/

  InvalidateRect(hwnd, NULL, FALSE);

  /*if (offset < 0) {
    pgriddata->nOriginalX += offset;
    if(pgriddata->nOriginalX < 0)
      pgriddata->nOriginalX = 0;
      }*/

  return;
}
static void GridRowBorderDrag (HWND hwnd, int x, int y)
{
  int mouseX = x, mouseY = y;
  PGRIDDATA pgriddata;
  RECT rcClient;
  int offset;
  PGRIDROWHDR pDrag;

  pgriddata = (PGRIDDATA)GetWindowAdditionalData2(hwnd);
  pDrag = pgriddata->pRowDraged;

  GetClientRect(hwnd, &rcClient);
  ScreenToClient(hwnd, &mouseX, &mouseY);

  if((pDrag->y - pgriddata->nOriginalY + ROWHEIGHTMIN) > mouseY - 1)
    return;

  offset = mouseY - (pDrag->y + pDrag->nHeight-pgriddata->nOriginalY);
  GridSetRowHeight(pgriddata->pRowDraged, pDrag->nHeight+offset, pgriddata);
  
  /*rect.left = rcClient.left;
  rect.right = rcClient.right;
  rect.top = rcClient.top;
  rect.bottom = rect.top + pgriddata->nHeadHeight+1;*/

  InvalidateRect(hwnd, NULL, FALSE);

  /*if (offset < 0) {
    pgriddata->nOriginalY += offset;
    if(pgriddata->nOriginalY < 0)
      pgriddata->nOriginalY = 0;
      }*/

  return;
}
/*********************** inline functions for convinient ******************/
static inline int AddRowTail(HWND hwnd, PGRIDDATA pGridData)
{
  GRIDROWHEADER row;

  memset(&row, 0, sizeof(row));
  row.nRows = pGridData->nRows;

  return AddRow(hwnd, &row, pGridData);
}
static inline int AddColTail(HWND hwnd, PGRIDDATA pGridData)
{
  GRIDCOLHEADER col;

  memset(&col, 0, sizeof(col));
  col.nCols = pGridData->nCols;

  return AddCol(hwnd, &col, pGridData);
} 
static inline int DeleteRowTail(PGRIDDATA pGridData)
{
  return DeleteRow(pGridData->nRows - 1, pGridData);
}
static inline int DeleteColTail(PGRIDDATA pGridData)
{
  return DeleteCol(pGridData->nCols - 1, pGridData);
}
/***************************************************************************/
/* Grid control constuctor.
 * @nRows: number of rows;
 * @nCols: number of columns
 * return pointer of Grid control if success, or return NULL
 */
static PGRIDDATA NewGrid(HWND hwnd, int nRows, int nCols)
{
  int i;
  PGRIDDATA pGridData = calloc(1, sizeof(GRIDDATA));
  if(!pGridData) return NULL;

  pGridData->nHeadHeight = GRID_HDRH_DEF(hwnd);
  pGridData->nHeadWidth = GRID_HDRW_DEF(hwnd);
  for(i=0; i<nRows; i++) {
    if(!AddRowTail(hwnd, pGridData)) return NULL;
  }
  for(i=0; i<nCols; i++) {
    if(!AddColTail(hwnd, pGridData)) return NULL;
  }

  pGridData->bkc_selected = LIGHTBLUE;
  pGridData->hWnd = hwnd;
  pGridData->hwnd_edit = 0;
  pGridData->pCellEdit = NULL;
  pGridData->pCellFocus = pGridData->pGridColHdr->pHeadCell;
  
  return pGridData;
}
/* Grid control deconstructor
 * @pGridData: pointer of control to delete
 * return 1 if success, 0 for failure
 */
static int DeleteGrid(PGRIDDATA pGridData)
{
  while(pGridData->nCols) 
    if(DeleteCol(0, pGridData) == 0) return 0;
  while(pGridData->nRows) 
    if(DeleteRow(0, pGridData) == 0) return 0;
  
  free(pGridData);

  return 1;
}
/* Delete a column indexed by nCols
 * @nCols: index.
 * @pGridData: Grid control instance.
 * return 1 if success
 */
static int DeleteCol(int nCols, PGRIDDATA pGridData)
{
  PGRIDCOLHDR pCol1, pCol2;
  PGRIDROWHDR pRowHdr;
  PCELLDATA pCell1, pCell2, pCell3;

  if(nCols > pGridData->nCols-1) return 0;

  pCol1 = pCol2 = pGridData->pGridColHdr;

  if(nCols == 0) {//the first column
    pGridData->pGridColHdr = pCol2->pNext;
    if(pCol1->pNext) 
      pCell1 = pCol1->pNext->pHeadCell;
    else
      pCell1 = NULL;
    pCell3 = pCell2 = pCol1->pHeadCell;
    
    //adjust pointers of row
    pRowHdr = pGridData->pGridRowHdr;
    while(pRowHdr) {
      pCell3 = pCell2->pNext;
      freecell(pCell2);
      pCell2 = pCell3;

      pRowHdr->pHeadCell = pCell1;
      pRowHdr = pRowHdr->pNext;
      if(pCell1) pCell1 = pCell1->pNext;
    }
    freecol(pCol2);
  }
  else {//!first column
    while(pCol2->nCols != nCols ) {
      pCol1 = pCol2;
      pCol2 = pCol2->pNext;
    }
    
    pCol1->pNext = pCol2->pNext;//adjust column header pointer    
    pRowHdr = pGridData->pGridRowHdr;//adjust pointers of previous column 
    pCell3 = pCol2->pHeadCell;

    while(pRowHdr) {
      pCell1 = pCell2 = pRowHdr->pHeadCell;
      while(pCell2 != pCell3) {
	pCell1 = pCell2;
	pCell2 = pCell2->pRight;
      }
      pCell1->pRight = pCell2->pRight;
      pRowHdr = pRowHdr->pNext;
      pCell3 = pCell3->pNext;
      freecell(pCell2);
    }
    freecol(pCol2);
  }

  //dec column header nCols field
  pCol2 = pCol1->pNext;
  while(pCol2) {
    pCol2->x = pCol1->x +pCol1->nWidth;
    pCol2->nCols--;
    pCol1 = pCol2;
    pCol2 = pCol2->pNext;
  }
  pGridData->nCols--;

  return 1;
}

/***************************************************************************/
/* Delete a row indexed by nRows
 * @nRows: index.
 * @pGridData: Grid control data.
 * return 1 if success
 */
static int DeleteRow(int nRows, PGRIDDATA pGridData)
{
  PGRIDROWHDR pRow1, pRow2;
  PGRIDCOLHDR pColHdr;
  PCELLDATA pCell1, pCell2, pCell3;

  if(nRows > pGridData->nRows-1) return 0;

  pRow1 = pRow2 = pGridData->pGridRowHdr;
  if(pRow2->nRows == nRows) {//the first row
    pGridData->pGridRowHdr = pRow2->pNext;
    if(pRow1->pNext)
      pCell1 = pRow1->pNext->pHeadCell;
    else
      pCell1 = NULL;
    pCell3 = pCell2 = pRow1->pHeadCell;
    
    //adjust pointers of row
    pColHdr = pGridData->pGridColHdr;
    while(pColHdr) {
      pCell3 = pCell2->pRight;
      freecell(pCell2);
      pCell2 = pCell3;

      pColHdr->pHeadCell = pCell1;
      pColHdr = pColHdr->pNext;
      if(pCell1) pCell1 = pCell1->pRight;
    }
    freerow(pRow2);
  }
  else {//!the first row
    while(pRow2->nRows != nRows ) {
      pRow1 = pRow2;
      pRow2 = pRow2->pNext;
    }
    
    pRow1->pNext = pRow2->pNext;//adjust row header pointer
    pColHdr = pGridData->pGridColHdr;//adjust pointers of previous row
    pCell3 = pRow2->pHeadCell;

    while(pColHdr) {
      pCell1 = pCell2 = pColHdr->pHeadCell;
      while(pCell2 != pCell3) {
	pCell1 = pCell2;
	pCell2 = pCell2->pNext;
      }
      pCell1->pNext = pCell2->pNext;
      pColHdr = pColHdr->pNext;
      pCell3 = pCell3->pRight;
      freecell(pCell2);
    }
    freerow(pRow2);
  }

  //dec row header nRows field
  pRow2 = pRow1->pNext;
  while(pRow2) {
    pRow2->y = pRow1->y + pRow1->nHeight;
    pRow2->nRows--;
    pRow1 = pRow2;
    pRow2 = pRow2->pNext;
  }
  pGridData->nRows--;

  return 1;//sucess
}

static int AddCol(HWND hwnd, PGRIDCOLHEADER pHeader, PGRIDDATA pGridData)
{
  PGRIDCOLHDR pCol1, pCol2;
  PGRIDROWHDR pRowHdr;
  PCELLDATA pCell1, pCell2, pCell3;
  GRIDCOLHEADER temp;
  memset(&temp, 0, sizeof(temp));
  //GRIDCELL cellinfo; 

  if(pGridData->nCols >= GRID_COL_MAX)
    return -1;
  
  //check pHeader
  if(!pHeader) pHeader = (PGRIDCOLHEADER)&temp;
  if(pHeader->nWidth < 10)
    pHeader->nWidth = pGridData->nHeadWidth;;
  if(pHeader->nCols > pGridData->nCols)
    pHeader->nCols = pGridData->nCols;

  pRowHdr = pGridData->pGridRowHdr;

  if(pHeader->nCols == 0) {//first column
    //header
    if((pCol1 = pCol2 = newcolhdr(pHeader)) == NULL) 
      return 0;//failed to get mem
    pCol1->x = pGridData->nHeadWidth;
    pCol1->pNext = pGridData->pGridColHdr;
    pGridData->pGridColHdr = pCol1;
    //first cell
    if(pRowHdr) {
      if((pCell1 = newcell(NULL)) == NULL)
	return 0;
      pCol1->pHeadCell = pCell1;
      pCell1->pRight = pRowHdr->pHeadCell;
      (PGRIDCOLHDR)pCell1->pRowHdr = pRowHdr;
      (PGRIDCOLHDR)pCell1->pColHdr = pCol1;
      pRowHdr->pHeadCell = pCell1;
      pRowHdr = pRowHdr->pNext;
    }
    //other cells
    while(pRowHdr) {
      if((pCell2 = newcell(NULL)) == NULL)
	return 0;
      pCell1->pNext = pCell2;
      pCell1 = pCell2;
      pCell2->pRight = pRowHdr->pHeadCell;
      (PGRIDCOLHDR)pCell2->pColHdr = pCol2;
      (PGRIDROWHDR)pCell2->pRowHdr = pRowHdr;
      pRowHdr->pHeadCell = pCell2;
      pRowHdr = pRowHdr->pNext;
    }
  }
  else {//!first col
    pCol1 = pGridData->pGridColHdr;
    while(pCol1->nCols != pHeader->nCols - 1) {
      pCol1 = pCol1->pNext;
    }
    //header
    if((pCol2 = newcolhdr(pHeader)) == NULL)
      return 0;
    pCol2->x = pCol1->x + pCol1->nWidth;
    pCol2->pNext = pCol1->pNext;
    pCol1->pNext = pCol2;
    //first cell
    if(pRowHdr) {
      pCell1 = pCol1->pHeadCell;
      if(pCol2->pNext)
	pCell2 = pCol2->pNext->pHeadCell;
      else
	pCell2 = NULL;
      if((pCell3 = newcell(NULL)) == NULL)
	return 0;
      pCol2->pHeadCell = pCell3;
      pCell1->pRight = pCell3;
      pCell3->pRight = pCell2;
      (PGRIDCOLHDR)pCell3->pColHdr = pCol2;
      (PGRIDROWHDR)pCell3->pRowHdr = pRowHdr;
      pRowHdr = pRowHdr->pNext;
      pCell1 = pCell1->pNext;
      if(pCell2) pCell2 = pCell2->pNext;
    }
    //other cells
    while(pRowHdr) {
      if((pCell3->pNext = newcell(NULL)) == NULL)
	return 0;
      pCell3 = pCell3->pNext;
      pCell1->pRight = pCell3;
      pCell3->pRight = pCell2;
      (PGRIDCOLHDR)pCell3->pColHdr = pCol2;
      (PGRIDROWHDR)pCell3->pRowHdr = pRowHdr;
      pRowHdr = pRowHdr->pNext;
      pCell1 = pCell1->pNext;
      if(pCell2) pCell2 = pCell2->pNext;
    }    
  }

  //inc headers nCols field
  pCol2 = pCol2->pNext;
  while(pCol2) {
    pCol2->nCols++;
    pCol2->x += pHeader->nWidth;
    pCol2 = pCol2->pNext;
  }
  pGridData->nCols++;
  
  return 1;//sucess
}

static int AddRow(HWND hwnd, PGRIDROWHEADER pHeader, PGRIDDATA pGridData)
{
  PGRIDROWHDR pRow1, pRow2;
  PGRIDCOLHDR pColHdr;
  PCELLDATA pCell1, pCell2, pCell3;
  GRIDROWHEADER temp;

  if(pGridData->nRows >= GRID_ROW_MAX)
    return -1;

  //check pHeader
  if(!pHeader) 
    pHeader = (PGRIDROWHEADER)&temp;
  if(pHeader->nHeight < 10)
    pHeader->nHeight = pGridData->nHeadHeight;
  if(pHeader->nRows > pGridData->nRows)
    pHeader->nRows = pGridData->nRows;

  pColHdr = pGridData->pGridColHdr;

  if(pHeader->nRows == 0) {//first row
    //header
    if((pRow1 = pRow2 = newrowhdr(pHeader)) == NULL) 
      return 0;//failed to get mem
    pRow1->y = pGridData->nHeadHeight;
    pRow1->pNext = pGridData->pGridRowHdr;
    pGridData->pGridRowHdr = pRow1;
    //first cell
    if(pColHdr) {
      if((pCell1 = newcell(NULL)) == NULL)
	return 0;
      pRow1->pHeadCell = pCell1;
      pCell1->pNext = pColHdr->pHeadCell;
      (PGRIDCOLHDR)pCell1->pColHdr = pColHdr;
      (PGRIDROWHDR)pCell1->pRowHdr = pRow2;
      pColHdr->pHeadCell = pCell1;
      pColHdr = pColHdr->pNext;
    }
    //other cells
    while(pColHdr) {
      if((pCell2 = newcell(NULL)) == NULL)
	return 0;
      pCell1->pRight = pCell2;
      pCell1 = pCell2;
      pCell2->pNext = pColHdr->pHeadCell;
      (PGRIDCOLHDR)pCell2->pColHdr = pColHdr;
      (PGRIDROWHDR)pCell2->pRowHdr = pRow2;
      pColHdr->pHeadCell = pCell2;
      pColHdr = pColHdr->pNext;
    }
  }
  else {//!first row
    pRow1 = pGridData->pGridRowHdr;
    while(pRow1->nRows != pHeader->nRows - 1) {
      pRow1 = pRow1->pNext;
    }
    //header
    if((pRow2 = newrowhdr(pHeader)) == NULL)
      return 0;
    pRow2->y = pRow1->y + pRow1->nHeight;
    pRow2->pNext = pRow1->pNext;
    pRow1->pNext = pRow2;
    //first cell
    if(pColHdr) {
      pCell1 = pRow1->pHeadCell;
      if(pRow2->pNext) 
	pCell2 = pRow2->pNext->pHeadCell;
      else
	pCell2 = NULL;
      if((pCell3 = newcell(NULL)) == NULL)
	return 0;
      pRow2->pHeadCell = pCell3;
      pCell1->pNext = pCell3;
      pCell3->pNext = pCell2;
      (PGRIDCOLHDR)pCell3->pColHdr = pColHdr;
      (PGRIDROWHDR)pCell3->pRowHdr = pRow2;

      pColHdr = pColHdr->pNext;
      if(pCell1) pCell1 = pCell1->pRight;
      if(pCell2) pCell2 = pCell2->pRight;
    }
    //other cells
    while(pColHdr) {
      if((pCell3->pRight = newcell(NULL)) == NULL)
	return 0;
      pCell3 = pCell3->pRight;
      pCell1->pNext = pCell3;
      pCell3->pNext = pCell2;
      (PGRIDCOLHDR)pCell3->pColHdr = pColHdr;
      (PGRIDROWHDR)pCell3->pRowHdr = pRow2;

      pColHdr = pColHdr->pNext;
      if(pCell1) pCell1 = pCell1->pRight;
      if(pCell2) pCell2 = pCell2->pRight;
    }    
  }
  
  //inc headers nCols field
  pRow2 = pRow2->pNext;
  while(pRow2) {
    pRow2->nRows++;
    pRow2->y += pHeader->nHeight;
    pRow2 = pRow2->pNext;
  }
  pGridData->nRows++;
  
  return 1;//sucess
}

/************************* internal functions ************************************/
static void freecell(PCELLDATA pCell)
{
  if(pCell->pszInfo) 
    free(pCell->pszInfo);
  free(pCell);
}
static void freerow(PGRIDROWHDR pRow)
{
  if(pRow->pTitle)
    free(pRow->pTitle);
  free(pRow);
}
static void freecol(PGRIDCOLHDR pCol)
{
  if(pCol->pTitle)
    free(pCol->pTitle);
  free(pCol);
}
static PCELLDATA newcell(PGRIDCELL pCell)
{
  PCELLDATA pNew = calloc(1, sizeof(CELLDATA));

  if(pCell) {
    //do some initial work here
    if(pCell->pszInfo) {
      pNew->pszInfo = malloc(strlen(pCell->pszInfo) + 1);
      strcpy(pNew->pszInfo, pCell->pszInfo);
    }
    else {
      pNew->pszInfo = malloc(1);
      strcpy(pNew->pszInfo, ""); 
    }
    if(pCell->nTextColor) 
      pNew->nTextColor = pCell->nTextColor;
    else
      pNew->nTextColor = COLOR_black;
    pNew->addData = pCell->addData;
  }

  return pNew;
}
static PGRIDCOLHDR newcolhdr(PGRIDCOLHEADER pHeader)
{
  char col[3]; //since the max col is 62
  PGRIDCOLHDR pCol = calloc(1, sizeof(GRIDCOLHDR));

  //initial work
  if(pHeader) {
    if(pHeader->nWidth) pCol->nWidth = pHeader->nWidth;
    if(pHeader->nCols) pCol->nCols = pHeader->nCols;

    if(pHeader->pTitle) {
      pCol->pTitle = malloc(strlen(pHeader->pTitle) + 1);
      strcpy(pCol->pTitle, pHeader->pTitle);
    }
    else {
      sprintf(col, "%d", pHeader->nCols);
      pCol->pTitle = malloc(strlen(col) + 1);
      strcpy(pCol->pTitle, col);
    }
    
    if(pHeader->image) pCol->Image = pHeader->image;
    if(pHeader->flags) pCol->flags = pHeader->flags;
  }

  return pCol;
}
static PGRIDROWHDR newrowhdr(PGRIDROWHEADER pHeader)
{
  char row[7]; //since the max row is 100000
  PGRIDROWHDR pRow = calloc(1, sizeof(GRIDROWHDR));

  if(pHeader) {
    //initial work
    if(pHeader->nHeight) pRow->nHeight = pHeader->nHeight;
    if(pHeader->nRows) pRow->nRows = pHeader->nRows;
    
    if(pHeader->pTitle) {
      pRow->pTitle = malloc(strlen(pHeader->pTitle) + 1);
      strcpy(pRow->pTitle, pHeader->pTitle);
    }
    else {
      sprintf(row, "%d", pHeader->nRows);
      pRow->pTitle = malloc(strlen(row) + 1);
      strcpy(pRow->pTitle, row);
    }
  
    if(pHeader->image) pRow->Image = pHeader->image;
    if(pHeader->flags) pRow->flags = pHeader->flags;
  }

  return pRow;
}
static int GetGridWidth(PGRIDDATA pGridData)
{
  int width;
  PGRIDCOLHDR pCol = pGridData->pGridColHdr;
  width = pGridData->nHeadWidth;
  while(pCol) {
    width += pCol->nWidth;
    pCol = pCol->pNext;
  }

  return width;  
}
static int GetGridHeight(PGRIDDATA pGridData)
{
  int height;
  PGRIDROWHDR pRow = pGridData->pGridRowHdr;
  height = pGridData->nHeadHeight;
  while(pRow) {
    height += pRow->nHeight;
    pRow = pRow->pNext;
  }

  return height;  
}
static void showGrid(PGRIDDATA pGridData)
{
  PGRIDROWHDR pRowHdr;
  PCELLDATA pCell;
  pRowHdr = pGridData->pGridRowHdr;
  printf("Show Grid NUMRow:%d   NUMCol:%d\n", pGridData->nRows, pGridData->nCols);
  while(pRowHdr) {
    printf("%s  ", pRowHdr->pTitle);
    pCell = pRowHdr->pHeadCell;
    while(pCell) {
      printf("%s  ", pCell->pszInfo);
      pCell = pCell->pRight;
    }
    pRowHdr = pRowHdr->pNext;
    printf("\n");
  }
}

/*************************** draw functions ************************************/
static void GridDrawText (HDC hdc, int x, int y, int width, int height, 
		const char *pszText, UINT format)
{
    RECT rect;
    //SIZE size;

    if (pszText != NULL)
    {
	SetRect (&rect, x+2, y+2, x+width, y+height);
        DrawText (hdc, pszText, -1, &rect, format);
    }
}
static void GridDrawHeader(HWND hwnd, HDC hdc)
{
  PGRIDCOLHDR pCol = NULL;
  PGRIDROWHDR pRow = NULL;
  PGRIDDATA pGridData;
  RECT rcClient;
  BOOL up = TRUE;
  UINT format;

  GetClientRect (hwnd, &rcClient);
  pGridData = (PGRIDDATA) GetWindowAdditionalData2 (hwnd);
  pCol = pGridData->pGridColHdr;
  pRow = pGridData->pGridRowHdr;

  SetBkColor (hdc, PIXEL_lightgray);
  SetBrushColor (hdc, PIXEL_lightgray);
  FillBox (hdc, rcClient.left, rcClient.top, rcClient.right - rcClient.left,
	   pGridData->nHeadHeight);
  FillBox(hdc, rcClient.left, rcClient.top, pGridData->nHeadWidth,
	  rcClient.bottom - rcClient.top);

  SetTextColor (hdc, PIXEL_black);

  //column header
  while (pCol) {
    if((pCol->x +pCol->nWidth - pGridData->nOriginalX > pGridData->nHeadWidth) 
       || (pCol->x +pCol->nWidth - pGridData->nOriginalX < GetGridWidth(pGridData))) {
      if(pCol == pGridData->pColSelected) {
	up = FALSE;
      }
#ifdef _FLAT_WINDOW_STYLE
      DrawFlatControlFrameEx (hdc, pCol->x - pGridData->nOriginalX+1,
			      GRID_HDR_TOP,// - pGridData->nOriginalY-1,
			      pCol->x - pGridData->nOriginalX + pCol->nWidth - 1,
			      GRID_HDR_TOP + pGridData->nHeadHeight, PIXEL_lightgray, 0, up);
#else
      Draw3DControlFrame (hdc, pCol->x - pGridData->nOriginalX + 1,
			  GRID_HDR_TOP,
			  pCol->x - pGridData->nOriginalX + pCol->nWidth - 1,
			  GRID_HDR_TOP + pGridData->nHeadHeight, PIXEL_lightgray, up);
#endif
      up = TRUE;
      if (pCol->flags & GRIDHF_CENTERALIGN)
	format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
      else if (pCol->flags & GRIDHF_RIGHTALIGN)
	format = DT_SINGLELINE | DT_RIGHT | DT_VCENTER;
      else
	format = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
      
      GridDrawText (hdc, pCol->x - pGridData->nOriginalX + 2, GRID_HDR_TOP,
		    pCol->nWidth - 4, pGridData->nHeadHeight, pCol->pTitle, format);
      //draw the cell line
      if(pCol->x + pCol->nWidth - pGridData->nOriginalX > pGridData->nHeadWidth) {
	MoveTo(hdc, pCol->x + pCol->nWidth - pGridData->nOriginalX-1, pGridData->nHeadHeight);
	LineTo(hdc, pCol->x + pCol->nWidth - pGridData->nOriginalX-1, GetGridHeight(pGridData));
      }
    }
    pCol = pCol->pNext;
  }
  //draws the right most unused header
  if ( !GRID_H_OUTWND(pGridData, rcClient) ) {
#ifdef _FLAT_WINDOW_STYLE
    DrawFlatControlFrameEx (hdc, GetGridWidth (pGridData)-2,
			    GRID_HDR_TOP-1,
			    rcClient.right+2,
			    GRID_HDR_TOP + pGridData->nHeadHeight, PIXEL_lightgray, 0, up);
#else
    Draw3DControlFrame (hdc, GetGridWidth (pGridData),
			GRID_HDR_TOP,
			rcClient.right+2,
			GRID_HDR_TOP + pGridData->nHeadHeight, PIXEL_lightgray, up);
#endif
  }
  //row header
  while (pRow) {
    if((pRow->y + pRow->nHeight - pGridData->nOriginalY > pGridData->nHeadHeight) 
       || (pRow->y + pRow->nHeight - pGridData->nOriginalY < GetGridHeight(pGridData))) {
      if(pRow == pGridData->pRowSelected)
	up = FALSE; 
#ifdef _FLAT_WINDOW_STYLE
      DrawFlatControlFrameEx (hdc, GRID_HDR_LEFT,
			      pRow->y - pGridData->nOriginalY+1,
			      GRID_HDR_LEFT + pGridData->nHeadWidth,
			      pRow->y - pGridData->nOriginalY + pRow->nHeight -1 , PIXEL_lightgray, 0, up);
#else
      Draw3DControlFrame (hdc, GRID_HDR_LEFT,
			  pRow->y - pGridData->nOriginalY + 1,
			  GRID_HDR_LEFT +pGridData->nHeadWidth,
			  pRow->y - pGridData->nOriginalY + pRow->nHeight - 1, PIXEL_lightgray, up);
#endif
      up = TRUE;
      if (pRow->flags & GRIDHF_CENTERALIGN)
	format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
      else if(pRow->flags & GRIDHF_RIGHTALIGN)
	format = DT_SINGLELINE | DT_RIGHT | DT_VCENTER;
      else
	format = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
      
      GridDrawText (hdc, GRID_HDR_LEFT + 2, pRow->y - pGridData->nOriginalY + 2,
		    pGridData->nHeadWidth - 4, pRow->nHeight - 4, pRow->pTitle, format);
      //draw the cell line
      if(pRow->y + pRow->nHeight - pGridData->nOriginalY > pGridData->nHeadHeight) {
	MoveTo(hdc, pGridData->nHeadWidth, pRow->y + pRow->nHeight - pGridData->nOriginalY-1);
	LineTo(hdc, GetGridWidth(pGridData), pRow->y + pRow->nHeight - pGridData->nOriginalY-1);
      }
    }
    pRow = pRow->pNext;
  }
  //draws the down most unused header
  if ( !GRID_V_OUTWND(pGridData, rcClient) ) {
#ifdef _FLAT_WINDOW_STYLE
    DrawFlatControlFrameEx (hdc, GRID_HDR_LEFT,
			    GetGridHeight (pGridData)-1,
			    GRID_HDR_LEFT + pGridData->nHeadWidth,
			    rcClient.bottom, PIXEL_lightgray, 0, up);
#else
    Draw3DControlFrame (hdc, GRID_HDR_LEFT, 
			GetGridHeight (pGridData) -1,
			GRID_HDR_LEFT + pGridData->nHeadWidth,
			rcClient.bottom, PIXEL_lightgray, up);
#endif
  }
  //draw the empty header
  SetBkColor (hdc, PIXEL_lightgray);
  SetBrushColor (hdc, PIXEL_lightgray);
  FillBox (hdc, rcClient.left, rcClient.top, pGridData->nHeadWidth+1,
	   pGridData->nHeadHeight+1);
}

static void GridDrawCells(HWND hwnd, HDC hdc)
{
  PGRIDDATA pGridData;
  PGRIDCOLHDR pCol;
  PGRIDROWHDR pRow;
  PCELLDATA pCell;
  //RECT rcClient;
  int format;

  pGridData = (PGRIDDATA) GetWindowAdditionalData2 (hwnd);
  pRow = pGridData->pGridRowHdr;
  pCol = pGridData->pGridColHdr;

  if((pGridData->flags & GRID_COL_PRIORTY) == GRID_COL_PRIORTY) {//column priorty
    while(pCol) {
      if((pCol->x +pCol->nWidth - pGridData->nOriginalX > pGridData->nHeadWidth) 
       || (pCol->x +pCol->nWidth - pGridData->nOriginalX < GetGridWidth(pGridData))) {
	pRow = pGridData->pGridRowHdr;
	pCell = pCol->pHeadCell;
	if (pCol->flags & GRIDHF_CENTERALIGN)
	  format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
	else if (pCol->flags & GRIDHF_RIGHTALIGN)
	  format = DT_SINGLELINE | DT_RIGHT | DT_VCENTER;
	else
	  format = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
	if (pCol != pGridData->pColSelected) {
	  SetBrushColor (hdc, GetWindowBkColor(pGridData->hWnd));
	  SetBkColor (hdc, GetWindowBkColor(pGridData->hWnd));
	  SetTextColor (hdc, pCell->nTextColor);
	}
	else {
	  SetBkColor (hdc, pGridData->bkc_selected);
	  SetBrushColor (hdc, pGridData->bkc_selected);
	  SetTextColor (hdc, PIXEL_lightwhite);
	}
	while(pRow) {
	  if((pRow->y + pRow->nHeight - pGridData->nOriginalY > pGridData->nHeadHeight) 
	     || (pRow->y + pRow->nHeight - pGridData->nOriginalY < GetGridHeight(pGridData))) {      
	    //check pcell flags here
	    
	    if(pCell != pGridData->pCellEdit) {
	      if(pCell == pGridData->pCellFocus) {
		MoveTo(hdc, pCol->x - pGridData->nOriginalX, pRow->y - pGridData->nOriginalY);
		LineTo(hdc, pCol->x - pGridData->nOriginalX + pCol->nWidth -2, pRow->y - pGridData->nOriginalY);
		LineTo(hdc, pCol->x - pGridData->nOriginalX + pCol->nWidth -2, 
		       pRow->y - pGridData->nOriginalY + pRow->nHeight - 2);
		LineTo(hdc, pCol->x - pGridData->nOriginalX, pRow->y - pGridData->nOriginalY + pRow->nHeight - 2);
		LineTo(hdc, pCol->x - pGridData->nOriginalX, pRow->y - pGridData->nOriginalY);
		
	      }
	      else
		FillBox (hdc,  pCol->x - pGridData->nOriginalX+1, pRow->y - pGridData->nOriginalY+1,
			 pCol->nWidth - 2, pRow->nHeight - 2);
	      
	      if(pCell->pszInfo)
		GridDrawText (hdc, pCol->x - pGridData->nOriginalX+2, pRow->y - pGridData->nOriginalY + 2,
			      pCol->nWidth - 4, pRow->nHeight - 4, pCell->pszInfo, format);
	    }
	    else {
	      MoveWindow(pGridData->hwnd_edit, pCol->x - pGridData->nOriginalX + 2, pRow->y - pGridData->nOriginalY + 2,
			 pCol->nWidth - 4,
			 pRow->nHeight -4, TRUE);
	    }
	  }
	  pRow = pRow->pNext;
	  pCell = pCell->pNext;
	}
      }
      pCol = pCol->pNext;
    }
  }
  else { //row priorty
    while(pRow) {
      if((pRow->y + pRow->nHeight - pGridData->nOriginalY > pGridData->nHeadHeight) 
       || (pRow->y + pRow->nHeight - pGridData->nOriginalY < GetGridHeight(pGridData))) {      
	pCol = pGridData->pGridColHdr;
	pCell = pRow->pHeadCell;
	if (pRow->flags & GRIDHF_CENTERALIGN)
	  format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
	else if (pCol->flags & GRIDHF_RIGHTALIGN)
	  format = DT_SINGLELINE | DT_RIGHT | DT_VCENTER;
	else
	  format = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
	if (pRow != pGridData->pRowSelected) {
	  SetBrushColor (hdc, GetWindowBkColor(pGridData->hWnd));
	  SetBkColor (hdc, GetWindowBkColor(pGridData->hWnd));
	  SetTextColor (hdc, pCell->nTextColor);
	}
	else {
	  SetBkColor (hdc, pGridData->bkc_selected);
	  SetBrushColor (hdc, pGridData->bkc_selected);
	  SetTextColor (hdc, PIXEL_lightwhite);
	}
	while(pCol) {
	  if((pCol->x +pCol->nWidth - pGridData->nOriginalX > pGridData->nHeadWidth) 
	     || (pCol->x +pCol->nWidth - pGridData->nOriginalX < GetGridWidth(pGridData))) {
	    //check pcell flags here
	    
	    if(pCell != pGridData->pCellEdit) {
	      if(pCell == pGridData->pCellFocus) {
		MoveTo(hdc, pCol->x - pGridData->nOriginalX, pRow->y - pGridData->nOriginalY);
		LineTo(hdc, pCol->x - pGridData->nOriginalX + pCol->nWidth -2, pRow->y - pGridData->nOriginalY);
		LineTo(hdc, pCol->x - pGridData->nOriginalX + pCol->nWidth -2, 
		       pRow->y - pGridData->nOriginalY + pRow->nHeight - 2);
		LineTo(hdc, pCol->x - pGridData->nOriginalX, pRow->y - pGridData->nOriginalY + pRow->nHeight - 2);
		LineTo(hdc, pCol->x - pGridData->nOriginalX, pRow->y - pGridData->nOriginalY);
	      }
	      else
		FillBox (hdc,  pCol->x - pGridData->nOriginalX+1, pRow->y - pGridData->nOriginalY+1,
			 pCol->nWidth - 2, pRow->nHeight - 2);
	      if(pCell->pszInfo)
		GridDrawText (hdc, pCol->x - pGridData->nOriginalX+2, pRow->y - pGridData->nOriginalY + 2,
			      pCol->nWidth - 4, pRow->nHeight - 4, pCell->pszInfo, format);
	    }
	    else {
	      MoveWindow(pGridData->hwnd_edit, pCol->x - pGridData->nOriginalX + 2, pRow->y - pGridData->nOriginalY + 2,
			 pCol->nWidth - 4,
			 pRow->nHeight -4, TRUE);
	    }
	  }
	  pCol = pCol->pNext;
	  pCell = pCell->pRight;
	}
      }
      pRow = pRow->pNext;
    }
  }
}
static void GridOnDraw (HWND hwnd, HDC hdc)
{
    RECT rcClient;
    PGRIDDATA pGridData;

    pGridData = (PGRIDDATA) GetWindowAdditionalData2 (hwnd);
    GetClientRect (hwnd, &rcClient);

    SetBkColor (hdc, PIXEL_lightwhite);
    SetBrushColor (hdc, PIXEL_lightwhite);
    FillBox (hdc, rcClient.left, rcClient.top, rcClient.right - rcClient.left,
             rcClient.bottom - rcClient.top);

    if(pGridData->pRowSelected)
      pGridData->flags &= ~GRID_COL_PRIORTY;
    else
      pGridData->flags |= GRID_COL_PRIORTY;

    // draws cells
    GridDrawCells(hwnd, hdc);
    // draws header
    GridDrawHeader(hwnd, hdc);
}
/***************************  scroll info  ***********************************/
static int GridSetVScrollInfo (PGRIDDATA pGridData)
{
  SCROLLINFO si;
  RECT rect;
  
  GetClientRect (pGridData->hWnd, &rect);
  
  if ((rect.bottom - rect.top) > GetGridHeight(pGridData))
  {
    ShowScrollBar (pGridData->hWnd, SB_VERT, FALSE);
    pGridData->nOriginalY = 0;
    
    return 0;
  }
  
  if (pGridData->nOriginalY < 0)
    pGridData->nOriginalY = 0;
  
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMax = GetGridHeight(pGridData) - pGridData->nHeadHeight;
  si.nMin = 0;
  si.nPage = rect.bottom - rect.top - pGridData->nHeadHeight;
  si.nPos = pGridData->nOriginalY;

  if (si.nPos > si.nMax - si.nPage) {
    si.nPos = si.nMax - si.nPage;
    pGridData->nOriginalY = si.nPos;
  }

  SetScrollInfo (pGridData->hWnd, SB_VERT, &si, TRUE);
  ShowScrollBar (pGridData->hWnd, SB_VERT, TRUE);

  return 0;
}

static int GridSetHScrollInfo (PGRIDDATA pGridData)
{
  SCROLLINFO si;
  RECT rect;
  
  GetClientRect (pGridData->hWnd, &rect);
  
  if ( !GRID_H_OUTWND(pGridData, rect) ) {
    ShowScrollBar (pGridData->hWnd, SB_HORZ, FALSE);
    return 0;
  }
  
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMax = GetGridWidth (pGridData) - pGridData->nHeadWidth;
  si.nMin = 0;
  si.nPage = rect.right - rect.left - pGridData->nHeadWidth;
  si.nPos = pGridData->nOriginalX;
  
  if (si.nPos > si.nMax - si.nPage) {
    si.nPos = si.nMax - si.nPage;
    pGridData->nOriginalX = si.nPos;
  }
  
  SetScrollInfo (pGridData->hWnd, SB_HORZ, &si, TRUE);
  ShowScrollBar (pGridData->hWnd, SB_HORZ, TRUE);
  
  return 0;
}

static void GridVScroll (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    int scrollHeight = 0;
    RECT rect;
    int scrollBoundMax;
    int scrollBoundMin;
    int vscroll = 0;
    PGRIDDATA pGridData;

    pGridData = (PGRIDDATA) GetWindowAdditionalData2 (hwnd);

    GetClientRect (hwnd, &rect);
    scrollBoundMax = GetGridHeight(pGridData) - (rect.bottom - rect.top);
    scrollBoundMin = 0;
    
    //decides the desired value to scroll
    if (wParam == SB_LINEUP || wParam == SB_LINEDOWN)
	vscroll = VSCROLL;
    else if (wParam == SB_PAGEUP || wParam == SB_PAGEDOWN)
      vscroll = rect.bottom - rect.top - pGridData->nHeadHeight;

    //scroll down
    if ( (wParam == SB_LINEDOWN || wParam == SB_PAGEDOWN) &&
                    pGridData->nOriginalY < scrollBoundMax )
    {
	if ((pGridData->nOriginalY + vscroll) > scrollBoundMax)
	{
	    scrollHeight = pGridData->nOriginalY - scrollBoundMax;
	    pGridData->nOriginalY = scrollBoundMax;
	}
	else
	{
	    pGridData->nOriginalY += vscroll;
	    scrollHeight = -vscroll;
	}
    }
    //scroll up
    else if ( (wParam == SB_LINEUP || wParam == SB_PAGEUP) &&
	            pGridData->nOriginalY > scrollBoundMin )
    {
	if ((pGridData->nOriginalY - vscroll) > scrollBoundMin)
	{
	    pGridData->nOriginalY -= vscroll;
	    scrollHeight = vscroll;
	}
	else
	{
	    scrollHeight = pGridData->nOriginalY - scrollBoundMin;
	    pGridData->nOriginalY = scrollBoundMin;
	}
    }
    //dragging
    else if ( wParam == SB_THUMBTRACK )
    {
	    int scrollNewPos = (int) lParam;

	    if (((scrollNewPos - pGridData->nOriginalY) < 5) &&
		  ((scrollNewPos - pGridData->nOriginalY) > -5) &&
		  (scrollNewPos > 5) && ((scrollBoundMax - scrollNewPos) > 5))
		return;

	    if ((scrollNewPos < pGridData->nOriginalY) && (scrollNewPos <= VSCROLL))
	    {
		scrollHeight = pGridData->nOriginalY - 0;
		pGridData->nOriginalY = 0;
	    }
	    else
	    {
		if ((scrollNewPos > pGridData->nOriginalY) && ((scrollBoundMax - scrollNewPos) < VSCROLL))
		{
		    scrollHeight = pGridData->nOriginalY - scrollBoundMax;
		    pGridData->nOriginalY = scrollBoundMax;
		}
		else
		{
		    scrollHeight = pGridData->nOriginalY - scrollNewPos;
		    pGridData->nOriginalY = scrollNewPos;
		}
	    }
    }

    if (scrollHeight != 0) {
	InvalidateRect (hwnd, NULL, FALSE);
        GridSetVScrollInfo (pGridData);
    }
}


static void GridHScroll (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    int scrollWidth = 0;
    int scrollBoundMax;
    int scrollBoundMin;
    RECT rect;
    int hscroll = 0;
    PGRIDDATA pGridData;

    pGridData = (PGRIDDATA) GetWindowAdditionalData2 (hwnd);

    GetClientRect (hwnd, &rect);
    scrollBoundMax = GetGridWidth (pGridData) - (rect.right - rect.left);
    scrollBoundMin = 0;

    //decides the desired value to scroll
    if (wParam == SB_LINERIGHT || wParam == SB_LINELEFT)
	hscroll = HSCROLL;
    else if (wParam == SB_PAGERIGHT || wParam == SB_PAGELEFT)
	hscroll = rect.right - rect.left - pGridData->nHeadWidth;

    //scroll right 
    if ( (wParam == SB_LINERIGHT || wParam == SB_PAGERIGHT) && 
		    pGridData->nOriginalX < scrollBoundMax )
    {
	if ((pGridData->nOriginalX + hscroll) > scrollBoundMax)
	{
	    scrollWidth = pGridData->nOriginalX - scrollBoundMax;
	    pGridData->nOriginalX = scrollBoundMax;
	}
	else
	{
	    pGridData->nOriginalX += hscroll;
	    scrollWidth = -hscroll;
	}
    }
    //scroll left 
    else if ( (wParam == SB_LINELEFT || wParam == SB_PAGELEFT) && 
		    pGridData->nOriginalX > scrollBoundMin)
    {
	if ((pGridData->nOriginalX - hscroll) > scrollBoundMin)
	{
	    pGridData->nOriginalX -= hscroll;
	    scrollWidth = hscroll;
	}
	else
	{
	    scrollWidth = pGridData->nOriginalX - scrollBoundMin;
	    pGridData->nOriginalX = scrollBoundMin;
	}
    }
    //draging
    else if (wParam == SB_THUMBTRACK)
    {
	int scrollNewPos = (int) lParam;

	if (((scrollNewPos - pGridData->nOriginalX) < HSCROLL) &&
		    ((scrollNewPos - pGridData->nOriginalX) > -HSCROLL) && (scrollNewPos > HSCROLL)
		    && ((scrollBoundMax - scrollNewPos) > HSCROLL))
		return;

	if ((scrollNewPos < pGridData->nOriginalX) && (scrollNewPos <= HSCROLL))
	{
		scrollWidth = pGridData->nOriginalX - 0;
		pGridData->nOriginalX = 0;
	}
	else
	{
		if ((scrollNewPos > pGridData->nOriginalX) && ((scrollBoundMax - scrollNewPos) < HSCROLL))
		{
		    scrollWidth = pGridData->nOriginalX - scrollBoundMax;
		    pGridData->nOriginalX = scrollBoundMax;
		}
		else
		{
		    scrollWidth = pGridData->nOriginalX - scrollNewPos;
		    pGridData->nOriginalX = scrollNewPos;
		}
	}
    }

    if (scrollWidth != 0) {
	InvalidateRect (hwnd, NULL, FALSE);
        GridSetHScrollInfo (pGridData);
    }
}
static int GridMakeCellVisible (HWND hwnd, PGRIDDATA pGridData, PCELLDATA pCell)
{
  //int scrollHeight = 0, scrollWidth = 0;
  RECT rect;
  int area_height, area_width;
  int temp = 0;
  PGRIDCOLHDR pCol = pCell->pColHdr;
  PGRIDROWHDR pRow = pCell->pRowHdr;
  
  GetClientRect (hwnd, &rect);
  area_height = rect.bottom - rect.top;// - pGridData->nHeadHeight;
  area_width = rect.right - rect.left;// - pGridData->nHeadWidth;
  
  if(pRow->y < pGridData->nOriginalY + pGridData->nHeadHeight) {
    pGridData->nOriginalY = pRow->y-pGridData->nHeadHeight;
    temp = 1;
  }
  else if((pRow->y + pRow->nHeight - pGridData->nOriginalY) > area_height) {
    pGridData->nOriginalY = pRow->y + pRow->nHeight - area_height;
    temp = 1;
  }
  if(temp)
    GridSetVScrollInfo(pGridData);
  
  if(pCol->x < pGridData->nOriginalX + pGridData->nHeadWidth) {
    pGridData->nOriginalX = pCol->x-pGridData->nHeadWidth;
    temp = 2;
  }
  else if((pCol->x + pCol->nWidth - pGridData->nOriginalX) > area_width) {
    pGridData->nOriginalX = pCol->x + pCol->nWidth - area_width;
    temp = 2;
  }
  if(temp == 2)
    GridSetHScrollInfo(pGridData);

  if(temp > 0) {
    InvalidateRect(hwnd, NULL, FALSE);
    return 1;
  }

  return -1;
}

static int sGridProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
  PGRIDDATA pgriddata = NULL;
  DWORD dwStyle;
  int mouseX, mouseY;
  int nCols, nRows;

  if(message != MSG_CREATE)
    pgriddata = (PGRIDDATA)GetWindowAdditionalData2(hwnd);

  switch(message) {
  case MSG_CREATE:
    dwStyle = GetWindowStyle(hwnd);

    if((pgriddata = NewGrid(hwnd, 1, 1)) == NULL)
      return -1;

    SetWindowAdditionalData2(hwnd, (DWORD)pgriddata);
    
    GridSetHScrollInfo(pgriddata);
    GridSetVScrollInfo(pgriddata);

    break;

  case MSG_SETCURSOR:
    mouseX = LOSWORD(lParam);
    mouseY = HISWORD(lParam);
            
    if ((GRIDSTATUS(hwnd) & GRID_BDDRAG) || 
	(GridInWhichColHdrBorder (mouseX, mouseY, NULL, pgriddata) > 0)) {
      SetCursor (GetSystemCursor (IDC_SPLIT_VERT));

      return 0;
    }
    break;

  case MSG_PAINT:
    {
      HDC hdc;
      
      hdc = BeginPaint(hwnd);
      GridOnDraw(hwnd, hdc);
      EndPaint(hwnd, hdc);
      
      return 0;
    }

  case MSG_SETFOCUS:
  case MSG_KILLFOCUS:
    /*if(message == MSG_SETFOCUS)
      pgriddata->bkc_selected = LIGHTBLUE;
    else
      pgriddata->bkc_selected = GRAYBLUE;
    if(pgriddata->nColSelected > 0) {
      RECT rc;
      GRID_GET_CELL_RECT(pgriddata->nColSelected, rc);
      InvalidateRect(hwnd, &rc, FALSE);
      }*/
    break;
    
    //case MSG_GET_DLGCODE:
    //return DLGC_WANTARROWS | DLGC_WANTCHARS;
    
  case MSG_MOUSEMOVE:
    mouseX = LOSWORD(lParam);
    mouseY = HISWORD(lParam);

    if(GRIDSTATUS(hwnd) & GRIDST_BDDRAG) {
      if(pgriddata->pColDraged)
	GridColBorderDrag(hwnd, mouseX, mouseY);
      if(pgriddata->pRowDraged)
	GridRowBorderDrag(hwnd, mouseX, mouseY);
    }
    break;

  case MSG_LBUTTONDOWN:
    {
      RECT rcClient;
      PGRIDCOLHDR pColHdr;
      PGRIDROWHDR pRowHdr;
      mouseX = LOSWORD (lParam);
      mouseY = HISWORD (lParam);

      GetClientRect(hwnd, &rcClient);
      /*      nCols = IsInGridColHdr(mouseX, mouseY, &pColHdr, pgriddata);
      nRows = IsInGridRowHdr(mouseX, mouseY, &pRowHdr, pgriddata);
      
      if(nCols > 0) {
	if(GetCapture() == hwnd)
	  break;
	
	SetCapture(hwnd);
	GRIDSTATUS(hwnd) |= (GRIDST_HEADSELECT | GRIDST_INHEAD);
	pgriddata->pColSelected = pColHdr;
	
	SetRect(&rect, pColHdr->x - pgriddata->nOriginalX, GRID_HDR_TOP,
		pColHdr->x - pgriddata->nOriginalX + pColHdr->nWidth,
		GRID_HDR_TOP+ GRID_HDRH_DEF(hwnd));
	InvalidateRect(hwnd, &rect, FALSE);
	}*/
      //else 
      {
	if((nCols = GridInWhichColHdrBorder(mouseX, mouseY, &pColHdr, pgriddata)) >= 0) {
	  GRIDSTATUS(hwnd) |= GRIDST_BDDRAG;
	  pgriddata->pColDraged = pColHdr;
	  pgriddata->pRowDraged = NULL;
	  SetCapture(hwnd);
	}
      }
      /*if(nRows > 0) {
	if(GetCapture() == hwnd)
	  break;
	
	SetCapture(hwnd);
	GRIDSTATUS(hwnd) |= (GRIDST_HEADSELECT | GRIDST_INHEAD);
	pgriddata->pRowSelected = pRowHdr;
	
	SetRect(&rect, GRID_HDR_LEFT, pRowHdr->y - pgriddata->nOriginalY,
		GRID_HDR_LEFT+ GRID_HDRW_DEF(hwnd),
		pRowHdr->y - pgriddata->nOriginalY + pRowHdr->nHeight);
	InvalidateRect(hwnd, &rect, FALSE);
      }
      else */
      {
	if((nRows = GridInWhichRowHdrBorder(mouseX, mouseY, &pRowHdr, pgriddata)) >= 0) {
	  GRIDSTATUS(hwnd) |= GRIDST_BDDRAG;
	  pgriddata->pRowDraged = pRowHdr;
	  pgriddata->pColDraged = NULL;
	  SetCapture(hwnd);
	}
      }
      break;
    }
  case MSG_LBUTTONUP: {
    PGRIDCOLHDR pColHdr;
    PGRIDROWHDR pRowHdr;
    PCELLDATA pCell;
    int nCells = -1;
    nCols = -1;
    nRows = -1;
    mouseX = LOSWORD (lParam);
    mouseY = HISWORD (lParam);
    
    if(!(GRIDSTATUS(hwnd) & GRIDST_BDDRAG)) {
      nCols = IsInGridColHdr(mouseX, mouseY, &pColHdr, pgriddata);
      nRows = IsInGridRowHdr(mouseX, mouseY, &pRowHdr, pgriddata);
      nCells = IsInGridCell(mouseX, mouseY, &pCell, pgriddata); 
    }

    //in col head
    if(nCols >= 0) {
      if(GRIDSTATUS(hwnd) & GRIDST_HEADSELECT) {
	if(pgriddata->pColSelected != pColHdr) {
	  GRIDSTATUS(hwnd) |= (GRIDST_HEADSELECT);
	  pgriddata->pColSelected = pColHdr;
	  pgriddata->pRowSelected = NULL;
	  //printf("%s\n", pgriddata->pColSelected->pTitle);
	}
	else {
	  GRIDSTATUS(hwnd) &= ~GRIDST_HEADSELECT;
	  pgriddata->pColSelected = NULL;
	}
      }
      else {
	GRIDSTATUS(hwnd) |= (GRIDST_HEADSELECT);
	pgriddata->pColSelected = pColHdr;
	pgriddata->pRowSelected = NULL;
	//printf("%s\n", pgriddata->pColSelected->pTitle);
      }
    }
    //in row head
    else if(nRows >= 0) {
      if(GRIDSTATUS(hwnd) & GRIDST_HEADSELECT) {
	if(pgriddata->pRowSelected != pRowHdr) {
	  GRIDSTATUS(hwnd) |= (GRIDST_HEADSELECT);
	  pgriddata->pRowSelected = pRowHdr;
	  pgriddata->pColSelected = NULL;
	  //printf("%s\n", pgriddata->pRowSelected->pTitle);
	}
	else {
	  GRIDSTATUS(hwnd) &= ~GRIDST_HEADSELECT;
	  pgriddata->pRowSelected = NULL;
	}
      }
      else {
	GRIDSTATUS(hwnd) |= (GRIDST_HEADSELECT);
	pgriddata->pRowSelected = pRowHdr;
	pgriddata->pColSelected = NULL;
	//printf("%s\n", pgriddata->pRowSelected->pTitle);
      }
    }
    //in cell
    else if(nCells > 0) {
      pgriddata->pCellFocus = pCell;

      if(pgriddata->hwnd_edit && pgriddata->pCellEdit != pCell)
	PostMessage(hwnd, GRID_EDITENTER, 1, (DWORD)pCell);
      GRIDSTATUS(hwnd) &= ~GRIDST_HEADSELECT;
      pgriddata->pColSelected = NULL;
      pgriddata->pRowSelected = NULL;
      GridMakeCellVisible(hwnd, pgriddata, pCell); 
    }      
    if (GRIDSTATUS(hwnd) & GRIDST_BDDRAG) {
      ReleaseCapture ();
      if(pgriddata->pColDraged) {
	GridSetHScrollInfo (pgriddata);
	pgriddata->pColDraged = NULL;
      }
      if(pgriddata->pRowDraged) {
	GridSetVScrollInfo(pgriddata);
	pgriddata->pRowDraged = NULL;
      }
      GRIDSTATUS(hwnd) &= ~GRIDST_BDDRAG;
    } 
    InvalidateRect (hwnd, NULL, FALSE);
  }
    break;

  case MSG_LBUTTONDBLCLK: {
    PCELLDATA pCell;
    mouseX = LOSWORD (lParam);
    mouseY = HISWORD (lParam);

    if(IsInGridCell(mouseX, mouseY, &pCell, pgriddata) > 0) {
      if(pgriddata->pCellEdit) {
	free(pgriddata->pCellEdit->pszInfo);
	pgriddata->pCellEdit->pszInfo = malloc(GetWindowTextLength(pgriddata->hwnd_edit)+1);
	GetWindowText(pgriddata->hwnd_edit, pgriddata->pCellEdit->pszInfo, 256);
	pgriddata->pCellEdit = NULL;
	DestroyWindow(pgriddata->hwnd_edit);
      }
      pgriddata->pCellEdit = pCell;
      pgriddata->hwnd_edit = CreateWindow (CTRL_EDIT,
					   pCell->pszInfo,
					   WS_CHILD | WS_VISIBLE | WS_BORDER,
					   IDC_EDIT,
					   pCell->pColHdr->x - pgriddata->nOriginalX + 3,
					   pCell->pRowHdr->y - pgriddata->nOriginalY + 3,
					   pCell->pColHdr->nWidth - 5,
					   pCell->pRowHdr->nHeight - 5, hwnd, 0);
      SetFocusChild(pgriddata->hwnd_edit);
      SetWindowAdditionalData(pgriddata->hwnd_edit, (DWORD)pgriddata);
      old_edit_proc = SetWindowCallbackProc(pgriddata->hwnd_edit, EnterEditBox);
    }
  }
    break;
  case GRID_EDITENTER: {
    if(pgriddata->pCellEdit) {
      free(pgriddata->pCellEdit->pszInfo);
      pgriddata->pCellEdit->pszInfo = malloc(GetWindowTextLength(pgriddata->hwnd_edit)+1);
      GetWindowText(pgriddata->hwnd_edit, pgriddata->pCellEdit->pszInfo, 256);
      //sprintf(pgriddata->pCellEdit->pszInfo, "%s", );
      if(wParam > 0)
	pgriddata->pCellFocus = (PCELLDATA)lParam;
      else {
	if(pgriddata->pCellEdit->pNext)
	  pgriddata->pCellFocus = pgriddata->pCellEdit->pNext;
      }
      pgriddata->pCellEdit = NULL;
      DestroyWindow(pgriddata->hwnd_edit);
      InvalidateRect(hwnd, NULL, FALSE);
    }
  }
    break;
  case MSG_KEYDOWN: {
    int id = LOWORD (wParam);
    
    switch(id) {
    case SCANCODE_CURSORBLOCKDOWN:
      if(pgriddata->pCellFocus) {
	if(pgriddata->pCellFocus->pNext) {
	  pgriddata->pCellFocus = pgriddata->pCellFocus->pNext;
	  if(pgriddata->pCellEdit)
	    PostMessage(hwnd, GRID_EDITENTER, 1, (DWORD)pgriddata->pCellEdit->pNext);
	}
	if(GridMakeCellVisible(hwnd, pgriddata, pgriddata->pCellFocus) < 0)
	  InvalidateRect(hwnd, NULL, FALSE);
      }
      break;
    case SCANCODE_CURSORBLOCKUP: {
      PCELLDATA pCell;
      if(pgriddata->pCellFocus) {
	pCell = pgriddata->pCellFocus->pColHdr->pHeadCell;
	while(pCell) {
	  if(pCell->pNext == pgriddata->pCellFocus) {
	    pgriddata->pCellFocus = pCell;
	    if(pgriddata->pCellEdit)
	      PostMessage(hwnd, GRID_EDITENTER, 1, (DWORD)pCell);
	    if(GridMakeCellVisible(hwnd, pgriddata, pCell) < 0)
	      InvalidateRect(hwnd, NULL, FALSE);
	    break;
	  }
	  else
	    pCell = pCell->pNext;
	}
      }
    }
      break;
    case SCANCODE_HOME:
      break;
    case SCANCODE_END:
      if(pgriddata->pCellFocus) {
	while(pgriddata->pCellFocus->pNext)
	  pgriddata->pCellFocus = pgriddata->pCellFocus->pNext;
      }
      //make cell visible
      if(GridMakeCellVisible(hwnd, pgriddata, pgriddata->pCellFocus) < 0)
	      InvalidateRect(hwnd, NULL, FALSE);
      break;
    case SCANCODE_CURSORBLOCKLEFT: {
      PCELLDATA pCell;
      if(pgriddata->pCellEdit)
	break;
      if(pgriddata->pCellFocus) {
	pCell = pgriddata->pCellFocus->pRowHdr->pHeadCell;
	while(pCell) {
	  if(pCell->pRight == pgriddata->pCellFocus) {
	    pgriddata->pCellFocus = pCell;
	    if(pgriddata->hwnd_edit)
	      DestroyWindow(pgriddata->hwnd_edit);
	    if(GridMakeCellVisible(hwnd, pgriddata, pCell) < 0)
	      InvalidateRect(hwnd, NULL, FALSE);
	    break;
	  }
	  else
	    pCell = pCell->pRight;
	}
      }
    }
      break;
    case SCANCODE_CURSORBLOCKRIGHT:
      if(pgriddata->pCellEdit)
	break;
      if(pgriddata->pCellFocus) {
	if(pgriddata->pCellFocus->pRight) {
	  pgriddata->pCellFocus = pgriddata->pCellFocus->pRight;
	  if(pgriddata->hwnd_edit)
	    DestroyWindow(pgriddata->hwnd_edit);
	}
	if(GridMakeCellVisible(hwnd, pgriddata, pgriddata->pCellFocus) < 0)
	  InvalidateRect(hwnd, NULL, FALSE);
      }
      break;
    case SCANCODE_PAGEUP:
      SendMessage (hwnd, MSG_VSCROLL, SB_PAGEUP, 0);
      break;
    case SCANCODE_PAGEDOWN:
      SendMessage (hwnd, MSG_VSCROLL, SB_PAGEDOWN, 0);
      break;
    }
      break;
    }
    
  case MSG_VSCROLL:
    GridVScroll (hwnd, wParam, lParam);
    return 0;
    
  case MSG_HSCROLL:
    GridHScroll (hwnd, wParam, lParam);
    return 0;

  case GRIDM_GETGRIDCOLS:
    return pgriddata->nCols;
  case GRIDM_SETGRIDCOLS:
    if((wParam >= pgriddata->nCols) && (wParam <= GRID_COL_MAX)) {
      while(pgriddata->nCols < wParam) {
	AddColTail(hwnd, pgriddata);
      }
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;
  case GRIDM_GETGRIDROWS:
    return pgriddata->nRows;
  case GRIDM_SETGRIDROWS:
    if((wParam >= pgriddata->nRows) && (wParam <= GRID_ROW_MAX)) {
      while(pgriddata->nRows < wParam) {
	AddRowTail(hwnd, pgriddata);
      }
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;

  case GRIDM_ADDCOL:
    if(AddCol(hwnd, (PGRIDCOLHEADER)lParam, pgriddata)) {
      GridSetHScrollInfo(pgriddata);
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;
    
  case GRIDM_DELCOL:
    if(DeleteCol(wParam, pgriddata) >0) {
      GridSetHScrollInfo(pgriddata);
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;

  case GRIDM_ADDROW:
    if(AddRow(hwnd, (PGRIDROWHEADER)lParam, pgriddata)) {
      GridSetVScrollInfo(pgriddata);
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;
    
  case GRIDM_DELROW:
    if(DeleteRow(wParam, pgriddata) >0) {
      GridSetVScrollInfo(pgriddata);
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;
    
  case GRIDM_SETHEADWIDTH:
    if(wParam > COLWIDTHMIN) {
      pgriddata->nHeadWidth = wParam;
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;
  case GRIDM_SETHEADHEIGHT:
    if(wParam > ROWHEIGHTMIN) {
      pgriddata->nHeadHeight = wParam;
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;
  case GRIDM_GETHEADWIDTH:
    return pgriddata->nHeadWidth;
  case GRIDM_GETHEADHEIGHT:
    return pgriddata->nHeadHeight;
    
  case GRIDM_SETCOLDATA: {
    PGRIDCOLHDR pColHdr;
    
    if(lParam) {
      pColHdr = GetColHdrByNum(((PGRIDCOLHEADER)lParam)->nCols, pgriddata);
      if(pColHdr) {
	GridSetColWidth(pColHdr, ((PGRIDCOLHEADER)lParam)->nWidth, pgriddata);
	if(((PGRIDCOLHEADER)lParam)->pTitle) {
	  if(pColHdr->pTitle) free(pColHdr->pTitle);
	  pColHdr->pTitle = malloc(strlen(((PGRIDCOLHEADER)lParam)->pTitle));
	  sprintf(pColHdr->pTitle, "%s", ((PGRIDCOLHEADER)lParam)->pTitle);
	}
	pColHdr->flags = ((PGRIDCOLHEADER)lParam)->flags;
	pColHdr->Image = ((PGRIDCOLHEADER)lParam)->image;
	//pColHdr->cellflags = ((PGRIDCOLHEADER)lParam)->cellflags;
	return GRID_OKAY;
      }
    }
    return GRID_ERR;
  }
  case GRIDM_SETROWDATA: {
    PGRIDROWHDR pRowHdr;
    
    if(lParam) {
      pRowHdr = GetRowHdrByNum(((PGRIDROWHEADER)lParam)->nRows, pgriddata);
      if(pRowHdr) {
	GridSetRowHeight(pRowHdr, ((PGRIDROWHEADER)lParam)->nHeight, pgriddata);
	if(((PGRIDROWHEADER)lParam)->pTitle) {
	  if(pRowHdr->pTitle) free(pRowHdr->pTitle);
	  pRowHdr->pTitle = malloc(strlen(((PGRIDROWHEADER)lParam)->pTitle));
	  sprintf(pRowHdr->pTitle, "%s", ((PGRIDROWHEADER)lParam)->pTitle);
	}
	pRowHdr->flags = ((PGRIDROWHEADER)lParam)->flags;
	pRowHdr->Image = ((PGRIDROWHEADER)lParam)->image;
	//pRowHdr->cellflags = ((PGRIDROWHEADER)lParam)->cellflags;
	return GRID_OKAY;
      }
    }
    return GRID_ERR;
  }
    break;
 
  case GRIDM_SETCOLWIDTH:
    if(GridSetColWidth(GetColHdrByNum(wParam, pgriddata), wParam, pgriddata) > 0) {
      InvalidateRect(hwnd, NULL, FALSE);
      GridSetHScrollInfo(pgriddata);
      return GRID_OKAY;
    }
    return GRID_ERR;
  case GRIDM_SETROWHEIGHT:
    if(GridSetRowHeight(GetRowHdrByNum(wParam, pgriddata), wParam, pgriddata) > 0) {
      InvalidateRect(hwnd, NULL, FALSE);
      GridSetVScrollInfo(pgriddata);
      return GRID_OKAY;
    }
    return GRID_ERR;
    
  case GRIDM_SETCELLFOCUS: {
    PCELLDATA pCell = GetCellByNum(wParam, lParam, pgriddata);
    if(pCell) {
      pgriddata->pCellFocus = pCell;
      InvalidateRect(hwnd, NULL, FALSE);
      return GRID_OKAY;
    }
    return GRID_ERR;
  }
    break;

  case MSG_DESTROY:
    DeleteGrid (pgriddata);
    break;
  }
  return DefaultControlProc (hwnd, message, wParam, lParam);
}

//***********************************************************************************
BOOL RegisterGridControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = CTRL_GRID;
    WndClass.dwStyle = WS_NONE;
    WndClass.dwExStyle = WS_EX_NONE;
    WndClass.hCursor = GetSystemCursor (0);
    WndClass.iBkColor = PIXEL_lightwhite;
    WndClass.WinProc = sGridProc;

    return RegisterWindowClass (&WndClass);
}

void GridControlCleanup (void)
{
    UnregisterWindowClass (CTRL_GRID);
}

#endif /* _EXT_CTRL_GRID */
