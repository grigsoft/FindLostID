// FltDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fileiter.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void	FullScanDir(LPCTSTR  lpszPath, CStringStack& storage);
#define IsSuchFile(name) ((BOOL)(GetFileAttributes(name)!=(DWORD)-1))

Filter::Filter(LPCTSTR name, LPCTSTR mask)
{
	m_sName=name;
	m_sFull=mask;
}

void Filter::Drop()
{
	SetSize(0);
}

void Filter::Ready()
{
	if (m_sFull.GetLength()==0)
		m_sFull=_T("*.*");
	// scan string and fill array
	CWordArray beg;
	CWordArray end;
	beg.SetSize(50);
	end.SetSize(50);
	WORD i=0, n=0;
	LPCTSTR mask=(LPCTSTR)m_sFull;	// buffer access
	TCHAR inWord=0, ch=mask[0];
	while (ch)
	{
		ch=mask[i++];
		if (ch==' ' || ch==';' || ch==',' || ch==0)
		{
			if (inWord)
			{
				end[n-1]=WORD(i-1);
				inWord=0;
			}
			continue;
		}
		// it's char
		if (inWord==0)
		{
			beg[n++]=(WORD)(i-1);
			inWord=1;
		}
	}
	ASSERT(inWord==0);
	SetSize(n);
	for (i=0; i<n; i++)
	{
		SetAt(i, m_sFull.Mid(beg[i], end[i]-beg[i]));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CString Stack

CStringStack::CStringStack()
{
	SetSize(0, 25);
	top=0;
}
void CStringStack::Push(LPCTSTR str)
{
	SetAtGrow(top++, str);
}

LPCTSTR CStringStack::Pop()
{
	if (!top)
		return NULL;
	top--;
	return ElementAt(top);
}

/////////////////////////////////////////////////////////////////////////
#define S_NONE INVALID_HANDLE_VALUE
CSmartFileIterator::CSmartFileIterator()
{
	m_lSearch=S_NONE;
	m_nFilter=-1;
}

CSmartFileIterator::~CSmartFileIterator()
{
	CloseSearch();
}

void CSmartFileIterator::Init(LPCTSTR  dir, LPCTSTR  filter, BOOL bRecurs, BOOL bDirAsFile)
{
	m_sPath=MakePath(dir);
	m_sRelat=_T("");
	m_nDirLen=m_sPath.GetLength();
	m_bRec=(char)bRecurs;
	m_bDir=(char)bDirAsFile;
	m_Filter.m_sFull=filter;
	m_Filter.Ready();
	m_nFilter=-1;
}
void CSmartFileIterator::StartNewSearch()
{
	CloseSearch();
	ASSERT(m_nFilter>=0 && m_nFilter<m_Filter.GetSize());
	CString str=MakePath(m_sPath, m_Filter.GetAt(m_nFilter));
	m_lSearch=FindFirstFile(str, &m_Data);
}
void CSmartFileIterator::CloseSearch()
{
	if (m_lSearch!=S_NONE)
	{
		FindClose(m_lSearch);
		m_lSearch=S_NONE;
	}
}

int CSmartFileIterator::FindNext()
{
	BOOL ok;
	do
	{
		ok=TRUE;
		if (m_lSearch==S_NONE)	// need new search
		{
			if (m_nFilter<(m_Filter.GetSize()-1))	// goto next filter
			{
				m_nFilter++;
				if (m_bRec && (m_nFilter==0) )	// 1st time in dir : scan subdirs
					FullScanDir(m_sPath, m_Stack);

				StartNewSearch();
			}
			else if (m_Stack.IsEmpty())
				return 0;	// end of search
			else	// go to new subdir
			{
				m_sPath=m_Stack.Pop();
				m_sPath+='\\';
				m_sRelat=m_sPath.Right(m_sPath.GetLength()-m_nDirLen);
				if (m_bDir)
				{
					m_nFilter=-1;	// 1st file found only in next enter
					// notify, that started new subdir
					return 2;
				}
				m_nFilter=0;
				StartNewSearch();
			}
			if (m_lSearch==S_NONE)	// not found by 1st search
			{
				CloseSearch();
				ok=FALSE;
			}
		}
		else if (FindNextFile(m_lSearch, &m_Data)==0)	// not found
		{
			CloseSearch();
			ok=FALSE;
		}
		if (m_lSearch!=S_NONE && m_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{	
			if (!m_bRec && m_bDir 
				&& _tcscmp(m_Data.cFileName, _T(".")) 
				&& _tcscmp(m_Data.cFileName, _T("..")) )
				return 2;	// got dir
			ok=FALSE;
		}
	}
	while(!ok);
	return 1;
}

void FullScanDir(LPCTSTR  lpszPath, CStringStack& storage) 
{
	// Full scan directory with all subfolders
	// and push the items in the storage
	WIN32_FIND_DATA wfd;
	
	if (_tchdir(lpszPath)<0)
		return;
	HANDLE hsr = FindFirstFile(_T("*.*"), &wfd);
	BOOL bRet = ((long)hsr != -1);
	while(bRet) 
	{
		bRet = FindNextFile(hsr, &wfd);
		if( bRet && 
			wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && 
			_tcscmp(wfd.cFileName,_T("..")) )
		{    
			CString sResult = lpszPath;
			if( sResult.GetAt(sResult.GetLength()-1) != '\\' )
				sResult+=_T("\\");
			sResult+=wfd.cFileName;
			storage.Push(sResult);
			TRACE(_T("\nAdded folder: %s"), sResult);
			FullScanDir(sResult, storage);          
		} 
	}
	FindClose(hsr);
}

CString MakePath(LPCTSTR  Dir, LPCTSTR  file)
{
	CString Buf;
	Buf=Dir;
	int l = (int)_tcslen(Dir);
	if (Dir[l-1]!='\\')
		Buf+='\\';
	if (file)
	{
		if (*file=='\\')
			file++;
		Buf+=file;
	}
	return Buf;
}

void CSmartFileIterator::DropDir()
{
	CloseSearch();
	m_nFilter=(int)m_Filter.GetSize();
}
LPCTSTR CSmartFileIterator::GetFileName()
{
	static CString filename;
	filename=m_sPath+m_Data.cFileName;
	return filename;
}

BOOL CreateFolderTree(LPCTSTR dir)
{
	CString sDir(dir);
	int i=2;	// skip _T("e:\") or "\\"
	int l = sDir.GetLength();
	TCHAR* Dir=sDir.GetBuffer();
	while(i<=l)
	{
		i++;
		TCHAR ch=Dir[i];
		if (ch!='\\' && ch!=0)
			continue;
		Dir[i]=0;
		DWORD dwA = GetFileAttributes(Dir);
		if (dwA==(DWORD)-1)
		{
			CreateDirectory(Dir,NULL);
		}
		else if ((dwA & FILE_ATTRIBUTE_DIRECTORY)==0)
		{
			return 0;	// file exists
		}
		Dir[i]=ch;
	}
	return TRUE;
}

BOOL	DeleteFolderTree(LPCTSTR sPath)
{
	if (GetFileAttributes(sPath)==(DWORD)-1)
		return TRUE;

	CStringArray dirRmv;
	dirRmv.SetSize(0, 10);
	int	nRmv=0;
	dirRmv.SetAtGrow(nRmv++, sPath);
	// use brackets to destroy search object-it prevents folder deletion
	{
		CSmartFileIterator iter;
		iter.Init(sPath, _T("*.*"), TRUE, TRUE);
		WIN32_FIND_DATA* fd=iter.GetData();
		int ret=0;
		while ((ret=iter.FindNext())!=0)
		{
			CString sFile=MakePath(iter.m_sPath, fd->cFileName);
			if (ret==1)	// file
			{
				DWORD dw = GetFileAttributes(sFile);
				if (dw & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))
					SetFileAttributes(sFile, FILE_ATTRIBUTE_NORMAL);
				if (!DeleteFile(sFile))
					return FALSE;
				continue;
			}
			dirRmv.SetAtGrow(nRmv++, iter.m_sPath);
		}
	}
	CString sDir = MakePath(sPath, _T(".."));
	SetCurrentDirectory(sDir);
	// OK, looks like we deleted all files in dirs
	for (int i=nRmv-1; i>=0; i--)
	{
		if (!RemoveDirectory(dirRmv[i]))
			return FALSE;
	}
	return TRUE;
}

BOOL DoesStringMatch(LPCTSTR mask, LPCTSTR str)
{
	// make all lowercase 
	CString sMask=mask;
	CString sStr=str;
	sMask.MakeLower();
	sStr.MakeLower();
	if (sMask.Find('.')>=0 && sStr.Find('.')<0)
		sStr+='.';
	str=(LPCTSTR)sStr;
	mask=(LPCTSTR)sMask;

	LPCTSTR pMask=mask;
	LPCTSTR pAsterisk=NULL;
	LPCTSTR pStoredPos=NULL;
	if (!str || !_tcslen(str))
		return FALSE;
	if (!mask || !_tcslen(mask) || !_tcscmp(mask, _T("*")))
		return TRUE;
	while (*pMask)
	{
		TCHAR cM=*pMask++;
		if (!*str && cM!='*')	// for case like _T("aaa") && _T("aaa*")
			return FALSE;
		switch (cM)
		{
		case '?':
			str++;
			break;
		case '*':
			{
				TCHAR c=*pMask;
				if (!c)	// last *
					return TRUE;
				LPCTSTR t = _tcschr(str, c);
				if (!t)
					return FALSE;	// unable to continue
				pAsterisk=pMask-1;
				pMask++;
				pStoredPos=t+1;
				str=t+1;
				break;
			}
		default:
			{
				TCHAR cS=*str++;
				if (cS==cM)		//ok
					continue;
				// failed!, try to move back
				if (!pAsterisk)
					return FALSE;
				// assign last found char to * and goon
				pMask=pAsterisk;
				str=pStoredPos;
			}
		}	// switch
	}	// while
	return (!*str);
}
