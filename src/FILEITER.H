// Flt.h : header file for Fileiterator
//
#ifndef _FILTER_H_
#define _FILTER_H_

#include "afxtempl.h"
#include "io.h"

#define TOOL_EXPORT
#ifndef TOOL_EXPORT
#define TOOL_EXPORT  __declspec(dllimport)
#endif

class Filter  : public CStringArray
{
public:
	Filter(LPCTSTR Name=NULL, LPCTSTR Mask=NULL);
	void Ready();	// scan m_sFull & fill array
	void Drop();	// clear array

	CString m_sName;
	CString m_sFull;
};

class CStringStack : public CStringArray
{
public:
	CStringStack();
	void Push(LPCTSTR str);
	LPCTSTR Pop();
	BOOL IsEmpty() { return (top==0); };
	int top;
};


class TOOL_EXPORT CSmartFileIterator
{
public:
	CSmartFileIterator();
	~CSmartFileIterator();
	
	void Init(LPCTSTR  dir, LPCTSTR  Filters, BOOL bRecurs, BOOL bDirAsFile);
	
	WIN32_FIND_DATA* GetData() { return &m_Data; };
	int FindNext();	// return 0/1/2 : for subdir, if bDirAsFiles
	void DropDir();

	CString m_sPath;	// currrent file path(without name)
	CString	m_sRelat;	// relative path (without start dir)
	LPCTSTR GetFileName();	// get full file name
protected:
	void StartNewSearch();
	void CloseSearch();
	WIN32_FIND_DATA m_Data;
	HANDLE   m_lSearch;
	int		 m_nFilter;
	Filter	 m_Filter;
	char	 m_bRec;
	char	 m_bDir;
	int		 m_nDirLen;	// length of start dir name, for relative path
	CStringStack m_Stack;
};
CString 	TOOL_EXPORT MakePath(LPCTSTR  Dir, LPCTSTR  file=NULL);
BOOL	TOOL_EXPORT CreateFolderTree(LPCTSTR sPath);
BOOL	TOOL_EXPORT DeleteFolderTree(LPCTSTR sPath);	// with files!
BOOL DoesStringMatch(LPCTSTR mask, LPCTSTR str);
#endif
