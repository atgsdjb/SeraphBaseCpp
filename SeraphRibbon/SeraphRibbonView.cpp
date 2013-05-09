// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// SeraphRibbonView.cpp : implementation of the CSeraphRibbonView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SeraphRibbon.h"
#endif

#include "SeraphRibbonDoc.h"
#include "SeraphRibbonView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSeraphRibbonView

IMPLEMENT_DYNCREATE(CSeraphRibbonView, CView)

BEGIN_MESSAGE_MAP(CSeraphRibbonView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSeraphRibbonView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CSeraphRibbonView construction/destruction

CSeraphRibbonView::CSeraphRibbonView()
{
	// TODO: add construction code here

}

CSeraphRibbonView::~CSeraphRibbonView()
{
}

BOOL CSeraphRibbonView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSeraphRibbonView drawing

void CSeraphRibbonView::OnDraw(CDC* /*pDC*/)
{
	CSeraphRibbonDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CSeraphRibbonView printing


void CSeraphRibbonView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSeraphRibbonView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSeraphRibbonView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSeraphRibbonView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CSeraphRibbonView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSeraphRibbonView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CSeraphRibbonView diagnostics

#ifdef _DEBUG
void CSeraphRibbonView::AssertValid() const
{
	CView::AssertValid();
}

void CSeraphRibbonView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSeraphRibbonDoc* CSeraphRibbonView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSeraphRibbonDoc)));
	return (CSeraphRibbonDoc*)m_pDocument;
}
#endif //_DEBUG


// CSeraphRibbonView message handlers
