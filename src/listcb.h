#pragma once

class CListCombo : public CComboBox
{
public:
	CListCombo();
	void SetName(LPCTSTR IniKey, LPCTSTR IniSection=NULL);
	void SetIni(LPCTSTR Ini) {m_sIni=Ini;};
	void Update();

	void Save();
	void Reload();

	BOOL	m_bCaseSens;
	BOOL	m_bDirty;
	BOOL	m_bAllowEmpty;	// allow empty item to be restored
	//{{AFX_VIRTUAL(CListCombo)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	virtual ~CListCombo();

protected:
	int SearchString(LPCTSTR s);
	CString m_sKey;
	CString	m_sSect;
	CString	m_sIni;
	//{{AFX_MSG(CListCombo)
	afx_msg void OnLostfocus();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


