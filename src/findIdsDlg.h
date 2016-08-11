
// findIdsDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "listcb.h"
#include "etslayout.h"

struct CDef
{
	CDef() {nMask = 0; nUsages=0;};
	CString sDef;
	CString sSource;
	int	nMask;
	int nUsages;
};
// CfindIdsDlg dialog
class CfindIdsDlg : public ETSLayoutDialog
{
// Construction
public:
	CfindIdsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FINDIDS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();

	void AddRules();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CArray<CDef> m_aDefines;

	CListCombo m_lbFolder;
	CString m_sFolder;
	afx_msg void OnEnChangeRes();
	CString m_sFilterRes;
	CString m_sFilterSrc;
	afx_msg void OnBnClickedOk();

	void FilterFoundDefines();

	void ScanFile(LPCTSTR sFile, LPCTSTR sShort);
	void SearchFile(LPCTSTR sFile);
	void SetStatus(LPCTSTR sText);
	void SetSubStatus(LPCTSTR sText);
	CListBox m_lbResult;
	CString m_sMask;
	afx_msg void OnLbnDblclkResult();
	afx_msg void OnBnClickedFolderBrs();
};
