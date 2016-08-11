#include "stdafx.h"
#include "ufile.h"

#define MAX_LINE	5000
#define BUF_LEN 65536

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CUnicodeFile::CUnicodeFile(int nType)
{
	m_nUnicodeType = nType;
	m_bUTFCheck = FALSE;
	m_pConvert = NULL;
	m_nConvSize = 0;
	m_bSign = 1;
	m_bFullUnicode = FALSE;
	m_pLineBuf = m_pData = NULL;
	m_pWriteBuf = NULL;
	m_nWritePos = 0;
	m_nWriteLen = 0;
	m_nStringLength = 0;
	m_dwTotalRead = 0;
	m_bMoreData = TRUE;
	m_crlf=CRLF_STYLE_DOS;
	m_bKeepCR = FALSE;
	m_nCodePage = CP_ACP;
}
CUnicodeFile::~CUnicodeFile()
{
	delete[] m_pConvert;
	delete[] m_pWriteBuf;
	if (m_pLineBuf)
	{
		delete[] m_pLineBuf;
		delete[] m_pData;
	}
}
void CUnicodeFile::KeepCRs(BOOL bKeep)
{
	m_bKeepCR = bKeep;
}

LPTSTR CUnicodeFile::PrepareConvertBuf(int nLen)
{
	if (m_pConvert && m_nConvSize>=nLen )
		return m_pConvert;
	delete[] m_pConvert;
	m_nConvSize = ((nLen/16)+1)*16;
	m_pConvert = new TCHAR[m_nConvSize];
	return m_pConvert;
}
#ifndef _UNICODE
void CUnicodeFile::Write( const void* lpBuf, UINT nCount )
{
	// should we write unicode sign?
	if (m_nUnicodeType != UNICODE_NONE && m_nUnicodeType != UNICODE_UNKN)
	{
		if (m_bSign)
		{
			m_bSign = 0;
			WORD w = (WORD)m_nUnicodeType;
			BufWrite(&w, 2);
		}
		// convert to unicode
		int nReq = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCTSTR)lpBuf, nCount, NULL, 0);
		int nNew = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCTSTR)lpBuf, nCount, 
			(LPWSTR)PrepareConvertBuf(nReq*2), nReq);
		if (m_nUnicodeType == UNICODE_BE)
		{	//rev chars
			int i = 0;
			while (i<nNew*2)
			{
				TCHAR c = m_pConvert[i+1];	// m_pConvert is prepared by PrepareConvertBuf
				m_pConvert[i+1] = m_pConvert[i];
				m_pConvert[i] = c;
				i += 2;
			}
		}
		BufWrite(m_pConvert, nReq*2);
		return;
	}
	BufWrite(lpBuf, nCount);
}

