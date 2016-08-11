#ifndef _UFILE_H_
#define _UFILE_H_

#define TOOL_EXPORT
#ifndef TOOL_EXPORT
#define TOOL_EXPORT  __declspec(dllimport)
#endif

enum CRLFSTYLE
{
    CRLF_STYLE_AUTOMATIC    = -1,   // Slave \ IDE
	CRLF_STYLE_DOS			= 0,	// DA
	CRLF_STYLE_UNIX			= 1,	// AD   in fact: UNUSED
	CRLF_STYLE_MAC			= 2,	// A	in fact: UNIX
	CRLF_STYLE_CR			= 3,	// D	in fact: MAC
	CRLF_STYLE_DEFAULT		= 4,	// use file detected, should be last!
	CRLF_STYLE_NONE			= 5,	// no LF at line end (file end)
	CRLF_MASK				= 7,	// 
	XF_MATCHES				= 0x08,	// m_pMatches is valid
	XF_REGEXP				= 0x10,	// regexp were applied
	XF_RE_CUT				= 0x20,	// regexp cut file at this point
};

enum UnicodeType
{
	UNICODE_UNKN =	-5,
	UNICODE_NONE = 0,
	UNICODE_BE	 = 0xFFFE,		// Note: on save byte order in sign is reversed!
	UNICODE_LE	 = 0xFEFF,
	UNICODE_UTF	 = 0xEFBB00BF,
#ifdef _UNICODE
	UNICODE_UTF8 = 0xF8,		// UTF8, no special file signature required
#endif
};

enum CodePages
{
	CP_FONT	=	-1,
	CP_ARABIC = 1256,
	CP_BALTIC = 1257,
	CP_K = 949,
	CP_J = 932,
	CP_JEW = 1255,
	CP_CHT = 950,
	CP_CHS = 936,
	CP_CE = 1250,
	CP_WE = 1252,
	CP_CYR = 1251,
	CP_KOI = 20866,
	CP_GREEK = 1253,
	CP_TURC = 1254,
	CP_V = 1258,
	CP_THAI = 874,
};


class TOOL_EXPORT CUnicodeFile : public CFile
{
public:			   
	CUnicodeFile(int nType = UNICODE_UNKN);
	~CUnicodeFile();

	BOOL SafeOpen(LPCTSTR sFile);	// for reading only
	virtual void Write( const void* lpBuf, UINT nCount );
	void WriteString( LPCTSTR buf);
#ifndef _UNICODE
	void WriteString( WCHAR* buf);	// still keep an ability to write unicode
#endif
	void WriteFlush();
	virtual void Close();
	virtual UINT Read( void* lpBuf, UINT nCount );
	void WriteNewLine( int nCrlf = CRLF_STYLE_DOS);

	int GetUnicodeMode() {return m_nUnicodeType;};
	void SetUnicodeMode(UnicodeType utype) {m_nUnicodeType = utype;};

	int SetCodePage(int nCP) {int nOld =m_nCodePage; m_nCodePage = nCP; return nOld;};
	
	void	KeepCRs(BOOL bKeep = TRUE);

	BOOL	m_bFullUnicode;	// 1, if non-std chars encountered

	// line read support
	LPCTSTR ReadString();
	LPCTSTR ReadString(CString& s);
	BYTE	m_fLine;	// crlf of last line
	int		m_crlf;		// file crlf - default for new lines
	DWORD	m_nStringLength;	// last ReadString() return length
	DWORD	m_dwTotalRead;		// for progress
protected:
	void ReturnCR();	// add current line end to buffer
	void BufWrite( const void* lpBuf, UINT nCount );
	LPTSTR	PrepareConvertBuf(int nLen);
	BOOL	m_bMoreData;		// false, after could not read required num of bytes
	int		m_nUnicodeType;		// unicode type
	BOOL	m_bUTFCheck;		// UTF8 check done w/o success
	LPTSTR	m_pConvert;		// area for converter
	int		m_nConvSize;
	BOOL	m_bSign;
	//
	LPTSTR	m_pLineBuf;		// cur line buf;
	DWORD	m_dwLineMax;	// line buf len
	LPTSTR	m_pData;		// read Buffer
	DWORD	m_dwRead;		// characters in buf
	DWORD	m_dwBufPtr;		// cur char in buf
	int		m_nCurrentLength;	// cur line len
	int		m_nPrev;		// prev char (for crlf mode detect)
	LPBYTE	m_pWriteBuf;
	int		m_nWritePos;
	int		m_nWriteLen;
	BOOL	m_bKeepCR;
	int		m_nCodePage;
};


#endif