
// SeraphMFCView.h : interface of the CSeraphMFCView class
//

#pragma once


class CSeraphMFCView : public CListView
{
protected: // create from serialization only
	CSeraphMFCView();
	DECLARE_DYNCREATE(CSeraphMFCView)

// Attributes
public:
	CSeraphMFCDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CSeraphMFCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SeraphMFCView.cpp
inline CSeraphMFCDoc* CSeraphMFCView::GetDocument() const
   { return reinterpret_cast<CSeraphMFCDoc*>(m_pDocument); }
#endif