UINT CUnicodeFile::Read( void* lpBuf, UINT nCount )
{
	if (m_nUnicodeType == UNICODE_UNKN)
	{	
		// detect unicode
		m_nUnicodeType = UNICODE_NONE;
		TCHAR sBuf[4];
		int nSkip = 0;
		if (GetLength() > 4 && CFile::Read(sBuf, 4)==4)
		{
			if  (sBuf[0]=='\xFF' && sBuf[1]=='\xFE') 
			{
				nSkip = 2;
				m_nUnicodeType = UNICODE_LE;
			}
			else if (sBuf[0]!=0 && sBuf[2]!=0 && sBuf[1]==0 && sBuf[3]==0) 
				m_nUnicodeType = UNICODE_LE;
			else if (sBuf[1]=='\xFF' && sBuf[0]=='\xFE')
			{
				nSkip = 2;
				m_nUnicodeType = UNICODE_BE;
			}
			else if (sBuf[1]!=0 && sBuf[3]!=0 && sBuf[0]==0 && sBuf[2]==0)
				m_nUnicodeType  = UNICODE_BE;
		}
		SeekToBegin();
		if (nSkip>0)
			Seek(nSkip, CFile::begin);
	}
	if (m_nUnicodeType == UNICODE_NONE)
	{
		int nRet = CFile::Read(lpBuf, nCount);
		if (nRet<(int)nCount)
			m_bMoreData = FALSE;
		return nRet;
	}
	// read unicode
	int nRead = CFile::Read(PrepareConvertBuf(nCount*2), nCount*2);
	if (nRead<(int)(nCount*2) )
		m_bMoreData = FALSE;
	if (nRead>0)
	{
		if (m_nUnicodeType == UNICODE_BE)
		{
			//rev chars
			int i = 0;
			while (i<nRead)
			{
				TCHAR c = m_pConvert[i+1];
				m_pConvert[i+1] = m_pConvert[i];
				m_pConvert[i] = c;
				i += 2;
			}
		}
		
		if (m_bFullUnicode == 0)	// detect bad chars
		{
			return WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)m_pConvert, nRead/2, 
				(LPTSTR)lpBuf, nCount, NULL, &m_bFullUnicode);
		}
		return WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)m_pConvert, nRead/2, 
			(LPTSTR)lpBuf, nCount, NULL, NULL);
	}
	return 0;
}
#else
// UNICODE versions of read/write file functions
void CUnicodeFile::Write( const void* lpBuf, UINT nCount )
{
	// should we write notice?
	if (m_nUnicodeType != UNICODE_NONE && m_nUnicodeType != UNICODE_UNKN)
	{
		if (m_bSign && m_nUnicodeType != UNICODE_UTF8)
		{
			m_bSign = 0;
			if (m_nUnicodeType == UNICODE_UTF)
			{
				BYTE sign[3] = {0xEF, 0xBB, 0xBF};
				BufWrite(sign, 3);
			}
			else
			{
				WORD w = (WORD)m_nUnicodeType;
				BufWrite(&w, 2);
			}
		}
		LPTSTR pBuf = (LPTSTR)lpBuf;
		if (m_nUnicodeType == UNICODE_BE)
		{	//rev chars
			memcpy(PrepareConvertBuf(nCount), lpBuf, nCount*sizeof(TCHAR));
			UINT i = 0;
			while (i<nCount)
			{
				TCHAR c = m_pConvert[i];	// m_pConvert is prepared by PrepareConvertBuf
				TCHAR cn = MAKEWORD(HIBYTE(c), LOBYTE(c));
				m_pConvert[i] = cn;
				i++;
			}
			pBuf = m_pConvert;
		}
		else if (m_nUnicodeType == UNICODE_UTF8 || m_nUnicodeType == UNICODE_UTF)
		{
			// convert to utf8
			int nReq = WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)lpBuf, nCount, NULL, 0,
				NULL, NULL);
			int nNew = WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)lpBuf, nCount, 
				(LPSTR)PrepareConvertBuf(nReq+3), nReq, NULL, NULL);
			
			BufWrite(m_pConvert, nNew);
			return;
		}
		BufWrite(pBuf, nCount*sizeof(TCHAR));
		return;
	}
	// back conversion is requred
	int nReq = WideCharToMultiByte(m_nCodePage, WC_COMPOSITECHECK, (LPCTSTR)lpBuf, nCount, NULL, 0,
		NULL, NULL);
	int nNew = WideCharToMultiByte(m_nCodePage, WC_COMPOSITECHECK, (LPCTSTR)lpBuf, nCount, 
		(LPSTR)PrepareConvertBuf(nReq+3), nReq, NULL, NULL);
	
	BufWrite(m_pConvert, nNew);
}

