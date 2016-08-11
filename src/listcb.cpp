#include "stdafx.h"
#include "listcb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_TOSTORE 10
/////////////////////////////////////////////////////////////////////////////
// CListCombo

CListCombo::CListCombo()
{
	m_sSect=_T("Combos");
	m_bCaseSens=FALSE;
	m_bDirty = FALSE;
	m_bAllowEmpty = FALSE;
}

CListCombo::~CListCombo()
{
}
void CListCombo::SetName(LPCTSTR IniKey, LPCTSTR IniSect)
{
	if (IniSect)
		m_sSect=IniSect;
	m_sKey=IniKey;
	CString key=_T("Combo_");
	m_sKey=key+m_sKey;
	Reload();
}


BEGIN_MESSAGE_MAP(CListCombo, CComboBox)
	//{{AFX_MSG_MAP(CListCombo)
	ON_CONTROL_REFLECT(CBN_KILLFOCUS, OnLostfocus)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCombo message handlers

int CListCombo::SearchString(LPCTSTR s)
{
	int n=GetCount();
	CString	str;
	for (int i=0; i<n; i++)
	{
		GetLBText(i, str);
		BOOL bEq=0;
		if (m_bCaseSens)
			bEq=(str==s);
		else
			bEq=(str.CompareNoCase(s)==0);
		if (bEq)
			return i;
	}
	return CB_ERR;
}

void CListCombo::OnLostfocus()
{
	CString Txt;
	GetWindowText(Txt);
	if (Txt.IsEmpty())
		return;
	int idx=SearchString(Txt);
	if (idx==CB_ERR)
	{
		InsertString(0, Txt);
		m_bDirty = TRUE;
	}
	else
		SetCurSel(idx);
}
void CListCombo::OnDestroy()
{
	if (!m_sKey.IsEmpty())	// save state
	{
		Save();
	}
	CComboBox::OnDestroy();
}
void CListCombo::Update()
{
	OnLostfocus();
}

BOOL CListCombo::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_KEYDOWN)
	{
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			switch (pMsg->wParam)
			{
			case 'V':
				Paste();
				return TRUE;
			case 'C':
			case VK_INSERT:
				Copy();
				return TRUE;
			case 'X':
				Cut();
				return TRUE;
			}
		}
		else if ((GetKeyState(VK_SHIFT) & 0x8000) &&
			pMsg->wParam==VK_INSERT)
		{
			Paste();
			return TRUE;
		}
	}
	return CComboBox::PreTranslateMessage(pMsg);
}
void CListCombo::Save()
{
	OnLostfocus();
	if (m_sKey.IsEmpty())
		return;
	CString key;
	key=m_sKey+_T("!");
	int c=GetCurSel();
	if (c<0 && !m_bAllowEmpty)
		c=0;
	AfxGetApp()->WriteProfileInt(m_sSect, key, c);	// current
	if (!m_bDirty)
		return;
	c=GetCount();
	if (c>MAX_TOSTORE)
		c=MAX_TOSTORE;
	AfxGetApp()->WriteProfileInt(m_sSect, m_sKey, c);	// total
	TCHAR buf[10];
	for (int i=0; i<c; i++)
	{
		_itot(i, buf, 10);
		key=m_sKey+buf;
		CString txt;
		GetLBText(i, txt);
		// keep last/first quotes
		if (txt.Find('\"') >= 0)
		{
			CString sNew;
			sNew.Format(_T("<%s>"), txt);
			txt = sNew;
		}
		AfxGetApp()->WriteProfileString(m_sSect, key, txt);	// list
	}
}
void CListCombo::Reload()
{
	// load state
	ResetContent();
	if (m_sKey.IsEmpty())
		return;

	int c=AfxGetApp()->GetProfileInt(m_sSect, m_sKey, 0);
	TCHAR buf[10];
	CString key;
	for (int i=0; i<c; i++)
	{
		_itot(i, buf, 10);
		key=m_sKey+buf;
		CString txt=AfxGetApp()->GetProfileString(m_sSect, key, NULL);
		// keep last/first quotes
		if (txt.Find('\"') >= 0 && txt[0]=='<' && txt[txt.GetLength()-1]=='>')
		{
			CString sNew = txt.Mid(1, txt.GetLength()-2);
			txt = sNew;
		}
		AddString(txt);
	}
	key=m_sKey+_T("!");
	c=AfxGetApp()->GetProfileInt(m_sSect, key, -1);	// current
	SetCurSel(c);
	m_bDirty = FALSE;
}


