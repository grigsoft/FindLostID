
// findIdsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "findIds.h"
#include "findIdsDlg.h"
#include "afxdialogex.h"
#include "ufile.h"
#include "fileiter.h"
#include "folderdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CfindIdsDlg dialog
CfindIdsDlg::CfindIdsDlg(CWnd* pParent /*=NULL*/)
	: ETSLayoutDialog(CfindIdsDlg::IDD, pParent, _T("FindIDs"))
	, m_sFolder(_T(""))
	, m_sFilterRes(_T("resource.h; *res.h"))
	, m_sFilterSrc(_T("*.cpp; *.rc; *.h"))
	, m_sMask(_T("IDS_*"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CfindIdsDlg::DoDataExchange(CDataExchange* pDX)
{
	ETSLayoutDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RESULT, m_lbResult);
	DDX_Control(pDX, IDC_FOLDER, m_lbFolder);
	DDX_Text(pDX, IDC_FOLDER, m_sFolder);
	DDX_Text(pDX, IDC_RES, m_sFilterRes);
	DDX_Text(pDX, IDC_SCAN, m_sFilterSrc);
	DDX_Text(pDX, IDC_MASK, m_sMask);
}

BEGIN_MESSAGE_MAP(CfindIdsDlg, ETSLayoutDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_RES, &CfindIdsDlg::OnEnChangeRes)
	ON_BN_CLICKED(IDOK, &CfindIdsDlg::OnBnClickedOk)
	ON_LBN_DBLCLK(IDC_RESULT, &CfindIdsDlg::OnLbnDblclkResult)
	ON_BN_CLICKED(IDC_FOLDER_BRS, &CfindIdsDlg::OnBnClickedFolderBrs)
END_MESSAGE_MAP()


// CfindIdsDlg message handlers

BOOL CfindIdsDlg::OnInitDialog()
{
	ETSLayoutDialog::OnInitDialog();

	m_lbFolder.SetName(_T("list"));

	AddRules();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	UpdateLayout();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CfindIdsDlg::AddRules()
{
	CreateRoot(VERTICAL)
		<< ( pane(HORIZONTAL, ABSOLUTE_VERT)
			<< item(IDC_S_FOLDER, NORESIZE)
			<< item(IDC_FOLDER, ABSOLUTE_VERT)
			<< item(IDC_FOLDER_BRS, NORESIZE)
		)
		<< ( pane(HORIZONTAL, ABSOLUTE_VERT)
			<< item(IDC_S_RES, NORESIZE)
			<< item(IDC_RES, ABSOLUTE_VERT)
		)
		<< ( pane(HORIZONTAL, ABSOLUTE_VERT)
			<< item(IDC_S_SRC, NORESIZE)
			<< item(IDC_SCAN, ABSOLUTE_VERT)
		)
		<< ( pane(HORIZONTAL, ABSOLUTE_VERT)
			<< item(IDC_S_MASK, NORESIZE)
			<< item(IDC_MASK, ABSOLUTE_VERT)
		)
		<< item(IDC_RESULT, GREEDY)
		<< ( pane(HORIZONTAL, ABSOLUTE_VERT)
				<< ( pane(VERTICAL, ABSOLUTE_VERT)
				<< item(IDC_STATUS, ABSOLUTE_VERT)
				<< item(IDC_STATUS2, ABSOLUTE_VERT)
				)
			<< item(IDOK, NORESIZE)
			<< item(IDCANCEL, NORESIZE)
		);
}

void CfindIdsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		ETSLayoutDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CfindIdsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		ETSLayoutDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CfindIdsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CfindIdsDlg::OnEnChangeRes()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the ETSLayoutDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
#define SDEFINE _T("#define")
const int nDefLen = _tcslen(SDEFINE);
BOOL IsValidChar(TCHAR ch)
{
	return (_istalnum(ch) || ch =='_');
}
void CfindIdsDlg::ScanFile(LPCTSTR sFile, LPCTSTR sShort)
{
	CUnicodeFile file;
	if (!file.SafeOpen(sFile))
		return;
	CString s;
	while (file.ReadString(s))
	{
		// is it define?
		if (s.GetLength()<nDefLen || s[0]!='#')
			continue;
		if (_tcsncmp(s, SDEFINE, nDefLen)!=0)
			continue;
		LPCTSTR sBuf = (LPCTSTR)s + nDefLen;
		while (*sBuf==' ' || *sBuf=='\t') sBuf++;
		LPCTSTR sStart = sBuf;
		while (IsValidChar(*sBuf)) sBuf++;
		CString sDef(sStart, sBuf-sStart);
		CDef def;
		def.sDef = sDef;
		def.sSource = sShort;
		def.nMask = 0;
		m_aDefines.Add(def);
	}
	file.Close();
}

void CfindIdsDlg::SearchFile(LPCTSTR sFile)
{
	CUnicodeFile file;
	if (!file.SafeOpen(sFile))
		return;
	TRACE(_T("scanning %s\n"), sFile);
	CString s;
	// read file into array
	CStringArray asText;
	while (file.ReadString(s))
	{
		// is it define?
		if (s.GetLength()<2)
			continue;
		s.Trim();
		if (s.GetLength()<2)
			continue;
		asText.Add(s);
	}
	file.Close();
	// search all defines
	for (int i=0; i<m_aDefines.GetSize(); i++)
	{
		if (m_aDefines[i].nUsages>2)
			continue;
		for (int si=0; si<asText.GetSize(); si++)
		{
			int nStartPos = 0;
			CString& sDefine = m_aDefines[i].sDef;
			CString& sTest = asText[si];
			int nFound = -1;
			while (1)
			{
				nFound = asText[si].Find(sDefine, nStartPos);
				if (nFound<0)
					break;
				// check if this is full word match?
				if (nFound>0 && IsValidChar(asText[si][nFound-1]))
				{
					nStartPos = nFound+sDefine.GetLength();
					continue;
				}
				nFound += sDefine.GetLength();
				if (nFound < (asText[si].GetLength()) 
					&& IsValidChar(asText[si][nFound]))
				{
					nStartPos = nFound;
					continue;
				}
				// ok, seems to be valid
				m_aDefines[i].nUsages++;
				break;
			}
			if (nFound>=0 && m_aDefines[i].nUsages>2)
				break;
		}
	}
}

void CfindIdsDlg::OnBnClickedOk()
{
	// 1. search all IDs
	m_aDefines.RemoveAll();
	UpdateData();

	CSmartFileIterator iter;
	iter.Init(m_sFolder, m_sFilterRes, TRUE, FALSE);
	SetStatus(_T("Searching defines.."));
	int nFiles = 0;
	while (iter.FindNext())
	{
		CString sShort = iter.m_sRelat;
		sShort += iter.GetData()->cFileName;
		ScanFile(iter.GetFileName(), sShort);
		nFiles++;
	}
	FilterFoundDefines();

	CString sStatus;
	sStatus.Format(_T("%d #define's found in %d files. Searching usages..."), m_aDefines.GetSize(), nFiles);
	SetStatus(sStatus);
	// 2. scan all files
	// 3. for each file - scan for all IDs
	iter.Init(m_sFolder, m_sFilterSrc, TRUE, FALSE);
	nFiles = 0;
	while (iter.FindNext())
	{
		SetSubStatus(iter.GetFileName());
		SearchFile(iter.GetFileName());
		nFiles++;
	}
	m_lbResult.ResetContent();
	nFiles = 0;
	int nLimit = 2;
	if (m_sFilterSrc.Find(_T(".rc"))<0)	// if not scanning RCs, 2 matches is good enough
		nLimit = 1;
	for (int i=0; i<m_aDefines.GetSize(); i++)
	{
		if (m_aDefines[i].nUsages>nLimit)
			continue;
		CString sDefine;
		sDefine.Format(_T("%s : %d (%s)"), m_aDefines[i].sDef, m_aDefines[i].nUsages, m_aDefines[i].sSource);
		int nIdx = m_lbResult.AddString(sDefine);
		m_lbResult.SetItemData(nIdx, i);
		nFiles++;
	}
	sStatus.Format(_T("%d probably unused #define found"), nFiles);
	SetStatus(sStatus);
	SetSubStatus(_T("doubleclick to copy name"));
}

void CfindIdsDlg::FilterFoundDefines()
{
	m_sMask.Trim();
	if (!m_sMask.IsEmpty()) // delete not matched defines
	{
		Filter flt(NULL, m_sMask);
		flt.Ready();

		for (int i=m_aDefines.GetSize()-1; i>=0; i--)
		{
			BOOL bMatch = FALSE;
			for (int m=0; m<flt.GetSize() && !bMatch; m++)
			{
				if (DoesStringMatch(flt[m], m_aDefines[i].sDef))
					bMatch = TRUE;
			}
			if (!bMatch)
				m_aDefines.RemoveAt(i);
		}
	}
}

void CfindIdsDlg::SetStatus(LPCTSTR sText)
{
	SetDlgItemText(IDC_STATUS, sText);
	SetDlgItemText(IDC_STATUS2, NULL);
}
void CfindIdsDlg::SetSubStatus(LPCTSTR sText)
{
	SetDlgItemText(IDC_STATUS2, sText);
}

BOOL SetClipboardText(const CString& szData)
{
	if (szData.IsEmpty())
	{
		if (!::OpenClipboard (NULL))
			return FALSE;
		EmptyClipboard();
		CloseClipboard();
		return TRUE;
	}
	HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(szData.GetLength()+5));
	if (!h)
		return FALSE;
	LPTSTR arr = (LPTSTR)GlobalLock(h);
	if (!arr)
	{
		GlobalFree(h);
		return FALSE;
	}
	_tcscpy_s(arr, szData.GetLength()+5, (LPCTSTR)szData);
	GlobalUnlock(h);

	if (!::OpenClipboard (NULL))
		return NULL;
	EmptyClipboard();
#ifdef _UNICODE
	int nFormat = CF_UNICODETEXT;
#else
	int nFormat = CF_TEXT;
#endif

	SetClipboardData(nFormat, h);
	CloseClipboard();
	return TRUE;
}


void CfindIdsDlg::OnLbnDblclkResult()
{
	int nCurSel = m_lbResult.GetCaretIndex();
	if (nCurSel<0)
		return;
	int i = (int)m_lbResult.GetItemData(nCurSel);
	if (i<0 || i>=m_aDefines.GetSize())
		return;
	SetClipboardText(m_aDefines[i].sDef);
}



void CfindIdsDlg::OnBnClickedFolderBrs()
{
	UpdateData();
	CFolderDialog dlg(m_sFolder);
	if (dlg.DoModal()!=IDOK)
		return;
	m_sFolder = dlg.GetPathName();
	UpdateData(FALSE);
}