UINT CUnicodeFile::Read( void* lpBuf, UINT nCount )
{
	if (m_nUnicodeType == UNICODE_UNKN)
	{	
		// detect unicode
		m_nUnicodeType = UNICODE_NONE;
		BYTE sBuf[4];
		int nSkip = 0;
		int nRead = 0;
		if (GetLength() >= 4)
			nRead = 4;
		else if (GetLength()>=2)
			nRead = 2;
		if (nRead && CFile::Read(sBuf, nRead)==(UINT)nRead)
		{
			if  (nRead>2 && sBuf[0]==0xeF && sBuf[1]==0xBB && sBuf[2]==0xBF ) 
			{
				nSkip = 3;
				m_nUnicodeType = UNICODE_UTF;
			}
			if  (sBuf[0]==0xFF && sBuf[1]==0xFE) 
			{
				nSkip = 2;
				m_nUnicodeType = UNICODE_LE;
			}
			else if (nRead>2 && sBuf[0]!=0 && sBuf[2]!=0 && sBuf[1]==0 && sBuf[3]==0) 
				m_nUnicodeType = UNICODE_LE;
			else if (sBuf[1]==0xFF && sBuf[0]==0xFE)
			{
				nSkip = 2;
				m_nUnicodeType = UNICODE_BE;
			}
			else if (nRead>2 && sBuf[1]!=0 && sBuf[3]!=0 && sBuf[0]==0 && sBuf[2]==0)
				m_nUnicodeType  = UNICODE_BE;
		}
		SeekToBegin();
		if (nSkip>0)
			Seek(nSkip, CFile::begin);
	}
	if (m_nUnicodeType == UNICODE_NONE || m_nUnicodeType == UNICODE_UTF 
		|| m_nUnicodeType == UNICODE_UTF8)
	{
		UINT nRead = CFile::Read(PrepareConvertBuf(nCount), nCount);
		// translation is required
		if (nRead<=0)
		{
			m_bMoreData = FALSE;
			return 0;
		}
		if (nRead<nCount)
			m_bMoreData = FALSE;
		// UTF check
		if (m_nUnicodeType == UNICODE_NONE && !m_bUTFCheck)
		{
			// search for UTF8 sequence
			BOOL bUTF8 = FALSE;
			BYTE* pBBuf = (BYTE*)m_pConvert;
			UINT i=0;
			while (i<nRead)
			{
				if (pBBuf[i]>=0x80 && pBBuf[i]<0xC0 )
				{
					m_bUTFCheck = TRUE;
					break;	// it's not UTF8
				}
				if (pBBuf[i]>=0xC0)	// starting character for UTF?
				{
					int nMoreChars = 1;
					if (pBBuf[i]>=0xE0)
						nMoreChars = 2;
					if (pBBuf[i]>=0xF0)
						nMoreChars = 3;
					if (pBBuf[i]>=0xF8)
						nMoreChars = 4;
					if (i+nMoreChars>=nRead)
						nMoreChars = nRead-i-1;

					for (int check=1; check<=nMoreChars; check++)
					{
						if (pBBuf[i+check]<0x80 || pBBuf[i+check]>=0xC0)
						{
							m_bUTFCheck = TRUE;
							break;	// it's not UTF8
						}
					}
					if (m_bUTFCheck)
						break;
					bUTF8 = TRUE;
					i += nMoreChars;
				}
				i++;
			}
			if (bUTF8 && !m_bUTFCheck)
				m_nUnicodeType = UNICODE_UTF8;
		}
		if (m_nUnicodeType == UNICODE_UTF8 || m_nUnicodeType == UNICODE_UTF)
		{
			// check for not complete UTF character at end
			BYTE* pBBuf = (BYTE*)m_pConvert;
			int nXChars = 1;
			while (pBBuf[nRead-nXChars]>=0x80)
			{
				if (pBBuf[nRead-nXChars]>=0xC0)	// OK, this is first
				{
					if (nRead>(UINT)nXChars)
					{
						CFile::Seek(-nXChars, CFile::current);
						nRead -= nXChars;
						m_bMoreData = TRUE;
					}
					break;
				}
				if (nXChars>6)
				{
					m_nUnicodeType = UNICODE_NONE;
					m_bUTFCheck = TRUE;
					break;
				}
				nXChars++;
			}
			nRead = MultiByteToWideChar(CP_UTF8, 0, 
				(LPCSTR)m_pConvert, nRead, (LPTSTR)lpBuf, nCount);
		}
		
		if (m_nUnicodeType == UNICODE_NONE )
		{
			// bug fix: even plain text can include 2-byte symbols (japanese)
			// since there is no simple way to understand sequence,
			// skip back till we will find either plain text or last char (<0x80)
			BYTE* pBBuf = (BYTE*)m_pConvert;
			if (nRead>2 && pBBuf[nRead-1]>=0x80)
			{
				int nBad = 2;
				while ((int)nRead>nBad && pBBuf[nRead-nBad]>=0x80 && nBad<50)	// limit scan back in russian files
					nBad++;
				if ((int)nRead>nBad)
				{
					nBad--;
					CFile::Seek(-nBad, CFile::current);
					nRead -= nBad;
					if (nBad>0)
						m_bMoreData = TRUE;
				}
			}
			nRead = MultiByteToWideChar(m_nCodePage, MB_PRECOMPOSED, 
			  (LPCSTR)m_pConvert, nRead, (LPTSTR)lpBuf, nCount);
		}
		/*
		else if (m_nUnicodeType == UNICODE_UTF)
		{
			nRead = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)m_pConvert, nRead, 
				(LPTSTR)lpBuf, nCount);
		}
		*/

		return nRead;
	}
	// read unicode
	int nRead = CFile::Read(lpBuf, nCount*sizeof(TCHAR));
	if (nRead < (int)(nCount*sizeof(TCHAR)))
		m_bMoreData = FALSE;
	if (nRead>0)
	{
		if (m_nUnicodeType == UNICODE_BE)
		{
			//rev chars
			int i = 0;
			LPTSTR pStr = (LPTSTR)lpBuf;
			int nChars = nRead/sizeof(TCHAR);
			while (i<nChars)
			{
				TCHAR c = pStr[i];
				TCHAR cn = MAKEWORD(HIBYTE(c), LOBYTE(c));
				pStr[i] = cn;
				i++;
			}
		}
		return nRead/sizeof(TCHAR);
	}
	return 0;
}
#endif
LPCTSTR CUnicodeFile::ReadString()
{
	if (!m_pLineBuf)		// 1st time init
	{
		m_pLineBuf =new TCHAR[MAX_LINE];
		m_pData=new TCHAR[BUF_LEN];
		m_dwLineMax=MAX_LINE;
		m_dwRead = Read(m_pData, BUF_LEN );
		m_dwTotalRead = m_dwRead;

		// first get crlf mode
		m_crlf=CRLF_STYLE_DOS;
		LPCTSTR pCR=(LPCTSTR)memchr(m_pData, _T('\x0A'), m_dwRead);
		if (!pCR)
		{
			pCR = (LPCTSTR)memchr(m_pData, '\x0D', m_dwRead);
			if (!pCR)
				m_crlf = CRLF_STYLE_DOS;
			else
				m_crlf = CRLF_STYLE_CR;
		}
		else
		{
			DWORD I=(DWORD)(pCR-m_pData);
			//	Otherwise, analyse the first occurance of line-feed character
			if (I>0 && *(pCR-1) == _T('\x0d'))
			{
				m_crlf  = CRLF_STYLE_DOS;
			}
			else
			{
				if (I < (m_dwRead - 1) && *(pCR+1) == _T('\x0d'))
					m_crlf  = CRLF_STYLE_UNIX;
				else
					m_crlf  = CRLF_STYLE_MAC;
			}
		}
		m_dwBufPtr = 0;
		m_nCurrentLength = 0;
		m_nPrev=-1;
	}
	while (1)
	{
		// do we need another portion?
		if (m_dwBufPtr >= m_dwRead)
		{
			// BUG: m_dwRead<BUF_LEN can be true after UTF conversion
			if (!m_bMoreData/*m_dwRead<BUF_LEN*/)
			{
				// it was last portion: do we have line to return?
				if (m_nCurrentLength>0)
				{
					if (m_nPrev==0x0A)
					{
						m_fLine = CRLF_STYLE_MAC;
						m_nCurrentLength--;
					}
					else if (m_nPrev==0x0D)
					{
						m_nCurrentLength--;
						m_fLine = CRLF_STYLE_CR;
					}
					else
						m_fLine = CRLF_STYLE_NONE;
					m_pLineBuf[m_nCurrentLength]=0;
					m_nStringLength = m_nCurrentLength;
					m_nCurrentLength = 0;
					ReturnCR();
					return m_pLineBuf;
				}
				else
				{
					if (m_fLine == CRLF_STYLE_NONE)	// previous line was without CR
						m_fLine = CRLF_STYLE_DEFAULT;	// to disable last line add
					else
						m_fLine = CRLF_STYLE_NONE;
					m_nStringLength = 0;
					ReturnCR();
					return NULL;
				}
			}
			else
			{
				m_dwRead=Read(m_pData, BUF_LEN );
				m_dwTotalRead += m_dwRead;
				m_dwBufPtr = 0;
			}
		}

		TCHAR c = m_pData[m_dwBufPtr++];
		if (c!=0)
			m_pLineBuf[m_nCurrentLength++] = (TCHAR) c;
		else
			m_pLineBuf[m_nCurrentLength++] = (TCHAR)183;

		if ((DWORD)m_nCurrentLength==m_dwLineMax)
		{
			m_dwLineMax += 1024;
			if (m_dwLineMax>50000)
				m_dwLineMax *= 2;
			TCHAR* pcNewBuf = new TCHAR[m_dwLineMax];
			memcpy(pcNewBuf, m_pLineBuf, m_nCurrentLength*sizeof(TCHAR));
			delete[] m_pLineBuf;
			m_pLineBuf = pcNewBuf;
		}
		BYTE bLine=CRLF_STYLE_DEFAULT;
		if ( m_nPrev==0x0A && c==0x0D)
			bLine=CRLF_STYLE_UNIX;
		else if (m_nPrev==0x0D && c==0x0A)
			bLine=CRLF_STYLE_DOS;
		else if (m_nPrev==0x0A || (m_nPrev==0 && m_nCurrentLength>50))//for binary files
			bLine=CRLF_STYLE_MAC;
		else if (m_nPrev==0x0D)
			bLine=CRLF_STYLE_CR;
		m_nPrev=c;
		if (bLine!=CRLF_STYLE_DEFAULT) // got line end!
		{
			m_pLineBuf[m_nCurrentLength - 2] = 0;
			m_nStringLength = m_nCurrentLength - 2;
			m_nCurrentLength = 0;
			m_fLine = bLine;
			ReturnCR();
			if (bLine==CRLF_STYLE_MAC || bLine==CRLF_STYLE_CR)	// start new line with c char
				m_dwBufPtr--;	// reuse that char again
			m_nPrev=-1;
			return m_pLineBuf;
		}
	}
	ASSERT(FALSE);
	m_nStringLength = 0;
	return NULL;
}

