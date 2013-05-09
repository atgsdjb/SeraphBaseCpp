
// SeraphMFCView.cpp : implementation of the CSeraphMFCView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SeraphMFC.h"
#endif

#include "SeraphMFCDoc.h"
#include "SeraphMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSeraphMFCView

IMPLEMENT_DYNCREATE(CSeraphMFCView, CListView)

BEGIN_MESSAGE_MAP(CSeraphMFCView, CListView)
	ON_WM_STYLECHANGED()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CSeraphMFCView construction/destruction

CSeraphMFCView::CSeraphMFCView()
{
	// TODO: add construction code here

}

CSeraphMFCView::~CSeraphMFCView()
{
}

BOOL CSeraphMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

void CSeraphMFCView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}

void CSeraphMFCView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSeraphMFCView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CSeraphMFCView diagnostics

#ifdef _DEBUG
void CSeraphMFCView::AssertValid() const
{
	CListView::AssertValid();
}

void CSeraphMFCView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CSeraphMFCDoc* CSeraphMFCView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSeraphMFCDoc)));
	return (CSeraphMFCDoc*)m_pDocument;
}
#endif //_DEBUG


// CSeraphMFCView message handlers
void CSeraphMFCView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window	
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);	
}