TCHAR* szCRs[]=
{
	_T("\r\n"),	// dos
	_T("\n\r"),	// unix
	_T("\n"),	// mac
	_T("\r"),
};

// add current line end to buffer
//  normally should not just skip it
void CUnicodeFile::ReturnCR()
{
	if (!m_bKeepCR || m_fLine<0 || m_fLine>(sizeof(szCRs)/sizeof(szCRs[0])))
		return;
	
	m_pLineBuf[m_nStringLength++] = szCRs[m_fLine][0];
	if (m_fLine<2)	// only for these there is 2nd char
		m_pLineBuf[m_nStringLength++] = szCRs[m_fLine][1];
}

LPCTSTR CUnicodeFile::ReadString(CString& s)
{
	LPCTSTR p = ReadString();
	if (!p)
	{
		s.Empty();
		return NULL;
	}
	s = p;
	return (LPCTSTR)s;
}
void CUnicodeFile::WriteFlush()
{
	if (!m_pWriteBuf || m_nWritePos<=0)
		return;	// no buffered write
	CFile::Write(m_pWriteBuf, m_nWritePos);
	m_nWritePos = 0;
}
void CUnicodeFile::Close()
{
	WriteFlush();
	CFile::Close();
}


void CUnicodeFile::BufWrite( const void* lpBuf, UINT nCount )
{
	if (nCount==0)
		return;
	if (!m_pWriteBuf)
	{
		m_nWriteLen = 60000;
		m_pWriteBuf = new BYTE[m_nWriteLen];
		m_nWritePos = 0;
	}
	if (nCount + m_nWritePos > (UINT)m_nWriteLen )	// this would not fix in buffer
		WriteFlush();	// drop bufer to disk
	if (nCount + m_nWritePos > (UINT)m_nWriteLen )	// it would not fit in empty buf?
	{
		CFile::Write(lpBuf, nCount);
		return;
	}
	// OK, just add to buffer
	memcpy(m_pWriteBuf+m_nWritePos, lpBuf, nCount);
	m_nWritePos += nCount;
}
void CUnicodeFile::WriteString( LPCTSTR buf)
{
	if (!buf)
		return;
	Write(buf, (UINT)_tcslen(buf));
}

static const TCHAR *crlfs[] =
{
	_T("\x0d\x0a"),			//	DOS/Windows style
	_T("\x0a\x0d"),			//	UNIX style
	_T("\x0a"),				//	Macintosh style
	_T("\x0d"),				//	unknown LF style
};

void CUnicodeFile::WriteNewLine( int nCrlf )
{
    if (nCrlf>=sizeof(crlfs)/sizeof(crlfs[0]) || nCrlf < 0)
		nCrlf = CRLF_STYLE_DOS;
	WriteString(crlfs[nCrlf]);
}

#ifndef _UNICODE
void CUnicodeFile::WriteString( WCHAR* buf)
{
	if (!buf)
		return;
	
	int nLen = wcslen(buf);
	if (m_nUnicodeType == UNICODE_BE)
	{	//rev chars
		int i = 0;
		BYTE* pBytes = (BYTE*)buf;
		while (i<nLen*2)
		{
			BYTE c = pBytes[i+1];
			pBytes[i+1] = pBytes[i];
			pBytes[i] = c;
			i += 2;
		}
	}
	BufWrite(buf, nLen*2);
}
#endif

BOOL CUnicodeFile::SafeOpen(LPCTSTR sFile)
{
	m_bCloseOnDelete = FALSE;
	m_hFile = hFileNull;

	m_strFileName = sFile;

	HANDLE hFile = ::CreateFile(sFile, 
		GENERIC_READ,  FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		FILE* pFile = _tfopen(sFile, _T("rb"));
		if (!pFile)
			return 0;
		if (pFile)
			fclose(pFile);

		return FALSE;
	}
	m_hFile = hFile;
	m_bCloseOnDelete = TRUE;
	return TRUE;
}
